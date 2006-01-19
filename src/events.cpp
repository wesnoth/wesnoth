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
#include "mouse.hpp"
#include "preferences_display.hpp"
#include "sound.hpp"
#include "video.hpp"
#include "wassert.hpp"

#include "SDL.h"

#include <algorithm>
#include <deque>
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

event_context::event_context(bool create) : create_(create)
{
	if(create_) {
		event_contexts.push_back(context());
	}
}

event_context::~event_context()
{
	if(create_) {
		wassert(event_contexts.empty() == false);

		event_contexts.pop_back();
	}
}

handler::handler() : unicode_(SDL_EnableUNICODE(1))
{
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
	event_contexts.back().add_handler(this);
}

handler::~handler()
{
	wassert(event_contexts.empty() == false);

	for(std::deque<context>::reverse_iterator i = event_contexts.rbegin(); i != event_contexts.rend(); ++i) {
		if(i->remove_handler(this)) {
			break;
		}
	}

	SDL_EnableUNICODE(unicode_);
}

void focus_handler(const handler* ptr)
{
	if(event_contexts.empty() == false) {
		event_contexts.back().set_focus(ptr);
	}
}

void cycle_focus()
{
	if(event_contexts.empty() == false) {
		event_contexts.back().cycle_focus();
	}
}

bool has_focus(const handler* ptr)
{
	if(event_contexts.empty()) {
		return true;
	}

	if(ptr->requires_event_focus() == false) {
		return true;
	}

	const int index = event_contexts.back().focused_handler;

	//if no-one has focus at the moment, this handler obviously wants
	//focus, so give it to it.
	if(index == -1) {
		focus_handler(ptr);
		return true;
	} else {
		return event_contexts.back().handlers[index] == ptr;
	}
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

			case SDL_APPMOUSEFOCUS: {
				SDL_ActiveEvent& ae = reinterpret_cast<SDL_ActiveEvent&>(event);
				if(ae.state == SDL_APPMOUSEFOCUS || ae.state == SDL_APPINPUTFOCUS) {
					cursor::set_focus(ae.gain == 1);
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

				//mouse wheel support
				else if(event.button.button == 4) {
					gui::scroll_dec();
				} else if(event.button.button == 5) {
					gui::scroll_inc();
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

}
