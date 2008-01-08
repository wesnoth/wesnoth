/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
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

#include "SDL.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <utility>
#include <vector>

#define ERR_GEN LOG_STREAM(err, general)
#define INFO_GEN LOG_STREAM(info, general)
#define INPUT_MASK (SDL_EVENTMASK(SDL_KEYDOWN)|\
			               SDL_EVENTMASK(SDL_KEYUP)|\
			               SDL_EVENTMASK(SDL_MOUSEBUTTONDOWN)|\
			               SDL_EVENTMASK(SDL_MOUSEBUTTONUP)|\
			               SDL_EVENTMASK(SDL_JOYBUTTONDOWN)|\
			               SDL_EVENTMASK(SDL_JOYBUTTONUP))

unsigned input_blocker::instance_count = 0; //static initialization

namespace events
{

void raise_help_string_event(int mousex, int mousey);

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

	void delete_handler_index(size_t handler);
};

void context::add_handler(handler* ptr)
{
	handlers.push_back(ptr);
}

void context::delete_handler_index(size_t handler)
{
	if(focused_handler == static_cast<int>(handler)) {
		focused_handler = -1;
	} else if(focused_handler > static_cast<int>(handler)) {
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

std::vector<pump_monitor*> pump_monitors;

} //end anon namespace

pump_monitor::pump_monitor() {
	pump_monitors.push_back(this);
}

pump_monitor::~pump_monitor() {
	pump_monitors.erase(
		std::remove(pump_monitors.begin(), pump_monitors.end(), this),
		pump_monitors.end());
}

event_context::event_context()
{
	event_contexts.push_back(context());
}

event_context::~event_context()
{
	assert(event_contexts.empty() == false);
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
		assert(event_contexts.empty() == false);
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

bool has_focus(const handler* hand, const SDL_Event* event)
{
	if(event_contexts.empty()) {
		return true;
	}

	if(hand->requires_event_focus(event) == false) {
		return true;
	}

	const int foc_i = event_contexts.back().focused_handler;

	//if no-one has focus at the moment, this handler obviously wants
	//focus, so give it to it.
	if(foc_i == -1) {
		focus_handler(hand);
		return true;
	}

	handler *const foc_hand = event_contexts.back().handlers[foc_i];
	if(foc_hand == hand){
		return true;
	} else if(!foc_hand->requires_event_focus(event)) {
		//if the currently focused handler doesn't need focus for this event
		//allow the most recent interested handler to take care of it
		int back_i = event_contexts.back().handlers.size() - 1;
		for(int i=back_i; i>=0; --i) {
			handler *const thief_hand = event_contexts.back().handlers[i];
			if(i != foc_i && thief_hand->requires_event_focus(event)) {
				//steal focus
				focus_handler(thief_hand);
				if(foc_i < back_i) {
					//position the previously focused handler to allow stealing back
					event_contexts.back().delete_handler_index(foc_i);
					event_contexts.back().add_handler(foc_hand);
				}
				return thief_hand == hand;
			}
		}
	}
	return false;
}

void pump()
{
	SDL_PumpEvents();

	pump_info info;

	//used to keep track of double click events
	static int last_mouse_down = -1;
	static int last_click_x = -1, last_click_y = -1;

	SDL_Event temp_event;
	int poll_count = 0;
	int begin_ignoring = 0;
	std::vector< SDL_Event > events;
	while(SDL_PollEvent(&temp_event)) {
		++poll_count;
		if(!begin_ignoring && temp_event.type == SDL_ACTIVEEVENT) {
			begin_ignoring = poll_count;
		} else if(begin_ignoring > 0 && SDL_EVENTMASK(temp_event.type)&INPUT_MASK) {
			//ignore user input events that occurred after the window was activated
			continue;
		}
		events.push_back(temp_event);
	}
	std::vector<SDL_Event>::iterator ev_it = events.begin();
	for(int i=1; i < begin_ignoring; ++i){
		if(SDL_EVENTMASK(ev_it->type)&INPUT_MASK) {
			//ignore user input events that occurred before the window was activated
			ev_it = events.erase(ev_it);
		} else {
			++ev_it;
		}
	}
	std::vector<SDL_Event>::iterator ev_end = events.end();
	for(ev_it = events.begin(); ev_it != ev_end; ++ev_it){
		SDL_Event &event = *ev_it;
		switch(event.type) {

			case SDL_ACTIVEEVENT: {
				SDL_ActiveEvent& ae = reinterpret_cast<SDL_ActiveEvent&>(event);
				if((ae.state & SDL_APPMOUSEFOCUS) != 0 || (ae.state & SDL_APPINPUTFOCUS) != 0) {
					cursor::set_focus(ae.gain != 0);
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
				info.resize_dimensions.first = resize->w;
				info.resize_dimensions.second = resize->h;
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
					if(last_mouse_down >= 0 && info.ticks() - last_mouse_down < DoubleClickTime &&
					   abs(event.button.x - last_click_x) < DoubleClickMaxMove &&
					   abs(event.button.y - last_click_y) < DoubleClickMaxMove) {
						SDL_UserEvent user_event;
						user_event.type = DOUBLE_CLICK_EVENT;
						user_event.code = 0;
						user_event.data1 = reinterpret_cast<void*>(event.button.x);
						user_event.data2 = reinterpret_cast<void*>(event.button.y);
						::SDL_PushEvent(reinterpret_cast<SDL_Event*>(&user_event));
					}
					last_mouse_down = info.ticks();
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

	//inform the pump monitors that an events::pump() has occurred
	for(size_t i1 = 0, i2 = pump_monitors.size(); i1 != i2 && i1 < pump_monitors.size(); ++i1) {
		pump_monitors[i1]->process(info);
	}
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

int discard(Uint32 event_mask)
{
	int discard_count = 0;
	SDL_Event temp_event;
	std::vector< SDL_Event > keepers;
	SDL_Delay(10);
	while(SDL_PollEvent(&temp_event) > 0) {
		if((SDL_EVENTMASK(temp_event.type) & event_mask) == 0) {
			keepers.push_back( temp_event );
		} else {
			++discard_count;
		}
	}

	//FIXME: there is a chance new events are added before kept events are replaced
	for (unsigned int i=0; i < keepers.size(); ++i)
	{
		if(SDL_PushEvent(&keepers[i]) != 0) {
			ERR_GEN << "failed to return an event to the queue.";
		}
	}

	return discard_count;
}

int pump_info::ticks(unsigned *refresh_counter, unsigned refresh_rate) {
	if(!ticks_ && !(refresh_counter && ++*refresh_counter % refresh_rate)) {
		ticks_ = ::SDL_GetTicks();
	}
	return ticks_;
}

} //end events namespace

input_blocker::input_blocker()
{
	SDL_EventState(SDL_KEYDOWN, SDL_IGNORE);
	SDL_EventState(SDL_KEYUP, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);
	SDL_EventState(SDL_JOYBUTTONDOWN, SDL_IGNORE);
	SDL_EventState(SDL_JOYBUTTONUP, SDL_IGNORE);
	instance_count++;
}

input_blocker::~input_blocker()
{
	instance_count--;
	if(instance_count == 0) {
		events::discard(INPUT_MASK);
		SDL_EventState(SDL_KEYDOWN, SDL_ENABLE);
		SDL_EventState(SDL_KEYUP, SDL_ENABLE);
		SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_ENABLE);
		SDL_EventState(SDL_MOUSEBUTTONUP, SDL_ENABLE);
		SDL_EventState(SDL_JOYBUTTONDOWN, SDL_ENABLE);
		SDL_EventState(SDL_JOYBUTTONUP, SDL_ENABLE);
	}
}
