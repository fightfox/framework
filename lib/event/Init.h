#ifndef WORLD2D_EVENT_INIT
#define WORLD2D_EVENT_INIT

#include "Event.h"

struct Init : Event {
	Init(Game*);
	void handle(); 
};

#endif // WORLD2D_EVENT_INIT


