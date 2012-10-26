/* $Id$ */
/*
   Copyright (C) 2003 - 2012 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "game_controller.hpp"

#include "about.hpp"
#include "addon/manager.hpp"
#include "ai/configuration.hpp"
#include "builder.hpp"
#include "construct_dialog.hpp"
#include "gettext.hpp"
#include "gui/dialogs/addon_connect.hpp"
#include "gui/dialogs/campaign_difficulty.hpp"
#include "gui/dialogs/campaign_selection.hpp"
#include "gui/dialogs/language_selection.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/mp_method_selection.hpp"
#include "gui/dialogs/title_screen.hpp"
#include "gui/dialogs/transient_message.hpp"
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
#include "gui/widgets/debug.hpp"
#endif
#include "gui/auxiliary/event/handler.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "intro.hpp"
#include "language.hpp"
#include "loadscreen.hpp"
#include "log.hpp"
#include "map_exception.hpp"
#include "multiplayer.hpp"
#include "network.hpp"
#include "playcampaign.hpp"
#include "preferences_display.hpp"
#include "replay.hpp"
#include "savegame.hpp"
#include "scripting/lua.hpp"
#include "statistics.hpp"
#include "wml_exception.hpp"
#include "gui/dialogs/mp_host_game_prompt.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_config("config");
#define ERR_CONFIG LOG_STREAM(err, log_config)
#define WRN_CONFIG LOG_STREAM(warn, log_config)
#define LOG_CONFIG LOG_STREAM(info, log_config)

#define LOG_GENERAL LOG_STREAM(info, lg::general)
#define WRN_GENERAL LOG_STREAM(warn, lg::general)
#define DBG_GENERAL LOG_STREAM(debug, lg::general)

static lg::log_domain log_network("network");
#define ERR_NET LOG_STREAM(err, log_network)

static lg::log_domain log_enginerefac("enginerefac");
#define LOG_RG LOG_STREAM(info, log_enginerefac)

static bool less_campaigns_rank(const config &a, const config &b) {
	return a["rank"].to_int(1000) < b["rank"].to_int(1000);
}

game_controller::game_controller(const commandline_options& cmdline_opts, const char *appname) :
	game_controller_abstract(cmdline_opts),
	thread_manager(),
	font_manager_(),
	prefs_manager_(),
	image_manager_(),
	main_event_context_(),
	hotkey_manager_(),
	music_thinker_(),
	resize_monitor_(),
	paths_manager_(),
	test_scenario_("test"),
	screenshot_map_(),
	screenshot_filename_(),
	game_config_(),
	old_defines_map_(),
	state_(),
	multiplayer_server_(),
	jump_to_multiplayer_(false),
	jump_to_campaign_(false, -1, "", ""),
	jump_to_editor_(false),
	cache_(game_config::config_cache::instance())
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
		game_config::path = get_cwd() + '/' + game_config::path;
		font_manager_.update_font_path();
	}

	const std::string app_basename = file_name(appname);
	jump_to_editor_ = app_basename.find("editor") != std::string::npos;

	if (cmdline_opts_.campaign)	{
		jump_to_campaign_.jump_ = true;
		jump_to_campaign_.campaign_id_ = *cmdline_opts_.campaign;
		std::cerr << "selected campaign id: [" << jump_to_campaign_.campaign_id_ << "]\n";

		if (cmdline_opts_.campaign_difficulty) {
			jump_to_campaign_.difficulty_ = *cmdline_opts_.campaign_difficulty;
			std::cerr << "selected difficulty: [" << jump_to_campaign_.difficulty_ << "]\n";
		}
		else
			jump_to_campaign_.difficulty_ = -1; // let the user choose the difficulty

		if (cmdline_opts_.campaign_scenario) {
			jump_to_campaign_.scenario_id_ = *cmdline_opts_.campaign_scenario;
			std::cerr << "selected scenario id: [" << jump_to_campaign_.scenario_id_ << "]\n";
		}
	}
	if (cmdline_opts_.clock)
		gui2::show_debug_clock_button = true;
	if (cmdline_opts_.debug) {
		game_config::debug = true;
		game_config::mp_debug = true;
	}
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	if (cmdline_opts_.debug_dot_domain)
		gui2::tdebug_layout_graph::set_domain (*cmdline_opts_.debug_dot_domain);
	if (cmdline_opts_.debug_dot_level)
		gui2::tdebug_layout_graph::set_level (*cmdline_opts_.debug_dot_level);
#endif
	if (cmdline_opts_.editor)
	{
		jump_to_editor_ = true;
		if(!cmdline_opts_.editor->empty())
			game::load_game_exception::game = *cmdline_opts_.editor;
	}
	if (cmdline_opts_.fps)
		preferences::set_show_fps(true);
	if (cmdline_opts_.fullscreen)
		preferences::set_fullscreen(true);
	if (cmdline_opts_.load)
		game::load_game_exception::game = *cmdline_opts_.load;
	if (cmdline_opts_.max_fps) {
		int fps;
		//FIXME: remove the next line once the weird util.cpp specialized template lexical_cast_default() linking issue is solved
		fps = lexical_cast_default<int>("", 50);
		fps = *cmdline_opts_.max_fps;
		fps = std::min<int>(fps, 1000);
		fps = std::max<int>(fps, 1);
		fps = 1000 / fps;
		// increase the delay to avoid going above the maximum
		if(1000 % fps != 0) {
			++fps;
		}
		preferences::set_draw_delay(fps);
	}
	if (cmdline_opts_.nogui) {
		no_sound = true;
		preferences::disable_preferences_save();
	}
	if (cmdline_opts_.new_storyscreens)
		// This is a hidden option to help testing
		// the work-in-progress new storyscreen code.
		// Don't document.
		set_new_storyscreen(true);
	if (cmdline_opts_.new_widgets)
		gui2::new_widgets = true;
	if (cmdline_opts_.nocache)
		cache_.set_use_cache(false);
	if (cmdline_opts_.nodelay)
		game_config::no_delay = true;
	if (cmdline_opts_.nomusic)
		no_music = true;
	if (cmdline_opts_.nosound)
		no_sound = true;
	//These commented lines should be used to implement support of connection
	//through a proxy via command line options.
	//The ANA network module should implement these methods (while the SDL_net won't.)
	if (cmdline_opts_.proxy)
		network::enable_connection_through_proxy();
	if (cmdline_opts_.proxy_address)
	{
		network::enable_connection_through_proxy();
		network::set_proxy_address(*cmdline_opts_.proxy_address);
	}
	if (cmdline_opts_.proxy_password)
	{
		network::enable_connection_through_proxy();
		network::set_proxy_password(*cmdline_opts_.proxy_password);
	}
	if (cmdline_opts_.proxy_port)
	{
		network::enable_connection_through_proxy();
		network::set_proxy_port(*cmdline_opts_.proxy_port);
	}
	if (cmdline_opts_.proxy_user)
	{
		network::enable_connection_through_proxy();
		network::set_proxy_user(*cmdline_opts_.proxy_user);
	}
	if (cmdline_opts_.resolution) {
		const int xres = cmdline_opts_.resolution->get<0>();
		const int yres = cmdline_opts_.resolution->get<1>();
		if(xres > 0 && yres > 0) {
			const std::pair<int,int> resolution(xres,yres);
			preferences::set_resolution(resolution);
		}
	}
	if (cmdline_opts_.screenshot) {
		//TODO it could be simplified to use cmdline_opts_ directly if there is no other way to enter screenshot mode
		screenshot_map_ = *cmdline_opts_.screenshot_map_file;
		screenshot_filename_ = *cmdline_opts_.screenshot_output_file;
		no_sound = true;
		preferences::disable_preferences_save();
	}
	if (cmdline_opts_.server){
		jump_to_multiplayer_ = true;
		//Do we have any server specified ?
		if (!cmdline_opts_.server->empty())
			multiplayer_server_ = *cmdline_opts_.server;
		else //Pick the first server in config
		{
			if (game_config::server_list.size() > 0)
				multiplayer_server_ = preferences::network_host();
			else
				multiplayer_server_ = "";
		}
	}
	if (cmdline_opts_.username) {
		preferences::disable_preferences_save();
		preferences::set_login(*cmdline_opts_.username);
	}
	if (cmdline_opts_.password) {
		preferences::disable_preferences_save();
		preferences::set_password(*cmdline_opts_.password);
	}
	if (cmdline_opts_.smallgui)
		game_config::small_gui = true;
	if (cmdline_opts_.test)
	{
		if (!cmdline_opts_.test->empty())
			test_scenario_ = *cmdline_opts_.test;
	}
	if (cmdline_opts_.validcache)
		cache_.set_force_valid_cache(true);
	if (cmdline_opts_.windowed)
		preferences::set_fullscreen(false);
	if (cmdline_opts_.with_replay)
		game::load_game_exception::show_replay = true;

	std::cerr << '\n';
	std::cerr << "Data directory: " << game_config::path
		<< "\nUser configuration directory: " << get_user_config_dir()
		<< "\nUser data directory: " << get_user_data_dir()
		<< "\nCache directory: " << get_cache_dir()
		<< '\n';

	// disable sound in nosound mode, or when sound engine failed to initialize
	if (no_sound || ((preferences::sound_on() || preferences::music_on() ||
	                  preferences::turn_bell() || preferences::UI_sound_on()) &&
	                 !sound::init_sound())) {
		preferences::set_sound(false);
		preferences::set_music(false);
		preferences::set_turn_bell(false);
		preferences::set_UI_sound(false);
	}
	else if (no_music) { // else disable the music in nomusic mode
		preferences::set_music(false);
	}
}

bool game_controller::init_config(const bool force)
{
	cache_.clear_defines();

	// make sure that multiplayer mode is set if command line parameter is selected
	if (cmdline_opts_.multiplayer)
		cache_.add_define("MULTIPLAYER");

	if (cmdline_opts_.test)
		cache_.add_define("TEST");

	if (jump_to_editor_)
		cache_.add_define("EDITOR");

	if (!cmdline_opts_.multiplayer && !cmdline_opts_.test && !jump_to_editor_)
		cache_.add_define("TITLE_SCREEN");

	load_game_cfg(force);

	game_config::load_config(game_config_.child("game_config"));

	hotkey::deactivate_all_scopes();
	hotkey::set_scope_active(hotkey::SCOPE_GENERAL);
	hotkey::set_scope_active(hotkey::SCOPE_GAME);

	hotkey::load_hotkeys(game_config(), true);
	paths_manager_.set_paths(game_config());
	::init_textdomains(game_config());
	about::set_about(game_config());
	ai::configuration::init(game_config());

	return true;
}

bool game_controller::play_test()
{
	static bool first_time = true;

	if(!cmdline_opts_.test) {
		return true;
	}
	if(!first_time)
		return false;

	first_time = false;

	state_.classification().campaign_type = "test";
	state_.carryover_sides_start["next_scenario"] = test_scenario_;
	state_.classification().campaign_define = "TEST";
	cache_.add_define("TEST");

	load_game_cfg();

	paths_manager_.set_paths(game_config());

	try {
		play_game(disp(),state_,game_config());
	} catch (game::load_game_exception &) {
		return true;
	}

	return false;
}

bool game_controller::play_screenshot_mode()
{
	if(!cmdline_opts_.screenshot) {
		return true;
	}

	cache_.clear_defines();
	cache_.add_define("EDITOR");
	load_game_cfg();
	const binary_paths_manager bin_paths_manager(game_config());
	::init_textdomains(game_config());

	editor::start(game_config(), video_, screenshot_map_, true, screenshot_filename_);
	return false;
}

bool game_controller::play_multiplayer_mode()
{
	state_ = game_state();

	if(!cmdline_opts_.multiplayer) {
		return true;
	}

	std::string era = "era_default";
	std::string scenario = "multiplayer_The_Freelands";
	std::map<int,std::string> side_types, side_controllers, side_algorithms, side_ai_configs;
	std::map<int,utils::string_map> side_parameters;
	std::string turns = "50";
	std::string label = "";

	size_t sides_counted = 0;

	if (cmdline_opts_.multiplayer_ai_config)
	{
		for(std::vector<boost::tuple<unsigned int, std::string> >::const_iterator it=cmdline_opts_.multiplayer_ai_config->begin(); it!=cmdline_opts_.multiplayer_ai_config->end(); ++it)
		{
			const unsigned int side = it->get<0>();
			const std::string ai_cfg_name = it->get<1>();
			if (side > sides_counted)
			{
				std::cerr << "counted sides: " << side << "\n";
				sides_counted = side;
			}
			side_ai_configs[side] = ai_cfg_name;
		}
	}
	if (cmdline_opts_.multiplayer_algorithm)
	{
		for(std::vector<boost::tuple<unsigned int, std::string> >::const_iterator it=cmdline_opts_.multiplayer_algorithm->begin(); it!=cmdline_opts_.multiplayer_algorithm->end(); ++it)
		{
			const unsigned int side = it->get<0>();
			const std::string algorithm_id = it->get<1>();
			if (side > sides_counted)
			{
				std::cerr << "counted sides: " << side << "\n";
				sides_counted = side;
			}
			side_algorithms[side] = algorithm_id;
		}
	}
	if (cmdline_opts_.multiplayer_controller)
	{
		for(std::vector<boost::tuple<unsigned int, std::string> >::const_iterator it=cmdline_opts_.multiplayer_controller->begin(); it!=cmdline_opts_.multiplayer_controller->end(); ++it)
		{
			const unsigned int side = it->get<0>();
			const std::string controller_id = it->get<1>();
			if (side > sides_counted)
			{
				std::cerr << "counted sides: " << side << "\n";
				sides_counted = side;
			}
			side_controllers[side] = controller_id;
		}
	}
	if (cmdline_opts_.multiplayer_era)
		era = *cmdline_opts_.multiplayer_era;
	if (cmdline_opts_.multiplayer_exit_at_end)
		game_config::exit_at_end = true;
	if (cmdline_opts_.multiplayer_label)
		label = *cmdline_opts_.multiplayer_label;
	if (cmdline_opts_.multiplayer_parm)
	{
		for(std::vector<boost::tuple<unsigned int, std::string, std::string> >::const_iterator it=cmdline_opts_.multiplayer_parm->begin(); it!=cmdline_opts_.multiplayer_parm->end(); ++it)
		{
			const unsigned int side = it->get<0>();
			const std::string param_name = it->get<1>();
			const std::string param_value = it->get<2>();
			side_parameters[side][param_name] = param_value;
		}
	}
	if (cmdline_opts_.multiplayer_scenario)
		scenario = *cmdline_opts_.multiplayer_scenario;
	if (cmdline_opts_.multiplayer_side)
	{
		for(std::vector<boost::tuple<unsigned int, std::string> >::const_iterator it=cmdline_opts_.multiplayer_side->begin(); it!=cmdline_opts_.multiplayer_side->end(); ++it)
		{
			const unsigned int side = it->get<0>();
			const std::string faction_id = it->get<1>();
			if (side > sides_counted)
			{
				std::cerr << "counted sides: " << side << "\n";
				sides_counted = side;
			}
			side_types[side] = faction_id;
		}
	}
	if (cmdline_opts_.multiplayer_turns)
		turns = *cmdline_opts_.multiplayer_turns;

	const config &lvl = game_config().find_child("multiplayer", "id", scenario);
	if (!lvl) {
		std::cerr << "Could not find scenario '" << scenario << "'\n";
		return false;
	}

	state_.classification().campaign_type = "multiplayer";
	state_.carryover_sides_start["next_scenario"] = "";
	state_.snapshot = config();

	config level = lvl;

	const config &era_cfg = game_config().find_child("era","id",era);
	if (!era_cfg) {
		std::cerr << "Could not find era '" << era << "'\n";
		return false;
	}

	level["turns"] = turns;

	const config &side = era_cfg.child("multiplayer_side");
	if (!side) {
		std::cerr << "Could not find multiplayer side\n";
		return false;
	}

	while (level.child_count("side") < sides_counted) {
		std::cerr << "now adding side...\n";
		level.add_child("side");
	}

	int side_num = 1;
	BOOST_FOREACH(config &s, level.child_range("side"))
	{
		std::map<int,std::string>::const_iterator type = side_types.find(side_num),
		                                          controller = side_controllers.find(side_num),
		                                          algorithm = side_algorithms.find(side_num),
							  ai_config = side_ai_configs.find(side_num);

		const config* side = type == side_types.end() ?
			&era_cfg.find_child("multiplayer_side", "random_faction", "yes") :
			&era_cfg.find_child("multiplayer_side", "id", type->second);

		if (!*side) {
			std::string side_name = (type == side_types.end() ? "default" : type->second);
			std::cerr << "Could not find side '" << side_name << "' for side " << side_num << "\n";
			return false;
		}

		if ((*side)["random_faction"].to_bool())
		{
			std::vector<std::string> faction_choices, faction_excepts;
			faction_choices = utils::split((*side)["choices"]);
			if(faction_choices.size() == 1 && faction_choices.front() == "") {
				faction_choices.clear();
			}
			faction_excepts = utils::split((*side)["except"]);;
			if(faction_excepts.size() == 1 && faction_excepts.front() == "") {
				faction_excepts.clear();
			}
			unsigned j = 0;
			BOOST_FOREACH(const config &faction, era_cfg.child_range("multiplayer_side"))
			{
				if (faction["random_faction"].to_bool()) continue;
				const std::string &faction_id = faction["id"];
				if (!faction_choices.empty() &&
				    std::find(faction_choices.begin(), faction_choices.end(), faction_id) == faction_choices.end())
					continue;
				if (!faction_excepts.empty() &&
				    std::find(faction_excepts.begin(), faction_excepts.end(), faction_id) != faction_excepts.end())
					continue;
				if (rand() % ++j == 0)
					side = &faction;
			}
			if ((*side)["random_faction"].to_bool()) {
				std::cerr << "Could not find any non-random faction for side " << side_num << "\n";
				return false;
			}
			std::cerr << " Faction " << (*side)["name"] <<
				" selected for side " << side_num << ".\n";
		}

		s["side"] = side_num;
		s["canrecruit"] = true;

		s.append(*side);

		if(controller != side_controllers.end()) {
			s["controller"] = controller->second;
		}

		if(algorithm != side_algorithms.end()) {
			s["ai_algorithm"] = algorithm->second;
		}

		if(ai_config != side_ai_configs.end()) {
			s["ai_config"] = ai_config->second;
		}

		config& ai_params = s.add_child("ai");

		//now add in any arbitrary parameters given to the side
		for(utils::string_map::const_iterator j = side_parameters[side_num].begin(); j != side_parameters[side_num].end(); ++j) {
			s[j->first] = j->second;
			ai_params[j->first] = j->second;
		}
		++side_num;
	}
	level.add_child("era", era_cfg);

	try {
		recorder.add_log_data("ai_log","ai_label",label);
		state_.snapshot = level;
		play_game(disp(), state_, game_config());
	} catch (game::load_game_exception &) {
		//the user's trying to load a game, so go into the normal title screen loop and load one
		return true;
	} catch(twml_exception& e) {
		e.show(disp());
		return false;
	} catch(std::exception& e) {
		std::cerr << "caught exception: " << e.what() << "\n";
	} catch(...) {
		std::cerr << "caught unknown error playing level...\n";
	}

	return false;
}

bool game_controller::is_loading() const
{
	return !game::load_game_exception::game.empty();
}

bool game_controller::load_game()
{
	savegame::loadgame load(disp(), game_config(), state_);

	try {
		load.load_game(game::load_game_exception::game, game::load_game_exception::show_replay, game::load_game_exception::cancel_orders, game::load_game_exception::select_difficulty, game::load_game_exception::difficulty);

		cache_.clear_defines();
		game_config::scoped_preproc_define dificulty_def(load.load_config().child("carryover_sides_start")["difficulty"]);

		game_config::scoped_preproc_define campaign_define_def(state_.classification().campaign_define, !state_.classification().campaign_define.empty());

		game_config::scoped_preproc_define campaign_type_def("MULTIPLAYER", state_.classification().campaign_define.empty() && (state_.classification().campaign_type == "multiplayer"));

		typedef boost::shared_ptr<game_config::scoped_preproc_define> define_ptr;
		std::deque<define_ptr> extra_defines;
		for(std::vector<std::string>::const_iterator i = state_.classification().campaign_xtra_defines.begin(); i != state_.classification().campaign_xtra_defines.end(); ++i) {
			define_ptr newdefine(new game_config::scoped_preproc_define(*i));
			extra_defines.push_back(newdefine);
		}

		try {
			load_game_cfg();
		} catch(config::error&) {
			cache_.clear_defines();
			load_game_cfg();
			return false;
		}

		paths_manager_.set_paths(game_config());
		load.set_gamestate();

	} catch(load_game_cancelled_exception&) {
		clear_loaded_game();
		return false;
	} catch(config::error& e) {
		if(e.message.empty()) {
			gui2::show_error_message(disp().video(), _("The file you have tried to load is corrupt"));
		}
		else {
			gui2::show_error_message(disp().video(), _("The file you have tried to load is corrupt: '") + e.message + '\'');
		}
		return false;
	} catch(twml_exception& e) {
		e.show(disp());
		return false;
	} catch(io_exception& e) {
		if(e.message.empty()) {
			gui2::show_error_message(disp().video(), _("File I/O Error while reading the game"));
		} else {
			gui2::show_error_message(disp().video(), _("File I/O Error while reading the game: '") + e.message + '\'');
		}
		return false;
	} catch(game::error& e) {
		if(e.message.empty()) {
			gui2::show_error_message(disp().video(), _("The file you have tried to load is corrupt"));
		}
		else {
			gui2::show_error_message(disp().video(), _("The file you have tried to load is corrupt: '") + e.message + '\'');
		}
		return false;
	}
	recorder = replay(state_.replay_data);
	recorder.start_replay();
	recorder.set_skip(false);

	LOG_CONFIG << "has snapshot: " << (state_.snapshot.child("side") ? "yes" : "no") << "\n";

	if (!state_.snapshot.child("side")) {
		// No snapshot; this is a start-of-scenario
		if (load.show_replay()) {
			// There won't be any turns to replay, but the
			// user gets to watch the intro sequence again ...
			LOG_CONFIG << "replaying (start of scenario)\n";
		} else {
			LOG_CONFIG << "skipping...\n";
			recorder.set_skip(false);
		}
	} else {
		// We have a snapshot. But does the user want to see a replay?
		if(load.show_replay()) {
			statistics::clear_current_scenario();
			LOG_CONFIG << "replaying (snapshot)\n";
		} else {
			LOG_CONFIG << "setting replay to end...\n";
			recorder.set_to_end();
			if(!recorder.at_end()) {
				WRN_CONFIG << "recorder is not at the end!!!\n";
			}
		}
	}

	if(state_.classification().campaign_type == "multiplayer") {
		BOOST_FOREACH(config &side, state_.snapshot.child_range("side"))
		{
			if (side["controller"] == "network")
				side["controller"] = "human";
			if (side["controller"] == "network_ai")
				side["controller"] = "human_ai";
		}
	}

	if (load.cancel_orders()) {
		BOOST_FOREACH(config &side, state_.snapshot.child_range("side"))
		{
			if (side["controller"] != "human") continue;
			BOOST_FOREACH(config &unit, side.child_range("unit"))
			{
				unit["goto_x"] = -999;
				unit["goto_y"] = -999;
			}
		}
	}

	return true;
}

void game_controller::set_tutorial()
{
	state_ = game_state();
	state_.classification().campaign_type = "tutorial";
	state_.carryover_sides_start["next_scenario"] = "tutorial";
	state_.classification().campaign_define = "TUTORIAL";
	cache_.clear_defines();
	cache_.add_define("TUTORIAL");

}

void game_controller::mark_completed_campaigns(std::vector<config> &campaigns)
{
	BOOST_FOREACH(config &campaign, campaigns) {
		campaign["completed"] = preferences::is_campaign_completed(campaign["id"]);
	}
}

bool game_controller::new_campaign()
{
	state_ = game_state();
	state_.classification().campaign_type = "scenario";

	const config::const_child_itors &ci = game_config().child_range("campaign");
	std::vector<config> campaigns(ci.first, ci.second);
	mark_completed_campaigns(campaigns);
	std::stable_sort(campaigns.begin(),campaigns.end(),less_campaigns_rank);

	if(campaigns.begin() == campaigns.end()) {
	  gui2::show_error_message(disp().video(),
				  _("No campaigns are available.\n"));
		return false;
	}

	int campaign_num = -1;
	// No campaign selected from command line
	if (jump_to_campaign_.campaign_id_.empty() == true)
	{
		gui2::tcampaign_selection dlg(campaigns);

		try {
			dlg.show(disp().video());
		} catch(twml_exception& e) {
			e.show(disp());
			return false;
		}

		if(dlg.get_retval() != gui2::twindow::OK) {
			return false;
		}

		campaign_num = dlg.get_choice();
	}
	else
	{
		// don't reset the campaign_id_ so we can know
		// if we should quit the game or return to the main menu

		// checking for valid campaign name
		for(size_t i = 0; i < campaigns.size(); ++i)
		{
			if (campaigns[i]["id"] == jump_to_campaign_.campaign_id_)
			{
				campaign_num = i;
				break;
			}
		}

		// didn't found any campaign with that id
		if (campaign_num == -1)
		{
			std::cerr<<"No such campaign id to jump to: ["<<jump_to_campaign_.campaign_id_<<"]\n";
			return false;
		}
	}

	const config &campaign = campaigns[campaign_num];
	state_.classification().campaign = campaign["id"].str();
	state_.classification().abbrev = campaign["abbrev"].str();

	// we didn't specify in the command line the scenario to be started
	if (jump_to_campaign_.scenario_id_.empty())
		state_.carryover_sides_start["next_scenario"] = campaign["first_scenario"].str();
	else
		state_.carryover_sides_start["next_scenario"] = jump_to_campaign_.scenario_id_;

	state_.classification().end_text = campaign["end_text"].str();
	state_.classification().end_text_duration = campaign["end_text_duration"];

	const std::string difficulty_descriptions = campaign["difficulty_descriptions"];
	std::vector<std::string> difficulty_options = utils::split(difficulty_descriptions, ';');

	const std::vector<std::string> difficulties = utils::split(campaign["difficulties"]);

	if(difficulties.empty() == false) {
		int difficulty = 0;
		if (jump_to_campaign_.difficulty_ == -1){
			if(difficulty_options.size() != difficulties.size()) {
				difficulty_options.resize(difficulties.size());
				std::copy(difficulties.begin(),difficulties.end(),difficulty_options.begin());
			}

			gui2::tcampaign_difficulty dlg(difficulty_options);
			dlg.show(disp().video());

			if(dlg.selected_index() == -1) {
				if (jump_to_campaign_.campaign_id_.empty() == false)
				{
					jump_to_campaign_.campaign_id_ = "";
				}
				// canceled difficulty dialog, relaunch the campaign selection dialog
				return new_campaign();
			}
			difficulty = dlg.selected_index();
		}
		else
		{
			if (jump_to_campaign_.difficulty_
					> static_cast<int>(difficulties.size()))
			{
				std::cerr << "incorrect difficulty number: [" <<
					jump_to_campaign_.difficulty_ << "]. maximum is [" <<
					difficulties.size() << "].\n";
				return false;
			}
			else
			{
				difficulty = jump_to_campaign_.difficulty_ - 1;
			}
		}

		state_.carryover_sides_start["difficulty"] = difficulties[difficulty];
		cache_.clear_defines();
		cache_.add_define(difficulties[difficulty]);
	} else {
		//clear even when there is no difficulty
		cache_.clear_defines();
	}

	state_.classification().campaign_define = campaign["define"].str();
	state_.classification().campaign_xtra_defines = utils::split(campaign["extra_defines"]);

	return true;
}

std::string game_controller::jump_to_campaign_id() const
{
	return jump_to_campaign_.campaign_id_;
}

bool game_controller::goto_campaign()
{
	if(jump_to_campaign_.jump_){
		if(new_campaign()) {
			jump_to_campaign_.jump_ = false;
			launch_game(game_controller::RELOAD_DATA);
		}else{
			jump_to_campaign_.jump_ = false;
			return false;
		}
	}
	return true;
}

bool game_controller::goto_multiplayer()
{
	if(jump_to_multiplayer_){
		jump_to_multiplayer_ = false;
		if(play_multiplayer()){
			;
		}else{
			return false;
		}
	}
	return true;
}

bool game_controller::goto_editor()
{
	if(jump_to_editor_){
		jump_to_editor_ = false;
		if (start_editor(normalize_path(game::load_game_exception::game)) ==
		    editor::EXIT_QUIT_TO_DESKTOP)
		{
			return false;
		}
		clear_loaded_game();
	}
	return true;
}

void game_controller::reload_changed_game_config()
{
	// rebuild addon version info cache
	refresh_addon_version_info_cache();

	//force a reload of configuration information
	cache_.recheck_filetree_checksum();
	old_defines_map_.clear();
	clear_binary_paths_cache();
	init_config(true);
}

void game_controller::start_wesnothd()
{
	const std::string wesnothd_program =
		preferences::get_mp_server_program_name().empty() ?
		get_program_invocation("wesnothd") : preferences::get_mp_server_program_name();

	std::string config = get_user_config_dir() + "/lan_server.cfg";
	if (!file_exists(config)) {
		// copy file if it isn't created yet
		write_file(config, read_file(get_wml_location("lan_server.cfg")));
	}

#ifndef _WIN32
	std::string command = "\"" + wesnothd_program +"\" -c \"" + config + "\" -d -t 2 -T 5";
#else
	// start wesnoth as background job
	std::string command = "cmd /C start \"wesnoth server\" /B \"" + wesnothd_program + "\" -c \"" + config + "\" -t 2 -T 5";
#endif
	LOG_GENERAL << "Starting wesnothd: "<< command << "\n";
	if (std::system(command.c_str()) == 0) {
		// Give server a moment to start up
		SDL_Delay(50);
		return;
	}
	preferences::set_mp_server_program_name("");

	// Couldn't start server so throw error
	WRN_GENERAL << "Failed to run server start script\n";
	throw game::mp_server_error("Starting MP server failed!");
}

bool game_controller::play_multiplayer()
{
	int res;

	state_ = game_state();
	state_.classification().campaign_type = "multiplayer";
	state_.classification().campaign_define = "MULTIPLAYER";

	//Print Gui only if the user hasn't specified any server
	if( multiplayer_server_.empty() ){

		int start_server;
		do {
			start_server = 0;

			gui2::tmp_method_selection dlg;

			dlg.show(disp().video());

			if(dlg.get_retval() == gui2::twindow::OK) {
				res = dlg.get_choice();
			} else {
				return false;

			}

			if (res == 2 && preferences::mp_server_warning_disabled() < 2) {
				start_server = !gui2::tmp_host_game_prompt::execute(disp().video());
			}
		} while (start_server);
		if (res < 0) {
			return false;
		}

	}else{
		res = 4;
	}

	try {
		if (res == 2)
		{
			try {
				start_wesnothd();
			} catch(game::mp_server_error&)
			{
				std::string path = preferences::show_wesnothd_server_search(disp());

				if (!path.empty())
				{
					preferences::set_mp_server_program_name(path);
					start_wesnothd();
				}
				else
				{
					return false;
				}
			}


		}

		/* do */ {
			cache_.clear_defines();
			game_config::scoped_preproc_define multiplayer(state_.classification().campaign_define);
			load_game_cfg();
			events::discard(INPUT_MASK); // prevent the "keylogger" effect
			cursor::set(cursor::NORMAL);
			// update binary paths
			paths_manager_.set_paths(game_config());
			clear_binary_paths_cache();
		}

		if(res == 3) {
			config game_data;

			const mp::controller cntr = mp::CNTR_LOCAL;

			mp::start_local_game(disp(), game_config(), cntr);

		} else if((res >= 0 && res <= 2) || res == 4) {
			std::string host;
			if(res == 0) {
				host = preferences::server_list().front().address;
			}else if(res == 2) {
				host = "localhost";
			}else if(res == 4){
				host = multiplayer_server_;
				multiplayer_server_ = "";
			}
			mp::start_client(disp(), game_config(), host);
		}

	} catch(game::mp_server_error& e) {
		gui2::show_error_message(disp().video(), _("Error while starting server: ") + e.message);
	} catch(game::load_game_failed& e) {
		gui2::show_error_message(disp().video(), _("The game could not be loaded: ") + e.message);
	} catch(game::game_error& e) {
		gui2::show_error_message(disp().video(), _("Error while playing the game: ") + e.message);
	} catch(network::error& e) {
		if(e.message != "") {
			ERR_NET << "caught network::error: " << e.message << "\n";
			gui2::show_transient_message(disp().video()
					, ""
					, gettext(e.message.c_str()));
		} else {
			ERR_NET << "caught network::error\n";
		}
	} catch(config::error& e) {
		if(e.message != "") {
			ERR_CONFIG << "caught config::error: " << e.message << "\n";
			gui2::show_transient_message(disp().video(), "", e.message);
		} else {
			ERR_CONFIG << "caught config::error\n";
		}
	} catch(incorrect_map_format_error& e) {
		gui2::show_error_message(disp().video(), std::string(_("The game map could not be loaded: ")) + e.message);
	} catch (game::load_game_exception &) {
		//this will make it so next time through the title screen loop, this game is loaded
	} catch(twml_exception& e) {
		e.show(disp());
	}

	return false;
}

bool game_controller::change_language()
{
	gui2::tlanguage_selection dlg;
	dlg.show(disp().video());
	if (dlg.get_retval() != gui2::twindow::OK) return false;

	if (!cmdline_opts_.nogui) {
		std::string wm_title_string = _("The Battle for Wesnoth");
		wm_title_string += " - " + game_config::revision;
		SDL_WM_SetCaption(wm_title_string.c_str(), NULL);
	}

	return true;
}

void game_controller::show_preferences()
{
	const preferences::display_manager disp_manager(&disp());
	preferences::show_preferences_dialog(disp(),game_config());

	disp().redraw_everything();
}

void game_controller::set_unit_data()
{
	loadscreen::start_stage("load unit types");
	if (config &units = game_config_.child("units")) {
		unit_types.set_config(units);
	}
}

void game_controller::load_game_cfg(const bool force)
{
	// make sure that 'debug mode' symbol is set if command line parameter is selected
	// also if we're in multiplayer and actual debug mode is disabled
	if (game_config::debug || game_config::mp_debug) {
		cache_.add_define("DEBUG_MODE");
	}

	if (!game_config_.empty() && !force
			&& old_defines_map_ == cache_.get_preproc_map())
		return; // game_config already holds requested config in memory
	old_defines_map_ = cache_.get_preproc_map();
	loadscreen::global_loadscreen_manager loadscreen_manager(disp().video());
	cursor::setter cur(cursor::WAIT);
	// The loadscreen will erase the titlescreen
	// NOTE: even without loadscreen, needed after MP lobby
	try {
		/**
		 * Read all game configs
		 * First we should load data/
		 * Then handle terrains so that they are last loaded from data/
		 * 2nd everything in userdata
		 **/
		loadscreen::start_stage("verify cache");
		data_tree_checksum();
		loadscreen::start_stage("create cache");

		// start transaction so macros are shared
		game_config::config_cache_transaction main_transaction;

		cache_.get_config(game_config::path +"/data", game_config_);

		main_transaction.lock();

		/* Put the gfx rules aside so that we can prepend the add-on
		   rules to them. */
		config core_terrain_rules;
		core_terrain_rules.splice_children(game_config_, "terrain_graphics");

		// load usermade add-ons
		const std::string user_campaign_dir = get_addon_campaigns_dir();
		std::vector< std::string > error_addons;
		// Scan addon directories
		std::vector<std::string> user_dirs;
		// Scan for standalone files
		std::vector<std::string> user_files;

		// The addons that we'll actually load
		std::vector<std::string> addons_to_load;

		get_files_in_dir(user_campaign_dir,&user_files,&user_dirs,ENTIRE_FILE_PATH);
		std::stringstream user_error_log;

		// Append the $user_campaign_dir/*.cfg files to addons_to_load.
		for(std::vector<std::string>::const_iterator uc = user_files.begin(); uc != user_files.end(); ++uc) {
			const std::string file = *uc;
			int size_minus_extension = file.size() - 4;
			if(file.substr(size_minus_extension, file.size()) == ".cfg") {
				// Allowing it if the dir doesn't exist, for the single-file add-on.
				// Turn this into an error later, possibly in 1.11.0
				if(file_exists(file.substr(0, size_minus_extension)))
					lg::wml_error << '\'' << file << "' is deprecated, use '" << file.substr(0, size_minus_extension) << "/_main.cfg' instead.\n";
				addons_to_load.push_back(file);
			}
		}

		// Append the $user_campaign_dir/*/_main.cfg files to addons_to_load.
		for(std::vector<std::string>::const_iterator uc = user_dirs.begin(); uc != user_dirs.end(); ++uc){
			const std::string main_cfg = *uc + "/_main.cfg";
			if (file_exists(main_cfg))
				addons_to_load.push_back(main_cfg);
		}

		// Load the addons
		for(std::vector<std::string>::const_iterator uc = addons_to_load.begin(); uc != addons_to_load.end(); ++uc) {
			const std::string toplevel = *uc;
			try {
				config umc_cfg;
				cache_.get_config(toplevel, umc_cfg);

				game_config_.append(umc_cfg);
			} catch(config::error& err) {
				ERR_CONFIG << "error reading usermade add-on '" << *uc << "'\n";
				error_addons.push_back(*uc);
				user_error_log << err.message << "\n";
			} catch(preproc_config::error& err) {
				ERR_CONFIG << "error reading usermade add-on '" << *uc << "'\n";
				error_addons.push_back(*uc);
				user_error_log << err.message << "\n";
			} catch(io_exception&) {
				ERR_CONFIG << "error reading usermade add-on '" << *uc << "'\n";
				error_addons.push_back(*uc);
			}
			if(error_addons.empty() == false) {
				std::stringstream msg;
				msg << _n("The following add-on had errors and could not be loaded:",
						"The following add-ons had errors and could not be loaded:",
						error_addons.size());
				for(std::vector<std::string>::const_iterator i = error_addons.begin(); i != error_addons.end(); ++i) {
					msg << "\n" << *i;
				}

				msg << '\n' << _("ERROR DETAILS:") << '\n' << user_error_log.str();

				gui2::show_error_message(disp().video(),msg.str());
			}
		}

		// Extract the Lua scripts at toplevel.
		extract_preload_scripts(game_config_);
		game_config_.clear_children("lua");

		config colorsys_info;
		colorsys_info.splice_children(game_config_, "color_range");
		colorsys_info.splice_children(game_config_, "color_palette");

		game_config_.merge_children("units");
		game_config_.splice_children(core_terrain_rules, "terrain_graphics");

		config& hashes = game_config_.add_child("multiplayer_hashes");
		BOOST_FOREACH(const config &ch, game_config_.child_range("multiplayer")) {
			hashes[ch["id"]] = ch.hash();
		}

		game_config::add_color_info(colorsys_info);

		set_unit_data();

		terrain_builder::set_terrain_rules_cfg(game_config());

		::init_strings(game_config());

		theme::set_known_themes(&game_config());

	} catch(game::error& e) {
		ERR_CONFIG << "Error loading game configuration files\n";
		gui2::show_error_message(disp().video(), _("Error loading game configuration files: '") +
			e.message + _("' (The game will now exit)"));
		throw;
	}
}

void game_controller::launch_game(RELOAD_GAME_DATA reload)
{
	loadscreen::global_loadscreen_manager loadscreen_manager(disp().video());
	loadscreen::start_stage("load data");
	if(reload == RELOAD_DATA) {
		game_config::scoped_preproc_define campaign_define(state_.classification().campaign_define, state_.classification().campaign_define.empty() == false);

		typedef boost::shared_ptr<game_config::scoped_preproc_define> define_ptr;
		std::deque<define_ptr> extra_defines;
		for(std::vector<std::string>::const_iterator i = state_.classification().campaign_xtra_defines.begin(); i != state_.classification().campaign_xtra_defines.end(); ++i) {
			define_ptr newdefine(new game_config::scoped_preproc_define(*i));
			extra_defines.push_back(newdefine);
		}
		try {
			load_game_cfg();
		} catch(config::error&) {
			cache_.clear_defines();
			load_game_cfg();
			return;
		}
	}

	const binary_paths_manager bin_paths_manager(game_config());

	try {
		const LEVEL_RESULT result = play_game(disp(),state_,game_config());
		// don't show The End for multiplayer scenario
		// change this if MP campaigns are implemented
		if(result == VICTORY && (state_.classification().campaign_type.empty() || state_.classification().campaign_type != "multiplayer")) {
			preferences::add_completed_campaign(state_.classification().campaign);
			the_end(disp(), state_.classification().end_text, state_.classification().end_text_duration);
			if(state_.classification().end_credits) {
				about::show_about(disp(),state_.classification().campaign);
			}
		}

		clear_loaded_game();
	} catch (game::load_game_exception &) {
		//this will make it so next time through the title screen loop, this game is loaded
	} catch(twml_exception& e) {
		e.show(disp());
	}
}

void game_controller::play_replay()
{
	const binary_paths_manager bin_paths_manager(game_config());

	try {
		::play_replay(disp(),state_,game_config(),video_);

		clear_loaded_game();
	} catch (game::load_game_exception &) {
		//this will make it so next time through the title screen loop, this game is loaded
	} catch(twml_exception& e) {
		e.show(disp());
	}
}

editor::EXIT_STATUS game_controller::start_editor(const std::string& filename)
{
	while(true){
		cache_.clear_defines();
		cache_.add_define("EDITOR");
		load_game_cfg();
		const binary_paths_manager bin_paths_manager(game_config());
		::init_textdomains(game_config());

		editor::EXIT_STATUS res = editor::start(game_config(), video_, filename);

		if(res != editor::EXIT_RELOAD_DATA)
			return res;

		reload_changed_game_config();
		image::flush_cache();
	}
	return editor::EXIT_ERROR; // not supposed to happen
}

game_controller::~game_controller()
{
	gui::dialog::delete_empty_menu();
	sound::close_sound();
}
