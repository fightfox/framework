#include "CreatePlayer.h"
#include "../engine/Game.h"

CreatePlayer::CreatePlayer(Time time, Game* game, uint32_t id_) : Event(CREATE_PLAYER, time, game) {
  player_id = id_;
}

void CreatePlayer::handle() {
  auto player = g->player_list.get(player_id);
  if (player!=nullptr && player->alive)
    for (auto& fn : g->fns_create_player)
      fn(this);
}