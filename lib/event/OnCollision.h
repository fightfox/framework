#ifndef WORLD2D_EVENT_ON_COLLISION
#define WORLD2D_EVENT_ON_COLLISION

#include "Event.h"

struct OnCollision : Event {
  uint32_t id1, id2;
  OnCollision(Time, Game*, uint32_t, uint32_t);
  void handle();
};

#endif // WORLD2D_EVENT_ON_COLLISION

