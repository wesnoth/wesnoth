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

#pragma once

#include "editor/editor_main.hpp"    // for EXIT_STATUS
#include "events.hpp"                // for event_context
#include "font/font_config.hpp"      // for manager
#include "game_end_exceptions.hpp"   // for LEVEL_RESULT, etc
#include "hotkey/hotkey_manager.hpp" // for manager
#include "picture.hpp"               // for manager
#include "saved_game.hpp"            // for saved_game
#include "savegame.hpp"              // for clean_saves, etc
#include "sound.hpp"                 // for music_thinker
#include "utils/optional_fwd.hpp"

#include <string>                       // for string
#include <vector>                       // for vector

class commandline_options;

struct jump_to_campaign_info
{
	/** Whether the game should immediately start a campaign. */
	bool jump = false;

	/** Whether the story screen should be skipped. */
	bool skip_story = false;

	/** The difficulty at which to launch the campaign. */
	int difficulty = -1;

	/** The ID of the campaign to launch. */
	std::string campaign_id = "";

	/** The ID of the scenario within the campaign to jump to. */
	std::string scenario_id = "";
};

class game_launcher
{
public:
	game_launcher(const commandline_options& cmdline_opts);
	~game_launcher();

	enum class mp_mode { CONNECT, HOST, LOCAL };
	enum class reload_mode { RELOAD_DATA, NO_RELOAD_DATA };

	/**
	 * Status code after running a unit test, should match the run_wml_tests
	 * script and the documentation for the --unit_test command-line option.
	 */
	enum class unit_test_result : int {
		TEST_PASS = 0,
		TEST_FAIL = 1,
		// 2 is reserved for timeouts
		TEST_FAIL_LOADING_REPLAY = 3,
		TEST_FAIL_PLAYING_REPLAY = 4,
		//TEST_FAIL_BROKE_STRICT = 5,
		TEST_FAIL_WML_EXCEPTION = 6,
		TEST_FAIL_BY_DEFEAT = 7,
		TEST_PASS_BY_VICTORY = 8,
		BROKE_STRICT_TEST_PASS = 9,
		BROKE_STRICT_TEST_FAIL = 10,
		BROKE_STRICT_TEST_FAIL_BY_DEFEAT = 11,
		BROKE_STRICT_TEST_PASS_BY_VICTORY = 12,
	};

	bool init_video();
	bool init_language();
	bool init_lua_script();

	bool play_test();
	bool play_screenshot_mode();
	bool play_render_image_mode();
	/** Runs unit tests specified on the command line */
	unit_test_result unit_test();

	bool has_load_data() const;
	bool load_game();
	void set_test(const std::string& id);

	/** Return the ID of the campaign to jump to (skipping the main menu). */
	std::string jump_to_campaign_id() const;
	bool new_campaign();
	bool goto_campaign();
	bool goto_multiplayer();
	bool goto_editor();

	void select_mp_server(const std::string& server) { multiplayer_server_ = server; }
	bool play_multiplayer(mp_mode mode);
	bool play_multiplayer_commandline();
	bool play_campaign();
	bool change_language();

	void launch_game(reload_mode reload = reload_mode::RELOAD_DATA);
	void play_replay();

	editor::EXIT_STATUS start_editor() { return start_editor(""); }

	const commandline_options & opts() const { return cmdline_opts_; }

private:
	game_launcher(const game_launcher&) = delete;
	game_launcher& operator=(const game_launcher&) = delete;

	void clear_loaded_game();
	void start_wesnothd();

	editor::EXIT_STATUS start_editor(const std::string& filename);
	unit_test_result pass_victory_or_defeat(level_result::type res);

	/**
	 * Internal to the implementation of unit_test(). If a single instance of
	 * Wesnoth is running multiple unit tests, this gets called once per test.
	 */
	unit_test_result single_unit_test();

	const commandline_options& cmdline_opts_;
	bool start_in_fullscreen_ = false;

	font::manager font_manager_;
	const image::manager image_manager_;
	const events::event_context main_event_context_;
	const hotkey::manager hotkey_manager_;
	sound::music_thinker music_thinker_;
	sound::music_muter music_muter_;

	std::vector<std::string> test_scenarios_;

	std::string screenshot_map_, screenshot_filename_;

	saved_game state_;
	bool play_replay_;

	std::string multiplayer_server_;
	bool jump_to_multiplayer_;
	jump_to_campaign_info jump_to_campaign_;

	bool jump_to_editor_;
	utils::optional<savegame::load_game_metadata> load_data_;
};
