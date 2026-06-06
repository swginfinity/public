/*
 * ActionManager.h
 *
 * Registry and factory for actions using static self-registration
 */

#ifndef ACTIONMANAGER_H_
#define ACTIONMANAGER_H_

#include "system/lang.h"
#include "ActionBase.h"

/**
 * Action registry and factory
 *
 * Uses Meyers singleton pattern for thread-safe static initialization
 * Actions register themselves via static initializers at link time
 *
 * Example registration in action .cpp file:
 *   static bool _registered_myAction =
 *       (ActionManager::registerAction("myAction", MyAction::factory, MyAction::fromArgs, MyAction::fromJSON), true);
 */
class ActionManager {
public:
	// Function pointer types for cleaner signatures
	typedef ActionBase* (*FactoryFunc)();
	typedef Vector<ActionBase*> (*FromArgsFunc)(const Vector<String>&, int, int&);
	typedef ActionBase* (*FromJSONFunc)(const JSONSerializationType&);

	struct ActionRegistry {
		FactoryFunc factory;
		FromArgsFunc fromArgs;
		FromJSONFunc fromJSON;
	};

	/**
	 * Register an action with all its factory methods
	 *
	 * @param name Action name (e.g., "loginAccount")
	 * @param factory Function that creates empty action instance
	 * @param fromArgs Function that parses CLI args and creates action(s) with friends
	 * @param fromJSON Function that parses JSON config and creates single action
	 *
	 * Called by static initializers before main()
	 */
	static void registerAction(const char* name, FactoryFunc factory, FromArgsFunc fromArgs, FromJSONFunc fromJSON);

	/**
	 * Create an action instance by name (using basic factory)
	 *
	 * @param name Action name to create
	 * @return New action instance, or nullptr if not registered
	 *
	 * Caller is responsible for deleting the returned action
	 */
	static ActionBase* createAction(const char* name);

	/**
	 * Create action(s) from CLI arguments (may include friends)
	 *
	 * @param name Action type to try
	 * @param args CLI arguments
	 * @param startIndex Current position in args
	 * @param consumed OUTPUT: number of args consumed
	 * @return Vector of actions (me + friends), empty if not interested
	 */
	static Vector<ActionBase*> fromArgs(const char* name, const Vector<String>& args, int startIndex, int& consumed);

	/**
	 * Create action from JSON config (single action, no friends)
	 *
	 * @param name Action type to create
	 * @param config JSON configuration object
	 * @return Configured action instance, or nullptr on error
	 */
	static ActionBase* fromJSON(const char* name, const JSONSerializationType& config);

	/**
	 * List all registered action names
	 *
	 * @return Vector of action names
	 *
	 * Useful for introspection and help text generation
	 */
	static Vector<String> listActions();

	/**
	 * Check if an action is registered
	 *
	 * @param name Action name to check
	 * @return true if action is registered
	 */
	static bool hasAction(const char* name);

private:
	/**
	 * Meyers singleton for thread-safe registry initialization
	 * Registry is a map of action name -> registry entry (factory + parsers)
	 */
	static VectorMap<String, ActionRegistry>& getRegistry() {
		static VectorMap<String, ActionRegistry> registry;
		return registry;
	}
};

#endif /* ACTIONMANAGER_H_ */
