#include "RemovePlayer.h"
#include "../engine/Game.h"

RemovePlayer::RemovePlayer(Time time, Game* game, uint32_t id_) : Event(REMOVE_PLAYER, time, game) {
  player_id = id_;
}

void RemovePlayer::handle() {
  assert(false && "THE IMPOSSIBLE HAPPENS!");
}