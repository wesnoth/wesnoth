/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "server/game.hpp"

#include "filesystem.hpp"
#include "game_config.hpp" // game_config::observer_team_name
#include "lexical_cast.hpp"
#include "log.hpp"
#include "preferences/credentials.hpp"
#include "serialization/string_utils.hpp"
#include "server/player_network.hpp"
#include "server/server.hpp"

#include <cstdio>
#include <iomanip>
#include <sstream>

static lg::log_domain log_server("server");
#define ERR_GAME LOG_STREAM(err, log_server)
#define WRN_GAME LOG_STREAM(warn, log_server)
#define LOG_GAME LOG_STREAM(info, log_server)
#define DBG_GAME LOG_STREAM(debug, log_server)

static lg::log_domain log_config("config");
#define WRN_CONFIG LOG_STREAM(warn, log_config)

namespace
{
struct split_conv_impl
{
	void operator()(std::vector<int>& res, const simple_wml::string_span& span)
	{
		if(!span.empty()) {
			res.push_back(span.to_int());
		}
	}
};

template<typename TResult, typename TConvert>
std::vector<TResult> split(const simple_wml::string_span& val, TConvert conv, const char c = ',')
{
	std::vector<TResult> res;

	simple_wml::string_span::const_iterator i1 = val.begin();
	simple_wml::string_span::const_iterator i2 = i1;

	while(i2 != val.end()) {
		if(*i2 == c) {
			conv(res, simple_wml::string_span(i1, i2));
			++i2;
			i1 = i2;
		} else {
			++i2;
		}
	}

	conv(res, simple_wml::string_span(i1, i2));
	return res;
}
}

namespace wesnothd
{
template<typename Container>
void send_to_players(simple_wml::document& data, const Container& players, socket_ptr exclude = socket_ptr())
{
	for(const auto& player : players) {
		if(player != exclude) {
			send_to_player(player, data);
		}
	}
}

int game::id_num = 1;

void game::missing_user(socket_ptr /*socket*/, const std::string& func) const
{
	WRN_GAME << func << "(): Could not find user (socket:\t<some C++ pointer>"
			 << ") in player_info_ in game:\t\"" << name_ << "\" (" << id_ << ")\n";
}

game::game(player_connections& player_connections,
		const socket_ptr& host,
		const std::string& name,
		bool save_replays,
		const std::string& replay_save_path)
	: player_connections_(player_connections)
	, id_(id_num++)
	, name_(name)
	, password_()
	, owner_(host)
	, players_()
	, observers_()
	, muted_observers_()
	, sides_()
	, side_controllers_()
	, nsides_(0)
	, started_(false)
	, level_()
	, history_()
	, description_(nullptr)
	, end_turn_(0)
	, num_turns_(0)
	, all_observers_muted_(false)
	, bans_()
	, termination_()
	, save_replays_(save_replays)
	, replay_save_path_(replay_save_path)
	, rng_()
	, last_choice_request_id_(-1) /* or maybe 0 ? it shouldn't matter*/
{
	assert(owner_);
	players_.push_back(owner_);

	const auto iter = player_connections_.find(owner_);
	if(iter == player_connections_.end()) {
		missing_user(owner_, __func__);
		return;
	}

	// Mark the host as unavailable in the lobby.
	iter->info().mark_available(id_, name_);
	iter->info().set_status(player::PLAYING);
}

game::~game()
{
	try {
		save_replay();

		for(const socket_ptr& user_ptr : all_game_users()) {
			remove_player(user_ptr, false, true);
		}

		clear_history();
	} catch(...) {
	}
}

/// returns const so that operator [] won't create empty keys if not existent
static const simple_wml::node& get_multiplayer(const simple_wml::node& root)
{
	if(const simple_wml::node* multiplayer = root.child("multiplayer")) {
		return *multiplayer;
	} else {
		ERR_GAME << "no [multiplayer] found. Returning root\n";
		return root;
	}
}

bool game::allow_observers() const
{
	return get_multiplayer(level_.root())["observer"].to_bool(true);
}

bool game::registered_users_only() const
{
	return get_multiplayer(level_.root())["registered_users_only"].to_bool(true);
}

bool game::is_observer(const socket_ptr& player) const
{
	return std::find(observers_.begin(), observers_.end(), player) != observers_.end();
}

bool game::is_muted_observer(const socket_ptr& player) const
{
	if(!is_observer(player)) {
		return false;
	}

	if(all_observers_muted_) {
		return true;
	}

	return std::find(muted_observers_.begin(), muted_observers_.end(), player) != muted_observers_.end();
}

bool game::is_player(const socket_ptr& player) const
{
	return std::find(players_.begin(), players_.end(), player) != players_.end();
}

namespace
{
std::string describe_turns(int turn, int num_turns)
{
	std::ostringstream buf;
	buf << turn;

	// If the game has a turn limit.
	if(num_turns > -1) {
		buf << '/' << num_turns;
	}

	return buf.str();
}

} // anon namespace

std::string game::username(const socket_ptr& player) const
{
	const auto iter = player_connections_.find(player);
	if(iter != player_connections_.end()) {
		return iter->info().name();
	}

	return "(unknown)";
}

std::string game::list_users(user_vector users, const std::string& func) const
{
	std::string list;

	for(const user_vector::value_type& user : users) {
		const auto iter = player_connections_.find(user);

		if(iter != player_connections_.end()) {
			if(!list.empty()) {
				list += ", ";
			}

			list += iter->info().name();
		} else {
			missing_user(user, func);
		}
	}

	return list;
}

void game::perform_controller_tweaks()
{
	const simple_wml::node::child_list& sides = get_sides_list();

	DBG_GAME << "****\n Performing controller tweaks. sides = " << std::endl;
	DBG_GAME << debug_sides_info() << std::endl;
	DBG_GAME << "****" << std::endl;

	update_side_data(); // Necessary to read the level_ and get sides_, etc. updated to match

	for(unsigned side_index = 0; side_index < sides.size(); ++side_index) {
		simple_wml::node& side = *sides[side_index];

		if(side["controller"] != "null") {
			if(sides_[side_index] == 0) {
				sides_[side_index] = owner_;
				std::stringstream msg;
				msg << "Side " << side_index + 1
					<< " had no controller during controller tweaks! The host was assigned control.";

				LOG_GAME << msg.str() << " (game id: " << id_ << ")\n";
				send_and_record_server_message(msg.str());
			}

			const auto user = player_connections_.find(sides_[side_index]);
			std::string user_name = "null (server missing user)";
			if(user == player_connections_.end()) {
				missing_user(user->socket(), __func__);
			} else {
				user_name = username(user->socket());
			}

			// Issue change_controller command, transfering this side to its owner with proper name and controller.
			// Ensures that what the server now thinks is true is effected on all of the clients.
			//
			// We use the "player_left" field as follows. Normally change_controller sends one message to the owner,
			// and one message to everyone else. In case that a player drops, the owner is gone and should not get
			// a message, instead the host gets a [side_drop] message.
			//
			// In the server controller tweaks, we want to avoid sending controller change messages to the host.
			// Doing this has the negative consequence that all of the AI side names are given the owners name.
			// Therefore, if the side belongs to the host, we pass player_left = true, otherwise player_left = false.
			change_controller(side_index, sides_[side_index], user_name, sides_[side_index] == owner_);

			// next line change controller types found in level_ to be what is appropriate for an observer at game
			// start.
			side.set_attr("is_local", "no");

			if(sides_[side_index] == 0) {
				std::stringstream msg;
				msg << "Side " << side_index + 1 << " had no controller AFTER controller tweaks! Ruh Roh!";
				LOG_GAME << msg.str() << " (game id: " << id_ << ")\n";
			}
		}
	}

	// This is the last time that update_side_data will actually run, as now the game will start and
	// started_ will be true.
	update_side_data();

	// TODO: Does it matter that the server is telling the host to change a bunch of sides?
	// According to playturn.cpp, the host should ignore all such messages. Still might be better
	// not to send them at all, although not if it complicates the server code.
}

void game::start_game(const socket_ptr& starter)
{
	const simple_wml::node::child_list& sides = get_sides_list();
	DBG_GAME << "****\n Starting game. sides = " << std::endl;
	DBG_GAME << debug_sides_info() << std::endl;
	DBG_GAME << "****" << std::endl;

	started_ = true;
	// Prevent inserting empty keys when reading.
	const simple_wml::node& multiplayer = get_multiplayer(level_.root());

	const bool save = multiplayer["savegame"].to_bool();
	LOG_GAME
		<< client_address(starter) << "\t" << player_connections_.find(starter)->name() << "\t"
		<< "started" << (save ? " reloaded" : "") << " game:\t\"" << name_ << "\" (" << id_
		// << ") with: " << list_users(players_, __func__) << ". Settings: map: " << s["id"]
		<< ") with: " << list_users(players_, __func__)
		<< ". Settings: map: " << multiplayer["mp_scenario"]
		// << "\tera: "       << (s.child("era") ? (*s.child("era"))["id"] : "")
		<< "\tera: "       << multiplayer["mp_era"]
		<< "\tXP: "        << multiplayer["experience_modifier"]
		<< "\tGPV: "       << multiplayer["mp_village_gold"]
		<< "\tfog: "       << multiplayer["mp_fog"]
		<< "\tshroud: "    << multiplayer["mp_shroud"]
		<< "\tobservers: " << multiplayer["observer"]
		<< "\tshuffle: "   << multiplayer["shuffle_sides"]
		<< "\ttimer: "     << multiplayer["mp_countdown"]
		<< (multiplayer["mp_countdown"].to_bool()
			? "\treservoir time: "   + multiplayer["mp_countdown_reservoir_time"].to_string()
				+ "\tinit time: "    + multiplayer["mp_countdown_init_time"].to_string()
				+ "\taction bonus: " + multiplayer["mp_countdown_action_bonus"].to_string()
				+ "\tturn bonus: "   + multiplayer["mp_countdown_turn_bonus"].to_string()
			: "")
		<< "\n";


	for(unsigned side_index = 0; side_index < sides.size(); ++side_index) {
		simple_wml::node& side = *sides[side_index];

		if(side["controller"] != "null") {
			if(side_index >= sides_.size()) {
				continue;
			}

			if(sides_[side_index] == 0) {
				std::stringstream msg;
				msg << "Side " << side_index + 1
				    << " has no controller but should! The host needs to assign control for the game to proceed past "
					   "that side's turn.";

				LOG_GAME << msg.str() << " (game id: " << id_ << ")\n";
				send_and_record_server_message(msg.str());
			}
		}
	}

	DBG_GAME << "Number of sides: " << nsides_ << "\n";
	int turn = 1;
	int side = 0;

	// Savegames have a snapshot that tells us which side starts.
	if(const simple_wml::node* snapshot = level_.root().child("snapshot")) {
		turn = lexical_cast_default<int>((*snapshot)["turn_at"], 1);
		side = lexical_cast_default<int>((*snapshot)["playing_team"], 0);
		LOG_GAME << "Reload from turn: " << turn << ". Current side is: " << side + 1 << ".\n";
	}

	end_turn_ = (turn - 1) * nsides_ + side - 1;
	num_turns_ = lexical_cast_default<int>((*starting_pos(level_.root()))["turns"], -1);

	end_turn();
	clear_history();

	// Send [observer] tags for all observers that are already in the game.
	send_observerjoins();
}

void game::update_game()
{
	started_ = false;
	description_->set_attr("turn", "");

	update_side_data();
	describe_slots();
}

bool game::send_taken_side(simple_wml::document& cfg, const simple_wml::node* side) const
{
	const size_t side_index = (*side)["side"].to_int() - 1;

	// Negative values are casted (int -> size_t) to very high values to this check will fail for them too.
	if(side_index >= sides_.size()) {
		return false;
	}

	if(sides_[side_index] != 0) {
		return false;
	}

	// We expect that the host will really use our proposed side number. (He could do different...)
	cfg.root().set_attr_dup("side", (*side)["side"]);

	// Tell the host which side the new player should take.
	send_to_player(owner_, cfg);
	return true;
}

bool game::take_side(const socket_ptr& user)
{
	DBG_GAME << "take_side...\n";

	if(started_) {
		return false;
	}

	simple_wml::document cfg;
	cfg.root().set_attr_dup("name", player_connections_.find(user)->name().c_str());

	// FIXME: The client code (multiplayer.wait.cpp) the host code (connect_engine.cpp) and the server code
	// (this file) has this code to figure out a fitting side for new players, this is clearly too much.
	// Check if we can figure out a fitting side.
	const simple_wml::node::child_list& sides = get_sides_list();

	for(const simple_wml::node* side : sides) {
		if(((*side)["controller"] == "human" || (*side)["controller"] == "reserved")
				&& (*side)["current_player"] == player_connections_.find(user)->name().c_str()) {

			if(send_taken_side(cfg, side)) {
				return true;
			}
		}
	}

	// If there was no fitting side just take the first available.
	for(const simple_wml::node* side : sides) {
		if((*side)["controller"] == "human") {
			if(send_taken_side(cfg, side)) {
				return true;
			}
		}
	}

	DBG_GAME << "take_side: there are no more sides available\n";

	// If we get here we couldn't find a side to take
	return false;
}

void game::reset_sides()
{
	side_controllers_.clear();
	sides_.clear();

	nsides_ = get_sides_list().size();

	side_controllers_.resize(nsides_);
	sides_.resize(nsides_);
}

void game::update_side_data()
{
	// Added by iceiceice: since level_ will now reflect how an observer views the replay start
	// position and not the current position, the sides_, side_controllers_, players_ info should
	// not be updated from the level_ after the game has started. Controller changes are now stored
	// in the history, so an observer that joins will get up to date that way.
	if(started_) {
		return;
	}

	DBG_GAME << "update_side_data...\n";
	DBG_GAME << debug_player_info();

	// Remember everyone that is in the game.
	const user_vector users = all_game_users();

	players_.clear();
	observers_.clear();

	reset_sides();

	const simple_wml::node::child_list& level_sides = get_sides_list();

	// For each user:
	// * Find the username.
	// * Find the side this username corresponds to.
	for(const socket_ptr& user : users) {
		auto iter = player_connections_.find(user);
		if(iter == player_connections_.end()) {
			missing_user(user, __func__);
			continue;
		}

		bool side_found = false;
		for(unsigned side_index = 0; side_index < level_sides.size(); ++side_index) {
			const simple_wml::node* side = level_sides[side_index];

			if(side_index >= sides_.size() || sides_[side_index] != 0) {
				continue;
			}

			const simple_wml::string_span& player_id = (*side)["player_id"];
			const simple_wml::string_span& controller = (*side)["controller"];

			if(player_id == iter->info().name().c_str()) {
				// We found invalid [side] data. Some message would be cool.
				if(controller != "human" && controller != "ai") {
					continue;
				}

				side_controllers_[side_index].parse(controller);
				sides_[side_index] = user;
				side_found = true;
			} else if(user == owner_ && (controller == "null")) {
				// the *user == owner_ check has no effect,
				// it's just an optimisation so that we only do this once.
				side_controllers_[side_index].parse(controller);
			}
		}

		if(side_found) {
			players_.push_back(user);
			iter->info().set_status(player::PLAYING);
		} else {
			observers_.push_back(user);
			iter->info().set_status(player::OBSERVING);
		}
	}

	DBG_GAME << debug_player_info();
}

void game::transfer_side_control(const socket_ptr& sock, const simple_wml::node& cfg)
{
	DBG_GAME << "transfer_side_control...\n";

	if(!is_player(sock) && sock != owner_) {
		send_server_message("You cannot change controllers: not a player.", sock);
		return;
	}

	// Check the side number.
	const unsigned int side_num = cfg["side"].to_int();
	if(side_num < 1 || side_num > sides_.size()) {
		std::ostringstream msg;
		msg << "The side number has to be between 1 and " << sides_.size() << ".";
		send_server_message(msg.str(), sock);
		return;
	}

	if(side_num > get_sides_list().size()) {
		send_server_message("Invalid side number.", sock);
		return;
	}

	const simple_wml::string_span& newplayer_name = cfg["player"];
	const socket_ptr& old_player = sides_[side_num - 1];
	const auto oldplayer = player_connections_.find(old_player);
	if(oldplayer == player_connections_.end()) {
		missing_user(old_player, __func__);
	}

	const std::string old_player_name = username(old_player);

	// Not supported anymore.
	if(newplayer_name.empty()) {
		std::stringstream msg;
		msg << "Recived invalid [change_controller] with no player= attribute specified";
		DBG_GAME << msg.str() << "\n";
		send_server_message(msg.str(), sock);
		return;
	}

	// Check if the sender actually owns the side he gives away or is the host.
	if(!(sock == old_player || sock == owner_)) {
		std::stringstream msg;
		msg << "You can't give away side " << side_num << ". It's controlled by '" << old_player_name << "' not you.";
		DBG_GAME << msg.str() << "\n";
		send_server_message(msg.str(), sock);
		return;
	}

	// find the player that is passed control
	socket_ptr newplayer = find_user(newplayer_name);

	// Is he in this game?
	if(player_connections_.find(newplayer) == player_connections_.end() || !is_member(newplayer)) {
		send_server_message(newplayer_name.to_string() + " is not in this game", sock);
		return;
	}

	if(newplayer == old_player) {
		std::stringstream msg;
		msg << "That's already " << newplayer_name << "'s side, silly.";
		send_server_message(msg.str(), sock);
		return;
	}

	sides_[side_num - 1].reset();

	// If the old player lost his last side, make him an observer.
	if(std::find(sides_.begin(), sides_.end(), old_player) == sides_.end() && is_player(old_player)) {
		observers_.push_back(old_player);

		player_connections_.find(old_player)->info().set_status(player::OBSERVING);
		players_.erase(std::remove(players_.begin(), players_.end(), old_player), players_.end());

		// Tell others that the player becomes an observer.
		send_and_record_server_message(old_player_name + " becomes an observer.");

		// Update the client side observer list for everyone except old player.
		simple_wml::document observer_join;
		observer_join.root().add_child("observer").set_attr_dup("name", old_player_name.c_str());
		send_data(observer_join, old_player);
	}

	change_controller(side_num - 1, newplayer, player_connections_.find(newplayer)->info().name(), false);

	// If we gave the new side to an observer add him to players_.
	if(is_observer(newplayer)) {
		players_.push_back(newplayer);
		player_connections_.find(newplayer)->info().set_status(player::PLAYING);
		observers_.erase(std::remove(observers_.begin(), observers_.end(), newplayer), observers_.end());
		// Send everyone but the new player the observer_quit message.
		send_observerquit(newplayer);
	}
}

void game::change_controller(
		const size_t side_index, const socket_ptr& sock, const std::string& player_name, const bool player_left)
{
	DBG_GAME << __func__ << "...\n";

	const std::string& side = lexical_cast_default<std::string, size_t>(side_index + 1);
	sides_[side_index] = sock;

	if(player_left && side_controllers_[side_index] == CONTROLLER::AI) {
		// Automatic AI side transfer.
	} else {
		if(started_) {
			send_and_record_server_message(player_name + " takes control of side " + side + ".");
		}
	}

	simple_wml::document response;
	simple_wml::node& change = response.root().add_child("change_controller");

	change.set_attr("side", side.c_str());
	change.set_attr("player", player_name.c_str());

	// Tell everyone but the new player that this side's controller changed.
	change.set_attr("controller", side_controllers_[side_index].to_cstring());
	change.set_attr("is_local", "no");

	send_data(response, sock);

	if(started_) { // this is added instead of the if (started_) {...} below
		// the purpose of these records is so that observers, replay viewers, get controller updates correctly
		record_data(response.clone());
	}

	// Tell the new player that he controls this side now.
	// Just don't send it when the player left the game. (The host gets the
	// side_drop already.)
	if(!player_left) {
		change.set_attr("is_local", "yes");
		send_to_player(sock, response);
	}
}

void game::notify_new_host()
{
	const std::string owner_name = username(owner_);
	simple_wml::document cfg;
	cfg.root().add_child("host_transfer");

	std::string message = owner_name + " has been chosen as the new host.";
	send_to_player(owner_, cfg);
	send_and_record_server_message(message);
}

bool game::describe_slots()
{
	if(started_ || description_ == nullptr) {
		return false;
	}

	int available_slots = 0;
	int num_sides = get_sides_list().size();
	int i = 0;

	for(const simple_wml::node* side : get_sides_list()) {
		if(((*side)["allow_player"].to_bool(true) == false) || (*side)["controller"] == "null") {
			num_sides--;
		} else if(sides_[i] == 0) {
			++available_slots;
		}

		++i;
	}

	std::ostringstream buf;
	buf << available_slots << '/' << num_sides;
	std::string descr = buf.str();

	if((*description_)["slots"] != descr) {
		description_->set_attr_dup("slots", descr.c_str());
		return true;
	} else {
		return false;
	}
}

bool game::player_is_banned(const socket_ptr& sock) const
{
	auto ban = std::find(bans_.begin(), bans_.end(), client_address(sock));
	return ban != bans_.end();
}

void game::mute_all_observers()
{
	all_observers_muted_ = !all_observers_muted_;
	if(all_observers_muted_) {
		send_and_record_server_message("All observers have been muted.");
	} else {
		send_and_record_server_message("Muting of all observers has been removed.");
	}
}

void game::send_muted_observers(const socket_ptr& user) const
{
	if(all_observers_muted_) {
		send_server_message("All observers are muted.", user);
		return;
	}

	std::string muted_nicks = list_users(muted_observers_, __func__);

	send_server_message("Muted observers: " + muted_nicks, user);
}

void game::mute_observer(const simple_wml::node& mute, const socket_ptr& muter)
{
	if(muter != owner_) {
		send_server_message("You cannot mute: not the game host.", muter);
		return;
	}

	const simple_wml::string_span& username = mute["username"];
	if(username.empty()) {
		send_muted_observers(muter);
		return;
	}

	const socket_ptr& user = find_user(username);

	/**
	 * @todo FIXME: Maybe rather save muted nicks as a set of strings and
	 * also allow muting of usernames not in the game.
	 */
	if(!user || !is_observer(user)) {
		send_server_message("Observer '" + username.to_string() + "' not found.", muter);
		return;
	}

	// Prevent muting ourselves.
	if(user == muter) {
		send_server_message("Don't mute yourself, silly.", muter);
		return;
	}

	if(is_muted_observer(user)) {
		send_server_message(username.to_string() + " is already muted.", muter);
		return;
	}

	LOG_GAME << client_address(muter) << "\t" << game::username(muter) << " muted: " << username << " ("
	         << client_address(user) << ")\tin game:\t\"" << name_ << "\" (" << id_ << ")\n";

	muted_observers_.push_back(user);
	send_and_record_server_message(username.to_string() + " has been muted.");
}

void game::unmute_observer(const simple_wml::node& unmute, const socket_ptr& unmuter)
{
	if(unmuter != owner_) {
		send_server_message("You cannot unmute: not the game host.", unmuter);
		return;
	}

	const simple_wml::string_span& username = unmute["username"];
	if(username.empty()) {
		muted_observers_.clear();
		send_and_record_server_message("Everyone has been unmuted.");
		return;
	}

	const socket_ptr& user = find_user(username);
	if(!user || !is_observer(user)) {
		send_server_message("Observer '" + username.to_string() + "' not found.", unmuter);
		return;
	}

	if(!is_muted_observer(user)) {
		send_server_message(username.to_string() + " is not muted.", unmuter);
		return;
	}

	LOG_GAME << client_address(unmuter) << "\t" << game::username(unmuter) << " unmuted: " << username << " ("
	         << client_address(user) << ")\tin game:\t\"" << name_ << "\" (" << id_ << ")\n";

	muted_observers_.erase(std::remove(muted_observers_.begin(), muted_observers_.end(), user), muted_observers_.end());
	send_and_record_server_message(username.to_string() + " has been unmuted.");
}

void game::send_leave_game(const socket_ptr& user) const
{
	static simple_wml::document leave_game("[leave_game]\n[/leave_game]\n", simple_wml::INIT_COMPRESSED);
	send_to_player(user, leave_game);
}

socket_ptr game::kick_member(const simple_wml::node& kick, const socket_ptr& kicker)
{
	if(kicker != owner_) {
		send_server_message("You cannot kick: not the game host", kicker);
		return socket_ptr();
	}

	const simple_wml::string_span& username = kick["username"];
	const socket_ptr& user = find_user(username);

	if(!user || !is_member(user)) {
		send_server_message("'" + username.to_string() + "' is not a member of this game.", kicker);
		return socket_ptr();
	} else if(user == kicker) {
		send_server_message("Don't kick yourself, silly.", kicker);
		return socket_ptr();
	} else if(player_connections_.find(user)->info().is_moderator()) {
		send_server_message("You're not allowed to kick a moderator.", kicker);
		return socket_ptr();
	}

	LOG_GAME << client_address(kicker) << "\t" << game::username(kicker) << "\tkicked: " << username << " ("
	         << client_address(user) << ")\tfrom game:\t\"" << name_ << "\" (" << id_ << ")\n";

	send_and_record_server_message(username.to_string() + " has been kicked.");

	// Tell the user to leave the game.
	send_leave_game(user);
	remove_player(user);
	return user;
}

socket_ptr game::ban_user(const simple_wml::node& ban, const socket_ptr& banner)
{
	if(banner != owner_) {
		send_server_message("You cannot ban: not the game host", banner);
		return socket_ptr();
	}

	const simple_wml::string_span& username = ban["username"];
	const socket_ptr& user = find_user(username);

	if(!user) {
		send_server_message("User '" + username.to_string() + "' not found.", banner);
		return socket_ptr();
	} else if(user == banner) {
		send_server_message("Don't ban yourself, silly.", banner);
		return socket_ptr();
	} else if(player_is_banned(user)) {
		send_server_message("'" + username.to_string() + "' is already banned.", banner);
		return socket_ptr();
	} else if(player_connections_.find(user)->info().is_moderator()) {
		send_server_message("You're not allowed to ban a moderator.", banner);
		return socket_ptr();
	}

	LOG_GAME << client_address(banner) << "\t" << game::username(banner) << "\tbanned: " << username << " ("
	         << client_address(user) << ")\tfrom game:\t\"" << name_ << "\" (" << id_ << ")\n";

	bans_.push_back(client_address(user));
	send_and_record_server_message(username.to_string() + " has been banned.");

	if(is_member(user)) {
		// tell the user to leave the game.
		send_leave_game(user);
		remove_player(user);
		return user;
	}

	// Don't return the user if he wasn't in this game.
	return socket_ptr();
}

void game::unban_user(const simple_wml::node& unban, const socket_ptr& unbanner)
{
	if(unbanner != owner_) {
		send_server_message("You cannot unban: not the game host.", unbanner);
		return;
	}

	const simple_wml::string_span& username = unban["username"];
	const socket_ptr& user = find_user(username);

	if(!user) {
		send_server_message("User '" + username.to_string() + "' not found.", unbanner);
		return;
	}

	if(!player_is_banned(user)) {
		send_server_message("'" + username.to_string() + "' is not banned.", unbanner);
		return;
	}

	LOG_GAME
		<< client_address(unbanner) << "\t" << player_connections_.find(unbanner)->info().name()
		<< "\tunbanned: " << username << " (" << client_address(user) << ")\tfrom game:\t\"" << name_ << "\" ("
		<< id_ << ")\n";

	bans_.erase(std::remove(bans_.begin(), bans_.end(), client_address(user)), bans_.end());
	send_and_record_server_message(username.to_string() + " has been unbanned.");
}

void game::process_message(simple_wml::document& data, const socket_ptr& user)
{
	if(!owner_) {
		ERR_GAME << "No owner in game::process_message" << std::endl;
	}

	simple_wml::node* const message = data.root().child("message");
	assert(message);
	message->set_attr_dup("sender", player_connections_.find(user)->info().name().c_str());

	const simple_wml::string_span& msg = (*message)["message"];
	chat_message::truncate_message(msg, *message);

	send_data(data, user, "game message");
}

bool game::is_legal_command(const simple_wml::node& command, const socket_ptr& user)
{
	const bool is_player = this->is_player(user);
	const bool is_host = user == owner_;
	const bool is_current = is_current_player(user);

	if(command.has_attr("from_side")) {
		const size_t from_side_index = command["from_side"].to_int() - 1;

		// Someone pretends to be the server...
		if(command["from_side"] == "server") {
			return false;
		}

		if(from_side_index >= sides_.size() || sides_[from_side_index] != user) {
			return false;
		}
	}

	if(is_current) {
		return true;
	}

	// Only single commands allowed.
	// NOTE: some non-dependent commands like move,attack.. might contain a [checkup] tag after their first data.
	// But those packages are only sent by the currently active player which we check above.
	if(!command.one_child()) {
		return false;
	}

	// Chatting is never an illegal command.
	if(command.child("speak")) {
		return true;
	}
	if(command.child("surrender")) {
		const simple_wml::string_span& sn = command.child("surrender")->attr("side_number");
		if(sn.is_null()) {
			return false;
		}

		size_t side_number = sn.to_int();
		if(side_number >= sides_.size() || sides_[side_number] != user) {
			return false;
		} else {
			return true;
		}
	}

	// AKA it's generated by get_user_input for example [global_variable]
	if(is_player && command.has_attr("dependent") && command.has_attr("from_side")) {
		return true;
	}

	if((is_player || is_host) && (
		command.child("label") ||
		command.child("clear_labels") ||
		command.child("rename") ||
		command.child("countdown_update")
	)) {
		return true;
	}

	return false;
}

bool game::process_turn(simple_wml::document& data, const socket_ptr& user)
{
	// DBG_GAME << "processing commands: '" << cfg << "'\n";
	if(!started_) {
		return false;
	}

	simple_wml::node* const turn = data.root().child("turn");
	bool turn_ended = false;

	// Any private 'speak' commands must be repackaged separate
	// to other commands, and re-sent, since they should only go
	// to some clients.
	bool repackage = false;
	int index = 0;
	std::vector<int> marked;

	const simple_wml::node::child_list& commands = turn->children("command");

	for(simple_wml::node* command : commands) {
		DBG_GAME << "game " << id_ << " received [" << (*command).first_child() << "] from player '" << username(user)
				 << "'(" << user << ") during turn " << end_turn_ << "\n";
		if(!is_legal_command(*command, user)) {
			LOG_GAME << "ILLEGAL COMMAND in game: " << id_ << " (((" << simple_wml::node_to_string(*command)
					 << ")))\n";

			std::stringstream msg;
			msg << "Removing illegal command '" << (*command).first_child().to_string() << "' from: " << username(user)
				<< ". Current player is: " << username(current_player()) << " (" << end_turn_ + 1 << "/" << nsides_
				<< ").";
			LOG_GAME << msg.str() << " (socket: " << current_player() << ") (game id: " << id_ << ")\n";
			send_and_record_server_message(msg.str());

			marked.push_back(index - marked.size());
		} else if((*command).child("speak")) {
			simple_wml::node& speak = *(*command).child("speak");
			if(!speak["to_sides"].empty() || is_muted_observer(user)) {
				DBG_GAME << "repackaging..." << std::endl;
				repackage = true;
			}

			const simple_wml::string_span& msg = speak["message"];
			chat_message::truncate_message(msg, speak);

			// Force the description to be correct,
			// to prevent spoofing of messages.
			speak.set_attr_dup("id", player_connections_.find(user)->info().name().c_str());

			// Also check the side for players.
			if(is_player(user)) {
				const size_t side_index = speak["side"].to_int() - 1;

				if(side_index >= sides_.size() || sides_[side_index] != user) {
					if(user == current_player()) {
						speak.set_attr_dup("side", lexical_cast_default<std::string>(current_side() + 1).c_str());
					} else {
						const auto s = std::find(sides_.begin(), sides_.end(), user);
						speak.set_attr_dup("side", lexical_cast_default<std::string>(s - sides_.begin() + 1).c_str());
					}
				}
			}
		} else if (command->child("surrender")) {
			size_t side_index = 0;

			for(auto s : sides_) {
				if(s == user) {
					break;
				}
				++side_index;
			}

			if(side_index < sides_.size()) {
				simple_wml::document cfg;
				std::string playername;
				cfg.root().set_attr("side", std::to_string(side_index + 1).c_str());

				// figure out who gets the surrendered side
				if(owner_ == user) {
					playername = username(sides_[(side_index + 1) % sides_.size()]);
				} else {
					playername = username(owner_);
				}

				cfg.root().set_attr("player", playername.c_str());
				transfer_side_control(user, cfg.root());
			}
			send_and_record_server_message(username(user) + " has surrendered.");
		} else if(is_current_player(user) && (*command).child("end_turn")) {
			turn_ended = end_turn();
		}

		++index;
	}

	for(const int j : marked) {
		turn->remove_child("command", j);
	}

	if(turn->no_children()) {
		return false;
	}

	if(!repackage) {
		record_data(data.clone());
		send_data(data, user, "game replay");
		return turn_ended;
	}

	for(simple_wml::node* command : commands) {
		simple_wml::node* const speak = (*command).child("speak");
		if(speak == nullptr) {
			simple_wml::document* mdata = new simple_wml::document;
			simple_wml::node& mturn = mdata->root().add_child("turn");
			(*command).copy_into(mturn.add_child("command"));
			send_data(*mdata, user, "game replay");
			record_data(mdata);
			continue;
		}

		const simple_wml::string_span& to_sides = (*speak)["to_sides"];

		// Anyone can send to the observer team.
		if(is_muted_observer(user) && to_sides != game_config::observer_team_name.c_str()) {
			send_server_message("You have been muted, others can't see your message!", user);
			continue;
		}

		std::unique_ptr<simple_wml::document> message(new simple_wml::document);
		simple_wml::node& message_turn = message->root().add_child("turn");
		simple_wml::node& message_turn_command = message_turn.add_child("command");
		speak->copy_into(message_turn_command.add_child("speak"));

		if(to_sides.empty()) {
			send_data(*message, user, "game message");
			record_data(message.release());
		} else if(to_sides == game_config::observer_team_name) {
			send_to_players(*message, observers_, user);
			record_data(message.release());
		} else {
			send_data_sides(*message, to_sides, user, "game message");
		}
	}

	return turn_ended;
}

void game::handle_random_choice(const simple_wml::node&)
{
	uint32_t seed = rng_.get_next_random();

	std::stringstream stream;
	stream << std::setfill('0') << std::setw(sizeof(uint32_t) * 2) << std::hex << seed;

	simple_wml::document* mdata = new simple_wml::document;
	simple_wml::node& turn = mdata->root().add_child("turn");
	simple_wml::node& command = turn.add_child("command");
	simple_wml::node& random_seed = command.add_child("random_seed");

	random_seed.set_attr_dup("new_seed", stream.str().c_str());

	command.set_attr("from_side", "server");
	command.set_attr("dependent", "yes");

	send_data(*mdata, socket_ptr(), "game replay");
	record_data(mdata);
}

void game::handle_controller_choice(const simple_wml::node& req)
{
	const size_t side_index = req["side"].to_int() - 1;
	CONTROLLER new_controller;
	CONTROLLER old_controller;

	if(!new_controller.parse(req["new_controller"])) {
		send_and_record_server_message(
			"Could not handle [request_choice] [change_controller] with invalid controller '" + req["new_controller"].to_string() + "'");
		return;
	}

	if(!old_controller.parse(req["old_controller"])) {
		send_and_record_server_message(
			"Could not handle [request_choice] [change_controller] with invalid controller '" + req["old_controller"].to_string() + "'");
		return;
	}

	if(old_controller != this->side_controllers_[side_index]) {
		send_and_record_server_message(
			"Found unexpected old_controller= '" + old_controller.to_string() + "' in [request_choice] [change_controller]");
	}

	if(side_index >= sides_.size()) {
		send_and_record_server_message(
			"Could not handle [request_choice] [change_controller] with invalid side '" + req["side"].to_string() + "'");
		return;
	}

	const bool was_null = this->side_controllers_[side_index] == CONTROLLER::EMPTY;
	const bool becomes_null = new_controller == CONTROLLER::EMPTY;

	if(was_null) {
		assert(sides_[side_index] == 0);
		sides_[side_index] = current_player();
	}

	if(becomes_null) {
		sides_[side_index].reset();
	}

	side_controllers_[side_index] = new_controller;

	simple_wml::document* mdata = new simple_wml::document;
	simple_wml::node& turn = mdata->root().add_child("turn");
	simple_wml::node& command = turn.add_child("command");
	simple_wml::node& change_controller_wml = command.add_child("change_controller_wml");

	change_controller_wml.set_attr("controller", new_controller.to_cstring());
	change_controller_wml.set_attr("is_local", "yes");

	command.set_attr("from_side", "server");
	command.set_attr("dependent", "yes");

	// Calling send_to_one to 0 connect causes the package to be sent to all clients.
	if(sides_[side_index] != 0) {
		send_to_player(sides_[side_index], *mdata);
	}

	change_controller_wml.set_attr("is_local", "no");

	send_data(*mdata, sides_[side_index], "game replay");
	record_data(mdata);
}

void game::handle_choice(const simple_wml::node& data, const socket_ptr& user)
{
	// note, that during end turn events, it's side=1 for the server but side= side_count() on the clients.

	// Otherwise we allow observers to cause OOS for the playing clients by sending
	// server choice requests based on incompatible local changes. To solve this we block
	// server choice requests from observers.
	if(!started_) {
		return;
	}

	if(user != owner_ && !is_player(user)) {
		return;
	}

	int request_id = lexical_cast_default<int>(data["request_id"], -10);
	if(request_id <= last_choice_request_id_) {
		// We gave already an anwer to this request.
		return;
	}

	DBG_GAME << "answering seed request " << request_id << " by player "
			 << player_connections_.find(user)->info().name() << "(" << user << ")" << std::endl;
	last_choice_request_id_ = request_id;

	if(const simple_wml::node* rand = data.child("random_seed")) {
		handle_random_choice(*rand);
	} else if(const simple_wml::node* ccw = data.child("change_controller_wml")) {
		handle_controller_choice(*ccw);
	} else {
		send_and_record_server_message("Found unknown server choice request: [" + data.first_child().to_string() + "]");
	}
}

void game::process_whiteboard(simple_wml::document& data, const socket_ptr& user)
{
	if(!started_ || !is_player(user)) {
		return;
	}

	const simple_wml::node& wb_node = *data.child("whiteboard");

	// Ensure "side" attribute match with user
	const simple_wml::string_span& to_sides = wb_node["to_sides"];
	size_t const side_index = wb_node["side"].to_int() - 1;

	if(side_index >= sides_.size() || sides_[side_index] != user) {
		std::ostringstream msg;
		msg << "Ignoring illegal whiteboard data, sent from user '" << player_connections_.find(user)->info().name()
		    << "' which had an invalid side '" << side_index + 1 << "' specified" << std::endl;

		const std::string& msg_str = msg.str();

		LOG_GAME << msg_str << std::endl;
		send_and_record_server_message(msg_str);
		return;
	}

	send_data_sides(data, to_sides, user, "whiteboard");
}

void game::process_change_turns_wml(simple_wml::document& data, const socket_ptr& user)
{
	if(!started_ || !is_player(user)) {
		return;
	}

	const simple_wml::node& ctw_node = *data.child("change_turns_wml");
	const int current_turn = ctw_node["current"].to_int();
	const int num_turns = ctw_node["max"].to_int();
	if(num_turns > 10000 || current_turn > 10000) {
		// ignore this to prevent errors related to integer overflow.
		return;
	}

	set_current_turn(current_turn);
	num_turns_ = num_turns;

	assert(static_cast<int>(this->current_turn()) == current_turn);
	description_->set_attr_dup("turn", describe_turns(current_turn, num_turns_).c_str());
	// Dont send or store this change, all players should have gotten it by wml.
}

bool game::end_turn()
{
	// It's a new turn every time each side in the game ends their turn.
	++end_turn_;

	bool turn_ended = false;
	if((current_side()) == 0) {
		turn_ended = true;
	}

	// Skip over empty sides.
	for(int i = 0; i < nsides_ && side_controllers_[current_side()] == CONTROLLER::EMPTY; ++i) {
		++end_turn_;

		if(current_side() == 0) {
			turn_ended = true;
		}
	}

	if(!turn_ended) {
		return false;
	}

	if(description_ == nullptr) {
		return false;
	}

	description_->set_attr_dup("turn", describe_turns(current_turn(), num_turns_).c_str());
	return true;
}

///@todo differentiate between "observers not allowed" and "player already in the game" errors.
//      maybe return a string with an error message.
bool game::add_player(const socket_ptr& player, bool observer)
{
	if(is_member(player)) {
		ERR_GAME << "ERROR: Player is already in this game. (socket: " << player << ")\n";
		return false;
	}

	socket_ptr user = player;

	DBG_GAME << debug_player_info();

	bool became_observer = false;
	if(!started_ && !observer && take_side(user)) {
		DBG_GAME << "adding player...\n";
		players_.push_back(player);

		player_connections_.find(user)->info().set_status(player::PLAYING);

		send_and_record_server_message(player_connections_.find(user)->info().name() + " has joined the game.", player);
	} else if(!allow_observers() && !player_connections_.find(user)->info().is_moderator()) {
		return false;
	} else {
		if(!observer) {
			became_observer = true;
			observer = true;
		}

		DBG_GAME << "adding observer...\n";
		observers_.push_back(player);
		if(!allow_observers()) {
			send_and_record_server_message(
				player_connections_.find(user)->info().name() + " is now observing the game.", player);
		}

		simple_wml::document observer_join;
		observer_join.root()
			.add_child("observer")
			.set_attr_dup("name", player_connections_.find(user)->info().name().c_str());

		// Send observer join to everyone except the new observer.
		send_data(observer_join, player);
	}

	LOG_GAME
		<< client_address(player) << "\t" << player_connections_.find(user)->info().name() << "\tjoined game:\t\""
		<< name_ << "\" (" << id_ << ")" << (observer ? " as an observer" : "") << ". (socket: " << player
		<< ")\n";

	player_connections_.find(user)->info().mark_available(id_, name_);
	player_connections_.find(user)->info().set_status((observer) ? player::OBSERVING : player::PLAYING);
	DBG_GAME << debug_player_info();

	// Send the user the game data.
	send_to_player(player, level_);

	if(started_) {
		// Tell this player that the game has started
		static simple_wml::document start_game_doc("[start_game]\n[/start_game]\n", simple_wml::INIT_COMPRESSED);
		send_to_player(player, start_game_doc);

		// Send observer join of all the observers in the game to the new player
		// only once the game started. The client forgets about it anyway otherwise.
		send_observerjoins(player);

		// Send the player the history of the game to-date.
		send_history(player);
	} else {
		send_user_list();
	}

	const std::string clones = has_same_ip(player, observer);
	if(!clones.empty()) {
		send_and_record_server_message(
			player_connections_.find(user)->info().name() + " has the same IP as: " + clones);
	}

	if(became_observer) {
		// in case someone took the last slot right before this player
		send_server_message("You are an observer.", player);
	}

	return true;
}

bool game::remove_player(const socket_ptr& player, const bool disconnect, const bool destruct)
{
	if(!is_member(player)) {
		ERR_GAME << "ERROR: User is not in this game. (socket: " << player << ")\n";
		return false;
	}

	DBG_GAME << debug_player_info();
	DBG_GAME << "removing player...\n";

	const bool host = (player == owner_);
	const bool observer = is_observer(player);

	players_.erase(std::remove(players_.begin(), players_.end(), player), players_.end());
	observers_.erase(std::remove(observers_.begin(), observers_.end(), player), observers_.end());

	const bool game_ended = players_.empty() || (host && !started_);

	socket_ptr user = player;

	LOG_GAME
		<< client_address(user)
		<< "\t" << player_connections_.find(user)->info().name()
		<< ((game_ended && !(observer && destruct)) ? (started_ ? "\tended" : "\taborted") : "\thas left")
		<< " game:\t\"" << name_ << "\" (" << id_ << ")"
		<< (game_ended && started_ && !(observer && destruct)
			? " at turn: " + lexical_cast_default<std::string, size_t>(current_turn())
				+ " with reason: '" + termination_reason() + "'"
			: "")
		<< (observer ? " as an observer" : "") << (disconnect ? " and disconnected" : "") << ". (socket: " << user
		<< ")\n";

	if(game_ended && started_ && !(observer && destruct)) {
		send_server_message_to_all(player_connections_.find(user)->info().name() + " ended the game.", player);
	}

	if(game_ended || destruct) {
		return game_ended;
	}

	// Don't mark_available() since the player got already removed from the
	// games_and_users_list_.
	if(!disconnect) {
		player_connections_.find(user)->info().mark_available();
	}

	if(observer) {
		send_observerquit(user);
	} else {
		send_and_record_server_message(player_connections_.find(user)->info().name()
			+ (disconnect ? " has disconnected." : " has left the game."), player);
	}

	// If the player was host choose a new one.
	if(host) {
		owner_ = players_.front();
		notify_new_host();
	}

	bool ai_transfer = false;

	// Look for all sides the player controlled and drop them.
	// (Give them to the host.
	for(unsigned side_index = 0; side_index < sides_.size(); ++side_index) {
		const socket_ptr& side = sides_[side_index];

		if(side != player) {
			continue;
		}

		if(side_controllers_[side_index] == CONTROLLER::AI) {
			ai_transfer = true;
		}

		change_controller(side_index, owner_, username(owner_));

		// Check whether the host is actually a player and make him one if not.
		if(!is_player(owner_)) {
			DBG_GAME << "making the owner a player...\n";
			player_connections_.find(owner_)->info().set_status(player::PLAYING);
			observers_.erase(std::remove(observers_.begin(), observers_.end(), owner_), observers_.end());
			players_.push_back(owner_);
			send_observerquit(owner_);
		}

		// send the host a notification of removal of this side
		const std::string side_drop = lexical_cast_default<std::string, size_t>(side_index + 1);

		simple_wml::document drop;
		auto& node_side_drop = drop.root().add_child("side_drop");

		node_side_drop.set_attr("side_num", side_drop.c_str());
		node_side_drop.set_attr("controller", side_controllers_[side_index].to_cstring());

		DBG_GAME << "*** sending side drop: \n" << drop.output() << std::endl;

		send_to_player(owner_, drop);
	}

	if(ai_transfer) {
		send_and_record_server_message("AI sides transferred to host.");
	}

	DBG_GAME << debug_player_info();

	send_user_list(player);
	return false;
}

void game::send_user_list(const socket_ptr& exclude) const
{
	// If the game hasn't started yet, then send all players a list of the users in the game.
	if(started_ /*|| description_ == nullptr*/) {
		return;
	}

	simple_wml::document cfg;
	simple_wml::node& list = cfg.root();

	for(const socket_ptr& user_ptr : all_game_users()) {
		const auto pl = player_connections_.find(user_ptr);

		if(pl != player_connections_.end()) {
			simple_wml::node& user = list.add_child("user");

			// Don't need to duplicate pl->info().name().c_str() because the
			// document will be destroyed by the end of the function
			user.set_attr("name", pl->info().name().c_str());
			user.set_attr("host", is_owner(user_ptr) ? "yes" : "no");
			user.set_attr("observer", is_observer(user_ptr) ? "yes" : "no");
		}
	}

	send_data(cfg, exclude);
}

void game::load_next_scenario(const socket_ptr& user)
{
	send_server_message_to_all(player_connections_.find(user)->info().name() + " advances to the next scenario", user);

	simple_wml::document cfg_scenario;
	simple_wml::node& next_scen = cfg_scenario.root().add_child("next_scenario");
	level_.root().copy_into(next_scen);
	next_scen.set_attr("started", started_ ? "yes" : "no");

	DBG_GAME << "****\n loading next scenario for a client. sides info = " << std::endl;
	DBG_GAME << debug_sides_info() << std::endl;
	DBG_GAME << "****" << std::endl;

	//
	// Change the controller to match that client.
	//
	// FIXME: This breaks scenario transitions with mp connect screen shown.
	//
	// FIXME: This causes bugs, esp if controller have changed since the
	//        beginning of the next scenario
	//
	//        There are currently 2 possible ideas to fix this issue:
	//
	//          1) When the scenario starts, we store the controllers at that
	//             point and use that data when a client loads the the next
	//             scenario (here)
	//
	//          2) When a client loads the next scenario we send it the
	//             observers' starting point (meaning we don't change sides
	//             here), and then we send that side an automatic controller
	//             change later.
	//
	simple_wml::document doc_controllers;
	simple_wml::node& cfg_controllers = doc_controllers.root().add_child("controllers");

	for(const auto& side_user : sides_) {
		simple_wml::node& cfg_controller = cfg_controllers.add_child("controller");
		cfg_controller.set_attr("is_local", side_user == user ? "yes" : "no");
	}

	send_to_player(user, cfg_scenario);
	send_to_player(user, doc_controllers);

	// Send the player the history of the game to-date.
	send_history(user);

	// Send observer join of all the observers in the game to the user.
	send_observerjoins(user);
}

void game::send_data(simple_wml::document& data, const socket_ptr& exclude, std::string /*packet_type*/) const
{
	send_to_players(data, all_game_users(), exclude);
}

namespace
{
struct controls_side_helper
{
	const wesnothd::game& game_;
	const std::vector<int>& sides_;

	controls_side_helper(const wesnothd::game& game, const std::vector<int>& sides)
		: game_(game)
		, sides_(sides)
	{
	}

	bool operator()(socket_ptr user) const
	{
		return game_.controls_side(sides_, user);
	}
};
}

void game::send_data_sides(simple_wml::document& data,
		const simple_wml::string_span& sides,
		const socket_ptr& exclude,
		std::string /*packet_type*/) const
{
	std::vector<int> sides_vec = ::split<int>(sides, ::split_conv_impl());

	DBG_GAME << __func__ << "...\n";

	decltype(players_) filtered_players;

	std::copy_if(players_.begin(), players_.end(), std::back_inserter(filtered_players),
		controls_side_helper(*this, sides_vec));

	send_to_players(data, filtered_players, exclude);
}

bool game::controls_side(const std::vector<int>& sides, const socket_ptr& player) const
{
	for(int side : sides) {
		size_t side_index = side - 1;

		if(side_index < sides_.size() && sides_[side_index] == player) {
			return true;
		}
	}

	return false;
}

std::string game::has_same_ip(const socket_ptr& user, bool observer) const
{
	const user_vector users = observer ? players_ : all_game_users();
	const std::string ip = client_address(user);

	std::string clones;
	for(const socket_ptr& u : users) {
		if(ip == client_address(u) && user != u) {
			const auto pl = player_connections_.find(u);

			if(pl != player_connections_.end()) {
				clones += (clones.empty() ? "" : ", ") + pl->info().name();
			}
		}
	}

	return clones;
}

void game::send_observerjoins(const socket_ptr& sock) const
{
	for(const socket_ptr& ob : observers_) {
		if(ob == sock) {
			continue;
		}

		simple_wml::document cfg;
		cfg.root().add_child("observer").set_attr_dup("name", player_connections_.find(ob)->info().name().c_str());

		if(sock == socket_ptr()) {
			// Send to everyone except the observer in question.
			send_data(cfg, ob);
		} else {
			// Send to the (new) user.
			send_to_player(sock, cfg);
		}
	}
}

void game::send_observerquit(const socket_ptr& observer) const
{
	simple_wml::document observer_quit;

	// Don't need to dup the attribute because this document is short-lived.
	observer_quit.root()
		.add_child("observer_quit")
		.set_attr("name", player_connections_.find(observer)->info().name().c_str());

	send_data(observer_quit, observer);
}

void game::send_history(const socket_ptr& socket) const
{
	if(history_.empty()) {
		return;
	}

	// we make a new document based on converting to plain text and
	// concatenating the buffers.
	// TODO: Work out how to concentate buffers without decompressing.
	std::string buf;
	for(auto& h : history_) {
		buf += h.output();
	}

	try {
		simple_wml::document* doc = new simple_wml::document(buf.c_str(), simple_wml::INIT_STATIC);
		doc->compress();

		send_to_player(socket, *doc);

		history_.clear();
		history_.push_back(doc);
	} catch(simple_wml::error& e) {
		WRN_CONFIG << __func__ << ": simple_wml error: " << e.message << std::endl;
	}
}

static bool is_invalid_filename_char(char c)
{
	return !(isalnum(c) ||
		(c == '_') ||
		(c == '-') ||
		(c == '.') ||
		(c == '(') ||
		(c == ')') ||
		(c == '#') ||
		(c == ',') ||
		(c == '!') ||
		(c == '?') ||
		(c == '^') ||
		(c == '+') ||
		(c == '*') ||
		(c == ':') ||
		(c == '=') ||
		(c == '@') ||
		(c == '%') ||
		(c == '\'')
	);
}

void game::save_replay()
{
	if(!save_replays_ || !started_ || history_.empty()) {
		return;
	}

	std::string replay_commands;
	for(const auto& h : history_) {
		const simple_wml::node::child_list& turn_list = h.root().children("turn");

		for(const simple_wml::node* turn : turn_list) {
			replay_commands += simple_wml::node_to_string(*turn);
		}
	}

	history_.clear();

	std::stringstream name;
	name << (*starting_pos(level_.root()))["name"] << " Turn " << current_turn();

	std::stringstream replay_data;
	try {
		// level_.set_attr_dup("label", name.str().c_str());

		// Used by replays.wesnoth.org as of December 2017. No client usecases.
		level_.set_attr_dup("mp_game_title", name_.c_str());

		const bool has_old_replay = level_.child("replay") != nullptr;

		// If there is already a replay in the level_, which means this is a reloaded game,
		// then we dont need to add the [start] in the replay.
		replay_data
			<< level_.output()
			// This can result in having 2 [replay] at toplevel since level_ can contain one already. But the
			// client can handle this (simply merges them).
			<< "[replay]\n"
			// The [start] is generated at the clients and not sent over the network so we add it here.
			// It usualy contains some checkup data that is used to check whether the calculated results
			// match the ones calculated in the replay. But thats not necessary
			<< (has_old_replay ? "" : "\t[command]\n\t\t[start]\n\t\t[/start]\n\t[/command]\n")
			<< replay_commands << "[/replay]\n";

		name << " (" << id_ << ").bz2";

		std::string replay_data_str = replay_data.str();
		simple_wml::document replay(replay_data_str.c_str(), simple_wml::INIT_STATIC);

		std::string filename(name.str());

		std::replace(filename.begin(), filename.end(), ' ', '_');
		filename.erase(std::remove_if(filename.begin(), filename.end(), is_invalid_filename_char), filename.end());

		DBG_GAME << "saving replay: " << filename << std::endl;

		filesystem::scoped_ostream os(filesystem::ostream_file(replay_save_path_ + filename));
		(*os) << replay.output_compressed(true);

		if(!os->good()) {
			ERR_GAME << "Could not save replay! (" << filename << ")" << std::endl;
		}
	} catch(simple_wml::error& e) {
		WRN_CONFIG << __func__ << ": simple_wml error: " << e.message << std::endl;
	}
}

void game::record_data(simple_wml::document* data)
{
	data->compress();
	history_.push_back(data);
}

void game::clear_history()
{
	history_.clear();
}

void game::set_description(simple_wml::node* desc)
{
	description_ = desc;
	if(!password_.empty()) {
		description_->set_attr("password", "yes");
	}
}

void game::set_termination_reason(const std::string& reason)
{
	/*	if (reason == "out of sync") {
			simple_wml::string_span era;
			if (level_.child("era")) {
				era = level_.child("era")->attr("id");
			}
			termination_ = "out of sync - " + era.to_string();
		}*/
	if(termination_.empty()) {
		termination_ = reason;
	}
}

const user_vector game::all_game_users() const
{
	user_vector res;

	res.insert(res.end(), players_.begin(), players_.end());
	res.insert(res.end(), observers_.begin(), observers_.end());

	return res;
}

std::string game::debug_player_info() const
{
	std::stringstream result;
	result << "game id: " << id_ << "\n";

	//	result << "players_.size: " << players_.size() << "\n";
	for(const socket_ptr& p : players_) {
		const auto user = player_connections_.find(p);

		if(user != player_connections_.end()) {
			result << "player: " << user->info().name().c_str() << "\n";
		} else {
			result << "player: '" << p << "' not found\n";
		}
	}

	//	result << "observers_.size: " << observers_.size() << "\n";
	for(const socket_ptr& o : observers_) {
		const auto user = player_connections_.find(o);

		if(user != player_connections_.end()) {
			result << "observer: " << user->info().name().c_str() << "\n";
		} else {
			result << "observer: '" << o << "' not found\n";
		}
	}
	/*	result << "player_info_: begin\n";
		for (player_map::const_iterator info = player_info_->begin(); info != player_info_->end(); info++){
			result << info->second.name().c_str() << "\n";
		}
		result << "player_info_: end\n";*/
	return result.str();
}

std::string game::debug_sides_info() const
{
	std::stringstream result;
	result << "game id: " << id_ << "\n";
	const simple_wml::node::child_list& sides = get_sides_list();

	result << "\t\t level, server\n";

	for(const simple_wml::node* s : sides) {
		result
			<< "side " << (*s)["side"].to_int()
			<< " :\t" << (*s)["controller"].to_string()
			<< "\t, " << side_controllers_[(*s)["side"].to_int() - 1].to_cstring()
			<< "\t( " << sides_[(*s)["side"].to_int() - 1]
			<< ",\t" << (*s)["current_player"].to_string() << " )\n";
	}

	return result.str();
}

socket_ptr game::find_user(const simple_wml::string_span& name)
{
	const auto iter = player_connections_.get<name_t>().find(name.to_string());
	if(iter != player_connections_.get<name_t>().end()) {
		return iter->socket();
	} else {
		return socket_ptr();
	}
}

void game::send_and_record_server_message(const char* message, const socket_ptr& exclude)
{
	simple_wml::document* doc = new simple_wml::document;
	send_server_message(message, socket_ptr(), doc);
	send_data(*doc, exclude, "message");

	if(started_) {
		record_data(doc);
	} else {
		delete doc;
	}
}

void game::send_server_message_to_all(const char* message, const socket_ptr& exclude) const
{
	simple_wml::document doc;
	send_server_message(message, socket_ptr(), &doc);
	send_data(doc, exclude, "message");
}

void game::send_server_message(const char* message, const socket_ptr& sock, simple_wml::document* docptr) const
{
	simple_wml::document docbuf;
	if(docptr == nullptr) {
		docptr = &docbuf;
	}

	simple_wml::document& doc = *docptr;

	if(started_) {
		simple_wml::node& cmd = doc.root().add_child("turn");
		simple_wml::node& cfg = cmd.add_child("command");
		simple_wml::node& msg = cfg.add_child("speak");

		msg.set_attr("id", "server");
		msg.set_attr_dup("message", message);
	} else {
		simple_wml::node& msg = doc.root().add_child("message");

		msg.set_attr("sender", "server");
		msg.set_attr_dup("message", message);
	}

	if(sock) {
		send_to_player(sock, doc);
	}
}
} // namespace wesnothd
