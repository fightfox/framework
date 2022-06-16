#include "Player.h"
Player::Player(Type type_, uint32_t id_, shared_ptr<Socket>& socket_, uint8_t own_priv_) 
  : type(type_), id(id_), alive(true), socket(socket_), own_priv(own_priv_) {
  socket->player_id = id;
}