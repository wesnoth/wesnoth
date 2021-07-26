/*
	Copyright (C) 2003 - 2021
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
 * Support functions for dealing with units.
 */

#include "units/unit.hpp"
#include "units/helper.hpp"
#include "units/types.hpp"
#include "resources.hpp"
#include "play_controller.hpp"
#include "saved_game.hpp"

namespace unit_helper {

int number_of_possible_advances(const unit &u)
{
	return u.advances_to().size() + u.get_modification_advances().size();
}

bool will_certainly_advance(const unit_map::iterator &u)
{
	if(!u.valid()) {
		return false;
	}
	return u->advances() && number_of_possible_advances(*u) > 0;
}

std::string resistance_color(const int resistance)
{
	if (resistance < 0)
		return std::string("#FF0000");

	if (resistance <= 20)
		return std::string("#FFFF00");

	if (resistance <= 40)
		return std::string("#FFFFFF");

	return std::string("#00FF00");
}

static std::string unit_level_tooltip(const int level, const std::vector<std::string> &adv_to_types, const std::vector<config> &adv_to_mods)
{
	std::ostringstream tooltip;
	tooltip << _("Level: ") << "<b>" << level << "</b>\n";
	const bool has_advancements = !adv_to_types.empty() || !adv_to_mods.empty();
	if(has_advancements) {
		tooltip << _("Advancements:") << "\n<b>\t";
		if(!adv_to_types.empty())
			tooltip << utils::join(adv_to_types, "\n\t");
		if(!adv_to_mods.empty()) {
			if(!adv_to_types.empty())
				tooltip << "\n\t";
			std::vector<std::string> descriptions;
			for(const config& adv : adv_to_mods)
				descriptions.push_back(adv["description"].str());
			tooltip << utils::join(descriptions, "\n\t");
		}
		tooltip << "</b>";
	} else {
		tooltip << _("No advancement");
	}
	return tooltip.str();
}

std::string unit_level_tooltip(const unit &u)
{
	return unit_level_tooltip(u.level(), u.advances_to_translated(), u.get_modification_advances());
}

std::string unit_level_tooltip(const unit_type &type)
{
	const auto mod_adv_iters = type.modification_advancements();
	const std::vector<config> mod_advancements(mod_adv_iters.begin(), mod_adv_iters.end());

	return unit_level_tooltip(type.level(), type.advances_to(), mod_advancements);
}

}
