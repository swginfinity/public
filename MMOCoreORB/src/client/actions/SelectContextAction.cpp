/*
 * SelectContextAction.cpp
 *
 * Selects the target context (character + galaxy) for subsequent operations
 * Stores selection in core.targetCharacterOid and core.targetGalaxyId
 */

#include "client/ActionBase.h"
#include "client/ActionResult.h"
#include "client/ActionManager.h"
#include "client/ClientCore.h"
#include "client/login/LoginSession.h"

class SelectContextAction : public ActionBase {
	// Configuration
	uint32 galaxyId;
	String galaxyName;
	uint64 characterOid;
	String characterFirstname;

	// Result
	ActionResult result;
	bool skipped;

public:
	SelectContextAction() :
		galaxyId(0),
		characterOid(0),
		skipped(false) {
		setLogLevel(Logger::INFO);
		setLoggingName("SelectContext");
	}

	const char* getName() const override {
		return "selectContext";
	}

	// ===== Static Factories =====

	static Vector<ActionBase*> fromArgs(const Vector<String>& args, int startIndex, int& consumed) {
		Vector<ActionBase*> result;
		consumed = 0;
		SelectContextAction* me = nullptr;

		int i = startIndex;
		while (i < args.size()) {
			int lastConsumed = consumed;

			if (args.get(i) == "--galaxy" && i + 1 < args.size()) {
				if (me == nullptr) me = new SelectContextAction();
				String val = args.get(i + 1);
				// Try parsing as number first, otherwise treat as name
				try {
					me->galaxyId = UnsignedInteger::valueOf(val);
				} catch (...) {
					me->galaxyName = val;
				}
				consumed += 2;
				i += 2;
			}
			else if (args.get(i) == "--character-oid" && i + 1 < args.size()) {
				if (me == nullptr) me = new SelectContextAction();
				me->characterOid = UnsignedLong::valueOf(args.get(i + 1));
				consumed += 2;
				i += 2;
			}
			else if (args.get(i) == "--character-firstname" && i + 1 < args.size()) {
				if (me == nullptr) me = new SelectContextAction();
				me->characterFirstname = args.get(i + 1);
				consumed += 2;
				i += 2;
			}

			if (consumed == lastConsumed) break;
		}

		if (me != nullptr) {
			result.add(me);

			// CLI CONVENIENCE: Character selection implies wanting to zone in
			if (!me->characterFirstname.isEmpty() || me->characterOid != 0) {
				ActionBase* zoneIn = ActionManager::createAction("zoneInCharacter");
				if (zoneIn != nullptr) {
					result.add(zoneIn);
				}
			}
		}

		return result;
	}

	static ActionBase* fromJSON(const JSONSerializationType& config) {
		SelectContextAction* me = new SelectContextAction();

		if (config.contains("galaxyId")) {
			me->galaxyId = config["galaxyId"].get<uint32>();
		}
		if (config.contains("galaxyName")) {
			me->galaxyName = String(config["galaxyName"].get<std::string>().c_str());
		}
		if (config.contains("characterOid")) {
			me->characterOid = config["characterOid"].get<uint64>();
		}
		if (config.contains("characterFirstname")) {
			me->characterFirstname = String(config["characterFirstname"].get<std::string>().c_str());
		}

		return me;  // JSON is explicit - no friends
	}

	bool needsZone() const override {
		return false;  // Login-phase metadata selection
	}

	void run(ClientCore& core) override {
		if (core.loginSession == nullptr || !core.loginSession->isLoggedIn()) {
			result.setError("No active login session", 1);
			return;
		}

		// Resolve galaxy
		uint32 resolvedGalaxyId = 0;

		if (galaxyId != 0) {
			// Explicit galaxy ID
			resolvedGalaxyId = galaxyId;
		} else if (!galaxyName.isEmpty()) {
			// Galaxy by name
			auto& galaxies = core.loginSession->getGalaxies();
			bool found = false;
			for (int i = 0; i < galaxies.size(); i++) {
				if (galaxies.get(i).getName() == galaxyName) {
					resolvedGalaxyId = galaxies.get(i).getID();
					found = true;
					break;
				}
			}
			if (!found) {
				result.setError("Galaxy name not found: " + galaxyName, 2);
				return;
			}
		}

		// Resolve character
		uint64 resolvedCharacterOid = 0;

		if (characterOid != 0) {
			// Explicit character OID
			auto character = core.loginSession->selectCharacterByOID(characterOid);
			if (!character) {
				result.setError("Character OID not found", 3);
				return;
			}
			resolvedCharacterOid = character->getObjectID();
			uint32 charGalaxyId = character->getGalaxyID();

			// Validate galaxy matches if both specified
			if (resolvedGalaxyId != 0 && resolvedGalaxyId != charGalaxyId) {
				result.setError("Character is on galaxy " + String::valueOf(charGalaxyId) +
				                " but you specified galaxy " + String::valueOf(resolvedGalaxyId), 4);
				return;
			}

			resolvedGalaxyId = charGalaxyId;
			info() << "Selected character: " << character->getFirstName() << " (OID: " << resolvedCharacterOid << ")";

		} else if (!characterFirstname.isEmpty()) {
			// Character by firstname
			auto character = core.loginSession->selectCharacterByFirstname(characterFirstname);
			if (!character) {
				result.setError("Character firstname not found: " + characterFirstname, 5);
				return;
			}
			resolvedCharacterOid = character->getObjectID();
			uint32 charGalaxyId = character->getGalaxyID();

			// Validate galaxy matches if both specified
			if (resolvedGalaxyId != 0 && resolvedGalaxyId != charGalaxyId) {
				result.setError("Character '" + characterFirstname + "' is on galaxy " + String::valueOf(charGalaxyId) +
				                " but you specified galaxy " + String::valueOf(resolvedGalaxyId), 6);
				return;
			}

			resolvedGalaxyId = charGalaxyId;
			info() << "Selected character: " << characterFirstname << " (OID: " << resolvedCharacterOid << ")";

		} else {
			// Default: pick first/random character (if any exist)
			auto numCharacters = core.loginSession->getCharacterListSize();
			if (numCharacters > 0) {
				auto character = core.loginSession->selectRandomCharacter();
				if (character) {
					resolvedCharacterOid = character->getObjectID();
					uint32 charGalaxyId = character->getGalaxyID();

					// If galaxy not specified, use character's galaxy
					if (resolvedGalaxyId == 0) {
						resolvedGalaxyId = charGalaxyId;
					}

					info() << "Auto-selected character: " << character->getFirstName() << " (OID: " << resolvedCharacterOid << ")";
				}
			}
		}

		// If still no galaxy, pick first available
		if (resolvedGalaxyId == 0) {
			auto& galaxies = core.loginSession->getGalaxies();
			if (galaxies.size() > 0) {
				resolvedGalaxyId = galaxies.get(0).getID();
				info() << "Auto-selected galaxy: " << galaxies.get(0).getName() << " (ID: " << resolvedGalaxyId << ")";
			} else {
				result.setError("No galaxies available", 7);
				return;
			}
		}

		// Store resolved context (merge with any previous selectContext calls)
		if (resolvedGalaxyId != 0) {
			if (core.targetGalaxyId != 0 && core.targetGalaxyId != resolvedGalaxyId) {
				warning() << "Changing target galaxy from " << core.targetGalaxyId << " to " << resolvedGalaxyId;
			}
			core.targetGalaxyId = resolvedGalaxyId;
			galaxyId = resolvedGalaxyId;  // Store in action for toJSON()
		}

		if (resolvedCharacterOid != 0) {
			if (core.targetCharacterOid != 0 && core.targetCharacterOid != resolvedCharacterOid) {
				warning() << "Changing target character from " << core.targetCharacterOid << " to " << resolvedCharacterOid;
			}
			core.targetCharacterOid = resolvedCharacterOid;
			characterOid = resolvedCharacterOid;  // Store in action for toJSON()
		}

		result.setSuccess();
		info() << "Context selected - Galaxy: " << core.targetGalaxyId << ", Character: " << core.targetCharacterOid;
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
		if (galaxyId != 0) {
			json["galaxyId"] = galaxyId;
		}
		if (!galaxyName.isEmpty()) {
			json["galaxyName"] = galaxyName.toCharArray();
		}
		if (characterOid != 0) {
			json["characterOid"] = characterOid;
		}
		if (!characterFirstname.isEmpty()) {
			json["characterFirstname"] = characterFirstname.toCharArray();
		}

		// Status
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
		return new SelectContextAction();
	}
};

// Static registration (runs before main())
static bool _registered_selectContext =
	(ActionManager::registerAction("selectContext", SelectContextAction::factory, SelectContextAction::fromArgs, SelectContextAction::fromJSON), true);
