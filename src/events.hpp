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

	virtual bool requires_event_focus() const { return false; }

	virtual void process_help_string(int mousex, int mousey) {}

protected:
	handler();
	virtual ~handler();

private:
	int unicode_;
};

void focus_handler(const handler* ptr);
void cycle_focus();

bool has_focus(const handler* ptr);

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
	event_context(bool create=true);
	~event_context();

private:
	bool create_;
};

//causes events to be dispatched to all handler objects.
void pump();

void raise_process_event();
void raise_draw_event();
void raise_volatile_draw_event();
void raise_volatile_undraw_event();
void raise_help_string_event(int mousex, int mousey);
}

#endif
