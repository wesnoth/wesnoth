#ifndef EVENTS_HPP_INCLUDED
#define EVENTS_HPP_INCLUDED

#include "SDL.h"

//our user-defined double-click event type
#define DOUBLE_CLICK_EVENT SDL_USEREVENT

namespace events
{

//an object which prevents resizing of the screen occuring during
//its lifetime.
struct resize_lock {
	resize_lock();
	~resize_lock();
};

//any classes that derive from this class will automatically
//receive sdl events through the handle function for their lifetime
//note that handlers should *always* be allocated as automatic variables
//(never on the free store or in static memory), as the event mechanism
//relies on handlers being created and destroyed in LIFO ordering.
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

//causes events to be dispatched to all handler objects.
void pump();
}

#endif
