/*
	Copyright (C) 2009 - 2024
	by Tomasz Sniatowski <kailoran@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "game_initialization/lobby_info.hpp"

#include "addon/manager.hpp" // for installed_addons
#include "log.hpp"
#include "mp_ui_alerts.hpp"


static lg::log_domain log_engine("engine");
#define WRN_NG LOG_STREAM(warn, log_engine)

static lg::log_domain log_lobby("lobby");
#define DBG_LB LOG_STREAM(debug, log_lobby)
#define WRN_LB LOG_STREAM(warn, log_lobby)
#define ERR_LB LOG_STREAM(err, log_lobby)
#define SCOPE_LB log_scope2(log_lobby, __func__)

namespace mp
{
lobby_info::lobby_info()
	: installed_addons_()
	, gamelist_()
	, gamelist_initialized_(false)
	, games_by_id_()
	, games_()
	, users_()
	, game_filters_()
	, game_filter_invert_()
	, games_visibility_()
{
	refresh_installed_addons_cache();
}

void lobby_info::refresh_installed_addons_cache()
{
	// This function does not refer to an addon database, it calls filesystem functions.
	// For the sanity of the mp lobby, this list should be fixed for the entire lobby session,
	// even if the user changes the contents of the addon directory in the meantime.
	installed_addons_ = ::installed_addons();
}

void do_notify(notify_mode mode, const std::string& sender, const std::string& message)
{
	switch(mode) {
	case notify_mode::whisper:
	case notify_mode::whisper_other_window:
	case notify_mode::own_nick:
		mp::ui_alerts::private_message(true, sender, message);
		break;
	case notify_mode::friend_message:
		mp::ui_alerts::friend_message(true, sender, message);
		break;
	case notify_mode::server_message:
		mp::ui_alerts::server_message(true, sender, message);
		break;
	case notify_mode::lobby_quit:
		mp::ui_alerts::player_leaves(true);
		break;
	case notify_mode::lobby_join:
		mp::ui_alerts::player_joins(true);
		break;
	case notify_mode::message:
		mp::ui_alerts::public_message(true, sender, message);
		break;
	case notify_mode::game_created:
		mp::ui_alerts::game_created(sender, message);
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
	for(const auto& v : games) {
		const game_info& game = v.second;
		ss << "G" << game.id << "(" << game.name << ") " << game.display_status_string() << " ";
	}

	ss << "\n";
	return ss.str();
}

std::string dump_games_config(const config& gamelist)
{
	std::stringstream ss;
	for(const auto& c : gamelist.child_range("game")) {
		ss << "g" << c["id"] << "(" << c["name"] << ") " << c[config::diff_track_attribute] << " ";
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

	games_by_id_.clear();

	for(const auto& c : gamelist_.mandatory_child("gamelist").child_range("game")) {
		game_info game(c, installed_addons_);
		games_by_id_.emplace(game.id, std::move(game));
	}

	DBG_LB << dump_games_map(games_by_id_);
	DBG_LB << dump_games_config(gamelist_.mandatory_child("gamelist"));

	process_userlist();
}

bool lobby_info::process_gamelist_diff(const config& data)
{
	if(!process_gamelist_diff_impl(data)) {
		// the gamelist is now corrupted, stop further processing and wait for a fresh list.
		gamelist_initialized_ = false;
		return false;
	} else {
		return true;
	}
}

bool lobby_info::process_gamelist_diff_impl(const config& data)
{
	SCOPE_LB;
	if(!gamelist_initialized_) {
		return false;
	}

	DBG_LB << "prediff " << dump_games_config(gamelist_.mandatory_child("gamelist"));

	try {
		gamelist_.apply_diff(data, true);
	} catch(const config::error& e) {
		ERR_LB << "Error while applying the gamelist diff: '" << e.message << "' Getting a new gamelist.";
		return false;
	}

	DBG_LB << "postdiff " << dump_games_config(gamelist_.mandatory_child("gamelist"));
	DBG_LB << dump_games_map(games_by_id_);

	for(config& c : gamelist_.mandatory_child("gamelist").child_range("game")) {
		DBG_LB << "data process: " << c["id"] << " (" << c[config::diff_track_attribute] << ")";

		const int game_id = c["id"].to_int();
		if(game_id == 0) {
			ERR_LB << "game with id 0 in gamelist config";
			return false;
		}

		auto current_i = games_by_id_.find(game_id);

		const std::string& diff_result = c[config::diff_track_attribute];

		if(diff_result == "new" || diff_result == "modified") {
			// note: at this point (1.14.3) the server never sends a 'modified' and instead
			// just sends a 'delete' followed by a 'new', it still works becasue the delete doesn't
			// delete the element and just marks it as game_info::DELETED so that game_info::DELETED
			// is replaced by game_info::UPDATED below. See also
			// https://github.com/wesnoth/wesnoth/blob/1.14/src/server/server.cpp#L149
			if(current_i == games_by_id_.end()) {
				games_by_id_.emplace(game_id, game_info(c, installed_addons_));
				continue;
			}

			// Had a game with that id, so update it and mark it as such
			current_i->second = game_info(c, installed_addons_);
			current_i->second.display_status = game_info::disp_status::UPDATED;
		} else if(diff_result == "deleted") {
			if(current_i == games_by_id_.end()) {
				WRN_LB << "Would have to delete a game that I don't have: " << game_id;
				continue;
			}

			if(current_i->second.display_status == game_info::disp_status::NEW) {
				// This means the game never made it through to the user interface,
				// so just deleting it is fine.
				games_by_id_.erase(current_i);
			} else {
				current_i->second.display_status = game_info::disp_status::DELETED;
			}
		}
	}

	DBG_LB << dump_games_map(games_by_id_);

	try {
		gamelist_.clear_diff_track(data);
	} catch(const config::error& e) {
		ERR_LB << "Error while applying the gamelist diff (2): '" << e.message << "' Getting a new gamelist.";
		return false;
	}

	DBG_LB << "postclean " << dump_games_config(gamelist_.mandatory_child("gamelist"));

	process_userlist();
	return true;
}

void lobby_info::process_userlist()
{
	SCOPE_LB;

	users_.clear();
	for(const auto& c : gamelist_.child_range("user")) {
		user_info& ui = users_.emplace_back(c);

		if(ui.game_id == 0) {
			continue;
		}

		game_info* g = get_game_by_id(ui.game_id);
		if(!g) {
			WRN_NG << "User " << ui.name << " has unknown game_id: " << ui.game_id;
			continue;
		}

		switch(ui.get_relation()) {
		case user_info::user_relation::FRIEND:
			g->has_friends = true;
			break;
		case user_info::user_relation::IGNORED:
			g->has_ignored = true;
			break;
		default:
			break;
		}
	}

	std::stable_sort(users_.begin(), users_.end());
}

std::function<void()> lobby_info::begin_state_sync()
{
	// First, update the list of game pointers to reflect any changes made to games_by_id_.
	// This guarantees anything that calls games() before post cleanup has valid pointers,
	// since there will likely have been changes to games_by_id_ caused by network traffic.
	make_games_vector();

	return [this]() {
		DBG_LB << "lobby_info, second state sync stage";
		DBG_LB << "games_by_id_ size: " << games_by_id_.size();

		auto i = games_by_id_.begin();

		while(i != games_by_id_.end()) {
			if(i->second.display_status == game_info::disp_status::DELETED) {
				i = games_by_id_.erase(i);
			} else {
				i->second.display_status = game_info::disp_status::CLEAN;
				++i;
			}
		}

		DBG_LB << " -> " << games_by_id_.size();

		make_games_vector();

		// Now that both containers are again in sync, update the visibility mask. We want to do
		// this last since the filer functions are expensive.
		apply_game_filter();
	};
}

game_info* lobby_info::get_game_by_id(int id)
{
	auto i = games_by_id_.find(id);
	return i == games_by_id_.end() ? nullptr : &i->second;
}

const game_info* lobby_info::get_game_by_id(int id) const
{
	auto i = games_by_id_.find(id);
	return i == games_by_id_.end() ? nullptr : &i->second;
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

void lobby_info::make_games_vector()
{
	games_.reserve(games_by_id_.size());
	games_.clear();

	for(auto& v : games_by_id_) {
		games_.push_back(&v.second);
	}

	// Reset the visibility mask. Its size should then match games_'s and all its bits be true.
	games_visibility_.resize(games_.size());
	games_visibility_.reset();
	games_visibility_.flip();
}

bool lobby_info::is_game_visible(const game_info& game)
{
	for(const auto& filter_func : game_filters_) {
		if(!game_filter_invert_(filter_func(game))) {
			return false;
		}
	}

	return true;
}

void lobby_info::apply_game_filter()
{
	// Since games_visibility_ is a visibility mask over games_,
	// they need to be the same size or we'll end up with issues.
	assert(games_visibility_.size() == games_.size());

	for(unsigned i = 0; i < games_.size(); ++i) {
		games_visibility_[i] = is_game_visible(*games_[i]);
	}
}

} // end namespace mp
