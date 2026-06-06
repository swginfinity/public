/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef BADGESRESPONSEMESSAGE_H_
#define BADGESRESPONSEMESSAGE_H_

#include "engine/service/proto/BaseMessage.h"
#include "server/zone/objects/creature/CreatureObject.h"
#include "server/zone/objects/player/badges/Badges.h"

class BadgesResponseMessage : public BaseMessage {
public:
    BadgesResponseMessage(SceneObject* player, Badges* badges) : BaseMessage() {
		insertShort(0x03);
		insertInt(0x6D89D25B);  // CRC
		insertLong(player != nullptr ? player->getObjectID() : 0); //Player ID

		//TODO: Ensure that number of bitmasks doesn't exceed the highest idx settable in the badgeMap.
		//insertInt(0x05); //Number of bitmasks to send

		insertInt(0x0F); // from docs seems like if its not 0x0F badges dont show in the client

		setCompression(true);

		//TODO: Loop through a vector of bitmasks.
		// Infinity:  increase the number of bitmasks added to the code from 5 to 10
		// you loop through the number of bitmasks added (as seen below 10 is the temporary magic number)
		// you will also need to edit MMOCoreORB/src/server/zone/objects/player/badges/Badges.h
		for (int i = 0; i < 10; ++i) {
			insertInt(badges != nullptr ? badges->getBitmask(i) : 0);
		}

		insertInt(0);
		insertInt(0);
		insertInt(0);
		insertInt(0);
		insertInt(0);
		insertInt(0);
		insertInt(0);
		insertInt(0);
		insertInt(0);
		insertInt(0);
		insertInt(0);
	}

};

#endif /*BADGESRESPONSEMESSAGE_H_*/
