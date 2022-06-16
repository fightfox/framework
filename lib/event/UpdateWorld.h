#ifndef WORLD2D_EVENT_UPDATE_WORLD
#define WORLD2D_EVENT_UPDATE_WORLD

#include "Event.h"

struct UpdateWorld : Event {
	UpdateWorld(Time, Game*);
	void handle();
};

#endif // WORLD2D_EVENT_UPDATE_WORLD

