/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef PLAYTURN_HPP_INCLUDED
#define PLAYTURN_HPP_INCLUDED

class replay_network_sender;

#include "global.hpp"

#include "actions.hpp"
#include "config.hpp"
#include "display.hpp"
#include "gamestatus.hpp"
#include "generic_event.hpp"
#include "network.hpp"
#include "team.hpp"
#include "unit.hpp"

#include <map>
#include <vector>

class turn_info
{
public:
	turn_info(const game_data& gameinfo, game_state& state_of_game,
	          const gamestatus& status, display& gui, gamemap& map,
		  std::vector<team>& teams, unsigned int team_num, unit_map& units,
		  replay_network_sender& network_sender, undo_list& undo_stack);

	~turn_info();

	void sync_network();

	void send_data();

	enum PROCESS_DATA_RESULT { PROCESS_CONTINUE, PROCESS_RESTART_TURN, PROCESS_END_TURN };

	//function which will process incoming network data, and act on it. If there is
	//more data than a single turn's worth, excess data will be placed into 'backlog'.
	//No more than one turn's worth of data will be placed into a single backlog item,
	//so it is safe to assume that backlog won't be touched if cfg is a member of a previous
	//backlog.
	//data will be forwarded to all peers other than 'from', unless 'from' is null, in
	//which case data will not be forwarded
	PROCESS_DATA_RESULT process_network_data(const config& cfg,network::connection from,std::deque<config>& backlog, bool skip_replay);

	events::generic_event& replay_error() { return replay_error_; }
private:
	static void change_side_controller(const std::string& side, const std::string& player, bool own_side=false);
	static void take_side(const std::string& side, const std::string& controller);

	const game_data& gameinfo_;
	game_state& state_of_game_;
	const gamestatus& status_;
	display& gui_;
	gamemap& map_;
	std::vector<team>& teams_;
	unsigned int team_num_;
	unit_map& units_;

	undo_list& undo_stack_;

	replay_network_sender& replay_sender_;

	events::generic_event replay_error_;
};

#endif
