/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef HOTKEYS_HPP_INCLUDED
#define HOTKEYS_HPP_INCLUDED

#include "config.hpp"
#include "display.hpp"
#include "events.hpp"
#include "key.hpp"

//the hotkey system allows hotkey definitions to be loaded from
//configuration objects, and then detect if a keyboard event
//refers to a hotkey command being executed.
namespace hotkey {

//function to load a hotkey configuration object. hotkey configuration objects
//are a list of nodes that look like:
//[hotkey]
//command='cmd'
//key='k'
//ctrl=(yes|no)
//alt=(yes|no)
//shift=(yes|no)
//[/hotkey]
//where 'cmd' is a command name, and 'k' is a key. see hotkeys.cpp for the
//valid command names.
void add_hotkeys(config& cfg);

//abstract base class for objects that implement the ability
//to execute hotkey commands.
class command_executor
{
public:
	virtual void cycle_units() = 0;
	virtual void end_turn() = 0;
	virtual void goto_leader() = 0;
	virtual void end_unit_turn() = 0;
	virtual void undo() = 0;
	virtual void redo() = 0;
	virtual void terrain_table() = 0;
	virtual void attack_resistance() = 0;
	virtual void unit_description() = 0;
	virtual void save_game() = 0;
	virtual void toggle_grid() = 0;
	virtual void status_table() = 0;
	virtual void recall() = 0;
	virtual void recruit() = 0;
};

//function to be called every time a key event is intercepted. Will
//call the relevant function in executor if the keyboard event is
//not NULL. Also handles some events in the function itself, and so
//is still meaningful to call with executor=NULL
void key_event(display& disp, const SDL_KeyboardEvent& event,
               command_executor* executor);

//object which will ensure that basic keyboard events like escape
//are handled properly for the duration of its lifetime
struct basic_handler : public events::handler {
	basic_handler(display& disp);

	void handle_event(const SDL_Event& event);

private:
	display& disp_;
};

}

#endif
