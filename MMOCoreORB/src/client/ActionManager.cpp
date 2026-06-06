/*
 * ActionManager.cpp
 *
 * Implementation of action registry and factory
 */

#include "ActionManager.h"

void ActionManager::registerAction(const char* name, FactoryFunc factory, FromArgsFunc fromArgs, FromJSONFunc fromJSON) {
	ActionRegistry entry;
	entry.factory = factory;
	entry.fromArgs = fromArgs;
	entry.fromJSON = fromJSON;
	getRegistry().put(String(name), entry);
}

ActionBase* ActionManager::createAction(const char* name) {
	auto& registry = getRegistry();
	String actionName(name);

	if (!registry.contains(actionName)) {
		return nullptr;
	}

	// Call basic factory function to create new instance
	return registry.get(actionName).factory();
}

Vector<ActionBase*> ActionManager::fromArgs(const char* name, const Vector<String>& args, int startIndex, int& consumed) {
	auto& registry = getRegistry();
	String actionName(name);

	consumed = 0;

	if (!registry.contains(actionName)) {
		return Vector<ActionBase*>();
	}

	auto& entry = registry.get(actionName);

	if (entry.fromArgs == nullptr) {
		return Vector<ActionBase*>();
	}

	// Call static fromArgs factory
	return entry.fromArgs(args, startIndex, consumed);
}

ActionBase* ActionManager::fromJSON(const char* name, const JSONSerializationType& config) {
	auto& registry = getRegistry();
	String actionName(name);

	if (!registry.contains(actionName)) {
		return nullptr;
	}

	auto& entry = registry.get(actionName);

	if (entry.fromJSON == nullptr) {
		return nullptr;
	}

	// Call static fromJSON factory
	return entry.fromJSON(config);
}

Vector<String> ActionManager::listActions() {
	Vector<String> names;
	auto& registry = getRegistry();

	for (int i = 0; i < registry.size(); i++) {
		names.add(registry.elementAt(i).getKey());
	}

	return names;
}

bool ActionManager::hasAction(const char* name) {
	auto& registry = getRegistry();
	return registry.contains(String(name));
}
