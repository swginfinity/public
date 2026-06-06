/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef CLIENTPROXY_H_
#define CLIENTPROXY_H_

#include "engine/engine.h"
#include "engine/service/proto/events/BaseClientEvent.h"

// Client-side BaseClient with LOCKFREE reentrantTask scheduling
// (Server uses BaseClientProxy which has init() for this purpose)
class ClientProxy : public BaseClient {
public:
	ClientProxy(const String& host, int port) : BaseClient(host, port) {
#ifdef LOCKFREE_BCLIENT_BUFFERS
		// Schedule reentrantTask for lockfree buffer processing
		reentrantTask->scheduleInIoScheduler(10);
#endif
	}
};

#endif // CLIENTPROXY_H_
