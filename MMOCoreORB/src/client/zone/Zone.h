/*
 				Copyright <SWGEmu>
		See file COPYING for copying conditions. */

#ifndef ZONE_H_
#define ZONE_H_

#include "ZoneClient.h"
#include "client/zone/ZoneClientThread.h"
#include "engine/util/JSONSerializationType.h"

class ObjectController;
class ObjectManager;
class SceneObject;

class Zone : public Thread, public Mutex, public Logger {
	uint32 accountID;
	String sessionID;
	String galaxyAddress;
	uint32 galaxyPort;

	Reference<ZoneClient*> client;
	ZoneClientThread* clientThread;

	class ClientCore* clientCore;  // Reference to core context for vars storage
	ObjectController* objectController;

	Condition sceneReadyCondition;

	ObjectManager* objectManager;

	Time startTime;
	bool started;
	bool sceneReady;

	// Client permissions from server
	bool canLogin;
	bool canCreateRegularCharacter;
	bool canCreateJediCharacter;
	bool canSkipTutorial;

	String lastError;
	uint16 lastErrorCode;

	// Generic async response handling
	VectorMap<uint32, Condition*> waitConditions;

public:
	Zone(class ClientCore* core, uint32 account, const String& sessionID, const String& galaxyAddress, uint32 galaxyPort);
	~Zone();

	void run();

	void disconnect() {
		if (client != nullptr) {
			client->disconnect();
			clientThread = nullptr;
		}
	}

	void setSceneStarted() {
		info(true) << __FUNCTION__ << " in " << startTime.miliDifference() << "ms";
	}

	void setSceneReady() {
		Locker locker(this);

		info(true) << __FUNCTION__ << " in " << startTime.miliDifference() << "ms";
		sceneReady = true;

		sceneReadyCondition.signal(this);
	}

	bool waitForSceneReady(int timeoutMs) {
		Locker locker(this);

		if (sceneReady) {
			return true;
		}

		Time timeout;
		timeout.addMiliTime(timeoutMs);
		bool success = (sceneReadyCondition.timedWait(this, &timeout) == 0);

		return success && sceneReady;
	}

	// ===== Generic Wait/Signal Mechanism =====

	/**
	 * Wait for a specific packet response (generic)
	 *
	 * @param opcode Packet opcode to wait for (use STRING_HASHCODE)
	 * @param timeoutMs Timeout in milliseconds
	 * @return true if signal received, false if timeout
	 */
	bool waitFor(uint32 opcode, int timeoutMs) {
		Locker locker(this);

		if (!waitConditions.contains(opcode)) {
			waitConditions.put(opcode, new Condition());
		}

		Condition* cond = waitConditions.get(opcode);

		Time timeout;
		timeout.addMiliTime(timeoutMs);
		return (cond->timedWait(this, &timeout) == 0);
	}

	/**
	 * Signal that a specific packet was received
	 *
	 * @param opcode Packet opcode (use STRING_HASHCODE)
	 */
	void signal(uint32 opcode) {
		Locker locker(this);

		if (waitConditions.contains(opcode)) {
			waitConditions.get(opcode)->signal(this);
		}
	}

	/**
	 * Wait for ANY of multiple possible packet responses
	 *
	 * @param opcodes Array of packet opcodes to wait for
	 * @param count Number of opcodes in array
	 * @param timeoutMs Timeout in milliseconds
	 * @return true if any signal received, false if timeout
	 *
	 * Use for scenarios where multiple responses are possible (success/failed/error).
	 * After wait returns true, check ClientCore::vars to determine which response arrived.
	 */
	bool waitForAny(uint32 opcodes[], int count, int timeoutMs) {
		Locker locker(this);

		// Create ONE shared condition for all opcodes
		Condition* sharedCond = new Condition();

		// Register under all opcodes
		for (int i = 0; i < count; i++) {
			waitConditions.put(opcodes[i], sharedCond);
		}

		// Wait (releases lock during wait, re-acquires on return)
		Time timeout;
		timeout.addMiliTime(timeoutMs);
		bool success = (sharedCond->timedWait(this, &timeout) == 0);

		// Cleanup: Remove all opcodes and free condition
		for (int i = 0; i < count; i++) {
			waitConditions.drop(opcodes[i]);
		}
		delete sharedCond;

		return success;
	}

	SceneObject* getObject(uint64 objid);

	inline const String& getGalaxyAddress() {
		return galaxyAddress;
	}

	inline uint32 getGalaxyPort() {
		return galaxyPort;
	}

	inline ZoneClient* getZoneClient() {
		return client;
	}

	inline ClientCore* getClientCore() {
		return clientCore;
	}

	inline ObjectManager* getObjectManager() {
		return objectManager;
	}

	inline ObjectController* getObjectController() {
		return objectController;
	}

	bool isStarted() {
		return started;
	}

	bool isSceneReady() {
		return sceneReady;
	}

	void setPermissions(bool login, bool createRegular, bool createJedi, bool skipTutorial) {
		canLogin = login;
		canCreateRegularCharacter = createRegular;
		canCreateJediCharacter = createJedi;
		canSkipTutorial = skipTutorial;
	}

	bool getCanLogin() const {
		return canLogin;
	}

	bool getCanCreateCharacter() const {
		return canCreateRegularCharacter;
	}

	bool getCanCreateJedi() const {
		return canCreateJediCharacter;
	}

	bool getCanSkipTutorial() const {
		return canSkipTutorial;
	}

	bool isConnected() const {
		return client != nullptr && client->getClient() != nullptr && client->getClient()->isAvailable();
	}

	const String& getLastError() const {
		return lastError;
	}

	uint16 getLastErrorCode() const {
		return lastErrorCode;
	}

	void setError(const String& msg, uint16 code) {
		lastError = msg;
		lastErrorCode = code;
	}

	void clearError() {
		lastError = "";
		lastErrorCode = 0;
	}

	JSONSerializationType collectStats();
};

#endif /* ZONE_H_ */
