/*
	Copyright (C) 2026
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
*/

#pragma once

#include "gettext.hpp"

#include <string>

namespace gui2::dialogs
{

/** Translate locale-independent competitive-play message IDs from wesnothd. */
inline std::string localized_competitive_message(const std::string& message_id)
{
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

	return {};
}

} // namespace gui2::dialogs
