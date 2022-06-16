#ifndef WORLD2D_EVENT_QUEUE
#define WORLD2D_EVENT_QUEUE

#include "Event.h"
#include "../util/heap.h"

struct EventQueue {
  heap<Event> h;
  void insert(Event*);
  // TODO: sort event by time and tie breaking the same time by the insert order
  unique_ptr<Event> consume();
  Time earliestTime();
  void remove(Event*);
};

#endif // WORLD2D_EVENT_QUEUE