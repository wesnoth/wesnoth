#ifndef GAME_HPP_INCLUDED
#define GAME_HPP_INCLUDED

#include "../config.hpp"
#include "../network.hpp"

#include "player.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <vector>

typedef std::map<network::connection,player> player_map;

class game
{
public:
	game(const player_map& info);

	bool is_member(network::connection player) const;
	bool is_needed(network::connection player) const;

	void start_game();

	bool take_side(network::connection player, const config& cfg);
	
	size_t available_slots() const;

	//function to set the description to the number of slots
	//returns true if the number of slots has changed
	bool describe_slots();

	void add_player(network::connection player);
	void remove_player(network::connection player);

	int id() const;

	void send_data(const config& data, network::connection exclude=0);
	void record_data(const config& data);

	//the full scenario data
	bool level_init() const;
	config& level();
	bool empty() const;
	void disconnect();

	//functions to set/get the address of the game's summary description as
	//sent to players in the lobby
	void set_description(config* desc);
	config* description();

	void add_players(const game& other_game);

	//function which will process game commands and update the state of the
	//game accordingly. Will return true iff the game's description changes.
	bool process_commands(const config& cfg);

private:

	//function which should be called every time a player ends their turn
	//(i.e. [end_turn] received). This will update the 'turn' attribute for
	//the game's description when appropriate. Will return true if there has
	//been a change.
	bool end_turn();

	const player_map* player_info_;

	static int id_num;
	int id_;
	std::vector<network::connection> players_;
	std::map<network::connection,std::string> sides_;
	std::set<std::string> sides_taken_;
	bool started_;

	config level_;

	std::vector<config> history_;

	config* description_;

	int end_turn_;

	bool allow_observers_;
};

struct game_id_matches
{
	game_id_matches(int id) : id_(id) {}
	bool operator()(const game& g) const { return g.id() == id_; }

private:
	int id_;
};

#endif
