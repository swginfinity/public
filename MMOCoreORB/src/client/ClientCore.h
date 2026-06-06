/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef CLIENTCORE_H_
#define CLIENTCORE_H_

#include "system/lang.h"
#include "server/login/objects/GalaxyList.h"

class Zone;
class ActionBase;

struct ClientCoreOptions {
	// Single source of truth
	JSONSerializationType config;
	Vector<ActionBase*> actions;

	// Constructor does ALL parsing
	ClientCoreOptions() = default;
	ClientCoreOptions(int argc, char** argv);

	// Generic accessor with defaults
	template<typename T>
	T get(const char* path, T defaultVal = T()) const {
		try {
			return config.at(JSONSerializationType::json_pointer(path)).get<T>();
		} catch (...) {
			return defaultVal;
		}
	}

	// Setter
	void set(const char* path, const JSONSerializationType& val) {
		config[JSONSerializationType::json_pointer(path)] = val;
	}

	// Utility methods
	JSONSerializationType getAsJSON() const;
	String toString() const;
	String toStringData() const;

private:
	void loadEnvFile(const String& filename);
	int parseLogLevel(const String& levelStr);
	String resolveFileReference(const String& value);
	void parseArgumentsIntoActions(const Vector<String>& args);
	void parseJSONIntoActions(const JSONSerializationType& jsonActions);
	void resolveDependencies();
};

class ClientCore : public Core, public Logger {
public:
	ClientCoreOptions options;
	Reference<class LoginSession*> loginSession;
	Zone* zone;
	JSONSerializationType vars;  // Unified storage for dynamic data (async responses, user vars, etc.)
	uint64 selectedCharacterOid;  // Which character we're playing as (set by zoneInCharacter, confirmed by server)
	uint64 targetCharacterOid;    // Target character for operations (set by selectContext)
	uint32 targetGalaxyId;        // Target galaxy (set by selectContext)

private:
	Time overallStartTime;
	Optional<Galaxy> selectedGalaxy;

public:
	static ClientCore* getCoreInstance() {
		Core* instance = Core::getCoreInstance();
		return (instance != nullptr) ? static_cast<ClientCore*>(instance) : nullptr;
	}

	static int getLogLevel() {
		ClientCore* clientCore = getCoreInstance();
		if (clientCore != nullptr) {
			int level = clientCore->options.get<int>("/logLevel", -1);
			if (level != -1) return level;
		}
		return Core::getIntProperty("Client3.LogLevel", Logger::INFO);
	}

	static String getLoginHost() {
		ClientCore* clientCore = getCoreInstance();
		if (clientCore != nullptr) {
			std::string host = clientCore->options.get<std::string>("/loginHost", "");
			if (!host.empty()) return String(host.c_str());
		}
		return Core::getProperty("Client3.LoginHost", "127.0.0.1");
	}

	static int getLoginPort() {
		ClientCore* clientCore = getCoreInstance();
		if (clientCore != nullptr) {
			int port = clientCore->options.get<int>("/loginPort", 0);
			if (port != 0) return port;
		}
		return Core::getIntProperty("Client3.LoginPort", 44453);
	}

	static String getClientVersion() {
		ClientCore* clientCore = getCoreInstance();
		if (clientCore != nullptr) {
			std::string ver = clientCore->options.get<std::string>("/clientVersion", "");
			if (!ver.empty()) return String(ver.c_str());
		}
		return Core::getProperty("Client3.ClientVersion", "20050408-18:00");
	}

	static int getLoginTimeout() {
		ClientCore* clientCore = getCoreInstance();
		if (clientCore != nullptr) {
			int timeout = clientCore->options.get<int>("/loginTimeout", 0);
			if (timeout != 0) return timeout;
		}
		return Core::getIntProperty("Client3.LoginTimeout", 10);
	}

	static int getZoneTimeout() {
		ClientCore* clientCore = getCoreInstance();
		if (clientCore != nullptr) {
			int timeout = clientCore->options.get<int>("/zoneTimeout", 0);
			if (timeout != 0) return timeout;
		}
		return Core::getIntProperty("Client3.ZoneTimeout", 30);
	}

public:
	ClientCore(const ClientCoreOptions& opts);

	void initialize();

	void run();

	void executeActions();

	// ===== Variable Storage (JSON-backed) =====

	template<typename T>
	void setVar(const String& path, T value) {
		String jsonPath = "/" + path;
		vars[JSONSerializationType::json_pointer(jsonPath.toCharArray())] = value;
	}

	// String specialization (convert to std::string for JSON)
	void setVar(const String& path, const String& value) {
		String jsonPath = "/" + path;
		vars[JSONSerializationType::json_pointer(jsonPath.toCharArray())] = value.toCharArray();
	}

	template<typename T>
	T getVar(const String& path, T defaultVal = T()) const {
		try {
			String jsonPath = "/" + path;
			return vars.at(JSONSerializationType::json_pointer(jsonPath.toCharArray())).get<T>();
		} catch (...) {
			return defaultVal;
		}
	}

	// String specialization (JSON uses std::string)
	String getVar(const String& path, const String& defaultVal) const {
		try {
			String jsonPath = "/" + path;
			std::string val = vars.at(JSONSerializationType::json_pointer(jsonPath.toCharArray())).get<std::string>();
			return String(val.c_str());
		} catch (...) {
			return defaultVal;
		}
	}

	bool hasVar(const String& path) const {
		try {
			String jsonPath = "/" + path;
			vars.at(JSONSerializationType::json_pointer(jsonPath.toCharArray()));
			return true;
		} catch (...) {
			return false;
		}
	}

	String substituteVars(const String& input) const {
		String result = input;

		// Iterate over JSON object keys
		if (vars.is_object()) {
			for (auto it = vars.begin(); it != vars.end(); ++it) {
				String key(it.key().c_str());
				String value;

				// Convert value to string for substitution
				if (it.value().is_string()) {
					value = String(it.value().get<std::string>().c_str());
				} else {
					value = String(it.value().dump().c_str());
				}

				String pattern = "{" + key + "}";
				result = result.replaceAll(pattern, value);
			}
		}

		return result;
	}

private:
	void saveStateToFile(const String& filename, class LoginSession* loginSession);
};

#endif /*CLIENTCORE_H_*/
