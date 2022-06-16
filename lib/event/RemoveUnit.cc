#include "RemoveUnit.h"
#include "../engine/Game.h"

RemoveUnit::RemoveUnit(Time time, Game* game, uint32_t id_, Trace* tr) : Event(REMOVE_UNIT, time, game) {
  unit_id = id_;
  trace = tr;
}

void RemoveUnit::handle() {
  assert(false && "THE IMPOSSIBLE HAPPENS!");
}