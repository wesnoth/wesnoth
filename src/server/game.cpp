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

#include "game.hpp"
#include "player.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <cassert>
#include <sstream>

#define ERR_GAME LOG_STREAM(err, mp_server)
#define WRN_GAME LOG_STREAM(warn, mp_server)
#define LOG_GAME LOG_STREAM(info, mp_server)
#define DBG_GAME LOG_STREAM(debug, mp_server)

namespace chat_message {

const size_t max_message_length = 256;
static void truncate_message(t_string& str) {
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

game::~game()
{
	for(std::vector<simple_wml::document*>::iterator i = history_.begin(); i != history_.end(); ++i) {
		delete *i;
	}
}

bool game::allow_observers() const {
	return level_["observer"].to_bool(true);
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
std::string describe_turns(int turn, const simple_wml::string_span& num_turns)
{
	char buf[50];
	snprintf(buf,sizeof(buf),"%d/",int(turn));

	if(num_turns == "-1") {
		return buf + std::string("-");
	} else {
		return buf + std::string(num_turns.begin(), num_turns.end());
	}
}

}//anon namespace

void game::start_game(const player_map::const_iterator starter) {
	// If the game was already started we're actually advancing.
	const bool advance = started_;
	started_ = true;
	// Prevent inserting empty keys when reading.
	const simple_wml::node& s = level_.root();
	const bool save = s["savegame"].to_bool();
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
		<< (s["mp_countdown"].to_bool() ?
			"\treservoir time: " + s["mp_countdown_reservoir_time"].to_string() +
			"\tinit time: "      + s["mp_countdown_init_time"].to_string() +
			"\taction bonus: "   + s["mp_countdown_action_bonus"].to_string() +
			"\tturn bonus: "     + s["mp_countdown_turn_bonus"].to_string() : "")
		<< "\n";

	nsides_ = 0;
	// Set all side controllers to 'human' so that observers will understand
	// that they can't take control of any sides if they happen to have the
	// same name as one of the descriptions.
	const simple_wml::node::child_list& sides = level_.root().children("side");
	for(simple_wml::node::child_list::const_iterator s = sides.begin(); s != sides.end(); ++s) {
		nsides_++;
		if((**s)["controller"] != "null" && !advance) {
			(*s)->set_attr("controller", "human");
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
		static simple_wml::document notify_next_scenario("[notify_next_scenario]\n[/notify_next_scenario]\n", simple_wml::INIT_COMPRESSED);
		send_data(notify_next_scenario, starter->first);
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

	simple_wml::document cfg;
	cfg.root().set_attr_dup("name", user->second.name().c_str());
	cfg.root().set_attr("faction", "random");
	cfg.root().set_attr("leader", "random");
	cfg.root().set_attr("gender", "random");

	size_t side_num;
	// Check if we can figure out a fitting side.
	const simple_wml::node::child_list& sides = level_.root().children("side");
	for(simple_wml::node::child_list::const_iterator side = sides.begin(); side != sides.end(); ++side) {
		if((**side)["controller"] == "network"
				&& ((**side)["save_id"] == user->second.name().c_str()
				|| (**side)["current_player"] == user->second.name().c_str()))
		{
			side_num = (**side)["side"].to_int();
			if (side_num < 1 || side_num > gamemap::MAX_PLAYERS) continue;
			side_controllers_[side_num - 1] = "network";
			sides_[side_num - 1] = user->first;
			sides_taken_[side_num - 1] = true;
			cfg.root().set_attr_dup("side", (**side)["side"]);
			// Tell the host which side the new player should take.
			
			simple_wml::string_span data = cfg.output_compressed();
			network::send_raw_data(data.begin(), data.size(), owner_);
			DBG_GAME << debug_player_info();
			return true;
		}
	}
	// If there was no fitting side just take the first available.
	for(simple_wml::node::child_list::const_iterator side = sides.begin(); side != sides.end(); ++side) {
		if((**side)["controller"] == "network") {
			//don't allow players to take sides in games with invalid side numbers
			try {
				side_num = (**side)["side"].to_int();
			} catch (bad_lexical_cast&) { continue; }
			if (side_num < 1 || side_num > gamemap::MAX_PLAYERS) continue;
			side_controllers_[side_num - 1] = "network";
			sides_[side_num - 1] = user->first;
			sides_taken_[side_num - 1] = true;
			cfg.root().set_attr_dup("side", (**side)["side"]);
			// Tell the host which side the new player should take.
			simple_wml::string_span data = cfg.output_compressed();
			network::send_raw_data(data.begin(), data.size(), owner_);
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

	const simple_wml::node::child_list& level_sides = level_.root().children("side");
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
		for (simple_wml::node::child_list::const_iterator side = level_sides.begin();
				side != level_sides.end(); ++side)
		{
			size_t side_num = (**side)["side"].to_int();
			if (side_num < 1 || side_num > gamemap::MAX_PLAYERS
					|| sides_taken_[side_num - 1]) continue;

			if ((**side)["controller"] == "network") {
				side_controllers_[side_num - 1] = "network";
				if ((**side)["current_player"] == info->second.name().c_str()) {
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

void game::transfer_side_control(const network::connection sock, const simple_wml::node& cfg) {
	DBG_GAME << "transfer_side_control...\n";
	if (!is_player(sock)) {
		send_server_message("You cannot change controllers: not a player.", sock);
		return;
	}

	//check, if this socket belongs to a player
	const user_vector::iterator pl = std::find(players_.begin(), players_.end(), sock);
	if (pl == players_.end()) {
		ERR_GAME << "ERROR: Not a player of this game. (socket: " << sock << ")\n";
		return;
	}
	const simple_wml::string_span& newplayer_name = cfg["player"];
	//find the player that is passed control
	player_map::const_iterator newplayer;
	for (newplayer = player_info_->begin(); newplayer != player_info_->end(); newplayer++) {
		if (newplayer_name == newplayer->second.name().c_str()) {
			break;
		}
	}
	// Is he in this game?
	if (newplayer == player_info_->end() || !is_member(newplayer->first)) {
		send_server_message("Player/Observer not in this game", sock);
		return;
	}
	// Check the side number.
	const int side_num = cfg["side"].to_int();
	if(side_num < 1 || side_num > gamemap::MAX_PLAYERS) {
		std::ostringstream msg;
		msg << "The side number has to be between 1 and " 
		    << gamemap::MAX_PLAYERS << ".";
		send_server_message(msg.str().c_str(), sock);
		return;
	}

	if (side_num > level_.root().children("side").size()) {
		send_server_message("Invalid side number.", sock);
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
		send_server_message(msg.str().c_str(), sock);
		return;
	}
	if (newplayer->first == old_player) {
		std::stringstream msg;
		msg << "That's already " << newplayer_name << "'s side, silly.";
		send_server_message(msg.str().c_str(), sock);
		return;
	}
	sides_[side_num - 1] = 0;
	bool host_leave = false;
	// If the old player lost his last side, make him an observer.
	if (std::find(sides_.begin(), sides_.end(), old_player) == sides_.end()) {
		observers_.push_back(old_player);
		players_.erase(std::remove(players_.begin(), players_.end(), old_player), players_.end());
		// Tell others that the player becomes an observer.
		send_and_record_server_message((old_player_name + " becomes an observer.").c_str());
		// Update the client side observer list for everyone except old player.
		simple_wml::document observer_join;
		observer_join.root().add_child("observer").set_attr_dup("name", old_player_name.c_str());
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
		simple_wml::document observer_quit;
		observer_quit.root().add_child("observer_quit").set_attr_dup("name", newplayer_name);
		send_data(observer_quit, newplayer->first);
	}
}

//! Send [change_controller] message to tell all clients the new controller's name.
void game::send_change_controller(const size_t side_num,
		const player_map::const_iterator newplayer, const bool player_left)
{
	if (newplayer == player_info_->end()) {
		return;
	}

	const std::string& side = lexical_cast<std::string, size_t>(side_num);
	if (started_) {
		send_and_record_server_message((newplayer->second.name()
				+ " takes control of side " + side + ".").c_str());
	}

	simple_wml::document response;
	simple_wml::node& change = response.root().add_child("change_controller");

	change.set_attr("side", side.c_str());
	change.set_attr("player", newplayer->second.name().c_str());

	// Tell everyone but the new player that this side is network controlled now.
	change.set_attr("controller", "network");
	send_data(response, newplayer->first);

	// Tell the new player that he controls this side now.
	// Just don't send it when the player left the game. (The host gets the
	// side_drop already.)
	if (!player_left) {
		change.set_attr("controller", "human");
		send_to_one(response, newplayer->first);
	}

	// Update the level so observers who join get the new name.
	const simple_wml::node::child_list& side_list = level_.root().children("side");
	const int index = side_num - 1;
	assert(index < side_list.size());
	side_list[index]->set_attr_dup("current_player", newplayer->second.name().c_str());
}

void game::transfer_ai_sides() {
	bool ai_transfer = false;
	// Check for ai sides first and drop them, too, if the host left.
	for (size_t side = 0; side < side_controllers_.size(); ++side){
		//send the host a notification of removal of this side
		if (side_controllers_[side] != "ai") continue;

		ai_transfer = true;
		simple_wml::document drop;
		const std::string side_drop = lexical_cast<std::string, size_t>(side + 1);
		drop.root().set_attr("side_drop", side_drop.c_str());
		drop.root().set_attr("controller", "ai");
		const simple_wml::string_span data = drop.output_compressed();
		network::send_raw_data(data.begin(), data.size(), owner_);
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
	simple_wml::document cfg;
	simple_wml::node& cfg_host_transfer = cfg.root().add_child("host_transfer");

	// Why do we send the new host his own name?
	cfg_host_transfer.set_attr("name", owner_name.c_str());
	cfg_host_transfer.set_attr("value", "1");
	const simple_wml::string_span data = cfg.output_compressed();
	network::send_raw_data(data.begin(), data.size(), owner_);
	send_and_record_server_message((owner_name
			+ " has been chosen as the new host.").c_str());
}

bool game::describe_slots() {
	if(started_ || description_ == NULL)
		return false;

	int available_slots = 0;
	int num_sides = level_.root().children("side").size();
	int i = 0;
	const simple_wml::node::child_list& side_list = level_.root().children("side");
	for(simple_wml::node::child_list::const_iterator it = side_list.begin(); it != side_list.end(); ++it, ++i) {
		if (((**it)["allow_player"].to_bool(true) == false) || (**it)["controller"] == "null") {
			num_sides--;
		} else {
			if (!sides_taken_[i])
				available_slots++;
		}
	}
	char buf[50];
	snprintf(buf,sizeof(buf), "%d/%d", available_slots, num_sides);

	if ((*description_)["slots"] != buf) {
		description_->set_attr_dup("slots", buf);
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
void game::mute_observer(const simple_wml::node& mute, const player_map::const_iterator muter) {
	if (muter->first != owner_) {
		send_server_message("You cannot mute: not the game host.", muter->first);
		return;
	}
	const simple_wml::string_span& name = mute["username"];
	if (name.empty()) {
		if (all_observers_muted_) {
			send_server_message("All observers are muted.", muter->first);
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

		send_server_message(("Muted observers: " + muted_nicks).c_str(), muter->first);
		return;
	}
	const player_map::const_iterator user = find_user(name);
	//! @todo FIXME: Maybe rather save muted nicks as a vector of strings
	//! and also allow muting of usernames not in the game.
	if (user == player_info_->end() || !is_observer(user->first)) {
		send_server_message("Observer not found.", muter->first);
		return;
	}
	//! Prevent muting ourselves.
	if (user->first == muter->first) {
		send_server_message("Don't mute yourself, silly.", muter->first);
		return;
	}
	if (is_muted_observer(user->first)) {
		send_server_message((user->second.name() + " is already muted.").c_str(), muter->first);
		return;
	}
	muted_observers_.push_back(user->first);
	LOG_GAME << network::ip_address(muter->first) << "\t"
		<< muter->second.name() << "muted: " << user->second.name()
		<< "\tfrom game:\t\"" << name_ << "\" (" << id_ << ")\n";
	send_and_record_server_message((user->second.name() + " has been muted.").c_str());
}

//! Kick a member by name.
//! @return the network handle of the removed member if successful, '0' otherwise.
network::connection game::kick_member(const simple_wml::node& kick, 
		const player_map::const_iterator kicker)
{
	if (kicker->first != owner_) {
		send_server_message("You cannot kick: not the game host", kicker->first);
		return 0;
	}
	const simple_wml::string_span& name = kick["username"];
	const player_map::const_iterator user = find_user(name);
	if (user == player_info_->end() || !is_member(user->first)) {
		send_server_message("Not a member of this game.", kicker->first);
		return 0;
	}
	if (user->first == kicker->first) {
		send_server_message("Don't kick yourself, silly.", kicker->first);
		return 0;
	}
	LOG_GAME << network::ip_address(kicker->first) << "\t"
		<< kicker->second.name() << "\tkicked: " << user->second.name()
		<< "\tfrom game:\t\"" << name_ << "\" (" << id_ << ")\n";
	send_and_record_server_message((name.to_string() + " has been kicked.").c_str());

	// Tell the user to leave the game.
	static simple_wml::document leave_game("[leave_game]\n[/leave_game]\n", simple_wml::INIT_COMPRESSED);
	static const simple_wml::string_span leave_game_data = leave_game.output_compressed();
	network::send_raw_data(leave_game_data.begin(), leave_game_data.size(), user->first);
	remove_player(user->first);
	return user->first;
}

//! Ban a user by name.
//! The user does not need to be in this game but logged in.
//! @return the network handle of the banned player if he was in this game, '0'
//! otherwise.
network::connection game::ban_user(const simple_wml::node& ban,
		const player_map::const_iterator banner)
{
	if (banner->first != owner_) {
		send_server_message("You cannot ban: not the game host", banner->first);
		return 0;
	}
	const simple_wml::string_span& name = ban["username"];
	const player_map::const_iterator user = find_user(name);
	if (user == player_info_->end()) {
		send_server_message("User not found", banner->first);
		return 0;
	}
	if (user->first == banner->first) {
		send_server_message("Don't ban yourself, silly.", banner->first);
		return 0;
	}
	if (player_is_banned(user->first)) {
		std::ostringstream stream;
		stream << name << " is already banned.";
		send_server_message(stream.str().c_str(), banner->first);
		return 0;
	}
	LOG_GAME << network::ip_address(banner->first) << "\t"
		<< banner->second.name() << "\tbanned: " << name << "\tfrom game:\t"
		<< name_ << "\" (" << id_ << ")\n";
	bans_.push_back(network::ip_address(user->first));
	send_and_record_server_message((name.to_string() + " has been banned.").c_str());
	if (is_member(user->first)) {
		//tell the user to leave the game.
		static simple_wml::document leave_game("[leave_game]\n[/leave_game]\n", simple_wml::INIT_COMPRESSED);
		static const simple_wml::string_span leave_game_data = leave_game.output_compressed();
		network::send_raw_data(leave_game_data.begin(), leave_game_data.size(), user->first);
		return user->first;
	}
	// Don't return the user if he wasn't in this game.
	return 0;
}

void game::process_message(simple_wml::document& data, const player_map::iterator user) {
	// Hack to handle the pseudo game lobby_.
	if (owner_ != 0) {
	} else if (user->second.silenced()) {
		return;
	} else if (user->second.is_message_flooding()) {
		send_server_message(
				"Warning: you are sending too many messages too fast. "
				"Your message has not been relayed.", user->first);
		return;
	}

	simple_wml::node* const message = data.root().child("message");
	assert(message);
	message->set_attr_dup("sender", user->second.name().c_str());
	
	const simple_wml::string_span& msg = (*message)["message"];
	if(msg.size() > chat_message::max_message_length) {
		t_string str(msg.begin(), msg.end());
		chat_message::truncate_message(str);
		message->set_attr_dup("message", str.c_str());
	}

	send_data(data, user->first);
}

bool game::is_legal_command(const simple_wml::node& command, bool is_player) {
	// Only single commands allowed.
	if (!command.one_child()) return false;
	// Chatting is never an illegal command.
	if (command.child("speak")) return true;
	if (is_player
	&& (command.child("label")
		|| command.child("clear_labels")
		|| command.child("rename")
		|| command.child("countdown_update")
		/* || command.child("choose")*/))
	{
		return true;
	}
	return false;
}

//! Handles [end_turn], repackages [commands] with private [speak]s in them
//! and sends the data.
//! Also filters commands from all but the current player.
//! Currently removes all commands but [speak] for observers and all but
//! [speak], [label] and [rename] for players.
//! Returns true if the turn ended.
bool game::process_turn(simple_wml::document& data, const player_map::const_iterator user) {
	//DBG_GAME << "processing commands: '" << cfg << "'\n";
	if (!started_) return false;
	simple_wml::node* const turn = data.root().child("turn");
	bool turn_ended = false;
	// Any private 'speak' commands must be repackaged separate
	// to other commands, and re-sent, since they should only go
	// to some clients.
	bool repackage = false;
	int index = 0;
	std::vector<int> marked;
	const simple_wml::node::child_list& commands = turn->children("command");
	simple_wml::node::child_list::const_iterator command;
	for (command = commands.begin(); command != commands.end(); ++command) {
		if (!is_current_player(user->first)
		&& !is_legal_command(**command, is_player(user->first))) {
			std::cerr << "ILLEGAL COMMAND: (((" << data.output() << ")))\n";
			std::stringstream msg;
			msg << "Removing illegal command from: " << user->second.name()
				<< ". Current player is: "
				<< (player_info_->find(current_player()) != player_info_->end()
					? player_info_->find(current_player())->second.name()
					: "(unfound) ") << nsides_ << "/" << end_turn_
				<< ".\n";
			LOG_GAME << msg.str();
			send_and_record_server_message(msg.str().c_str());
			//TODO: make an easy way to convert a simple wml node to a string
			//LOG_GAME << (**command).output();
			marked.push_back(index - marked.size());
		} else if ((**command).child("speak")) {
			simple_wml::node& speak = *(**command).child("speak");
			if (!(speak["team_name"] == "")
			|| (is_muted_observer(user->first))) {
				repackage = true;
			}

			const simple_wml::string_span& msg = speak["message"];
			if(msg.size() > chat_message::max_message_length) {
				t_string str(msg.begin(), msg.end());
				chat_message::truncate_message(str);
				speak.set_attr_dup("message", str.c_str());
			}

			// Force the description to be correct,
			// to prevent spoofing of messages.
			speak.set_attr_dup("description", user->second.name().c_str());
			// Also check the side for players.
			if (is_player(user->first)) {
				const size_t side_num = speak["side"].to_int();
				if (side_num < 1 || side_num > gamemap::MAX_PLAYERS
				|| sides_[side_num - 1] != user->first) {
					if (user->first == current_player()) {
						speak.set_attr_dup("side", lexical_cast<std::string>(current_side() + 1).c_str());
					} else {
						const side_vector::const_iterator s =
								std::find(sides_.begin(), sides_.end(), user->first);
						speak.set_attr_dup("side", lexical_cast<std::string>(s - sides_.begin() + 1).c_str());
					}
				}
			}
		} else if (is_current_player(user->first) && (**command).child("end_turn")) {
			turn_ended = end_turn();
		}
		++index;
	}
	for(std::vector<int>::const_iterator j = marked.begin(); j != marked.end(); ++j) {
		turn->remove_child("command",*j);
	}
	if (turn->no_children()) {
		return false;
	}
	if (!repackage) {
		record_data(data.clone());
		send_data(data, user->first);
		return turn_ended;
	}
	for (command = commands.begin(); command != commands.end(); ++command) {
		simple_wml::node* const speak = (**command).child("speak");
		if (speak == NULL) {
			simple_wml::document* mdata = new simple_wml::document;
			simple_wml::node& turn = mdata->root().add_child("turn");
			(**command).copy_into(turn.add_child("command"));
			send_data(*mdata, user->first);
			record_data(mdata);
			continue;
		}
		const simple_wml::string_span& team_name = (*speak)["team_name"];
		// Anyone can send to the observer team.
		if (team_name == game_config::observer_team_name.c_str()) {
		// Don't send if the member is muted.
		} else if (is_muted_observer(user->first)) {
			send_server_message("You have been muted, others can't see your message!", user->first);
			continue;
		// Don't send if the player addresses a different team.
		} else if (!is_on_team(team_name, user->first)) {
			std::ostringstream msg;
			msg << "Removing illegal message from " << user->second.name() << " to " << std::string(team_name.begin(), team_name.end()) << ".";
			const std::string& msg_str = msg.str();
			LOG_GAME << msg_str << std::endl;
			send_and_record_server_message(msg_str.c_str());
			continue;
		}

		std::auto_ptr<simple_wml::document> message(new simple_wml::document);
		simple_wml::node& turn = message->root().add_child("turn");
		simple_wml::node& command = turn.add_child("command");
		speak->copy_into(command.add_child("speak"));
		if (team_name == "") {
			send_data(*message, user->first);
			record_data(message.release());
		} else if (team_name == game_config::observer_team_name) {
			send_data_observers(*message, user->first);
			record_data(message.release());
		} else {
			send_data_team(*message, team_name, user->first);
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

	description_->set_attr_dup("turn", describe_turns(current_turn(), level_["turns"]).c_str());

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
		send_and_record_server_message((user->second.name()
				+ " has joined the game.").c_str(), player);
	} else if (!allow_observers()) {
		return; //false;
	} else {
		DBG_GAME << "adding observer...\n";
		observers_.push_back(player);

		simple_wml::document observer_join;
		observer_join.root().add_child("observer").set_attr_dup("name", user->second.name().c_str());

		// Send observer join to everyone except the new observer.
		send_data(observer_join, player);
	}
	DBG_GAME << debug_player_info();
	// Send the user the game data.
	//std::cerr << "SENDING LEVEL {{{" << level_.output() << "}}}\n";
	simple_wml::string_span level_data = level_.output_compressed();
	network::send_raw_data(level_data.begin(), level_data.size(), player);
	if(started_) {
		//tell this player that the game has started
		static simple_wml::document start_game_doc("[start_game]\n[/start_game]\n", simple_wml::INIT_COMPRESSED);
		static const simple_wml::string_span start_game = start_game_doc.output_compressed();
		network::send_raw_data(start_game.begin(), start_game.size(), player);
		// Send observer join of all the observers in the game to the new player
		// only once the game started. The client forgets about it anyway
		// otherwise.
		send_observerjoins(player);
		// Send the player the history of the game to-date.
		send_history(player);
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
		send_server_message((user->second.name() + " ended the game.").c_str(), player);
		return true;
	}
	// Don't mark_available() since the player got already removed from the
	// games_and_users_list_.
	if (!disconnect) user->second.mark_available();
	if (observer) {
		//they're just an observer, so send them having quit to clients
		simple_wml::document observer_quit;

		//don't need to dup the attribute because this document is
		//short-lived.
		observer_quit.root().add_child("observer_quit").set_attr("name", user->second.name().c_str());
		send_data(observer_quit);
	} else {
		send_and_record_server_message((user->second.name()
				+ (disconnect ? " has disconnected." : " has left the game.")).c_str(), player);
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
		char side_drop_buf[64];
		sprintf(side_drop_buf, "%d", static_cast<int>(side_num + 1));
		simple_wml::document drop;
		drop.root().set_attr("side_drop", side_drop_buf);
		drop.root().set_attr("controller", side_controllers_[side_num].c_str());

		send_to_one(drop, owner_);
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
	simple_wml::document cfg;
	cfg.root().add_child("gamelist");
	user_vector users = all_game_users();
	for(user_vector::const_iterator p = users.begin(); p != users.end(); ++p) {
		const player_map::const_iterator pl = player_info_->find(*p);
		if (pl != player_info_->end()) {
			//don't need to duplicate pl->second.name().c_str() because the
			//document will be destroyed by the end of the function
			cfg.root().add_child("user").set_attr("name", pl->second.name().c_str());
		}
	}
	send_data(cfg, exclude);
}

//! A member asks for the next scenario to advance to.
void game::load_next_scenario(const player_map::const_iterator user) const {
	send_server_message((user->second.name() + " advances to the next scenario").c_str(), user->first);
	simple_wml::document cfg_scenario;
	level_.root().copy_into(cfg_scenario.root().add_child("next_scenario"));
	simple_wml::string_span data = cfg_scenario.output_compressed();
	network::send_raw_data(data.begin(), data.size(), user->first);
	// Send the player the history of the game to-date.
	send_history(user->first);
	// Send observer join of all the observers in the game to the user.
	send_observerjoins(user->first);
}

void game::send_data(simple_wml::document& data, const network::connection exclude) const
{
	simple_wml::string_span s = data.output_compressed();
	const user_vector& users = all_game_users();
	for(user_vector::const_iterator i = users.begin(); i != users.end(); ++i) {
		if (*i != exclude) {
			network::send_raw_data(s.begin(), s.size(), *i);
		}
	}
}

void game::send_to_one(simple_wml::document& data, const network::connection sock) const
{
	simple_wml::string_span s = data.output_compressed();
	network::send_raw_data(s.begin(), s.size(), sock);
}

void game::send_data_team(simple_wml::document& data,
                          const simple_wml::string_span& team,
                          const network::connection exclude) const
{
	simple_wml::string_span s = data.output_compressed();
	for(user_vector::const_iterator i = players_.begin(); i != players_.end(); ++i) {
		if(*i != exclude && is_on_team(team,*i)) {
			network::send_raw_data(s.begin(), s.size(), *i);
		}
	}
}

void game::send_data_observers(simple_wml::document& data, const network::connection exclude) const {
	simple_wml::string_span s = data.output_compressed();
	for(user_vector::const_iterator i = observers_.begin(); i != observers_.end(); ++i) {
		if (*i != exclude) {
			network::send_raw_data(s.begin(), s.size(), *i);
		}
	}
}

bool game::is_on_team(const simple_wml::string_span& team, const network::connection player) const {
	side_vector::const_iterator side = std::find(sides_.begin(), sides_.end(), player);
	if(side == sides_.end()) {
		return false;
	}
	const std::string side_str =
		lexical_cast<std::string, size_t>(side - sides_.begin() + 1);
	const simple_wml::node::child_list& side_list = level_.root().children("side");
	for(simple_wml::node::child_list::const_iterator i = side_list.begin();
	    i != side_list.end(); ++i) {
		if((**i)["side"] == side_str.c_str()) {
			return (**i)["team_name"] == team;
		}
	}

	return false;
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

		simple_wml::document cfg;
		cfg.root().add_child("observer").set_attr_dup("name", obs->second.name().c_str());
		if (sock == 0) {
			// Send to everyone except the observer in question.
			send_data(cfg, *ob);
		} else {
			// Send to the (new) user.
			const simple_wml::string_span& data = cfg.output_compressed();
			network::send_raw_data(data.begin(), data.size(), sock);
		}
	}
}

void game::send_history(const network::connection sock) const
{
	for(std::vector<simple_wml::document*>::const_iterator i = history_.begin();
	    i != history_.end(); ++i) {
		const simple_wml::string_span& data = (*i)->output_compressed();
		network::send_raw_data(data.begin(), data.size(), sock);
	}
}

void game::record_data(simple_wml::document* data) {
	data->compress();
	history_.push_back(data);
}

void game::set_description(simple_wml::node* desc) {
	description_ = desc;
	if(!password_.empty()) {
		description_->set_attr("password", "yes");
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
player_map::const_iterator game::find_user(const simple_wml::string_span& name) const {
	player_map::const_iterator pl;
	for (pl = player_info_->begin(); pl != player_info_->end(); pl++) {
		if (name == pl->second.name().c_str()) {
			return pl;
		}
	}
	return player_info_->end();
}

void game::send_and_record_server_message(const char* message,
		                                  const network::connection exclude)
{
	simple_wml::document* doc = new simple_wml::document;
	send_server_message(message, 0, doc);
	send_data(*doc, exclude);
	record_data(doc);
}

void game::send_server_message(const char* message, network::connection sock, simple_wml::document* docptr) const
{
	simple_wml::document docbuf;
	if(docptr == NULL) {
		docptr = &docbuf;
	}

	simple_wml::document& doc = *docptr;
	if(started_) {
		simple_wml::node& cmd = doc.root().add_child("turn");
		simple_wml::node& cfg = cmd.add_child("command");
		simple_wml::node& msg = cfg.add_child("speak");
		msg.set_attr("description", "server");
		msg.set_attr_dup("message", message);
	} else {
		simple_wml::node& msg = doc.root().add_child("message");
		msg.set_attr("sender", "server");
		msg.set_attr_dup("message", message);
	}

	if(sock) {
		simple_wml::string_span str = doc.output_compressed();
		network::send_raw_data(str.begin(), str.size(), sock);
	}
}
