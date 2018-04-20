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

#include "help/utils.hpp"

#include "display.hpp"
#include "game_config.hpp"
#include "game_config_manager.hpp"
#include "gettext.hpp"
#include "help/constants.hpp"
#include "help/section.hpp"
#include "help/topic.hpp"
#include "hotkey/hotkey_command.hpp"
#include "map/map.hpp"
#include "preferences/game.hpp"
#include "terrain/type_data.hpp"
#include "units/types.hpp"

#include <iostream>

namespace help
{
/** Implementation helpers. */
namespace
{
} // end anon namespace

bool string_less::operator()(const std::string& s1, const std::string& s2) const
{
	return translation::compare(s1, s2) < 0;
}

std::string hidden_symbol(bool hidden)
{
	return hidden ? "." : "";
}

bool is_visible_id(const std::string& id)
{
	return id.empty() || id[0] != '.';
}

bool is_valid_id(const std::string& id)
{
	if(id == "toplevel") {
		return false;
	}

	if(id.compare(0, unit_prefix.length(), unit_prefix) == 0
			|| id.compare(hidden_symbol().length(), unit_prefix.length(), unit_prefix) == 0) {
		return false;
	}

	if(id.compare(0, ability_prefix.length(), ability_prefix) == 0) {
		return false;
	}

	if(id.compare(0, weapon_special_prefix.length(), weapon_special_prefix) == 0) {
		return false;
	}

	if(id == "hidden") {
		return false;
	}

	return true;
}

UNIT_DESCRIPTION_TYPE description_type(const unit_type& type)
{
	if(game_config::debug || preferences::show_all_units_in_help() || hotkey::is_scope_active(hotkey::SCOPE_EDITOR)) {
		return FULL_DESCRIPTION;
	}

	const std::set<std::string>& encountered_units = preferences::encountered_units();
	if(encountered_units.find(type.id()) != encountered_units.end()) {
		return FULL_DESCRIPTION;
	}

	return NO_DESCRIPTION;
}

std::string escape(const std::string& s)
{
	return utils::escape(s, "'\\");
}

ter_data_cache load_terrain_types_data()
{
	if(display* d = display::get_singleton()) {
		return d->get_map().tdata();
	} else if(game_config_manager* g = game_config_manager::get()) {
		return g->terrain_types();
	} else {
		return ter_data_cache();
	}
}

} // namespace help
