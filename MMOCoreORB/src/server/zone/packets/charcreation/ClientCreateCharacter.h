/*
*			Copyright <SWGEmu>
*	See file COPYING for copying conditions.
*/

#ifndef CLIENTCREATECHARACTER_H_
#define CLIENTCREATECHARACTER_H_

#include "engine/service/proto/BaseMessage.h"

class ClientCreateCharacter : public BaseMessage {
public:
	// Full constructor matching Core3 server parsing
	ClientCreateCharacter(
		const UnicodeString& characterName,
		const String& templateName,
		float scaleFactor,
		const String& customAppearanceData,
		const String& hairTemplateName,
		const String& hairAppearanceData,
		const String& profession,
		const UnicodeString& biography,
		bool useNewbieTutorial
	) {
		insertShort(12);
		insertInt(STRING_HASHCODE("ClientCreateCharacter")); // 0xB97F3074

		insertAscii(customAppearanceData);    // Customization
		insertUnicode(characterName);         // Name
		insertAscii(templateName);            // Race template
		insertAscii("");                      // Starting location (ignored by server)
		insertAscii(hairTemplateName);        // Hair object
		insertAscii(hairAppearanceData);      // Hair customization
		insertAscii(profession);              // Profession
		insertByte(0x00);                     // Unknown byte
		insertFloat(scaleFactor);             // Height
		insertUnicode(biography);             // Biography
		insertByte(useNewbieTutorial ? 0x01 : 0x00);  // Tutorial flag
	}

	// Simple constructor for backward compatibility
	ClientCreateCharacter(const UnicodeString& name)
		: ClientCreateCharacter(
			name,
			"object/creature/player/human_male.iff",
			1.0f,
			"",
			"",
			"",
			"crafting_artisan",
			UnicodeString(""),
			false  // skip tutorial
		) {}
};

#endif /*CLIENTCREATECHARACTER_H_*/
