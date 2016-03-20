/*
   Copyright (C) 2009 - 2016 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "gui/dialogs/lobby/lobby_info.hpp"

#include "config.hpp"
#include "game_preferences.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "network.hpp"
#include "utils/foreach.hpp"
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
#define DBG_LB LOG_STREAM(debug, log_lobby)
#define LOG_LB LOG_STREAM(info, log_lobby)
#define WRN_LB LOG_STREAM(warn, log_lobby)
#define ERR_LB LOG_STREAM(err, log_lobby)
#define SCOPE_LB log_scope2(log_lobby, __func__)


lobby_info::lobby_info(const config& game_config)
	: game_config_(game_config)
	, gamelist_()
	, gamelist_initialized_(false)
	, rooms_()
	, games_by_id_()
	, games_()
	, games_filtered_()
	, users_()
	, users_sorted_()
	, whispers_()
	, game_filter_()
	, game_filter_invert_(false)
	, games_visibility_()
{
}

lobby_info::~lobby_info()
{
	delete_games();
}

void lobby_info::delete_games()
{
	FOREACH(const AUTO & v, games_by_id_)
	{
		delete v.second;
	}
}

namespace
{

std::string dump_games_map(const lobby_info::game_info_map& games)
{
	std::stringstream ss;
	FOREACH(const AUTO & v, games)
	{
		const game_info& game = *v.second;
		ss << "G" << game.id << "(" << game.name << ") "
		   << game.display_status_string() << " ";
	}
	ss << "\n";
	return ss.str();
}

std::string dump_games_config(const config& gamelist)
{
	std::stringstream ss;
	FOREACH(const AUTO & c, gamelist.child_range("game"))
	{
		ss << "g" << c["id"] << "(" << c["name"] << ") "
		   << c[config::diff_track_attribute] << " ";
	}
	ss << "\n";
	return ss.str();
}

} // end anonymous namespace

void lobby_info::process_gamelist(const config& data)
{
	SCOPE_LB;
	gamelist_ = data;
	gamelist_initialized_ = true;
	delete_games();
	games_by_id_.clear();
	FOREACH(const AUTO & c, gamelist_.child("gamelist").child_range("game"))
	{
		game_info* game = new game_info(c, game_config_);
		games_by_id_[game->id] = game;
	}
	DBG_LB << dump_games_map(games_by_id_);
	DBG_LB << dump_games_config(gamelist_.child("gamelist"));
	process_userlist();
}


bool lobby_info::process_gamelist_diff(const config& data)
{
	SCOPE_LB;
	if(!gamelist_initialized_)
		return false;
	DBG_LB << "prediff " << dump_games_config(gamelist_.child("gamelist"));
	try
	{
		gamelist_.apply_diff(data, true);
	}
	catch(config::error& e)
	{
		ERR_LB << "Error while applying the gamelist diff: '" << e.message
			   << "' Getting a new gamelist.\n";
		network::send_data(config("refresh_lobby"), 0);
		return false;
	}
	DBG_LB << "postdiff " << dump_games_config(gamelist_.child("gamelist"));
	DBG_LB << dump_games_map(games_by_id_);
	config::child_itors range = gamelist_.child("gamelist").child_range("game");
	for(config::child_iterator i = range.first; i != range.second; ++i) {
		config& c = *i;
		DBG_LB << "data process: " << c["id"] << " ("
			   << c[config::diff_track_attribute] << ")\n";
		int game_id = c["id"];
		if(game_id == 0) {
			ERR_LB << "game with id 0 in gamelist config" << std::endl;
			network::send_data(config("refresh_lobby"), 0);
			return false;
		}
		game_info_map::iterator current_i = games_by_id_.find(game_id);
		const std::string& diff_result = c[config::diff_track_attribute];
		if(diff_result == "new" || diff_result == "modified") {
			if(current_i == games_by_id_.end()) {
				games_by_id_.insert(std::make_pair(
						game_id, new game_info(c, game_config_)));
			} else {
				// had a game with that id, so update it and mark it as such
				*(current_i->second) = game_info(c, game_config_);
				current_i->second->display_status = game_info::UPDATED;
			}
		} else if(diff_result == "deleted") {
			if(current_i == games_by_id_.end()) {
				WRN_LB << "Would have to delete a game that I don't have: "
					   << game_id << "\n";
			} else {
				if(current_i->second->display_status == game_info::NEW) {
					// this means the game never made it through to the user
					// interface
					// so just deleting it is fine
					games_by_id_.erase(current_i);
				} else {
					current_i->second->display_status = game_info::DELETED;
				}
			}
		}
	}
	DBG_LB << dump_games_map(games_by_id_);
	try
	{
		gamelist_.clear_diff_track(data);
	}
	catch(config::error& e)
	{
		ERR_LB << "Error while applying the gamelist diff (2): '" << e.message
			   << "' Getting a new gamelist.\n";
		network::send_data(config("refresh_lobby"), 0);
		return false;
	}
	DBG_LB << "postclean " << dump_games_config(gamelist_.child("gamelist"));
	process_userlist();
	return true;
}

void lobby_info::process_userlist()
{
	SCOPE_LB;
	users_.clear();
	FOREACH(const AUTO & c, gamelist_.child_range("user"))
	{
		users_.push_back(user_info(c));
	}
	FOREACH(AUTO & ui, users_)
	{
		if(ui.game_id != 0) {
			game_info* g = get_game_by_id(ui.game_id);
			if(g == NULL) {
				WRN_NG << "User " << ui.name
					   << " has unknown game_id: " << ui.game_id << "\n";
			} else {
				switch(ui.relation) {
					case user_info::FRIEND:
						g->has_friends = true;
						break;
					case user_info::IGNORED:
						g->has_ignored = true;
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
	DBG_LB << "lobby_info::sync_games_display_status";
	DBG_LB << "games_by_id_ size: " << games_by_id_.size();
	game_info_map::iterator i = games_by_id_.begin();
	while(i != games_by_id_.end()) {
		if(i->second->display_status == game_info::DELETED) {
			games_by_id_.erase(i++);
		} else {
			i->second->display_status = game_info::CLEAN;
			++i;
		}
	}
	DBG_LB << " -> " << games_by_id_.size() << "\n";
	make_games_vector();
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

room_info* lobby_info::get_room(const std::string& name)
{
	FOREACH(AUTO & r, rooms_)
	{
		if(r.name() == name)
			return &r;
	}
	return NULL;
}

const room_info* lobby_info::get_room(const std::string& name) const
{
	FOREACH(const AUTO & r, rooms_)
	{
		if(r.name() == name)
			return &r;
	}
	return NULL;
}

bool lobby_info::has_room(const std::string& name) const
{
	return get_room(name) != NULL;
}

chat_log& lobby_info::get_whisper_log(const std::string& name)
{
	return whispers_[name];
}

void lobby_info::open_room(const std::string& name)
{
	if(!has_room(name)) {
		rooms_.push_back(room_info(name));
	}
}

void lobby_info::close_room(const std::string& name)
{
	room_info* r = get_room(name);
	DBG_LB << "lobby info: closing room " << name << " "
		   << static_cast<void*>(r) << "\n";
	if(r) {
		rooms_.erase(rooms_.begin() + (r - &rooms_[0]));
	}
}

const std::vector<game_info*>& lobby_info::games_filtered() const
{
	return games_filtered_;
}

void lobby_info::add_game_filter(game_filter_base* f)
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

void lobby_info::make_games_vector()
{
	games_filtered_.clear();
	games_visibility_.clear();
	games_.clear();
	FOREACH(const AUTO & v, games_by_id_)
	{
		games_.push_back(v.second);
	}
}

void lobby_info::apply_game_filter()
{
	games_filtered_.clear();
	games_visibility_.clear();
	FOREACH(AUTO g, games_)
	{
		game_info& gi = *g;
		bool show = game_filter_.match(gi);
		if(game_filter_invert_) {
			show = !show;
		}
		games_visibility_.push_back(show);
		if(show) {
			games_filtered_.push_back(&gi);
		}
	}
}

void lobby_info::update_user_statuses(int game_id, const room_info* room)
{
	FOREACH(AUTO & user, users_)
	{
		user.update_state(game_id, room);
	}
}


struct user_sorter_name
{
	bool operator()(const user_info& u1, const user_info& u2)
	{
		return u1.name < u2.name;
	}
	bool operator()(const user_info* u1, const user_info* u2)
	{
		return operator()(*u1, *u2);
	}
};

struct user_sorter_relation
{
	bool operator()(const user_info& u1, const user_info& u2)
	{
		return static_cast<int>(u1.relation) < static_cast<int>(u2.relation);
	}
	bool operator()(const user_info* u1, const user_info* u2)
	{
		return operator()(*u1, *u2);
	}
};

struct user_sorter_relation_name
{
	bool operator()(const user_info& u1, const user_info& u2)
	{
		return static_cast<int>(u1.relation) < static_cast<int>(u2.relation)
			   || (u1.relation == u2.relation && u1.name < u2.name);
	}
	bool operator()(const user_info* u1, const user_info* u2)
	{
		return operator()(*u1, *u2);
	}
};

void lobby_info::sort_users(bool by_name, bool by_relation)
{
	users_sorted_.clear();
	FOREACH(AUTO & u, users_)
	{
		users_sorted_.push_back(&u);
	}
	if(by_name) {
		if(by_relation) {
			std::sort(users_sorted_.begin(),
					  users_sorted_.end(),
					  user_sorter_relation_name());
		} else {
			std::sort(users_sorted_.begin(),
					  users_sorted_.end(),
					  user_sorter_name());
		}
	} else if(by_relation) {
		std::sort(users_sorted_.begin(),
				  users_sorted_.end(),
				  user_sorter_relation());
	}
}

const std::vector<user_info*>& lobby_info::users_sorted() const
{
	return users_sorted_;
}
