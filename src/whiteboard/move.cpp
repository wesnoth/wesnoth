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
#include "unit_display.hpp"

namespace wb {

const double move::ALPHA_HIGHLIGHT = 2.0;
const double move::ALPHA_NORMAL = 0.2;

move::move(unit& subject, const map_location& source_hex, const map_location& target_hex,
		boost::shared_ptr<arrow> arrow,	boost::shared_ptr<unit> fake_unit)
: unit_(subject),
  orig_hex_(source_hex),
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
		//resources::screen->invalidate(orig_hex_);
		resources::screen->invalidate(fake_unit_->get_location());
		unit_.set_standing();
	}
}

//FIXME: move this out of here if it ends up being used only once
static team& get_current_team()
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
	arrow_->set_alpha(ALPHA_HIGHLIGHT);

	static const bool show_move = false;
	map_location next_unit;
	::move_unit(NULL, arrow_->get_path(), &recorder, resources::undo_stack, show_move, &next_unit,
			get_current_team().auto_shroud_updates());
	// next_unit now contains the final unit location
	// if that isn't needed, pass NULL rather than &next_unit
}

modifier_ptr move::apply_temp_modifier(unit_map& unit_map)
{
	//TODO: properly handle movement points

	assert(unit_.get_location() == orig_hex_);
	DBG_WB << "Adding temp unit mover for unit " << unit_.name() << " [" << unit_.underlying_id() << "] "
			<< " from (" << orig_hex_ << ") to (" << dest_hex_ <<")\n";
	modifier_ptr modifier(new temporary_unit_mover(unit_map, orig_hex_, dest_hex_));
	return modifier;
}

bool move::is_related_to(const map_location& hex) const
{
	bool is_related = arrow_->path_contains(hex);
	return is_related;
}

bool move::is_related_to(const unit& unit) const
{
	bool is_related = &unit_ == &unit;
	return is_related;
}

} // end namespace wb
