#include "Close.h"
#include "../engine/Game.h"

Close::Close(Time time, Game* game, shared_ptr<Socket>& socket_) : Event(CLOSE, time, game) {
  socket = socket_;
}
void Close::handle() {
  player_id = socket->player_id;
  auto player = g->player_list.get(player_id);
  if (player!=nullptr && player->alive)
    g->fn_close(this);
  socket->player_id = INVALID_ID;
}