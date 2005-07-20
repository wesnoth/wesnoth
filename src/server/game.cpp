/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "../global.hpp"

#include "game.hpp"
#include "../util.hpp"
#include "../wassert.hpp"

#include <iostream>

int game::id_num = 1;

game::game(const player_map& info) : player_info_(&info), id_(id_num++), sides_taken_(9), side_controllers_(9), started_(false), description_(NULL), end_turn_(0), allow_observers_(true)
{}

bool game::is_owner(network::connection player) const
{
	wassert(!players_.empty());
	return player == players_.front();
}

bool game::is_member(network::connection player) const
{
	return std::find(players_.begin(),players_.end(),player) != players_.end();
}

bool game::is_needed(network::connection player) const
{
	wassert(!players_.empty());
	return player == players_.front();
}

bool game::is_observer(network::connection player) const
{
	if(is_member(player) == false) {
		return false;
	}

	return is_needed(player) == false && sides_.count(player) == 0;
}

bool game::observers_can_label() const
{
	return false;
}

bool game::observers_can_chat() const
{
	return true;
}

bool game::filter_commands(network::connection player, config& cfg)
{
	if(is_observer(player)) {
		std::vector<int> marked;
		int index = 0;

		const config::child_list& children = cfg.get_children("command");
		for(config::child_list::const_iterator i = children.begin(); i != children.end(); ++i) {

			if(observers_can_label() && (*i)->child("label") != NULL && (*i)->all_children().size() == 1) {
				;
			} else if(observers_can_chat() && (*i)->child("speak") != NULL && (*i)->all_children().size() == 1) {
				;
			} else {
				std::cerr << "removing observer's illegal command\n";
				marked.push_back(index - marked.size());
			}

			++index;
		}

		for(std::vector<int>::const_iterator j = marked.begin(); j != marked.end(); ++j) {
			cfg.remove_child("command",*j);
		}

		return cfg.all_children().empty() == false;
	}

	return true;
}

namespace {
std::string describe_turns(int turn, const std::string& num_turns)
{
	char buf[50];
	sprintf(buf,"%d/",int(turn));

	if(num_turns == "-1") {
		return buf + std::string("-");
	} else {
		return buf + num_turns;
	}
}

}

void game::start_game()
{
	started_ = true;

	describe_slots();
	if(description()) {
		description()->values["turn"] = describe_turns(1,level()["turns"]);
	}

	allow_observers_ = level_["observer"] != "no";

	//send [observer] tags for all observers that have already joined.
	//we can do this by re-joining all players that don't have sides, and aren't
	//the first player (game creator)
	for(std::vector<network::connection>::const_iterator pl = players_.begin()+1; pl != players_.end(); ++pl) {
		if(sides_.count(*pl) == 0) {
			add_player(*pl);
		}
	}
}

bool game::take_side(network::connection player, const config& cfg)
{
	wassert(is_member(player));

	// verify that side is a side id
	const std::string& side = cfg["side"];
	size_t side_num;
	try {
		side_num = lexical_cast<size_t, std::string>(side);
		if(side_num < 1 || side_num > 9)
			return false;
	}
	catch(bad_lexical_cast& bad_lexical) {
		return false;
	}

	size_t side_index = static_cast<size_t>(side_num - 1);

	//if the side is already taken, see if we can give the player
	//another side instead
	if(side_controllers_[side_index] == "human" || (sides_taken_[side_index] && side_controllers_[side_index] == "network")) {
		const config::child_list& sides = level_.get_children("side");
		for(config::child_list::const_iterator i = sides.begin(); i != sides.end(); ++i) {
			if((**i)["controller"] == "network") {
				// don't allow players to take sides in games with invalid side numbers
				try {
					side_num = lexical_cast<size_t, std::string>((**i)["side"]);
					if(side_num < 1 || side_num > 9)
						return false;
				}
				catch(bad_lexical_cast& bad_lexical) {
					return false;
				}
				// check if the side is taken if not take it
				side_index = static_cast<size_t>(side_num - 1);
				if(!sides_taken_[side_index]) {
					side_controllers_[side_index] = "network";
					sides_.insert(std::pair<network::connection, size_t>(player, side_index));
					sides_taken_[side_index] = true;
					config new_cfg = cfg;
					new_cfg["side"] = (**i)["side"];
					network::queue_data(new_cfg, players_.front());
					return true;
				}
			}
		}
		// if we get here we couldn't find a side to take
		return false;
	}
	// else take the current side
	side_controllers_[side_index] = "network";
	sides_.insert(std::pair<network::connection, size_t>(player, side_index));
	sides_taken_[side_index] = true;
	network::queue_data(cfg, players_.front());
	return true;
}

void game::update_side_data()
{
	sides_taken_.clear();
	sides_taken_.resize(9);
	sides_.clear();

	const config::child_itors level_sides = level_.child_range("side");

	//for each player:
	// * Find the player name
	// * Find the side this player name corresponds to
	for(std::vector<network::connection>::const_iterator player = players_.begin();
			player != players_.end(); ++player) {

		player_map::const_iterator info = player_info_->find(*player);
		if (info == player_info_->end()) {
			std::cerr << "Error: unable to find player info for connection " << *player << "\n";
			continue;
		}

		size_t side_num;
		size_t side_index;
		config::child_iterator sd;
		for(sd = level_sides.first; sd != level_sides.second; ++sd) {
			try {
				side_num = lexical_cast<size_t, std::string>((**sd)["side"]);
				if(side_num < 1 || side_num > 9)
					continue;
			}
			catch(bad_lexical_cast& bad_lexical) {
				continue;
			}
			side_index = static_cast<size_t>(side_num - 1);

			if((**sd)["controller"] == "network") {
				side_controllers_[side_index] = "network";
				if((**sd)["user_description"] == info->second.name()) {
					sides_.insert(std::pair<network::connection, size_t>(*player, side_index));
					sides_taken_[side_index] = true;
				}
				else {
					sides_taken_[side_index] = false;
				}
			}
			else if((**sd)["controller"] == "ai") {
				side_controllers_[side_index] = "ai";
				sides_taken_[side_index] = true;
			}
			else if((**sd)["controller"] == "human") {
				sides_.insert(std::pair<network::connection,size_t>(players_.front(),side_index));
				sides_taken_[side_index] = true;
				side_controllers_[side_index] = "human";
			}
		}
	}
}

const std::string& game::transfer_side_control(const config& cfg)
{
	const std::string& player = cfg["player"];

	std::vector<network::connection>::const_iterator i;
	for(i = players_.begin(); i != players_.end(); ++i) {
		const player_map::const_iterator pl = player_info_->find(*i);
		if(pl != player_info_->end() && pl->second.name() == player) {
			break;
		}
	}

	if(i == players_.end()) {
		static const std::string notfound = "Player not found";
		return notfound;
	}
	const std::string& side = cfg["side"];
	static const std::string invalid = "Invalid side number";
	size_t side_num;
	try {
		side_num = lexical_cast<size_t, std::string>(side);
		if(side_num < 1 || side_num > 9)
			return invalid;
	}
	catch(bad_lexical_cast& bad_lexical) {
		return invalid;
	}
	const size_t nsides = level_.get_children("side").size();
	if(side_num > nsides)
		return invalid;

	const size_t side_index = static_cast<size_t>(side_num - 1);

	if(side_controllers_[side_index] == "network" && sides_taken_[side_index]) {
		static const std::string already = "This side is already controlled by a player";
		return already;
	}

	if(cfg["orphan_side"] != "yes") {
		const size_t active_side = end_turn_/nsides + 1;

		if(lexical_cast_default<size_t>(side,0) == active_side) {
			static const std::string not_during_turn = "You cannot change a side's controller during its turn";
			return not_during_turn;
		}
	}
	side_controllers_[side_index] = "network";
	sides_taken_[side_index] = true;
	sides_.insert(std::pair<const network::connection,size_t>(*i, side_index));

	// send a response to the host and to the new controller
	config response;
	config& change = response.add_child("change_controller");

	change["side"] = side;

	change["controller"] = "network";
	network::queue_data(response,players_.front());

	change["controller"] = "human";
	network::queue_data(response,*i);

	if(sides_.count(*i) < 2) {
		//send everyone a message saying that the observer who is taking the side has quit
		config observer_quit;
		observer_quit.add_child("observer_quit").values["name"] = player;
		send_data(observer_quit);
	}
	static const std::string success = "";
	return success;
}

size_t game::available_slots() const
{
	size_t available_slots = 0;
	const config::child_list& sides = level_.get_children("side");
	for(config::child_list::const_iterator i = sides.begin(); i != sides.end(); ++i) {
		std::cerr << "side controller: '" << (**i)["controller"] << "'\n";
		if((**i)["controller"] == "network" && (**i)["description"].empty()) {
			++available_slots;
		}
	}

	return available_slots;
}

bool game::describe_slots()
{
	if(description() == NULL)
		return false;

	const int val = int(available_slots());
	char buf[50];
	sprintf(buf,"%d",val);

	if(buf != (*description())["slots"]) {
		description()->values["slots"] = buf;
		description()->values["observer"] = level_["observer"];
		return true;
	} else {
		return false;
	}
}

bool game::player_is_banned(network::connection sock) const
{
	if(bans_.empty()) {
		return false;
	}

	const player_map::const_iterator itor = player_info_->find(sock);
	if(itor == player_info_->end()) {
		return false;
	}

	const player& info = itor->second;
	const std::string& ipaddress = network::ip_address(sock);
	for(std::vector<ban>::const_iterator i = bans_.begin(); i != bans_.end(); ++i) {
		if(info.name() == i->username || ipaddress == i->ipaddress) {
			return true;
		}
	}

	return false;
}

void game::ban_player(network::connection sock)
{
	const player_map::const_iterator itor = player_info_->find(sock);
	if(itor != player_info_->end()) {
		bans_.push_back(ban(itor->second.name(),network::ip_address(sock)));
	}

	remove_player(sock);
}

bool game::process_commands(const config& cfg)
{
	//std::cerr << "processing commands: '" << cfg.write() << "'\n";
	bool res = false;
	const config::child_list& cmd = cfg.get_children("command");
	for(config::child_list::const_iterator i = cmd.begin(); i != cmd.end(); ++i) {
		if((**i).child("end_turn") != NULL) {
			res = res || end_turn();
			//std::cerr << "res: " << (res ? "yes" : "no") << "\n";
		}
	}

	return res;
}

bool game::end_turn()
{
	//it's a new turn every time each side in the game ends their turn.
	++end_turn_;

	const size_t nsides = level_.get_children("side").size();

	if((end_turn_%nsides) != 0) {
		return false;
	}

	const size_t turn = end_turn_/nsides + 1;

	config* const desc = description();
	if(desc == NULL) {
		return false;
	}

	desc->values["turn"] = describe_turns(int(turn),level()["turns"]);

	return true;
}

void game::add_player(network::connection player)
{
	//if the game has already started, we add the player as an observer
	if(started_) {
		if(!allow_observers_) {
			return;
		}

		player_map::const_iterator info = player_info_->find(player);
		if(info != player_info_->end()) {
			config observer_join;
			observer_join.add_child("observer").values["name"] = info->second.name();
			//send observer join to everyone except player
			send_data(observer_join, player);
		}

		//tell this player that the game has started
		config cfg;
		cfg.add_child("start_game");
		network::queue_data(cfg, player);

		//send observer join of all the observers in the game to player
		for(std::vector<network::connection>::const_iterator pl = players_.begin()+1; pl != players_.end(); ++pl) {
			if(sides_.count(*pl) == 0 && *pl != player) {
				info = player_info_->find(*pl);
				if(info != player_info_->end()) {
					cfg.clear();
					cfg.add_child("observer").values["name"] = info->second.name();
					network::queue_data(cfg, player);
				}
			}
		}
	}

	//if the player is already in the game, don't add them.
	if(std::find(players_.begin(),players_.end(),player) != players_.end()) {
		return;
	}

	players_.push_back(player);

	send_user_list();

	//send the player the history of the game to-date
	network::queue_data(history_,player);
}

void game::remove_player(network::connection player, bool notify_creator)
{
	const std::vector<network::connection>::iterator itor =
	             std::find(players_.begin(),players_.end(),player);

	if(itor != players_.end())
		players_.erase(itor);

	bool observer = true;
	std::pair<std::multimap<network::connection,size_t>::const_iterator,
	          std::multimap<network::connection,size_t>::const_iterator> sides = sides_.equal_range(player);
	while(sides.first != sides.second) {
		//send the host a notification of removal of this side
		if(notify_creator && players_.empty() == false) {
			config drop;
			drop["side_drop"] = lexical_cast<std::string, size_t>(sides.first->second + 1);
			network::queue_data(drop, players_.front());
		}
		sides_taken_[sides.first->second] = false;
		observer = false;
		++sides.first;
	}
	if(!observer)
		sides_.erase(player);

	send_user_list();

	const player_map::const_iterator pl = player_info_->find(player);
	if(!observer || pl == player_info_->end()) {
		return;
	}

	//they're just an observer, so send them having quit to clients
	config observer_quit;
	observer_quit.add_child("observer_quit").values["name"] = pl->second.name();
	send_data(observer_quit);
}

void game::send_user_list()
{
	//if the game hasn't started yet, then send all players a list
	//of the users in the game
	if(started_ == false && description() != NULL) {
		config cfg;
		cfg.add_child("gamelist");
		for(std::vector<network::connection>::const_iterator p = players_.begin(); p != players_.end(); ++p) {

			const player_map::const_iterator info = player_info_->find(*p);
			if(info != player_info_->end()) {
				config& user = cfg.add_child("user");
				user["name"] = info->second.name();
			}
		}

		send_data(cfg);
	}
}


int game::id() const
{
	return id_;
}

void game::send_data(const config& data, network::connection exclude)
{
	for(std::vector<network::connection>::const_iterator
	    i = players_.begin(); i != players_.end(); ++i) {
		if(*i != exclude && (allow_observers_ || is_needed(*i) || sides_.count(*i) == 1)) {
			network::queue_data(data,*i);
		}
	}
}

bool game::player_on_team(const std::string& team, network::connection player) const
{
	std::pair<std::multimap<network::connection,size_t>::const_iterator,
	          std::multimap<network::connection,size_t>::const_iterator> sides = sides_.equal_range(player);
	while(sides.first != sides.second) {
		const config* const side_cfg = level_.find_child("side","side", lexical_cast<std::string, size_t>(sides.first->second + 1));
		if(side_cfg != NULL && (*side_cfg)["team_name"] == team) {
			return true;
		}
		++sides.first;
	}

	return false;
}

void game::send_data_team(const config& data, const std::string& team, network::connection exclude)
{
	for(std::vector<network::connection>::const_iterator i = players_.begin(); i != players_.end(); ++i) {
		if(*i != exclude && player_on_team(team,*i)) {
			network::queue_data(data,*i);
		}
	}
}

void game::send_data_observers(const config& data)
{
	for(std::vector<network::connection>::const_iterator i = players_.begin(); i != players_.end(); ++i) {
		if(is_observer(*i)) {
			network::queue_data(data,*i);
		}
	}
}

void game::record_data(const config& data)
{
	history_.append(data);
}

void game::reset_history()
{
	history_.clear();
}

bool game::level_init() const
{
	return level_.child("side") != NULL;
}

config& game::level()
{
	return level_;
}

bool game::empty() const
{
	return players_.empty();
}

void game::disconnect()
{
	for(std::vector<network::connection>::iterator i = players_.begin();
	    i != players_.end(); ++i) {
		network::queue_disconnect(*i);
	}

	players_.clear();
}

void game::set_description(config* desc)
{
	description_ = desc;
}

config* game::description()
{
	return description_;
}

void game::add_players(const game& other_game)
{
	players_.insert(players_.end(),
	                other_game.players_.begin(),other_game.players_.end());
}

bool game::started() const
{
	return started_;
}
