/*
 * ActionBase.h
 *
 * Pure virtual interface for all client actions
 * Actions self-register at link time using static initializers
 */

#ifndef ACTIONBASE_H_
#define ACTIONBASE_H_

#include "system/lang.h"
#include "engine/log/Logger.h"
#include "engine/util/JSONSerializationType.h"

// Forward declaration
class ClientCore;

/**
 * Base interface for all client actions
 *
 * Actions are self-contained operations that:
 * - Parse their own arguments (CLI and JSON)
 * - Declare their requirements (needs zone connection?)
 * - Execute a single operation
 * - Report results (success, error, data)
 * - Self-register at link time via static initializer
 *
 * Design pattern: Similar to Google Test (TEST macro) registration
 */
class ActionBase : public Object, public Logger {
public:
	ActionBase() : Logger("Action") {}
	virtual ~ActionBase() {}

	// ===== Identity =====

	/**
	 * Get the action name (e.g., "loginAccount", "createCharacter")
	 * Used for registry lookup and JSON output
	 */
	virtual const char* getName() const = 0;

	// ===== Static Factory Methods =====
	// NOTE: These are implemented as static methods in each concrete action class
	// They cannot be declared virtual here (C++ limitation) but follow this contract:

	/**
	 * Create action(s) from CLI arguments (SMART - may add friends)
	 *
	 * Static factory that:
	 * - Parses CLI args starting at given index
	 * - Creates configured action instance(s)
	 * - May add "friend" actions based on semantic intent
	 * - Returns vector of actions to add to chain
	 *
	 * @param args Vector of argument strings
	 * @param startIndex Current position in args vector
	 * @param consumed OUTPUT: Number of arguments consumed (0 = not interested)
	 * @return Vector of actions (empty if not interested, 1+ if matched)
	 *
	 * Example implementation:
	 *   static Vector<ActionBase*> fromArgs(const Vector<String>& args, int startIndex, int& consumed) {
	 *       Vector<ActionBase*> result;
	 *       consumed = 0;
	 *       MyAction* me = nullptr;
	 *
	 *       int i = startIndex;
	 *       while (i < args.size()) {
	 *           int lastConsumed = consumed;
	 *
	 *           if (args.get(i) == "--my-flag" && i + 1 < args.size()) {
	 *               if (me == nullptr) me = new MyAction();
	 *               me->property = args.get(i + 1);
	 *               consumed += 2;
	 *               i += 2;
	 *           }
	 *
	 *           if (consumed == lastConsumed) break;
	 *       }
	 *
	 *       if (me != nullptr) {
	 *           result.add(me);
	 *           // Add friends based on configuration
	 *           if (me->needsFriend) result.add(new FriendAction());
	 *       }
	 *
	 *       return result;
	 *   }
	 *
	 * CLI Convenience: May return multiple actions (me + friends)
	 * Example: --character-firstname returns [selectContext, zoneInCharacter]
	 */
	// static Vector<ActionBase*> fromArgs(const Vector<String>& args, int startIndex, int& consumed);

	/**
	 * Create action from JSON configuration (EXPLICIT - no friends)
	 *
	 * Static factory that:
	 * - Parses JSON configuration object
	 * - Creates single configured action instance
	 * - NEVER adds friend actions (JSON must be explicit)
	 * - Returns configured action or nullptr on error
	 *
	 * @param config JSON object for this action
	 * @return Configured action instance (caller owns memory)
	 *
	 * Example implementation:
	 *   static ActionBase* fromJSON(const JSONSerializationType& config) {
	 *       MyAction* me = new MyAction();
	 *
	 *       if (config.contains("property")) {
	 *           me->property = config["property"].get<Type>();
	 *       }
	 *
	 *       return me;
	 *   }
	 *
	 * JSON Precision: Always returns exactly 1 action (or nullptr)
	 * Users must specify complete action chains explicitly in JSON
	 */
	// static ActionBase* fromJSON(const JSONSerializationType& config);

	// ===== Requirements =====

	/**
	 * Does this action require an active zone connection?
	 *
	 * @return true if action needs zone (triggers auto-insert of connectToZone)
	 *
	 * Examples:
	 *   - loginAccount: false (login-phase action)
	 *   - deleteCharacter: false (login-phase action)
	 *   - createCharacter: true (zone-phase action)
	 *   - sendCommand: true (zone-phase action)
	 */
	virtual bool needsZone() const = 0;

	/**
	 * Does this action require a character to be selected?
	 *
	 * @return true if action needs character selected (triggers auto-insert of selectCharacter)
	 *
	 */
	virtual bool needsCharacter() const { return false; }  // Default: not required

	/**
	 * Does this action require target context (character/galaxy selection)?
	 *
	 * @return true if action needs targetCharacterOid/targetGalaxyId (triggers auto-insert of selectContext)
	 */
	virtual bool needsTarget() const { return false; }  // Default: not required

	// ===== Execution =====

	/**
	 * Execute the action
	 *
	 * @param core ClientCore reference for accessing sessions, options, etc.
	 *
	 * Actions should:
	 * - Validate prerequisites (check loginSession, zone, etc.)
	 * - Perform their operation
	 * - Store results in internal result object
	 * - NOT throw exceptions (catch and store in result)
	 */
	virtual void run(ClientCore& core) = 0;

	// ===== Results =====

	/**
	 * Check if action succeeded
	 * @return true if action completed successfully
	 */
	virtual bool isOK() const = 0;

	/**
	 * Get error message (if action failed)
	 * @return Error description (empty if isOK())
	 */
	virtual String getError() const = 0;

	/**
	 * Get error code (if action failed)
	 * @return Error code (0 if isOK())
	 */
	virtual uint16 getErrorCode() const = 0;

	/**
	 * Serialize action result to JSON
	 *
	 * @return JSON object with action result
	 *
	 * Format:
	 *   {
	 *     "action": "actionName",
	 *     "status": "ok" | "failed" | "skipped",
	 *     "error": "...",        // if failed
	 *     "errorCode": 123,      // if failed
	 *     ... action-specific data ...
	 *   }
	 */
	virtual JSONSerializationType toJSON() const = 0;

	// ===== State Management =====

	/**
	 * Mark this action as skipped (never ran due to earlier failure)
	 * When an action fails, all subsequent actions are marked skipped
	 */
	virtual void setSkipped() = 0;

	// ===== Documentation =====

	/**
	 * Get help text for --help output
	 * @return Human-readable description of this action's CLI usage
	 *
	 * Example:
	 *   "--delete-character <oid>  Delete character with specified OID"
	 */
	virtual String getHelpText() const = 0;
};

#endif /* ACTIONBASE_H_ */
