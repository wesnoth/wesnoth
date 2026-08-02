/*
	Copyright (C) 2026
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
*/

#pragma once

#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "serialization/string_utils.hpp"

#include <string>
#include <vector>

namespace gui2::dialogs
{

/**
 * Formats the tournament match label used by both game setup and the lobby.
 * Tournament and organizer-defined names remain literal; only the fixed
 * labels for round and game numbers are localized.
 */
inline std::string format_tournament_match_label(
	const std::string& tournament_name,
	const std::string& phase_name,
	const std::string& group_name,
	const std::string& round_number,
	const std::string& game_number)
{
	std::vector<std::string> components;
	if(!phase_name.empty()) {
		components.push_back(phase_name);
	}
	if(!group_name.empty()) {
		components.push_back(group_name);
	}
	if(!round_number.empty()) {
		components.push_back(VGETTEXT("Round $round", {{"round", round_number}}));
	}
	if(!game_number.empty()) {
		components.push_back(VGETTEXT("Game $game", {{"game", game_number}}));
	}

	return components.empty() ? tournament_name : tournament_name + " — " + utils::join(components, " — ");
}

} // namespace gui2::dialogs
