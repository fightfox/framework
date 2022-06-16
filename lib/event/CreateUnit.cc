#include "CreateUnit.h"
#include "../engine/Game.h"

CreateUnit::CreateUnit(Time time, Game* game, uint32_t id_) : Event(CREATE_UNIT, time, game) {
  unit_id = id_;
}

void CreateUnit::handle() {
  auto unit = g->unit_list.get(unit_id);
  if (unit!=nullptr && unit->alive)
    for (auto& fn : g->fns_create_unit)
      fn(this);
}