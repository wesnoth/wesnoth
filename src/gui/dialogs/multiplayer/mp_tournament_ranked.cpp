/*
	Copyright (C) 2026
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/multiplayer/mp_tournament_ranked.hpp"

#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "serialization/string_utils.hpp"
#include "tstring.hpp"

#include <vector>

namespace gui2::dialogs
{

std::string localized_competitive_message(const std::string& message_id)
{
	// Server messages use stable IDs so localization is performed by each client.
	if(message_id == "ranked_access_required") {
		return _("You are not enabled for ranked matches. Enable ranked matches in your user profile: https://tournament.wesnoth.org/help/getting-started");
	}
	if(message_id == "tournament_join_denied") {
		return _("You are not allowed to join this tournament game.");
	}
	if(message_id == "tournament_creation_denied") {
		return _("You are not a participant in this tournament.");
	}
	if(message_id == "tournament_mode_mismatch") {
		return _("The selected tournament determines whether this game is ranked.");
	}
	if(message_id == "side_assignment_denied") {
		return _("That player is not eligible for this ranked or tournament game.");
	}
	if(message_id == "tournament_game_already_recorded") {
		return _("This tournament game already has a competitive game recorded and cannot be started again.");
	}
	if(message_id == "tournament_team_mismatch") {
		return _("The tournament team composition does not match the Tournament Manager assignment. Correct the teams and try again.");
	}

	return {};
}

std::string format_tournament_match_label(
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

void apply_tournament_settings(config& settings, const mp_tournament_info* tournament)
{
	// The create-game request must remain flat because wesnothd reads these
	// attributes directly from the [multiplayer] node.
	settings["tournament_id"] = tournament ? tournament->id : "";
	settings["tournament_name"] = tournament ? tournament->name : "";
	settings["tournament_game_id"] = tournament ? tournament->game_id : "";
	settings["tournament_phase_name"] = tournament ? tournament->phase_name : "";
	settings["tournament_group_name"] = tournament ? tournament->group_name : "";
	settings["tournament_round_number"] = tournament ? tournament->round_number : "";
	settings["tournament_game_number"] = tournament ? tournament->game_number : "";
}

void apply_tournament_settings(mp_game_settings& settings, const mp_tournament_info* tournament)
{
	// Keep the local staging settings synchronized with the request sent above.
	settings.competitive.tournament_id = tournament ? tournament->id : "";
	settings.competitive.tournament_name = tournament ? tournament->name : "";
	settings.competitive.tournament_game_id = tournament ? tournament->game_id : "";
	settings.competitive.tournament_phase_name = tournament ? tournament->phase_name : "";
	settings.competitive.tournament_group_name = tournament ? tournament->group_name : "";
	settings.competitive.tournament_round_number = tournament ? tournament->round_number : "";
	settings.competitive.tournament_game_number = tournament ? tournament->game_number : "";
}

} // namespace gui2::dialogs
