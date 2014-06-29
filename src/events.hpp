/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef EVENTS_HPP_INCLUDED
#define EVENTS_HPP_INCLUDED

#include "SDL_events.h"
#include "SDL_version.h"
#include <vector>

//our user-defined double-click event type
#define DOUBLE_CLICK_EVENT SDL_USEREVENT
#define TIMER_EVENT (SDL_USEREVENT + 1)
#define HOVER_REMOVE_POPUP_EVENT (SDL_USEREVENT + 2)
#define DRAW_EVENT (SDL_USEREVENT + 3)
#define CLOSE_WINDOW_EVENT (SDL_USEREVENT + 4)
#define SHOW_HELPTIP_EVENT (SDL_USEREVENT + 5)

namespace events
{

//any classes that derive from this class will automatically
//receive sdl events through the handle function for their lifetime,
//while the event context they were created in is active.
//
//NOTE: an event_context object must be initialized before a handler object
//can be initialized, and the event_context must be destroyed after
//the handler is destroyed.
class handler
{
public:
	virtual void handle_event(const SDL_Event& event) = 0;
	virtual void process_event() {}
	virtual void draw() {}

	virtual void volatile_draw() {}
	virtual void volatile_undraw() {}

	virtual bool requires_event_focus(const SDL_Event * = NULL) const { return false; }

	virtual void process_help_string(int /*mousex*/, int /*mousey*/) {}
	virtual void process_tooltip_string(int /*mousex*/, int /*mousey*/) {}

	virtual void join(); /*joins the current event context*/
	virtual void leave(); /*leave the event context*/

protected:
	handler(const bool auto_join=true);
	virtual ~handler();
	virtual std::vector<handler*> handler_members()
	{
		return std::vector<handler*>();
	}

private:
	int unicode_;
	bool has_joined_;
};

void focus_handler(const handler* ptr);
void cycle_focus();

bool has_focus(const handler* ptr, const SDL_Event* event);

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

//causes events to be dispatched to all handler objects.
void pump();

struct pump_info {
	pump_info() : resize_dimensions(), ticks_(0) {}
	std::pair<int,int> resize_dimensions;
	int ticks(unsigned *refresh_counter=NULL, unsigned refresh_rate=1);
private:
	int ticks_; //0 if not calculated
};

class pump_monitor {
//pump_monitors receive notification after an events::pump() occurs
public:
	pump_monitor();
	virtual ~pump_monitor();
	virtual void process(pump_info& info) = 0;
};

void raise_process_event();
void raise_draw_event();
void raise_volatile_draw_event();
void raise_volatile_undraw_event();
void raise_help_string_event(int mousex, int mousey);


/**
 * Is the event an input event?
 *
 * @returns                       Whether or not the event is an input event.
 */
bool is_input(const SDL_Event& event);

/** Discards all input events. */
void discard_input();

}

typedef std::vector<events::handler*> handler_vector;

#if ! SDL_VERSION_ATLEAST(2,0,0)

/**
 * Removes events from the queue.
 *
 * This emulates the function available in SDL 2.0.
 *
 * @param type                    The type of event to flush.
 */
void SDL_FlushEvent(Uint32 type);

#endif

#endif
