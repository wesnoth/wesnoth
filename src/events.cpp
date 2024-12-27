/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "draw_manager.hpp"
#include "preferences/preferences.hpp"
#include "quit_confirmation.hpp"
#include "sdl/userevent.hpp"
#include "utils/ranges.hpp"
#include "utils/general.hpp"
#include "video.hpp"

#if defined _WIN32
#include "desktop/windows_tray_notification.hpp"
#endif

#include <algorithm>
#include <cassert>
#include <deque>
#include <future>
#include <iterator>
#include <thread>
#include <utility>
#include <vector>

#include <SDL2/SDL.h>

#define ERR_GEN LOG_STREAM(err, lg::general)

static lg::log_domain log_display("display");
#define LOG_DP LOG_STREAM(info, log_display)

static lg::log_domain log_event("event");
#define LOG_EV LOG_STREAM(info, log_event)
#define DBG_EV LOG_STREAM(debug, log_event)

namespace
{
struct invoked_function_data
{
	explicit invoked_function_data(const std::function<void(void)>& func)
		: f(func)
		, finished()
	{
	}

	/** The actual function to call. */
	const std::function<void(void)>& f;

	/** Whether execution in the main thread is complete. */
	std::promise<void> finished;

	void call()
	{
		try {
			f();
		} catch(const video::quit&) {
			// Handle this exception in the main thread.
			throw;
		} catch(...) {
			DBG_EV << "Caught exception in invoked function: " << utils::get_unknown_exception_type();
			finished.set_exception(std::current_exception());
			return;
		}

		finished.set_value();
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

bool context::has_handler(const sdl_handler* ptr) const
{
	if(handlers.cend() != std::find(handlers.cbegin(), handlers.cend(), ptr)) {
		return true;
	}
	return staging_handlers.cend() != std::find(staging_handlers.cbegin(), staging_handlers.cend(), ptr);
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
	utils::erase(pump_monitors, this);
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

sdl_handler::sdl_handler(const sdl_handler &that)
	: has_joined_(that.has_joined_)
	, has_joined_global_(that.has_joined_global_)
{
	if(has_joined_global_) {
		assert(!event_contexts.empty());
		event_contexts.front().add_handler(this);
	} else if(has_joined_) {
		bool found_context = false;
		for(auto &context : event_contexts | utils::views::reverse) {
			if(context.has_handler(&that)) {
				found_context = true;
				context.add_handler(this);
				break;
			}
		}

		if (!found_context) {
			throw std::logic_error("Copy-constructing a sdl_handler that has_joined_ but can't be found by searching contexts");
		}
	}
}

sdl_handler &sdl_handler::operator=(const sdl_handler &that)
{
	if(that.has_joined_global_) {
		join_global();
	} else if(that.has_joined_) {
		for(auto &context : event_contexts | utils::views::reverse) {
			if(context.has_handler(&that)) {
				join(context);
				break;
			}
		}
	} else if(has_joined_) {
		leave();
	} else if(has_joined_global_) {
		leave_global();
	}

	return *this;
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

	for(auto& context : event_contexts | utils::views::reverse) {
		if(context.has_handler(parent)) {
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

	for(auto& context : event_contexts | utils::views::reverse) {
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

static void raise_window_event(const SDL_Event& event)
{
	for(auto& context : event_contexts) {
		for(auto handler : context.handlers) {
			handler->handle_window_event(event);
		}
	}

	for(auto global_handler : event_contexts.front().handlers) {
		global_handler->handle_window_event(event);
	}
}

// TODO: I'm uncertain if this is always safe to call at static init; maybe set in main() instead?
static const std::thread::id main_thread = std::this_thread::get_id();

// this should probably be elsewhere, but as the main thread is already
// being tracked here, this went here.
bool is_in_main_thread()
{
	return std::this_thread::get_id() == main_thread;
}

void pump()
{
	if(!is_in_main_thread()) {
		// Can only call this on the main thread!
		return;
	}

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

	auto ev_it = events.begin();
	for(int i = 1; i < begin_ignoring; ++i) {
		if(is_input(*ev_it)) {
			// ignore user input events that occurred before the window was activated
			ev_it = events.erase(ev_it);
		} else {
			++ev_it;
		}
	}

	for(SDL_Event& event : events) {
		for(context& c : event_contexts) {
			c.add_staging_handlers();
		}

#ifdef MOUSE_TOUCH_EMULATION
		switch (event.type) {
			// TODO: Implement SDL_MULTIGESTURE. Some day.
			case SDL_MOUSEMOTION:
				if(event.motion.which != SDL_TOUCH_MOUSEID && event.motion.state == 0) {
					return;
				}

				if(event.motion.state & SDL_BUTTON(SDL_BUTTON_RIGHT))
				{
					// Events are given by SDL in draw space
					point c = video::game_canvas_size();

					// TODO: Check if SDL_FINGERMOTION is actually signaled for COMPLETE motions (I doubt, but tbs)
					SDL_Event touch_event;
					touch_event.type = SDL_FINGERMOTION;
					touch_event.tfinger.type = SDL_FINGERMOTION;
					touch_event.tfinger.timestamp = event.motion.timestamp;
					touch_event.tfinger.touchId = 1;
					touch_event.tfinger.fingerId = 1;
					touch_event.tfinger.dx = static_cast<float>(event.motion.xrel) / c.x;
					touch_event.tfinger.dy = static_cast<float>(event.motion.yrel) / c.y;
					touch_event.tfinger.x = static_cast<float>(event.motion.x) / c.x;
					touch_event.tfinger.y = static_cast<float>(event.motion.y) / c.y;
					touch_event.tfinger.pressure = 1;
					::SDL_PushEvent(&touch_event);

					event.motion.state = SDL_BUTTON(SDL_BUTTON_LEFT);
					event.motion.which = SDL_TOUCH_MOUSEID;
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				if(event.button.button == SDL_BUTTON_RIGHT)
				{
					event.button.button = SDL_BUTTON_LEFT;
					event.button.which = SDL_TOUCH_MOUSEID;

					// Events are given by SDL in draw space
					point c = video::game_canvas_size();

					SDL_Event touch_event;
					touch_event.type = (event.type == SDL_MOUSEBUTTONDOWN) ? SDL_FINGERDOWN : SDL_FINGERUP;
					touch_event.tfinger.type = touch_event.type;
					touch_event.tfinger.timestamp = event.button.timestamp;
					touch_event.tfinger.touchId = 1;
					touch_event.tfinger.fingerId = 1;
					touch_event.tfinger.dx = 0;
					touch_event.tfinger.dy = 0;
					touch_event.tfinger.x = static_cast<float>(event.button.x) / c.x;
					touch_event.tfinger.y = static_cast<float>(event.button.y) / c.y;
					touch_event.tfinger.pressure = 1;
					::SDL_PushEvent(&touch_event);

				}
				break;
			default:
				break;
		}
#endif

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

			// Size changed is called before resized.
			// We can ensure the video framebuffer is valid here.
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				LOG_DP << "events/SIZE_CHANGED "
					<< event.window.data1 << 'x' << event.window.data2;
				video::update_buffers(false);
				break;

			// Resized comes after size_changed.
			// Here we can trigger any watchers for resize events.
			// Video settings such as game_canvas_size() will be correct.
			case SDL_WINDOWEVENT_RESIZED:
				LOG_DP << "events/RESIZED "
					<< event.window.data1 << 'x' << event.window.data2;
				prefs::get().set_resolution(video::window_size());
				break;

			// Once everything has had a chance to respond to the resize,
			// an expose is triggered to display the changed content.
			case SDL_WINDOWEVENT_EXPOSED:
				LOG_DP << "events/EXPOSED";
				draw_manager::invalidate_all();
				break;

			case SDL_WINDOWEVENT_MAXIMIZED:
				LOG_DP << "events/MAXIMIZED";
				prefs::get().set_maximized(true);
				break;
			case SDL_WINDOWEVENT_RESTORED:
				LOG_DP << "events/RESTORED";
				prefs::get().set_maximized(prefs::get().fullscreen());
				break;
			case SDL_WINDOWEVENT_SHOWN:
			case SDL_WINDOWEVENT_MOVED:
				// Not used.
				break;
			}

			raise_window_event(event);

			// This event was just distributed, don't re-distribute.
			continue;

		case SDL_MOUSEMOTION: {
			// Always make sure a cursor is displayed if the mouse moves or if the user clicks
			cursor::set_focus(true);
			process_tooltip_strings(event.motion.x, event.motion.y);
			break;
		}

		case SDL_MOUSEBUTTONDOWN: {
			// Always make sure a cursor is displayed if the mouse moves or if the user clicks
			cursor::set_focus(true);
			if(event.button.button == SDL_BUTTON_LEFT || event.button.which == SDL_TOUCH_MOUSEID) {
				if(event.button.clicks == 2) {
					sdl::UserEvent user_event(DOUBLE_CLICK_EVENT, event.button.which, event.button.x, event.button.y);
					::SDL_PushEvent(reinterpret_cast<SDL_Event*>(&user_event));
				}
			}
			break;
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

		for(auto global_handler : event_contexts.front().handlers) {
			global_handler->handle_event(event);
		}

		if(event_contexts.empty() == false) {
			// As pump() can recurse, pretty much anything can happen here
			// including destroying handlers or the event context.
			size_t ec_index = event_contexts.size();
			context& c = event_contexts.back();
			handler_list& h = c.handlers;
			size_t h_size = h.size();
			for(auto it = h.begin(); it != h.end(); ++it) {
				// Pass the event on to the handler.
				(*it)->handle_event(event);
				// Escape if anything has changed.
				if(event_contexts.size() != ec_index) {
					LOG_EV << "ec size changed! bugging out";
					break;
				}
				if(h_size != h.size()) {
					LOG_EV << "h size changed! bugging out";
					break;
				}
			}
		}
	}

	// Inform the pump monitors that an events::pump() has occurred
	for(auto monitor : pump_monitors) {
		monitor->process();
	}
}

void draw()
{
	draw_manager::sparkle();
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
	point size = video::window_size();
	SDL_Event event;
	event.window.type = SDL_WINDOWEVENT;
	event.window.event = SDL_WINDOWEVENT_SIZE_CHANGED;
	event.window.windowID = 0; // We don't check this anyway... I think...
	event.window.data1 = size.x;
	event.window.data2 = size.y;

	raise_window_event(event);
}

void process_tooltip_strings(int mousex, int mousey)
{
	if(event_contexts.empty() == false) {
		for(auto handler : event_contexts.back().handlers) {
			handler->process_tooltip_string(mousex, mousey);
		}
	}
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

void call_in_main_thread(const std::function<void(void)>& f)
{
	if(is_in_main_thread()) {
		// nothing special to do if called from the main thread.
		f();
		return;
	}

	invoked_function_data fdata{f};

	SDL_Event sdl_event;
	sdl::UserEvent sdl_userevent(INVOKE_FUNCTION_EVENT, &fdata);

	sdl_event.type = INVOKE_FUNCTION_EVENT;
	sdl_event.user = sdl_userevent;

	SDL_PushEvent(&sdl_event);

	// Block until execution is complete in the main thread. Rethrows any exceptions.
	fdata.finished.get_future().wait();
}

} // end events namespace
