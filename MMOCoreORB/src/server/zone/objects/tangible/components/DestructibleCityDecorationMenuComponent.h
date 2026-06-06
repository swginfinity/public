/*
 * DestructibleCityDecorationMenuComponent.h
 *
 *  Created on: July 5, 2022
 *      Author: wickedHangover
 * 
 * // Infinity:  Custom
 */

#ifndef DESTRUCTIBLECITYDECORATIONMENUCOMPONENT_H_
#define DESTRUCTIBLECITYDECORATIONMENUCOMPONENT_H_

#include "TangibleObjectMenuComponent.h"

class DestructibleCityDecorationMenuComponent : public TangibleObjectMenuComponent {
public:

	/**
	 * Fills the radial options, needs to be overriden
	 * @pre { this object is locked }
	 * @post { this object is locked, menuResponse is complete}
	 * @param menuResponse ObjectMenuResponse that will be sent to the client
	 */
	virtual void fillObjectMenuResponse(SceneObject* sceneObject, ObjectMenuResponse* menuResponse, CreatureObject* player) const;

	/**
	 * Handles the radial selection sent by the client, must be overriden by inherited objects
	 * @pre { this object is locked, player is locked }
	 * @post { this object is locked, player is locked }
	 * @param player PlayerCreature that selected the option
	 * @param selectedID selected menu id
	 * @returns 0 if successfull
	 */
	virtual int handleObjectMenuSelect(SceneObject* sceneObject, CreatureObject* player, byte selectedID) const;

	bool isInInventory(SceneObject* sceneObject, CreatureObject* player) const;
};






#endif /* DESTRUCTIBLECITYDECORATIONMENUCOMPONENT_H_ */
