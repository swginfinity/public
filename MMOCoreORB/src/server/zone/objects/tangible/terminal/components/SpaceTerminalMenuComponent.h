/*
 * SpaceTerminalMenuComponent.h
 *
 *  Created on: Jan 30, 2026
 */

#ifndef SPACETERMINALMENUCOMPONENT_H_
#define SPACETERMINALMENUCOMPONENT_H_

#include "server/zone/objects/scene/components/ObjectMenuComponent.h"

class SpaceTerminalMenuComponent : public ObjectMenuComponent {
public:
	virtual void fillObjectMenuResponse(SceneObject* sceneObject, ObjectMenuResponse* menuResponse, CreatureObject* player) const;
	virtual int handleObjectMenuSelect(SceneObject* sceneObject, CreatureObject* player, byte selectedID) const;
};

#endif /* SPACETERMINALMENUCOMPONENT_H_ */
