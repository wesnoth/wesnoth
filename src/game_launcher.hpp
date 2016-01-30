/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef GAME_LAUNCHER_H_INCLUDED
#define GAME_LAUNCHER_H_INCLUDED

#include "global.hpp"

#include "editor/editor_main.hpp"       // for EXIT_STATUS
#include "events.hpp"                   // for event_context
#include "font.hpp"                     // for manager
#include "game_errors.hpp"              // for load_game_exception, etc
#include "game_preferences.hpp"         // for manager
#include "hotkey/hotkey_manager.hpp"    // for manager
#include "image.hpp"                    // for manager
#include "saved_game.hpp"               // for saved_game
#include "scoped_resource.hpp"          // for scoped_ptr
#include "sound.hpp"                    // for music_thinker
#include "thread.hpp"                   // for manager

#include <string>                       // for string
#include <vector>                       // for vector

class commandline_options;
class config;
class CVideo;
class resize_monitor;

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

class game_launcher
{
public:
	game_launcher(const commandline_options& cmdline_opts, const char* appname);
	~game_launcher();

	CVideo& video() { return *video_; }

	bool init_video();
	bool init_language();
	bool init_joystick();
	bool init_lua_script();

	bool play_test();
	bool play_screenshot_mode();
	bool play_render_image_mode();
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

	const commandline_options & opts() const { return cmdline_opts_; }
private:
	game_launcher(const game_launcher&);
	void operator=(const game_launcher&);

	void mark_completed_campaigns(std::vector<config>& campaigns);

	editor::EXIT_STATUS start_editor(const std::string& filename);

	const commandline_options& cmdline_opts_;
	//Never null.
	boost::scoped_ptr<CVideo> video_;

	//this should get destroyed *after* the video, since we want
	//to clean up threads after the display disappears.
	const threading::manager thread_manager;

	const font::manager font_manager_;
	const preferences::manager prefs_manager_;
	const image::manager image_manager_;
	const events::event_context main_event_context_;
	const hotkey::manager hotkey_manager_;
	sound::music_thinker music_thinker_;
	//Never null.
	boost::scoped_ptr<resize_monitor> resize_monitor_;

	std::string test_scenario_;

	std::string screenshot_map_, screenshot_filename_;

	saved_game state_;
	bool play_replay_;

	std::string multiplayer_server_;
	bool jump_to_multiplayer_;
	jump_to_campaign_info jump_to_campaign_;

	bool jump_to_editor_;
};

#endif
