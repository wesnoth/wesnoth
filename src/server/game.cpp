#include "game.hpp"

int game::id_num = 1;

game::game() : id_(id_num++)
{}

bool game::is_member(network::connection player) const
{
	return std::find(players_.begin(),players_.end(),player) != players_.end();
}

void game::add_player(network::connection player)
{
	players_.push_back(player);
}

void game::remove_player(network::connection player)
{
	const std::vector<network::connection>::iterator itor =
	             std::find(players_.begin(),players_.end(),player);
	if(itor != players_.end())
		players_.erase(itor);
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
		network::disconnect(*i);
	}

	players_.clear();
}
