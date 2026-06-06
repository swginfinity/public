/*
 * ClientRandomNameRequestPacket.h
 *
 * Client-side packet to request random name from server
 */

#ifndef CLIENTRANDOMNAMEREQUESTPACKET_H_
#define CLIENTRANDOMNAMEREQUESTPACKET_H_

#include "engine/service/proto/BaseMessage.h"

class ClientRandomNameRequestPacket : public BaseMessage {
public:
	ClientRandomNameRequestPacket(const String& raceTemplate) : BaseMessage() {
		insertShort(0x02);
		insertInt(STRING_HASHCODE("ClientRandomNameRequest"));  // 0xD6D1B6D1
		insertAscii(raceTemplate);

		setCompression(false);
	}
};

#endif /* CLIENTRANDOMNAMEREQUESTPACKET_H_ */
