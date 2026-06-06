/*
 * VPNScreener.cpp
 *
 * See VPNScreener.h for design notes.
 *
 * HTTP transport: popen() + the system `curl` binary. cpprestsdk is gated
 * behind ENABLE_SWGREALMS in this codebase and not available in dev builds,
 * and pulling it in just for one synchronous GET would be heavyweight. The
 * `curl` binary is universal and the fork+exec overhead only hits on cache
 * miss (rare after warmup).
 */

#include "VPNScreener.h"

#include "server/db/ServerDatabase.h"
#include "conf/ConfigManager.h"
#include "engine/util/json.hpp"

#include <cstdio>
#include <cstdlib>
#include <string>

VPNScreener::VPNScreener() : Logger("VPNScreener") {
	enabled = false;
	cacheTTLHours = 24;
	apiTimeoutMs = 3000;
	apiBaseURL = "https://vpnapi.io/api/";
}

VPNScreener::~VPNScreener() {}

void VPNScreener::initialize() {
	auto config = ConfigManager::instance();

	enabled = config->getBool("Core3.Login.VPNAutoBlock.Enabled", false);

	if (!enabled) {
		info(true) << "VPNScreener: disabled by config (Core3.Login.VPNAutoBlock.Enabled=false)";
		return;
	}

	cacheTTLHours = config->getInt("Core3.Login.VPNAutoBlock.CacheTTLHours", 24);
	apiTimeoutMs = config->getInt("Core3.Login.VPNAutoBlock.APITimeoutMs", 3000);

	String configBaseURL = config->getString("Core3.Login.VPNAutoBlock.APIBaseURL", "");

	if (!configBaseURL.isEmpty())
		apiBaseURL = configBaseURL;

	const char* envKey = getenv("VPNAPI_KEY");

	if (envKey == nullptr || envKey[0] == '\0') {
		warning() << "VPNScreener: enabled but VPNAPI_KEY not set in environment — "
				  << "screener will fail-open on every login. Source ~/.vpnapi.env "
				  << "before launching core3.";
		enabled = false;
		return;
	}

	apiKey = envKey;

	info(true) << "VPNScreener: enabled (cacheTTLHours=" << cacheTTLHours
			   << ", apiTimeoutMs=" << apiTimeoutMs
			   << ", apiBaseURL=" << apiBaseURL << ")";
}

namespace {

// True for loopback (127.0.0.0/8, ::1) and RFC1918 private ranges
// (10/8, 172.16-31/12, 192.168/16). A same-host or LAN login (chat
// bridges, internal services) is never a VPN/proxy, so it is exempted
// up front: no allowlist row, no vpnapi.io call, no quota burn.
bool isLoopbackOrPrivate(const String& ip) {
	if (ip == "::1")
		return true;

	int o1 = -1, o2 = -1;

	if (sscanf(ip.toCharArray(), "%d.%d", &o1, &o2) < 1)
		return false;

	// loopback 127/8 + RFC1918 private 10/8, 172.16-31/12, 192.168/16
	if (o1 == 127 || o1 == 10)
		return true;
	if (o1 == 192 && o2 == 168)
		return true;
	if (o1 == 172 && o2 >= 16 && o2 <= 31)
		return true;

	return false;
}

} // anonymous namespace

VPNScreener::CheckResult VPNScreener::checkIP(const String& ip, const String& username) {
	CheckResult result;
	result.verdict = ALLOW;
	result.reason = "screener_disabled";

	if (!enabled)
		return result;

	if (ip.isEmpty()) {
		result.reason = "empty_ip";
		return result;
	}

	if (isLoopbackOrPrivate(ip)) {
		result.reason = "loopback_or_private";
		return result;
	}

	// Per-account allowlist — the only allowlist path. Players on rotating VPN
	// exit IPs (or staff) are whitelisted by SWG account username. The legacy
	// IP-based allowlist (swgemu.vpn_allowlist) was removed 2026-05-24; staff
	// and trusted accounts are now expressed as username rows in
	// swgemu.vpn_username_allowlist. Emit at INFO so each bypass is visible
	// in the audit trail.
	if (!username.isEmpty() && isUsernameAllowed(username)) {
		info(true) << "VPNScreener: username allowlist hit (" << username << ") — skipping VPN check for " << ip;
		result.reason = "username_allowlisted";
		return result;
	}

	CheckResult cached;

	if (readCache(ip, cached)) {
		return cached;
	}

	if (!callVPNAPI(ip, result)) {
		result.verdict = ALLOW;
		result.reason = "api_failed_open";
		warning() << "VPNScreener: API call failed for " << ip << " — failing open";
		return result;
	}

	bool isVPN = (result.verdict == BLOCK_VPN);

	writeCache(ip, result, isVPN);

	return result;
}

bool VPNScreener::isUsernameAllowed(const String& username) {
	// Defense-in-depth: skip the DB roundtrip on obviously-bogus input before
	// hitting escapeString. The column is VARCHAR(64); anything past that is
	// guaranteed not to match and burns a query for no reason.
	if (username.isEmpty() || username.length() > 64)
		return false;

	String escaped = username;
	Database::escapeString(escaped);

	// utf8mb4_general_ci collation on the column gives case-insensitive
	// matching — no LOWER() needed on either side. PRIMARY KEY point lookup,
	// so this is a O(1) index hit even at growth.
	StringBuffer query;
	query << "SELECT 1 FROM swgemu.vpn_username_allowlist WHERE username = '" << escaped << "' LIMIT 1";

	try {
		UniqueReference<ResultSet*> res(ServerDatabase::instance()->executeQuery(query.toString()));

		if (res != nullptr && res->next())
			return true;
	} catch (const Exception& e) {
		error() << "VPNScreener: username allowlist query failed: " << e.getMessage();
	}

	return false;
}

bool VPNScreener::readCache(const String& ip, CheckResult& out) {
	String escaped = ip;
	Database::escapeString(escaped);

	StringBuffer query;
	query << "SELECT cidr, is_vpn, is_proxy, is_tor, is_relay, provider, "
		  << "TIMESTAMPDIFF(HOUR, checked_at, NOW()) AS age_hours "
		  << "FROM swgemu.vpn_ip_cache WHERE ip = '" << escaped << "' LIMIT 1";

	try {
		UniqueReference<ResultSet*> res(ServerDatabase::instance()->executeQuery(query.toString()));

		if (res == nullptr || !res->next())
			return false;

		int ageHours = res->getInt(6);

		if (ageHours > cacheTTLHours)
			return false;

		String cidr = res->getString(0);
		bool isVPN   = res->getBoolean(1);
		bool isProxy = res->getBoolean(2);
		bool isTor   = res->getBoolean(3);
		bool isRelay = res->getBoolean(4);
		String provider = res->getString(5);

		bool flagged = (isVPN || isProxy || isTor || isRelay);

		out.verdict = flagged ? BLOCK_VPN : ALLOW;
		out.reason = flagged ? ("cache_vpn:" + provider) : "cache_clean";
		out.cidr = cidr;
		out.provider = provider;

		return true;
	} catch (const Exception& e) {
		error() << "VPNScreener: cache read failed: " << e.getMessage();
		return false;
	}
}

void VPNScreener::writeCache(const String& ip, const CheckResult& result, bool isVPN) {
	String escapedIP = ip;
	Database::escapeString(escapedIP);
	String escapedCIDR = result.cidr;
	Database::escapeString(escapedCIDR);
	String escapedProvider = result.provider;
	Database::escapeString(escapedProvider);

	StringBuffer query;
	query << "REPLACE INTO swgemu.vpn_ip_cache "
		  << "(ip, cidr, is_vpn, is_proxy, is_tor, is_relay, provider, asn, checked_at) VALUES ('"
		  << escapedIP << "', '" << escapedCIDR << "', "
		  << (isVPN ? 1 : 0) << ", 0, 0, 0, '"
		  << escapedProvider << "', '', NOW())";

	try {
		ServerDatabase::instance()->executeStatement(query.toString());
	} catch (const Exception& e) {
		error() << "VPNScreener: cache write failed: " << e.getMessage();
	}
}

namespace {

// Reject anything outside [0-9a-fA-F.:] so we can safely embed IP in a shell
// command argv. Real IP/IPv6 form check happens server-side anyway; this is
// just shell-injection defense.
bool isShellSafeIP(const String& ip) {
	if (ip.isEmpty() || ip.length() > 45)
		return false;

	for (int i = 0; i < ip.length(); ++i) {
		char c = ip.charAt(i);
		bool ok = (c >= '0' && c <= '9')
			|| (c >= 'a' && c <= 'f')
			|| (c >= 'A' && c <= 'F')
			|| c == '.'
			|| c == ':';

		if (!ok)
			return false;
	}

	return true;
}

// Same defense for the API key (vpnapi.io keys are alphanumeric).
bool isShellSafeKey(const String& key) {
	if (key.isEmpty() || key.length() > 128)
		return false;

	for (int i = 0; i < key.length(); ++i) {
		char c = key.charAt(i);
		bool ok = (c >= '0' && c <= '9')
			|| (c >= 'a' && c <= 'z')
			|| (c >= 'A' && c <= 'Z');

		if (!ok)
			return false;
	}

	return true;
}

} // anonymous namespace

bool VPNScreener::callVPNAPI(const String& ip, CheckResult& out) {
	if (!isShellSafeIP(ip)) {
		error() << "VPNScreener: rejecting unsafe IP literal: " << ip;
		return false;
	}

	if (!isShellSafeKey(apiKey)) {
		error() << "VPNScreener: API key contains unexpected characters; refusing to invoke curl";
		return false;
	}

	int timeoutSec = (apiTimeoutMs / 1000);

	if (timeoutSec < 1)
		timeoutSec = 1;

	StringBuffer cmdBuf;
	cmdBuf << "curl -s --max-time " << timeoutSec
		   << " '" << apiBaseURL << ip.toCharArray() << "?key=" << apiKey.toCharArray() << "'";

	String cmd = cmdBuf.toString();

	FILE* pipe = popen(cmd.toCharArray(), "r");

	if (pipe == nullptr) {
		error() << "VPNScreener: popen failed for " << ip;
		return false;
	}

	std::string body;
	body.reserve(2048);

	char buf[1024];

	while (fgets(buf, sizeof(buf), pipe) != nullptr) {
		body.append(buf);
	}

	int rc = pclose(pipe);

	if (rc != 0) {
		error() << "VPNScreener: curl exited with rc=" << rc << " for " << ip;
		return false;
	}

	if (body.empty()) {
		error() << "VPNScreener: empty response from vpnapi.io for " << ip;
		return false;
	}

	try {
		auto json = nlohmann::json::parse(body);

		if (!json.contains("security") || !json.contains("network")) {
			error() << "VPNScreener: vpnapi.io response missing security or network fields for " << ip;
			return false;
		}

		const auto& security = json["security"];
		const auto& network  = json["network"];

		bool isVPN   = security.value("vpn",   false);
		bool isProxy = security.value("proxy", false);
		bool isTor   = security.value("tor",   false);
		bool isRelay = security.value("relay", false);

		String cidr     = network.value("network", std::string("")).c_str();
		String provider = network.value("autonomous_system_organization", std::string("")).c_str();

		bool flagged = (isVPN || isProxy || isTor || isRelay);

		out.verdict = flagged ? BLOCK_VPN : ALLOW;
		out.reason = flagged ? ("api_vpn:" + provider) : "api_clean";
		out.cidr = cidr;
		out.provider = provider;

		return true;
	} catch (const std::exception& e) {
		error() << "VPNScreener: JSON parse failed for " << ip << ": " << e.what();
		return false;
	}
}
