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

#pragma once

#include <SDL2/SDL_events.h>
#include <vector>
#include <list>
#include <functional>

//our user-defined double-click event type
#define DOUBLE_CLICK_EVENT SDL_USEREVENT
#define TIMER_EVENT (SDL_USEREVENT + 1)
#define HOVER_REMOVE_POPUP_EVENT (SDL_USEREVENT + 2)
#define DRAW_EVENT (SDL_USEREVENT + 3)
#define CLOSE_WINDOW_EVENT (SDL_USEREVENT + 4)
#define SHOW_HELPTIP_EVENT (SDL_USEREVENT + 5)
#define INVOKE_FUNCTION_EVENT (SDL_USEREVENT + 6)

namespace events
{

class sdl_handler;

typedef std::list<sdl_handler*> handler_list;

class context
{
public:
	context() :
		handlers(),
		focused_handler(handlers.end()),
		staging_handlers()
	{
	}

	~context();

	context(const context&) = delete;

	void add_handler(sdl_handler* ptr);
	/** Returns true if @a ptr is found in either the handlers or staging_handlers lists */
	bool has_handler(const sdl_handler* ptr) const;
	bool remove_handler(sdl_handler* ptr);
	void cycle_focus();
	void set_focus(const sdl_handler* ptr);
	void add_staging_handlers();

	handler_list handlers;
	handler_list::iterator focused_handler;
	std::vector<sdl_handler*> staging_handlers;
};

//any classes that derive from this class will automatically
//receive sdl events through the handle function for their lifetime,
//while the event context they were created in is active.
//
//NOTE: an event_context object must be initialized before a handler object
//can be initialized, and the event_context must be destroyed after
//the handler is destroyed.
class sdl_handler
{
friend class context;
public:
	virtual void handle_event(const SDL_Event& event) = 0;
	virtual void handle_window_event(const SDL_Event&) {};
	virtual void process_event() {}

	virtual bool requires_event_focus(const SDL_Event * = nullptr) const { return false; }

	virtual void process_tooltip_string(int /*mousex*/, int /*mousey*/) {}

	virtual void join(); /*joins the current event context*/
	virtual void join(context &c); /*joins the specified event context*/
	virtual void join_same(sdl_handler* parent); /*joins the same event context as the parent is already associated with */
	virtual void leave(); /*leave the event context*/

	virtual void join_global(); /*join the global event context*/
	virtual void leave_global(); /*leave the global event context*/

	virtual bool has_joined() { return has_joined_;}
	virtual bool has_joined_global() { return has_joined_global_;}

	/**
	 * Moving would require two instances' context membership to be handled,
	 * it's simpler to delete these and require the two instances to be
	 * separately constructed / destructed.
	 */
	sdl_handler &operator=(sdl_handler &&) = delete;
	sdl_handler(sdl_handler &&) = delete;

protected:
	sdl_handler(const bool auto_join=true);
	sdl_handler(const sdl_handler &);
	sdl_handler &operator=(const sdl_handler &);
	virtual ~sdl_handler();
	virtual std::vector<sdl_handler*> handler_members()
	{
		return std::vector<sdl_handler*>();
	}

private:
	bool has_joined_;
	bool has_joined_global_;
};

void focus_handler(const sdl_handler* ptr);

bool has_focus(const sdl_handler* ptr, const SDL_Event* event);

// whether the currently executing thread is the main thread.
bool is_in_main_thread();

void call_in_main_thread(const std::function<void (void)>& f);

//event_context objects control the handler objects that SDL events are sent
//to. When an event_context is created, it will become the current event context.
//event_context objects MUST be created in LIFO ordering in relation to each other,
//and in relation to handler objects. That is, all event_context objects should be
//created as automatic/stack variables.
//
//handler objects need not be created as automatic variables (e.g. you could put
//them in a vector) however you must guarantee that handler objects are destroyed
//before their context is destroyed
struct event_context
{
	event_context();
	~event_context();
};

/** Process all events currently in the queue. */
void pump();

/** Trigger a draw cycle. */
void draw();

/** pump() then immediately draw() */
inline void pump_and_draw() { pump(); draw(); }
// TODO: draw_manager - should this also raise_process_event? Some things do some don't

class pump_monitor {
//pump_monitors receive notification after an events::pump() occurs
public:
	pump_monitor();
	virtual ~pump_monitor();
	virtual void process() = 0;
};

void raise_process_event();
void raise_resize_event();
/**
 * Triggered by mouse-motion, sends the cursor position to all handlers to
 * check whether a tooltip should be shown.
 */
void process_tooltip_strings(int mousex, int mousey);


/**
 * Is the event an input event?
 *
 * @returns                       Whether or not the event is an input event.
 */
bool is_input(const SDL_Event& event);

/** Discards all input events. */
void discard_input();

}

typedef std::vector<events::sdl_handler*> sdl_handler_vector;
