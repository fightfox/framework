#ifndef WORLD2D_EVENT_END
#define WORLD2D_EVENT_END

#include "Event.h"
#include "../io/Socket.h"

struct Player;

struct End : Event {
  Player* player;
  shared_ptr<Socket> socket;
  End(Time, Game*, Player*, shared_ptr<Socket>&);
  void handle();
};

#endif // WORLD2D_EVENT_END
