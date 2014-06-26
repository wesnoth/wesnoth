/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
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

#ifndef __func__
 #ifdef __FUNCTION__
  #define __func__ __FUNCTION__
 #endif
#endif


#define ERR_GAME LOG_STREAM(err, mp_server)
#define WRN_GAME LOG_STREAM(warn, mp_server)
#define LOG_GAME LOG_STREAM(info, mp_server)
#define DBG_GAME LOG_STREAM(debug, mp_server)
#define WRN_CONFIG LOG_STREAM(warn, config)

namespace chat_message {

const size_t max_message_length = 256;
static void truncate_message(const simple_wml::string_span& str, simple_wml::node& message) {
	// testing for msg.size() is not sufficient but we're not getting false negatives
	// and it's cheaper than always converting to wstring.
	if(str.size() > static_cast<int>(chat_message::max_message_length)) {
		std::string tmp(str.begin(), str.end());
		// The string can contain utf-8 characters so truncate as wide_string otherwise
		// a corrupted utf-8 string can be returned.
		utils::truncate_as_wstring(tmp, max_message_length);
		message.set_attr_dup("message", tmp.c_str());
	}
}

} // end chat_message namespace

namespace wesnothd {
int game::id_num = 1;

game::game(player_map& players, const network::connection host,
		const std::string name, bool save_replays,
		const std::string replay_save_path) :
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
	replay_save_path_(replay_save_path)
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
	// Hack to handle the pseudo games lobby_ and not_logged_in_.
	if (owner_ == 0) return;

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

	update_side_data();

	nsides_ = 0;
	// Set all side controllers to 'human' so that observers will understand
	// that they can't take control of any sides if they happen to have the
	// same name as one of the descriptions.
	const simple_wml::node::child_list& sides = level_.root().children("side");
	for(simple_wml::node::child_list::const_iterator s = sides.begin(); s != sides.end(); ++s) {
		nsides_++;
		if ((**s)["controller"] != "null") {
			int side_num = (**s)["side"].to_int() - 1;
			if (sides_[side_num] == 0) {
				std::stringstream msg;
				msg << "Side "  << side_num + 1 << " has no controller but should! The host needs to assign control for the game to proceed past that side's turn.";
				LOG_GAME << msg.str() << " (game id: " << id_ << ")\n";
				send_and_record_server_message(msg.str().c_str());
			}
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
	clear_history();
	if (advance) {
		// When the host advances tell everyone that the next scenario data is
		// available.
		static simple_wml::document notify_next_scenario("[notify_next_scenario]\n[/notify_next_scenario]\n", simple_wml::INIT_COMPRESSED);
		send_data(notify_next_scenario, starter->first);
	}
	// Send [observer] tags for all observers that are already in the game.
	send_observerjoins();
}

bool game::send_taken_side(simple_wml::document& cfg, const simple_wml::node::child_list::const_iterator side) const
{
	const size_t side_num = (**side)["side"].to_int();
	if (side_num < 1 || side_num > gamemap::MAX_PLAYERS) return false;
	if (sides_[side_num - 1] != 0) return false;
	// We expect that the host will really use our proposed side number. (He could do different...)
	cfg.root().set_attr_dup("side", (**side)["side"]);

	// Tell the host which side the new player should take.
	return send_to_one(cfg, owner_);
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
	if (!lg::debug.dont_log(lg::mp_server)) {
		for (simple_wml::node::child_list::const_iterator side = level_sides.begin();
				side != level_sides.end(); ++side)
			DBG_GAME << "[side]\n" << simple_wml::node_to_string(**side) << "[/side]\n";
	}
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
			int side_num = (**side)["side"].to_int() - 1;
			if (side_num < 0 || side_num >= gamemap::MAX_PLAYERS
					|| sides_[side_num] != 0) continue;

			if ((**side)["controller"] == "network") {
				if ((**side)["current_player"] == info->second.name().c_str()) {
					side_controllers_[side_num] = "human";
					sides_[side_num] = *user;
					side_found = true;
				}
			} else if (*user == owner_
			&& ((**side)["controller"] == "ai" || (**side)["controller"] == "human")) {
				side_controllers_[side_num] = (**side)["controller"].to_string();
				sides_[side_num] = owner_;
				side_found = true;
			} else {
				// "null", "reserved"
				side_controllers_[side_num] = (**side)["controller"].to_string();
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
		send_server_message(msg.str().c_str(), sock);
		return;
	}

	if (side_num > level_.root().children("side").size()) {
		send_server_message("Invalid side number.", sock);
		return;
	}

	const simple_wml::string_span& newplayer_name = cfg["player"];
	const network::connection old_player = sides_[side_num - 1];
	const player_map::const_iterator oldplayer = player_info_->find(old_player);
	if (oldplayer == player_info_->end()) {
		WRN_GAME << "Could not find old player in player_info_. (socket: "
			<< old_player << ")\n";
	}
	const std::string old_player_name(oldplayer != player_info_->end() ? oldplayer->second.name() : "(unknown)");

	// A player (un)droids his side.
	if (newplayer_name.empty()) {
		if (sock != old_player) {
			if (cfg["controller"].empty()) {
				send_server_message("No player name or controller type given.", sock);
			} else {
				send_server_message("You can only (un)droid your own sides!", sock);
			}
			return;
		} else if (cfg["controller"] != "human_ai" && cfg["controller"] != "human") {
			std::stringstream msg;
			msg << "Wrong controller type received: '" << cfg["controller"] << "'";
			DBG_GAME << msg.str() << "\n";
			send_server_message(msg.str().c_str(), sock);
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
		send_server_message(msg.str().c_str(), sock);
		return;
	}
	//find the player that is passed control
	player_map::const_iterator newplayer;
	for (newplayer = player_info_->begin(); newplayer != player_info_->end(); newplayer++) {
		if (newplayer_name == newplayer->second.name().c_str()) {
			break;
		}
	}
	// Is he in this game?
	if (newplayer == player_info_->end() || !is_member(newplayer->first)) {
		send_server_message((newplayer_name.to_string() + " is not in this game").c_str(), sock);
		return;
	}

	if (newplayer->first == old_player) {
		std::stringstream msg;
		msg << "That's already " << newplayer_name << "'s side, silly.";
		send_server_message(msg.str().c_str(), sock);
		return;
	}
	sides_[side_num - 1] = 0;
	// If the old player lost his last side, make him an observer.
	if (std::find(sides_.begin(), sides_.end(), old_player) == sides_.end()
	&& is_player(old_player)) {
		observers_.push_back(old_player);
		players_.erase(std::remove(players_.begin(), players_.end(), old_player), players_.end());
		// Tell others that the player becomes an observer.
		send_and_record_server_message((old_player_name + " becomes an observer.").c_str());
		// Update the client side observer list for everyone except old player.
		simple_wml::document observer_join;
		observer_join.root().add_child("observer").set_attr_dup("name", old_player_name.c_str());
		send_data(observer_join, old_player);
	}
	change_controller(side_num - 1, newplayer->first, newplayer->second.name(), false);

	// If we gave the new side to an observer add him to players_.
	if (is_observer(newplayer->first)) {
		players_.push_back(newplayer->first);
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

	const std::string& side = lexical_cast<std::string, size_t>(side_num + 1);
	sides_[side_num] = sock;

	if (player_left && side_controllers_[side_num] == "ai") {
		// Automatic AI side transfer.
	} else if (controller.empty()) {
		send_and_record_server_message((player_name
				+ " takes control of side " + side + ".").c_str());
		side_controllers_[side_num] = "human";
	} else {
		send_and_record_server_message((player_name + (controller == "human_ai" ? " " : " un")
				+ "droids side " + side + ".").c_str());
		side_controllers_[side_num] = (controller == "human_ai" ? "ai" : "human");
	}

	simple_wml::document response;
	simple_wml::node& change = response.root().add_child("change_controller");

	change.set_attr("side", side.c_str());
	change.set_attr("player", player_name.c_str());

	// Tell everyone but the new player that this side's controller changed.
	change.set_attr("controller", (side_controllers_[side_num] == "ai" ? "network_ai" : "network"));
	send_data(response, sock);

	// Tell the new player that he controls this side now.
	// Just don't send it when the player left the game. (The host gets the
	// side_drop already.)
	if (!player_left) {
		change.set_attr("controller", (side_controllers_[side_num] == "ai" ? "human_ai" : "human"));
		send_to_one(response, sock);
	}

	// Update the level so observers who join get the new name. (The host handles level changes before game start.)
	if (started_) {
		const simple_wml::node::child_list& side_list = level_.root().children("side");
		assert(side_num < side_list.size());
		side_list[side_num]->set_attr_dup("current_player", player_name.c_str());
		// Also update controller type (so savegames of observers have proper controllers)
		side_list[side_num]->set_attr_dup("controller", side_controllers_[side_num].c_str());
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
	std::string message = owner_name + " has been chosen as the new host.";
	if (!send_to_one(cfg, owner_)) {
		message += " But an internal error occured. You probably have to abandon this game.";
	}
	send_and_record_server_message(message.c_str());
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
			if (sides_[i] == 0) ++available_slots;
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
			muted_nicks += (player_info_->find(*muted_obs) != player_info_->end()
					? player_info_->find(*muted_obs)->second.name() : "");
		}

		send_server_message(("Muted observers: " + muted_nicks).c_str(), muter->first);
		return;
	}
	const player_map::const_iterator user = find_user(name);
	/**
	 * @todo FIXME: Maybe rather save muted nicks as a vector of strings and
	 * also allow muting of usernames not in the game.
	 */
	if (user == player_info_->end() || !is_observer(user->first)) {
		send_server_message("Observer not found.", muter->first);
		return;
	}

	// Prevent muting ourselves.
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
		<< muter->second.name() << " muted: " << name << " ("
		<< network::ip_address(user->first) << ")\tin game:\t\""
		<< name_ << "\" (" << id_ << ")\n";
	send_and_record_server_message((user->second.name() + " has been muted.").c_str());
}

void game::send_leave_game(network::connection user) const
{
	static simple_wml::document leave_game("[leave_game]\n[/leave_game]\n", simple_wml::INIT_COMPRESSED);
	send_to_one(leave_game, user);
}

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
	} else if (user->first == kicker->first) {
		send_server_message("Don't kick yourself, silly.", kicker->first);
		return 0;
	} else if (user->second.is_moderator()) {
		send_server_message("You're not allowed to kick a moderator.", kicker->first);
		return 0;
	}
	LOG_GAME << network::ip_address(kicker->first) << "\t"
		<< kicker->second.name() << "\tkicked: " << name << " ("
		<< network::ip_address(user->first) << ")\tfrom game:\t\""
		<< name_ << "\" (" << id_ << ")\n";
	send_and_record_server_message((name.to_string() + " has been kicked.").c_str());

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
	const simple_wml::string_span& name = ban["username"];
	const player_map::const_iterator user = find_user(name);
	if (user == player_info_->end()) {
		send_server_message("User not found", banner->first);
		return 0;
	} else if (user->first == banner->first) {
		send_server_message("Don't ban yourself, silly.", banner->first);
		return 0;
	} else if (player_is_banned(user->first)) {
		send_server_message(("'" + name.to_string() + "' is already banned.").c_str(), banner->first);
		return 0;
	} else if (user->second.is_moderator()) {
		send_server_message("You're not allowed to ban a moderator.", banner->first);
		return 0;
	}
	LOG_GAME << network::ip_address(banner->first) << "\t"
		<< banner->second.name() << "\tbanned: " << name << " ("
		<< network::ip_address(user->first) << ")\tfrom game:\t\""
		<< name_ << "\" (" << id_ << ")\n";
	bans_.push_back(network::ip_address(user->first));
	send_and_record_server_message((name.to_string() + " has been banned.").c_str());
	if (is_member(user->first)) {
		//tell the user to leave the game.
		send_leave_game(user->first);
		remove_player(user->first);
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
	chat_message::truncate_message(msg, *message);

	// Only log in the lobby_.
	std::string game_prefix;
	if (owner_ != 0) {
		game_prefix = "game ";
	} else if (msg.size() >= 3 && simple_wml::string_span(msg.begin(), 4) == "/me ") {
		LOG_GAME << network::ip_address(user->first) << "\t<"
			<< user->second.name() << simple_wml::string_span(msg.begin() + 3, msg.size() - 3) << ">\n";
        } else {
		LOG_GAME << network::ip_address(user->first) << "\t<"
			<< user->second.name() << "> " << msg << "\n";
	}

	send_data(data, user->first, game_prefix + "message");
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
				<< (player_info_->find(current_player()) != player_info_->end()
					? player_info_->find(current_player())->second.name()
					: "(unfound)")
				<< " (" << end_turn_ + 1 << "/" << nsides_ << ").";
			LOG_GAME << msg.str() << " (socket: " << current_player()
				<< ") (game id: " << id_ << ")\n";
			send_and_record_server_message(msg.str().c_str());

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
		send_data(data, user->first, "game replay");
		return turn_ended;
	}
	for (command = commands.begin(); command != commands.end(); ++command) {
		simple_wml::node* const speak = (**command).child("speak");
		if (speak == NULL) {
			simple_wml::document* mdata = new simple_wml::document;
			simple_wml::node& turn = mdata->root().add_child("turn");
			(**command).copy_into(turn.add_child("command"));
			send_data(*mdata, user->first,"game replay");
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
			send_data(*message, user->first, "game message");
			record_data(message.release());
		} else if (team_name == game_config::observer_team_name) {
			send_data_observers(*message, user->first, "game message");
			record_data(message.release());
		} else {
			send_data_team(*message, team_name, user->first, "game message");
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

//@todo differentiate between "observers not allowed" and "player already in the game" errors.
//      maybe return a string with an error message.
bool game::add_player(const network::connection player, bool observer) {
	if(is_member(player)) {
		ERR_GAME << "ERROR: Player is already in this game. (socket: "
			<< player << ")\n";
		return false;
	}
	// Hack to handle the pseudo games lobby_ and not_logged_in_.
	if (owner_ == 0) {
		observers_.push_back(player);
		return true;
	}
	const player_map::iterator user = player_info_->find(player);
	if (user == player_info_->end()) {
		ERR_GAME << "ERROR: Could not find user in player_info_. (socket: "
			<< owner_ << ")\n";
		return false;
	}
	DBG_GAME << debug_player_info();
	bool became_observer = false;
	if (!started_ && !observer && take_side(user)) {
		DBG_GAME << "adding player...\n";
		players_.push_back(player);
		send_and_record_server_message((user->second.name() + " has joined the game.").c_str(), player);
	} else if (!allow_observers() && !user->second.is_moderator()) {
		return false;
	} else {
		if (!observer) became_observer = true;
		DBG_GAME << "adding observer...\n";
		observers_.push_back(player);
		if (!allow_observers()) send_and_record_server_message((user->second.name() + " is now observing the game.").c_str(), player);

		simple_wml::document observer_join;
		observer_join.root().add_child("observer").set_attr_dup("name", user->second.name().c_str());

		// Send observer join to everyone except the new observer.
		send_data(observer_join, player);
	}
	LOG_GAME << network::ip_address(player) << "\t" << user->second.name()
		<< "\tjoined game:\t\"" << name_ << "\" (" << id_ << ")"
		<< (observer || became_observer ? " as an observer" : "")
		<< ". (socket: " << player << ")\n";
	user->second.mark_available(id_, name_);
	DBG_GAME << debug_player_info();
	// Send the user the game data.
	if (!send_to_one(level_, player)) return false;

	if(started_) {
		//tell this player that the game has started
		static simple_wml::document start_game_doc("[start_game]\n[/start_game]\n", simple_wml::INIT_COMPRESSED);
		if (!send_to_one(start_game_doc, player)) return false;
		// Send observer join of all the observers in the game to the new player
		// only once the game started. The client forgets about it anyway
		// otherwise.
		send_observerjoins(player);
		// Send the player the history of the game to-date.
		send_history(player);
	} else {
		send_user_list();
	}

	const std::string clones = has_same_ip(player, observer || became_observer);
	if (!clones.empty()) {
		send_and_record_server_message((user->second.name() + " has the same IP as: " + clones).c_str());
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
	const bool game_ended = players_.empty() || (host && !started_);
	const player_map::iterator user = player_info_->find(player);
	if (user == player_info_->end()) {
		ERR_GAME << "ERROR: Could not find user in player_info_. (socket: "
			<< player << ")\n";
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
		send_server_message_to_all((user->second.name() + " ended the game.").c_str(), player);
	}
	if (game_ended || destruct) return game_ended;

	// Don't mark_available() since the player got already removed from the
	// games_and_users_list_.
	if (!disconnect) user->second.mark_available();
	if (observer) {
		send_observerquit(user);
	} else {
		send_and_record_server_message((user->second.name()
				+ (disconnect ? " has disconnected." : " has left the game.")).c_str(), player);
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

		player_map::const_iterator o = player_info_->find(owner_);
		const std::string owner_name = (o != player_info_->end() ? o->second.name() : "(unknown)");
		change_controller(side_num, owner_, owner_name);
		// Check whether the host is actually a player and make him one if not.
		if (!is_player(owner_)) {
			DBG_GAME << "making the owner a player...\n";
			observers_.erase(std::remove(observers_.begin(), observers_.end(), owner_), observers_.end());
			players_.push_back(owner_);
			send_observerquit(o);
		}

		//send the host a notification of removal of this side
		const std::string side_drop = lexical_cast<std::string, size_t>(side_num + 1);
		simple_wml::document drop;
		drop.root().set_attr("side_drop", side_drop.c_str());
		drop.root().set_attr("controller", side_controllers_[side_num].c_str());

		send_to_one(drop, owner_);
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

void game::load_next_scenario(const player_map::const_iterator user) const {
	send_server_message_to_all((user->second.name() + " advances to the next scenario").c_str(), user->first);
	simple_wml::document cfg_scenario;
	level_.root().copy_into(cfg_scenario.root().add_child("next_scenario"));
	if (!send_to_one(cfg_scenario, user->first)) return;
	// Send the player the history of the game to-date.
	send_history(user->first);
	// Send observer join of all the observers in the game to the user.
	send_observerjoins(user->first);
}

void game::send_data(simple_wml::document& data, const network::connection exclude, std::string packet_type) const
{
	if (packet_type.empty())
		packet_type = data.root().first_child().to_string();
	try {
		simple_wml::string_span s = data.output_compressed();
		const user_vector& users = all_game_users();
		for(user_vector::const_iterator i = users.begin(); i != users.end(); ++i) {
			if (*i != exclude) {
				network::send_raw_data(s.begin(), s.size(), *i, packet_type);
			}
		}
	} catch (simple_wml::error& e) {
		WRN_CONFIG << __func__ << ": simple_wml error: " << e.message << std::endl;
	}
}

bool game::send_to_one(simple_wml::document& data, const network::connection sock, std::string packet_type) const
{
	if (packet_type.empty())
		packet_type = data.root().first_child().to_string();
	try {
		simple_wml::string_span s = data.output_compressed();
		network::send_raw_data(s.begin(), s.size(), sock, packet_type);
	} catch (simple_wml::error& e) {
		WRN_CONFIG << __func__ << ": simple_wml error: " << e.message << std::endl;
		return false;
	}
	return true;
}

void game::send_data_team(simple_wml::document& data,
                          const simple_wml::string_span& team,
                          const network::connection exclude,
						  std::string packet_type) const
{
	DBG_GAME << __func__ << "...\n";
	if (packet_type.empty())
		packet_type = data.root().first_child().to_string();
	try {
		simple_wml::string_span s = data.output_compressed();
		for(user_vector::const_iterator i = players_.begin(); i != players_.end(); ++i) {
			if(*i != exclude && is_on_team(team,*i)) {
				network::send_raw_data(s.begin(), s.size(), *i, packet_type);
			}
		}
	} catch (simple_wml::error& e) {
		WRN_CONFIG << __func__ << ": simple_wml error: " << e.message << std::endl;
	}
}

void game::send_data_observers(simple_wml::document& data, const network::connection exclude, std::string packet_type) const {
	if (packet_type.empty())
		packet_type = data.root().first_child().to_string();
	try {
		simple_wml::string_span s = data.output_compressed();
		for(user_vector::const_iterator i = observers_.begin(); i != observers_.end(); ++i) {
			if (*i != exclude) {
				network::send_raw_data(s.begin(), s.size(), *i, packet_type);
			}
		}
	} catch (simple_wml::error& e) {
		WRN_CONFIG << __func__ << ": simple_wml error: " << e.message << std::endl;
	}
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
			send_to_one(cfg, sock);
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
		<< "[replay]\n" << replay_commands << "[/replay]\n"
		<< "[replay_start]\n" << level_.output() << "[/replay_start]\n";

		name << " (" << id_ << ").gz";

		std::string replay_data_str = replay_data.str();
		simple_wml::document replay(replay_data_str.c_str(), simple_wml::INIT_STATIC);

		std::string filename(name.str());
		std::replace(filename.begin(), filename.end(), ' ', '_');
		filename.erase(std::remove_if(filename.begin(), filename.end(), is_invalid_filename_char), filename.end());
		DBG_GAME << "saving replay: " << filename << std::endl;
		scoped_ostream os(ostream_file(replay_save_path_ + filename));
		(*os) << replay.output_compressed();

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
		send_to_one(doc, sock, "message");
	}
}
}
