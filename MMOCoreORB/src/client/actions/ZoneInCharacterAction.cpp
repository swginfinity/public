/*
 * ZoneInCharacterAction.cpp
 *
 * Zones in as the target character (sends SelectCharacter packet, waits for scene)
 * Uses core.targetCharacterOid from selectContext
 * Requires active zone connection
 */

#include "client/ActionBase.h"
#include "client/ActionResult.h"
#include "client/ActionManager.h"
#include "client/ClientCore.h"
#include "client/zone/Zone.h"
#include "server/zone/packets/zone/SelectCharacter.h"

class ZoneInCharacterAction : public ActionBase {
	// Result
	ActionResult result;
	bool skipped;

public:
	ZoneInCharacterAction() :
		skipped(false) {
		setLogLevel(Logger::INFO);
		setLoggingName("ZoneInCharacter");
	}

	const char* getName() const override {
		return "zoneInCharacter";
	}

	bool needsZone() const override {
		return true;  // Requires zone connection
	}

	// ===== Static Factories =====

	static Vector<ActionBase*> fromArgs(const Vector<String>& args, int startIndex, int& consumed) {
		Vector<ActionBase*> result;
		consumed = 0;
		// ZoneInCharacter has no CLI args - always auto-inserted or friend-injected
		return result;
	}

	static ActionBase* fromJSON(const JSONSerializationType& config) {
		// ZoneInCharacter has no configuration - uses core.targetCharacterOid
		return new ZoneInCharacterAction();
	}

	bool needsTarget() const override {
		return true;  // Requires targetCharacterOid from selectContext
	}

	void run(ClientCore& core) override {
		// Validate prerequisites
		if (core.zone == nullptr || !core.zone->isConnected()) {
			result.setError("No zone connection", 1);
			return;
		}

		if (core.targetCharacterOid == 0) {
			result.setError("No target character selected (need selectContext first)", 2);
			return;
		}

		// Use target from selectContext
		uint64 oid = core.targetCharacterOid;
		core.selectedCharacterOid = oid;  // Track which character we're playing as

		// Send SelectCharacter packet
		info() << "Zoning in as character OID: " << oid;
		BaseMessage* selectMsg = new SelectCharacter(oid);
		core.zone->getZoneClient()->sendMessage(selectMsg);

		// Wait for scene to be ready
		int zoneTimeout = ClientCore::getZoneTimeout() * 1000;
		if (!core.zone->waitForSceneReady(zoneTimeout)) {
			result.setError("Zone scene timeout", 3);
			return;
		}

		// Success
		result.setSuccess();
		info() << "Zone-in complete - scene ready";
	}

	bool isOK() const override {
		return !skipped && result.isOK();
	}

	String getError() const override {
		return result.getError();
	}

	uint16 getErrorCode() const override {
		return result.getErrorCode();
	}

	void setSkipped() override {
		skipped = true;
	}

	JSONSerializationType toJSON() const override {
		JSONSerializationType json;
		json["action"] = getName();

		if (skipped) {
			json["status"] = "skipped";
		} else if (result.isOK()) {
			json["status"] = "ok";
		} else {
			json["status"] = "failed";
			json["error"] = result.getError().toCharArray();
			json["errorCode"] = result.getErrorCode();
		}

		return json;
	}

	String getHelpText() const override {
		return "";  // Auto-inserted, no user-facing options
	}

	// Factory function for static registration
	static ActionBase* factory() {
		return new ZoneInCharacterAction();
	}
};

// Static registration (runs before main())
static bool _registered_zoneInCharacter =
	(ActionManager::registerAction("zoneInCharacter", ZoneInCharacterAction::factory, ZoneInCharacterAction::fromArgs, ZoneInCharacterAction::fromJSON), true);
