/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Routines for showing the help-dialog.
 */

#include "help/help.hpp"

#include "help/constants.hpp"
#include "help/manager.hpp"
#include "help/utils.hpp"
#include "terrain/terrain.hpp"
#include "units/types.hpp"
#include "units/unit.hpp"

#include <cassert>

namespace help
{
/** The help manager. What else would it be? */
static help_manager manager;

void show_help(const std::string& show_topic)
{
	manager.open_help_browser_to(show_topic);
}

void reset()
{
	manager.reset_contents();
}

void show_unit_description(const unit& u)
{
	show_unit_description(u.type());
}

void show_unit_description(const unit_type& t)
{
	std::string var_id = t.get_cfg()["variation_id"].str();
	if(var_id.empty()) {
		var_id = t.get_cfg()["variation_name"].str();
	}

	bool hide_help = t.hide_help();
	bool use_variation = false;

	if(!var_id.empty()) {
		const unit_type* parent = unit_types.find(t.id());
		assert(parent);

		if(hide_help) {
			hide_help = parent->hide_help();
		} else {
			use_variation = true;
		}
	}

	if(use_variation) {
		show_variation_help(t.id(), var_id, hide_help);
	} else {
		show_unit_help(t.id(), t.show_variations_in_help(), hide_help);
	}
}

void show_unit_help(const std::string& show_topic, bool has_variations, bool hidden)
{
	show_help(hidden_symbol(hidden) + (has_variations ? ".." : "") + unit_prefix + show_topic);
}

void show_variation_help(const std::string& unit, const std::string& variation, bool hidden)
{
	show_help(hidden_symbol(hidden) + variation_prefix + unit + "_" + variation);
}

void show_terrain_description(const terrain_type& t)
{
	show_terrain_help(t.id(), t.hide_in_editor() || t.is_combined());
}

void show_terrain_help(const std::string& show_topic, bool hidden)
{
	show_help(hidden_symbol(hidden) + terrain_prefix + show_topic);
}

} // End namespace help.
