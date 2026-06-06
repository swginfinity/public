/*
 				Copyright <SWGEmu>
		See file COPYING for copying conditions. */

/**
 * file InvisibleDelayEvent.h
 * author Polonel
 * date 10.01.2010
 */

#ifndef INVISIBLEDELAYEVENT_H_
#define INVISIBLEDELAYEVENT_H_

#include "engine/engine.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "templates/creature/PlayerCreatureTemplate.h"
#include "server/zone/Zone.h"

class InvisibleDelayEvent: public Task {
	ManagedReference<CreatureObject*> player;

public:
	InvisibleDelayEvent(CreatureObject* pl) {
		player = pl;
	}

	void run() {
		Locker playerLocker(player);

		PlayerObject* targetGhost = player->getPlayerObject();

		try {
			if (player->isOnline() && !targetGhost->isLoggingOut()) {
				player->removePendingTask("invisibledelayevent");

				ManagedReference<Zone*> zone = player->getZone();

				if (zone == nullptr)
					return;

				PlayerCreatureTemplate* playerTemplate = dynamic_cast<PlayerCreatureTemplate*>(player->getObjectTemplate());

				if (playerTemplate == nullptr)
					return;

				ManagedReference<ImageDesignSession*> session = player->getActiveSession(SessionFacadeType::IMAGEDESIGN).castTo<ImageDesignSession*>();

				if (session != nullptr) {
					session->sessionTimeout();
				}

				float height = player->getHeight();

				bool goingInvisible = !player->isInvisible();

				if (goingInvisible) {
					if (playerTemplate->getMinScale() <= height) {
						player->setHeight(height * 0.25f);
					}

					player->sendSystemMessage("You are now invisible to other players and creatures.");
				} else {
					if (playerTemplate->getMinScale() > height) {
						player->setHeight(height * 4.0f);
					}

					player->sendSystemMessage("You are now visible to all players and creatures.");
				}

				// switchZone toggles invisible flag and re-inserts player
				player->switchZone(zone->getZoneName(), player->getPositionX(), player->getPositionZ(), player->getPositionY(), player->getParentID(), true);

				// Send destroy to all nearby clients so cached objects are removed
				// (handles headless/hacked clients that don't respect isInvisible)
				if (goingInvisible) {
					auto closeObjects = player->getCloseObjects();

					if (closeObjects != nullptr) {
						SortedVector<TreeEntry*> entries;
						closeObjects->safeCopyReceiversTo(entries, CloseObjectsVector::PLAYERTYPE);

						for (int i = 0; i < entries.size(); i++) {
							SceneObject* obj = static_cast<SceneObject*>(entries.get(i));

							if (obj == nullptr || obj == player.get())
								continue;

							player->sendDestroyTo(obj);
						}
					}
				}
			}

		} catch (Exception& e) {
			player->error("unreported exception caught in InvisibleDelayEvent::run");
		}

	}

};

#endif /* INVISIBLEDELAYEVENT_H_ */
