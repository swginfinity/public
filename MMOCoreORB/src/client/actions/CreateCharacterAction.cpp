/*
 * CreateCharacterAction.cpp
 *
 * Creates a new character on the zone server
 * Extracted from ZonePacketHandler for action framework
 */

#include "client/ActionBase.h"
#include "client/ActionResult.h"
#include "client/ActionManager.h"
#include "client/ClientCore.h"
#include "client/zone/Zone.h"
#include "server/zone/packets/charcreation/ClientCreateCharacter.h"
#include "client/zone/packets/ClientRandomNameRequestPacket.h"

class CreateCharacterAction : public ActionBase {
	// Configuration
	String characterName;
	String race;
	String profession;
	float height;
	String customization;
	String hairTemplate;
	String hairCustomization;
	String biography;
	bool skipTutorial;

	// Result
	CreateCharacterResult result;
	bool skipped;

public:
	CreateCharacterAction() :
		height(1.0f),
		skipTutorial(true),
		skipped(false) {
		setLogLevel(Logger::INFO);
		setLoggingName("CreateCharacter");
	}

	const char* getName() const override {
		return "createCharacter";
	}

	// ===== Static Factories =====

	static Vector<ActionBase*> fromArgs(const Vector<String>& args, int startIndex, int& consumed) {
		Vector<ActionBase*> result;
		consumed = 0;
		CreateCharacterAction* me = nullptr;

		int i = startIndex;
		while (i < args.size()) {
			int lastConsumed = consumed;

			if (args.get(i) == "--create-character") {
				if (me == nullptr) me = new CreateCharacterAction();
				consumed += 1;
				i += 1;
			}
			else if (args.get(i) == "--char-name" && i + 1 < args.size()) {
				if (me == nullptr) me = new CreateCharacterAction();
				me->characterName = args.get(i + 1);
				consumed += 2;
				i += 2;
			}
			else if (args.get(i) == "--char-race" && i + 1 < args.size()) {
				if (me == nullptr) me = new CreateCharacterAction();
				me->race = args.get(i + 1);
				consumed += 2;
				i += 2;
			}
			else if (args.get(i) == "--char-profession" && i + 1 < args.size()) {
				if (me == nullptr) me = new CreateCharacterAction();
				me->profession = args.get(i + 1);
				consumed += 2;
				i += 2;
			}
			else if (args.get(i) == "--char-height" && i + 1 < args.size()) {
				if (me == nullptr) me = new CreateCharacterAction();
				me->height = Float::valueOf(args.get(i + 1));
				consumed += 2;
				i += 2;
			}
			else if (args.get(i) == "--char-customization" && i + 1 < args.size()) {
				if (me == nullptr) me = new CreateCharacterAction();
				me->customization = args.get(i + 1);
				consumed += 2;
				i += 2;
			}
			else if (args.get(i) == "--char-hair-template" && i + 1 < args.size()) {
				if (me == nullptr) me = new CreateCharacterAction();
				me->hairTemplate = args.get(i + 1);
				consumed += 2;
				i += 2;
			}
			else if (args.get(i) == "--char-hair-customization" && i + 1 < args.size()) {
				if (me == nullptr) me = new CreateCharacterAction();
				me->hairCustomization = args.get(i + 1);
				consumed += 2;
				i += 2;
			}
			else if (args.get(i) == "--char-biography" && i + 1 < args.size()) {
				if (me == nullptr) me = new CreateCharacterAction();
				me->biography = args.get(i + 1);
				consumed += 2;
				i += 2;
			}

			if (consumed == lastConsumed) break;
		}

		if (me != nullptr) {
			result.add(me);
			// No friends - createCharacter is self-contained
		}

		return result;
	}

	static ActionBase* fromJSON(const JSONSerializationType& config) {
		CreateCharacterAction* me = new CreateCharacterAction();

		if (config.contains("name")) {
			me->characterName = String(config["name"].get<std::string>().c_str());
		}
		if (config.contains("race")) {
			me->race = String(config["race"].get<std::string>().c_str());
		}
		if (config.contains("profession")) {
			me->profession = String(config["profession"].get<std::string>().c_str());
		}
		if (config.contains("height")) {
			me->height = config["height"];
		}
		if (config.contains("customization")) {
			me->customization = String(config["customization"].get<std::string>().c_str());
		}
		if (config.contains("hairTemplate")) {
			me->hairTemplate = String(config["hairTemplate"].get<std::string>().c_str());
		}
		if (config.contains("hairCustomization")) {
			me->hairCustomization = String(config["hairCustomization"].get<std::string>().c_str());
		}
		if (config.contains("biography")) {
			me->biography = String(config["biography"].get<std::string>().c_str());
		}
		if (config.contains("skipTutorial")) {
			me->skipTutorial = config["skipTutorial"];
		}

		return me;
	}

	bool needsZone() const override {
		return true;  // Zone-phase action
	}

	bool needsTarget() const override {
		return true;  // Needs targetGalaxyId to know where to create
	}

	void run(ClientCore& core) override {
		// Warn if targetCharacterOid is set (we're creating new, not using existing)
		if (core.targetCharacterOid != 0) {
			warning() << "Ignoring selected character OID " << core.targetCharacterOid
			          << " (createCharacter creates new character)";
		}
		// Validate prerequisites
		if (core.zone == nullptr || !core.zone->isConnected()) {
			result.setError("No active zone connection", 1);
			return;
		}

		// Flow: connectToZone → createCharacter → server creates → SelectCharacter → scene loads

		// Use configured values or defaults
		String charName = characterName;
		String charRace = race;
		String charProfession = profession;
		float charHeight = height;
		String charCustomization = customization;
		String charHairTemplate = hairTemplate;
		String charHairCustomization = hairCustomization;
		String charBiography = biography;

		// Default race if not provided (needed for name request)
		if (charRace.isEmpty()) {
			charRace = "object/creature/player/human_male.iff";
		}

		// Request random name from server if not provided
		if (charName.isEmpty()) {
			info() << "No character name specified, requesting random name from server...";

			BaseMessage* randomNameReq = new ClientRandomNameRequestPacket(charRace);
			core.zone->getZoneClient()->sendMessage(randomNameReq);

			if (!core.zone->waitFor(STRING_HASHCODE("ClientRandomNameResponse"), 5000)) {
				result.setError("Timeout waiting for random name from server", 5);
				return;
			}

			String defaultName("");
			charName = core.getVar("ClientRandomNameResponse/name", defaultName);

			if (charName.isEmpty()) {
				result.setError("Server did not provide a valid name", 6);
				return;
			}

			info() << "Server suggested name: " << charName;
		}

		// Default profession if not provided
		if (charProfession.isEmpty()) {
			charProfession = "crafting_artisan";
		}

		// Clamp height
		if (charHeight < 0.7f || charHeight > 1.5f) {
			warning("Height " + String::valueOf(charHeight) + " out of range, clamping to 1.0");
			charHeight = 1.0f;
		}

		info() << "Creating character:";
		info() << "  Name: " << charName;
		info() << "  Race: " << charRace;
		info() << "  Profession: " << charProfession;
		info() << "  Height: " << charHeight;
		info() << "  Skip Tutorial: " << skipTutorial;

		// Build and send character creation packet
		UnicodeString unicodeName(charName.toCharArray());
		UnicodeString unicodeBio(charBiography.toCharArray());

		BaseMessage* createPacket = new ClientCreateCharacter(
			unicodeName,
			charRace,
			charHeight,
			charCustomization,
			charHairTemplate,
			charHairCustomization,
			charProfession,
			unicodeBio,
			!skipTutorial  // Note: inverted
		);

		core.zone->getZoneClient()->sendMessage(createPacket);

		// Wait for one of three possible responses
		uint32 responses[] = {
			STRING_HASHCODE("ClientCreateCharacterSuccess"),
			STRING_HASHCODE("ClientCreateCharacterFailed"),
			STRING_HASHCODE("ErrorMessage")
		};

		if (!core.zone->waitForAny(responses, 3, 30000)) {
			result.setError("Character creation timeout", 5);
			return;
		}

		// Check which response we got
		if (core.hasVar("ClientCreateCharacterSuccess/oid")) {
			// Success!
			uint64 createdOid = core.getVar<uint64>("ClientCreateCharacterSuccess/oid", 0);
			result.setSuccess();
			result.setCreatedCharacter(createdOid, charName);

			// Set selected character OID so scene handler knows what character we're becoming
			core.selectedCharacterOid = createdOid;

			// Store for other actions
			core.setVar("createdOid", createdOid);
			core.setVar("createdCharacterName", charName);

			info() << "Character created successfully - OID: " << createdOid;
			return;
		}

		if (core.hasVar("ClientCreateCharacterFailed/errorCode")) {
			String defaultError("unknown");
			String errorCode = core.getVar("ClientCreateCharacterFailed/errorCode", defaultError);
			result.setError("Character creation failed: " + errorCode, 3);
			return;
		}

		if (!core.zone->getLastError().isEmpty()) {
			result.setError("Character creation error: " + core.zone->getLastError(), 4);
			return;
		}

		// Shouldn't reach here
		result.setError("Character creation - unknown response", 7);
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

		// Include configuration (for template generation)
		if (!characterName.isEmpty()) {
			json["name"] = characterName.toCharArray();
		}
		if (!race.isEmpty()) {
			json["race"] = race.toCharArray();
		}
		if (!profession.isEmpty()) {
			json["profession"] = profession.toCharArray();
		}
		if (height != 1.0f) {
			json["height"] = height;
		}
		if (!customization.isEmpty()) {
			json["customization"] = customization.toCharArray();
		}
		if (!hairTemplate.isEmpty()) {
			json["hairTemplate"] = hairTemplate.toCharArray();
		}
		if (!hairCustomization.isEmpty()) {
			json["hairCustomization"] = hairCustomization.toCharArray();
		}
		if (!biography.isEmpty()) {
			json["biography"] = biography.toCharArray();
		}
		if (!skipTutorial) {
			json["skipTutorial"] = skipTutorial;
		}

		// Status and results
		if (skipped) {
			json["status"] = "skipped";
		} else if (result.isOK()) {
			json["status"] = "ok";
			json["createdOid"] = result.getCreatedOid();
			json["characterName"] = result.getCharacterName().toCharArray();
		} else {
			json["status"] = "failed";
			json["error"] = result.getError().toCharArray();
			json["errorCode"] = result.getErrorCode();
		}

		return json;
	}

	String getHelpText() const override {
		return "--create-character  Create new character (uses --char-name, --char-race, --char-profession, etc.)";
	}

	// Factory function for static registration
	static ActionBase* factory() {
		return new CreateCharacterAction();
	}
};

// Static registration (runs before main())
static bool _registered_createCharacter =
	(ActionManager::registerAction("createCharacter", CreateCharacterAction::factory, CreateCharacterAction::fromArgs, CreateCharacterAction::fromJSON), true);
