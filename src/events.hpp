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
//note that handlers should *always* be allocated as automatic variables
//(never on the free store or in static memory)
class handler
{
public:
	virtual void handle_event(const SDL_Event& event) = 0;
protected:
	handler();
	virtual ~handler();

private:
	const int unicode_;
};

void pump();
}

#endif
