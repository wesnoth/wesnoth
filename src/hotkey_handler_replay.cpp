/*
   Copyright (C) 2014 - 2015 by Chris Beck <render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "hotkey_handler_replay.hpp"

#include "hotkey/hotkey_command.hpp"
#include "hotkey/hotkey_item.hpp"
#include "network.hpp"		//for nconnections (to determine if we are in a networked game)
#include "play_controller.hpp"
#include "replay_controller.hpp"

replay_controller::hotkey_handler::hotkey_handler(replay_controller & pc, saved_game & sg)
	: play_controller::hotkey_handler(pc, sg)
	, replay_controller_(pc)
{}

replay_controller::hotkey_handler::~hotkey_handler(){}

void replay_controller::hotkey_handler::preferences(){
	play_controller::hotkey_handler::preferences();
	replay_controller_.update_gui(); //todo: why is this needed?
}

void replay_controller::hotkey_handler::show_statistics(){
	menu_handler_.show_statistics(gui()->playing_team()+1); //playing team instead of viewing team
}

bool replay_controller::hotkey_handler::can_execute_command(const hotkey::hotkey_command& cmd, int index) const
{
	hotkey::HOTKEY_COMMAND command = cmd.id;
	bool result = play_controller::hotkey_handler::can_execute_command(cmd,index);

	switch(command) {

	//commands we can always do
	case hotkey::HOTKEY_REPLAY_SHOW_EVERYTHING:
	case hotkey::HOTKEY_REPLAY_SHOW_EACH:
	case hotkey::HOTKEY_REPLAY_SHOW_TEAM1:
	case hotkey::HOTKEY_REPLAY_SKIP_ANIMATION:
	case hotkey::HOTKEY_SAVE_GAME:
	case hotkey::HOTKEY_SAVE_REPLAY:
	case hotkey::HOTKEY_CHAT_LOG:
		return true;

	case hotkey::HOTKEY_REPLAY_RESET:
		return events::commands_disabled <= 1;

	//commands we only can do before the end of the replay
	case hotkey::HOTKEY_REPLAY_STOP:
		return !replay_controller_.recorder_at_end();
	case hotkey::HOTKEY_REPLAY_PLAY:
	case hotkey::HOTKEY_REPLAY_NEXT_TURN:
	case hotkey::HOTKEY_REPLAY_NEXT_SIDE:
	case hotkey::HOTKEY_REPLAY_NEXT_MOVE:
		//we have one events_disabler when starting the replay_controller and a second when entering the synced context.
		return (events::commands_disabled <= 1 ) && !replay_controller_.recorder_at_end();
	default:
		return result;
	}
}
