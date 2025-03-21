/*
	Copyright (C) 2010 - 2025
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "preferences/preferences.hpp"
#include "random.hpp"
#include "serialization/string_utils.hpp"
#include "utils/general.hpp"

namespace gui2
{
game_tip::game_tip(const config& cfg)
	: text(cfg["text"].t_str())
	, source(cfg["source"].t_str())
	, unit_filter(utils::split(cfg["encountered_units"]))
{
}

namespace tip_of_the_day
{
std::vector<game_tip> load(const config& cfg)
{
	const auto range = cfg.child_range("tip");
	return { range.begin(), range.end() };
}

std::vector<game_tip> shuffle(const std::vector<game_tip>& tips)
{
	std::vector<game_tip> result = tips;
	const std::set<std::string>& units = prefs::get().encountered_units();

	// Remove entries whose filters do not match from the tips list.
	utils::erase_if(result, [&units](const game_tip& tip) {
		const auto& must_have_seen = tip.unit_filter;

		// No units to encounter, tip is always visible.
		if(must_have_seen.empty()) {
			return false;
		}

		// At least one given unit type must have been encountered.
		return std::none_of(must_have_seen.begin(), must_have_seen.end(),
			[&units](const std::string& u) { return utils::contains(units, u); });
	});

	// Shuffle the list.
	std::shuffle(result.begin(), result.end(), randomness::rng::default_instance());
	return result;
}

} // namespace tip_of_the_day

} // namespace gui2
