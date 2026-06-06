/*
 * GetArticleMessageCallback.h
 *
 * No-op stub for GetArticleMessage (opcode 0x5E7B4846) — the in-game
 * Knowledge Base / help-article request. SWGEmu never implemented the
 * customerService message family and Infinity has no KB content, so this
 * stub exists only so ZonePacketHandler stops logging it as an unknown
 * opcode. If a real Knowledge Base is ever wanted, this is where the
 * GetArticleResponseMessage reply would be built.
 */

#ifndef GETARTICLEMESSAGECALLBACK_H_
#define GETARTICLEMESSAGECALLBACK_H_

#include "server/zone/packets/MessageCallback.h"

class GetArticleMessageCallback : public MessageCallback {
public:
	GetArticleMessageCallback(ZoneClientSession* client, ZoneProcessServer* server) : MessageCallback(client, server) {
	}

	void parse(Message* message) {
	}

	void run() {
	}
};

#endif /* GETARTICLEMESSAGECALLBACK_H_ */
