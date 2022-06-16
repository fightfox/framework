#ifndef WORLD2D_EVENT_COMMAND
#define WORLD2D_EVENT_COMMAND

#include "Event.h"
#include "../io/Buffer.h"
#include "../io/Socket.h"

struct Player;

struct Command : Event {
	unique_ptr<Buffer::Reader> reader;
	shared_ptr<Socket> socket;
  uint32_t player_id;
	Command(Time, Game*, shared_ptr<Socket>&, unique_ptr<Buffer::Reader>);
	void handle();
};

#endif // WORLD2D_EVENT_COMMAND
