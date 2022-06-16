#ifndef WORLD2D_EVENT_CREATE_UNIT
#define WORLD2D_EVENT_CREATE_UNIT

#include "Event.h"

struct CreateUnit : Event {
  uint32_t unit_id;
  CreateUnit(Time, Game*, uint32_t);
  void handle(); 
};

#endif // WORLD2D_EVENT_CREATE_UNIT
