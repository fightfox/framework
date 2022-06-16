#include "Command.h"
#include "../engine/Game.h"

Command::Command(Time time, Game* game, shared_ptr<Socket>& socket_, unique_ptr<Buffer::Reader> reader_) : Event(COMMAND, time, game) {
  reader = std::move(reader_);
  socket = socket_;
}
void Command::handle() {
  player_id = socket->player_id;
  auto player = g->player_list.get(player_id);
  if (player!=nullptr && player->alive)
    g->fn_command(this);
}