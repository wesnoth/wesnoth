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
#ifndef GAME_CONTROLLER_ABSTRACT_H_INCLUDED
#define GAME_CONTROLLER_ABSTRACT_H_INCLUDED
#include "commandline_options.hpp"
#include "editor/editor_main.hpp"
#include "scoped_resource.hpp"
#include "video.hpp"
#include <string>

class config;
class game_display;

class game_controller_abstract
{
public:
	game_controller_abstract(const commandline_options &cmdline_opts);
	virtual ~game_controller_abstract() {}

	game_display& disp();

	bool init_video();
	virtual bool init_config() = 0;
	bool init_language();
	bool init_joystick();
	virtual bool play_test() = 0;
	virtual bool play_multiplayer_mode() = 0;
	virtual bool play_screenshot_mode() = 0;

	virtual void reload_changed_game_config() = 0;

	virtual bool is_loading() const = 0;
	virtual void clear_loaded_game() = 0;
	virtual bool load_game() = 0;
	virtual void set_tutorial() = 0;

	virtual std::string jump_to_campaign_id() const = 0;
	virtual bool new_campaign() = 0;
	virtual bool goto_campaign() = 0;
	virtual bool goto_multiplayer() = 0;
	virtual bool goto_editor() = 0;

	virtual bool play_multiplayer() = 0;
	virtual bool change_language() = 0;

	virtual void show_preferences() = 0;

	enum RELOAD_GAME_DATA { RELOAD_DATA, NO_RELOAD_DATA };
	virtual void launch_game(RELOAD_GAME_DATA) = 0;
	virtual void play_replay() = 0;

	virtual editor::EXIT_STATUS start_editor() = 0;

	virtual const config& game_config() const = 0;
protected:
	const commandline_options& cmdline_opts_;

	util::scoped_ptr<game_display> disp_;

	CVideo video_;
};
#endif
