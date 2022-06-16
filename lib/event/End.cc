#include "End.h"
#include "../engine/Game.h"

End::End(Time time, Game* game, Player* player, shared_ptr<Socket>& socket) 
  : Event(END, time, game), player(player), socket(socket) {}

void End::handle() {
  if (socket->player_id!=INVALID_ID)
    g->fn_end(this);
  delete player;
}