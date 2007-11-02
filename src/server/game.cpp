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

#include "game.hpp"
#include "../log.hpp"
#include "../util.hpp"
#include "../wassert.hpp"

#include <iostream>
#include <sstream>

#define ERR_GAME LOG_STREAM(err, mp_server)
#define LOG_GAME LOG_STREAM(info, mp_server)
#define DBG_GAME LOG_STREAM(debug, mp_server)

int game::id_num = 1;

game::game(const player_map& pl) : player_info_(&pl), id_(id_num++), sides_(9),
	sides_taken_(9), side_controllers_(9), started_(false), description_(NULL),
	end_turn_(0), allow_observers_(true), all_observers_muted_(false)
{}

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

bool game::filter_commands(const network::connection player, config& cfg) const {
	if(is_observer(player)) {
		std::vector<int> marked;
		int index = 0;

		const config::child_list& children = cfg.get_children("command");
		for(config::child_list::const_iterator i = children.begin();
			i != children.end(); ++i)
		{
			if ((observers_can_label() && (*i)->child("label") != NULL)
				|| (observers_can_chat() && (*i)->child("speak") != NULL)
				&& (*i)->all_children().size() == 1)
			{
			} else {
				DBG_GAME << "removing observer's illegal command\n";
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
	snprintf(buf,sizeof(buf),"%d/",int(turn));

	if(num_turns == "-1") {
		return buf + std::string("-");
	} else {
		return buf + num_turns;
	}
}

}//anon namespace

void game::start_game()
{
	started_ = true;

	//set all side controllers to 'human' so that observers will understand that they can't
	//take control of any sides if they happen to have the same name as one of the descriptions
	
	for(config::child_itors sides = level_.child_range("side");
		sides.first != sides.second; ++sides.first)
	{
		if ((**sides.first)["controller"] != "null")
			(**sides.first)["controller"] = "human";
	}

	int turn =  lexical_cast_default<int>(level()["turn_at"], 1);
	end_turn_ = (turn - 1) * level_.get_children("side").size();
	if(description()) {
		description()->values["turn"] = describe_turns(turn, level()["turns"]);
	} else {
		ERR_GAME << "ERROR: Game without description_ started. (" << id_ << ")\n";
	}

	allow_observers_ = level_["observer"] != "no";

	//send [observer] tags for all observers that have already joined.
	//we can do this by re-joining all players that don't have sides, and aren't
	//the first player (game creator)
	// Observer tags should have already been sent at the appropriate times.
/*	for(user_vector::const_iterator pl = observers_.begin(); pl != observers_.end(); ++pl) {
		if(sides_.count(*pl) == 0) {
			add_player(*pl);
		}
	}*/
}

bool game::take_side(network::connection player, const config& cfg)
{
	if (!is_member(player)) return false;

	//verify that side is a side id
	const std::string& side = cfg["side"];
	size_t side_num = lexical_cast_in_range<size_t, std::string>(side, 1, 1, 9);

	//if the side is already taken, see if we can give the player
	//another side instead
	if(side_controllers_[side_num - 1] == "human"
	   || (sides_taken_[side_num - 1] && side_controllers_[side_num - 1] == "network")) {
		const config::child_list& sides = level_.get_children("side");
		for(config::child_list::const_iterator i = sides.begin(); i != sides.end(); ++i) {
			if((**i)["controller"] == "network") {
				//don't allow players to take sides in games with invalid side numbers
				try {
					side_num = lexical_cast<size_t, std::string>((**i)["side"]);
					if(side_num < 1 || side_num > 9)
						return false;
				}
				catch(bad_lexical_cast&) {
					return false;
				}
				//check if the side is taken if not take it
				if(!sides_taken_[side_num - 1]) {
					side_controllers_[side_num - 1] = "network";
					sides_[side_num - 1] = player;
					sides_taken_[side_num - 1] = true;
					config new_cfg = cfg;
					new_cfg["side"] = (**i)["side"];
					network::queue_data(new_cfg, owner_);
					return true;
				}
			}
		}
		//if we get here we couldn't find a side to take
		return false;
	}
	//else take the current side
	if (player == owner_ && started_ && !cfg["controller"].empty()) {
		//if the owner transfered a side to an ai or a human in game
		//fake a "change_controller" command so other player update controller name
		config fake;
		config& change = fake.add_child("change_controller");
		change["side"] = side;
		change["player"] = cfg["name"];
		change["controller"] = "network";
		send_data(fake, owner_); //send change to all except owner
		//send a server message displaying new controller name
		send_data(construct_server_message(cfg["name"] + " takes control of side " + side));

		//update level_ so observers who join get the new player name
		config::child_itors it = level_.child_range("side");
		it.first += side_num - 1;
		wassert(it.first != it.second);
		(**it.first)["current_player"] = cfg["name"];

		side_controllers_[side_num - 1] = cfg["controller"];
	} else
		side_controllers_[side_num - 1] = "network";
	sides_[side_num - 1] = player;
	sides_taken_[side_num - 1] = true;
	// Send the taken side to the host.
	network::queue_data(cfg, owner_);
	return true;
}

//! Resets the side configuration according to the scenario data.
void game::update_side_data() {
	// Remember everyone that is in the game.
	const user_vector users = all_game_users();

	sides_taken_.clear();
	sides_taken_.resize(9);
	sides_.clear();
	sides_.resize(9);
	players_.clear();
	observers_.clear();

	const config::child_itors level_sides = level_.child_range("side");
	bool side_found;
	//for each player:
	// * Find the player name
	// * Find the side this player name corresponds to
	//! @todo: Should iterate over sides_ then over users.
	for(user_vector::const_iterator player = users.begin(); player != users.end(); ++player) {
		player_map::const_iterator info = player_info_->find(*player);
		if (info == player_info_->end()) {
			ERR_GAME << "Error: unable to find player info for connection: "
				<< *player << "\n";
			continue;
		}

		side_found = false;
		size_t side_num;
		for(config::child_iterator sd = level_sides.first;
			sd != level_sides.second; ++sd)
		{
			try {
				side_num = lexical_cast<size_t, std::string>((**sd)["side"]);
				if(side_num < 1 || side_num > 9)
					continue;
			}
			catch(bad_lexical_cast&) {
				continue;
			}
			if (!sides_taken_[side_num - 1]){
				if((**sd)["controller"] == "network") {
					side_controllers_[side_num - 1] = "network";
					/*TODO : change the next line (and all related code :o) to :
					  if((**sd)["curent_player"] == info->second.name()) {
					  I won't make it before 1.2 is out in case of regression */
					if((**sd)["user_description"] == info->second.name()) {
						sides_[side_num - 1] = *player;
						sides_taken_[side_num - 1] = true;
						side_found = true;
					}
					else {
						sides_taken_[side_num - 1] = false;
					}
				}
				else if((**sd)["controller"] == "ai") {
					sides_[side_num - 1] = owner_;
					side_controllers_[side_num - 1] = "ai";
					sides_taken_[side_num - 1] = true;
					side_found = true;
				}
				else if((**sd)["controller"] == "human") {
					sides_[side_num - 1] = owner_;
					sides_taken_[side_num - 1] = true;
					side_found = true;
					side_controllers_[side_num - 1] = "human";
				}
			}
		}

		if (side_found){
			players_.push_back(*player);
		} else {
			observers_.push_back(*player);
		}
	}
	DBG_GAME << debug_player_info();
}

void game::transfer_side_control(const network::connection sock, const config& cfg) {
	DBG_GAME << "transfer_side_control...\n";

	//check, if this socket belongs to a player
	const user_vector::iterator pl = find_connection(sock, players_);
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
		network::send_data(construct_server_message("Player/Observer not in this game."), sock);
		return;
	}
	// Check the side number.
	const std::string& side = cfg["side"];
	size_t side_num;
	try {
		side_num = lexical_cast<size_t, std::string>(side);
		if(side_num < 1 || side_num > 9) {
			network::send_data(construct_server_message(
				"The side number has to be between 1 and 9."), sock);
			return;
		}
	}
	catch(bad_lexical_cast&) {
		network::send_data(construct_server_message("Not a side number."), sock);
		return;
	}
	if (side_num > level_.get_children("side").size()) {
		network::send_data(construct_server_message("Invalid side number."), sock);
		return;
	}

	if(side_controllers_[side_num - 1] == "network" && sides_taken_[side_num - 1]
	   && cfg["own_side"] != "yes")
	{
		network::send_data(construct_server_message(
			"This side is already controlled by a player."), sock);
		return;
	}
	// Check if the sender actually owns the side he gives away or is the host.
	if (!(sides_[side_num - 1] == sock || (sock == owner_))) {
		DBG_GAME << "Side belongs to: " << sides_[side_num - 1] << "\n";
		network::send_data(construct_server_message("Not your side."), sock);
		return;
	}
	if (newplayer->first == sock) {
		network::send_data(construct_server_message(
			"That's already your side, silly."), sock);
		return;
	}
	sides_[side_num - 1] = 0;
	bool host_leave = false;
	// If the player gave up their last side, make them an observer.
	if (std::find(sides_.begin(), sides_.end(), sock) == sides_.end()) {
		observers_.push_back(*pl);
		players_.erase(pl);
		// Tell others that the player becomes an observer.
		send_data(construct_server_message(find_player(sock)->name()
			+ " becomes an observer."));
		// Update the client side observer list for everyone except player.
		config observer_join;
		observer_join.add_child("observer").values["name"] = find_player(sock)->name();
		send_data(observer_join, sock);
		// If this player was the host of the game, choose another player.
		if (sock == owner_) {
			host_leave = true;
			if (!players_.empty()) {
				owner_ = players_.front();
				send_data(construct_server_message(find_player(owner_)->name()
					+ " has been chosen as the new host."));
			} else {
				owner_ = newplayer->first;
				send_data(construct_server_message(newplayer_name
					+ " has been chosen as the new host."));
			}
		}
	}
	side_controllers_[side_num - 1] = "network";
	sides_taken_[side_num - 1] = true;
	sides_[side_num - 1] = newplayer->first;

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
	network::queue_data(response, newplayer->first);

	// Update the level so observer who join get the new name.
	config::child_itors it = level_.child_range("side");
	it.first += side_num - 1;
	wassert(it.first != it.second);
	(**it.first)["current_player"] = newplayer_name;

	//if the host left and there are ai sides, transfer them to the new host
	if (host_leave) {
		for (unsigned int i = 0; i < side_controllers_.size(); i++) {
			if (side_controllers_[i] == "ai") {
				change["side"] = lexical_cast<std::string, unsigned int>(i + 1);
				change["controller"] = "ai";
				network::queue_data(response, owner_);
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
		// Send everyone a message saying that the observer who is taking the
		// side has quit.
		config observer_quit;
		observer_quit.add_child("observer_quit").values["name"] = newplayer_name;
		send_data(observer_quit);
	}
	send_data(construct_server_message(newplayer_name
		+ " takes control of side " + side + "."));
}

bool game::describe_slots() {
	if (started_ || description_ == NULL)
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
		description_->values["slots"] = buf;
		return true;
	} else {
		return false;
	}
}

bool game::player_is_banned(const network::connection sock) const {
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

const player* game::mute_observer(const network::connection sock) {
	const player* muted_observer = NULL;
	player_map::const_iterator it = player_info_->find(sock);
	if (it != player_info_->end() && ! is_muted_observer(sock)){
		muted_observers_.push_back(sock);
		muted_observer = &it->second;
	}

	return muted_observer;
}

void game::ban_player(const network::connection sock) {
	const player_map::const_iterator itor = player_info_->find(sock);
	if (itor != player_info_->end()) {
		bans_.push_back(ban(itor->second.name(),network::ip_address(sock)));
		network::send_data(construct_server_message("You have been banned."), sock);
		send_data(construct_server_message(itor->second.name() + " has been banned."));
		remove_player(sock);
	} else {
		ERR_GAME << "ERROR: Player not found in player_info_. (socket: " << sock << ")\n";
		//! @todo: Should return something indicating the player wasn't found.
	}
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
	//it's a new turn every time each side in the game ends their turn.
	++end_turn_;

	const size_t nsides = level_.get_children("side").size();

	if(nsides == 0 || (end_turn_%nsides) != 0) {
		return false;
	}

	const size_t turn = end_turn_/nsides + 1;

	if (description_ == NULL) {
		return false;
	}
	description_->values["turn"] = describe_turns(int(turn), level()["turns"]);

	return true;
}

void game::add_player(const network::connection player, const bool observer) {
	// Hack to handle the pseudo games lobby_players_ and not_logged_in_.
	if (id_ <= 2) {
		observers_.push_back(player);
		return;
	}
	//if the player is already in the game, don't add them.
	if(is_member(player)) {
		return;
	}

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
		network::queue_data(config("start_game"), player);

		//send observer join of all the observers in the game to players
		for(user_vector::const_iterator ob = observers_.begin(); ob != observers_.end(); ++ob) {
			if(*ob != player) {
				info = player_info_->find(*ob);
				if(info != player_info_->end()) {
					config cfg;
					cfg.add_child("observer").values["name"] = info->second.name();
					network::queue_data(cfg, player);
				}
			}
		}
	}

	//Check first if there are available sides
	//If not, add the player as observer
	unsigned int human_sides = 0;
	if (level_.get_children("side").size() > 0 && !observer){
		config::child_list sides = level_.get_children("side");
		for (config::child_list::const_iterator side = sides.begin(); side != sides.end(); side++){
			if (((**side)["controller"] == "human") || ((**side)["controller"] == "network")){
				human_sides++;
			}
		}
	}
	else{
		if (level_.get_attribute("human_sides") != ""){
			human_sides = lexical_cast<unsigned int>(level_["human_sides"]);
		}
	}
	DBG_GAME << debug_player_info();
	if (human_sides > players_.size()){
		DBG_GAME << "adding player...\n";
		players_.push_back(player);
	} else{
		DBG_GAME << "adding observer...\n";
		observers_.push_back(player);
		player_map::const_iterator info = player_info_->find(player);
		if(info != player_info_->end()) {
			config observer_join;
			observer_join.add_child("observer").values["name"] = info->second.name();
			//send observer join to everyone except player
			send_data(observer_join, player);
		}
	}
	DBG_GAME << debug_player_info();
	send_user_list();

	//send the player the history of the game to-date
	network::queue_data(history_,player);
}

void game::remove_player(const network::connection player, const bool notify_creator) {
	// Hack to handle the pseudo games lobby_players_ and not_logged_in_.
	if (id_ <= 2) {
		const user_vector::iterator itor = std::find(observers_.begin(), observers_.end(), player);
		if (itor != players_.end())
			observers_.erase(itor);
		else
			DBG_GAME << "ERROR: Player is not in this game. (socket: "
			<< player << ")\n";
		return;
	}
	DBG_GAME << debug_player_info();
	DBG_GAME << "removing player...\n";

	if (!is_member(player))
		return;
	bool host = (player == owner_);
	bool observer = true;
	{
		const user_vector::iterator itor = std::find(players_.begin(), players_.end(), player);
		if (itor != players_.end()) {
			players_.erase(itor);
			observer = false;
		}
	}
	{
		const user_vector::iterator itor = std::find(observers_.begin(), observers_.end(), player);
		if (itor != observers_.end()) {
			observers_.erase(itor);
			if (!observer) 
				ERR_GAME << "ERROR: Player is also an observer. (socket: "
				<< player << ")\n";
			observer = true;
		}
	}
	// If the player was host choose a new one.
	if (host && !players_.empty() && started_) {
		owner_ = players_.front();
		const player_map::const_iterator host = player_info_->find(owner_);
		if (host == player_info_->end()) {
			ERR_GAME << "ERROR: Could not find new host in player_info_. (socket: "
				<< owner_ << ")\n";
			return;
		}
		send_data(construct_server_message(host->second.name()
			+ " has been chosen as new host"));
		//check for ai sides first and drop them, too, if the host left
		bool ai_transfer = false;
		//can't do this with an iterator, because it doesn't know the side_num - 1
		for (size_t side = 0; side < side_controllers_.size(); ++side){
			//send the host a notification of removal of this side
			if(notify_creator && players_.empty() == false && side_controllers_[side] == "ai") {
				ai_transfer = true;
				config drop;
				drop["side_drop"] = lexical_cast<std::string, size_t>(side + 1);
				drop["controller"] = "ai";
				network::queue_data(drop, owner_);
				sides_taken_[side] = false;
			}
		}
		if (ai_transfer) {
			std::string msg = "AI transferred to new host";
			send_data(construct_server_message(msg));
		}
	}

	//look for all sides the player controlled and drop them
	for (side_vector::iterator side = sides_.begin(); side != sides_.end();
		 side = std::find(side, sides_.end(), player))
	{
		//send the host a notification of removal of this side
		if (notify_creator && players_.empty() == false) {
			config drop;
			drop["side_drop"] = lexical_cast<std::string, size_t>(side - sides_.begin() + 1);
			drop["controller"] = side_controllers_[side - sides_.begin()];
			network::queue_data(drop, owner_);
		}
		side_controllers_[side - sides_.begin()] = "null";
		sides_taken_[side - sides_.begin()] = false;
		sides_[side - sides_.begin()] = 0;
	}
	DBG_GAME << debug_player_info();

	send_user_list(player);
	if (!observer) {
		return;
	}

	const player_map::const_iterator obs = player_info_->find(player);
	if (obs == player_info_->end()) {
		ERR_GAME << "ERROR: Could not find observer in player_info_. (socket: "
			<< player << ")\n";
		return;
	}

	//they're just an observer, so send them having quit to clients
	config observer_quit;
	observer_quit.add_child("observer_quit").values["name"] = obs->second.name();
	send_data(observer_quit);
}

void game::send_user_list(const network::connection exclude) const {
	//if the game hasn't started yet, then send all players a list
	//of the users in the game
	if (started_ == false && description_ != NULL) {
		//! @todo: Should be renamed to userlist.
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
}

void game::send_data(const config& data, const network::connection exclude) const {
	const user_vector users = all_game_users();
	for(user_vector::const_iterator i = users.begin(); i != users.end(); ++i) {
		if (*i != exclude) {
			network::queue_data(data,*i);
		}
	}
}

bool game::player_on_team(const std::string& team, const network::connection player) const {
	for (side_vector::const_iterator side = sides_.begin(); side != sides_.end();
		 side = std::find(side, sides_.end(), player))
	{
		const config* const side_cfg = level_.find_child("side", "side",
			lexical_cast<std::string, size_t>(side - sides_.begin() + 1));
		if (side_cfg != NULL && (*side_cfg)["team_name"] == team) {
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
			network::queue_data(data,*i);
		}
	}
}

void game::send_data_observers(const config& data, const network::connection exclude) const {
	for(user_vector::const_iterator i = observers_.begin(); i != observers_.end(); ++i) {
		if (*i != exclude) {
			network::queue_data(data,*i);
		}
	}
}

void game::record_data(const config& data) {
	history_.append(data);
}

void game::reset_history() {
	history_.clear();
	end_turn_ = 0;
}

void game::end_game() {
	send_data(config("leave_game"));
	players_.clear();
	observers_.clear();
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
	if (id_ < 3) return result.str();
	result << "---------------------------------------\n";
	result << "game id: " << id_ << "\n";
	result << "players_.size: " << players_.size() << "\n";
	for (user_vector::const_iterator p = players_.begin(); p != players_.end(); p++){
		const player_map::const_iterator user = player_info_->find(*p);
		if (user != player_info_->end()){
			result << "player: " << user->second.name().c_str() << "\n";
		}
		else{
			result << "player not found\n";
		}
	}
	result << "observers_.size: " << observers_.size() << "\n";
	for (user_vector::const_iterator o = observers_.begin(); o != observers_.end(); o++){
		const player_map::const_iterator user = player_info_->find(*o);
		if (user != player_info_->end()){
			result << "observer: " << user->second.name().c_str() << "\n";
		}
		else{
			result << "observer not found\n";
		}
	}
	{
		result << "player_info_: begin\n";
		for (player_map::const_iterator info = player_info_->begin(); info != player_info_->end(); info++){
			result << info->second.name().c_str() << "\n";
		}
		result << "player_info_: end\n";
	}
	result << "---------------------------------------\n";
	return result.str();
}

const player* game::find_player(const network::connection sock) const {
	const player* result = NULL;
	for (player_map::const_iterator info = player_info_->begin(); info != player_info_->end(); info++){
		if (info->first == sock){
			result = &info->second;
		}
	}
	return result;
}

user_vector::iterator game::find_connection(const network::connection sock,
	user_vector& users) const
{
	return std::find(users.begin(), users.end(), sock);
}

config game::construct_server_message(const std::string& message) const {
	config turn;
	if(started()) {
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
