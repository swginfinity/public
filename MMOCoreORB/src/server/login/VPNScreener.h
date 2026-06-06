/*
 * VPNScreener.h
 *
 * Screens an IP at login against an allowlist + cached vpnapi.io verdicts,
 * with a synchronous API call on cache miss. Verdict is consumed in
 * AccountManager::loginAccount immediately after the brute-force check.
 *
 * Design (locked 2026-05-02 MrO + 2026-05-04 Deej greenlight):
 *   - Storage: same swgemu MariaDB instance Core3 already uses for accounts
 *     (no LAN dependency on the login path).
 *   - API: vpnapi.io with key from ~/.vpnapi.env (CIDR + provider + verdict
 *     all in one response, 10K/day budget).
 *   - Cache TTL configurable; allowlist matched first to skip the API.
 *   - Fail-open on API error so vpnapi.io outages can't lock players out.
 */

#ifndef _VPNSCREENER_H
#define _VPNSCREENER_H

#include "engine/engine.h"

class VPNScreener : public Singleton<VPNScreener>, public Logger, public Object {
public:
	enum Verdict {
		ALLOW = 0,           // allowlisted, cached-clean, fresh-clean, or fail-open
		BLOCK_VPN = 1,       // verdict from cache or API: vpn || proxy || tor || relay
	};

	struct CheckResult {
		Verdict verdict;
		String reason;       // for logging: "allowlisted", "cache_clean", "api_clean", "cache_vpn", "api_vpn:<provider>", "api_failed_open"
		String cidr;         // populated when verdict is BLOCK_VPN
		String provider;     // populated when verdict is BLOCK_VPN
	};

	VPNScreener();
	~VPNScreener();

	void initialize();

	// Main entry point. Synchronous: hits the cache or vpnapi.io with a short
	// timeout. Always safe to call; returns ALLOW with reason "screener_disabled"
	// when the runtime flag is off.
	//
	// `username` (optional, empty by default) is the SWG login username. When
	// non-empty it is consulted against `vpn_username_allowlist` after the
	// loopback/private fast-path and before the IP allowlist — an allowlisted
	// username bypasses the screener regardless of the source IP. This is the
	// per-account allowlist mechanism added 2026-05-23 so a player using a VPN
	// whose exit IP rotates can still log in.
	CheckResult checkIP(const String& ip, const String& username = "");

	bool isEnabled() const { return enabled; }

	// Runtime kill-switch — flip the screener off (or back on) without a
	// restart. Console-driven via the `vpn enable|disable` command in
	// ServerCore. Use case: emergency disable if vpnapi.io classifications
	// are causing widespread legit-player blocks.
	void setEnabled(bool e) { enabled = e; }

private:
	bool enabled;
	int cacheTTLHours;
	int apiTimeoutMs;
	String apiKey;
	String apiBaseURL;

	bool isUsernameAllowed(const String& username);
	bool readCache(const String& ip, CheckResult& out);
	void writeCache(const String& ip, const CheckResult& result, bool isVPN);

	// Synchronous vpnapi.io call. Returns false on API failure (caller fail-opens).
	bool callVPNAPI(const String& ip, CheckResult& out);
};

#endif // _VPNSCREENER_H
