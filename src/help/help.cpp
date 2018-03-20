/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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

#define GETTEXT_DOMAIN "wesnoth-help"

#include "help/help.hpp"

#include "config.hpp"         // for config, etc
#include "gettext.hpp"        // for _
#include "gui/dialogs/help_browser.hpp"
#include "help/help_impl.hpp"
#include "preferences/game.hpp"
#include "terrain/terrain.hpp" // for terrain_type
#include "units/types.hpp"     // for unit_type, unit_type_data, etc
#include "units/unit.hpp"      // for unit

#include <cassert>   // for assert
#include <ostream>   // for basic_ostream, operator<<, etc
#include <vector>    // for vector, vector<>::iterator

namespace help
{
void show_unit_description(const unit& u)
{
	help::show_unit_description(u.type());
}

void show_terrain_description(const terrain_type& t)
{
	help::show_terrain_help(t.id(), t.hide_in_editor() || t.is_combined());
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
		help::show_variation_help(t.id(), var_id, hide_help);
	} else {
		help::show_unit_help(t.id(), t.show_variations_in_help(), hide_help);
	}
}

extern config dummy_cfg;

help_manager::help_manager(const config* cfg) //, gamemap *_map)
{
	game_cfg = cfg == nullptr ? &dummy_cfg : cfg;
	//	map = _map;
}

help_manager::~help_manager()
{
	game_cfg = nullptr;

	//	map = nullptr;
	default_toplevel.clear();
	hidden_sections.clear();

	// These last numbers must be reset so that the content is regenerated.
	// Upon next start.
	last_num_encountered_units = -1;
	last_num_encountered_terrains = -1;
}

void show_help(const std::string& show_topic)
{
	show_help(default_toplevel, show_topic);
}

void show_unit_help(const std::string& show_topic, bool has_variations, bool hidden)
{
	show_help(default_toplevel, hidden_symbol(hidden) + (has_variations ? ".." : "") + unit_prefix + show_topic);
}

void show_terrain_help(const std::string& show_topic, bool hidden)
{
	show_help(default_toplevel, hidden_symbol(hidden) + terrain_prefix + show_topic);
}

void show_variation_help(const std::string& unit, const std::string& variation, bool hidden)
{
	show_help(default_toplevel, hidden_symbol(hidden) + variation_prefix + unit + "_" + variation);
}

void init_help()
{
	// Find all unit_types that have not been constructed yet and fill in the information
	// needed to create the help topics
	unit_types.build_all(unit_type::HELP_INDEXED);

	if(preferences::encountered_units().size() != std::size_t(last_num_encountered_units) ||
	   preferences::encountered_terrains().size() != std::size_t(last_num_encountered_terrains) ||
	   last_debug_state != game_config::debug || last_num_encountered_units < 0)
	{
		// More units or terrains encountered, update the contents.
		last_num_encountered_units = preferences::encountered_units().size();
		last_num_encountered_terrains = preferences::encountered_terrains().size();
		last_debug_state = game_config::debug;

		generate_contents();
	}
}

void show_help(const section& toplevel_sec, const std::string& show_topic)
{
	gui2::dialogs::help_browser::display(toplevel_sec, show_topic);
}

} // End namespace help.
