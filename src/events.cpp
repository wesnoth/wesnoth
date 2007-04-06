/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "clipboard.hpp"
#include "cursor.hpp"
#include "events.hpp"
#include "log.hpp"
#include "preferences_display.hpp"
#include "sound.hpp"
#include "video.hpp"
#include "wassert.hpp"

#include "SDL.h"

#include <algorithm>
#include <deque>
#include <utility>
#include <vector>

#define ERR_GEN LOG_STREAM(err, general)


unsigned input_blocker::instance_count = 0; //static initialization

namespace events
{

void raise_help_string_event(int mousex, int mousey);

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

namespace {

struct context
{
	context() : focused_handler(-1) {}
	void add_handler(handler* ptr);
	bool remove_handler(handler* ptr);
	int cycle_focus();
	void set_focus(const handler* ptr);

	std::vector<handler*> handlers;
	int focused_handler;

private:
	void delete_handler_index(size_t handler);
};

void context::add_handler(handler* ptr)
{
	handlers.push_back(ptr);
}

void context::delete_handler_index(size_t handler)
{
	if(focused_handler == int(handler)) {
		focused_handler = -1;
	} else if(focused_handler > int(handler)) {
		--focused_handler;
	}

	handlers.erase(handlers.begin()+handler);
}

bool context::remove_handler(handler* ptr)
{
	if(handlers.empty()) {
		return false;
	}

	static int depth = 0;
	++depth;

	//the handler is most likely on the back of the events array,
	//so look there first, otherwise do a complete search.
	if(handlers.back() == ptr) {
		delete_handler_index(handlers.size()-1);
	} else {
		const std::vector<handler*>::iterator i = std::find(handlers.begin(),handlers.end(),ptr);
		if(i != handlers.end()) {
			delete_handler_index(i - handlers.begin());
		} else {
			return false;
		}
	}

	--depth;

	if(depth == 0) {
		cycle_focus();
	} else {
		focused_handler = -1;
	}

	return true;
}

int context::cycle_focus()
{
	int index = focused_handler+1;
	for(size_t i = 0; i != handlers.size(); ++i) {
		if(size_t(index) == handlers.size()) {
			index = 0;
		}

		if(handlers[size_t(index)]->requires_event_focus()) {
			focused_handler = index;
			break;
		}
	}

	return focused_handler;
}

void context::set_focus(const handler* ptr)
{
	const std::vector<handler*>::const_iterator i = std::find(handlers.begin(),handlers.end(),ptr);
	if(i != handlers.end() && (**i).requires_event_focus()) {
		focused_handler = int(i - handlers.begin());
	}
}

//this object stores all the event handlers. It is a stack of event 'contexts'.
//a new event context is created when e.g. a modal dialog is opened, and then
//closed when that dialog is closed. Each context contains a list of the handlers
//in that context. The current context is the one on the top of the stack
std::deque<context> event_contexts;

} //end anon namespace

event_context::event_context()
{
	event_contexts.push_back(context());
}

event_context::~event_context()
{
	wassert(event_contexts.empty() == false);
	event_contexts.pop_back();
}

handler::handler(const bool auto_join) : unicode_(SDL_EnableUNICODE(1)), has_joined_(false)
{
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
	if(auto_join) {
		event_contexts.back().add_handler(this);
		has_joined_ = true;
	}
}

handler::~handler()
{
	leave();
	SDL_EnableUNICODE(unicode_);
}

void handler::join()
{
	if(has_joined_) {
		leave(); // should not be in multiple event contexts
	}
	//join self
	event_contexts.back().add_handler(this);
	has_joined_ = true;

	//instruct members to join
	handler_vector members = handler_members();
	if(!members.empty()) {
		for(handler_vector::iterator i = members.begin(); i != members.end(); i++) {
			(*i)->join();
		}
	}
}

void handler::leave()
{
	handler_vector members = handler_members();
	if(!members.empty()) {
		for(handler_vector::iterator i = members.begin(); i != members.end(); i++) {
			(*i)->leave();
		}
	} else {
		wassert(event_contexts.empty() == false);
	}
	for(std::deque<context>::reverse_iterator i = event_contexts.rbegin(); i != event_contexts.rend(); ++i) {
		if(i->remove_handler(this)) {
			break;
		}
	}
	has_joined_ = false;
}

void focus_handler(const handler* ptr)
{
	if(event_contexts.empty() == false) {
		event_contexts.back().set_focus(ptr);
	}
}

bool has_focus(const handler* ptr, const SDL_Event* event)
{
	if(event_contexts.empty()) {
		return true;
	}

	if(ptr->requires_event_focus(event) == false) {
		return true;
	}

	const int index = event_contexts.back().focused_handler;

	//if no-one has focus at the moment, this handler obviously wants
	//focus, so give it to it.
	if(index == -1) {
		focus_handler(ptr);
		return true;
	} else if(event_contexts.back().handlers[index] == ptr){
		return true;
	} else if(!event_contexts.back().handlers[index]->requires_event_focus(event)) {
		//if the currently focused handler doesn't need focus for this event
		//allow the first-in interested handler to take care of it
		for(int i=0; i<event_contexts.back().handlers.size(); i++) {
			if(i != index && event_contexts.back().handlers[i]->requires_event_focus(event)) {
				//focus_handler(event_contexts.back().handlers[i]); //steal focus?
				return event_contexts.back().handlers[i] == ptr;
			}
		}
	}
	return false;
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

		switch(event.type) {

			case SDL_ACTIVEEVENT: {
				SDL_ActiveEvent& ae = reinterpret_cast<SDL_ActiveEvent&>(event);
				if((ae.state & SDL_APPMOUSEFOCUS) != 0 || (ae.state & SDL_APPINPUTFOCUS) != 0) {
					cursor::set_focus(ae.gain != 0);
					events::flush( SDL_EVENTMASK(SDL_KEYDOWN)|
			               SDL_EVENTMASK(SDL_KEYUP)|
			               SDL_EVENTMASK(SDL_MOUSEBUTTONDOWN)|
			               SDL_EVENTMASK(SDL_MOUSEBUTTONUP)|
			               SDL_EVENTMASK(SDL_JOYBUTTONDOWN)|
			               SDL_EVENTMASK(SDL_JOYBUTTONUP)
			             );
				}
				break;
			}

			//if the window must be redrawn, update the entire screen
			case SDL_VIDEOEXPOSE: {
				update_whole_screen();
				break;
			}

			case SDL_VIDEORESIZE: {
				const SDL_ResizeEvent* const resize = reinterpret_cast<SDL_ResizeEvent*>(&event);

				if(resize->w < min_allowed_width || resize->h < min_allowed_height) {
					resize_dimensions.first = 0;
					resize_dimensions.second = 0;
				} else {
					resize_dimensions.first = resize->w;
					resize_dimensions.second = resize->h;
				}

				break;
			}

			case SDL_MOUSEMOTION: {
				//always make sure a cursor is displayed if the
				//mouse moves or if the user clicks
				cursor::set_focus(true);
				raise_help_string_event(event.motion.x,event.motion.y);
				break;
			}

			case SDL_MOUSEBUTTONDOWN: {
				//always make sure a cursor is displayed if the
				//mouse moves or if the user clicks
				cursor::set_focus(true);
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

				break;
			}

#if defined(_X11) && !defined(__APPLE__)
			case SDL_SYSWMEVENT: {
				//clipboard support for X11
				handle_system_event(event);
				break;
			}
#endif

			case SDL_QUIT: {
				throw CVideo::quit();
			}
		}

		if(event_contexts.empty() == false) {

			const std::vector<handler*>& event_handlers = event_contexts.back().handlers;

			//events may cause more event handlers to be added and/or removed,
			//so we must use indexes instead of iterators here.
			for(size_t i1 = 0, i2 = event_handlers.size(); i1 != i2 && i1 < event_handlers.size(); ++i1) {
				event_handlers[i1]->handle_event(event);
			}
		}
	}

	if(resize_dimensions.first > 0 && disallow_resize == 0) {
		preferences::set_resolution(resize_dimensions);
		resize_dimensions.first = 0;
		resize_dimensions.second = 0;
	}

	if (preferences::music_on())
		sound::think_about_music();
}

void raise_process_event()
{
	if(event_contexts.empty() == false) {

		const std::vector<handler*>& event_handlers = event_contexts.back().handlers;

		//events may cause more event handlers to be added and/or removed,
		//so we must use indexes instead of iterators here.
		for(size_t i1 = 0, i2 = event_handlers.size(); i1 != i2 && i1 < event_handlers.size(); ++i1) {
			event_handlers[i1]->process_event();
		}
	}
}

void raise_draw_event()
{
	if(event_contexts.empty() == false) {

		const std::vector<handler*>& event_handlers = event_contexts.back().handlers;

		//events may cause more event handlers to be added and/or removed,
		//so we must use indexes instead of iterators here.
		for(size_t i1 = 0, i2 = event_handlers.size(); i1 != i2 && i1 < event_handlers.size(); ++i1) {
			event_handlers[i1]->draw();
		}
	}
}

void raise_volatile_draw_event()
{
	if(event_contexts.empty() == false) {

		const std::vector<handler*>& event_handlers = event_contexts.back().handlers;

		//events may cause more event handlers to be added and/or removed,
		//so we must use indexes instead of iterators here.
		for(size_t i1 = 0, i2 = event_handlers.size(); i1 != i2 && i1 < event_handlers.size(); ++i1) {
			event_handlers[i1]->volatile_draw();
		}
	}
}

void raise_volatile_undraw_event()
{
	if(event_contexts.empty() == false) {

		const std::vector<handler*>& event_handlers = event_contexts.back().handlers;

		//events may cause more event handlers to be added and/or removed,
		//so we must use indexes instead of iterators here.
		for(size_t i1 = 0, i2 = event_handlers.size(); i1 != i2 && i1 < event_handlers.size(); ++i1) {
			event_handlers[i1]->volatile_undraw();
		}
	}
}

void raise_help_string_event(int mousex, int mousey)
{
	if(event_contexts.empty() == false) {

		const std::vector<handler*>& event_handlers = event_contexts.back().handlers;

		for(size_t i1 = 0, i2 = event_handlers.size(); i1 != i2 && i1 < event_handlers.size(); ++i1) {
			event_handlers[i1]->process_help_string(mousex,mousey);
		}
	}
}

int flush(Uint32 event_mask)
{
	int flush_count = 0;
	SDL_Event temp_event;
	std::vector< SDL_Event > keepers;
	SDL_Delay(10);
	while(SDL_PollEvent(&temp_event) > 0) {
		if((SDL_EVENTMASK(temp_event.type) & event_mask) == 0) {
			keepers.push_back( temp_event );
		} else {
			++flush_count;
		}
	}

	//FIXME: there is a chance new events are added before kept events are replaced
	for (unsigned int i=0; i < keepers.size(); ++i) 
	{
		if(SDL_PushEvent(&keepers[i]) != 0) {
                       ERR_GEN << "failed to return an event to the queue.";
                }
        }

	return flush_count;
}

}
