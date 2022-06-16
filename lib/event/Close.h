#ifndef WORLD2D_EVENT_CLOSE
#define WORLD2D_EVENT_CLOSE

#include "Event.h"
#include "../io/Socket.h"

struct Player;

struct Close : Event {
  shared_ptr<Socket> socket;
	uint32_t player_id;
  Close(Time, Game*, shared_ptr<Socket>&);
  void handle();
};

#endif // WORLD2D_EVENT_CLOSE
