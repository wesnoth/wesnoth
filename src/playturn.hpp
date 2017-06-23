/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <string>                       // for string
#include "generic_event.hpp"            // for generic_event
#include "replay.hpp"

class config;  // lines 18-18
class playturn_network_adapter;

/**
	TODO: rename this class since it isn't that much related to turns.
*/
class turn_info
{
public:
	turn_info(replay_network_sender &network_sender, playturn_network_adapter &network_reader);

	~turn_info();



	enum PROCESS_DATA_RESULT
	{
		PROCESS_CONTINUE,
		PROCESS_RESTART_TURN,
		PROCESS_END_TURN,
		/** When the host uploaded the next scenario this is returned. */
		PROCESS_END_LINGER,
		/** When we couldn't process the network data because we found a dependent command, this should only happen if we were called playmp_controller::from handle_generic_event -> sync_network*/
		PROCESS_FOUND_DEPENDENT,
		/** We foudn a player action in the replay that caused the game to end*/
		PROCESS_END_LEVEL
	};

	PROCESS_DATA_RESULT sync_network();

	void send_data();

	//function which will process incoming network data received with playturn_network_adapter, and act on it.
	PROCESS_DATA_RESULT process_network_data(const config& cfg);

	//reads as much data from network_reader_ as possible and processed it.
	PROCESS_DATA_RESULT process_network_data_from_reader();

	events::generic_event& host_transfer() { return host_transfer_; }

	static PROCESS_DATA_RESULT replay_to_process_data_result(REPLAY_RETURN replayreturn);
private:
	static void change_side_controller(int side, const std::string& player);
	PROCESS_DATA_RESULT handle_turn(const config& t);

	void do_save();

	replay_network_sender& replay_sender_;

	events::generic_event host_transfer_;

	playturn_network_adapter& network_reader_;
};
