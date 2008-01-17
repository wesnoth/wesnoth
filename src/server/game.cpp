/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
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

#include <iostream>
#include <cassert>
#include <sstream>

#define ERR_GAME LOG_STREAM(err, mp_server)
#define LOG_GAME LOG_STREAM(info, mp_server)
#define DBG_GAME LOG_STREAM(debug, mp_server)

int game::id_num = 1;

game::game(player_map& players, const network::connection host, const std::string name)
	: player_info_(&players), id_(id_num++), name_(name), owner_(host),
	sides_(gamemap::MAX_PLAYERS), sides_taken_(gamemap::MAX_PLAYERS),
	side_controllers_(gamemap::MAX_PLAYERS), started_(false),
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
	return utils::string_bool(level_["observer"], true);
}

bool game::is_observer(const network::connection player) const {
	return std::find(observers_.begin(),observers_.end(),player) != observers_.end();
}

bool game::is_muted_observer(const network::connection player) const {
	return std::find(muted_observers_.begin(), muted_observers_.end(), player)
		!= muted_observers_.end();
}

bool game::is_player(const network::connection player) const {
	return std::find(players_.begin(),players_.end(),player) != players_.end();
}

bool game::is_current_player(const network::connection player) const {
	size_t current_side = end_turn_ % nsides_;
	return (sides_[current_side] == player);
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
	LOG_GAME << network::ip_address(starter->first) << "\t" << starter->second.name()
		<< "\tstarted game:\t\"" << name_ << "\" (" << id_
		<< "). Settings: map: " << level_["id"]
		<< "\tera: " << (level_.child("era") != NULL
			? level_.child("era")->get_attribute("id") : "")
		<< "\tXP: " << level_["experience_modifier"]
		<< "\tGPV: " << level_["mp_village_gold"]
		<< "\tfog: " << level_["mp_fog"]
		<< "\tshroud: " << level_["mp_shroud"]
		<< "\tobservers: " << level_["observer"]
		<< "\ttimer: " << level_["mp_countdown"]
		<< (utils::string_bool(level_["mp_countdown"], false) ?
			"\treservoir time: " + level_["mp_countdown_reservoir_time"] +
			"\tinit time: " + level_["mp_countdown_init_time"] +
			"\taction bonus: " + level_["mp_countdown_action_bonus"] +
			"\tturn bonus: " + level_["mp_countdown_turn_bonus"] : "")
		<< "\n";

	started_ = true;
	nsides_ = 0;
	// Set all side controllers to 'human' so that observers will understand
	// that they can't take control of any sides if they happen to have the
	// same name as one of the descriptions.
	for(config::child_itors sides = level_.child_range("side");
		sides.first != sides.second; ++sides.first)
	{
		if ((**sides.first)["controller"] != "null") {
			nsides_++;
			(**sides.first)["controller"] = "human";
		}
	}
	int turn =  lexical_cast_default<int>(level_["turn_at"], 1);
	end_turn_ = (turn - 1) * nsides_;
	if (description_) {
		(*description_)["turn"] = describe_turns(turn, level_["turns"]);
	} else {
		ERR_GAME << "ERROR: Game without description_ started. (" << id_ << ")\n";
	}
	// Send [observer] tags for all observers that are already in the game.
	send_observerjoins();
}

//! Resets a couple of things when advancing to the next scenario.
void game::start_next_scenario() {
	nsides_ = 0;
	// Set all side controllers to 'human' so that observers will understand
	// that they can't take control of any sides if they happen to have the
	// same name as one of the descriptions.
	for(config::child_itors sides = level_.child_range("side");
		sides.first != sides.second; ++sides.first)
	{
		if ((**sides.first)["controller"] != "null") {
			nsides_++;
			(**sides.first)["controller"] = "human";
		}
	}
	end_turn_ = 0;
	if (description_) {
		(*description_)["turn"] = describe_turns(1, level_["turns"]);
	} else {
		ERR_GAME << "ERROR: Game without description_ started. (" << id_ << ")\n";
	}
	history_.clear();
	// Re-assign sides.
	update_side_data();
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
		DBG_GAME << (*side)->debug();*/
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

	if(side_controllers_[side_num - 1] == "network" && sides_taken_[side_num - 1]
	   && cfg["own_side"] != "yes")
	{
		network::send_data(construct_server_message(
			"This side is already controlled by a player."), sock, true);
		return;
	}
	// Check if the sender actually owns the side he gives away or is the host.
	if (!(sides_[side_num - 1] == sock || (sock == owner_))) {
		DBG_GAME << "Side belongs to: " << sides_[side_num - 1] << "\n";
		network::send_data(construct_server_message("Not your side."), sock, true);
		return;
	}
	if (newplayer->first == sock) {
		network::send_data(construct_server_message(
			"That's already your side, silly."), sock, true);
		return;
	}
	sides_[side_num - 1] = 0;
	bool host_leave = false;
	// If the player gave up their last side, make them an observer.
	if (std::find(sides_.begin(), sides_.end(), sock) == sides_.end()) {
		observers_.push_back(*pl);
		players_.erase(pl);
		const std::string& name = player_info_->find(sock)->second.name();
		// Tell others that the player becomes an observer.
		send_data(construct_server_message(name	+ " becomes an observer."));
		// Update the client side observer list for everyone except player.
		config observer_join;
		observer_join.add_child("observer")["name"] = name;
		send_data(observer_join, sock);
		// If this player was the host of the game, choose another player.
		if (sock == owner_) {
			host_leave = true;
			if (!players_.empty()) {
				owner_ = players_.front();
				send_data(construct_server_message(name
					+ " has been chosen as the new host."));
			} else {
				owner_ = newplayer->first;
				send_data(construct_server_message(newplayer_name
					+ " has been chosen as the new host."));
			}
			notify_new_host();
		}
	}
	side_controllers_[side_num - 1] = "network";
	sides_taken_[side_num - 1] = true;
	sides_[side_num - 1] = newplayer->first;

	const config& msg = construct_server_message(newplayer_name
			+ " takes control of side " + side + ".");
	record_data(msg);
	send_data(msg);

	//send "change_controller" msg that make all client update
	//the current player name
	config response;
	config& change = response.add_child("change_controller");

	change["side"] = side;
	change["player"] = newplayer_name;
	// Tell everyone but the new player that this side is network controlled now.
	change["controller"] = "network";
	send_data(response, newplayer->first);
	// Tell the new player that he controls this side now.
	change["controller"] = "human";
	network::send_data(response, newplayer->first, true);

	// Update the level so observers who join get the new name.
	config::child_itors it = level_.child_range("side");
	it.first += side_num - 1;
	assert(it.first != it.second);
	(**it.first)["current_player"] = newplayer_name;

	//if the host left and there are ai sides, transfer them to the new host
	//and notify the new host's client about its new status
	if (host_leave) {
		for (unsigned int i = 0; i < side_controllers_.size(); i++) {
			if (side_controllers_[i] == "ai") {
				change["side"] = lexical_cast<std::string, unsigned int>(i + 1);
				change["controller"] = "ai";
				network::send_data(response, owner_, true);
				sides_[side_num - 1] = owner_;
			}
		}
	}

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

void game::notify_new_host(){
	const player_map::const_iterator it_host = player_info_->find(owner_);
	if (it_host != player_info_->end()){
		config cfg;
		config& cfg_host_transfer = cfg.add_child("host_transfer");
		cfg_host_transfer["name"] = it_host->second.name();
		cfg_host_transfer["value"] = "1";
		network::send_data(cfg, owner_, true);
	}
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
	if (user->first != muter->first) {
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
	network::send_data(construct_server_message(
			"You have been muted."), user->first, true);
	send_data(construct_server_message(user->second.name()
			+ " has been muted."), user->first);
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
	network::send_data(construct_server_message("You have been kicked."),
			user->first, true);
	const config& msg = construct_server_message(name + " has been kicked.");
	send_data(msg, user->first);
	record_data(msg);
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
	const config& msg = construct_server_message(name + " has been banned.");
	send_data(msg, user->first);
	record_data(msg);
	if (is_member(user->first)) {
		network::send_data(construct_server_message("You have been banned."),
				user->first, true);
		// Tell the user to leave the game.
		network::send_data(config("leave_game"), user->first, true);
		remove_player(user->first);
		return user->first;
	}
	// Don't return the user if he wasn't in this game.
	return 0;
}

//! Process [turn].
bool game::process_turn(config data, const player_map::const_iterator user) {
	if (!started_) return false;
	config* const turn = data.child("turn");
	filter_commands(user->first, *turn);

	//! Return value that tells whether the description changed.
	const bool res = process_commands(*turn);

	// Any private 'speak' commands must be repackaged separate
	// to other commands, and re-sent, since they should only go
	// to some clients.
	const config::child_itors speaks = turn->child_range("command");
	int npublic = 0, nprivate = 0, nother = 0;
	std::string team_name;
	for (config::child_iterator i = speaks.first; i != speaks.second; ++i) {
		config* const speak = (*i)->child("speak");
		if (speak == NULL) {
			++nother;
			continue;
		}
		if ((all_observers_muted() && is_observer(user->first))
			|| is_muted_observer(user->first))
		{
			network::send_data(construct_server_message(
					"You have been muted, others can't see your message!"),
					user->first, true);
			return res;
		}
		chat_message::truncate_message((*speak)["message"]);

		// Force the description to be correct,
		// to prevent spoofing of messages
		(*speak)["description"] = user->second.name();

		if ((*speak)["team_name"] == "") {
			++npublic;
		} else {
			++nprivate;
			team_name = (*speak)["team_name"];
		}
	}

	// If all there are are messages and they're all private, then
	// just forward them on to the client that should receive them.
	if (nprivate > 0 && npublic == 0 && nother == 0) {
		if (team_name == game_config::observer_team_name) {
			send_data_observers(data, user->first);
		} else {
			send_data_team(data, team_name, user->first);
		}
		return res;
	}

	// At the moment, if private messages are mixed in with other data,
	// then let them go through. It's exceedingly unlikely that
	// this will happen anyway, and if it does, the client should
	// respect not displaying the message.
	// The client displays messages from the currently *viewed* team,
	// so it does not always respect it.
	send_data(data, user->first);
	record_data(data);
	return res;
}

//! Filter commands from all but the current player.
//! Currently removes all commands but [speak] for observers.
bool game::filter_commands(const network::connection player, config& cfg) const {
	if (is_current_player(player)) return true;

	std::vector<int> marked;
	int index = 0;
	const config::child_list& children = cfg.get_children("command");
	for(config::child_list::const_iterator i = children.begin();
		i != children.end(); ++i)
	{
		if ((observers_can_label() && (*i)->child("label"))
			|| (observers_can_chat() && (*i)->child("speak") != NULL)
			&& (*i)->all_children().size() == 1)
		{
		} else {
			LOG_GAME << "Removing illegal command: '" << (*i)->debug() << "'.\n";
			marked.push_back(index - marked.size());
		}

		++index;
	}

	for(std::vector<int>::const_iterator j = marked.begin(); j != marked.end(); ++j) {
		cfg.remove_child("command",*j);
	}

	return cfg.all_children().empty() == false;
}

bool game::process_commands(const config& cfg) {
	//DBG_GAME << "processing commands: '" << cfg.debug() << "'\n";
	bool res = false;
	const config::child_list& cmd = cfg.get_children("command");
	for(config::child_list::const_iterator i = cmd.begin(); i != cmd.end(); ++i) {
		if((**i).child("end_turn") != NULL) {
			res = end_turn();
			//DBG_GAME << "res: " << (res ? "yes" : "no") << "\n";
		}
	}
	return res;
}

bool game::end_turn() {
	// It's a new turn every time each side in the game ends their turn.
	++end_turn_;

	if (nsides_ == 0 || (end_turn_%nsides_) != 0) return false;

	const int turn = end_turn_/nsides_ + 1;

	if (description_ == NULL) {
		return false;
	}
	(*description_)["turn"] = describe_turns(turn, level_["turns"]);

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
	send_user_list();
	// Send the user the game data.
	network::send_data(level_, player, true);
	if(started_) {
		//tell this player that the game has started
		network::send_data(config("start_game"), player, true);
		// Send the player the history of the game to-date.
		network::send_data(history_, player, true);
		// Send observer join of all the observers in the game to the new player
		// only once the game started. The client forgets about it anyway
		// otherwise.
		send_observerjoins(player);
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
			+ (description_ ? (*description_)["turn"] : "-/-")
			+ " with reason: '" + termination_reason() + "'" : "")
		<< (observer ? " as an observer" : "")
		<< (disconnect ? " and disconnected" : "")
		<< ". (socket: " << user->first << ")\n";
	if (game_ended) return true;
	// Don't mark_available() since the player got already removed from the
	// games_and_users_list_.
	if (!disconnect) user->second.mark_available();
	if (observer) {
		//they're just an observer, so send them having quit to clients
		config observer_quit;
		observer_quit.add_child("observer_quit")["name"] = user->second.name();
		send_data(observer_quit);
	} else {
		const config& msg = construct_server_message(user->second.name()
				+ (disconnect ? " has disconnected." : " has left the game."));
		send_data(msg, player);
		record_data(msg);
	}
	// If the player was host choose a new one.
	if (host) {
		owner_ = players_.front();
		std::string owner_name = "";
		const player_map::const_iterator owner = player_info_->find(owner_);
		if (owner == player_info_->end()) {
			ERR_GAME << "ERROR: Could not find new host in player_info_. (socket: "
				<< owner_ << ")\n";
		} else {
			owner_name = owner->second.name();
		}
		notify_new_host();
		send_data(construct_server_message(owner_name
			+ " has been chosen as new host."));
		//check for ai sides first and drop them, too, if the host left
		bool ai_transfer = false;
		//can't do this with an iterator, because it doesn't know the side_num - 1
		for (size_t side = 0; side < side_controllers_.size(); ++side){
			//send the host a notification of removal of this side
			if (side_controllers_[side] == "ai") {
				ai_transfer = true;
				config drop;
				drop["side_drop"] = lexical_cast<std::string, size_t>(side + 1);
				drop["controller"] = "ai";
				network::send_data(drop, owner_, true);
				sides_taken_[side] = false;
			}
		}
		if (ai_transfer) {
			send_data(construct_server_message("AI transferred to new host"));
		}
	}

	//look for all sides the player controlled and drop them
	for (side_vector::iterator side = sides_.begin(); side != sides_.end(); ++side)	{
		if (*side != player) continue;
		//send the host a notification of removal of this side
		if (players_.empty() == false) {
			config drop;
			drop["side_drop"] = lexical_cast<std::string, size_t>(side - sides_.begin() + 1);
			drop["controller"] = side_controllers_[side - sides_.begin()];
			network::send_data(drop, owner_, true);
		}
		side_controllers_[side - sides_.begin()] = "null";
		sides_taken_[side - sides_.begin()] = false;
		sides_[side - sides_.begin()] = 0;
	}
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

bool game::player_on_team(const std::string& team, const network::connection player) const {
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
		if(*i != exclude && player_on_team(team,*i)) {
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
