/*
 * ActionResult.h
 *
 * Base class and specialized result types for actions
 */

#ifndef ACTIONRESULT_H_
#define ACTIONRESULT_H_

#include "system/lang.h"

/**
 * Base result class for all actions
 * Stores success/error state and error details
 */
class ActionResult : public Object {
protected:
	String errorMessage;
	uint16 errorCode;

public:
	ActionResult() : errorCode(0) {}

	virtual ~ActionResult() {}

	bool isOK() const { return errorMessage.isEmpty(); }
	bool isError() const { return !errorMessage.isEmpty(); }

	const String& getError() const { return errorMessage; }
	uint16 getErrorCode() const { return errorCode; }

	void setError(const String& msg, uint16 code = 1) {
		errorMessage = msg;
		errorCode = code;
	}

	void setSuccess() {
		errorMessage = "";
		errorCode = 0;
	}
};

/**
 * Result for LoginAccountAction
 * Captures account authentication details
 */
class LoginAccountResult : public ActionResult {
	uint32 accountId;
	String sessionId;
	int numCharacters;

public:
	LoginAccountResult() : accountId(0), numCharacters(0) {}

	uint32 getAccountId() const { return accountId; }
	const String& getSessionId() const { return sessionId; }
	int getNumCharacters() const { return numCharacters; }

	void setAccountInfo(uint32 accId, const String& sessId, int charCount) {
		accountId = accId;
		sessionId = sessId;
		numCharacters = charCount;
	}
};

/**
 * Result for ConnectToZoneAction
 * Captures zone connection details
 */
class ConnectToZoneResult : public ActionResult {
	String galaxyAddress;
	uint32 galaxyPort;
	uint64 characterOid;

public:
	ConnectToZoneResult() : galaxyPort(0), characterOid(0) {}

	const String& getGalaxyAddress() const { return galaxyAddress; }
	uint32 getGalaxyPort() const { return galaxyPort; }
	uint64 getCharacterOid() const { return characterOid; }

	void setZoneInfo(const String& address, uint32 port, uint64 oid) {
		galaxyAddress = address;
		galaxyPort = port;
		characterOid = oid;
	}
};

/**
 * Result for CreateCharacterAction
 * Captures newly created character details
 */
class CreateCharacterResult : public ActionResult {
	uint64 createdOid;
	String characterName;

public:
	CreateCharacterResult() : createdOid(0) {}

	uint64 getCreatedOid() const { return createdOid; }
	const String& getCharacterName() const { return characterName; }

	void setCreatedCharacter(uint64 oid, const String& name) {
		createdOid = oid;
		characterName = name;
	}
};

#endif /* ACTIONRESULT_H_ */
