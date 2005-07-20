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

#include "../global.hpp"

#include "scenario_editor.hpp"

scenario_editor::scenario_editor(game_data& gameinfo, const config& game_config, CVideo& video)
: gameinfo_(gameinfo), game_config_(game_config), video_(video), disp_(NULL), map_(NULL), status_(NULL)
{
}

void scenario_editor::handle_event(const SDL_Event& event)
{
}

void scenario_editor::cycle_units()
{
}

void scenario_editor::goto_leader()
{
}

void scenario_editor::undo()
{
}

void scenario_editor::redo()
{
}

void scenario_editor::terrain_table()
{
}

void scenario_editor::attack_resistance()
{
}

void scenario_editor::unit_description()
{
}

void scenario_editor::rename_unit()
{
}

void scenario_editor::save_game()
{
}

void scenario_editor::toggle_grid()
{
}

void scenario_editor::status_table()
{
}

void scenario_editor::create_unit()
{
}

void scenario_editor::preferences()
{
}

void scenario_editor::objectives()
{
}

void scenario_editor::unit_list()
{
}

void scenario_editor::label_terrain()
{
}


void scenario_editor::edit_set_terrain()
{
}