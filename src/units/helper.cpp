/*
	Copyright (C) 2003 - 2024
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
 * Support functions for dealing with units.
 */

#include "units/unit.hpp"
#include "units/helper.hpp"
#include "units/types.hpp"
#include "play_controller.hpp"
#include "serialization/markup.hpp"

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

/**
 * Maps resistance <= -60 (resistance value <= -60%) to intense red.
 * Maps resistance >= 60 (resistance value >= 60%) to intense green.
 * Intermediate values are affinely mapped to the red-to-green scale,
 * with 0 (0%) being mapped to yellow.
 * Compare attack_info_percent_color() in reports.cpp.
 */
std::string resistance_color(const int resistance)
{
	// Passing false to select the more saturated red-to-green scale.
	return game_config::red_to_green(50.0 + resistance * 5.0 / 6.0, false).to_hex_string();
}

static std::string unit_level_tooltip(const int level, const std::vector<std::string> &adv_to_types, const std::vector<config> &adv_to_mods)
{
	std::ostringstream tooltip;
	tooltip << _("Level: ") << markup::bold(level) << "\n";
	const bool has_advancements = !adv_to_types.empty() || !adv_to_mods.empty();
	if(has_advancements) {
		tooltip << _("Advancements:") << "\n\t";
		if(!adv_to_types.empty())
			tooltip << markup::bold(utils::join(adv_to_types, "\n\t"));
		if(!adv_to_mods.empty()) {
			if(!adv_to_types.empty())
				tooltip << "\n\t";
			std::vector<std::string> descriptions;
			for(const config& adv : adv_to_mods)
				descriptions.push_back(adv["description"].str());
			tooltip << markup::bold(utils::join(descriptions, "\n\t"));
		}
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

std::string maybe_inactive(const std::string& str, bool active)
{
	return (active ? str : markup::span_color(font::INACTIVE_COLOR, str));
}

std::string format_cost_string(int unit_recall_cost, const int team_recall_cost)
{
	std::stringstream str;

	if(unit_recall_cost < 0) {
		unit_recall_cost = team_recall_cost;
	}

	if(unit_recall_cost > team_recall_cost) {
		str << markup::img("themes/gold.png") << markup::span_color(font::BAD_COLOR, unit_recall_cost);
	} else if(unit_recall_cost == team_recall_cost) {
		str << markup::img("themes/gold.png") << unit_recall_cost;
	} else if(unit_recall_cost < team_recall_cost) {
		str << markup::img("themes/gold.png~GS()") << markup::span_color(font::GREEN_COLOR, unit_recall_cost);
	}

	return str.str();
}

std::string format_cost_string(int unit_cost)
{
	return formatter() << markup::img("themes/gold.png") << unit_cost;
}

std::string format_level_string(const int level, bool recallable)
{
	if(!recallable) {
		// Same logic as when recallable, but always in inactive_color.
		return markup::span_color(font::INACTIVE_COLOR,
			(level < 2 ? std::to_string(level) : markup::bold(level)));
	} else if(level < 1) {
		return markup::span_color(font::INACTIVE_COLOR, level);
	} else if(level == 1) {
		return std::to_string(level);
	} else if(level == 2) {
		return markup::bold(level);
	} else if(level == 3) {
		return markup::span_color(color_t(0xe2, 0xb7, 0x76), markup::bold(level));
	} else {
		return markup::span_color(color_t(0xdd, 0x66, 0x00), markup::bold(level));
	}
}

std::string format_movement_string(const int moves_left, const int moves_max)
{
	if(moves_left == 0) {
		return markup::span_color(font::BAD_COLOR, moves_left, "/", moves_max);
	} else if(moves_left > moves_max) {
		return markup::span_color(font::YELLOW_COLOR, moves_left, "/", moves_max);
	} else {
		return markup::span_color(font::GREEN_COLOR, moves_left, "/", moves_max);
	}
}

}
