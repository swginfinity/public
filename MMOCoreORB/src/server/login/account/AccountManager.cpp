/*
 * AccountManager.cpp
 *
 *  Created on: Oct 13, 2010
 *      Author: crush
 */

#include "server/login/account/Account.h"
#include "AccountManager.h"
#include "server/login/LoginClient.h"
#include "server/login/LoginServer.h"
#include "server/login/VPNScreener.h"
#ifdef WITH_SWGREALMS_API
#include "server/login/SWGRealmsAPI.h"
#endif // WITH_SWGREALMS_API
#include "server/login/packets/AccountVersionMessage.h"
#include "server/login/packets/EnumerateCharacterId.h"
#include "server/login/packets/LoginClientToken.h"
#include "server/login/packets/LoginClusterStatus.h"
#include "server/login/packets/LoginEnumCluster.h"
#include "server/ServerCore.h"

#include "server/zone/managers/object/ObjectManager.h"

ReadWriteLock AccountManager::mutex;
AccountManager* AccountManager::singleInstance = nullptr;

AccountManager::AccountManager(LoginServer* loginserv) : Logger("AccountManager") {
	singleInstance = this;
	loginServer = loginserv;

	autoRegistration = true;
	requiredVersion = "";
	// Infinity:  Allow 3 characters online at a time up from 1
	maxOnlineCharacters = 3;

	setLogging(false);
	setGlobalLogging(false);

#ifndef WITH_SWGREALMS_API
	if (ServerCore::truncateDatabases()) {
		try {
			String query = "TRUNCATE TABLE characters";

			UniqueReference<ResultSet*> res(ServerDatabase::instance()->executeQuery(query));

			info("characters table truncated", true);
		} catch (const Exception& e) {
			error(e.getMessage());
		}
	}
#endif // !WITH_SWGREALMS_API
}

AccountManager::~AccountManager() {

}

bool AccountManager::isIPLockedOut(const String& ip) {
	Locker locker(&failedLoginMutex);

	std::string key = ip.toCharArray();
	auto it = failedLogins.find(key);

	if (it == failedLogins.end())
		return false;

	auto& record = it->second;

	// Reset if last attempt was more than 15 minutes ago
	uint64 nowMs = Time().getMiliTime();

	if (nowMs - record.lastAttemptMs > 15 * 60 * 1000) {
		failedLogins.erase(it);
		return false;
	}

	if (nowMs >= record.lockoutUntilMs)
		return false;

	return true;
}

void AccountManager::recordFailedLogin(const String& ip, const String& username, int adminLevel) {
	Locker locker(&failedLoginMutex);

	std::string key = ip.toCharArray();
	auto& record = failedLogins[key];

	record.count++;
	record.lastAttemptMs = Time().getMiliTime();

	bool isStaff = adminLevel > 0;

	// Staff accounts: stricter thresholds
	int lockoutSeconds = 0;

	if (isStaff) {
		if (record.count >= 8)
			lockoutSeconds = 3600;
		else if (record.count >= 5)
			lockoutSeconds = 600;
		else if (record.count >= 3)
			lockoutSeconds = 60;
	} else {
		if (record.count >= 15)
			lockoutSeconds = 1800;
		else if (record.count >= 10)
			lockoutSeconds = 300;
		else if (record.count >= 5)
			lockoutSeconds = 30;
	}

	if (lockoutSeconds > 0) {
		record.lockoutUntilMs = record.lastAttemptMs + (uint64)lockoutSeconds * 1000;

		error() << "Login: IP " << ip << " locked out for " << lockoutSeconds << "s after " << record.count << " failed attempts (username: " << username << ", adminLevel: " << adminLevel << ")";
	} else if (isStaff || record.count >= 3) {
		// Always log staff failures, log regular users at 3+
		error() << "Login: failed attempt #" << record.count << " from IP " << ip << " (username: " << username << ", adminLevel: " << adminLevel << ")";
	}
}

void AccountManager::clearFailedLogins(const String& ip) {
	Locker locker(&failedLoginMutex);

	std::string key = ip.toCharArray();
	failedLogins.erase(key);
}

void AccountManager::dumpFailedLogins() {
	Locker locker(&failedLoginMutex);

	if (failedLogins.empty()) {
		info(true) << "No failed login records.";
		return;
	}

	uint64 nowMs = Time().getMiliTime();

	info(true) << "Current failed login records (" << failedLogins.size() << "):";

	for (const auto& entry : failedLogins) {
		const auto& record = entry.second;
		int ageSec = (nowMs - record.lastAttemptMs) / 1000;

		StringBuffer line;
		line << "  " << entry.first.c_str() << " — " << record.count << " failures, last " << ageSec << "s ago";

		if (nowMs < record.lockoutUntilMs) {
			int remainSec = (record.lockoutUntilMs - nowMs) / 1000;
			line << ", LOCKED OUT (" << remainSec << "s remaining)";
		}

		info(true) << line.toString();
	}
}

void AccountManager::loginAccount(LoginClient* client, Message* packet) {
	String username, password, version;
	AccountVersionMessage::parse(packet, username, password, version);

	// Infinity: brute force protection — check IP lockout before processing
	String clientIP = client->getIPAddress();

	if (isIPLockedOut(clientIP)) {
		client->sendErrorMessage("Login Error", "Too many failed login attempts. Please try again later.");
		return;
	}

	// Infinity: VPN auto-block. Synchronous: allowlist + cache hit are sub-ms;
	// cache miss does a vpnapi.io call with a short timeout (config:
	// Core3.Login.VPNAutoBlock.APITimeoutMs). API failures fail-open.
	{
		VPNScreener::CheckResult vpnResult = VPNScreener::instance()->checkIP(clientIP, username);

		if (vpnResult.verdict == VPNScreener::BLOCK_VPN) {
			warning() << "Login: blocked VPN/proxy IP " << clientIP
				   << " (" << vpnResult.reason << ", cidr=" << vpnResult.cidr
				   << ", username=" << username << ")";
			client->sendErrorMessage("Login Error", "Connection blocked. Contact support if you believe this is in error.");
			return;
		}
	}

#ifndef WITH_SWGREALMS_API
	Database::escapeString(username);
	Database::escapeString(password);

	if (!isRequiredVersion(version)) {
		client->sendErrorMessage("Login Error", "The client you are attempting to connect with does not match that required by the server.");
		return;
	}

	Reference<Account*> account = validateAccountCredentials(client, username, password);

	if (account == nullptr)
		return;

#else // WITH_SWGREALMS_API
	StringBuffer clientEndpoint;

	auto session = client->getSession();

	if (session == nullptr)
		return;

	auto address = session->getAddress();

	clientEndpoint << address.getIPAddress() << ":" << address.getPort();

	SWGRealmsAPI::instance()->createSession(username, password, version, clientEndpoint.toString(),
			[this, username, loginClient = Reference<LoginClient*>(client)](const SessionApprovalResult& result) {

		if (result.isActionTemporaryFailure()) {
			error() << "Unexpected failure in createSession for user [" << username << "]: " << result.getLogMessage();
		}

		if (loginClient == nullptr)
			return;

		if (!result.isActionAllowed()) {
			auto clientSession = loginClient->getSession();
			if (clientSession != nullptr) {
				recordFailedLogin(loginClient->getIPAddress(), username);
			}
			loginClient->sendErrorMessage(result.getTitle(), result.getMessage(true));
			return;
		}

		auto sessionId = result.getSessionID();

		if (sessionId.isEmpty()) {
			StringBuffer errorMsg;
			errorMsg << "Your session key was invalid, exit the client and try logging in again, if this continues contact support.\n\ntrx_id: " << result.getTrxId();

			loginClient->sendErrorMessage("Login Error", errorMsg.toString());

			error() << "missing sessionId in createSession for user [" << username << "]: " << result.getLogMessage();

			return;
		}

		Reference<Account*> loginAccount = getAccount(result.getAccountID(), true);

		if (loginAccount == nullptr) {
			loginClient->sendErrorMessage("Login Error", "Failed to find your account, please contact support.");

			error() << "getAccount(" << result.getAccountID() << ") failed in createSession for user [" << username << "]: " << result.getLogMessage();
			return;
		}

		Locker locker(loginAccount);

		loginAccount->setSessionId(sessionId);
		loginAccount->setStationID(result.getStationID());

		if (!loginFinalize(loginClient, loginAccount)) {
			return;
		}

		loginApprovedAccount(loginClient, loginAccount);
	});
};

void AccountManager::loginApprovedAccount(LoginClient* client, ManagedReference<Account*> account) {
#endif // WITH_SWGREALMS_API
	// Infinity: clear failed login counter on successful login
	clearFailedLogins(client->getIPAddress());

	String sessionID = account->getSessionId();

	if (sessionID.isEmpty()) {
		sessionID = Crypto::randomSalt<64>();
		client->debug() << "generated sessionID: " << sessionID;
	} else {
		client->debug() << "logged in with sessionID: " << sessionID;
	}

	auto* lct = new LoginClientToken(account, sessionID);
	client->sendMessage(lct);

	uint32 accountID = account->getAccountID();

	client->setAccountID(accountID);

	String ip = client->getSession()->getAddress().getIPAddress();

#ifdef WITH_SWGREALMS_API
	SWGRealmsAPI::instance()->notifySessionStart(ip, accountID);
#endif // WITH_SWGREALMS_API

	// Write session to local DB when session-based login is enabled
	if (ConfigManager::instance()->getLoginEnableSessionId()) {
		int sessionDuration = ConfigManager::instance()->getInt("Core3.Login.SessionDuration", 365);
		StringBuffer sessionQuery;
		sessionQuery << "REPLACE INTO sessions (account_id, session_id, ip, expires) VALUES (";
		sessionQuery << accountID << ", '" << sessionID << "', '" << ip << "' , ADDDATE(NOW(), INTERVAL " << sessionDuration << " DAY));";

		try {
			ServerDatabase::instance()->executeStatement(sessionQuery);
		} catch (const DatabaseException& e) {
			client->error() << "Session write failed: " << e.getMessage();
		}
	}

	StringBuffer logQuery;
	logQuery << "INSERT INTO account_log (account_id, ip_address, timestamp) VALUES (" << accountID << ", '" << ip << "', NOW());";

	try {
		ServerDatabase::instance()->executeStatement(logQuery);
	} catch (const DatabaseException& e) {
		client->error() << e.getMessage();
	}

	client->sendMessage(loginServer->getLoginEnumClusterMessage(account));
	client->sendMessage(loginServer->getLoginClusterStatusMessage(account));

	auto eci = new EnumerateCharacterId(account);
	client->sendMessage(eci);
}

#ifndef WITH_SWGREALMS_API
Reference<Account*> AccountManager::validateAccountCredentials(LoginClient* client, const String& username, const String& password) {
	if (client == nullptr) {
		return nullptr;
	}

	bool isSessionIdLogin = false;
	String passwordStored;
	Reference<Account*> account = nullptr;

	if (ConfigManager::instance()->getLoginEnableSessionId()) {
		StringBuffer sessionIdQuery;
		sessionIdQuery << "SELECT a.active, a.username, a.password, a.salt, a.account_id, a.station_id, "
			     "UNIX_TIMESTAMP(a.created), a.admin_level, IFNULL(s.session_id, '') AS session_id "
			     "FROM accounts a, sessions s "
			     "WHERE s.account_id = a.account_id AND s.session_id = '" << password << "'";

		if (!username.isEmpty()) {
			sessionIdQuery << " AND a.username = '" << username << "'";
		}

		sessionIdQuery << "LIMIT 1;";

		try {

			account = getAccount(sessionIdQuery.toString(), passwordStored, true);

		} catch (const DatabaseException& e) {

			error(e.getMessage());
			if (client != nullptr) {
				error("ValidateAccountCredentials - IP = " + client->getIPAddress());
			}
		}

		if (account != nullptr) {
			isSessionIdLogin = true;
		}
	}

	if (account == nullptr) {
		StringBuffer query;
		query << "SELECT a.account_id, a.username, a.password, a.salt, a.account_id, a.station_id, "
			"UNIX_TIMESTAMP(a.created), a.admin_level, '' as session_id FROM accounts a WHERE a.username = '" << username << "' LIMIT 1;";

		account = getAccount(query.toString(), passwordStored, true);
	}

	if (account == nullptr) {
		// The user name didn't exist, so we check if auto registration is enabled and create a new account
		if (isAutoRegistrationEnabled()) {
			if (username.isEmpty()) {
				client->sendErrorMessage("Login Error", "You must choose a valid username!");
				return nullptr;
			}

			account = createAccount(username, password, passwordStored);
		} else {
			client->sendErrorMessage("Login Error",
				ConfigManager::instance()->getString("Core3.RegistrationMessage",
					"Automatic registration is currently disabled. "
					"Please contact the administrators of the server in order to get an authorized account."
				)
			);

			return nullptr;
		}
	}

	// Handle username / password login
	if (!isSessionIdLogin) {
		// Check hash version
		String passwordHashed;

		if (account->getSalt() == "") {
			passwordHashed = Crypto::SHA1Hash(password);
		} else {
			passwordHashed = Crypto::SHA256Hash(dbSecret + password + account->getSalt());
		}

		if (passwordStored != passwordHashed) {
			recordFailedLogin(client->getIPAddress(), username, account->getAdminLevel());
			client->sendErrorMessage("Wrong Password", "The password you entered was incorrect.");

			return nullptr;
		}

		// update hash if unsalted
		if (account->getSalt() == "")
			updateHash(username, password);
	}

	return loginFinalize(client, account) == true ? account : nullptr;
}
#endif // !WITH_SWGREALMS_API

bool AccountManager::loginFinalize(LoginClient* client, ManagedReference<Account*> account) {
	if (client == nullptr || account == nullptr) {
		return false;
	}

	Locker lock(account);

	if (!account->isActive()) {
		const String& inactTitle = ConfigManager::instance()->getInactiveAccountTitle();
		const String& inactText = ConfigManager::instance()->getInactiveAccountText();

		client->sendErrorMessage(
			inactTitle.length() == 0 ? "Account Disabled" : inactTitle,
			inactText.length() == 0 ? "The server administrators have disabled your account." : inactText
		);

		return false;
	}

	// Check if they are banned
	if (account->isBanned()) {
		StringBuffer reason;

		reason << "Your account has been banned from the server by the administrators.\n\n";
		int totalBan = account->getBanExpires() - time(0);

		int daysBanned = floor((float)totalBan / 60.f / 60.f / 24.f);
		totalBan -= (daysBanned * 60 * 60 * 24);

		int hoursBanned = floor(((float)totalBan / 60.0f) / 60.f);
		totalBan -= (hoursBanned * 60 * 60);

		int minutesBanned = floor((float)totalBan / 60.0f);
		totalBan -= (minutesBanned * 60);

		reason << "Time remaining: ";

		if (daysBanned > 0)
			reason << daysBanned << " Days ";

		if (hoursBanned > 0)
			reason << hoursBanned << " Hours ";

		if (minutesBanned > 0)
			reason << minutesBanned << " Minutes ";

		reason << totalBan << " Seconds\n";

		reason << "Reason: " << account->getBanReason();

		client->sendErrorMessage("Account Banned", reason.toString());

		return false;
	}

	return true;
}

#ifndef WITH_SWGREALMS_API
void AccountManager::updateHash(const String& username, const String& password) {
	String salt = Crypto::randomSalt();
	String hash = Crypto::SHA256Hash(dbSecret + password + salt);

	StringBuffer query;
	query << "UPDATE accounts SET password = '" << hash << "', ";
	query << "salt = '" << salt << "' ";
	query << "WHERE username = '" << username << "';";

	try {
		ServerDatabase::instance()->executeStatement(query);
	} catch (const DatabaseException& e) {
		error(e.getMessage());
	}
}

Reference<Account*> AccountManager::createAccount(const String& username, const String& password, String& passwordStored) {
	uint32 stationID = System::random();

	String salt = Crypto::randomSalt();
	String hash = Crypto::SHA256Hash(dbSecret + password + salt);

	StringBuffer query;
	query << "INSERT INTO accounts (username, password, station_id, salt) VALUES (";
	query << "'" << username << "',";
	query << "'" << hash << "',";
	query << stationID << ",";
	query << "'" << salt << "');";

	UniqueReference<ResultSet*> result(ServerDatabase::instance()->executeQuery(query.toString()));

	if (result == nullptr)
		return nullptr;

	uint32 accountID = result->getLastAffectedRow();

	return getAccount(accountID, passwordStored, true);
}

Reference<Account*> AccountManager::getAccount(uint32 accountID, bool forceSqlUpdate) {
	static Logger logger("AccountManager");

	Locker locker(&mutex);

	Reference<Account*> accObj;

	static uint64 databaseID = ObjectDatabaseManager::instance()->getDatabaseID("accounts");

	uint64 oid = (accountID | (databaseID << 48));

	accObj = Core::getObjectBroker()->lookUp(oid).castTo<Account*>();

	if (accObj == nullptr) {
		// Lazily create account object
		accObj = dynamic_cast<Account*>(ObjectManager::instance()->createObject("Account", 3, "accounts", oid));

		if (accObj == nullptr) {
			logger.error("Error creating account object with account ID " + String::hexvalueOf((int64)oid));

			return nullptr;
		}
	} else if (!forceSqlUpdate && accObj->isSqlLoaded() && !accObj->isAccountDataStale()) {
		return accObj;
	}

	StringBuffer query;
	query << "SELECT a.active, a.username, a.password, a.salt, a.account_id, a.station_id, UNIX_TIMESTAMP(a.created), a.admin_level, '' AS session_id FROM accounts a WHERE a.account_id = '" << accountID << "' LIMIT 1;";

	UniqueReference<ResultSet*> result(ServerDatabase::instance()->executeQuery(query.toString()));

	if (result->next()) {
		Locker locker(accObj);

		accObj->setActive(result->getBoolean(0));
		accObj->setUsername(result->getString(1));
		accObj->setSalt(result->getString(3));
		accObj->setAccountID(accountID);
		accObj->setStationID(result->getUnsignedInt(5));

		Time ttl;
		ttl.addMiliTime(3600 * 1000);
		accObj->setAccountDataValidUntil(ttl);

		if (!ConfigManager::instance()->getBool("Core3.AccountManager.CreatedDateFirstConnect", false)) {
			accObj->setTimeCreated(result->getUnsignedInt(6));
		} else {
			if (accObj->getTimeCreated() == 0) {
				Time now;
				accObj->setTimeCreated(now.getTime());
			}
		}

		accObj->setAdminLevel(result->getInt(7));

		accObj->updateFromDatabase();

		return accObj;
	}

	return nullptr;
}

Reference<Account*> AccountManager::getAccount(uint32 accountID, String& passwordStored, bool forceSqlUpdate) {
	StringBuffer query;
	query << "SELECT a.active, a.username, a.password, a.salt, a.account_id, a.station_id, UNIX_TIMESTAMP(a.created), a.admin_level, '' AS session_id FROM accounts a WHERE a.account_id = '" << accountID << "' LIMIT 1;";

	return getAccount(query.toString(), passwordStored, forceSqlUpdate);
}

Reference<Account*> AccountManager::getAccount(String query, String& passwordStored, bool forceSqlUpdate) {
	static Logger logger("AccountManager");

	Locker locker(&mutex);

	Reference<Account*> account;

	UniqueReference<ResultSet*> result(ServerDatabase::instance()->executeQuery(query));

	if (result->next()) {
		static uint64 databaseID = ObjectDatabaseManager::instance()->getDatabaseID("accounts");
		uint64 accountID = result->getUnsignedInt(4);

		uint64 oid = (accountID | (databaseID << 48));

		account = Core::getObjectBroker()->lookUp(oid).castTo<Account*>();

		if (account == nullptr) {
			// Lazily create account object
			account = dynamic_cast<Account*>(ObjectManager::instance()->createObject("Account", 3, "accounts", oid));

			if (account == nullptr) {
				logger.error("Error creating account object with account ID " + String::hexvalueOf((int64)oid));

				return nullptr;
			}
		} else if (!forceSqlUpdate && account->isSqlLoaded()) {
			return account;
		}

		if (account == nullptr) {
			return nullptr;
		}

		Locker locker(account);

		account->setActive(result->getBoolean(0));
		account->setUsername(result->getString(1));

		passwordStored = result->getString(2);

		account->setSalt(result->getString(3));
		account->setAccountID(accountID);
		account->setStationID(result->getUnsignedInt(5));

		if (!ConfigManager::instance()->getBool("Core3.AccountManager.CreatedDateFirstConnect", false)) {
			account->setTimeCreated(result->getUnsignedInt(6));
		} else {
			if (account->getTimeCreated() == 0) {
				Time now;
				account->setTimeCreated(now.getTime());
			}
		}

		account->setAdminLevel(result->getInt(7));

		account->setSessionId(result->getString(8));

		Time ttl;
		ttl.addMiliTime(3600 * 1000);
		account->setAccountDataValidUntil(ttl);

		account->updateFromDatabase();

		return account;
	}

	return nullptr;
}

Reference<Account*> AccountManager::getAccount(const String& accountName, bool forceSqlUpdate) {
	String name = accountName;

	Database::escapeString(name);

	StringBuffer query;
	query << "SELECT a.active, a.username, a.password, a.salt, a.account_id, a.station_id, UNIX_TIMESTAMP(a.created), a.admin_level, '' AS session_id FROM accounts a WHERE a.username = '" << name << "' LIMIT 1;";

	String temp;

	return getAccount(query.toString(), temp, forceSqlUpdate);
}
#else // WITH_SWGREALMS_API
Reference<Account*> AccountManager::getAccount(uint32 accountID, bool forceSqlUpdate) {
	static Logger logger("AccountManager");

	Reference<Account*> accObj;

	{
		// Scope mutext ot just ObjectBroker since API can process result on separate thread
		Locker locker(&mutex);

		static uint64 databaseID = ObjectDatabaseManager::instance()->getDatabaseID("accounts");

		uint64 oid = (accountID | (databaseID << 48));

		accObj = Core::getObjectBroker()->lookUp(oid).castTo<Account*>();

		if (accObj == nullptr) {
			// Lazily create account object
			accObj = dynamic_cast<Account*>(ObjectManager::instance()->createObject("Account", 3, "accounts", oid));

			if (accObj == nullptr) {
				logger.error("Error creating account object with account ID " + String::hexvalueOf((int64)oid));

				return nullptr;
			}
		} else if (!forceSqlUpdate && accObj->isSqlLoaded() && !accObj->isAccountDataStale()) {
			return accObj;
		}
	}

	// Try to get account data from API
	String errorMessage;
	auto swgRealmsAPI = SWGRealmsAPI::instance();

	if (swgRealmsAPI == nullptr) {
		logger.error() << "SWGRealms API instance is null";
		return nullptr;
	}

	if (swgRealmsAPI->getAccountDataBlocking(accountID, accObj, errorMessage)) {
		Locker locker(accObj);
		accObj->updateFromDatabase();

		return accObj;
	}

	logger.error() << "SWGRealms API getAccountDataBlocking failed for accountID " << accountID << ": " << errorMessage;
	return nullptr;
}

Reference<Account*> AccountManager::getAccount(const String& accountName, bool forceSqlUpdate) {
	static Logger logger("AccountManager");

	String errorMessage;
	auto swgRealmsAPI = SWGRealmsAPI::instance();

	// Get account_id from username via API
	uint32 accountID = swgRealmsAPI->getAccountID(accountName, errorMessage);

	if (accountID == 0) {
		logger.error() << "Failed to get account_id for username " << accountName << ": " << errorMessage;
		return nullptr;
	}

	// Use account_id to get full account (may use cache, avoiding second API call)
	return getAccount(accountID, forceSqlUpdate);
}
#endif // WITH_SWGREALMS_API

#ifndef WITH_SWGREALMS_API
void AccountManager::expireSession(Reference<Account*> account, const String& sessionID) {
	if (account == nullptr || sessionID.isEmpty()) {
		return;
	}

	StringBuffer delQuery;
	delQuery << "DELETE FROM sessions WHERE account_id = " << account->getAccountID();
	delQuery << " AND `expires` < NOW()";
	delQuery << ";";

	try {
		ServerDatabase::instance()->executeStatement(delQuery);
	} catch (const DatabaseException& e) {
		static Logger logger("AccountManager");
		logger.error() << e.getMessage();
	}
}
#endif // !WITH_SWGREALMS_API
