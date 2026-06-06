/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef ZONEPACKETHANDLER_H_
#define ZONEPACKETHANDLER_H_

#include "engine/log/Logger.h"

class Zone;

class ZonePacketHandler : public Mutex, public Logger {
	Zone* zone;
	class ClientCore* core;
	VectorMap<uint32, uint32> unknownOpcodes;  // opcode -> count

public:
	ZonePacketHandler(const String& s, Zone* z, class ClientCore* clientCore);

	~ZonePacketHandler() {
	}

	void handleMessage(Message* pack);
	void handleSceneObjectCreateMessage(Message* pack);
	void handleCmdStartScene(Message* pack);
	void handleBaselineMessage(Message* pack);
	void handleChatInstantMessageToClient(Message* pack);
	void handleChatSystemMessage(Message* pack);
	void handleObjectControllerMessage(Message* pack);
	void handleUpdateContainmentMessage(Message* pack);
	void handleClientPermissionsMessage(Message* pack);
	void handleCmdSceneReady(Message* pack);
	void handleClientCreateCharacterSuccess(Message* pack);
	void handleClientCreateCharacterFailed(Message* pack);
	void handleClientRandomNameResponse(Message* pack);
	void handleErrorMessage(Message* pack);

	const VectorMap<uint32, uint32>& getUnknownOpcodes() const {
		return unknownOpcodes;
	}
};

#endif /* ZONEPACKETHANDLER_H_ */
