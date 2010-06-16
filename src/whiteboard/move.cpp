/* $Id$ */
/*
 Copyright (C) 2010 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 2
 or at your option any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.

 See the COPYING file for more details.
 */

/**
 * @file move.cpp
 */

#include "move.hpp"

#include "visitor.hpp"

#include "actions.hpp"
#include "arrow.hpp"
#include "unit.hpp"
#include "config.hpp"
#include "play_controller.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "team.hpp"
#include "game_display.hpp"

namespace wb {

move::move(unit& subject, const map_location& target_hex, boost::shared_ptr<arrow> arrow,
		boost::shared_ptr<unit> fake_unit)
: unit_(subject),
  orig_hex_(subject.get_location()),
  dest_hex_(target_hex),
  arrow_(arrow),
  fake_unit_(fake_unit)
{
}

move::~move()
{
	if (resources::screen)
	{
		if (fake_unit_)
		{
			resources::screen->remove_temporary_unit(fake_unit_.get());
		}
	}
}

team& get_current_team()
{
	int current_side = resources::controller->current_side();
	team& current_team = (*resources::teams)[current_side - 1];
	return current_team;
}

void move::accept(visitor& v)
{
	v.visit_move(shared_from_this());
}

void move::execute()
{
	static bool show_move = true;
	map_location* next_unit = NULL; //FIXME: Not sure what to do with this variable as of now
	::move_unit(NULL, arrow_->get_path(), &recorder, resources::undo_stack, show_move, next_unit,
			get_current_team().auto_shroud_updates());
}

modifier_ptr move::apply_temp_modifier(unit_map& unit_map)
{
	assert(unit_.get_location() == orig_hex_);
	DBG_WB << "Adding temp unit mover for unit " << unit_.name() << " [" << unit_.underlying_id() << "] "
			<< " from (" << orig_hex_ << ") to (" << dest_hex_ <<")\n";
	modifier_ptr modifier(new temporary_unit_mover(unit_map, orig_hex_, dest_hex_));
	return modifier;
}

} // end namespace wb
