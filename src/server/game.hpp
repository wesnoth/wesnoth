#ifndef GAME_HPP_INCLUDED
#define GAME_HPP_INCLUDED

#include "../config.hpp"
#include "../network.hpp"

#include <algorithm>
#include <vector>

class game
{
public:
	game();

	bool is_member(network::connection player) const;

	void add_player(network::connection player);
	void remove_player(network::connection player);

	int id() const;

	void send_data(const config& data, network::connection exclude=0);

	bool level_init() const;
	config& level();
	bool empty() const;
	void disconnect();

private:
	static int id_num;
	int id_;
	std::vector<network::connection> players_;
	config level_;
};

struct game_id_matches
{
	game_id_matches(int id) : id_(id) {}
	bool operator()(const game& g) const { return g.id() == id_; }

private:
	int id_;
};

#endif
