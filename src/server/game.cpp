/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "../global.hpp"

#include "../game_config.hpp" // game_config::observer_team_name
#include "../log.hpp"
#include "../map.hpp" // gamemap::MAX_PLAYERS
#include "../serialization/string_utils.hpp" // utils::string_bool()

#include "game.hpp"
#include "player.hpp"

#include <algorithm>
#include <iostream>
#include <cassert>
#include <sstream>

#define ERR_GAME LOG_STREAM(err, mp_server)
#define WRN_GAME LOG_STREAM(warn, mp_server)
#define LOG_GAME LOG_STREAM(info, mp_server)
#define DBG_GAME LOG_STREAM(debug, mp_server)

namespace chat_message {

static void truncate_message(t_string& str) {
	const size_t max_message_length = 256;
	// The string send can contain utf-8 so truncate as wide_string otherwise
	// an corrupted utf-8 string can be returned.
	std::string tmp = str.str();
	utils::truncate_as_wstring(tmp, max_message_length);
	str = tmp;
}

} // end chat_message namespace

int game::id_num = 1;

game::game(player_map& players, const network::connection host, const std::string name)
	: player_info_(&players), id_(id_num++), name_(name), owner_(host),
	sides_(gamemap::MAX_PLAYERS), sides_taken_(gamemap::MAX_PLAYERS),
	side_controllers_(gamemap::MAX_PLAYERS), nsides_(0), started_(false),
	description_(NULL), end_turn_(0), all_observers_muted_(false)
{
	// Hack to handle the pseudo games lobby_ and not_logged_in_.
	if (owner_ == 0) return;
	players_.push_back(owner_);
	const player_map::iterator pl = player_info_->find(owner_);
	if (pl == player_info_->end()) {
		ERR_GAME << "ERROR: Could not find host in player_info_. (socket: "
			<< owner_ << ")\n";
		return;
	}
	// Mark the host as unavailable in the lobby.
	pl->second.mark_available(id_, name_);

}

bool game::allow_observers() const {
	return utils::string_bool(level_.get_attribute("observer"), true);
}

bool game::is_observer(const network::connection player) const {
	return std::find(observers_.begin(),observers_.end(),player) != observers_.end();
}

bool game::is_muted_observer(const network::connection player) const {
	if (is_observer(player)) {
		if (all_observers_muted_) return true;
	} else {
		return false;
	}
	return std::find(muted_observers_.begin(), muted_observers_.end(), player)
		!= muted_observers_.end();
}

bool game::is_player(const network::connection player) const {
	return std::find(players_.begin(),players_.end(),player) != players_.end();
}

namespace {
std::string describe_turns(int turn, const std::string& num_turns)
{
	char buf[50];
	snprintf(buf,sizeof(buf),"%d/",int(turn));

	if(num_turns == "-1") {
		return buf + std::string("-");
	} else {
		return buf + num_turns;
	}
}

}//anon namespace

void game::start_game(const player_map::const_iterator starter) {
	// If the game was already started we're actually advancing.
	const bool advance = started_;
	started_ = true;
	// Prevent inserting empty keys when reading.
	const config& s = level_;
	const bool save = utils::string_bool(s["savegame"]);
	LOG_GAME << network::ip_address(starter->first) << "\t"
		<< starter->second.name() << "\t" << (advance ? "advanced" : "started")
		<< (save ? " reloaded" : "") << " game:\t\"" << name_ << "\" (" << id_
		<< "). Settings: map: " << s["id"]
		<< "\tera: "       << (s.child("era") ? (*s.child("era"))["id"] : "")
		<< "\tXP: "        << s["experience_modifier"]
		<< "\tGPV: "       << s["mp_village_gold"]
		<< "\tfog: "       << s["mp_fog"]
		<< "\tshroud: "    << s["mp_shroud"]
		<< "\tobservers: " << s["observer"]
		<< "\ttimer: "     << s["mp_countdown"]
		<< (utils::string_bool(s["mp_countdown"]) ?
			"\treservoir time: " + s["mp_countdown_reservoir_time"] +
			"\tinit time: "      + s["mp_countdown_init_time"] +
			"\taction bonus: "   + s["mp_countdown_action_bonus"] +
			"\tturn bonus: "     + s["mp_countdown_turn_bonus"] : "")
		<< "\n";

	nsides_ = 0;
	// Set all side controllers to 'human' so that observers will understand
	// that they can't take control of any sides if they happen to have the
	// same name as one of the descriptions.
	for(config::child_itors sides = level_.child_range("side");
		sides.first != sides.second; ++sides.first)
	{
		nsides_++;
		if ((**sides.first)["controller"] != "null" && !advance) {
			(**sides.first)["controller"] = "human";
		}
	}
	DBG_GAME << "Number of sides: " << nsides_ << "\n";
	int turn = 1;
	int side = 0;
	// Savegames have a snapshot that tells us which side starts.
	if (s.child("snapshot")) {
		turn = lexical_cast_default<int>((*s.child("snapshot"))["turn_at"], 1);
		side = lexical_cast_default<int>((*s.child("snapshot"))["playing_team"], 0);
		LOG_GAME << "Reload from turn: " << turn
			<< ". Current side is: " << side + 1 << ".\n";
	}
	end_turn_ = (turn - 1) * nsides_ + side - 1;
	end_turn();
	history_.clear();
	if (advance) {
		// Re-assign sides to allow correct filtering of commands.
		update_side_data();
		// When the host advances tell everyone that the next scenario data is
		// available.
		send_data(config("notify_next_scenario"), starter->first);
	}
	// Send [observer] tags for all observers that are already in the game.
	send_observerjoins();
}

//! Figures out which side to take and tells that side to the game owner.
//! The owner then should send a [scenario_diff] that implements the side
//! change and a subsequent update_side_data() call makes it actually
//! happen.
//! First we look for a side where save_id= or current_player= matches the
//! new user's name then we search for the first controller="network" side.
bool game::take_side(const player_map::const_iterator user)
{
	DBG_GAME << "take_side...\n";
	DBG_GAME << debug_player_info();

	if (started_) return false;

	config cfg;
	cfg["name"] = user->second.name();
	cfg["faction"] = "random";
	cfg["leader"] = "random";
	cfg["gender"] = "random";
	size_t side_num;
	// Check if we can figure out a fitting side.
	const config::child_list& sides = level_.get_children("side");
	for(config::child_list::const_iterator side = sides.begin(); side != sides.end(); ++side) {
		if((**side)["controller"] == "network"
				&& ((**side)["save_id"] == user->second.name()
				|| (**side)["current_player"] == user->second.name()))
		{
			try {
				side_num = lexical_cast<size_t, std::string>((**side)["side"]);
			} catch (bad_lexical_cast&) { continue; }
			if (side_num < 1 || side_num > gamemap::MAX_PLAYERS) continue;
			side_controllers_[side_num - 1] = "network";
			sides_[side_num - 1] = user->first;
			sides_taken_[side_num - 1] = true;
			cfg["side"] = (**side)["side"];
			// Tell the host which side the new player should take.
			network::send_data(cfg, owner_, true);
			DBG_GAME << debug_player_info();
			return true;
		}
	}
	// If there was no fitting side just take the first available.
	for(config::child_list::const_iterator side = sides.begin(); side != sides.end(); ++side) {
		if((**side)["controller"] == "network") {
			//don't allow players to take sides in games with invalid side numbers
			try {
				side_num = lexical_cast<size_t, std::string>((**side)["side"]);
			} catch (bad_lexical_cast&) { continue; }
			if (side_num < 1 || side_num > gamemap::MAX_PLAYERS) continue;
			side_controllers_[side_num - 1] = "network";
			sides_[side_num - 1] = user->first;
			sides_taken_[side_num - 1] = true;
			cfg["side"] = (**side)["side"];
			// Tell the host which side the new player should take.
			network::send_data(cfg, owner_, true);
			DBG_GAME << debug_player_info();
			return true;
		}
	}
	//if we get here we couldn't find a side to take
	return false;
}

//! Resets the side configuration according to the scenario data.
void game::update_side_data() {
	DBG_GAME << "update_side_data...\n";
	DBG_GAME << debug_player_info();
	// Remember everyone that is in the game.
	const user_vector users = all_game_users();

	side_controllers_.clear();
	side_controllers_.resize(gamemap::MAX_PLAYERS);
	sides_taken_.clear();
	// Resize because we assume a default of 'false' for all sides later.
	sides_taken_.resize(gamemap::MAX_PLAYERS);
	sides_.clear();
	sides_.resize(gamemap::MAX_PLAYERS);
	players_.clear();
	observers_.clear();

	const config::child_itors level_sides = level_.child_range("side");
/*	for (config::child_iterator side = level_sides.first;
			side != level_sides.second; ++side)
		DBG_GAME << (**side);*/
	// For each user:
	// * Find the username.
	// * Find the side this username corresponds to.
	for (user_vector::const_iterator user = users.begin(); user != users.end(); ++user) {
		const player_map::const_iterator info = player_info_->find(*user);
		if (info == player_info_->end()) {
			ERR_GAME << "Game: " << id_
				<< " ERROR: unable to find user info for connection: "
				<< *user << "\n";
			continue;
		}

		bool side_found = false;
		for (config::child_iterator side = level_sides.first;
				side != level_sides.second; ++side)
		{
			size_t side_num;
			try {
				side_num = lexical_cast<size_t, std::string>((**side)["side"]);
			} catch (bad_lexical_cast&) { continue; }
			if (side_num < 1 || side_num > gamemap::MAX_PLAYERS
					|| sides_taken_[side_num - 1]) continue;

			if ((**side)["controller"] == "network") {
				side_controllers_[side_num - 1] = "network";
				if ((**side)["current_player"] == info->second.name()) {
					sides_[side_num - 1] = *user;
					sides_taken_[side_num - 1] = true;
					side_found = true;
				} else sides_taken_[side_num - 1] = false;
			} else if ((**side)["controller"] == "ai") {
				side_controllers_[side_num - 1] = "ai";
				sides_[side_num - 1] = owner_;
				sides_taken_[side_num - 1] = true;
				side_found = true;
			} else if ((**side)["controller"] == "human") {
				side_controllers_[side_num - 1] = "human";
				sides_[side_num - 1] = owner_;
				sides_taken_[side_num - 1] = true;
				side_found = true;
			} else if ((**side)["controller"] == "null") {
				side_controllers_[side_num - 1] = "null";
			}
		}
		if (side_found) {
			players_.push_back(*user);
		} else {
			observers_.push_back(*user);
		}
	}
	DBG_GAME << debug_player_info();
}

void game::transfer_side_control(const network::connection sock, const config& cfg) {
	DBG_GAME << "transfer_side_control...\n";
	if (!is_player(sock)) {
		network::send_data(construct_server_message(
				"You cannot change controllers: not a player."), sock, true);
		return;
	}

	//check, if this socket belongs to a player
	const user_vector::iterator pl = std::find(players_.begin(), players_.end(), sock);
	if (pl == players_.end()) {
		ERR_GAME << "ERROR: Not a player of this game. (socket: " << sock << ")\n";
		return;
	}
	const std::string& newplayer_name = cfg["player"];
	//find the player that is passed control
	player_map::const_iterator newplayer;
	for (newplayer = player_info_->begin(); newplayer != player_info_->end(); newplayer++) {
		if (newplayer->second.name() == newplayer_name) {
			break;
		}
	}
	// Is he in this game?
	if (newplayer == player_info_->end() || !is_member(newplayer->first)) {
		network::send_data(construct_server_message("Player/Observer not in this game."), sock, true);
		return;
	}
	// Check the side number.
	const std::string& side = cfg["side"];
	size_t side_num;
	try {
		side_num = lexical_cast<size_t, std::string>(side);
		if(side_num < 1 || side_num > gamemap::MAX_PLAYERS) {
			std::ostringstream msg;
			msg << "The side number has to be between 1 and " 
			    << gamemap::MAX_PLAYERS << ".";
			network::send_data(construct_server_message(msg.str()), sock, true);
			return;
		}
	}
	catch(bad_lexical_cast&) {
		network::send_data(construct_server_message("Not a side number."), sock, true);
		return;
	}
	if (side_num > level_.get_children("side").size()) {
		network::send_data(construct_server_message("Invalid side number."), sock, true);
		return;
	}

	network::connection old_player = sides_[side_num - 1];
	const std::string old_player_name =
			(player_info_->find(old_player) != player_info_->end()
			? player_info_->find(old_player)->second.name() : "");
	// Check if the sender actually owns the side he gives away or is the host.
	if (sock != old_player && sock != owner_) {
		std::stringstream msg;
		msg << "You can't give away side " << side_num << ". It's controlled by '"
			<< old_player_name << "' not you.";
		DBG_GAME << msg << "\n";
		network::send_data(construct_server_message(msg.str()), sock, true);
		return;
	}
	if (newplayer->first == old_player) {
		network::send_data(construct_server_message(
			"That's already " + newplayer_name + "'s side, silly."), sock, true);
		return;
	}
	sides_[side_num - 1] = 0;
	bool host_leave = false;
	// If the old player lost his last side, make him an observer.
	if (std::find(sides_.begin(), sides_.end(), old_player) == sides_.end()) {
		observers_.push_back(old_player);
		players_.erase(std::remove(players_.begin(), players_.end(), old_player), players_.end());
		// Tell others that the player becomes an observer.
		send_and_record_server_message(old_player_name + " becomes an observer.");
		// Update the client side observer list for everyone except old player.
		config observer_join;
		observer_join.add_child("observer")["name"] = old_player_name;
		send_data(observer_join, old_player);
		// If the old player was the host of the game, choose another player.
		if (old_player == owner_) {
			host_leave = true;
			if (players_.empty()) {
				owner_ = newplayer->first;
			} else {
				owner_ = players_.front();
			}
			notify_new_host();
		}
	}
	side_controllers_[side_num - 1] = "network";
	sides_taken_[side_num - 1] = true;
	sides_[side_num - 1] = newplayer->first;

	send_change_controller(side_num, newplayer, false);
	if (host_leave) transfer_ai_sides();

	// If we gave the new side to an observer add him to players_.
	const user_vector::iterator itor = std::find(observers_.begin(),
			observers_.end(), newplayer->first);
	if (itor != observers_.end()) {
		players_.push_back(*itor);
		observers_.erase(itor);
		// Send everyone but the new player the observer_quit message.
		config observer_quit;
		observer_quit.add_child("observer_quit")["name"] = newplayer_name;
		send_data(observer_quit, newplayer->first);
	}
}

//! Send [change_controller] message to tell all clients the new controller's name.
void game::send_change_controller(const size_t side_num,
		const player_map::const_iterator newplayer, const bool player_left)
{
	if (newplayer == player_info_->end()) return;
	const std::string& side = lexical_cast<std::string, size_t>(side_num);
	if (started_) {
		send_and_record_server_message(newplayer->second.name()
				+ " takes control of side " + side + ".");
	}
	config response;
	config& change = response.add_child("change_controller");

	change["side"] = side;
	change["player"] = newplayer->second.name();
	// Tell everyone but the new player that this side is network controlled now.
	change["controller"] = "network";
	send_data(response, newplayer->first);
	// Tell the new player that he controls this side now.
	// Just don't send it when the player left the game. (The host gets the
	// side_drop already.)
	if (!player_left) {
		change["controller"] = "human";
		network::send_data(response, newplayer->first, true);
	}

	// Update the level so observers who join get the new name.
	config::child_itors it = level_.child_range("side");
	it.first += side_num - 1;
	assert(it.first != it.second);
	(**it.first)["current_player"] = newplayer->second.name();
}

void game::transfer_ai_sides() {
	bool ai_transfer = false;
	// Check for ai sides first and drop them, too, if the host left.
	for (size_t side = 0; side < side_controllers_.size(); ++side){
		//send the host a notification of removal of this side
		if (side_controllers_[side] != "ai") continue;

		ai_transfer = true;
		config drop;
		drop["side_drop"] = lexical_cast<std::string, size_t>(side + 1);
		drop["controller"] = "ai";
		network::send_data(drop, owner_, true);
		sides_[side] = owner_;
	}
	if (ai_transfer) {
		send_and_record_server_message("AI sides transferred to new host.");
	}
}

void game::notify_new_host(){
	const std::string owner_name =
			(player_info_->find(owner_) != player_info_->end()
			? player_info_->find(owner_)->second.name() : "");
	config cfg;
	config& cfg_host_transfer = cfg.add_child("host_transfer");
	// Why do we send the new host his own name?
	cfg_host_transfer["name"] = owner_name;
	cfg_host_transfer["value"] = "1";
	network::send_data(cfg, owner_, true);
	send_and_record_server_message(owner_name
			+ " has been chosen as the new host.");
}

bool game::describe_slots() {
	if(started_ || description_ == NULL)
		return false;

	int available_slots = 0;
	int num_sides = level_.get_children("side").size();
	int i = 0;
	for(config::child_list::const_iterator it = level_.get_children("side").begin(); it != level_.get_children("side").end(); ++it, ++i) {
		if ((**it)["allow_player"] == "no" || (**it)["controller"] == "null") {
			num_sides--;
		} else {
			if (!sides_taken_[i])
				available_slots++;
		}
	}
	char buf[50];
	snprintf(buf,sizeof(buf), "%d/%d", available_slots, num_sides);

	if (buf != (*description_)["slots"]) {
		(*description_)["slots"] = buf;
		return true;
	} else {
		return false;
	}
}

//! Checks whether the connection's ip address is banned.
bool game::player_is_banned(const network::connection sock) const {
	std::vector<std::string>::const_iterator ban =
		std::find(bans_.begin(), bans_.end(), network::ip_address(sock));
	return ban != bans_.end();
}

void game::mute_all_observers() {
	all_observers_muted_ = !all_observers_muted_;
	if (all_observers_muted_) {
		send_and_record_server_message("All observers have been muted.");
	} else {
		send_and_record_server_message("Muting of all observers has been removed.");
	}
}

//! Mute an observer or give a message of all currently muted observers if no
//! name is given.
void game::mute_observer(const config& mute, const player_map::const_iterator muter) {
	if (muter->first != owner_) {
		network::send_data(construct_server_message(
				"You cannot mute: not the game host."), muter->first, true);
		return;
	}
	const std::string name = mute["username"];
	if (name.empty()) {
		if (all_observers_muted_) {
			network::send_data(construct_server_message(
					"All observers are muted."), muter->first, true);
			return;
		}
		std::string muted_nicks = "";
		for (user_vector::const_iterator muted_obs = muted_observers_.begin();
			 muted_obs != muted_observers_.end(); ++muted_obs)
		{
			if (muted_nicks != "") {
				muted_nicks += ", ";
			}
			muted_nicks += player_info_->find(*muted_obs)->second.name();
		}
		network::send_data(construct_server_message("Muted observers: "
				+ muted_nicks), muter->first, true);
		return;
	}
	const player_map::const_iterator user = find_user(name);
	//! @todo FIXME: Maybe rather save muted nicks as a vector of strings
	//! and also allow muting of usernames not in the game.
	if (user == player_info_->end() || !is_observer(user->first)) {
		network::send_data(construct_server_message(
				"Observer not found."), muter->first, true);
		return;
	}
	//! Prevent muting ourselves.
	if (user->first == muter->first) {
		network::send_data(construct_server_message(
				"Don't mute yourself, silly."), muter->first, true);
		return;
	}
	if (is_muted_observer(user->first)) {
		network::send_data(construct_server_message(user->second.name()
				+ " is already muted."), muter->first, true);
		return;
	}
	muted_observers_.push_back(user->first);
	LOG_GAME << network::ip_address(muter->first) << "\t"
		<< muter->second.name() << "muted: " << user->second.name()
		<< "\tfrom game:\t\"" << name_ << "\" (" << id_ << ")\n";
	send_and_record_server_message(user->second.name() + " has been muted.");
}

//! Kick a member by name.
//! @return the network handle of the removed member if successful, '0' otherwise.
network::connection game::kick_member(const config& kick, 
		const player_map::const_iterator kicker)
{
	if (kicker->first != owner_) {
		network::send_data(construct_server_message(
				"You cannot kick: not the game host."), kicker->first, true);
		return 0;
	}
	const std::string name = kick["username"];
	const player_map::const_iterator user = find_user(name);
	if (user == player_info_->end() || !is_member(user->first)) {
		network::send_data(construct_server_message(
				"Not a member of this game."), kicker->first, true);
		return 0;
	}
	if (user->first == kicker->first) {
		network::send_data(construct_server_message(
				"Don't kick yourself, silly."), kicker->first, true);
		return 0;
	}
	LOG_GAME << network::ip_address(kicker->first) << "\t"
		<< kicker->second.name() << "\tkicked: " << user->second.name()
		<< "\tfrom game:\t\"" << name_ << "\" (" << id_ << ")\n";
	send_and_record_server_message(name + " has been kicked.");
	// Tell the user to leave the game.
	network::send_data(config("leave_game"), user->first, true);
	remove_player(user->first);
	return user->first;
}

//! Ban a user by name.
//! The user does not need to be in this game but logged in.
//! @return the network handle of the banned player if he was in this game, '0'
//! otherwise.
network::connection game::ban_user(const config& ban,
		const player_map::const_iterator banner)
{
	if (banner->first != owner_) {
		network::send_data(construct_server_message(
				"You cannot ban: not the game host."), banner->first, true);
		return 0;
	}
	const std::string name = ban["username"];
	const player_map::const_iterator user = find_user(name);
	if (user == player_info_->end()) {
		network::send_data(construct_server_message(
				"User not found."), banner->first, true);
		return 0;
	}
	if (user->first == banner->first) {
		network::send_data(construct_server_message(
				"Don't ban yourself, silly."), banner->first, true);
		return 0;
	}
	if (player_is_banned(user->first)) {
		network::send_data(construct_server_message(name
				+ " is already banned."), banner->first, true);
		return 0;
	}
	LOG_GAME << network::ip_address(banner->first) << "\t"
		<< banner->second.name() << "\tbanned: " << name << "\tfrom game:\t"
		<< name_ << "\" (" << id_ << ")\n";
	bans_.push_back(network::ip_address(user->first));
	send_and_record_server_message(name + " has been banned.");
	if (is_member(user->first)) {
		// Tell the user to leave the game.
		network::send_data(config("leave_game"), user->first, true);
		remove_player(user->first);
		return user->first;
	}
	// Don't return the user if he wasn't in this game.
	return 0;
}

void game::process_message(config data, const player_map::iterator user) {
	// Hack to handle the pseudo game lobby_.
	if (owner_ != 0) {
	} else if (user->second.silenced()) {
		return;
	} else if (user->second.is_message_flooding()) {
		network::send_data(construct_server_message(
				"Warning: you are sending too many messages too fast. "
				"Your message has not been relayed."), user->first, true);
		return;
	}

	config* const message = data.child("message");
	(*message)["sender"] = user->second.name();
	chat_message::truncate_message((*message)["message"]);

	std::string msg = (*message)["message"].base_str();
	// Only log in the lobby_.
	if (owner_ != 0) {
	} else if (msg.substr(0,3) == "/me") {
		LOG_GAME << network::ip_address(user->first) << "\t<"
			<< user->second.name() << msg.substr(3) << ">\n";
	} else {
		LOG_GAME << network::ip_address(user->first) << "\t<"
			<< user->second.name() << "> " << msg << "\n";
	}
	send_data(data, user->first);
}

//! Process [turn].
bool game::process_turn(config data, const player_map::const_iterator user) {
	if (!started_) return false;
	config* const turn = data.child("turn");
	filter_commands(*turn, user);
	if (turn->all_children().size() == 0) return false;
	//! Return value that tells whether the description changed.
	const bool res = process_commands(data, user);

	return res;
}

//! Filter commands from all but the current player.
//! Currently removes all commands but [speak] for observers and all but
//! [speak], [label] and [rename] for players.
void game::filter_commands(config& turn, const player_map::const_iterator user) {
	if (is_current_player(user->first)) return;
	std::vector<int> marked;
	int index = 0;
	const config::child_list& children = turn.get_children("command");
	for(config::child_list::const_iterator i = children.begin();
		i != children.end(); ++i)
	{
		// Only single commands allowed.
		if ((*i)->all_children().size() != 1
		// Chatting is never an illegal command.
		|| !((*i)->child("speak") || (is_player(user->first)
			&& ((*i)->child("label") || (*i)->child("clear_labels")
				|| (*i)->child("rename") || (*i)->child("countdown_update")
				/* || (*i)->child("choose")*/))))
		{
			std::stringstream msg;
			msg << "Removing illegal command from: " << user->second.name()
				<< ". Current player is: "
				<< (player_info_->find(current_player()) != player_info_->end()
					? player_info_->find(current_player())->second.name()
					: "")
				<< ".\n";
			LOG_GAME << msg.str();
			send_and_record_server_message(msg.str());
			LOG_GAME << (**i);
			marked.push_back(index - marked.size());
		}
		++index;
	}

	for(std::vector<int>::const_iterator j = marked.begin(); j != marked.end(); ++j) {
		turn.remove_child("command",*j);
	}
}

//! Handles [end_turn], repackages [commands] with private [speak]s in them
//! and sends the data.
bool game::process_commands(const config& data, const player_map::const_iterator user) {
	//DBG_GAME << "processing commands: '" << cfg << "'\n";
	const config* const turn = data.child("turn");
	bool turn_ended = false;
	// Any private 'speak' commands must be repackaged separate
	// to other commands, and re-sent, since they should only go
	// to some clients.
	bool repackage = false;
	const config::child_list& commands = turn->get_children("command");
	config::child_list::const_iterator command;
	for (command = commands.begin(); command != commands.end(); ++command) {
		if ((**command).child("speak")) {
			config& speak = *(**command).child("speak");
			if (!(speak.get_attribute("team_name") == "")
			|| (is_muted_observer(user->first))) {
				repackage = true;
			}
			chat_message::truncate_message(speak["message"]);

			// Force the description to be correct,
			// to prevent spoofing of messages.
			speak["description"] = user->second.name();
			// Also check the side for players.
			if (is_player(user->first)) {
				const size_t side_num = lexical_cast_default<size_t>(
						speak.get_attribute("side"), 0);
				if (side_num < 1 || side_num > gamemap::MAX_PLAYERS
				|| sides_[side_num - 1] != user->first) {
					if (user->first == current_player()) {
						speak["side"] = lexical_cast<std::string>(current_side() + 1);
					} else {
						const side_vector::const_iterator s =
								std::find(sides_.begin(), sides_.end(), user->first);
						speak["side"] = lexical_cast<std::string>(s - sides_.begin() + 1);
					}
				}
			}
		} else if ((**command).child("end_turn")) {
			turn_ended = end_turn();
		}
	}
	if (!repackage) {
		record_data(data);
		send_data(data, user->first);
		return turn_ended;
	}
	for (command = commands.begin(); command != commands.end(); ++command) {
		config* const speak = (**command).child("speak");
		if (speak == NULL) {
			config mdata;
			config& turn = mdata.add_child("turn");
			turn.add_child("command", **command);
			record_data(mdata);
			send_data(mdata, user->first);
			continue;
		}
		const std::string team_name = speak->get_attribute("team_name");
		// Anyone can send to the observer team.
		if (team_name == game_config::observer_team_name) {
		// Don't send if the member is muted.
		} else if (is_muted_observer(user->first)) {
			network::send_data(construct_server_message(
					"You have been muted, others can't see your message!"),
					user->first, true);
			continue;
		// Don't send if the player addresses a different team.
		} else if (!is_on_team(team_name, user->first)) {
			const std::string msg = "Removing illegal message from "
					+ user->second.name() + " to team '" + team_name + "'.";
			LOG_GAME << msg << std::endl;
			send_and_record_server_message(msg);
			continue;
		}
		config message;
		config& turn = message.add_child("turn");
		config& command = turn.add_child("command");
		command.add_child("speak", *speak);
		if (team_name == "") {
			record_data(message);
			send_data(message, user->first);
		} else if (team_name == game_config::observer_team_name) {
			record_data(message);
			send_data_observers(message, user->first);
		} else {
			send_data_team(message, team_name, user->first);
		}
	}
	return turn_ended;
}

bool game::end_turn() {
	// It's a new turn every time each side in the game ends their turn.
	++end_turn_;
	bool turn_ended = false;
	if ((current_side()) == 0) {
		turn_ended = true;
	}
	// Skip over empty sides.
	for (int i = 0; i < nsides_ && side_controllers_[current_side()] == "null"; ++i) {
		++end_turn_;
		if (current_side() == 0) {
			turn_ended = true;
		}
	}
	if (!turn_ended) return false;

	if (description_ == NULL) {
		return false;
	}
	(*description_)["turn"] = describe_turns(current_turn(), level_.get_attribute("turns"));

	return true;
}

void game::add_player(const network::connection player, const bool observer) {
	if(is_member(player)) {
		ERR_GAME << "ERROR: Player is already in this game. (socket: "
			<< player << ")\n";
		return;
	}
	// Hack to handle the pseudo games lobby_ and not_logged_in_.
	if (owner_ == 0) {
		observers_.push_back(player);
		return;
	}
	const player_map::iterator user = player_info_->find(player);
	if (user == player_info_->end()) {
		ERR_GAME << "ERROR: Could not find user in player_info_. (socket: "
			<< owner_ << ")\n";
		return;
	}
	user->second.mark_available(id_, name_);
	DBG_GAME << debug_player_info();
	if (!started_ && !observer && take_side(user)) {
		DBG_GAME << "adding player...\n";
		players_.push_back(player);
		send_and_record_server_message(user->second.name()
				+ " has joined the game.", player);
	} else if (!allow_observers()) {
		return; //false;
	} else {
		DBG_GAME << "adding observer...\n";
		observers_.push_back(player);
		config observer_join;
		observer_join.add_child("observer")["name"] = user->second.name();
		// Send observer join to everyone except the new observer.
		send_data(observer_join, player);
	}
	DBG_GAME << debug_player_info();
	// Send the user the game data.
	network::send_data(level_, player, true);
	if(started_) {
		//tell this player that the game has started
		network::send_data(config("start_game"), player, true);
		// Send observer join of all the observers in the game to the new player
		// only once the game started. The client forgets about it anyway
		// otherwise.
		send_observerjoins(player);
		// Send the player the history of the game to-date.
		network::send_data(history_, player, true);
	} else {
		send_user_list();
	}
}

//! Removes a user from the game.
//! @return true iff the game ends that is if there are no more players
//! or the host left on a not yet started game.
bool game::remove_player(const network::connection player, const bool disconnect) {
	if (!is_member(player)) {
		ERR_GAME << "ERROR: User is not in this game. (socket: "
			<< player << ")\n";
		return false;
	}
	// Hack to handle the pseudo games lobby_ and not_logged_in_.
	if (owner_ == 0) {
		const user_vector::iterator itor =
			std::find(observers_.begin(), observers_.end(), player);
		if (itor != observers_.end()) {
			observers_.erase(itor);
		} else {
			ERR_GAME << "ERROR: Observer is not in this game. (socket: "
				<< player << ")\n";
		}
		return false;
	}
	DBG_GAME << debug_player_info();
	DBG_GAME << "removing player...\n";

	const bool host = (player == owner_);
	const bool observer = is_observer(player);
	players_.erase(std::remove(players_.begin(), players_.end(), player), players_.end());
	observers_.erase(std::remove(observers_.begin(), observers_.end(), player), observers_.end());
	const bool game_ended = (players_.empty() || (host && !started_));
	const player_map::iterator user = player_info_->find(player);
	if (user == player_info_->end()) {
		ERR_GAME << "ERROR: Could not find user in player_info_. (socket: "
			<< player << ")\n";
		return false;
	}
	LOG_GAME << network::ip_address(user->first) << "\t" << user->second.name()
		<< (game_ended ? (started_ ? "\tended" : "\taborted") : "\thas left")
		<< " game:\t\"" << name_ << "\" (" << id_ << ")"
		<< (game_ended && started_ ? " at turn: "
			+ lexical_cast_default<std::string,size_t>(current_turn())
			+ " with reason: '" + termination_reason() + "'" : "")
		<< (observer ? " as an observer" : "")
		<< (disconnect ? " and disconnected" : "")
		<< ". (socket: " << user->first << ")\n";
	if (game_ended) {
		send_data(construct_server_message(user->second.name()
				+ " ended the game."), player);
		return true;
	}
	// Don't mark_available() since the player got already removed from the
	// games_and_users_list_.
	if (!disconnect) user->second.mark_available();
	if (observer) {
		//they're just an observer, so send them having quit to clients
		config observer_quit;
		observer_quit.add_child("observer_quit")["name"] = user->second.name();
		send_data(observer_quit);
	} else {
		send_and_record_server_message(user->second.name()
				+ (disconnect ? " has disconnected." : " has left the game."), player);
	}
	// If the player was host choose a new one.
	if (host) {
		owner_ = players_.front();
		notify_new_host();
	}

	// Look for all sides the player controlled and drop them.
	// (Give them to the host.)
	for (side_vector::iterator side = sides_.begin(); side != sides_.end(); ++side)	{
		if (*side != player) continue;
		size_t side_num = side - sides_.begin();
		side_controllers_[side_num] = "human";
		sides_taken_[side_num] = true;
		sides_[side_num] = owner_;
		send_change_controller(side_num + 1, player_info_->find(owner_));

		//send the host a notification of removal of this side
		config drop;
		drop["side_drop"] = lexical_cast<std::string, size_t>(side_num + 1);
		drop["controller"] = side_controllers_[side_num];
		network::send_data(drop, owner_, true);
	}
	if (host) transfer_ai_sides();
	DBG_GAME << debug_player_info();

	send_user_list(player);
	return false;
}

void game::send_user_list(const network::connection exclude) const {
	//if the game hasn't started yet, then send all players a list
	//of the users in the game
	if (started_ || description_ == NULL) return;
	//! @todo Should be renamed to userlist.
	config cfg("gamelist");
	user_vector users = all_game_users();
	for(user_vector::const_iterator p = users.begin(); p != users.end(); ++p) {
		const player_map::const_iterator pl = player_info_->find(*p);
		if (pl != player_info_->end()) {
			config& user = cfg.add_child("user");
			user["name"] = pl->second.name();
		}
	}
	send_data(cfg, exclude);
}

//! A member asks for the next scenario to advance to.
void game::load_next_scenario(const player_map::const_iterator user) const {
	send_data(construct_server_message(user->second.name()
			+ " advances to the next scenario."), user->first);
	config cfg_scenario;
	cfg_scenario.add_child("next_scenario", level_);
	network::send_data(cfg_scenario, user->first, true);
	// Send the player the history of the game to-date.
	network::send_data(history_, user->first, true);
	// Send observer join of all the observers in the game to the user.
	send_observerjoins(user->first);
}

void game::send_data(const config& data, const network::connection exclude) const
{
	const user_vector users = all_game_users();
	for(user_vector::const_iterator i = users.begin(); i != users.end(); ++i) {
		if (*i != exclude) {
			network::send_data(data, *i, true);
		}
	}
}

bool game::is_on_team(const std::string& team, const network::connection player) const {
	for (side_vector::const_iterator side = sides_.begin(); side != sides_.end();
		 side++)
	{
		const config* const side_cfg = level_.find_child("side", "side",
			lexical_cast<std::string, size_t>(side - sides_.begin() + 1));
		if (*side == player && side_cfg != NULL && (*side_cfg)["team_name"] == team) {
			return true;
		}
	}
	return false;
}

void game::send_data_team(const config& data, const std::string& team,
	const network::connection exclude) const
{
	for(user_vector::const_iterator i = players_.begin(); i != players_.end(); ++i) {
		if(*i != exclude && is_on_team(team,*i)) {
			network::send_data(data, *i, true);
		}
	}
}

void game::send_data_observers(const config& data, const network::connection exclude) const {
	for(user_vector::const_iterator i = observers_.begin(); i != observers_.end(); ++i) {
		if (*i != exclude) {
			network::send_data(data, *i, true);
		}
	}
}

//! Send [observer] tags of all the observers in the game to the user or
//! everyone if none given.
void game::send_observerjoins(const network::connection sock) const {
	for (user_vector::const_iterator ob = observers_.begin(); ob != observers_.end(); ++ob) {
		if (*ob == sock) continue;
		const player_map::const_iterator obs = player_info_->find(*ob);
		if (obs == player_info_->end()) {
			ERR_GAME << "Game: " << id_
				<< " ERROR: Can not find observer in player_info_. (socket: "
				<< *ob << ")\n";
			continue;
		}
		config cfg;
		cfg.add_child("observer")["name"] = obs->second.name();
		if (sock == 0) {
			// Send to everyone except the observer in question.
			send_data(cfg, *ob);
		} else {
			// Send to the (new) user.
			network::send_data(cfg, sock, true);
		}
	}
}

void game::record_data(const config& data) {
	history_.append(data);
}

void game::set_description(config* desc) {
	description_ = desc;
	if(!password_.empty()) {
		(*description_)["password"] = "yes";
	}
}

void game::add_players(const game& other_game, const bool observer) {
	user_vector users = other_game.all_game_users();
	if (observer){
		observers_.insert(observers_.end(), users.begin(), users.end());
	}
	else{
		players_.insert(players_.end(), users.begin(), users.end());
	}
}

const user_vector game::all_game_users() const {
	user_vector res;

	res.insert(res.end(), players_.begin(), players_.end());
	res.insert(res.end(), observers_.begin(), observers_.end());

	return res;
}

std::string game::debug_player_info() const {
	std::stringstream result;
	result << "game id: " << id_ << "\n";
//	result << "players_.size: " << players_.size() << "\n";
	for (user_vector::const_iterator p = players_.begin(); p != players_.end(); p++){
		const player_map::const_iterator user = player_info_->find(*p);
		if (user != player_info_->end()){
			result << "player: " << user->second.name().c_str() << "\n";
		}
		else{
			result << "player: '" << *p << "' not found\n";
		}
	}
//	result << "observers_.size: " << observers_.size() << "\n";
	for (user_vector::const_iterator o = observers_.begin(); o != observers_.end(); o++){
		const player_map::const_iterator user = player_info_->find(*o);
		if (user != player_info_->end()){
			result << "observer: " << user->second.name().c_str() << "\n";
		}
		else{
			result << "observer: '" << *o << "' not found\n";
		}
	}
/*	result << "player_info_: begin\n";
	for (player_map::const_iterator info = player_info_->begin(); info != player_info_->end(); info++){
		result << info->second.name().c_str() << "\n";
	}
	result << "player_info_: end\n";*/
	return result.str();
}

//! Find a user by name.
player_map::const_iterator game::find_user(const std::string& name) const {
	player_map::const_iterator pl;
	for (pl = player_info_->begin(); pl != player_info_->end(); pl++) {
		if (pl->second.name() == name)
			return pl;
	}
	return player_info_->end();
}

void game::send_and_record_server_message(const std::string& message,
		const network::connection exclude)
{
	const config& server_message = construct_server_message(message);
	record_data(server_message);
	send_data(server_message, exclude);

}

config game::construct_server_message(const std::string& message) const
{
	config turn;
	if(started_) {
		config& cmd = turn.add_child("turn");
		config& cfg = cmd.add_child("command");
		config& msg = cfg.add_child("speak");
		msg["description"] = "server";
		msg["message"] = message;
	} else {
		config& msg = turn.add_child("message");
		msg["sender"] = "server";
		msg["message"] = message;
	}
	return turn;
}
