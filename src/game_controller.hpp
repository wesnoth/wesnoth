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
#ifndef GAME_CONTROLLER_H_INCLUDED
#define GAME_CONTROLLER_H_INCLUDED

#include "commandline_options.hpp"
#include "editor/editor_main.hpp"
#include "gamestatus.hpp"
#include "game_config_manager.hpp"
#include "game_display.hpp"
#include "game_preferences.hpp"
#include "hotkey/hotkey_manager.hpp"
#include "resources.hpp"
#include "sound.hpp"
#include "thread.hpp"

struct jump_to_campaign_info
{
public:
	jump_to_campaign_info(bool jump,int difficulty, std::string campaign_id,std::string scenario_id)
		: jump_(jump)
		, difficulty_(difficulty)
		, campaign_id_(campaign_id)
		, scenario_id_(scenario_id)
	{
	}
	bool jump_;
	int difficulty_;
	std::string campaign_id_,scenario_id_;
};

class game_controller
{
public:
	game_controller(const commandline_options& cmdline_opts, const char* appname);
	~game_controller();

	game_display& disp();

	bool init_video();
	bool init_language();
	bool init_joystick();

	bool play_test();
	bool play_screenshot_mode();
	int unit_test();

	bool is_loading() const;
	void clear_loaded_game() { game::load_game_exception::game.clear(); }
	bool load_game();
	void set_tutorial();

	std::string jump_to_campaign_id() const;
	bool new_campaign();
	bool goto_campaign();
	bool goto_multiplayer();
	bool goto_editor();

	bool jump_to_editor() const { return jump_to_editor_; }

	bool play_multiplayer();
	bool play_multiplayer_commandline();
	bool change_language();

	void show_preferences();

	enum RELOAD_GAME_DATA { RELOAD_DATA, NO_RELOAD_DATA };
	void launch_game(RELOAD_GAME_DATA reload=RELOAD_DATA);
	void play_replay();

	editor::EXIT_STATUS start_editor() { return start_editor(""); }

	void start_wesnothd();
private:
	game_controller(const game_controller&);
	void operator=(const game_controller&);

	void mark_completed_campaigns(std::vector<config>& campaigns);

	editor::EXIT_STATUS start_editor(const std::string& filename);

	const commandline_options& cmdline_opts_;
	util::scoped_ptr<game_display> disp_;
	CVideo video_;

	//this should get destroyed *after* the video, since we want
	//to clean up threads after the display disappears.
	const threading::manager thread_manager;

	const font::manager font_manager_;
	const preferences::manager prefs_manager_;
	const image::manager image_manager_;
	const events::event_context main_event_context_;
	const hotkey::manager hotkey_manager_;
	sound::music_thinker music_thinker_;
	resize_monitor resize_monitor_;

	std::string test_scenario_;

	std::string screenshot_map_, screenshot_filename_;

	game_state state_;

	std::string multiplayer_server_;
	bool jump_to_multiplayer_;
	jump_to_campaign_info jump_to_campaign_;

	bool jump_to_editor_;
};

#endif
