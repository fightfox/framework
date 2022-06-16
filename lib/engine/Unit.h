#ifndef WORLD2D_ENGINE_UNIT
#define WORLD2D_ENGINE_UNIT

#include "../util/Trace.h"
#include "../util/XY.h"
#include "../io/Socket.h"

struct Game;
struct Player;

struct Unit {
  enum Type: uint8_t;   // declaration. Defined by user
  Type type;
  Trace_uptr trace;
  uint8_t own_priv;
  uint32_t id;
  bool visible;
  bool alive;
  XY v;
  double mass;
  Unit(Type type, uint8_t own_priv):
    type(type), own_priv(own_priv), v(0,0), mass(1) {
      visible = true;
      alive = true;
    }
  virtual ~Unit() {}
  virtual void toMessage(uint8_t, Socket&) = 0;
};

#endif // WORLD2D_ENGINE_UNIT