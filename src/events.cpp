#include "events.hpp"
#include "mouse.hpp"
#include "preferences.hpp"

#include "SDL.h"

#include <algorithm>
#include <cassert>
#include <stack>
#include <utility>
#include <vector>

namespace events
{

namespace {
	int disallow_resize = 0;
}

resize_lock::resize_lock()
{
	++disallow_resize;
}

resize_lock::~resize_lock()
{
	--disallow_resize;
}

//this object stores all the event handlers. It is a stack of event 'contexts'.
//a new event context is created when e.g. a modal dialog is opened, and then
//closed when that dialog is closed. Each context contains a list of the handlers
//in that context. The current context is the one on the top of the stack
std::stack<std::vector<handler*> > event_contexts;

event_context::event_context()
{
	event_contexts.push(std::vector<handler*>());
}

event_context::~event_context()
{
	assert(event_contexts.empty() == false);
	assert(event_contexts.top().empty());

	event_contexts.pop();
}

handler::handler() : unicode_(SDL_EnableUNICODE(1))
{
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
	event_contexts.top().push_back(this);
}

handler::~handler()
{
	assert(event_contexts.empty() == false);

	std::vector<handler*>& handlers = event_contexts.top();

	assert(handlers.empty() == false);

	//the handler is most likely on the back of the events array,
	//so look there first, otherwise do a complete search.
	if(handlers.back() == this) {
		handlers.pop_back();
	} else {
		const std::vector<handler*>::iterator i = std::find(handlers.begin(),handlers.end(),this);
		if(i != handlers.end()) {
			handlers.erase(i);
		} else {
			std::cerr << "CRITICAL ERROR: Could not find event handler in events array\n";
		}
	}

	SDL_EnableUNICODE(unicode_);
}

void pump()
{
	SDL_PumpEvents();

	static std::pair<int,int> resize_dimensions(0,0);

	//used to keep track of double click events
	static int last_mouse_down = -1;
	static int last_click_x = -1, last_click_y = -1;

	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		if(event_contexts.empty() == false) {

			const std::vector<handler*>& event_handlers = event_contexts.top();

			//events may cause more event handlers to be added and/or removed,
			//so we must use indexes instead of iterators here.
			for(size_t i1 = 0, i2 = event_handlers.size(); i1 != i2 && i1 < event_handlers.size(); ++i1) {
				event_handlers[i1]->handle_event(event);
			}
		}

		switch(event.type) {
			case SDL_VIDEORESIZE: {
				const SDL_ResizeEvent* const resize = reinterpret_cast<SDL_ResizeEvent*>(&event);

				if(resize->w < 800 || resize->h < 600) {
					resize_dimensions.first = 0;
					resize_dimensions.second = 0;
				} else {
					resize_dimensions.first = resize->w;
					resize_dimensions.second = resize->h;
				}

				break;
			}
			
			case SDL_MOUSEBUTTONDOWN: {
				
				if(event.button.button == SDL_BUTTON_LEFT) {
					static const int DoubleClickTime = 500;

					static const int DoubleClickMaxMove = 3;
					const int current_ticks = ::SDL_GetTicks();
					if(last_mouse_down >= 0 && current_ticks - last_mouse_down < DoubleClickTime &&
					   abs(event.button.x - last_click_x) < DoubleClickMaxMove &&
					   abs(event.button.y - last_click_y) < DoubleClickMaxMove) {
						SDL_UserEvent user_event;
						user_event.type = DOUBLE_CLICK_EVENT;
						user_event.code = 0;
						user_event.data1 = reinterpret_cast<void*>(event.button.x);
						user_event.data2 = reinterpret_cast<void*>(event.button.y);
						::SDL_PushEvent(reinterpret_cast<SDL_Event*>(&user_event));
					}

					last_mouse_down = current_ticks;
					last_click_x = event.button.x;
					last_click_y = event.button.y;
				}

				//mouse wheel support
				else if(event.button.button == 4) {
					gui::scroll_dec();
				} else if(event.button.button == 5) {
					gui::scroll_inc();
				}

				break;
			}

			case SDL_QUIT: {
				throw CVideo::quit();
			}
		}
	}

	if(resize_dimensions.first > 0 && disallow_resize == 0) {
		preferences::set_resolution(resize_dimensions);
		resize_dimensions.first = 0;
		resize_dimensions.second = 0;
	}
}

void raise_process_event()
{
	if(event_contexts.empty() == false) {

		const std::vector<handler*>& event_handlers = event_contexts.top();

		//events may cause more event handlers to be added and/or removed,
		//so we must use indexes instead of iterators here.
		for(size_t i1 = 0, i2 = event_handlers.size(); i1 != i2 && i1 < event_handlers.size(); ++i1) {
			event_handlers[i1]->process();
		}
	}
}

void raise_draw_event()
{
	if(event_contexts.empty() == false) {

		const std::vector<handler*>& event_handlers = event_contexts.top();

		//events may cause more event handlers to be added and/or removed,
		//so we must use indexes instead of iterators here.
		for(size_t i1 = 0, i2 = event_handlers.size(); i1 != i2 && i1 < event_handlers.size(); ++i1) {
			event_handlers[i1]->draw();
		}
	}
}

}