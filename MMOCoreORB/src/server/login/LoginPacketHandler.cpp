/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/


#include "objects.h"

#include "LoginPacketHandler.h"
#include "server/login/LoginServer.h"
#include "LoginProcessServerImplementation.h"

#include "account/AccountManager.h"

//Infinity:  Custom includes
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/player/PlayerObject.h"

LoginPacketHandler::LoginPacketHandler(const String& s, LoginProcessServerImplementation* serv)
		: Logger(s) {

	processServer = serv;

	server = processServer->getLoginServer().get();

	setGlobalLogging(true);
	setLogging(false);
}

void LoginPacketHandler::handleMessage(Message* pack) {
	Reference<LoginClient*> client = server->getLoginClient(pack->getClient());

	if (client == nullptr)
		return;

	debug() << "parsing " << *pack;

	uint16 opcount = pack->parseShort();
	uint32 opcode = pack->parseInt();

	switch (opcount) {
	case 1:
		break;
	case 2:
		break;
	case 3:
		switch (opcode) {
			case 0xE87AD031:
			handleDeleteCharacterMessage(client, pack);
			break;
		}
		break;
	case 4:
		switch (opcode) {
		case 0x41131F96: //LoginClientID CLIENT VERSION BUILD DATE AND LOGIN INFO
			handleLoginClientID(client, pack);
			break;
		}
		break;
	case 5:
		break;
	default:
		break;
	}
}

void LoginPacketHandler::handleLoginClientID(LoginClient* client, Message* pack) {
	AccountManager* accountManager = server->getAccountManager();
	accountManager->loginAccount(client, pack);
}

void LoginPacketHandler::handleDeleteCharacterMessage(LoginClient* client, Message* pack) {
	if (!client->hasAccount()) {
		auto* msg = new DeleteCharacterReplyMessage(1); //FAIL
		client->sendMessage(msg);
		return;
	}

	uint32 accountId = client->getAccountID();

	uint32 ServerId = pack->parseInt();

	//pack->shiftOffset(4);
	uint64 charId = pack->parseLong();

	//Infinity;  Custom check on owned structures to not allow deletion of characters that own structures
	auto creature = Core::getObjectBroker()->lookUp(charId).castTo<CreatureObject*>();

	if (creature != nullptr) {
		auto ghost = creature->getPlayerObject();
		if (ghost != nullptr) {
			if (ghost->getTotalOwnedStructureCount() > 0) {
				auto errMsg = new ErrorMessage("Delete Error", "You cannot delete a character that owns structures.   Please delete, re-deed, or transfer structures before deleting.", 0x0);
				client->sendMessage(errMsg);
				return;
			}
		}
	}

	int dbDelete = 0;
	// Infinity fix #1528: a freshly-created character lives in `characters_dirty`
	// until ObjectManager::onCommitData promotes it to `characters`. If the
	// player deletes within that window, the SELECT below matches 0 rows and
	// the normal move+delete path silently no-ops, leaving the char orphaned
	// in `characters_dirty` — it later promotes to `characters` as a zombie.
	// We fall back to deleting directly from `characters_dirty` in that case.
	bool handledViaDirty = false;

#ifndef WITH_SWGREALMS_API
	StringBuffer moveStatement;
	moveStatement << "INSERT INTO deleted_characters SELECT *, 0 as db_deleted FROM characters WHERE character_oid = " << charId;
	moveStatement << " AND account_id = " << accountId << " AND galaxy_id = " << ServerId << ";";

	StringBuffer verifyStatement;
	verifyStatement << "SELECT * from deleted_characters WHERE character_oid = " << charId;
	verifyStatement << " AND account_id = " << accountId << " AND galaxy_id = " << ServerId << ";";

	StringBuffer deleteStatement;
	deleteStatement << "DELETE FROM characters WHERE character_oid = " << charId;
	deleteStatement << " AND account_id = " << accountId << " AND galaxy_id = " << ServerId << ";";

	StringBuffer dirtyDeleteStatement;
	dirtyDeleteStatement << "DELETE FROM characters_dirty WHERE character_oid = " << charId;
	dirtyDeleteStatement << " AND account_id = " << accountId << " AND galaxy_id = " << ServerId << ";";

	try {
		UniqueReference<ResultSet*> moveResults(ServerDatabase::instance()->executeQuery(moveStatement.toString()));

		if (moveResults == nullptr || moveResults.get()->getRowsAffected() == 0) {
			// Char not in `characters` yet — try the dirty fallback before
			// declaring failure. If the char is genuinely unknown, both
			// tables return 0 rows and we fall back to the original error.
			try {
				UniqueReference<ResultSet*> dirtyResults(ServerDatabase::instance()->executeQuery(dirtyDeleteStatement.toString()));

				if (dirtyResults != nullptr && dirtyResults.get()->getRowsAffected() > 0) {
					handledViaDirty = true;
				}
			} catch (const Exception& e) {
				error() << e.getMessage();
			}

			if (!handledViaDirty) {
				// Idempotency check: a prior delete attempt may have already
				// moved the character (e.g. client reconnect after a desync
				// between INSERT-success and the SUCCESS reply). On retry,
				// INSERT...SELECT FROM characters reports 0 rows because the
				// row is genuinely gone. Run verify before declaring failure
				// — if the row is already in deleted_characters, the move is
				// semantically complete, so treat it as handled rather than
				// emitting a false-positive "Could not move" error.
				UniqueReference<ResultSet*> verifyResults(ServerDatabase::instance()->executeQuery(verifyStatement.toString()));

				if (verifyResults != nullptr && verifyResults.get()->size() > 0) {
					handledViaDirty = true;
				} else {
					dbDelete = 1;

					error() << "Could not move character to deleted_characters table. " << endl <<
						"QUERY: " << moveStatement;
				}
			}
		}

		if (!handledViaDirty && !dbDelete) {
			UniqueReference<ResultSet*> verifyResults(ServerDatabase::instance()->executeQuery(verifyStatement.toString()));

			if (verifyResults == nullptr || verifyResults.get()->size() == 0) {
				dbDelete = 1;

				error() << "Could not verify character was moved to deleted_characters " << endl <<
					"QUERY: " << moveStatement;
			}
		}

	} catch (const Exception& e) {
		dbDelete = 1;

		error() << e.getMessage();
	}

	if (!dbDelete && !handledViaDirty) {
		try {
			UniqueReference<ResultSet*> deleteResults(ServerDatabase::instance()->executeQuery(deleteStatement));

			if (deleteResults == nullptr || deleteResults.get()->getRowsAffected() == 0) {
				error() << "Unable to delete character from character table. " << endl
					<< "QUERY: " << deleteStatement;

				dbDelete = 1;
			}
		} catch (const Exception& e) {
			error() << e.getMessage();
			dbDelete = 1;
		}
	}
#else // WITH_SWGREALMS_API
	auto swgRealmsAPI = SWGRealmsAPI::instance();
	String errorMessage;

	if (swgRealmsAPI == nullptr || !swgRealmsAPI->deleteCharacterBlocking(charId, accountId, ServerId, errorMessage)) {
		error() << "Failed to delete character: " << errorMessage;
		dbDelete = 1;
	}
#endif // WITH_SWGREALMS_API

	auto* msg = new DeleteCharacterReplyMessage(dbDelete);
	client->sendMessage(msg);
}


