/* $Id$ */
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

#ifndef GAME_CONTROLLER_NEW_HPP_INCLUDED
#define GAME_CONTROLLER_NEW_HPP_INCLUDED
#include "game_controller_abstract.hpp"

#include "config.hpp"

class game_controller_new : public game_controller_abstract
{
public:
	game_controller_new(const commandline_options& cmdline_opts);
	~game_controller_new();

	bool init_config();
	bool play_test();
	bool play_screenshot_mode();

	void reload_changed_game_config();

	bool is_loading() const;
	void clear_loaded_game();
	bool load_game();
	void set_tutorial();

	std::string jump_to_campaign_id() const;
	bool new_campaign();
	bool goto_campaign();
	bool goto_multiplayer();
	bool goto_editor();

	bool play_multiplayer();
	bool play_multiplayer_commandline();
	bool change_language();

	void show_preferences();

	void launch_game(RELOAD_GAME_DATA reload=RELOAD_DATA);
	void play_replay();

	editor::EXIT_STATUS start_editor();

	const config& game_config() const;

private:
	config main_config_;

};

#endif
