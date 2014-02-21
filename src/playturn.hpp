/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef PLAYTURN_HPP_INCLUDED
#define PLAYTURN_HPP_INCLUDED

class config;
class replay_network_sender;

#include "generic_event.hpp"
#include "network.hpp"
#include "replay.hpp"

class turn_info
{
public:
	turn_info(unsigned team_num, replay_network_sender &network_sender);

	~turn_info();

	void sync_network();

	void send_data();

	enum PROCESS_DATA_RESULT {
		PROCESS_CONTINUE,
		PROCESS_RESTART_TURN,
		PROCESS_END_TURN,
		/** When the host uploaded the next scenario this is returned. */
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

	events::generic_event& host_transfer() { return host_transfer_; }
private:
	static void change_controller(const std::string& side, const std::string& controller);
	static void change_side_controller(const std::string& side, const std::string& player);

	void handle_turn(
		bool& turn_end,
		const config& t,
		const bool skip_replay,
		std::deque<config>& backlog);

	void do_save();

	unsigned int team_num_;

	replay_network_sender& replay_sender_;

	events::generic_event host_transfer_;

	replay replay_;
};

#endif
