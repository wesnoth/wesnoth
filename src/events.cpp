/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "events.hpp"
#include "cursor.hpp"
#include "desktop/clipboard.hpp"
#include "log.hpp"
#include "ogl/utils.hpp"
#include "quit_confirmation.hpp"
#include "video.hpp"
#include "sdl/userevent.hpp"

#if defined _WIN32
#include "desktop/windows_tray_notification.hpp"
#endif

#include <algorithm>
#include <cassert>
#include <deque>
#include <iterator>
#include <utility>
#include <vector>

#include <SDL.h>

#include <boost/range/adaptor/reversed.hpp>
#include <boost/thread.hpp>

#define ERR_GEN LOG_STREAM(err, lg::general)

namespace
{
struct invoked_function_data
{
	bool finished;
	const std::function<void(void)>& f;

	invoked_function_data(bool finished_, const std::function<void(void)>& func)
		: finished(finished_)
		, f(func)
	{
	}

	void call()
	{
		f();
		finished = true;
	}
};
}

namespace events
{
void context::add_handler(sdl_handler* ptr)
{
	/* Add new handlers to the staging list initially.
	 * This ensures that if an event handler adds more handlers, the new handlers
	 * won't be called for the event that caused them to be added.
	 */
	staging_handlers.push_back(ptr);
}

bool context::remove_handler(sdl_handler* ptr)
{
	static int depth = 0;
	++depth;

	// The handler is most likely on the back of the events list,
	// so look there first, otherwise do a complete search.
	if(!handlers.empty() && handlers.back() == ptr) {
		if(focused_handler != handlers.end() && *focused_handler == ptr) {
			focused_handler = handlers.end();
		}

		handlers.pop_back();
	} else {
		const handler_list::iterator i = std::find(handlers.begin(), handlers.end(), ptr);

		if(i == handlers.end()) {
			--depth;

			// The handler may be in the staging area. Search it from there.
			auto j = std::find(staging_handlers.begin(), staging_handlers.end(), ptr);
			if(j != staging_handlers.end()) {
				staging_handlers.erase(j);
				return true;
			} else {
				return false;
			}
		}

		if(i == focused_handler) {
			focused_handler != handlers.begin() ? --focused_handler : ++focused_handler;
		}

		handlers.erase(i);
	}

	--depth;

	if(depth == 0) {
		cycle_focus();
	} else {
		focused_handler = handlers.end();
	}

	return true;
}

void context::cycle_focus()
{
	if(handlers.begin() == handlers.end()) {
		return;
	}

	handler_list::iterator current = focused_handler;
	handler_list::iterator last = focused_handler;

	if(last != handlers.begin()) {
		--last;
	}

	if(current == handlers.end()) {
		current = handlers.begin();
	} else {
		++current;
	}

	while(current != last) {
		if(current != handlers.end() && (*current)->requires_event_focus()) {
			focused_handler = current;
			break;
		}

		if(current == handlers.end()) {
			current = handlers.begin();
		} else {
			++current;
		}
	}
}

void context::set_focus(const sdl_handler* ptr)
{
	const handler_list::iterator i = std::find(handlers.begin(), handlers.end(), ptr);
	if(i != handlers.end() && (*i)->requires_event_focus()) {
		focused_handler = i;
	}
}

void context::add_staging_handlers()
{
	std::copy(staging_handlers.begin(), staging_handlers.end(), std::back_inserter(handlers));
	staging_handlers.clear();
}

context::~context()
{
	for(sdl_handler* h : handlers) {
		if(h->has_joined()) {
			h->has_joined_ = false;
		}

		if(h->has_joined_global()) {
			h->has_joined_global_ = false;
		}
	}
}

// This object stores all the event handlers. It is a stack of event 'contexts'.
// a new event context is created when e.g. a modal dialog is opened, and then
// closed when that dialog is closed. Each context contains a list of the handlers
// in that context. The current context is the one on the top of the stack.
// The global context must always be in the first position.
std::deque<context> event_contexts;

std::vector<pump_monitor*> pump_monitors;

pump_monitor::pump_monitor()
{
	pump_monitors.push_back(this);
}

pump_monitor::~pump_monitor()
{
	pump_monitors.erase(std::remove(pump_monitors.begin(), pump_monitors.end(), this), pump_monitors.end());
}

event_context::event_context()
{
	event_contexts.emplace_back();
}

event_context::~event_context()
{
	assert(event_contexts.empty() == false);
	event_contexts.pop_back();
}

sdl_handler::sdl_handler(const bool auto_join)
	: has_joined_(false)
	, has_joined_global_(false)
{
	if(auto_join) {
		assert(!event_contexts.empty());
		event_contexts.back().add_handler(this);
		has_joined_ = true;
	}
}

sdl_handler::~sdl_handler()
{
	if(has_joined_) {
		leave();
	}

	if(has_joined_global_) {
		leave_global();
	}
}

void sdl_handler::join()
{
	// this assert will fire if someone will inadvertently try to join
	// an event context but might end up in the global context instead.
	assert(&event_contexts.back() != &event_contexts.front());

	join(event_contexts.back());
}

void sdl_handler::join(context& c)
{
	if(has_joined_global_) {
		leave_global();
	}

	if(has_joined_) {
		leave(); // should not be in multiple event contexts
	}

	// join self
	c.add_handler(this);
	has_joined_ = true;

	// instruct members to join
	for(auto member : handler_members()) {
		member->join(c);
	}
}

void sdl_handler::join_same(sdl_handler* parent)
{
	if(has_joined_) {
		leave(); // should not be in multiple event contexts
	}

	for(auto& context : boost::adaptors::reverse(event_contexts)) {
		handler_list& handlers = context.handlers;
		if(std::find(handlers.begin(), handlers.end(), parent) != handlers.end()) {
			join(context);
			return;
		}
	}

	join(event_contexts.back());
}

void sdl_handler::leave()
{
	sdl_handler_vector members = handler_members();

	if(members.empty()) {
		assert(event_contexts.empty() == false);
	}

	for(auto member : members) {
		member->leave();
	}

	for(auto& context : boost::adaptors::reverse(event_contexts)) {
		if(context.remove_handler(this)) {
			break;
		}
	}

	has_joined_ = false;
}

void sdl_handler::join_global()
{
	if(has_joined_) {
		leave();
	}

	if(has_joined_global_) {
		leave_global(); // Should not be in multiple event contexts
	}

	// Join self
	event_contexts.front().add_handler(this);
	has_joined_global_ = true;

	// Instruct members to join
	for(auto member : handler_members()) {
		member->join_global();
	}
}

void sdl_handler::leave_global()
{
	for(auto member : handler_members()) {
		member->leave_global();
	}

	event_contexts.front().remove_handler(this);

	has_joined_global_ = false;
}

void focus_handler(const sdl_handler* ptr)
{
	if(event_contexts.empty() == false) {
		event_contexts.back().set_focus(ptr);
	}
}

bool has_focus(const sdl_handler* hand, const SDL_Event* event)
{
	if(event_contexts.empty()) {
		return true;
	}

	if(hand->requires_event_focus(event) == false) {
		return true;
	}

	const handler_list::iterator foc = event_contexts.back().focused_handler;
	auto& handlers = event_contexts.back().handlers;

	// If no-one has focus at the moment, this handler obviously wants
	// focus, so give it to it.
	if(foc == handlers.end()) {
		focus_handler(hand);
		return true;
	}

	sdl_handler* const foc_hand = *foc;
	if(foc_hand == hand) {
		return true;
	} else if(!foc_hand->requires_event_focus(event)) {
		// If the currently focused handler doesn't need focus for this event
		// allow the most recent interested handler to take care of it
		for(auto i = handlers.rbegin(); i != handlers.rend(); ++i) {
			sdl_handler* const thief_hand = *i;

			if(thief_hand != foc_hand && thief_hand->requires_event_focus(event)) {
				// Steal focus
				focus_handler(thief_hand);

				// Position the previously focused handler to allow stealing back
				handlers.splice(handlers.end(), handlers, foc);

				return thief_hand == hand;
			}
		}
	}

	return false;
}

const uint32_t resize_timeout = 100;
SDL_Event last_resize_event;
bool last_resize_event_used = true;

static bool remove_on_resize(const SDL_Event& a)
{
	if(a.type == DRAW_EVENT || a.type == DRAW_ALL_EVENT) {
		return true;
	}

	if(a.type == SHOW_HELPTIP_EVENT) {
		return true;
	}

	if((a.type == SDL_WINDOWEVENT) && (
		a.window.event == SDL_WINDOWEVENT_RESIZED ||
		a.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
		a.window.event == SDL_WINDOWEVENT_EXPOSED)
	) {
		return true;
	}

	return false;
}

/**
 * The interval between draw events.
 *
 * When the window is shown this value is set, the callback function always
 * uses this value instead of the parameter send, that way the window can stop
 * drawing when it wants.
 */
static int draw_interval = 0;

SDL_TimerID draw_timer_id;

/**
 * SDL_AddTimer() callback for the draw event.
 *
 * When this callback is called it pushes a new draw event in the event queue.
 *
 * @returns                       The new timer interval, 0 to stop.
 */
static Uint32 draw_timer(Uint32, void*)
{
	//	DBG_GUI_E << "Pushing draw event in queue.\n";

	SDL_Event event;
	SDL_UserEvent data;

	data.type = DRAW_EVENT;
	data.code = 0;
	data.data1 = NULL;
	data.data2 = NULL;

	event.type = DRAW_EVENT;
	event.user = data;

	SDL_PushEvent(&event);
	return draw_interval;
}

void initialise()
{
	draw_interval = 20;
	draw_timer_id = SDL_AddTimer(draw_interval, draw_timer, NULL);
}

void finalize()
{
	SDL_RemoveTimer(draw_timer_id);
}

// TODO: I'm uncertain if this is always safe to call at static init; maybe set in main() instead?
static const boost::thread::id main_thread = boost::this_thread::get_id();

void pump()
{
	if(boost::this_thread::get_id() != main_thread) {
		// Can only call this on the main thread!
		return;
	}

	peek_for_resize();
	pump_info info;

	// Used to keep track of double click events
	static int last_mouse_down = -1;
	static int last_click_x = -1, last_click_y = -1;

	SDL_Event temp_event;
	int poll_count = 0;
	int begin_ignoring = 0;

	std::vector<SDL_Event> events;
	while(SDL_PollEvent(&temp_event)) {
		if(temp_event.type == INVOKE_FUNCTION_EVENT) {
			static_cast<invoked_function_data*>(temp_event.user.data1)->call();
			continue;
		}

		++poll_count;
		peek_for_resize();

		if(!begin_ignoring && temp_event.type == SDL_WINDOWEVENT && (
			temp_event.window.event == SDL_WINDOWEVENT_ENTER ||
			temp_event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
		) {
			begin_ignoring = poll_count;
		} else if(begin_ignoring > 0 && is_input(temp_event)) {
			// ignore user input events that occurred after the window was activated
			continue;
		}

		events.push_back(temp_event);
	}

	std::vector<SDL_Event>::iterator ev_it = events.begin();
	for(int i = 1; i < begin_ignoring; ++i) {
		if(is_input(*ev_it)) {
			// ignore user input events that occurred before the window was activated
			ev_it = events.erase(ev_it);
		} else {
			++ev_it;
		}
	}

	std::vector<SDL_Event>::iterator ev_end = events.end();
	bool resize_found = false;
	for(ev_it = events.begin(); ev_it != ev_end; ++ev_it) {
		SDL_Event& event = *ev_it;
		if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
			resize_found = true;
			last_resize_event = event;
			last_resize_event_used = false;
		}
	}

	// remove all inputs, draw events and only keep the last of the resize events
	// This will turn horrible after ~38 days when the uint32_t wraps.
	if(resize_found || SDL_GetTicks() <= last_resize_event.window.timestamp + resize_timeout) {
		events.erase(std::remove_if(events.begin(), events.end(), remove_on_resize), events.end());
	} else if(SDL_GetTicks() > last_resize_event.window.timestamp + resize_timeout && !last_resize_event_used) {
		events.insert(events.begin(), last_resize_event);
		last_resize_event_used = true;
	}

	// Move all draw events to the end of the queue
	auto first_draw_event = std::stable_partition(events.begin(), events.end(),
		[](const SDL_Event& e) { return e.type != DRAW_EVENT; }
	);

	if(first_draw_event != events.end()) {
		// Remove all draw events except one
		events.erase(first_draw_event + 1, events.end());
	}

	ev_end = events.end();

	for(ev_it = events.begin(); ev_it != ev_end; ++ev_it) {
		for(context& c : event_contexts) {
			c.add_staging_handlers();
		}

		SDL_Event& event = *ev_it;
		switch(event.type) {
		case SDL_WINDOWEVENT:
			switch(event.window.event) {
			case SDL_WINDOWEVENT_ENTER:
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				cursor::set_focus(1);
				break;

			case SDL_WINDOWEVENT_LEAVE:
			case SDL_WINDOWEVENT_FOCUS_LOST:
				cursor::set_focus(1);
				break;

			case SDL_WINDOWEVENT_RESIZED:
				info.resize_dimensions.first = event.window.data1;
				info.resize_dimensions.second = event.window.data2;
				break;
			}

			// make sure this runs in it's own scope.
			{
				flip_locker flip_lock(CVideo::get_singleton());
				for(auto& context : event_contexts) {
					for(auto handler : context.handlers) {
						handler->handle_window_event(event);
					}
				}

				for(auto global_handler : event_contexts.front().handlers) {
					global_handler->handle_window_event(event);
				}
			}

			// This event was just distributed, don't re-distribute.
			continue;

		case SDL_MOUSEMOTION: {
			// Always make sure a cursor is displayed if the mouse moves or if the user clicks
			cursor::set_focus(true);
			raise_help_string_event(event.motion.x, event.motion.y);
			break;
		}

		case SDL_MOUSEBUTTONDOWN: {
			// Always make sure a cursor is displayed if the mouse moves or if the user clicks
			cursor::set_focus(true);
			if(event.button.button == SDL_BUTTON_LEFT) {
				static const int DoubleClickTime = 500;
				static const int DoubleClickMaxMove = 3;

				if(last_mouse_down >= 0 && info.ticks() - last_mouse_down < DoubleClickTime
						&& std::abs(event.button.x - last_click_x) < DoubleClickMaxMove
						&& std::abs(event.button.y - last_click_y) < DoubleClickMaxMove
				) {
					sdl::UserEvent user_event(DOUBLE_CLICK_EVENT, event.button.x, event.button.y);
					::SDL_PushEvent(reinterpret_cast<SDL_Event*>(&user_event));
				}

				last_mouse_down = info.ticks();
				last_click_x = event.button.x;
				last_click_y = event.button.y;
			}
			break;
		}

		case DRAW_ALL_EVENT: {
			flip_locker flip_lock(CVideo::get_singleton());

			/* Iterate backwards as the most recent things will be at the top */
			// FIXME? ^ that isn't happening here.
			for(auto& context : event_contexts) {
				for(auto handler : context.handlers) {
					handler->handle_event(event);
				}
			}

			continue; // do not do further handling here
		}

#ifndef __APPLE__
		case SDL_KEYDOWN: {
			if(event.key.keysym.sym == SDLK_F4 &&
				(event.key.keysym.mod == KMOD_RALT || event.key.keysym.mod == KMOD_LALT)
			) {
				quit_confirmation::quit_to_desktop();
				continue; // this event is already handled
			}
			break;
		}
#endif

#if defined(_X11) && !defined(__APPLE__)
		case SDL_SYSWMEVENT: {
			// clipboard support for X11
			desktop::clipboard::handle_system_event(event);
			break;
		}
#endif

#if defined _WIN32
		case SDL_SYSWMEVENT: {
			windows_tray_notification::handle_system_event(event);
			break;
		}
#endif

		case SDL_QUIT: {
			quit_confirmation::quit_to_desktop();
			continue; // this event is already handled.
		}
		}

		const bool is_draw_event = event.type == DRAW_EVENT || event.type == DRAW_ALL_EVENT;

		if(is_draw_event) {
#ifdef USE_GL_RENDERING
			gl::clear_screen();
#else
			CVideo::get_singleton().clear_screen();
#endif
		}

		for(auto global_handler : event_contexts.front().handlers) {
			global_handler->handle_event(event);
		}

		if(event_contexts.empty() == false) {
			for(auto handler : event_contexts.back().handlers) {
				handler->handle_event(event);
			}
		}

		if(is_draw_event) {
			CVideo::get_singleton().render_screen();
		}
	}

	// Inform the pump monitors that an events::pump() has occurred
	for(auto monitor : pump_monitors) {
		monitor->process(info);
	}
}

void raise_process_event()
{
	if(event_contexts.empty() == false) {
		event_contexts.back().add_staging_handlers();

		for(auto handler : event_contexts.back().handlers) {
			handler->process_event();
		}
	}
}

void raise_resize_event()
{
	SDL_Event event;
	event.window.type = SDL_WINDOWEVENT;
	event.window.event = SDL_WINDOWEVENT_RESIZED;
	event.window.windowID = 0; // We don't check this anyway... I think...
	event.window.data1 = CVideo::get_singleton().get_width();
	event.window.data2 = CVideo::get_singleton().get_height();

	SDL_PushEvent(&event);
}

void raise_draw_event()
{
	if(event_contexts.empty() == false) {
		event_contexts.back().add_staging_handlers();

		// Events may cause more event handlers to be added and/or removed,
		// so we must use indexes instead of iterators here.
		for(auto handler : event_contexts.back().handlers) {
			handler->draw();
		}
	}
}

void raise_draw_all_event()
{
	for(auto& context : event_contexts) {
		for(auto handler : context.handlers) {
			handler->draw();
		}
	}
}

void raise_volatile_draw_event()
{
	if(event_contexts.empty() == false) {
		for(auto handler : event_contexts.back().handlers) {
			handler->volatile_draw();
		}
	}
}

void raise_volatile_draw_all_event()
{
	for(auto& context : event_contexts) {
		for(auto handler : context.handlers) {
			handler->volatile_draw();
		}
	}
}

void raise_volatile_undraw_event()
{
	if(event_contexts.empty() == false) {
		for(auto handler : event_contexts.back().handlers) {
			handler->volatile_undraw();
		}
	}
}

void raise_help_string_event(int mousex, int mousey)
{
	if(event_contexts.empty() == false) {
		for(auto handler : event_contexts.back().handlers) {
			handler->process_help_string(mousex, mousey);
			handler->process_tooltip_string(mousex, mousey);
		}
	}
}

int pump_info::ticks(unsigned* refresh_counter, unsigned refresh_rate)
{
	if(!ticks_ && !(refresh_counter && ++*refresh_counter % refresh_rate)) {
		ticks_ = ::SDL_GetTicks();
	}

	return ticks_;
}

/* The constants for the minimum and maximum are picked from the headers. */
#define INPUT_MIN 0x300
#define INPUT_MAX 0x8FF

bool is_input(const SDL_Event& event)
{
	return event.type >= INPUT_MIN && event.type <= INPUT_MAX;
}

void discard_input()
{
	SDL_FlushEvents(INPUT_MIN, INPUT_MAX);
}

void peek_for_resize()
{
#if 0
	SDL_Event events[100];
	int num = SDL_PeepEvents(events, 100, SDL_PEEKEVENT, SDL_WINDOWEVENT, SDL_WINDOWEVENT);
	for(int i = 0; i < num; ++i) {
		if(events[i].type == SDL_WINDOWEVENT && events[i].window.event == SDL_WINDOWEVENT_RESIZED) {
			// Add something here if needed.
		}
	}
#endif
}

void call_in_main_thread(const std::function<void(void)>& f)
{
	if(boost::this_thread::get_id() == main_thread) {
		// nothing special to do if called from the main thread.
		f();
		return;
	}

	invoked_function_data fdata{false, f};

	SDL_Event sdl_event;
	sdl::UserEvent sdl_userevent(INVOKE_FUNCTION_EVENT, &fdata);

	sdl_event.type = INVOKE_FUNCTION_EVENT;
	sdl_event.user = sdl_userevent;

	SDL_PushEvent(&sdl_event);

	while(!fdata.finished) {
		SDL_Delay(10);
	}
}

} // end events namespace
