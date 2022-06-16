#include "Event.h"
#include "../engine/Game.h"

Event::Event(Type type, Time time, Game* game): 
  type(type), time(time), g(game) {}
