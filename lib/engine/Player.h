#ifndef WORLD2D_ENGINE_PLAYER
#define WORLD2D_ENGINE_PLAYER

#include "../io/Socket.h"
#include "../util/BB.h"

struct Player {
  enum Type: uint8_t;
  Type type;
  uint32_t id;
  bool alive;
  shared_ptr<Socket> socket;
  uint8_t own_priv;
  Buffer::Writer move_list;
  Player(Type, uint32_t, shared_ptr<Socket>&, uint8_t);
  virtual void toMessage(uint8_t, Socket&) = 0;
  virtual BB getBB() = 0;
  virtual ~Player() {}
};

#endif // WORLD2D_ENGINE_PLAYER
