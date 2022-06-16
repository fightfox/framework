#ifndef WORLD2D_EVENT_CREATE_PLAYER
#define WORLD2D_EVENT_CREATE_PLAYER

#include "Event.h"

struct CreatePlayer : Event {
  uint32_t player_id;
  CreatePlayer(Time, Game*, uint32_t);
  void handle(); 
};

#endif // WORLD2D_EVENT_CREATE_PLAYER
