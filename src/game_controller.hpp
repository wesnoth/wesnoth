/* $Id$ */
/*
   Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
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

// class game_controller
// {
// public:
// 	game_controller(int argc, char** argv);
// 	~game_controller();
// 
// 	game_display& disp();
// 
// 	bool init_video();
// 	bool init_config(const bool force=false);
// 	bool init_language();
// 	bool play_test();
// 	bool play_multiplayer_mode();
// 	bool play_screenshot_mode();
// 
// 	void reload_changed_game_config();
// 
// 	bool is_loading() const;
// 	void clear_loaded_game() { game::load_game_exception::game.clear(); }
// 	bool load_game();
// 	void set_tutorial();
// 
// 	std::string jump_to_campaign_id() const;
// 	bool new_campaign();
// 	bool goto_campaign();
// 	bool goto_multiplayer();
// 	bool goto_editor();
// 
// 	bool play_multiplayer();
// 	bool change_language();
// 
// 	void show_preferences();
// 
// 	enum RELOAD_GAME_DATA { RELOAD_DATA, NO_RELOAD_DATA };
// 	void launch_game(RELOAD_GAME_DATA reload=RELOAD_DATA);
// 	void play_replay();
// 
// 	editor::EXIT_STATUS start_editor(const std::string& filename = "");
// 
// 	void start_wesnothd();
// 	const config& game_config() const { return game_config_; }
// 
// private:
// 	game_controller(const game_controller&);
// 	void operator=(const game_controller&);
// 
// 	void load_game_cfg(const bool force=false);
// 	void set_unit_data();
// 
// 	void mark_completed_campaigns(std::vector<config>& campaigns);
// 
// 	const int argc_;
// 	int arg_;
// 	const char* const * const argv_;
// 
// 	//this should get destroyed *after* the video, since we want
// 	//to clean up threads after the display disappears.
// 	const threading::manager thread_manager;
// 
// 	CVideo video_;
// 
// 	const font::manager font_manager_;
// 	const preferences::manager prefs_manager_;
// 	const image::manager image_manager_;
// 	const events::event_context main_event_context_;
// 	const hotkey::manager hotkey_manager_;
// 	sound::music_thinker music_thinker_;
// 	resize_monitor resize_monitor_;
// 	binary_paths_manager paths_manager_;
// 
// 	std::string test_scenario_;
// 
// 	bool test_mode_, multiplayer_mode_, no_gui_, screenshot_mode_;
// 	std::string screenshot_map_, screenshot_filename_;
// 	int force_bpp_;
// 
// 	config game_config_;
// 	preproc_map old_defines_map_;
// 
// 	util::scoped_ptr<game_display> disp_;
// 
// 	/// Stateful class taking over scenario-switching capabilities from the current game_controller and playsingle_controller. Currently only available when --new-syntax command line option is enabled.
// 	game_instance game_instance_;
// 	game_state state_;
// 
// 	std::string multiplayer_server_;
// 	bool jump_to_multiplayer_;
// 	jump_to_campaign_info jump_to_campaign_;
// 
// 	bool jump_to_editor_;
// 
// 	game_config::config_cache& cache_;
// };

#endif