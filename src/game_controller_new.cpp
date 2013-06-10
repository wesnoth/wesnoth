/*
   Copyright (C) 2011 - 2013 by Lukasz Dobrogowski <lukasz.dobrogowski@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "game_controller_new.hpp"
#include "game_display.hpp"

#include <iostream>

game_controller_new::game_controller_new(const commandline_options& cmdline_opts) :
	game_controller_abstract(cmdline_opts),
	main_config_()
{
}

game_controller_new::~game_controller_new()
{

}

bool game_controller_new::init_config()
{
	return true;
}

bool game_controller_new::play_test()
{
	return true;
}

bool game_controller_new::play_screenshot_mode()
{
	return true;
}

void game_controller_new::reload_changed_game_config()
{

}

bool game_controller_new::is_loading() const
{
	return true;
}

void game_controller_new::clear_loaded_game()
{

}

bool game_controller_new::load_game()
{
	return true;
}

void game_controller_new::set_tutorial()
{

}

std::string game_controller_new::jump_to_campaign_id() const
{
	return std::string();
}

bool game_controller_new::new_campaign()
{
	return true;
}

bool game_controller_new::goto_campaign()
{
	return true;
}

bool game_controller_new::goto_multiplayer()
{
	return true;
}

bool game_controller_new::goto_editor()
{
	return true;
}

bool game_controller_new::jump_to_editor() const
{
	return true;
}

bool game_controller_new::play_multiplayer()
{
	return true;
}

bool game_controller_new::play_multiplayer_commandline()
{
	return true;
}

bool game_controller_new::change_language()
{
	return true;
}

void game_controller_new::show_preferences()
{

}

void game_controller_new::launch_game ( game_controller_abstract::RELOAD_GAME_DATA /*reload*/ )
{

}

void game_controller_new::play_replay()
{

}

editor::EXIT_STATUS game_controller_new::start_editor()
{
	return editor::EXIT_NORMAL;
}

const config& game_controller_new::game_config() const
{
	return main_config_;
}
