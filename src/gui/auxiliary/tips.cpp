/*
   Copyright (C) 2010 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/auxiliary/tips.hpp"

#include "config.hpp"
#include "game_preferences.hpp"
#include "random.hpp"
#include "serialization/string_utils.hpp"

namespace gui2
{
game_tip::game_tip(const t_string& text, const t_string& source, const std::string& unit_filter)
	: text_(text)
	, source_(source)
	, unit_filter_(utils::split(unit_filter))
{
}

namespace tip_of_the_day
{
std::vector<game_tip> load(const config& cfg)
{
	std::vector<game_tip> result;

	for(const auto& tip : cfg.child_range("tip")) {
		result.emplace_back(tip["text"], tip["source"], tip["encountered_units"]);
	}

	return result;
}

std::vector<game_tip> shuffle(const std::vector<game_tip>& tips)
{
	std::vector<game_tip> result = tips;
	const std::set<std::string>& units = preferences::encountered_units();

	// Remove entries whose filters do not match from the tips list.
	const auto iter = std::remove_if(result.begin(), result.end(), [&units](const game_tip& tip) {
		const auto& filters = tip.unit_filter_;

		// Filter passes there's no filter at all or if every unit specified has already been
		// encountered in-game.
		const bool passes_filter = filters.empty()
			? true
			: std::all_of(filters.begin(), filters.end(), [&units](const std::string& u) {
				return units.find(u) != units.end();
			});

		return !passes_filter;
	});

	// Prune invalid entries and shrink the list.
	result.erase(iter, result.end());
	result.shrink_to_fit();

	// Shuffle the list.
	std::shuffle(result.begin(), result.end(), randomness::rng::default_instance());
	return result;
}

} // namespace tips

} // namespace gui2
