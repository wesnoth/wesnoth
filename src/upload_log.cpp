/* $Id$ */
/*
   Copyright (C) 2005 - 2007 by Rusty Russell <rusty@rustcorp.com.au>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file upload_log.cpp
//! Manage logfiles for uploading as feedback, e.g.\ for champaign-balancing.

#include "global.hpp"

#define GETTEXT_DOMAIN "wesnoth"

#include "construct_dialog.hpp"
#include "game_config.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "filesystem.hpp"
#include "serialization/parser.hpp"
#include "upload_log.hpp"
#include "wesconfig.h"
#include "wml_separators.hpp"

#include "SDL_net.h"

#include <vector>
#include <string>

#define TARGET_HOST "stats.wesnoth.org"
#define TARGET_URL "/upload.cgi"
#define TARGET_PORT 80

struct upload_log::thread_info upload_log::thread_;

// On exit, kill the upload thread if it's still going.
upload_log::manager::~manager()
{
	threading::thread *t = thread_.t;
	if (t)
		t->kill();
}

static void send_string(TCPsocket sock, const std::string &str)
{
	if (SDLNet_TCP_Send(sock, const_cast<void*>(static_cast<const void*>(
			str.c_str())), str.length()) != static_cast<int>(str.length())) {

		throw network::error("");
	}
}

// Function which runs in a background thread to upload logs to server.
// Uses http POST to port 80 for maximum firewall penetration & other-end
// compatibility.
static int upload_logs(void *_ti)
{
	TCPsocket sock = NULL;
	upload_log::thread_info *ti = static_cast<upload_log::thread_info*>(_ti);

	const char *header =
		"POST " TARGET_URL " HTTP/1.1\n"
		"Host: " TARGET_HOST "\n"
		"User-Agent: Wesnoth " VERSION "\n"
		"Content-Type: text/plain\n";

	try {
		std::vector<std::string> files;

		// These are sorted: send them one at a time until we get to lastfile.
		get_files_in_dir(get_upload_dir(), &files, NULL, ENTIRE_FILE_PATH);

		IPaddress ip;
		if (SDLNet_ResolveHost(&ip, TARGET_HOST, TARGET_PORT) == 0) {
			std::vector<std::string>::iterator i;
			for (i = files.begin(); i!=files.end() && *i!=ti->lastfile; i++) {
				std::string contents;
				char response[10]; //This needs to be strlen("HTTP/1.1 2");

				contents = read_file(*i);

				sock = SDLNet_TCP_Open(&ip);
				if (!sock)
					break;
				send_string(sock, header);
				send_string(sock, "Content-length: ");
				send_string(sock, lexical_cast<std::string>(contents.length()));
				send_string(sock, "\n\n");
				send_string(sock, contents.c_str());

				if (SDLNet_TCP_Recv(sock, response, sizeof(response))
					!= sizeof(response))
					break;
				// Must be version 1.x, must start with 2 (eg. 200) for success
				if (memcmp(response, "HTTP/1.", strlen("HTTP/1.")) != 0)
					break;
				if (memcmp(response+8, " 2", strlen(" 2")) != 0)
					break;

				delete_directory(*i);
				SDLNet_TCP_Close(sock);
				sock = NULL;
			}
		}
	} catch(...) { }

	if (sock)
		SDLNet_TCP_Close(sock);
	ti->t = NULL;
	return 0;
}

// Currently only enabled when playing campaigns.
upload_log::upload_log(bool enable) : game_(NULL), enabled_(enable)
{
	filename_ = next_filename(get_upload_dir(), 100);
	if (preferences::upload_log() && !thread_.t) {
		// Thread can outlive us; it uploads everything up to the
		// next filename, and unsets thread_.t when it's finished.
		thread_.lastfile = filename_;
		thread_.t = new threading::thread(upload_logs, &thread_);
	}
}

upload_log::~upload_log()
{
	// If last game has a conclusion, add it.
	if (game_finished(game_))
		config_.add_child("game", *game_);

	if (game_)
		delete game_;

	if (enabled_ && !config_.empty() && !game_config::debug) {
		config_["version"] = VERSION;
		config_["format_version"] = "1";
		config_["id"] = preferences::upload_id();
		config_["serial"] = lexical_cast<std::string>(time(NULL)) + file_name(filename_);
		std::ostream *out = ostream_file(filename_);
		write(*out, config_);
		delete out;

		// Try to upload latest log before exit.
		if (preferences::upload_log() && !thread_.t) {
			thread_.lastfile = next_filename(get_upload_dir(), 1000);
			thread_.t = new threading::thread(upload_logs, &thread_);
		}
	}
}

bool upload_log::game_finished(config *game)
{
	if (!game)
		return false;

	return game->child("victory") || game->child("defeat") || game->child("quit");
}

config &upload_log::add_game_result(const std::string &str, int turn)
{
	config &child = game_->add_child(str);
	child["time"] = lexical_cast<std::string>(SDL_GetTicks() / 1000);
	child["end_turn"] = lexical_cast<std::string>(turn);
	return child;
}

// User starts a game (may be new campaign or saved).
void upload_log::start(game_state &state, const team &team,
					   unsigned team_number,
					   const unit_map &units,
					   const t_string &turn,
					   int num_turns)
{
	std::vector<const unit*> all_units;

	// If we have a previous game which is finished, add it.
	if (game_finished(game_))
		config_.add_child("game", *game_);

	// Start could be called more than once,
	// so delete game_ to prevent memory leak
	delete game_;
	game_ = new config();
	(*game_)["time"] = lexical_cast<std::string>(SDL_GetTicks() / 1000);
	(*game_)["campaign"] = state.campaign_define;
	(*game_)["difficulty"] = state.difficulty;
	(*game_)["scenario"] = state.scenario;
	if (!state.version.empty())
		(*game_)["version"] = state.version;
	if (!turn.empty())
		(*game_)["start_turn"] = turn;
	(*game_)["gold"] = lexical_cast<std::string>(team.gold());
	(*game_)["num_turns"] = lexical_cast<std::string>(num_turns);

	// We seem to have to walk the map to find some units,
	// and the player's available_units for the rest.
	for (unit_map::const_iterator un = units.begin(); un != units.end(); ++un){
		if (un->second.side() == team_number) {
			all_units.push_back(&un->second);
		}
	}

	//! @todo FIXME: Assumes first player is "us"; is that valid?
	player_info &player = state.players.begin()->second;
	for (std::vector<unit>::iterator it = player.available_units.begin();
		 it != player.available_units.end();
		 ++it) {
		all_units.push_back(&*it);
	}

	// Record details of any special units.
	std::vector<const unit*>::const_iterator i;
	for (i = all_units.begin(); i != all_units.end(); ++i) {
		if ((*i)->can_recruit()) {
			config &sp = game_->add_child("special-unit");
			sp["name"] = (*i)->name();
			sp["level"] = lexical_cast<std::string>((*i)->level());
			sp["experience"] = lexical_cast<std::string>((*i)->experience());
		}
	}

	// Record summary of all units.
	config &summ = game_->add_child("units-by-level");
	bool higher_units = true;
	for (int level = 0; higher_units; level++) {
		std::map<std::string, int> tally;

		higher_units = false;
		for (i = all_units.begin(); i != all_units.end(); ++i) {
			if ((*i)->level() > level)
				higher_units = true;
			else if ((*i)->level() == level) {
				if (tally.find((*i)->id()) == tally.end())
					tally[(*i)->id()] = 1;
				else
					tally[(*i)->id()]++;
			}
		}
		if (!tally.empty()) {
			config &tc = summ.add_child(lexical_cast<std::string>(level));
			for (std::map<std::string, int>::iterator t = tally.begin();
				 t != tally.end();
				 t++) {
				config &uc = tc.add_child(t->first);
				uc["count"] = lexical_cast<std::string>(t->second);
			}
		}
	}
}

// User finishes a scenario.
void upload_log::defeat(int turn)
{
	// game_ can be NULL if user takes over partway through MP game.
	if (game_) {
		add_game_result("defeat", turn);
	}
}

void upload_log::victory(int turn, int gold)
{
	// game_ can be NULL if user takes over partway through MP game.
	if (game_) {
		config &e = add_game_result("victory", turn);
		e["gold"] = lexical_cast<std::string>(gold);
	}
}

void upload_log::quit(int turn)
{
	std::string turnstr = lexical_cast<std::string>(turn);

	// We only record the quit if they've actually played a turn.
	if (!game_ || game_->get_attribute("start_turn") == turnstr || turn == 1)
		return;

	add_game_result("quit", turn);
}

//! Ask user for permission to upload his game-stats.
void upload_log_dialog::show_beg_dialog(display& disp)
{
	std::string msg = std::string(_("Wesnoth relies on volunteers like yourself for feedback, especially beginners and new players. Wesnoth keeps summaries of your games: you can help us improve game play by giving permission to send these summaries (anonymously) to wesnoth.org.\n"))
		+ " \n`" + _("Summaries are stored here:")
		+ " \n`~" + get_upload_dir() + "\n \n`"
		+ _("You can view the results at:") + "\n`~"
		+ "http://stats.wesnoth.org/?" + preferences::upload_id() + "\n \n";
	gui::dialog d(disp, _("Help us make Wesnoth better for you!"), msg, gui::OK_ONLY);

	d.add_option(_("Enable summary uploads"),
		preferences::upload_log(), gui::dialog::BUTTON_CHECKBOX_LEFT);
	d.show();
	preferences::set_upload_log(d.option_checked());
}
