#ifndef EVENTS_HPP_INCLUDED
#define EVENTS_HPP_INCLUDED

#include "SDL.h"

namespace events
{
struct resize_lock {
	resize_lock();
	~resize_lock();
};

//any classes that derive from this class will automatically
//receive sdl events through the handle function for their lifetime
class handler
{
public:
	virtual void handle_event(const SDL_Event& event) {};
protected:
	handler();
	virtual ~handler();
};

void pump();
}

#endif
