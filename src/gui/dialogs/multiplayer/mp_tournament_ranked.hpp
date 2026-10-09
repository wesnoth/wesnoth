/*
	Copyright (C) 2026
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
*/

#pragma once

#include "mp_game_settings.hpp"

#include <string>

namespace gui2::dialogs
{

/** Translate locale-independent competitive-play message IDs from wesnothd. */
std::string localized_competitive_message(const std::string& message_id);

/**
 * Formats the tournament match label used by both game setup and the lobby.
 * Tournament and organizer-defined names remain literal; only the fixed
 * labels for round and game numbers are localized.
 */
std::string format_tournament_match_label(
	const std::string& tournament_name,
	const std::string& phase_name,
	const std::string& group_name,
	const std::string& round_number,
	const std::string& game_number);

/** Copy the selected Tournament Manager entry into the create-game settings. */
void apply_tournament_settings(config& settings, const mp_tournament_info* tournament);
void apply_tournament_settings(mp_game_settings& settings, const mp_tournament_info* tournament);

} // namespace gui2::dialogs
