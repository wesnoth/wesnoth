/*
	Copyright (C) 2014 - 2024
	by Chris Beck <render787@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "hotkey/hotkey_handler_mp.hpp"

#include "hotkey/hotkey_command.hpp"
#include "playsingle_controller.hpp"
#include "playmp_controller.hpp"

playmp_controller::hotkey_handler::hotkey_handler(playmp_controller & pc, saved_game & sg)
	: playsingle_controller::hotkey_handler(pc, sg)
	, playmp_controller_(pc)
{}

playmp_controller::hotkey_handler::~hotkey_handler(){}

void playmp_controller::hotkey_handler::whisper(){
	menu_handler_.whisper();
}

void playmp_controller::hotkey_handler::shout(){
	menu_handler_.shout();
}

void playmp_controller::hotkey_handler::start_network(){
	playmp_controller_.start_network();
}

void playmp_controller::hotkey_handler::stop_network(){
	playmp_controller_.stop_network();
}

bool playmp_controller::hotkey_handler::can_execute_command(const hotkey::ui_command& cmd) const
{
	hotkey::HOTKEY_COMMAND command = cmd.hotkey_command;
	bool res = true;
	switch (command){
		case hotkey::HOTKEY_ENDTURN:
			if  (linger())
			{
				return playmp_controller_.is_host() || !gamestate().has_next_scenario();
			}
			else
			{
				return playsingle_controller::hotkey_handler::can_execute_command(cmd);
			}
		case hotkey::HOTKEY_SPEAK:
		case hotkey::HOTKEY_SPEAK_ALLY:
		case hotkey::HOTKEY_SPEAK_ALL:
		case hotkey::HOTKEY_CHAT_LOG:
			res = true;
			break;
		case hotkey::HOTKEY_START_NETWORK:
			res = is_observer() && playmp_controller_.network_processing_stopped_;
			break;
		case hotkey::HOTKEY_STOP_NETWORK:
			res = is_observer() && !playmp_controller_.network_processing_stopped_;
			break;
	    default:
			return playsingle_controller::hotkey_handler::can_execute_command(cmd);
	}
	return res;
}
