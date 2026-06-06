/*
 * LoginSession.h
 *
 *  Created on: Sep 6, 2009
 *      Author: theanswer
 */

#ifndef LOGINSESSION_H_
#define LOGINSESSION_H_

#include "system/lang.h"
#include "engine/log/Logger.h"
#include "system/util/Optional.h"
#include "engine/util/JSONSerializationType.h"
#include "server/login/objects/GalaxyList.h"
#include "server/login/objects/CharacterListEntry.h"

class LoginClient;

class LoginSession : public Mutex, public Runnable, public Logger, public Object {
	Condition sessionFinalized;

	uint32 accountID;
	String sessionID;
	String username;
	String password;

	Vector<CharacterListEntry> characters;

	class LoginClientThread* loginThread;

	Reference<LoginClient*> login;

	VectorMap<uint32, Galaxy> galaxies;

	Time loginStartTime;

	String lastError;
	uint16 lastErrorCode;

	// Generic async response handling
	VectorMap<uint32, Condition*> waitConditions;

public:
	LoginSession(const String& username, const String& password);

	~LoginSession();

	void run();

	void addCharacter(const CharacterListEntry& entry) {
		characters.add(entry);
	}

	void signalCompletion() {
		lock();
		sessionFinalized.signal(this);
		unlock();
	}

	void setAccountID(uint32 id) {
		accountID = id;
	}

	void setSessionID(const String& sessionID) {
		this->sessionID = sessionID;
	}

	uint32 getAccountID() {
		return accountID;
	}

	const String& getSessionID() {
		return sessionID;
	}

	const CharacterListEntry& getCharacterByOID(uint32 id) {
		return characters.get(id);
	}

	const CharacterListEntry& getCharacterByIndex(int index) {
		return characters.get(index);
	}

	uint32 getCharacterListSize() {
		return characters.size();
	}

	Optional<const CharacterListEntry&> selectCharacterByOID(uint64 oid) {
		for (int i = 0; i < characters.size(); ++i) {
			if (characters.get(i).getObjectID() == oid) {
				return Optional<const CharacterListEntry&>(characters.get(i));
			}
		}
		return Optional<const CharacterListEntry&>();
	}

	Optional<const CharacterListEntry&> selectCharacterByFirstname(const String& firstname) {
		for (int i = 0; i < characters.size(); ++i) {
			if (characters.get(i).getFirstName() == firstname) {
				return Optional<const CharacterListEntry&>(characters.get(i));
			}
		}
		return Optional<const CharacterListEntry&>();
	}

	Optional<const CharacterListEntry&> selectRandomCharacter() {
		if (characters.size() == 0) {
			return Optional<const CharacterListEntry&>();
		}
		uint32 selectedIndex = System::random(characters.size() - 1);
		return Optional<const CharacterListEntry&>(characters.get(selectedIndex));
	}

	Galaxy& getGalaxy(uint32 galaxyId) {
		if (!galaxies.contains(galaxyId)) {
			galaxies.put(galaxyId, Galaxy(galaxyId));
		}

		return galaxies.get(galaxyId);
	}

	const VectorMap<uint32, Galaxy>& getGalaxies() const {
		return galaxies;
	}

	bool isConnected() const {
		return loginThread != nullptr && login != nullptr;
	}

	bool isLoggedIn() const {
		return accountID != 0;
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

	// ===== Generic Wait/Signal Mechanism =====

	bool waitFor(uint32 opcode, int timeoutMs) {
		lock();

		if (!waitConditions.contains(opcode)) {
			waitConditions.put(opcode, new Condition());
		}

		Condition* cond = waitConditions.get(opcode);

		Time timeout;
		timeout.addMiliTime(timeoutMs);
		bool success = (cond->timedWait(this, &timeout) == 0);

		unlock();
		return success;
	}

	void signal(uint32 opcode) {
		lock();

		if (waitConditions.contains(opcode)) {
			waitConditions.get(opcode)->signal(this);
		}

		unlock();
	}

	bool waitForAny(uint32 opcodes[], int count, int timeoutMs) {
		lock();

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

		unlock();
		return success;
	}

	void cleanup();

	JSONSerializationType collectStats();

	void sendMessage(BaseMessage* msg);
};


#endif /* LOGINSESSION_H_ */
