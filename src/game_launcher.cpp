/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "game_launcher.hpp"
#include "game_errors.hpp"

#include "ai/manager.hpp"          // for manager
#include "commandline_options.hpp" // for commandline_options
#include "config.hpp"              // for config, etc
#include "cursor.hpp"              // for set, CURSOR_TYPE::NORMAL
#include "exceptions.hpp"          // for error
#include "filesystem.hpp"          // for get_user_config_dir, etc
#include "game_classification.hpp" // for game_classification, etc
#include "game_config.hpp"         // for path, no_delay, revision, etc
#include "game_config_manager.hpp" // for game_config_manager
#include "game_initialization/create_engine.hpp"
#include "game_initialization/multiplayer.hpp"  // for start_client, etc
#include "game_initialization/playcampaign.hpp" // for play_game, etc
#include "game_initialization/singleplayer.hpp" // for sp_create_mode
#include "generators/map_generator.hpp"         // for mapgen_exception
#include "gettext.hpp"                          // for _
#include "gui/dialogs/language_selection.hpp"   // for language_selection
#include "gui/dialogs/loading_screen.hpp"
#include "gui/dialogs/message.hpp" // for show error message
#include "gui/dialogs/multiplayer/mp_connect.hpp"
#include "gui/dialogs/multiplayer/mp_host_game_prompt.hpp" // for host game prompt
#include "gui/dialogs/multiplayer/mp_method_selection.hpp"
#include "gui/dialogs/title_screen.hpp"      // for show_debug_clock_button
#include "gui/dialogs/transient_message.hpp" // for show_transient_message
#include "gui/widgets/retval.hpp"            // for window, etc
#include "gui/widgets/settings.hpp"          // for new_widgets
#include "language.hpp"                      // for language_def, etc
#include "log.hpp"                           // for LOG_STREAM, logger, general, etc
#include "map/exception.hpp"
#include "preferences/credentials.hpp"
#include "preferences/display.hpp"
#include "preferences/general.hpp" // for disable_preferences_save, etc
#include "save_index.hpp"
#include "scripting/application_lua_kernel.hpp"
#include "sdl/surface.hpp"                // for surface
#include "serialization/compression.hpp"  // for format::NONE
#include "serialization/string_utils.hpp" // for split
#include "statistics.hpp"
#include "tstring.hpp"       // for operator==, operator!=
#include "video.hpp"         // for CVideo
#include "wesnothd_connection_error.hpp"
#include "wml_exception.hpp" // for wml_exception

#include <algorithm> // for copy, max, min, stable_sort
#include <cstdlib>   // for system
#include <iostream>  // for operator<<, basic_ostream, etc
#include <new>
#include <utility> // for pair

#include <SDL2/SDL.h> // for SDL_INIT_JOYSTICK, etc

#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
#include "gui/widgets/debug.hpp"
#endif

// For wesnothd launch code.
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#endif // _WIN32

struct incorrect_map_format_error;

static lg::log_domain log_config("config");
#define ERR_CONFIG LOG_STREAM(err, log_config)
#define WRN_CONFIG LOG_STREAM(warn, log_config)
#define LOG_CONFIG LOG_STREAM(info, log_config)

#define LOG_GENERAL LOG_STREAM(info, lg::general())
#define WRN_GENERAL LOG_STREAM(warn, lg::general())
#define DBG_GENERAL LOG_STREAM(debug, lg::general())

static lg::log_domain log_mp_create("mp/create");
#define DBG_MP LOG_STREAM(debug, log_mp_create)

static lg::log_domain log_network("network");
#define ERR_NET LOG_STREAM(err, log_network)

static lg::log_domain log_enginerefac("enginerefac");
#define LOG_RG LOG_STREAM(info, log_enginerefac)

game_launcher::game_launcher(const commandline_options& cmdline_opts)
	: cmdline_opts_(cmdline_opts)
	, video_(new CVideo())
	, font_manager_()
	, prefs_manager_()
	, image_manager_()
	, main_event_context_()
	, hotkey_manager_()
	, music_thinker_()
	, music_muter_()
	, test_scenarios_{"test"}
	, screenshot_map_()
	, screenshot_filename_()
	, state_()
	, play_replay_(false)
	, multiplayer_server_()
	, jump_to_multiplayer_(false)
	, jump_to_campaign_{}
	, jump_to_editor_(false)
	, load_data_()
{
	bool no_music = false;
	bool no_sound = false;

	// The path can be hardcoded and it might be a relative path.
	if(!game_config::path.empty() &&
#ifdef _WIN32
		// use c_str to ensure that index 1 points to valid element since c_str() returns null-terminated string
		game_config::path.c_str()[1] != ':'
#else
		game_config::path[0] != '/'
#endif
	)
	{
		game_config::path = filesystem::get_cwd() + '/' + game_config::path;
		// font_manager_.update_font_path()
		// To update the font path, destroy and recreate the manager
		font_manager_.~manager();
		new (&font_manager_) font::manager();
	}

	if(cmdline_opts_.core_id) {
		preferences::set_core_id(*cmdline_opts_.core_id);
	}
	if(cmdline_opts_.campaign) {
		jump_to_campaign_.jump = true;
		jump_to_campaign_.campaign_id = *cmdline_opts_.campaign;
		std::cerr << "selected campaign id: [" << jump_to_campaign_.campaign_id << "]\n";

		if(cmdline_opts_.campaign_difficulty) {
			jump_to_campaign_.difficulty = *cmdline_opts_.campaign_difficulty;
			std::cerr << "selected difficulty: [" << jump_to_campaign_.difficulty << "]\n";
		} else {
			jump_to_campaign_.difficulty = -1; // let the user choose the difficulty
		}

		if(cmdline_opts_.campaign_scenario) {
			jump_to_campaign_.scenario_id = *cmdline_opts_.campaign_scenario;
			std::cerr << "selected scenario id: [" << jump_to_campaign_.scenario_id << "]\n";
		}

		if(cmdline_opts_.campaign_skip_story) {
			jump_to_campaign_.skip_story = true;
		}
	}
	if(cmdline_opts_.clock)
		gui2::dialogs::show_debug_clock_button = true;
	if(cmdline_opts_.debug) {
		game_config::set_debug(true);
		game_config::mp_debug = true;
	}
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	if(cmdline_opts_.debug_dot_domain)
		gui2::debug_layout_graph::set_domain(*cmdline_opts_.debug_dot_domain);
	if(cmdline_opts_.debug_dot_level)
		gui2::debug_layout_graph::set_level(*cmdline_opts_.debug_dot_level);
#endif
	if(cmdline_opts_.editor) {
		jump_to_editor_ = true;
		if(!cmdline_opts_.editor->empty()) {
			load_data_ = savegame::load_game_metadata{
				savegame::save_index_class::default_saves_dir(), *cmdline_opts_.editor};
		}
	}
	if(cmdline_opts_.fps)
		preferences::set_show_fps(true);
	if(cmdline_opts_.fullscreen)
		video_->set_fullscreen(true);
	if(cmdline_opts_.load)
		load_data_ = savegame::load_game_metadata{
			savegame::save_index_class::default_saves_dir(), *cmdline_opts_.load};
	if(cmdline_opts_.max_fps) {
		int fps = std::clamp(*cmdline_opts_.max_fps, 1, 1000);
		fps = 1000 / fps;
		// increase the delay to avoid going above the maximum
		if(1000 % fps != 0) {
			++fps;
		}
		preferences::set_draw_delay(fps);
	}
	if(cmdline_opts_.nogui || cmdline_opts_.headless_unit_test) {
		no_sound = true;
		preferences::disable_preferences_save();
	}
	if(cmdline_opts_.new_widgets)
		gui2::new_widgets = true;
	if(cmdline_opts_.nodelay)
		game_config::no_delay = true;
	if(cmdline_opts_.nomusic)
		no_music = true;
	if(cmdline_opts_.nosound)
		no_sound = true;
	if(cmdline_opts_.resolution) {
		const int xres = std::get<0>(*cmdline_opts_.resolution);
		const int yres = std::get<1>(*cmdline_opts_.resolution);
		if(xres > 0 && yres > 0) {
			preferences::_set_resolution(point(xres, yres));
			preferences::_set_maximized(false);
		}
	}
	if(cmdline_opts_.screenshot) {
		// TODO it could be simplified to use cmdline_opts_ directly if there is no other way to enter screenshot mode
		screenshot_map_ = *cmdline_opts_.screenshot_map_file;
		screenshot_filename_ = *cmdline_opts_.screenshot_output_file;
		no_sound = true;
		preferences::disable_preferences_save();
	}
	if (cmdline_opts_.server){
		jump_to_multiplayer_ = true;
		// Do we have any server specified ?
		if(!cmdline_opts_.server->empty()) {
			multiplayer_server_ = *cmdline_opts_.server;
		} else {
			// Pick the first server in config
			if(game_config::server_list.size() > 0) {
				multiplayer_server_ = preferences::network_host();
			} else {
				multiplayer_server_ = "";
			}
		}
		if(cmdline_opts_.username) {
			preferences::disable_preferences_save();
			preferences::set_login(*cmdline_opts_.username);
			if(cmdline_opts_.password) {
				preferences::disable_preferences_save();
				preferences::set_password(*cmdline_opts.server, *cmdline_opts.username, *cmdline_opts_.password);
			}
		}
	}
	if(cmdline_opts_.test) {
		if(!cmdline_opts_.test->empty()) {
			test_scenarios_ = {*cmdline_opts_.test};
		}
	}
	if(!cmdline_opts_.unit_test.empty()) {
		test_scenarios_ = cmdline_opts_.unit_test;
	}
	if(cmdline_opts_.windowed)
		video_->set_fullscreen(false);
	if(cmdline_opts_.with_replay && load_data_)
		load_data_->show_replay = true;
	if(cmdline_opts_.translation_percent)
		set_min_translation_percent(*cmdline_opts_.translation_percent);

	if(!cmdline_opts.nobanner) {
		std::cerr
			<< "\nData directory:               " << filesystem::sanitize_path(game_config::path)
			<< "\nUser configuration directory: " << filesystem::sanitize_path(filesystem::get_user_config_dir())
			<< "\nUser data directory:          " << filesystem::sanitize_path(filesystem::get_user_data_dir())
			<< "\nCache directory:              " << filesystem::sanitize_path(filesystem::get_cache_dir())
			<< '\n';
		std::cerr << '\n';
	}

	// disable sound in nosound mode, or when sound engine failed to initialize
	if(no_sound || ((preferences::sound_on() || preferences::music_on() ||
	                  preferences::turn_bell() || preferences::UI_sound_on()) &&
	                 !sound::init_sound())) {
		preferences::set_sound(false);
		preferences::set_music(false);
		preferences::set_turn_bell(false);
		preferences::set_UI_sound(false);
	} else if(no_music) { // else disable the music in nomusic mode
		preferences::set_music(false);
	}
}

bool game_launcher::init_language()
{
	if(!::load_language_list()) {
		return false;
	}

	language_def locale;
	if(cmdline_opts_.language) {
		std::vector<language_def> langs = get_languages();
		for(const language_def& def : langs) {
			if(def.localename == *cmdline_opts_.language) {
				locale = def;
				break;
			}
		}
		if(locale.localename.empty()) {
			std::cerr << "Language symbol '" << *cmdline_opts_.language << "' not found.\n";
			return false;
		}
	} else {
		locale = get_locale();
	}
	::set_language(locale);

	return true;
}

bool game_launcher::init_video()
{
	// Handle special commandline launch flags
	if(cmdline_opts_.nogui || cmdline_opts_.headless_unit_test) {
		if(!(cmdline_opts_.multiplayer || cmdline_opts_.screenshot || cmdline_opts_.plugin_file || cmdline_opts_.headless_unit_test)) {
			std::cerr << "--nogui flag is only valid with --multiplayer or --screenshot or --plugin flags\n";
			return false;
		}
		video_->make_fake();
		game_config::no_delay = true;
		return true;
	}

	// Initialize a new window
	video_->init_window();

	// Set window title and icon
	video_->set_window_title(game_config::get_default_title_string());

#if !(defined(__APPLE__))
	surface icon(image::get_image("icons/icon-game.png", image::UNSCALED));
	if(icon != nullptr) {
		video_->set_window_icon(icon);
	}
#endif
	return true;
}

bool game_launcher::init_lua_script()
{
	bool error = false;

	if(!cmdline_opts_.nobanner) std::cerr << "Checking lua scripts... ";

	if(cmdline_opts_.script_unsafe_mode) {
		// load the "package" package, so that scripts can get what packages they want
		plugins_manager::get()->get_kernel_base()->load_package();
	}

	// get the application lua kernel, load and execute script file, if script file is present
	if(cmdline_opts_.script_file) {
		filesystem::scoped_istream sf = filesystem::istream_file(*cmdline_opts_.script_file);

		if(!sf->fail()) {
			/* Cancel all "jumps" to editor / campaign / multiplayer */
			jump_to_multiplayer_ = false;
			jump_to_editor_ = false;
			jump_to_campaign_.jump = false;

			std::string full_script((std::istreambuf_iterator<char>(*sf)), std::istreambuf_iterator<char>());

			std::cerr << "\nRunning lua script: " << *cmdline_opts_.script_file << std::endl;

			plugins_manager::get()->get_kernel_base()->run(full_script.c_str(), *cmdline_opts_.script_file);
		} else {
			std::cerr << "Encountered failure when opening script '" << *cmdline_opts_.script_file << "'\n";
			error = true;
		}
	}

	if(cmdline_opts_.plugin_file) {
		std::string filename = *cmdline_opts_.plugin_file;

		std::cerr << "Loading a plugin file'" << filename << "'...\n";

		filesystem::scoped_istream sf = filesystem::istream_file(filename);

		try {
			if(sf->fail()) {
				throw std::runtime_error("failed to open plugin file");
			}

			/* Cancel all "jumps" to editor / campaign / multiplayer */
			jump_to_multiplayer_ = false;
			jump_to_editor_ = false;
			jump_to_campaign_.jump = false;

			std::string full_plugin((std::istreambuf_iterator<char>(*sf)), std::istreambuf_iterator<char>());

			plugins_manager& pm = *plugins_manager::get();

			std::size_t i = pm.add_plugin(filename, full_plugin);

			for(std::size_t j = 0; j < pm.size(); ++j) {
				std::cerr << j << ": " << pm.get_name(j) << " -- " << pm.get_detailed_status(j) << std::endl;
			}

			std::cerr << "Starting a plugin...\n";
			pm.start_plugin(i);

			for(std::size_t j = 0; j < pm.size(); ++j) {
				std::cerr << j << ": " << pm.get_name(j) << " -- " << pm.get_detailed_status(j) << std::endl;
			}

			plugins_context pc("init");

			for(std::size_t repeat = 0; repeat < 5; ++repeat) {
				std::cerr << "Playing a slice...\n";
				pc.play_slice();

				for(std::size_t j = 0; j < pm.size(); ++j) {
					std::cerr << j << ": " << pm.get_name(j) << " -- " << pm.get_detailed_status(j) << std::endl;
				}
			}

			return true;
		} catch(const std::exception& e) {
			gui2::show_error_message(std::string("When loading a plugin, error:\n") + e.what());
			error = true;
		}
	}

	if(!error && !cmdline_opts_.nobanner) {
		std::cerr << "ok\n";
	}

	return !error;
}

void game_launcher::set_test(const std::string& id)
{
	state_.clear();
	state_.classification().campaign_type = game_classification::CAMPAIGN_TYPE::TEST;
	state_.classification().campaign_define = "TEST";
	state_.classification().era_id = "era_default";

	state_.set_carryover_sides_start(config{"next_scenario", id});
}

bool game_launcher::play_test()
{
	// This first_time variable was added in 70f3c80a3e2 so that using the GUI
	// menu to load a game works. That seems to have edge-cases, for example if
	// you try to load a game a second time then Wesnoth exits.
	static bool first_time = true;

	if(!cmdline_opts_.test) {
		return true;
	}

	if(!first_time) {
		return false;
	}

	first_time = false;

	if(test_scenarios_.size() == 0) {
		// shouldn't happen, as test_scenarios_ is initialised to {"test"}
		std::cerr << "Error in the test handling code" << std::endl;
		return false;
	}

	if(test_scenarios_.size() > 1) {
		std::cerr << "You can't run more than one unit test in interactive mode" << std::endl;
	}

	set_test(test_scenarios_.at(0));

	game_config_manager::get()->load_game_config_for_game(state_.classification(), state_.get_scenario_id());

	try {
		campaign_controller ccontroller(state_);
		ccontroller.play_game();
	} catch(savegame::load_game_exception& e) {
		load_data_ = std::move(e.data_);
		return true;
	}

	return false;
}

/**
 * Runs unit tests specified on the command line.
 *
 * If multiple unit tests were specified, then this will stop at the first test
 * which returns a non-zero status.
 */
// Same as play_test except that we return the results of play_game.
// \todo "same ... except" ... and many other changes, such as testing the replay
game_launcher::unit_test_result game_launcher::unit_test()
{
	// There's no copy of play_test's first_time variable. That seems to be for handling
	// the player loading a game via the GUI, which makes no sense in a non-interactive test.
	if(cmdline_opts_.unit_test.empty()) {
		return unit_test_result::TEST_FAIL;
	}

	auto ret = unit_test_result::TEST_FAIL; // will only be returned if no test is run
	for(const auto& scenario : test_scenarios_) {
		set_test(scenario);
		ret = single_unit_test();
		const char* describe_result;
		switch(ret) {
		case unit_test_result::TEST_PASS:
			describe_result = "PASS TEST";
			break;
		case unit_test_result::TEST_FAIL_LOADING_REPLAY:
			describe_result = "FAIL TEST (INVALID REPLAY)";
			break;
		case unit_test_result::TEST_FAIL_PLAYING_REPLAY:
			describe_result = "FAIL TEST (ERRORED REPLAY)";
			break;
		case unit_test_result::TEST_FAIL_BROKE_STRICT:
			describe_result = "FAIL TEST (BROKE STRICT)";
			break;
		case unit_test_result::TEST_FAIL_WML_EXCEPTION:
			describe_result = "FAIL TEST (WML EXCEPTION)";
			break;
		case unit_test_result::TEST_FAIL_BY_DEFEAT:
			describe_result = "FAIL TEST (DEFEAT)";
			break;
		case unit_test_result::TEST_PASS_BY_VICTORY:
			describe_result = "PASS TEST (VICTORY)";
			break;
		default:
			describe_result = "FAIL TEST";
			break;
		}

		std::cerr << describe_result << ": " << scenario << std::endl;
		if(ret != unit_test_result::TEST_PASS) {
			break;
		}
	}

	return ret;
}

game_launcher::unit_test_result game_launcher::single_unit_test()
{
	game_config_manager::get()->load_game_config_for_game(state_.classification(), state_.get_scenario_id());

	LEVEL_RESULT game_res = LEVEL_RESULT::TEST_FAIL;
	try {
		campaign_controller ccontroller(state_, true);
		game_res = ccontroller.play_game();
		// TODO: How to handle the case where a unit test scenario ends without an explicit {SUCCEED} or {FAIL}?
		// ex: check_victory_never_ai_fail results in victory by killing one side's leaders
		if(game_res == LEVEL_RESULT::TEST_FAIL) {
			return unit_test_result::TEST_FAIL;
		}
		if(lg::broke_strict()) {
			return unit_test_result::TEST_FAIL_BROKE_STRICT;
		}
	} catch(const wml_exception& e) {
		std::cerr << "Caught WML Exception:" << e.dev_message << std::endl;
		return unit_test_result::TEST_FAIL_WML_EXCEPTION;
	}

	savegame::clean_saves(state_.classification().label);

	if(cmdline_opts_.noreplaycheck) {
		return pass_victory_or_defeat(game_res);
	}

	savegame::replay_savegame save(state_, compression::NONE);
	save.save_game_automatic(false, "unit_test_replay");

	load_data_ = savegame::load_game_metadata{
		savegame::save_index_class::default_saves_dir(), save.filename(), "", true, true, false};

	if(!load_game()) {
		std::cerr << "Failed to load the replay!" << std::endl;
		return unit_test_result::TEST_FAIL_LOADING_REPLAY; // failed to load replay
	}

	try {
		campaign_controller ccontroller(state_, true);
		ccontroller.play_replay();
		if(lg::broke_strict()) {
			std::cerr << "Observed failure on replay" << std::endl;
			return unit_test_result::TEST_FAIL_PLAYING_REPLAY;
		}
	} catch(const wml_exception& e) {
		std::cerr << "WML Exception while playing replay: " << e.dev_message << std::endl;
		return unit_test_result::TEST_FAIL_PLAYING_REPLAY;
	}

	return pass_victory_or_defeat(game_res);
}

game_launcher::unit_test_result game_launcher::pass_victory_or_defeat(LEVEL_RESULT res)
{
	if(res == LEVEL_RESULT::DEFEAT) {
		return unit_test_result::TEST_FAIL_BY_DEFEAT;
	} else if(res == LEVEL_RESULT::VICTORY) {
		return unit_test_result::TEST_PASS_BY_VICTORY;
	}

	return unit_test_result::TEST_PASS;
}

bool game_launcher::play_screenshot_mode()
{
	if(!cmdline_opts_.screenshot) {
		return true;
	}

	game_config_manager::get()->load_game_config_for_editor();

	::init_textdomains(game_config_manager::get()->game_config());

	editor::start(screenshot_map_, true, screenshot_filename_);
	return false;
}

bool game_launcher::play_render_image_mode()
{
	if(!cmdline_opts_.render_image) {
		return true;
	}

	state_.classification().campaign_type = game_classification::CAMPAIGN_TYPE::MULTIPLAYER;
	DBG_GENERAL << "Current campaign type: " << state_.classification().campaign_type << std::endl;

	try {
		game_config_manager::get()->load_game_config_for_game(state_.classification(), state_.get_scenario_id());
	} catch(const config::error& e) {
		std::cerr << "Error loading game config: " << e.what() << std::endl;
		return false;
	}

	// A default output filename
	std::string outfile = "wesnoth_image.bmp";

	// If a output path was given as an argument, use that instead
	if(cmdline_opts_.render_image_dst) {
		outfile = *cmdline_opts_.render_image_dst;
	}

	if(image::save_image(*cmdline_opts_.render_image, outfile) != image::save_result::success) {
		exit(1);
	}

	return false;
}

bool game_launcher::has_load_data() const
{
	return load_data_.has_value();
}

bool game_launcher::load_game()
{
	assert(game_config_manager::get());

	DBG_GENERAL << "Current campaign type: " << state_.classification().campaign_type << std::endl;

	savegame::loadgame load(savegame::save_index_class::default_saves_dir(), state_);
	if(load_data_) {
		load.data() = std::move(*load_data_);
		clear_loaded_game();
	}

	try {
		if(!load.load_game()) {
			return false;
		}

		load.set_gamestate();
		try {
			game_config_manager::get()->load_game_config_for_game(state_.classification(), state_.get_scenario_id());
		} catch(const config::error&) {
			return false;
		}

	} catch(const config::error& e) {
		if(e.message.empty()) {
			gui2::show_error_message(_("The file you have tried to load is corrupt"));
		} else {
			gui2::show_error_message(_("The file you have tried to load is corrupt: '") + e.message + '\'');
		}

		return false;
	} catch(const wml_exception& e) {
		e.show();
		return false;
	} catch(const filesystem::io_exception& e) {
		if(e.message.empty()) {
			gui2::show_error_message(_("File I/O Error while reading the game"));
		} else {
			gui2::show_error_message(_("File I/O Error while reading the game: '") + e.message + '\'');
		}

		return false;
	} catch(const game::error& e) {
		if(e.message.empty()) {
			gui2::show_error_message(_("The file you have tried to load is corrupt"));
		} else {
			gui2::show_error_message(_("The file you have tried to load is corrupt: '") + e.message + '\'');
		}

		return false;
	}

	play_replay_ = load.data().show_replay;
	LOG_CONFIG << "is middle game savefile: " << (state_.is_mid_game_save() ? "yes" : "no") << "\n";
	LOG_CONFIG << "show replay: " << (play_replay_ ? "yes" : "no") << "\n";
	// in case load.data().show_replay && !state_.is_mid_game_save()
	// there won't be any turns to replay, but the
	// user gets to watch the intro sequence again ...

	if(state_.is_mid_game_save() && load.data().show_replay) {
		statistics::clear_current_scenario();
	}

	if(state_.classification().is_multiplayer()) {
		state_.unify_controllers();
	}

	if(load.data().cancel_orders) {
		state_.cancel_orders();
	}

	return true;
}

bool game_launcher::new_campaign()
{
	state_.clear();
	state_.classification().campaign_type = game_classification::CAMPAIGN_TYPE::SCENARIO;
	play_replay_ = false;

	return sp::select_campaign(state_, jump_to_campaign_);
}

std::string game_launcher::jump_to_campaign_id() const
{
	return jump_to_campaign_.campaign_id;
}

bool game_launcher::goto_campaign()
{
	if(jump_to_campaign_.jump) {
		if(new_campaign()) {
			state_.set_skip_story(jump_to_campaign_.skip_story);
			jump_to_campaign_.jump = false;
			launch_game(reload_mode::NO_RELOAD_DATA);
		} else {
			jump_to_campaign_.jump = false;
			return false;
		}
	}

	return true;
}

bool game_launcher::goto_multiplayer()
{
	if(jump_to_multiplayer_) {
		jump_to_multiplayer_ = false;
		if(play_multiplayer(mp_mode::CONNECT)) {
			;
		} else {
			return false;
		}
	}

	return true;
}

bool game_launcher::goto_editor()
{
	if(jump_to_editor_) {
		jump_to_editor_ = false;

		const std::string to_open = load_data_ ? filesystem::normalize_path(load_data_->filename) : "";
		clear_loaded_game();

		if(start_editor(to_open) == editor::EXIT_QUIT_TO_DESKTOP) {
			return false;
		}
	}

	return true;
}

void game_launcher::start_wesnothd()
{
	const std::string wesnothd_program = preferences::get_mp_server_program_name().empty()
		? filesystem::get_program_invocation("wesnothd")
		: preferences::get_mp_server_program_name();

	std::string config = filesystem::get_user_config_dir() + "/lan_server.cfg";
	if (!filesystem::file_exists(config)) {
		// copy file if it isn't created yet
		filesystem::write_file(config, filesystem::read_file(filesystem::get_wml_location("lan_server.cfg")));
	}

#ifndef _WIN32
	std::string command = "\"" + wesnothd_program +"\" -c \"" + config + "\" -d -t 2 -T 5";
#else
	// start wesnoth as background job
	std::string command = "cmd /C start \"wesnoth server\" /B \"" + wesnothd_program + "\" -c \"" + config + "\" -t 2 -T 5";
	// Make sure wesnothd's console output is visible on the console window by
	// disabling SDL's stdio redirection code for this and future child
	// processes. No need to bother cleaning this up because it's only
	// meaningful to SDL applications during pre-main initialization.
	SetEnvironmentVariableA("SDL_STDIO_REDIRECT", "0");
#endif
	LOG_GENERAL << "Starting wesnothd: "<< command << "\n";
	if (std::system(command.c_str()) == 0) {
		// Give server a moment to start up
		SDL_Delay(50);
		return;
	}
	preferences::set_mp_server_program_name("");

	// Couldn't start server so throw error
	WRN_GENERAL << "Failed to run server start script" << std::endl;
	throw game::mp_server_error("Starting MP server failed!");
}

bool game_launcher::play_multiplayer(mp_mode mode)
{
	try {
		if(mode == mp_mode::HOST) {
			try {
				start_wesnothd();
			} catch(const game::mp_server_error&) {
				preferences::show_wesnothd_server_search();

				try {
					start_wesnothd();
				} catch(const game::mp_server_error&) {
					return false;
				}
			}
		}

		// If a server address wasn't specified, prompt for it now.
		if(mode != mp_mode::LOCAL && multiplayer_server_.empty()) {
			if(!gui2::dialogs::mp_connect::execute()) {
				return false;
			}

			// The prompt saves its input to preferences.
			multiplayer_server_ = preferences::network_host();

			if(multiplayer_server_ != preferences::builtin_servers_list().front().address) {
				preferences::set_network_host(multiplayer_server_);
			}
		}

		// create_engine already calls game_config_manager::get()->load_config but maybe its better to have MULTIPLAYER
		// defined while we are in the lobby.
		game_config_manager::get()->load_game_config_for_create(true);

		events::discard_input(); // prevent the "keylogger" effect
		cursor::set(cursor::NORMAL);

		if(mode == mp_mode::LOCAL) {
			mp::start_local_game();
		} else {
			mp::start_client(multiplayer_server_);
			multiplayer_server_.clear();
		}

	} catch(const wesnothd_rejected_client_error& e) {
		gui2::show_error_message(e.message);
	} catch(const game::mp_server_error& e) {
		gui2::show_error_message(_("Error while starting server: ") + e.message);
	} catch(const game::load_game_failed& e) {
		gui2::show_error_message(_("The game could not be loaded: ") + e.message);
	} catch(const game::game_error& e) {
		gui2::show_error_message(_("Error while playing the game: ") + e.message);
	} catch(const mapgen_exception& e) {
		gui2::show_error_message(_("Map generator error: ") + e.message);
	} catch(const wesnothd_error& e) {
		if(!e.message.empty()) {
			ERR_NET << "caught network error: " << e.message << std::endl;

			std::string user_msg;
			auto conn_err = dynamic_cast<const wesnothd_connection_error*>(&e);

			if(conn_err) {
				// The wesnothd_connection_error subclass is only thrown with messages
				// from boost::system::error_code which we can't translate ourselves.
				// It's also the originator of the infamous EOF error that happens when
				// the server dies. <https://github.com/wesnoth/wesnoth/issues/3005>. It
				// will provide a translated string instead of that when it happens.
				user_msg = !conn_err->user_message.empty()
					? conn_err->user_message
					: _("Connection failed: ") + e.message;
			} else {
				// This will be a message from the server itself, which we can
				// probably translate.
				user_msg = translation::gettext(e.message.c_str());
			}

			gui2::show_error_message(user_msg);
		} else {
			ERR_NET << "caught network error" << std::endl;
		}
	} catch(const config::error& e) {
		if(!e.message.empty()) {
			ERR_CONFIG << "caught config::error: " << e.message << std::endl;
			gui2::show_transient_message("", e.message);
		} else {
			ERR_CONFIG << "caught config::error" << std::endl;
		}
	} catch(const incorrect_map_format_error& e) {
		gui2::show_error_message(_("The game map could not be loaded: ") + e.message);
	} catch(savegame::load_game_exception& e) {
		load_data_ = std::move(e.data_);
		// this will make it so next time through the title screen loop, this game is loaded
	} catch(const wml_exception& e) {
		e.show();
	} catch(const game::error& e) {
		std::cerr << "caught game::error...\n";
		gui2::show_error_message(_("Error: ") + e.message);
	}

	return true;
}

bool game_launcher::play_multiplayer_commandline()
{
	if(!cmdline_opts_.multiplayer) {
		return true;
	}

	DBG_MP << "starting multiplayer game from the commandline" << std::endl;

	game_config_manager::get()->load_game_config_for_create(true);

	events::discard_input(); // prevent the "keylogger" effect
	cursor::set(cursor::NORMAL);

	mp::start_local_game_commandline(cmdline_opts_);

	return false;
}

bool game_launcher::change_language()
{
	if(!gui2::dialogs::language_selection::execute()) {
		return false;
	}

	if(!(cmdline_opts_.nogui || cmdline_opts_.headless_unit_test)) {
		video_->set_window_title(game_config::get_default_title_string());
	}

	t_string::reset_translations();
	image::flush_cache();
	sound::flush_cache();
	font::load_font_config();

	return true;
}

void game_launcher::launch_game(reload_mode reload)
{
	assert(!load_data_);
	if(play_replay_) {
		play_replay();
		return;
	}

	gui2::dialogs::loading_screen::display([this, reload]() {
		gui2::dialogs::loading_screen::progress(loading_stage::load_data);

		if(reload == reload_mode::RELOAD_DATA) {
			try {
				game_config_manager::get()->load_game_config_for_game(
					state_.classification(), state_.get_scenario_id());
			} catch(const config::error&) {
				return;
			}
		}
	});

	try {
		campaign_controller ccontroller(state_);
		ccontroller.play_game();
		ai::manager::singleton_ = nullptr;
	} catch(savegame::load_game_exception& e) {
		load_data_ = std::move(e.data_);
		// this will make it so next time through the title screen loop, this game is loaded
	} catch(const wml_exception& e) {
		e.show();
	} catch(const mapgen_exception& e) {
		gui2::show_error_message(_("Map generator error: ") + e.message);
	}
}

void game_launcher::play_replay()
{
	assert(!load_data_);
	try {
		campaign_controller ccontroller(state_);
		ccontroller.play_replay();
	} catch(savegame::load_game_exception& e) {
		load_data_ = std::move(e.data_);
		// this will make it so next time through the title screen loop, this game is loaded
	} catch(const wml_exception& e) {
		e.show();
	}
}

editor::EXIT_STATUS game_launcher::start_editor(const std::string& filename)
{
	while(true) {
		game_config_manager::get()->load_game_config_for_editor();

		::init_textdomains(game_config_manager::get()->game_config());

		editor::EXIT_STATUS res = editor::start(filename);

		if(res != editor::EXIT_RELOAD_DATA) {
			return res;
		}

		game_config_manager::get()->reload_changed_game_config();
	}

	return editor::EXIT_ERROR; // not supposed to happen
}

void game_launcher::clear_loaded_game()
{
	load_data_.reset();
}

game_launcher::~game_launcher()
{
	try {
		sound::close_sound();
	} catch(...) {
	}
}
