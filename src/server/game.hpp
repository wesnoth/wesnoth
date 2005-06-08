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

	bool is_owner(network::connection player) const;
	bool is_member(network::connection player) const;
	bool is_needed(network::connection player) const;
	bool is_observer(network::connection player) const;

	bool observers_can_label() const;
	bool observers_can_chat() const;

	//function which filters commands sent by a player to remove commands
	//that they don't have permission to execute.
	//Returns true iff there are still some commands left
	bool filter_commands(network::connection player, config& cfg);

	void start_game();

	bool take_side(network::connection player, const config& cfg);

	void update_side_data();

	const std::string& transfer_side_control(const config& cfg);

	size_t available_slots() const;

	//function to set the description to the number of slots
	//returns true if the number of slots has changed
	bool describe_slots();

	bool player_is_banned(network::connection player) const;
	void ban_player(network::connection player);

	void add_player(network::connection player);
	void remove_player(network::connection player, bool notify_creator=true);

	int id() const;

	void send_data(const config& data, network::connection exclude=0);
	void send_data_team(const config& data, const std::string& team, network::connection exclude=0);
	void send_data_observers(const config& data);

	void record_data(const config& data);
	void reset_history();

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

	bool started() const;

	size_t nplayers() const { return players_.size(); }

	const std::string& termination_reason() const {
		static const std::string aborted = "aborted";
		return termination_.empty() ? aborted : termination_;
	}

	void set_termination_reason(const std::string& reason) {
		if(termination_.empty()) { termination_ = reason; }
	}

private:

	//function which returns true iff 'player' is on 'team'.
	bool player_on_team(const std::string& team, network::connection player) const;

	//function which should be called every time a player ends their turn
	//(i.e. [end_turn] received). This will update the 'turn' attribute for
	//the game's description when appropriate. Will return true if there has
	//been a change.
	bool end_turn();

	//function to send a list of users to all clients. Only sends data before
	//the game has started.
	void send_user_list();

	const player_map* player_info_;

	static int id_num;
	int id_;
	std::vector<network::connection> players_;
	std::multimap<network::connection,size_t> sides_;
	std::vector<bool> sides_taken_;
	std::vector<std::string> side_controllers_;
	bool started_;

	config level_;

	config history_;

	config* description_;

	int end_turn_;

	bool allow_observers_;

	struct ban {
		ban(const std::string& name, const std::string& address)
		      : username(name), ipaddress(address)
		{}

		std::string username;
		std::string ipaddress;
	};

	std::vector<ban> bans_;

	std::string termination_;
};

struct game_id_matches
{
	game_id_matches(int id) : id_(id) {}
	bool operator()(const game& g) const { return g.id() == id_; }

private:
	int id_;
};

#endif
