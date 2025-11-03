/*
	Copyright (C) 2003 - 2025
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

/**
 * @file
 * Routines for showing the help-dialog.
 */

#define GETTEXT_DOMAIN "wesnoth-help"

#include "help/help.hpp"

#include "config.hpp"                   // for config, etc
#include "preferences/preferences.hpp"
#include "game_config_manager.hpp"
#include "gettext.hpp"                  // for _
#include "gui/dialogs/help_browser.hpp"
#include "log.hpp"                      // for LOG_STREAM, log_domain
#include "terrain/terrain.hpp"          // for terrain_type
#include "units/unit.hpp"               // for unit
#include "units/types.hpp"              // for unit_type, unit_type_data, etc

#include <cassert>                      // for assert
#include <algorithm>                    // for min
#include <vector>                       // for vector, vector<>::iterator


static lg::log_domain log_display("display");
#define WRN_DP LOG_STREAM(warn, log_display)

static lg::log_domain log_help("help");
#define ERR_HELP LOG_STREAM(err, log_help)

namespace help {

void show_terrain_description(const terrain_type& t)
{
	show_help(hidden_symbol(t.hide_help()) + terrain_prefix + t.id());
}

std::string get_unit_type_help_id(const unit_type& t)
{
	std::string var_id = t.variation_id();
	if(var_id.empty()) {
		var_id = t.variation_name();
	}
	bool hide_help = t.hide_help();
	bool use_variation = false;

	if(!var_id.empty()) {
		const unit_type* parent = unit_types.find(t.id());
		assert(parent);
		if (hide_help) {
			hide_help = parent->hide_help();
		} else {
			use_variation = true;
		}
	}

	if(use_variation) {
		return hidden_symbol(hide_help) + variation_prefix + t.id() + "_" + var_id;
	} else {
		return hidden_symbol(hide_help) + (t.show_variations_in_help() ? ".." : "") + unit_prefix + t.id();
	}
}

void show_unit_description(const unit& u)
{
	show_unit_description(u.type());
}

void show_unit_description(const unit_type& t)
{
	show_help(get_unit_type_help_id(t));
}

std::shared_ptr<help_manager> help_manager::get_instance()
{
	if(!singleton_) {
		singleton_.reset(new help_manager);
	}

	assert(singleton_);
	return singleton_;
}

void help_manager::verify_cache()
{
	// Find all unit_types that have not been constructed yet and fill in the information
	// needed to create the help topics
	unit_types.build_all(unit_type::HELP_INDEXED);

	const auto& enc_units = prefs::get().encountered_units();
	const auto& enc_terrains = prefs::get().encountered_terrains();

	if(enc_units.size() != std::size_t(last_num_encountered_units_) ||
		enc_terrains.size() != std::size_t(last_num_encountered_terrains_) ||
		last_debug_state_ != game_config::debug ||
		last_num_encountered_units_ < 0
	) {
		// More units or terrains encountered
		last_num_encountered_units_ = enc_units.size();
		last_num_encountered_terrains_ = enc_terrains.size();
		last_debug_state_ = game_config::debug;

		// Update the contents
		std::tie(default_toplevel_, hidden_sections_) = generate_contents();
	}
}

void show_with_toplevel(const section& toplevel_sec, const std::string& show_topic)
{
	gui2::dialogs::help_browser::display(toplevel_sec, show_topic);
}

void show_help(const std::string& show_topic)
{
	auto manager = help_manager::get_instance();
	manager->verify_cache();
	show_with_toplevel(manager->toplevel_section(), show_topic);
}

} // End namespace help.
