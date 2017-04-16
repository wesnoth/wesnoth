/*
   Copyright (C) 2003 - 2015 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "../global.hpp"

#include "../filesystem.hpp"
#include "../game_config.hpp" // game_config::observer_team_name
#include "../log.hpp"
#include "../map.hpp" // gamemap::MAX_PLAYERS

#include "game.hpp"
#include "player_network.hpp"
#include "serialization/string_utils.hpp"
#include "util.hpp"
#include "utils/foreach.tpp"

#include <boost/bind.hpp>

static lg::log_domain log_server("server");
#define ERR_GAME LOG_STREAM(err, log_server)
#define WRN_GAME LOG_STREAM(warn, log_server)
#define LOG_GAME LOG_STREAM(info, log_server)
#define DBG_GAME LOG_STREAM(debug, log_server)
static lg::log_domain log_config("config");
#define WRN_CONFIG LOG_STREAM(warn, log_config)

namespace wesnothd {
int game::id_num = 1;

void game::missing_user(network::connection socket, const std::string& func) const
{
	WRN_GAME << func << "(): Could not find user (socket:\t" << socket
		<< ") in player_info_ in game:\t\"" << name_ << "\" (" << id_ << ")\n";
}

game::game(player_map& players, const network::connection host,
		const std::string& name, bool save_replays,
		const std::string& replay_save_path) :
	player_info_(&players),
	id_(id_num++),
	name_(name),
	password_(),
	owner_(host),
	players_(),
	observers_(),
	muted_observers_(),
	sides_(gamemap::MAX_PLAYERS),
	side_controllers_(gamemap::MAX_PLAYERS),
	nsides_(0),
	started_(false),
	level_(),
	history_(),
	description_(NULL),
	end_turn_(0),
	all_observers_muted_(false),
	bans_(),
	termination_(),
	save_replays_(save_replays),
	replay_save_path_(replay_save_path),
	global_wait_side_(0),
	are_updating_game_(false)
{
	assert(owner_);
	players_.push_back(owner_);
	const player_map::iterator pl = player_info_->find(owner_);
	if (pl == player_info_->end()) {
		missing_user(owner_, __func__);
		return;
	}
	// Mark the host as unavailable in the lobby.
	pl->second.mark_available(id_, name_);
	pl->second.set_status(player::PLAYING);
}

game::~game()
{
	save_replay();

	user_vector users = all_game_users();
	for (user_vector::const_iterator u = users.begin(); u != users.end(); ++u) {
		remove_player(*u, false, true);
	}
	clear_history();
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
	snprintf(buf,sizeof(buf),"%d/",turn);

	if(num_turns == "-1") {
		return buf + std::string("-");
	} else {
		return buf + std::string(num_turns.begin(), num_turns.end());
	}
}

}//anon namespace

std::string game::username(const player_map::const_iterator pl) const
{
	if (pl != player_info_->end()) {
		return pl->second.name();
	}

	return "(unknown)";
}

std::string game::list_users(user_vector users, const std::string& func) const
{
	std::string list;

	for (user_vector::const_iterator user = users.begin(); user != users.end(); ++user) {
		const player_map::const_iterator pl = player_info_->find(*user);
		if (pl != player_info_->end()) {
			if (!list.empty()) list += ", ";
			list += pl->second.name();
		} else missing_user(*user, func);
	}

	return list;
}

void game::perform_controller_tweaks(const player_map::const_iterator starter) {
	const simple_wml::node::child_list & sides = level_.root().children("side");

	DBG_GAME << "****\n Performing controller tweaks. sides = " << std::endl; 
	DBG_GAME << debug_sides_info() << std::endl;
	DBG_GAME << "****" << std::endl;

	//this needs to happen before the controller tweaks if some players are lingering.
	if (are_updating_game_) {
		set_all_players_to_not_updated();
	}
	starter->second.set_updated_level(true); //the host of course has the updated level.
	are_updating_game = false;

	update_side_data(); // Necessary to read the level_ and get sides_, etc. updated to match

	nsides_ = 0;

	for(simple_wml::node::child_list::const_iterator s = sides.begin(); s != sides.end(); ++s) {
		nsides_++;
		if ((**s)["controller"] != "null") {
			int side_num = (**s)["side"].to_int() - 1;
			
			if (sides_[side_num] == 0) {
				sides_[side_num] = owner_;
				std::stringstream msg;
				msg << "Side "  << side_num + 1 << " had no controller during controller tweaks! The host was assigned control.";
				LOG_GAME << msg.str() << " (game id: " << id_ << ")\n";
				send_and_record_server_message(msg.str());
			}
			
			const player_map::const_iterator user = player_info_->find(sides_[side_num]);
			std::string user_name = "null (server missing user)";
			if (user == player_info_->end()) {
				missing_user(user->first, __func__);
			} else {			
				user_name = username(user);
			}

			change_controller(side_num, sides_[side_num], user_name , false, (**s)["controller"].to_string());

			//next lines change controller types found in level_ to be what is appropriate for an observer at game start.
			if ((**s)["controller"] == "ai") {
				(*s)->set_attr("controller", "network_ai");
			} else {	//this catches "reserved" also
				(*s)->set_attr("controller", "network");
			}

			if (sides_[side_num] == 0) {
				std::stringstream msg;
				msg << "Side "  << side_num + 1 << " had no controller AFTER controller tweaks! Ruh Roh!";
				LOG_GAME << msg.str() << " (game id: " << id_ << ")\n";
			}

		}
	}

	update_side_data(); // this is the last time that update_side_data will actually run, as now the game will start and started_ will be true.

	//TODO: Does it matter that the server is telling the host to change a bunch of sides? 
	//According to playturn.cpp, the host should ignore all such messages. Still might be better
	//not to send them at all, although not if it complicates the server code.
}

void game::start_game(const player_map::const_iterator starter) {
	const simple_wml::node::child_list & sides = level_.root().children("side");
	DBG_GAME << "****\n Starting game. sides = " << std::endl; 
	DBG_GAME << debug_sides_info() << std::endl;
	DBG_GAME << "****" << std::endl;


	started_ = true;
	// Prevent inserting empty keys when reading.
	const simple_wml::node& s = level_.root();
	const bool save = s["savegame"].to_bool();
	LOG_GAME << network::ip_address(starter->first) << "\t"
		<< starter->second.name() << "\t" << "started"
		<< (save ? " reloaded" : "") << " game:\t\"" << name_ << "\" (" << id_
		<< ") with: " << list_users(players_, __func__) << ". Settings: map: " << s["id"]
		<< "\tera: "       << (s.child("era") ? (*s.child("era"))["id"] : "")
		<< "\tXP: "        << s["experience_modifier"]
		<< "\tGPV: "       << s["mp_village_gold"]
		<< "\tfog: "       << s["mp_fog"]
		<< "\tshroud: "    << s["mp_shroud"]
		<< "\tobservers: " << s["observer"]
		<< "\tshuffle: "   << s["shuffle_sides"]
		<< "\ttimer: "     << s["mp_countdown"]
		<< (s["mp_countdown"].to_bool() ?
			"\treservoir time: " + s["mp_countdown_reservoir_time"].to_string() +
			"\tinit time: "      + s["mp_countdown_init_time"].to_string() +
			"\taction bonus: "   + s["mp_countdown_action_bonus"].to_string() +
			"\tturn bonus: "     + s["mp_countdown_turn_bonus"].to_string() : "")
		<< "\n";

	for(simple_wml::node::child_list::const_iterator s = sides.begin(); s != sides.end(); ++s) {
		if ((**s)["controller"] != "null") {
			int side_num = (**s)["side"].to_int() - 1;
			if (sides_[side_num] == 0) {
				std::stringstream msg;
				msg << "Side "  << side_num + 1 << " has no controller but should! The host needs to assign control for the game to proceed past that side's turn.";
				LOG_GAME << msg.str() << " (game id: " << id_ << ")\n";
				send_and_record_server_message(msg.str());
			}
		}
	}

	DBG_GAME << "Number of sides: " << nsides_ << "\n";
	int turn = 1;
	int side = 0;
	// Savegames have a snapshot that tells us which side starts.
	if (const simple_wml::node* snapshot = s.child("snapshot")) {
		turn = lexical_cast_default<int>((*snapshot)["turn_at"], 1);
		side = lexical_cast_default<int>((*snapshot)["playing_team"], 0);
		LOG_GAME << "Reload from turn: " << turn
			<< ". Current side is: " << side + 1 << ".\n";
	}
	end_turn_ = (turn - 1) * nsides_ + side - 1;
	end_turn();
	clear_history();
	// Send [observer] tags for all observers that are already in the game.
	send_observerjoins();
}

void game::update_game()
{
	started_ = false;
	are_updating_game_ = true;

	description_->set_attr("turn", "");

	update_side_data();
	describe_slots();
}

void game::set_all_players_to_not_updated()
{
	user_vector allusers = all_game_users(false); // false means we don't exclude lingering players from previous level, but there shouldn't be any right now
	for (user_vector::const_iterator x = allusers.begin(); x != allusers.end(); ++x) {
		x->set_updated_level(false); //the host now has updated the level, and the other players need to download it, and in the mean time not recieve game events. they will eventually get them in the history.
	}
}

bool game::send_taken_side(simple_wml::document& cfg, const simple_wml::node::child_list::const_iterator side) const
{
	const size_t side_num = (**side)["side"].to_int();
	if (side_num < 1 || side_num > gamemap::MAX_PLAYERS) return false;
	if (sides_[side_num - 1] != 0) return false;
	// We expect that the host will really use our proposed side number. (He could do different...)
	cfg.root().set_attr_dup("side", (**side)["side"]);

	// Tell the host which side the new player should take.
	return wesnothd::send_to_one(cfg, owner_);
}

bool game::take_side(const player_map::const_iterator user)
{
	DBG_GAME << "take_side...\n";

	if (started_) return false;

	simple_wml::document cfg;
	cfg.root().set_attr_dup("name", user->second.name().c_str());
	cfg.root().set_attr("faction", "random");
	cfg.root().set_attr("leader", "random");
	cfg.root().set_attr("gender", "random");

	// Check if we can figure out a fitting side.
	const simple_wml::node::child_list& sides = level_.root().children("side");
	for(simple_wml::node::child_list::const_iterator side = sides.begin(); side != sides.end(); ++side) {
		if(((**side)["controller"] == "network" || (**side)["controller"] == "reserved")
				&& ((**side)["save_id"] == user->second.name().c_str()
				|| (**side)["current_player"] == user->second.name().c_str()))
		{
			if (send_taken_side(cfg, side)) return true;
		}
	}
	// If there was no fitting side just take the first available.
	for(simple_wml::node::child_list::const_iterator side = sides.begin(); side != sides.end(); ++side) {
		if((**side)["controller"] == "network") {
			if (send_taken_side(cfg, side)) return true;
		}
	}
	DBG_GAME << "take_side: there are no more sides available\n";
	//if we get here we couldn't find a side to take
	return false;
}

void game::update_side_data() {
				//added by iceiceice: since level_ will now reflect how an observer 
	if (started_) return; 	//views the replay start position and not the current position, the sides_, side_controllers_,
				//players_ info should not be updated from the level_ after the game has started.
				//controller changes are now stored in the history, so an observer that joins will get up to
				//date that way.

	DBG_GAME << "update_side_data...\n";
	DBG_GAME << debug_player_info();
	// Remember everyone that is in the game.
	const user_vector users = all_game_users();

	side_controllers_.clear();
	side_controllers_.resize(gamemap::MAX_PLAYERS);
	sides_.clear();
	sides_.resize(gamemap::MAX_PLAYERS);
	players_.clear();
	observers_.clear();

	const simple_wml::node::child_list& level_sides = level_.root().children("side");
	/* This causes data corruption for some reason
	if (!lg::debug.dont_log(log_server)) {
		for (simple_wml::node::child_list::const_iterator side = level_sides.begin();
				side != level_sides.end(); ++side)
			DBG_GAME << "[side]\n" << simple_wml::node_to_string(**side) << "[/side]\n";
	}*/
	// For each user:
	// * Find the username.
	// * Find the side this username corresponds to.
	for (user_vector::const_iterator user = users.begin(); user != users.end(); ++user) {
		player_map::iterator info = player_info_->find(*user);
		if (info == player_info_->end()) {
			missing_user(*user, __func__);
			continue;
		}

		bool side_found = false;
		for (simple_wml::node::child_list::const_iterator side = level_sides.begin();
				side != level_sides.end(); ++side)
		{
			int side_num = (**side)["side"].to_int() - 1;
			if (side_num < 0 || side_num >= gamemap::MAX_PLAYERS
					|| sides_[side_num] != 0) continue;

			if ((**side)["controller"] == "network") {
				if ((**side)["current_player"] == info->second.name().c_str()) {
					side_controllers_[side_num] = "human";
					sides_[side_num] = *user;
					side_found = true;
				}
			} else if (*user == owner_) {
				if ((**side)["controller"] == "ai" || (**side)["controller"] == "human") {
					side_controllers_[side_num] = (**side)["controller"].to_string();
					sides_[side_num] = owner_;
					side_found = true;
				} else if ((**side)["controller"] == "network_ai") {
					side_controllers_[side_num] = "ai"; //on server this field should only contain "ai" for ais. (there are no ais local to server)
					sides_[side_num] = owner_;
					side_found = true;					
				} else {
					// "null"
					side_controllers_[side_num] = (**side)["controller"].to_string();
				}
			} else {
				// "reserved"
				side_controllers_[side_num] = (**side)["controller"].to_string();
			}
		}
		if (side_found) {
			players_.push_back(*user);
			info->second.set_status(player::PLAYING);
		} else {
			observers_.push_back(*user);
			info->second.set_status(player::OBSERVING);
		}
	}
	DBG_GAME << debug_player_info();
}

void game::transfer_side_control(const network::connection sock, const simple_wml::node& cfg) {
	DBG_GAME << "transfer_side_control...\n";
	if (!is_player(sock) && sock != owner_) {
		send_server_message("You cannot change controllers: not a player.", sock);
		return;
	}

	// Check the side number.
	const unsigned int side_num = cfg["side"].to_int();
	if(side_num < 1 || side_num > gamemap::MAX_PLAYERS) {
		std::ostringstream msg;
		msg << "The side number has to be between 1 and "
		    << gamemap::MAX_PLAYERS << ".";
		send_server_message(msg.str(), sock);
		return;
	}

	if (side_num > level_.root().children("side").size()) {
		send_server_message("Invalid side number.", sock);
		return;
	}

	const simple_wml::string_span& newplayer_name = cfg["player"];
	const network::connection old_player = sides_[side_num - 1];
	const player_map::iterator oldplayer = player_info_->find(old_player);
	if (oldplayer == player_info_->end()) missing_user(old_player, __func__);
	const std::string old_player_name = username(oldplayer);

	// A player (un)droids his side.
	if (newplayer_name.empty()) {
		if (sock != old_player) {
			if (cfg["controller"].empty()) {
				send_server_message("No player name or controller type given.", sock);
			} else {
				send_server_message("You can only (un)droid your own sides!", sock);
			}
			return;
		} else if (cfg["controller"] != "ai" && cfg["controller"] != "human") {
			std::stringstream msg;
			msg << "Wrong controller type received: '" << cfg["controller"] << "'";
			DBG_GAME << msg.str() << "\n";
			send_server_message(msg.str(), sock);
			return;
		}
		change_controller(side_num - 1, old_player, old_player_name, false, cfg["controller"].to_string());
		return;
	}

	// Check if the sender actually owns the side he gives away or is the host.
	if (!(sock == old_player || sock == owner_)) {
		std::stringstream msg;
		msg << "You can't give away side " << side_num << ". It's controlled by '"
			<< old_player_name << "' not you.";
		DBG_GAME << msg.str() << "\n";
		send_server_message(msg.str(), sock);
		return;
	}
	//find the player that is passed control
	player_map::iterator newplayer = find_user(newplayer_name);

	// Is he in this game?
	if (newplayer == player_info_->end() || !is_member(newplayer->first)) {
		send_server_message(newplayer_name.to_string() + " is not in this game", sock);
		return;
	}

	if (newplayer->first == old_player) {
		std::stringstream msg;
		msg << "That's already " << newplayer_name << "'s side, silly.";
		send_server_message(msg.str(), sock);
		return;
	}
	sides_[side_num - 1] = 0;
	// If the old player lost his last side, make him an observer.
	if (std::find(sides_.begin(), sides_.end(), old_player) == sides_.end()
	&& is_player(old_player)) {
		observers_.push_back(old_player);
		oldplayer->second.set_status(player::OBSERVING);
		players_.erase(std::remove(players_.begin(), players_.end(), old_player), players_.end());
		// Tell others that the player becomes an observer.
		send_and_record_server_message(old_player_name + " becomes an observer.");
		// Update the client side observer list for everyone except old player.
		simple_wml::document observer_join;
		observer_join.root().add_child("observer").set_attr_dup("name", old_player_name.c_str());
		send_data(observer_join, old_player);
	}
	change_controller(side_num - 1, newplayer->first, newplayer->second.name(), false);

	// If we gave the new side to an observer add him to players_.
	if (is_observer(newplayer->first)) {
		players_.push_back(newplayer->first);
		newplayer->second.set_status(player::PLAYING);
		observers_.erase(std::remove(observers_.begin(), observers_.end(), newplayer->first), observers_.end());
		// Send everyone but the new player the observer_quit message.
		send_observerquit(newplayer);
	}
}

void game::change_controller(const size_t side_num,
		const network::connection sock,
		const std::string& player_name,
		const bool player_left,
		const std::string& controller)
{
	DBG_GAME << __func__ << "...\n";

	const std::string& side = lexical_cast_default<std::string, size_t>(side_num + 1);
	sides_[side_num] = sock;

	if (player_left && side_controllers_[side_num] == "ai") {
		// Automatic AI side transfer.
	} else if (controller.empty()) {
		if (started_) {
			send_and_record_server_message(player_name + " takes control of side " + side + ".");
		}
		side_controllers_[side_num] = "human";
	} else {
		if (started_) {
			send_and_record_server_message(player_name + (controller == "ai" ? " " : " un")
					+ "droids side " + side + ".");
		}
		side_controllers_[side_num] = (controller == "ai" ? "ai" : "human");
	}

	simple_wml::document response;
	simple_wml::node& change = response.root().add_child("change_controller");

	change.set_attr("side", side.c_str());
	change.set_attr("player", player_name.c_str());

	// Tell everyone but the new player that this side's controller changed.
	change.set_attr("controller", (side_controllers_[side_num] == "ai" ? "network_ai" : "network"));

	send_data(response, sock);
	if (started_) { //this is added instead of the if (started_) {...} below
		simple_wml::document *record = new simple_wml::document;
		simple_wml::node& recchg = record->root().add_child("change_controller");
		recchg.set_attr_dup("side", side.c_str());
		recchg.set_attr_dup("player", player_name.c_str());
		recchg.set_attr_dup("controller", (side_controllers_[side_num] == "ai" ? "network_ai" : "network"));

		record_data(record); //the purpose of these records is so that observers, replay viewers, get controller updates correctly
	}

	// Tell the new player that he controls this side now.
	// Just don't send it when the player left the game. (The host gets the
	// side_drop already.)
	if (!player_left) {
		change.set_attr("controller", (side_controllers_[side_num] == "ai" ? "ai" : "human"));
		wesnothd::send_to_one(response, sock);
	}
}

void game::notify_new_host(){
	const std::string owner_name = username(player_info_->find(owner_));
	simple_wml::document cfg;
	simple_wml::node& cfg_host_transfer = cfg.root().add_child("host_transfer");

	// Why do we send the new host his own name?
	cfg_host_transfer.set_attr("name", owner_name.c_str());
	cfg_host_transfer.set_attr("value", "1");
	std::string message = owner_name + " has been chosen as the new host.";
	if (!wesnothd::send_to_one(cfg, owner_)) {
		message += " But an internal error occurred. You probably have to abandon this game.";
	}
	send_and_record_server_message(message);
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
		} else if (sides_[i] == 0) {
			++available_slots;
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

void game::send_muted_observers(const player_map::const_iterator user) const
{
	if (all_observers_muted_) {
		send_server_message("All observers are muted.", user->first);
		return;
	}
	std::string muted_nicks = list_users(muted_observers_, __func__);

	send_server_message("Muted observers: " + muted_nicks, user->first);
}

void game::mute_observer(const simple_wml::node& mute,
		const player_map::const_iterator muter)
{
	if (muter->first != owner_) {
		send_server_message("You cannot mute: not the game host.", muter->first);
		return;
	}
	const simple_wml::string_span& username = mute["username"];
	if (username.empty()) {
		send_muted_observers(muter);
		return;
	}

	const player_map::const_iterator user = find_user(username);
	/**
	 * @todo FIXME: Maybe rather save muted nicks as a set of strings and
	 * also allow muting of usernames not in the game.
	 */
	if (user == player_info_->end() || !is_observer(user->first)) {
		send_server_message("Observer '" + username.to_string() + "' not found.", muter->first);
		return;
	}

	// Prevent muting ourselves.
	if (user->first == muter->first) {
		send_server_message("Don't mute yourself, silly.", muter->first);
		return;
	}
	if (is_muted_observer(user->first)) {
		send_server_message(username.to_string() + " is already muted.", muter->first);
		return;
	}
	LOG_GAME << network::ip_address(muter->first) << "\t"
		<< muter->second.name() << " muted: " << username << " ("
		<< network::ip_address(user->first) << ")\tin game:\t\""
		<< name_ << "\" (" << id_ << ")\n";
	muted_observers_.push_back(user->first);
	send_and_record_server_message(username.to_string() + " has been muted.");
}

void game::unmute_observer(const simple_wml::node& unmute,
		const player_map::const_iterator unmuter)
{
	if (unmuter->first != owner_) {
		send_server_message("You cannot unmute: not the game host.", unmuter->first);
		return;
	}
	const simple_wml::string_span& username = unmute["username"];
	if (username.empty()) {
		muted_observers_.clear();
		send_and_record_server_message("Everyone has been unmuted.");
		return;
	}

	const player_map::const_iterator user = find_user(username);
	if (user == player_info_->end() || !is_observer(user->first)) {
		send_server_message("Observer '" + username.to_string() + "' not found.", unmuter->first);
		return;
	}

	if (!is_muted_observer(user->first)) {
		send_server_message(username.to_string() + " is not muted.", unmuter->first);
		return;
	}

	LOG_GAME << network::ip_address(unmuter->first) << "\t"
		<< unmuter->second.name() << " unmuted: " << username << " ("
		<< network::ip_address(user->first) << ")\tin game:\t\""
		<< name_ << "\" (" << id_ << ")\n";
	muted_observers_.erase(std::remove(muted_observers_.begin(),
				muted_observers_.end(), user->first), muted_observers_.end());
	send_and_record_server_message(username.to_string() + " has been unmuted.");
}

void game::send_leave_game(network::connection user) const
{
	static simple_wml::document leave_game("[leave_game]\n[/leave_game]\n", simple_wml::INIT_COMPRESSED);
	wesnothd::send_to_one(leave_game, user);
}

network::connection game::kick_member(const simple_wml::node& kick,
		const player_map::const_iterator kicker)
{
	if (kicker->first != owner_) {
		send_server_message("You cannot kick: not the game host", kicker->first);
		return 0;
	}
	const simple_wml::string_span& username = kick["username"];
	const player_map::const_iterator user = find_user(username);
	if (user == player_info_->end() || !is_member(user->first)) {
		send_server_message("'" + username.to_string() + "' is not a member of this game.", kicker->first);
		return 0;
	} else if (user->first == kicker->first) {
		send_server_message("Don't kick yourself, silly.", kicker->first);
		return 0;
	} else if (user->second.is_moderator()) {
		send_server_message("You're not allowed to kick a moderator.", kicker->first);
		return 0;
	}
	LOG_GAME << network::ip_address(kicker->first) << "\t"
		<< kicker->second.name() << "\tkicked: " << username << " ("
		<< network::ip_address(user->first) << ")\tfrom game:\t\""
		<< name_ << "\" (" << id_ << ")\n";
	send_and_record_server_message(username.to_string() + " has been kicked.");

	// Tell the user to leave the game.
	send_leave_game(user->first);
	remove_player(user->first);
	return user->first;
}

network::connection game::ban_user(const simple_wml::node& ban,
		const player_map::const_iterator banner)
{
	if (banner->first != owner_) {
		send_server_message("You cannot ban: not the game host", banner->first);
		return 0;
	}
	const simple_wml::string_span& username = ban["username"];
	const player_map::const_iterator user = find_user(username);
	if (user == player_info_->end()) {
		send_server_message("User '" + username.to_string() + "' not found.", banner->first);
		return 0;
	} else if (user->first == banner->first) {
		send_server_message("Don't ban yourself, silly.", banner->first);
		return 0;
	} else if (player_is_banned(user->first)) {
		send_server_message("'" + username.to_string() + "' is already banned.", banner->first);
		return 0;
	} else if (user->second.is_moderator()) {
		send_server_message("You're not allowed to ban a moderator.", banner->first);
		return 0;
	}
	LOG_GAME << network::ip_address(banner->first) << "\t"
		<< banner->second.name() << "\tbanned: " << username << " ("
		<< network::ip_address(user->first) << ")\tfrom game:\t\""
		<< name_ << "\" (" << id_ << ")\n";
	bans_.push_back(network::ip_address(user->first));
	send_and_record_server_message(username.to_string() + " has been banned.");
	if (is_member(user->first)) {
		//tell the user to leave the game.
		send_leave_game(user->first);
		remove_player(user->first);
		return user->first;
	}
	// Don't return the user if he wasn't in this game.
	return 0;
}

void game::unban_user(const simple_wml::node& unban,
		const player_map::const_iterator unbanner)
{
	if (unbanner->first != owner_) {
		send_server_message("You cannot unban: not the game host.", unbanner->first);
		return;
	}
	const simple_wml::string_span& username = unban["username"];
	const player_map::const_iterator user = find_user(username);
	if (user == player_info_->end()) {
		send_server_message("User '" + username.to_string() + "' not found.", unbanner->first);
		return;
	}
	if (!player_is_banned(user->first)) {
		send_server_message("'" + username.to_string() + "' is not banned.", unbanner->first);
		return;
	}
	LOG_GAME << network::ip_address(unbanner->first) << "\t"
		<< unbanner->second.name() << "\tunbanned: " << username << " ("
		<< network::ip_address(user->first) << ")\tfrom game:\t\""
		<< name_ << "\" (" << id_ << ")\n";
	bans_.erase(std::remove(bans_.begin(), bans_.end(), network::ip_address(user->first)), bans_.end());
	send_and_record_server_message(username.to_string() + " has been unbanned.");
}

void game::process_message(simple_wml::document& data, const player_map::iterator user) {
	if (owner_ == 0) {
		ERR_GAME << "No owner in game::process_message\n";
	}

	simple_wml::node* const message = data.root().child("message");
	assert(message);
	message->set_attr_dup("sender", user->second.name().c_str());

	const simple_wml::string_span& msg = (*message)["message"];
	chat_message::truncate_message(msg, *message);

	send_data(data, user->first, "game message");
}

bool game::is_legal_command(const simple_wml::node& command, bool is_player) {
	// Only single commands allowed.
	if (!command.one_child()) return false;
	// Chatting is never an illegal command.
	if (command.child("speak")) return true;
	/*
		assume the following situation: there are 2 sides, its side 1's turn, side 1 has a very slow pc, the following wml is executed:
		
		[get_global_variable]
			namespace=my_addon
			from_global=my_variable_name1
			to_local=foo1
			side=2
		[/get_global_variable]
		[get_global_variable]
			namespace=my_addon
			from_global=my_variable_name2
			to_local=foo2
			side=2
		[/get_global_variable]
		after the first [get_global_variable], both clients run the event simultaniously, 
		if player 1's pc is much faster side 2 sends the second [global_variable] before it is required on side 1 and the server doesnt accept it.
		thats why i deleted the foolowing lines.
	
	if (is_player && command.child("global_variable")) {
		const simple_wml::node *gvar = (command.child("global_variable"));
		if ((*gvar)["side"].to_int() == global_wait_side_) {
			global_wait_side_ = 0;
			return true;
		}
		return false;
	}
	*/
	
	if (is_player && command.has_attr("dependent") /*&& command.has_attr("from_side")*/) 
		//AKA it's generated by get_user_input for example [global_variable]
	{
		return true;
	}
	if (is_player
	&& (command.child("label")
		|| command.child("clear_labels")
		|| command.child("rename")
		|| command.child("countdown_update")
		|| command.child("global_variable")
		))
	{
		return true;
	}
	return false;
}

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
	const bool player = (is_player(user->first) || user->first == owner_);
	for (command = commands.begin(); command != commands.end(); ++command) {
		if (!is_current_player(user->first)
		&& !is_legal_command(**command, player)) {
			LOG_GAME << "ILLEGAL COMMAND in game: " << id_ << " ((("
				<< simple_wml::node_to_string(**command) << ")))\n";
			std::stringstream msg;
			msg << "Removing illegal command '" << (**command).first_child().to_string()
				<< "' from: " << user->second.name()
				<< ". Current player is: "
				<< username(player_info_->find(current_player()))
				<< " (" << end_turn_ + 1 << "/" << nsides_ << ").";
			LOG_GAME << msg.str() << " (socket: " << current_player()
				<< ") (game id: " << id_ << ")\n";
			send_and_record_server_message(msg.str());

			marked.push_back(index - marked.size());
		} else if ((**command).child("speak")) {
			simple_wml::node& speak = *(**command).child("speak");
			if (speak["team_name"] != "" || is_muted_observer(user->first)) {
				DBG_GAME << "repackaging..." << std::endl;
				repackage = true;
			}

			const simple_wml::string_span& msg = speak["message"];
			chat_message::truncate_message(msg, speak);

			// Force the description to be correct,
			// to prevent spoofing of messages.
			speak.set_attr_dup("id", user->second.name().c_str());
			// Also check the side for players.
			if (is_player(user->first)) {
				const size_t side_num = speak["side"].to_int();
				if (side_num < 1 || side_num > gamemap::MAX_PLAYERS
				|| sides_[side_num - 1] != user->first) {
					if (user->first == current_player()) {
						speak.set_attr_dup("side", lexical_cast_default<std::string>(current_side() + 1).c_str());
					} else {
						const side_vector::const_iterator s =
								std::find(sides_.begin(), sides_.end(), user->first);
						speak.set_attr_dup("side", lexical_cast_default<std::string>(s - sides_.begin() + 1).c_str());
					}
				}
			}
		} 
		else if ((**command).child("recall") || (**command).child("recruit")) {
			simple_wml::node& command_r = **command;
			// remove all [checkup] from recruit or replay [command]s, this fixes some false positive OOS reports caused by differences in unit checksum calculation between 1.12.4. and 1.12.5
			while(command_r.child("checkup")) {
				command_r.remove_child("checkup", 0);
			}
		}
		else if((**command).has_attr("from_side"))
		{
			if((**command)["from_side"] == "server")
			{
				//this could mean someone wants to cheat.
				(**command).set_attr("side_invalid", "true");
			}
			else
			{
				int from_side = (**command)["from_side"].to_int() - 1;
				if(from_side < 0 || static_cast<unsigned int>(from_side) >= sides_.size() ||sides_[from_side] != user->first)
				{
					//this could mean someone wants to cheat.
					(**command).set_attr("side_invalid", "true");
				}
			}
		}
		else if (is_current_player(user->first) && (**command).child("end_turn")) {
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
		send_data(data, user->first, "game replay");
		return turn_ended;
	}
	for (command = commands.begin(); command != commands.end(); ++command) {
		simple_wml::node* const speak = (**command).child("speak");
		if (speak == NULL) {
			simple_wml::document* mdata = new simple_wml::document;
			simple_wml::node& turn = mdata->root().add_child("turn");
			(**command).copy_into(turn.add_child("command"));
			send_data(*mdata, user->first, "game replay");
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
			send_and_record_server_message(msg_str);
			continue;
		}

		std::auto_ptr<simple_wml::document> message(new simple_wml::document);
		simple_wml::node& turn = message->root().add_child("turn");
		simple_wml::node& command = turn.add_child("command");
		speak->copy_into(command.add_child("speak"));
		if (team_name == "") {
			send_data(*message, user->first, "game message");
			record_data(message.release());
		} else if (team_name == game_config::observer_team_name) {
			wesnothd::send_to_many(*message, observers_, user->first, "game message");
			record_data(message.release());
		} else {
			send_data_team(*message, team_name, user->first, "game message");
		}
	}
	return turn_ended;
}

void game::require_random(const simple_wml::document &/*data*/, const player_map::iterator /*user*/)
{
	// note, that during end turn events, it's side=1 for the server but side= side_count() on the clients.
	
	int seed = rand() & 0x7FFFFFFF;
	simple_wml::document* mdata = new simple_wml::document;
	simple_wml::node& turn = mdata->root().add_child("turn");
	simple_wml::node& command = turn.add_child("command");
	simple_wml::node& random_seed = command.add_child("random_seed");
	random_seed.set_attr_int("new_seed",seed);
	command.set_attr("from_side", "server");
	command.set_attr("dependent", "yes");

	send_data(*mdata, 0, "game replay");
	record_data(mdata);

}
void game::process_whiteboard(simple_wml::document& data, const player_map::const_iterator user)
{
	if(!started_ || !is_player(user->first))
		return;

	simple_wml::node const& wb_node = *data.child("whiteboard");

	// Ensure "side" and "team_name" attributes match with user
	simple_wml::string_span const& team_name = wb_node["team_name"];
	size_t const side_num = wb_node["side"].to_int();
	if(!is_on_team(team_name,user->first)
			|| side_num < 1
			|| side_num > gamemap::MAX_PLAYERS
			|| sides_[side_num-1] != user->first)
	{
		if(sides_[side_num-1] != user->first) {
			//This case seems to happen quite frequently in mp matches, mute this warning since it gives us no new information.
			return;
		}
		std::ostringstream msg;
		msg << "Ignoring illegal whiteboard data, sent from user '" << user->second.name()
				<< "' to team '" << std::string(team_name.begin(), team_name.end()) << "'";
		if(side_num < 1 || side_num > gamemap::MAX_PLAYERS) {
			msg << " (invalid side number '" << side_num << "')";
		}
		else if(sides_[side_num-1] != user->first) {
			msg << " (player doesn't control side '" << side_num << "')";
		}
		else {
			FOREACH(AUTO p_side, level_.root().children("side"))
			{
				if((*p_side)["side"].to_int() == int(side_num)) {
					if((*p_side)["team_name"] != team_name) {
						msg << "(side '" << side_num << "' is on team '" << (*p_side)["team_name"] << "')";
					}
					break;
				}
			}
		}
		msg << "." << std::endl;
				
		const std::string& msg_str = msg.str();
		LOG_GAME << msg_str << std::endl;
		send_and_record_server_message(msg_str);
		return;
	}

	send_data_team(data,team_name,user->first,"whiteboard");
}

bool game::end_turn() {
	// It's a new turn every time each side in the game ends their turn.
	++end_turn_;
	bool turn_ended = false;
	if ((current_side()) == 0) {
		turn_ended = true;
	}
	// Skip over empty sides.
	for (int i = 0; i < nsides_ && nsides_ <= gamemap::MAX_PLAYERS && side_controllers_[current_side()] == "null"; ++i) {
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

///@todo differentiate between "observers not allowed" and "player already in the game" errors.
//      maybe return a string with an error message.
bool game::add_player(const network::connection player, bool observer) {
	if(is_member(player)) {
		ERR_GAME << "ERROR: Player is already in this game. (socket: "
			<< player << ")\n";
		return false;
	}
	const player_map::iterator user = player_info_->find(player);
	if (user == player_info_->end()) {
		missing_user(player, __func__);
		return false;
	}
	DBG_GAME << debug_player_info();
	bool became_observer = false;
	if (!started_ && !observer && take_side(user)) {
		DBG_GAME << "adding player...\n";
		players_.push_back(player);
		user->second.set_status(player::PLAYING);
		send_and_record_server_message(user->second.name() + " has joined the game.", player);
	} else if (!allow_observers() && !user->second.is_moderator()) {
		return false;
	} else {
		if (!observer) {
			became_observer = true;
			observer = true;
		}
		DBG_GAME << "adding observer...\n";
		observers_.push_back(player);
		if (!allow_observers()) send_and_record_server_message(user->second.name() + " is now observing the game.", player);

		simple_wml::document observer_join;
		observer_join.root().add_child("observer").set_attr_dup("name", user->second.name().c_str());

		// Send observer join to everyone except the new observer.
		send_data(observer_join, player);
	}
	LOG_GAME << network::ip_address(player) << "\t" << user->second.name()
		<< "\tjoined game:\t\"" << name_ << "\" (" << id_ << ")"
		<< (observer ? " as an observer" : "")
		<< ". (socket: " << player << ")\n";
	user->second.mark_available(id_, name_);
	user->second.set_status((observer) ? player::OBSERVING : player::PLAYING);
	DBG_GAME << debug_player_info();
	// Send the user the game data.
	if (!wesnothd::send_to_one(level_, player)) return false;
	player.set_updated_level(true); //the player now has the updated level and needs to recieve game events.

	if(started_) {
		//tell this player that the game has started
		static simple_wml::document start_game_doc("[start_game]\n[/start_game]\n", simple_wml::INIT_COMPRESSED);
		if (!wesnothd::send_to_one(start_game_doc, player)) return false;
		// Send observer join of all the observers in the game to the new player
		// only once the game started. The client forgets about it anyway
		// otherwise.
		send_observerjoins(player);
		// Send the player the history of the game to-date.
		send_history(player);
	} else {
		send_user_list();
	}

	const std::string clones = has_same_ip(player, observer);
	if (!clones.empty()) {
		send_and_record_server_message(user->second.name() + " has the same IP as: " + clones);
	}

	if (became_observer) {
		// in case someone took the last slot right before this player
		send_server_message("You are an observer.", player);
	}
	return true;
}

bool game::remove_player(const network::connection player, const bool disconnect, const bool destruct) {
	if (!is_member(player)) {
		ERR_GAME << "ERROR: User is not in this game. (socket: "
			<< player << ")\n";
		return false;
	}
	DBG_GAME << debug_player_info();
	DBG_GAME << "removing player...\n";

	const bool host = (player == owner_);
	const bool observer = is_observer(player);
	players_.erase(std::remove(players_.begin(), players_.end(), player), players_.end());
	observers_.erase(std::remove(observers_.begin(), observers_.end(), player), observers_.end());
	const bool game_ended = players_.empty() || (host && !started_);
	const player_map::iterator user = player_info_->find(player);
	if (user == player_info_->end()) {
		missing_user(player, __func__);
		return game_ended;
	}
	LOG_GAME << network::ip_address(user->first) << "\t" << user->second.name()
		<< ((game_ended && !(observer && destruct))
			? (started_ ? "\tended" : "\taborted") : "\thas left")
		<< " game:\t\"" << name_ << "\" (" << id_ << ")"
		<< (game_ended && started_ && !(observer && destruct) ? " at turn: "
			+ lexical_cast_default<std::string,size_t>(current_turn())
			+ " with reason: '" + termination_reason() + "'" : "")
		<< (observer ? " as an observer" : "")
		<< (disconnect ? " and disconnected" : "")
		<< ". (socket: " << user->first << ")\n";
	if (game_ended && started_ && !(observer && destruct)) {
		send_server_message_to_all(user->second.name() + " ended the game.", player);
	}
	if (game_ended || destruct) return game_ended;

	// Don't mark_available() since the player got already removed from the
	// games_and_users_list_.
	if (!disconnect) {
		user->second.mark_available();
	}
	if (observer) {
		send_observerquit(user);
	} else {
		send_and_record_server_message(user->second.name()
				+ (disconnect ? " has disconnected." : " has left the game."), player);
	}
	// If the player was host choose a new one.
	if (host) {
		owner_ = players_.front();
		notify_new_host();
	}

	bool ai_transfer = false;
	// Look for all sides the player controlled and drop them.
	// (Give them to the host.)
	for (side_vector::iterator side = sides_.begin(); side != sides_.end(); ++side)	{
		size_t side_num = side - sides_.begin();
		if (*side != player) continue;
		if (side_controllers_[side_num] == "ai") ai_transfer = true;

		player_map::iterator o = player_info_->find(owner_);
		change_controller(side_num, owner_, username(o));
		// Check whether the host is actually a player and make him one if not.
		if (!is_player(owner_)) {
			DBG_GAME << "making the owner a player...\n";
			o->second.set_status(player::PLAYING);
			observers_.erase(std::remove(observers_.begin(), observers_.end(), owner_), observers_.end());
			players_.push_back(owner_);
			send_observerquit(o);
		}

		//send the host a notification of removal of this side
		const std::string side_drop = lexical_cast_default<std::string, size_t>(side_num + 1);
		simple_wml::document drop;
		drop.root().set_attr("side_drop", side_drop.c_str());
		drop.root().set_attr("controller", side_controllers_[side_num].c_str());

		DBG_GAME << "*** sending side drop: \n" << drop.output() << std::endl;

		wesnothd::send_to_one(drop, owner_);
	}
	if (ai_transfer) send_and_record_server_message("AI sides transferred to host.");

	DBG_GAME << debug_player_info();

	send_user_list(player);
	return false;
}

void game::send_user_list(const network::connection exclude) const {
	//if the game hasn't started yet, then send all players a list
	//of the users in the game
	if (started_ || description_ == NULL) return;
	/** @todo Should be renamed to userlist. */
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

void game::load_next_scenario(const player_map::const_iterator user) {
	send_server_message_to_all(user->second.name() + " advances to the next scenario", user->first);
	simple_wml::document cfg_scenario;
	simple_wml::node & next_scen = cfg_scenario.root().add_child("next_scenario");
	level_.root().copy_into(next_scen);

	const simple_wml::node::child_list & sides = next_scen.children("side");

	DBG_GAME << "****\n loading next scenario for a client. sides info = " << std::endl; 
	DBG_GAME << debug_sides_info() << std::endl;
	DBG_GAME << "****" << std::endl;

	for(simple_wml::node::child_list::const_iterator s = sides.begin(); s != sides.end(); ++s) {
		if ((**s)["controller"] != "null") {
			int side_num = (**s)["side"].to_int() - 1;
			
			if (sides_[side_num] == 0) {
				sides_[side_num] = owner_;
				std::stringstream msg;
				msg << "Side "  << side_num + 1 << " had no controller while a client was loading next scenario! The host was assigned control.";
				LOG_GAME << msg.str() << " (game id: " << id_ << ")\n";
				send_and_record_server_message(msg.str());
			} else if (sides_[side_num] == user->first) {
				if (side_controllers_[side_num] == "human") {
					(*s)->set_attr("controller", "human");
				} else if (side_controllers_[side_num] == "ai") {
					(*s)->set_attr("controller", "ai");
				} else {
					std::stringstream msg;
					msg << "Side " << side_num + 1 << " had unexpected side_controller = " << side_controllers_[side_num] << " on server side.";
					LOG_GAME << msg.str() << " (game id: " << id_ << ")\n";
					send_and_record_server_message(msg.str());
				}
			} else {
				if (side_controllers_[side_num] == "human") {
					(*s)->set_attr("controller", "network");
				} else if (side_controllers_[side_num] == "ai") {
					(*s)->set_attr("controller", "network_ai");
				} else {
					std::stringstream msg;
					msg << "Side " << side_num + 1 << " had unexpected side_controller = " << side_controllers_[side_num] << " on server side.";
					LOG_GAME << msg.str() << " (game id: " << id_ << ")\n";
					send_and_record_server_message(msg.str());
				}
			}
		}
	}

	if (!wesnothd::send_to_one(cfg_scenario, user->first)) return;
	// Send the player the history of the game to-date.
	send_history(user->first);
	// Send observer join of all the observers in the game to the user.
	send_observerjoins(user->first);
	user->first.set_updated_level(true); //the player now has the updated level and needs to recieve game events.

}

void game::send_data(simple_wml::document& data,
						  const network::connection exclude,
						  std::string packet_type) const
{
	wesnothd::send_to_many(data, all_game_users(true), exclude, packet_type); //the flag (true) in all_game_users means exclude players lingering on last level
}

void game::send_data_team(simple_wml::document& data,
                          const simple_wml::string_span& team,
                          const network::connection exclude,
						  std::string packet_type) const
{
	DBG_GAME << __func__ << "...\n"; // TODO: This filter thing needs to be replaced with, get only the players which are not lingering on the last level.
	const std::vector<network::connection> not_lingering_players = filter(players_);
	wesnothd::send_to_many(data, not_lingering_players,
		boost::bind(&game::is_on_team, this, boost::ref(team), _1),
		exclude, packet_type);
}


bool game::is_on_team(const simple_wml::string_span& team, const network::connection player) const {
	const simple_wml::node::child_list& side_list = level_.root().children("side");
	for (side_vector::const_iterator side = sides_.begin(); side != sides_.end(); ++side) {
		if (*side != player) continue;
		for (simple_wml::node::child_list::const_iterator i = side_list.begin();
				i != side_list.end(); ++i) {
			if ((**i)["side"].to_int() != side - sides_.begin() + 1) continue;
			if ((**i)["team_name"] != team) continue;
			// Don't consider ai sides on a team.
			if ((**i)["controller"] == "ai") continue;
			if (side_controllers_[side - sides_.begin()] == "ai") continue;
			DBG_GAME << "side: " << (**i)["side"].to_int() << " with team_name: " << (**i)["team_name"]
			<< " belongs to player: " << player << std::endl;
			return true;
		}
	}

	return false;
}

std::string game::has_same_ip(const network::connection& user, bool observer) const {
	const user_vector users = observer ? players_ : all_game_users();
	const std::string ip = network::ip_address(user);
	std::string clones;
	for (user_vector::const_iterator i = users.begin(); i != users.end(); ++i) {
		if (ip == network::ip_address(*i) && user != *i) {
			const player_map::const_iterator pl = player_info_->find(*i);
			if (pl != player_info_->end()) {
				clones += (clones.empty() ? "" : ", ") + pl->second.name();
			}
		}
	}
	return clones;
}

void game::send_observerjoins(const network::connection sock) const {
	for (user_vector::const_iterator ob = observers_.begin(); ob != observers_.end(); ++ob) {
		if (*ob == sock) continue;
		const player_map::const_iterator obs = player_info_->find(*ob);
		if (obs == player_info_->end()) {
			missing_user(*ob, __func__);
			continue;
		}

		simple_wml::document cfg;
		cfg.root().add_child("observer").set_attr_dup("name", obs->second.name().c_str());
		if (sock == 0) {
			// Send to everyone except the observer in question.
			send_data(cfg, *ob);
		} else {
			// Send to the (new) user.
			wesnothd::send_to_one(cfg, sock);
		}
	}
}

void game::send_observerquit(const player_map::const_iterator observer) const {
	if (observer == player_info_->end()) {
		return;
	}
	simple_wml::document observer_quit;

	//don't need to dup the attribute because this document is
	//short-lived.
	observer_quit.root().add_child("observer_quit").set_attr("name", observer->second.name().c_str());
	send_data(observer_quit, observer->first);
}

void game::send_history(const network::connection sock) const
{
	if(history_.empty()) {
		return;
	}

	//we make a new document based on converting to plain text and
	//concatenating the buffers.
	//TODO: Work out how to concentate buffers without decompressing.
	std::string buf;
	for(std::vector<simple_wml::document*>::iterator i = history_.begin();
	    i != history_.end(); ++i) {
		buf += (*i)->output();
		delete *i;
	}

	try {
		simple_wml::document* doc = new simple_wml::document(buf.c_str(), simple_wml::INIT_STATIC);
		const simple_wml::string_span& data = doc->output_compressed();
		doc->compress();
		network::send_raw_data(data.begin(), data.size(), sock,"game_history");
		history_.clear();
		history_.push_back(doc);
	} catch (simple_wml::error& e) {
		WRN_CONFIG << __func__ << ": simple_wml error: " << e.message << std::endl;
	}

}

static bool is_invalid_filename_char(char c) {
	return !(isalnum(c) || (c == '_') || (c == '-') || (c == '.')
			|| (c == '(') || (c == ')') || (c == '#') || (c == ',')
			|| (c == '!') || (c == '?') || (c == '^') || (c == '+')
			|| (c == '*') || (c == ':') || (c == '=') || (c == '@')
			|| (c == '%') || (c == '\''));
}

void game::save_replay() {
	if (!save_replays_ || !started_ || history_.empty()) return;

	std::string replay_commands;
	for(std::vector<simple_wml::document*>::iterator i = history_.begin();
			i != history_.end(); ++i) {
		const simple_wml::node::child_list& turn_list = (*i)->root().children("turn");
		for (simple_wml::node::child_list::const_iterator turn = turn_list.begin();
				turn != turn_list.end(); ++turn) {
			replay_commands += simple_wml::node_to_string(**turn);
		}
		delete *i;
	}
	history_.clear();

	std::stringstream name;
	name << level_["name"] << " Turn " << current_turn();

	std::stringstream replay_data;
	try {
		replay_data << "campaign_type=\"multiplayer\"\n"
		<< "difficulty=\"NORMAL\"\n"
		<< "label=\"" << name.str() << "\"\n"
		<< "mp_game_title=\"" << name_ << "\"\n"
		<< "random_seed=\"" << level_["random_seed"] << "\"\n"
		<< "version=\"" << level_["version"] << "\"\n"
		<< "[replay]\n"
		<< "\t[command]\n\t\t[start]\n\t\t[/start]\n\t[/command]\n" //this is required by gfgtdf's sync mechanism, in PR 121
		<< replay_commands 
		<< "[/replay]\n"
		<< "[replay_start]\n" << level_.output() << "[/replay_start]\n";

		name << " (" << id_ << ").bz2";

		std::string replay_data_str = replay_data.str();
		simple_wml::document replay(replay_data_str.c_str(), simple_wml::INIT_STATIC);

		std::string filename(name.str());
		std::replace(filename.begin(), filename.end(), ' ', '_');
		filename.erase(std::remove_if(filename.begin(), filename.end(), is_invalid_filename_char), filename.end());
		DBG_GAME << "saving replay: " << filename << std::endl;
		filesystem::scoped_ostream os(filesystem::ostream_file(replay_save_path_ + filename));
		(*os) << replay.output_compressed(true);

		if (!os->good()) {
			ERR_GAME << "Could not save replay! (" << filename << ")\n";
		}
	} catch (simple_wml::error& e) {
		WRN_CONFIG << __func__ << ": simple_wml error: " << e.message << std::endl;
	}
}

void game::record_data(simple_wml::document* data) {
	data->compress();
	history_.push_back(data);
}

void game::clear_history() {
	if (history_.empty()) return;
	for(std::vector<simple_wml::document*>::iterator i = history_.begin(); i != history_.end(); ++i) {
		delete *i;
	}
	history_.clear();
}

void game::set_description(simple_wml::node* desc) {
	description_ = desc;
	if(!password_.empty()) {
		description_->set_attr("password", "yes");
	}
}

void game::set_termination_reason(const std::string& reason) {
/*	if (reason == "out of sync") {
		simple_wml::string_span era;
		if (level_.child("era")) {
			era = level_.child("era")->attr("id");
		}
		termination_ = "out of sync - " + era.to_string();
	}*/
	if (termination_.empty()) { termination_ = reason; }
}

void game::allow_global(const simple_wml::document &data) {
	const simple_wml::node *cfg = data.root().child("wait_global");
	int side = (*cfg)["side"].to_int();
	if ((side < 0) || (side > nsides_)) side = 0;
	global_wait_side_ = side;
}

const user_vector game::all_game_users(bool exclude_lingering) const {
	user_vector res;

	if (exclude_lingering) {
		for (user_vector::const_iterator x = players_.begin(); x != players_.end(); ++x) {
			if (x->has_updated_level()) {
				res.insert(res.end(),*x);
			}
		}
		for (user_vector::const_iterator x = observers_.begin(); x != observers_.end(); ++x) {
			if (x->has_updated_level()) {
				res.insert(res.end(),*x);
			}
		}
	} else {
		res.insert(res.end(), players_.begin(), players_.end());
		res.insert(res.end(), observers_.begin(), observers_.end());
	}

	return res;
}

std::string game::debug_player_info() const {
	std::stringstream result;
	result << "game id: " << id_ << "\n";
//	result << "players_.size: " << players_.size() << "\n";
	for (user_vector::const_iterator p = players_.begin(); p != players_.end(); ++p){
		const player_map::const_iterator user = player_info_->find(*p);
		if (user != player_info_->end()){
			result << "player: " << user->second.name().c_str() << "\n";
		}
		else{
			result << "player: '" << *p << "' not found\n";
		}
	}
//	result << "observers_.size: " << observers_.size() << "\n";
	for (user_vector::const_iterator o = observers_.begin(); o != observers_.end(); ++o){
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

std::string game::debug_sides_info() const {
	std::stringstream result;
	result << "game id: " << id_ << "\n";
	const simple_wml::node::child_list & sides = level_.root().children("side");

	result << "\t\t level, server\n";
	for(simple_wml::node::child_list::const_iterator s = sides.begin(); s != sides.end(); ++s) {
		result << "side " << (**s)["side"].to_int() << " :\t" << (**s)["controller"].to_string() 
			<< "\t, " << side_controllers_[(**s)["side"].to_int() - 1]
			<< "\t( " << sides_[(**s)["side"].to_int()-1] << ",\t" 
			<< (**s)["current_player"].to_string() << " )\n";
	}

	return result.str();
}

player_map::iterator game::find_user(const simple_wml::string_span& name)
{
	player_map::iterator pl;
	for (pl = player_info_->begin(); pl != player_info_->end(); ++pl) {
		if (name == pl->second.name().c_str()) {
			break;
		}
	}
	return pl;
}

void game::send_and_record_server_message(const char* message, const network::connection exclude)
{
	simple_wml::document* doc = new simple_wml::document;
	send_server_message(message, 0, doc);
	send_data(*doc, exclude, "message");
	if (started_) record_data(doc);
	else delete doc;
}

void game::send_server_message_to_all(const char* message, network::connection exclude) const
{
	simple_wml::document doc;
	send_server_message(message, 0, &doc);
	send_data(doc, exclude, "message");
}

void game::send_server_message(const char* message, network::connection sock, simple_wml::document* docptr) const
{
	simple_wml::document docbuf;
	if(docptr == NULL) {
		docptr = &docbuf;
	}

	simple_wml::document& doc = *docptr;
	if (started_) {
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
		wesnothd::send_to_one(doc, sock, "message");
	}
}
} // namespace wesnothd
