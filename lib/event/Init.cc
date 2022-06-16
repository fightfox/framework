#include "Init.h"
#include "UpdateWorld.h"
#include "../engine/Game.h"

Init::Init(Game* game) : Event(INIT, -100000, game) {}

void Init::handle() {
  g->now = g->getTime(); 
  g->tick_head = g->now;
  Time first_time_until = g->now+TIME_PER_TICK;
  g->animation.setUntil(first_time_until);
  g->event_queue.insert(new UpdateWorld(first_time_until, g));
  for (auto& fn : g->fns_init)
    fn(this);
  g->socket_manager.start(3000); 
} 

