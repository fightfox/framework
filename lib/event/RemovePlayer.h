#ifndef WORLD2D_EVENT_REMOVE_PLAYER
#define WORLD2D_EVENT_REMOVE_PLAYER

#include "Event.h"

struct RemovePlayer : Event {
  uint32_t player_id;
  RemovePlayer(Time, Game*, uint32_t);
  void handle(); // this is instant event
};

#endif // WORLD2D_EVENT_REMOVE_PLAYER
