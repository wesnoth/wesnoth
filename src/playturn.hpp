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
#ifndef PLAYTURN_HPP_INCLUDED
#define PLAYTURN_HPP_INCLUDED

class game_display;
class config;
class game_state;
class replay_network_sender;
class team;
class unit;

#include "global.hpp"

#include "actions.hpp"
#include "generic_event.hpp"
#include "network.hpp"

#include <map>
#include <vector>

class turn_info
{
public:
	turn_info(const game_data& gameinfo, game_state& state_of_game,
	          const gamestatus& status, game_display& gui, gamemap& map,
		  std::vector<team>& teams, unsigned int team_num, unit_map& units,
		  replay_network_sender& network_sender, undo_list& undo_stack);

	~turn_info();

	void sync_network();

	void send_data();

	enum PROCESS_DATA_RESULT { 
		PROCESS_CONTINUE, 
		PROCESS_RESTART_TURN, 
		PROCESS_END_TURN, 
		//! When the host uploaded the next scenario this is returned.
		PROCESS_END_LINGER        
		};

	//function which will process incoming network data, and act on it. If there is
	//more data than a single turn's worth, excess data will be placed into 'backlog'.
	//No more than one turn's worth of data will be placed into a single backlog item,
	//so it is safe to assume that backlog won't be touched if cfg is a member of a previous
	//backlog.
	//data will be forwarded to all peers other than 'from', unless 'from' is null, in
	//which case data will not be forwarded
	PROCESS_DATA_RESULT process_network_data(const config& cfg,network::connection from,std::deque<config>& backlog, bool skip_replay);

	events::generic_event& replay_error() { return replay_error_; }
	events::generic_event& host_transfer() { return host_transfer_; }
private:
	static void change_side_controller(const std::string& side, const std::string& player, bool own_side=false);

	const game_data& gameinfo_;
	game_state& state_of_game_;
	const gamestatus& status_;
	game_display& gui_;
	gamemap& map_;
	std::vector<team>& teams_;
	unsigned int team_num_;
	unit_map& units_;

	undo_list& undo_stack_;

	replay_network_sender& replay_sender_;

	events::generic_event replay_error_;
	events::generic_event host_transfer_;
};

#endif
