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

namespace hotkey {
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

//object which will ensure that basic keyboard events like escape
//are handled properly for the duration of its lifetime
struct basic_handler : public events::handler {
	basic_handler(display& disp);

	void handle_event(const SDL_Event& event);

private:
	display& disp_;
};

void add_hotkeys(config& cfg);

void key_event(display& disp, const SDL_KeyboardEvent& event,
               command_executor* executor);

}

#endif
