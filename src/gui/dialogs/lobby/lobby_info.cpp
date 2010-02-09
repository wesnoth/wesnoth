/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "gui/dialogs/lobby/lobby_info.hpp"

#include "config.hpp"
#include "game_preferences.hpp"
#include "filesystem.hpp"
#include "foreach.hpp"
#include "formula_string_utils.hpp"
#include "gettext.hpp"
#include "network.hpp"
#include "log.hpp"
#include "map.hpp"
#include "map_exception.hpp"
#include "wml_exception.hpp"

#include <iterator>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
static lg::log_domain log_engine("engine");
#define WRN_NG LOG_STREAM(warn, log_engine)

static lg::log_domain log_lobby("lobby");
#define DBG_LB LOG_STREAM(info, log_lobby)
#define LOG_LB LOG_STREAM(info, log_lobby)
#define ERR_LB LOG_STREAM(err, log_lobby)

namespace {

std::string dump_games_vector(const std::vector<game_info>& games)
{
	std::stringstream ss;
	foreach (const game_info& game, games) {
		ss << "G" << game.id << "(" << game.name << ") " << game.display_status_string() << " ";
	}
	ss << "\n";
	return ss.str();
}

std::string dump_games_config(const config& gamelist)
{
	std::stringstream ss;
	foreach (const config& c, gamelist.child_range("game")) {
		ss << "g" << c["id"] << "(" << c["name"] << ") " << c[config::diff_track_attribute] << " ";
	}
	ss << "\n";
	return ss.str();
}

} //end anonymous namespace

lobby_info::lobby_info(const config& game_config)
	: game_config_(game_config)
	, gamelist_()
	, gamelist_initialized_(false)
	, rooms_()
	, games_()
	, games_by_id_()
	, games_filtered_()
	, users_()
	, users_sorted_()
	, whispers_()
	, game_filter_()
	, game_filter_invert_(false)
	, games_shown_()
{
}

void lobby_info::process_gamelist(const config &data)
{
	gamelist_ = data;
	gamelist_initialized_ = true;
	games_.clear();
	foreach (const config& c, gamelist_.child("gamelist").child_range("game")) {
		games_.push_back(game_info(c, game_config_));
	}
	DBG_LB << dump_games_vector(games_);
	DBG_LB << dump_games_config(gamelist_.child("gamelist"));
	games_shown_.resize(games_.size());
	parse_gamelist();
}


bool lobby_info::process_gamelist_diff(const config &data)
{
	if (!gamelist_initialized_) return false;
	DBG_LB << "prediff " << dump_games_config(gamelist_.child("gamelist"));
	try {
		gamelist_.apply_diff(data, true);
	} catch(config::error& e) {
		ERR_LB << "Error while applying the gamelist diff: '"
			<< e.message << "' Getting a new gamelist.\n";
		network::send_data(config("refresh_lobby"), 0, true);
		return false;
	}
	DBG_LB << "postdiff " << dump_games_config(gamelist_.child("gamelist"));
	DBG_LB << dump_games_vector(games_);
	std::vector<game_info>::iterator game_i = games_.begin();
	config::child_itors range = gamelist_.child("gamelist").child_range("game");
	for (config::child_iterator i = range.first; i != range.second; ++i) {
		config& c = *i;
		DBG_LB << "data process: " << c["id"] << " (" << c[config::diff_track_attribute] << ")\n";
		if (c[config::diff_track_attribute] == "new") {
			if (game_i == games_.end()) {
				games_.insert(game_i, game_info(c, game_config_));
				game_i = games_.end();
			} else {
				//adding not at the end should rarely if ever happen
				int idx = std::distance(games_.begin(), game_i);
				games_.insert(game_i, game_info(c, game_config_));
				game_i = games_.begin();
				std::advance(game_i, idx + 1); //go back where we were accounting for the added item
			}
		} else {
			if (game_i == games_.end()) {
				ERR_LB << "Ran out of processed games while working on a gamelist diff\n";
				network::send_data(config("refresh_lobby"), 0, true);
				return false;
			}
			int config_game_id = lexical_cast_default<int>(c["id"]);
			if (game_i->id != config_game_id) {
				ERR_LB << "Game id doesn't match (in order) while applying diff "
					<< game_i->id << " " << config_game_id << "\n";
				network::send_data(config("refresh_lobby"), 0, true);
				return false;
			}
			if (c[config::diff_track_attribute] == "modified") {
				*game_i = game_info(c, game_config_);
				game_i->display_status = game_info::UPDATED;
			} else if (c[config::diff_track_attribute] == "deleted") {
				//check for a delete game X - insert "new" game X (same id) occurance
				config::child_iterator nexti = i;
				++nexti;
				if (nexti != range.second) {
					config& nextc = *nexti;
					if (nextc["id"] == c["id"]) {
						LOG_LB << "ID match in delete-add fix: "
							<< nextc["id"] << " " << nextc[config::diff_track_attribute] << "\n";
						if (nextc[config::diff_track_attribute] == "new") {
							nextc[config::diff_track_attribute] = "modified";
							continue;
						}
					}
				}
				game_i->display_status = game_info::DELETED;
			}
			++game_i;
		}
	}
	DBG_LB << dump_games_vector(games_);
	try {
		gamelist_.clear_diff_track(data);
	} catch(config::error& e) {
		ERR_LB << "Error while applying the gamelist diff (2): '"
			<< e.message << "' Getting a new gamelist.\n";
		network::send_data(config("refresh_lobby"), 0, true);
		return false;
	}
	DBG_LB << "postclean " << dump_games_config(gamelist_.child("gamelist"));
	games_shown_.resize(games_.size());
	parse_gamelist();
	return true;
}

void lobby_info::parse_gamelist()
{
	users_.clear();
	foreach (const config& c, gamelist_.child_range("user")) {
		users_.push_back(user_info(c));
	}

	games_by_id_.clear();
	foreach (game_info& gi, games_) {
		games_by_id_.insert(std::make_pair(gi.id, &gi));
	}
	foreach (user_info& ui, users_) {
		if (ui.game_id != 0) {
			std::map<int, game_info*>::iterator i = games_by_id_.find(ui.game_id);
			if (i == games_by_id_.end()) {
				WRN_NG << "User " << ui.name << " has unknown game_id: " << ui.game_id << "\n";
			} else {
				game_info& g = *i->second;
				switch (ui.relation) {
					case user_info::FRIEND:
						g.has_friends = true;
						break;
					case user_info::IGNORED:
						g.has_ignored = true;
						break;
					default:
						break;
				}
			}
		}
	}
}

void lobby_info::sync_games_display_status()
{
	DBG_LB << "sync_games_display_status\n";
	games_.erase(
		std::remove_if(games_.begin(), games_.end(),
			game_filter_value<game_info::game_display_status,
				&game_info::display_status>(game_info::DELETED)),
		games_.end());

	foreach (game_info& gi, games_) {
		gi.display_status = game_info::CLEAN;
	}
}

game_info* lobby_info::get_game_by_id(int id)
{
	std::map<int, game_info*>::iterator i = games_by_id_.find(id);
	return i == games_by_id_.end() ? NULL : i->second;
}

const game_info* lobby_info::get_game_by_id(int id) const
{
	std::map<int, game_info*>::const_iterator i = games_by_id_.find(id);
	return i == games_by_id_.end() ? NULL : i->second;
}

room_info* lobby_info::get_room(const std::string &name)
{
	foreach (room_info& r, rooms_) {
		if (r.name() == name) return &r;
	}
	return NULL;
}

const room_info* lobby_info::get_room(const std::string &name) const
{
	foreach (const room_info& r, rooms_) {
		if (r.name() == name) return &r;
	}
	return NULL;
}

bool lobby_info::has_room(const std::string &name) const
{
	return get_room(name) != NULL;
}

chat_log& lobby_info::get_whisper_log(const std::string &name)
{
	return whispers_[name];
}

void lobby_info::open_room(const std::string &name)
{
	if (!has_room(name)) {
		rooms_.push_back(room_info(name));
	}
}

void lobby_info::close_room(const std::string &name)
{
	room_info* r = get_room(name);
	DBG_LB << "lobby info: closing room " << name << " " << (void*)r << "\n";
	if (r) {
		rooms_.erase(rooms_.begin() + (r - &rooms_[0]));
	}
}

const std::vector<game_info*>& lobby_info::games_filtered() const
{
	return games_filtered_;
}

int lobby_info::games_shown_count() const
{
	return std::count(games_shown_.begin(), games_shown_.end(), true);
}

void lobby_info::add_game_filter(game_filter_base *f)
{
	game_filter_.append(f);
}

void lobby_info::clear_game_filter()
{
	game_filter_.clear();
}

void lobby_info::set_game_filter_invert(bool value)
{
	game_filter_invert_ = value;
}

void lobby_info::apply_game_filter()
{
	games_filtered_.clear();
	for (unsigned i = 0; i < games_.size(); ++i) {
		game_info& gi = games_[i];
		bool show = game_filter_.match(gi);
		if (game_filter_invert_) {
			show = !show;
		}
		games_shown_[i] = show;
		if (show) {
			games_filtered_.push_back(&gi);
		}
	}
}

void lobby_info::update_user_statuses(int game_id, const room_info *room)
{
	foreach (user_info& user, users_) {
		user.update_state(game_id, room);
	}
}


struct user_sorter_name
{
	bool operator()(const user_info& u1, const user_info& u2) {
		return u1.name < u2.name;
	}
	bool operator()(const user_info* u1, const user_info* u2) {
		return operator()(*u1, *u2);
	}
};

struct user_sorter_relation
{
	bool operator()(const user_info& u1, const user_info& u2) {
		return static_cast<int>(u1.relation) < static_cast<int>(u2.relation);
	}
	bool operator()(const user_info* u1, const user_info* u2) {
		return operator()(*u1, *u2);
	}
};

struct user_sorter_relation_name
{
	bool operator()(const user_info& u1, const user_info& u2) {
		return static_cast<int>(u1.relation) < static_cast<int>(u2.relation)
			|| (u1.relation == u2.relation && u1.name < u2.name);
	}
	bool operator()(const user_info* u1, const user_info* u2) {
		return operator()(*u1, *u2);
	}
};

void lobby_info::sort_users(bool by_name, bool by_relation)
{
	users_sorted_.clear();
	foreach (user_info& u, users_) {
		users_sorted_.push_back(&u);
	}
	if (by_name) {
		if (by_relation) {
			std::sort(users_sorted_.begin(), users_sorted_.end(), user_sorter_relation_name());
		} else {
			std::sort(users_sorted_.begin(), users_sorted_.end(), user_sorter_name());
		}
	} else if (by_relation) {
		std::sort(users_sorted_.begin(), users_sorted_.end(), user_sorter_relation());
	}
}

const std::vector<user_info*>& lobby_info::users_sorted() const
{
	return users_sorted_;
}
