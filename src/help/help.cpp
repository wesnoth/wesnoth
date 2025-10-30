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
#include "gui/widgets/settings.hpp"
#include "help/help_impl.hpp"           // for hidden_symbol, toplevel, etc
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
/**
 * Open a help dialog using a specified toplevel.
 *
 * This would allow for complete customization of the contents, although not in a
 * very easy way. It's used as the internal implementation of the other help*
 * functions.
 */
void show_with_toplevel(const section& toplevel, const std::string& show_topic = "");

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

help_manager::~help_manager()
{
	default_toplevel.clear();
	hidden_sections.clear();
	// These last numbers must be reset so that the content is regenerated.
	// Upon next start.
	last_num_encountered_units = -1;
	last_num_encountered_terrains = -1;
}

/**
 * Open the help browser, show topic with id show_topic.
 *
 * If show_topic is the empty string, the default topic will be shown.
 */
void show_help(const std::string& show_topic)
{
	auto manager = help_manager::get_instance();
	show_with_toplevel(default_toplevel, show_topic);
}

void init_help() {
	// Find all unit_types that have not been constructed yet and fill in the information
	// needed to create the help topics
	unit_types.build_all(unit_type::HELP_INDEXED);

	auto& enc_units = prefs::get().encountered_units();
	auto& enc_terrains = prefs::get().encountered_terrains();
	if(enc_units.size() != std::size_t(last_num_encountered_units) ||
		enc_terrains.size() != std::size_t(last_num_encountered_terrains) ||
		last_debug_state != game_config::debug ||
		last_num_encountered_units < 0) {
		// More units or terrains encountered, update the contents.
		last_num_encountered_units = enc_units.size();
		last_num_encountered_terrains = enc_terrains.size();
		last_debug_state = game_config::debug;
		generate_contents();
	}
}

/**
 * Open a help dialog using a toplevel other than the default.
 *
 * This allows for complete customization of the contents, although not in a
 * very easy way.
 */
void show_with_toplevel(const section &toplevel_sec, const std::string& show_topic)
{
	gui2::dialogs::help_browser::display(toplevel_sec, show_topic);
}

} // End namespace help.
