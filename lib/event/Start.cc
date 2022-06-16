#include "Start.h"
#include "../engine/Game.h"

Start::Start(Time time, Game* game, shared_ptr<Socket> socket_, unique_ptr<Buffer::Reader> reader_) 
: Event(START, time, game) {
  socket = socket_;
  reader = std::move(reader_);
}
void Start::handle() {
  if (socket->player_id != INVALID_ID) return;
  id = (g->player_list.last_player_id+=2);
  g->fn_start(this);
}
