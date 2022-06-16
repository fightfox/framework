#include "OnCollision.h"
#include "../engine/Game.h"

OnCollision::OnCollision(Time time, Game* game, uint32_t id1_, uint32_t id2_) : Event(ON_COLLISION, time, game) {
  id1 = id1_;
  id2 = id2_;
}

void OnCollision::handle() {
  auto unit1 = g->unit_list.get(id1);
  auto unit2 = g->unit_list.get(id2);
  if (unit1!=nullptr && unit2!=nullptr && unit1->alive && unit2->alive)
    for (auto& fn : g->fns_on_collision)
      fn(this);
}