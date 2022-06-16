#ifndef WORLD2D_EVENT_REMOVE_UNIT
#define WORLD2D_EVENT_REMOVE_UNIT

#include "Event.h"
#include "../util/Trace.h"

struct RemoveUnit : Event {
  uint32_t unit_id;
  Trace* trace;
  RemoveUnit(Time, Game*, uint32_t, Trace*);
  void handle(); // this is instant event
};

#endif // WORLD2D_EVENT_REMOVE_UNIT
