#include "game.hpp"

#include <cassert>
#include <iostream>

int game::id_num = 1;

game::game() : id_(id_num++), started_(false), description_(NULL)
{}

bool game::is_member(network::connection player) const
{
	return std::find(players_.begin(),players_.end(),player) != players_.end();
}

bool game::is_needed(network::connection player) const
{
	assert(!players_.empty());
	return player == players_.front() || started_ && sides_.count(player);
}

void game::start_game()
{
	started_ = true;
}

bool game::take_side(network::connection player, const config& cfg)
{
	assert(is_member(player));

	const std::string& side = cfg["side"];
	
	//if the player is already on a side or the side is already taken
	if(sides_.count(player) || sides_taken_.count(side))
		return false;

	sides_[player] = side;
	sides_taken_.insert(side);

	//send host notification of taking this side
	if(players_.empty() == false) {
		network::send_data(cfg,players_.front());
	}

	return true;
}

void game::add_player(network::connection player)
{
	players_.push_back(player);

	//send the player the history of the game to-date
	for(std::vector<config>::const_iterator i = history_.begin();
	    i != history_.end(); ++i) {
		network::send_data(*i,player);
	}
}

void game::remove_player(network::connection player)
{
	const std::vector<network::connection>::iterator itor =
	             std::find(players_.begin(),players_.end(),player);
	if(itor != players_.end())
		players_.erase(itor);

	std::map<network::connection,std::string>::iterator side
	                                                 = sides_.find(player);
	if(side != sides_.end()) {
		//send the host a notification of removal of this side
		if(players_.empty() == false) {
			config drop;
			drop["side_drop"] = side->second;
			network::send_data(drop,players_.front());
		}

		sides_taken_.erase(side->second);
		sides_.erase(side);
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
		if(*i != exclude) {
			network::send_data(data,*i);
		}
	}
}

void game::record_data(const config& data)
{
	history_.push_back(data);
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
