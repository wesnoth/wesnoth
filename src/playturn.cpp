/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "playturn.hpp"

#include "actions/undo.hpp"             // for undo_list
#include "chat_events.hpp"              // for chat_handler, etc
#include "config.hpp"                   // for config, etc
#include "display_chat_manager.hpp"	// for add_chat_message, add_observer, etc
#include "formula/string_utils.hpp"     // for VGETTEXT
#include "game_board.hpp"               // for game_board
#include "game_display.hpp"             // for game_display
#include "game_end_exceptions.hpp"      // for end_level_exception, etc
#include "gettext.hpp"                  // for _
#include "gui/dialogs/simple_item_selector.hpp"
#include "log.hpp"                      // for LOG_STREAM, logger, etc
#include "map/label.hpp"
#include "play_controller.hpp"          // for play_controller
#include "playturn_network_adapter.hpp"  // for playturn_network_adapter
#include "preferences/general.hpp"              // for message_bell
#include "replay.hpp"                   // for replay, recorder, do_replay, etc
#include "resources.hpp"                // for gameboard, screen, etc
#include "serialization/string_utils.hpp"  // for string_map
#include "synced_context.hpp"
#include "team.hpp"                     // for team, team::CONTROLLER::AI, etc
#include "wesnothd_connection_error.hpp"
#include "whiteboard/manager.hpp"       // for manager

#include <cassert>                      // for assert
#include <ctime>                        // for time
#include <vector>                       // for vector

static lg::log_domain log_network("network");
#define ERR_NW LOG_STREAM(err, log_network)

turn_info::turn_info(replay_network_sender &replay_sender,playturn_network_adapter &network_reader) :
	replay_sender_(replay_sender),
	host_transfer_("host_transfer"),
	network_reader_(network_reader)
{
}

turn_info::~turn_info()
{
}

