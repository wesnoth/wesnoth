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
#include "help/help_impl.hpp"

#include "gui/dialogs/help_browser.hpp"
#include "preferences/preferences.hpp"
#include "terrain/terrain.hpp"
#include "units/types.hpp"
#include "units/unit.hpp"

#include <boost/logic/tribool.hpp>
#include <cassert>

namespace help {

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

void show_terrain_description(const terrain_type& t)
{
	show_help(hidden_symbol(t.hide_help()) + terrain_prefix + t.id());
}

static void show_with_toplevel(const section& toplevel_sec, const std::string& show_topic)
{
	gui2::dialogs::help_browser::display(toplevel_sec, show_topic);
}

void show_help(const std::string& show_topic)
{
	const auto manager = help_manager::get_instance();
	show_with_toplevel(manager->regenerate(), show_topic);
}

//
// Help Manager Implementation
//

class help_manager::implementation
{
public:
	friend class help_manager;

	/**
	 * Regenerates the cached help topics if necessary.
	 *
	 * @returns the current toplevel section.
	 */
	const section& regenerate();

private:
	std::size_t last_num_encountered_units_{0};
	std::size_t last_num_encountered_terrains_{0};

	boost::tribool last_debug_state_{boost::indeterminate};

	/** The default toplevel. */
	section toplevel_section_;

	/** All sections and topics not referenced from the default toplevel. */
	section hidden_sections_;
};

const section& help_manager::implementation::regenerate()
{
	// Find all unit_types that have not been constructed yet and fill in the information
	// needed to create the help topics
	unit_types.build_all(unit_type::HELP_INDEXED);

	const auto& enc_units = prefs::get().encountered_units();
	const auto& enc_terrains = prefs::get().encountered_terrains();

	// More units or terrains encountered, or debug mode toggled
	if(boost::indeterminate(last_debug_state_)
		|| enc_units.size()    != last_num_encountered_units_
		|| enc_terrains.size() != last_num_encountered_terrains_
		|| last_debug_state_   != game_config::debug
	) {
		last_num_encountered_units_ = enc_units.size();
		last_num_encountered_terrains_ = enc_terrains.size();
		last_debug_state_ = game_config::debug;

		// Update the contents
		std::tie(toplevel_section_, hidden_sections_) = generate_contents();
	}

	return toplevel_section_;
}

help_manager::help_manager()
	: impl_(new help_manager::implementation)
{
}

/** Defined out-of-line so the implementation class is visible. */
help_manager::~help_manager() = default;

std::shared_ptr<help_manager> help_manager::get_instance()
{
	// Null if no manager instance exists
	std::shared_ptr instance = singleton_.lock();

	if(!instance) {
		instance.reset(new help_manager);
		singleton_ = instance;
	}

	assert(instance);
	return instance;
}

const section& help_manager::regenerate()
{
	return impl_->regenerate();
}

} // End namespace help.
