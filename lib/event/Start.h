#ifndef WORLD2D_EVENT_START
#define WORLD2D_EVENT_START

#include "Event.h"
#include "../io/Buffer.h"
#include "../io/Socket.h"

struct Start : Event {
	shared_ptr<Socket> socket;
	unique_ptr<Buffer::Reader> reader;
  uint32_t id;
	Start(Time, Game*, shared_ptr<Socket>, unique_ptr<Buffer::Reader>);
	void handle();
};

#endif // WORLD2D_EVENT_START
