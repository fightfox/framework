#ifndef WORLD2D_EVENT_EVENT
#define WORLD2D_EVENT_EVENT

#include "../std.h"

struct Game;

struct Event {
  enum Type : uint8_t{
    /* system events */
    INIT, 
    ON_COLLISION, 
    UPDATE_WORLD, 
    /* operation events */
    START,
    END,
    CLOSE, 
    COMMAND, 
    /* action events */
    CREATE_UNIT, 
    CREATE_PLAYER, 
    REMOVE_UNIT, 
    REMOVE_PLAYER
  } type;
  Time time;
  Game* g;
  uint32_t heap_id;
  uint32_t id;

  Event(Type, Time, Game*);
  virtual ~Event(){}
  virtual void handle() = 0;
};

#endif // WORLD2D_EVENT_EVENT
