/*
   Copyright (C) 2009 - 2017 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "game_initialization/lobby_info.hpp"

#include "config.hpp"
#include "game_preferences.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "map/exception.hpp"
#include "wml_exception.hpp"
#include "wesnothd_connection.hpp"
#include "mp_ui_alerts.hpp"
#include <iterator>

static lg::log_domain log_engine("engine");
#define WRN_NG LOG_STREAM(warn, log_engine)

static lg::log_domain log_lobby("lobby");
#define DBG_LB LOG_STREAM(debug, log_lobby)
#define WRN_LB LOG_STREAM(warn, log_lobby)
#define ERR_LB LOG_STREAM(err, log_lobby)
#define SCOPE_LB log_scope2(log_lobby, __func__)

namespace mp {

lobby_info::lobby_info(const config& game_config, const std::vector<std::string>& installed_addons)
	: game_config_(game_config)
	, installed_addons_(installed_addons)
	, gamelist_()
	, gamelist_initialized_(false)
	, rooms_()
	, games_by_id_()
	, games_()
	, games_filtered_()
	, users_()
	, users_sorted_()
	, whispers_()
	, game_filters_()
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
	for(const auto & v : games_by_id_) {
		delete v.second;
	}
}

void do_notify(notify_mode mode, const std::string & sender, const std::string & message)
{
	switch (mode) {
	case NOTIFY_WHISPER:
	case NOTIFY_WHISPER_OTHER_WINDOW:
	case NOTIFY_OWN_NICK:
		mp_ui_alerts::private_message(true, sender, message);
		break;
	case NOTIFY_FRIEND_MESSAGE:
		mp_ui_alerts::friend_message(true, sender, message);
		break;
	case NOTIFY_SERVER_MESSAGE:
		mp_ui_alerts::server_message(true, sender, message);
		break;
	case NOTIFY_LOBBY_QUIT:
		mp_ui_alerts::player_leaves(true);
		break;
	case NOTIFY_LOBBY_JOIN:
		mp_ui_alerts::player_joins(true);
		break;
	case NOTIFY_MESSAGE:
		mp_ui_alerts::public_message(true, sender, message);
		break;
	default:
		break;
	}
}

namespace
{

std::string dump_games_map(const lobby_info::game_info_map& games)
{
	std::stringstream ss;
	for(const auto & v : games) {
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
	for(const auto & c : gamelist.child_range("game")) {
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

	for(const auto & c : gamelist_.child("gamelist").child_range("game")) {
		game_info* game = new game_info(c, game_config_, installed_addons_);
		games_by_id_[game->id] = game;
	}

	DBG_LB << dump_games_map(games_by_id_);
	DBG_LB << dump_games_config(gamelist_.child("gamelist"));

	process_userlist();
}

bool lobby_info::process_gamelist_diff(const config& data)
{
	SCOPE_LB;
	if(!gamelist_initialized_) {
		return false;
	}

	DBG_LB << "prediff " << dump_games_config(gamelist_.child("gamelist"));
	try {
		gamelist_.apply_diff(data, true);
	} catch(config::error& e) {
		ERR_LB << "Error while applying the gamelist diff: '" << e.message
		       << "' Getting a new gamelist.\n";
		return false;
	}

	DBG_LB << "postdiff " << dump_games_config(gamelist_.child("gamelist"));
	DBG_LB << dump_games_map(games_by_id_);

	for(config& c : gamelist_.child("gamelist").child_range("game")) {
		DBG_LB << "data process: " << c["id"] << " (" << c[config::diff_track_attribute] << ")\n";
		const int game_id = c["id"];
		if(game_id == 0) {
			ERR_LB << "game with id 0 in gamelist config" << std::endl;
			return false;
		}

		game_info_map::iterator current_i = games_by_id_.find(game_id);

		const std::string& diff_result = c[config::diff_track_attribute];
		if(diff_result == "new" || diff_result == "modified") {
			if(current_i == games_by_id_.end()) {
				games_by_id_.insert({game_id, new game_info(c, game_config_, installed_addons_)});
				continue;
			}

			// Had a game with that id, so update it and mark it as such
			*(current_i->second) = game_info(c, game_config_, installed_addons_);
			current_i->second->display_status = game_info::UPDATED;
		} else if(diff_result == "deleted") {
			if(current_i == games_by_id_.end()) {
				WRN_LB << "Would have to delete a game that I don't have: " << game_id << "\n";
				continue;
			}

			if(current_i->second->display_status == game_info::NEW) {
				// This means the game never made it through to the user interface,
				// so just deleting it is fine
				games_by_id_.erase(current_i);
			} else {
				current_i->second->display_status = game_info::DELETED;
			}
		}
	}

	DBG_LB << dump_games_map(games_by_id_);
	try	{
		gamelist_.clear_diff_track(data);
	} catch(config::error& e) {
		ERR_LB << "Error while applying the gamelist diff (2): '" << e.message
			   << "' Getting a new gamelist.\n";
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
	for(const auto & c : gamelist_.child_range("user")) {
		users_.push_back(user_info(c));
	}

	for(auto & ui : users_) {
		if(ui.game_id == 0) {
			continue;
		}

		game_info* g = get_game_by_id(ui.game_id);
		if(!g) {
			WRN_NG << "User " << ui.name << " has unknown game_id: " << ui.game_id << "\n";
			continue;
		}

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

void lobby_info::sync_games_display_status()
{
	DBG_LB << "lobby_info::sync_games_display_status";
	DBG_LB << "games_by_id_ size: " << games_by_id_.size();

	game_info_map::iterator i = games_by_id_.begin();
	while(i != games_by_id_.end()) {
		if(i->second->display_status == game_info::DELETED) {
			i = games_by_id_.erase(i);
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
	game_info_map::iterator i = games_by_id_.find(id);
	return i == games_by_id_.end() ? nullptr : i->second;
}

const game_info* lobby_info::get_game_by_id(int id) const
{
	game_info_map::const_iterator i = games_by_id_.find(id);
	return i == games_by_id_.end() ? nullptr : i->second;
}

room_info* lobby_info::get_room(const std::string& name)
{
	for(auto & r : rooms_) {
		if(r.name() == name)
			return &r;
	}

	return nullptr;
}

const room_info* lobby_info::get_room(const std::string& name) const
{
	for(const auto & r : rooms_) {
		if(r.name() == name) {
			return &r;
		}
	}

	return nullptr;
}

bool lobby_info::has_room(const std::string& name) const
{
	return get_room(name) != nullptr;
}

user_info* lobby_info::get_user(const std::string& name)
{
	for(auto& user : users_) {
		if(user.name == name) {
			return &user;
		}
	}

	return nullptr;
}

chat_session& lobby_info::get_whisper_log(const std::string& name)
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
	DBG_LB << "lobby info: closing room " << name << "\n";
	if(room_info* r = get_room(name)) {
		rooms_.erase(rooms_.begin() + (r - &rooms_[0]));
	}
}

const std::vector<game_info*>& lobby_info::games_filtered() const
{
	return games_filtered_;
}

void lobby_info::add_game_filter(game_filter_func func)
{
	game_filters_.push_back(func);
}

void lobby_info::clear_game_filter()
{
	game_filters_.clear();
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

	for(const auto & v : games_by_id_) {
		games_.push_back(v.second);
	}
}

void lobby_info::apply_game_filter()
{
	games_filtered_.clear();
	games_visibility_.clear();
	for(auto g : games_) {
		game_info& gi = *g;

		bool show = true;
		for(const auto& filter_func : game_filters_) {
			show = filter_func(gi);
			if(!show) {
				break;
			}
		}

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
	for(auto & user : users_) {
		user.update_state(game_id, room);
	}
}

void lobby_info::sort_users(bool by_name, bool by_relation)
{
	users_sorted_.clear();
	for(auto& u : users_) {
		users_sorted_.push_back(&u);
	}

	// TODO: is this the simplest way to represent the sorting method?
	std::sort(users_sorted_.begin(), users_sorted_.end(), [&](const user_info* u1, const user_info* u2) {
		if(by_name && by_relation) {
			return u1->relation < u2->relation || (u1->relation == u2->relation && u1->name < u2->name);
		}

		return (by_name ? u1->name < u2->name : true) && (by_relation ? u1->relation < u2->relation : true);
	});
}

const std::vector<user_info*>& lobby_info::users_sorted() const
{
	return users_sorted_;
}

}
