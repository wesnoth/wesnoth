/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by Jörg Hinrichs, refactored from various
   places formerly created by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "savegame.hpp"

#include "gettext.hpp"
#include "log.hpp"
#include "preferences_display.hpp"
#include "serialization/binary_or_text.hpp"

#define LOG_SAVE LOG_STREAM(info, engine)

// Prototypes

// Interactive replay save
void save_replay(std::string& filename, game_state& gamestate)
{
	// Create an empty snapshot
	config snapshot;

	// FIXME
	// Doesn't make too much sense at the moment but will, once
	// including the savegame dialog here is done
	save_game(filename, snapshot, gamestate);
}

// Automatic replay save at the end of the scenario
void save_replay(game_state& gamestate)
{
	// FIXME:
	// We create a copy here in order to avoid having to change the original
	// gamestate. Once we made sure the player information is within
	// [replay_start] only, this whole stuff is no longer needed.
	game_state saveInfo = gamestate;

	// If the starting position contains player information, use this for
	// the replay savegame (this originally comes from the gamestate constructor,
	// where the player stuff is added to the starting position to be used here.
	config::child_itors player_list = gamestate.starting_pos.child_range("player");
	if (player_list.first != player_list.second) {
		saveInfo.players.clear();
		saveInfo.load_recall_list(player_list);
	}

	// Create an empty snapshot
	config snapshot;

	std::string filename = gamestate.label + _(" replay");
	save_game(filename, snapshot, saveInfo);

	// clear replay data and snapshot for the next scenario
	gamestate.replay_data = config();
	gamestate.snapshot = config();
}

/** Autosave */
std::string save_autosave(unsigned turn, const config& snapshot, game_state& gamestate)
{
	std::string filename;
	if (gamestate.label.empty())
		filename = _("Auto-Save");
	else
		filename = gamestate.label + "-" + _("Auto-Save") + lexical_cast<std::string>(turn);

	save_game(filename, snapshot, gamestate);

	return filename;
}

void save_game(std::string& filename, const config& snapshot, game_state& gamestate)
{
	LOG_SAVE << "savegame::save_game";

	if(preferences::compress_saves()) {
		filename += ".gz";
	}

	std::stringstream ss;
	{
		config_writer out(ss, preferences::compress_saves());
		::write_game(out, snapshot, gamestate);
		finish_save_game(out, gamestate, gamestate.label);
	}
	scoped_ostream os(open_save_game(filename));
	(*os) << ss.str();

	if (!os->good()) {
		throw game::save_game_failed(_("Could not write to file"));
	}
}

