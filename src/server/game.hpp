#ifndef GAME_HPP_INCLUDED
#define GAME_HPP_INCLUDED

#include "../config.hpp"
#include "../network.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <vector>

class game
{
public:
	game();

	bool is_member(network::connection player) const;
	bool is_needed(network::connection player) const;

	void start_game();

	bool take_side(network::connection player, const config& cfg);
	

	void add_player(network::connection player);
	void remove_player(network::connection player);

	int id() const;

	void send_data(const config& data, network::connection exclude=0);
	void record_data(const config& data);

	bool level_init() const;
	config& level();
	bool empty() const;
	void disconnect();

	void set_description(config* desc);
	config* description();

	void add_players(const game& other_game);

private:
	static int id_num;
	int id_;
	std::vector<network::connection> players_;
	std::map<network::connection,std::string> sides_;
	std::set<std::string> sides_taken_;	
	bool started_;

	config level_;

	std::vector<config> history_;

	config* description_;
};

struct game_id_matches
{
	game_id_matches(int id) : id_(id) {}
	bool operator()(const game& g) const { return g.id() == id_; }

private:
	int id_;
};

#endif
