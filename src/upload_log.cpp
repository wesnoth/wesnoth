/* $Id$ */
/*
   Copyright (C) 2005 - 2010 by Rusty Russell <rusty@rustcorp.com.au>
   Copyright (C) 2009 - 2010 by Greg Shikhman <cornmander@cornmander.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file upload_log.cpp
 *  Manage logfiles for uploading as feedback, e.g.\ for campaign-balancing.
 */

#include "global.hpp"

#include "upload_log.hpp"

#include "construct_dialog.hpp"
#include "filesystem.hpp"
#include "foreach.hpp"
#include "game_preferences.hpp"
#include "gamestatus.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "network.hpp"
#include "replay.hpp"
#include "serialization/parser.hpp"
#include "team.hpp"
#include "unit.hpp"
#include "unit_map.hpp"
#include "wesconfig.h"

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

static lg::log_domain log_uploader("uploader");
#define DBG_UPLD LOG_STREAM(debug, log_uploader)
#define ERR_UPLD LOG_STREAM(err, log_uploader)

extern replay recorder;

const std::string TARGET_HOST = "www.wesnoth.org";
const std::string TARGET_URL = "/wesstats/upload";
const Uint16 TARGET_PORT = 80;

struct upload_log::thread_info upload_log::thread_;
upload_log::manager* upload_log::manager_ = 0;

// On exit, kill the upload thread if it's still going.
upload_log::manager::~manager()
{
	upload_log::manager_ = 0;
	threading::thread *t = thread_.t;
	if (t)
		t->join();
}

void upload_log::manager::manage()
{

	// We have to wait for thread so it won't leak
	if (thread_.shutdown)
	{
		thread_.t->join();
		thread_.shutdown = false;
		thread_.t = 0;
	}
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
	DBG_UPLD << "attempting to upload game logs\n";
	TCPsocket sock = NULL;
	upload_log::thread_info *ti = static_cast<upload_log::thread_info*>(_ti);
	int numfiles = 0;

	const std::string header =
		"POST " + TARGET_URL + " HTTP/1.1\n"
		"Host: " + TARGET_HOST + "\n"
		"User-Agent: Wesnoth " VERSION "\n"
		"Content-Type: text/plain\n";

	try {
		std::vector<std::string> files;

		// These are sorted: send them one at a time until we get to lastfile.
		get_files_in_dir(get_upload_dir(), &files, NULL, ENTIRE_FILE_PATH);

		IPaddress ip;
		network::manager ensure_net_initialized;

		if (SDLNet_ResolveHost(&ip, TARGET_HOST.c_str(), TARGET_PORT) == 0) {
			std::vector<std::string>::iterator i;
			for (i = files.begin(); i!=files.end() && *i!=ti->lastfile; ++i) {
				std::string contents;
				char response[10]; //This needs to be strlen("HTTP/1.1 2");

				contents = read_file(*i);

				sock = SDLNet_TCP_Open(&ip);
				if (!sock) {
					ERR_UPLD << "error connecting to log server\n";
					break;
				} else {
					DBG_UPLD << "successfully connected to log server\n";
				}
				send_string(sock, header.c_str());
				send_string(sock, "Content-length: ");
				send_string(sock, lexical_cast<std::string>(contents.length()));
				send_string(sock, "\n\n");
				send_string(sock, contents);

				// As long as we can actually send the data, delete the file.
				// Even if the server gives a bad response, we don't want to
				// be sending the same bad data over and over to the server.
				delete_directory(*i);
				numfiles++;

				if (SDLNet_TCP_Recv(sock, response, sizeof(response))
					!= sizeof(response))
					break;
				// Must be version 1.x, must start with 2 (eg. 200) for success
				if (memcmp(response, "HTTP/1.", strlen("HTTP/1.")) != 0)
					break;
				if (memcmp(response+8, " 2", strlen(" 2")) != 0)
					break;

				SDLNet_TCP_Close(sock);
				sock = NULL;
			}
		}
	} catch(...) { }

	if (sock)
		SDLNet_TCP_Close(sock);
	ti->shutdown = true;
	DBG_UPLD << numfiles << " game logs successfully sent to server\n";

	return 0;
}

// Currently only enabled when playing campaigns.
upload_log::upload_log(bool enable) :
	config_(),
	game_(NULL),
	filename_(next_filename(get_upload_dir(), 100)),
	enabled_(enable)
{
	if (upload_log::manager_)
		upload_log::manager_->manage();
	if (preferences::upload_log() && !thread_.t) {
		// Thread can outlive us; it uploads everything up to the
		// next filename, and unsets thread_.t when it's finished.
		thread_.lastfile = filename_;
		thread_.t = new threading::thread(upload_logs, &thread_);
	}
}

void upload_log::read_replay()
{
	if( !enabled_ || game_config::debug ) {
		return;
	}

	//@TODO: check if game_ is always there
	if( !game_ ) {
		game_ = new config();
	}

	const config& rd = recorder.get_replay_data();
	foreach (const config &c, rd.child_range("command")) {
		if(c.child("attack")) {
			//search through the attack to see if a unit died
			foreach (const config &c2, c.child_range("random"))
			{
				const config &res = c2.child("results");
				if (res && res["dies"].to_bool()) {
					config& cfg = game_->add_child("kill_event");
					cfg.add_child("attack",c.child("attack"));
					cfg.add_child("results", res);
				}
			}
		}
	}

	if(rd.child("upload_log")) {
		game_->add_child("upload_log",rd.child("upload_log"));
	}
}

upload_log::~upload_log()
{
	// If last game has a conclusion, add it.
	if (game_finished())
		config_.add_child("game", *game_);

	delete game_;

	if (enabled_ && !config_.empty() && !game_config::debug) {
		config_["version"] = VERSION;
		config_["format_version"] = 2;
		config_["id"] = preferences::upload_id();
		config_["serial"] = lexical_cast<std::string>(time(NULL)) + file_name(filename_);
		config_["language"] = preferences::language();

#ifdef _WIN32
		config_["platform"] = "Windows";
#elif __APPLE__
		config_["platform"] = "Apple";
#else
		config_["platform"] = "unknown";
#endif

		std::ostream *out = ostream_file(filename_);
		{
			boost::iostreams::filtering_stream<boost::iostreams::output> filter;
			filter.push(boost::iostreams::gzip_compressor());
			filter.push(*out);
			write(filter, config_);
		}
		delete out;

		if (upload_log::manager_)
			upload_log::manager_->manage();

		// Try to upload latest log before exit.
		if (preferences::upload_log() && !thread_.t) {
			thread_.lastfile = next_filename(get_upload_dir(), 1000);
			thread_.t = new threading::thread(upload_logs, &thread_);
		}
	}
}

bool upload_log::game_finished()
{
	if (!game_)
		return false;

	bool has_ai_log = false;
	if (const config &c = game_->child("upload_log"))
		has_ai_log = c.child("ai_log");

	return game_->child("victory") || game_->child("defeat") || game_->child("quit") || has_ai_log;
}

config &upload_log::add_game_result(const std::string &str, int turn)
{
	config &child = game_->add_child(str);
	child["time"] = int(SDL_GetTicks() / 1000);
	child["end_turn"] = turn;
	return child;
}

// User starts a game (may be new campaign or saved).
void upload_log::start(game_state &state, const team &team,
					   const t_string &turn,
					   int num_turns,
					   const std::string& map_data)
{
	std::vector<const unit*> all_units;

	// If we have a previous game which is finished, add it.
	if (game_finished()) {
		config_.add_child("game", *game_);
	}

	// Start could be called more than once,
	// so delete game_ to prevent memory leak
	delete game_;
	game_ = new config();
	(*game_)["time"] = int(SDL_GetTicks() / 1000);
	(*game_)["campaign"] = state.classification().campaign_define;
	(*game_)["difficulty"] = state.classification().difficulty;
	(*game_)["scenario"] = state.classification().scenario;

	//replace newlines in map definition with semicolons so that braindead server-side wml parser doesn't get confused
	std::string encoded_map(map_data);
	for(size_t idx = 0; idx < encoded_map.length(); idx++) {
		if(encoded_map[idx] == '\n')
			encoded_map[idx] = ';';
	}
	(*game_)["map_data"] = encoded_map;

	if (!state.classification().version.empty())
		(*game_)["version"] = state.classification().version;
	if (!turn.empty())
		(*game_)["start_turn"] = turn;
	(*game_)["gold"] = team.gold();
	(*game_)["num_turns"] = num_turns;
}

void upload_log::start(game_state &state, const std::string& map_data)
{
	// If we have a previous game which is finished, add it.
	if (game_finished()) {
		config_.add_child("game", *game_);
	}

	// Start could be called more than once,
	// so delete game_ to prevent memory leak
	delete game_;
	game_ = new config();
	(*game_)["time"] = int(SDL_GetTicks() / 1000);
	(*game_)["campaign"] = state.classification().campaign_type;
	(*game_)["difficulty"] = state.classification().difficulty;
	(*game_)["scenario"] = state.classification().label;

	//replace newlines in map definition with semicolons so that braindead server-side wml parser doesn't get confused
	std::string encoded_map(map_data);
	for(size_t idx = 0; idx < encoded_map.length(); idx++) {
		if(encoded_map[idx] == '\n')
			encoded_map[idx] = ';';
	}
	(*game_)["map_data"] = encoded_map;

	if (!state.classification().version.empty())
		(*game_)["version"] = state.classification().version;
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
		e["gold"] = gold;
	}
}

void upload_log::quit(int turn)
{
	std::string turnstr = lexical_cast<std::string>(turn);

	// We only record the quit if they've actually played a turn.
	if (!game_ || (*game_)["start_turn"] == turnstr || turn == 1)
		return;

	add_game_result("quit", turn);
}

/** Ask user for permission to upload his game-stats. */
void upload_log_dialog::show_beg_dialog(display& disp)
{
	std::string msg = std::string(_("Wesnoth relies on volunteers like yourself for feedback, especially beginners and new players. Wesnoth keeps summaries of your games: you can help us improve game play by giving permission to send these summaries (anonymously) to wesnoth.org.\n"))
		+ " \n`" + _("Summaries are stored here:")
		+ " \n`~" + get_upload_dir() + "\n \n`"
		+ _("You can view the results at:") + "\n`~"
		+ "http://stats.wesnoth.org/?user_id=" + preferences::upload_id() + "\n \n";
	gui::dialog d(disp, _("Help us make Wesnoth better for you!"), msg, gui::OK_ONLY);

	d.add_option(_("Enable summary uploads"),
		preferences::upload_log(), gui::dialog::BUTTON_CHECKBOX_LEFT);
	d.show();
	preferences::set_upload_log(d.option_checked());
}
