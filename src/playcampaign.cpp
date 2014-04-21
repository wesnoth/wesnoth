/*
   Copyright (C) 2003-2005 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2014 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file
 *  Controls setup, play, (auto)save and replay of campaigns.
 */

#include "global.hpp"

#include "game_preferences.hpp"
#include "generators/map_create.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/window.hpp"
#include "playcampaign.hpp"
#include "persist_manager.hpp"
#include "playmp_controller.hpp"
#include "replay_controller.hpp"
#include "log.hpp"
#include "map_exception.hpp"
#include "mp_game_utils.hpp"
#include "multiplayer.hpp"
#include "multiplayer_connect_engine.hpp"
#include "dialogs.hpp"
#include "gettext.hpp"
#include "resources.hpp"
#include "savegame.hpp"
#include "sound.hpp"
#include "wml_exception.hpp"
#include "formula_string_utils.hpp"

#include <boost/foreach.hpp>

#define LOG_G LOG_STREAM(info, lg::general)

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)

static lg::log_domain log_enginerefac("enginerefac");
#define LOG_RG LOG_STREAM(info, log_enginerefac)

static void team_init(config& level, game_state& gamestate){
	//if we are at the start of a new scenario, initialize carryover_sides
	if(gamestate.snapshot.child_or_empty("variables")["turn_number"].to_int(-1)<1){
		gamestate.carryover_sides = gamestate.carryover_sides_start;


		carryover_info sides(gamestate.carryover_sides);

		sides.transfer_to(level);
		BOOST_FOREACH(config& side_cfg, level.child_range("side")){
			sides.transfer_all_to(side_cfg);
		}

		gamestate.carryover_sides = sides.to_config();
	}
}

static void do_carryover_WML(config & level, game_state& gamestate){

	if(gamestate.snapshot.child_or_empty("variables")["turn_number"].to_int(-1)<1){

		carryover_info sides(gamestate.carryover_sides_start);

		end_level_data end_level_ = sides.get_end_level();

		if(!end_level_.next_scenario_settings.empty()) {
			level.merge_with(end_level_.next_scenario_settings);
		}
		if(!end_level_.next_scenario_append.empty())
		{
			level.append_children(end_level_.next_scenario_append);
		}
	}
}

static void clear_carryover_WML (game_state & gamestate) {

	if (gamestate.carryover_sides.has_child("end_level_data")) {
		config & eld = gamestate.carryover_sides.child("end_level_data");
		eld.clear_children("next_scenario_settings");
		eld.clear_children("next_scenario_append");
	}	
}

static void store_carryover(game_state& gamestate, playsingle_controller& playcontroller, display& disp, const end_level_data& end_level){
	bool has_next_scenario = !resources::gamedata->next_scenario().empty() &&
			resources::gamedata->next_scenario() != "null";

	if(resources::teams->size() < 1){
		gamestate.carryover_sides_start["next_scenario"] = resources::gamedata->next_scenario();
		return;
	}

	carryover_info sides(gamestate.carryover_sides);

	sides.transfer_from(*resources::gamedata);

	std::ostringstream report;
	std::string title;

	bool obs = is_observer();

	if (obs) {
		title = _("Scenario Report");
	} else {
		title = _("Victory");
		report << "<b>" << _("You have emerged victorious!") << "</b>\n\n";
	}

	std::vector<team> teams = playcontroller.get_teams_const();
	int persistent_teams = 0;
	BOOST_FOREACH(const team &t, teams) {
		if (t.persistent()){
			++persistent_teams;
		}
	}

	if (persistent_teams > 0 && (has_next_scenario ||
			gamestate.classification().campaign_type == "test"))
	{
		gamemap map = playcontroller.get_map_const();
		int finishing_bonus_per_turn =
				map.villages().size() * game_config::village_income +
				game_config::base_income;
		tod_manager tod = playcontroller.get_tod_manager_const();
		int turns_left = std::max<int>(0, tod.number_of_turns() - tod.turn());
		int finishing_bonus = (end_level.gold_bonus && turns_left > -1) ?
				finishing_bonus_per_turn * turns_left : 0;


		BOOST_FOREACH(const team &t, teams)
		{
			if (!t.persistent()){
				continue;
			} else if (t.lost()) {
				sides.remove_side(t.save_id());
				continue;
			}
			int carryover_gold = div100rounded((t.gold() + finishing_bonus) * end_level.carryover_percentage);
			sides.transfer_from(t, carryover_gold);

			if (!t.is_human()){
				continue;
			}

			if (persistent_teams > 1) {
				report << "\n<b>" << t.current_player() << "</b>\n";
			}

			playcontroller.report_victory(report, carryover_gold, t.gold(), finishing_bonus_per_turn, turns_left, finishing_bonus);
		}
	}

	if (end_level.transient.carryover_report) {
		gui2::show_transient_message(disp.video(), title, report.str(), "", true);
	}

	gamestate.carryover_sides_start = sides.to_config();
}

static void generate_scenario(config const*& scenario)
{
	LOG_G << "randomly generating scenario...\n";
	const cursor::setter cursor_setter(cursor::WAIT);

	static config new_scenario;
	new_scenario = random_generate_scenario((*scenario)["scenario_generation"],
		scenario->child("generator"));

	//TODO comment or remove
	//level_ = scenario;
	//merge carryover information into the newly generated scenario

	scenario = &new_scenario;
}

static void generate_map(config const*& scenario)
{
	LOG_G << "randomly generating map...\n";
	const cursor::setter cursor_setter(cursor::WAIT);

	const std::string map_data = random_generate_map(
		(*scenario)["map_generation"], scenario->child("generator"));

	// Since we've had to generate the map,
	// make sure that when we save the game,
	// it will not ask for the map to be generated again on reload
	static config new_scenario;
	new_scenario = *scenario;
	new_scenario["map_data"] = map_data;
	scenario = &new_scenario;
}

void play_replay(display& disp, game_state& gamestate, const config& game_config,
		CVideo& video)
{
	std::string type = gamestate.classification().campaign_type;
	if(type.empty())
		type = "scenario";

	// 'starting_pos' will contain the position we start the game from.
	config starting_pos;

	if (gamestate.replay_start().empty()){
		// Backwards compatibility code for 1.2 and 1.2.1
		const config &scenario = game_config.find_child(type,"id",gamestate.carryover_sides_start["next_scenario"]);
		assert(scenario);
		gamestate.replay_start() = scenario;
	}
	starting_pos = gamestate.replay_start();

	//for replays, use the variables specified in starting_pos
	if (const config &vars = starting_pos.child("variables")) {
		gamestate.carryover_sides_start.child_or_add("variables") = vars;
	}

	try {
		// Preserve old label eg. replay
		if (gamestate.classification().label.empty())
			gamestate.classification().label = starting_pos["name"].str();
		//if (gamestate.abbrev.empty())
		//	gamestate.abbrev = (*scenario)["abbrev"];

		play_replay_level(game_config, &starting_pos, video, gamestate);

		gamestate.snapshot = config();
		recorder.clear();
		gamestate.replay_data.clear();

	} catch(game::load_game_failed& e) {
		gui2::show_error_message(disp.video(), _("The game could not be loaded: ") + e.message);
	} catch(game::game_error& e) {
		gui2::show_error_message(disp.video(), _("Error while playing the game: ") + e.message);
	} catch(incorrect_map_format_error& e) {
		gui2::show_error_message(disp.video(), std::string(_("The game map could not be loaded: ")) + e.message);
	} catch(twml_exception& e) {
		e.show(disp);
	}
}

static LEVEL_RESULT playsingle_scenario(const config& game_config,
		const config* level, display& disp, game_state& state_of_game,
		const config::const_child_itors &story,
		bool skip_replay, end_level_data &end_level)
{
	const int ticks = SDL_GetTicks();
	int num_turns = (*level)["turns"].to_int(-1);

	config init_level = *level;
	do_carryover_WML(init_level, state_of_game);
	team_init(init_level, state_of_game);
	clear_carryover_WML(state_of_game);

	LOG_NG << "creating objects... " << (SDL_GetTicks() - ticks) << "\n";
	playsingle_controller playcontroller(init_level, state_of_game, ticks, num_turns, game_config, disp.video(), skip_replay);
	LOG_NG << "created objects... " << (SDL_GetTicks() - playcontroller.get_ticks()) << "\n";

	LEVEL_RESULT res = playcontroller.play_scenario(story, skip_replay);
	end_level = playcontroller.get_end_level_data_const();

	if (state_of_game.carryover_sides.has_child("end_level_data")) {
		config& cfg_end_level = state_of_game.carryover_sides.child("end_level_data");
		end_level.write(cfg_end_level);
	} else {
		config& cfg_end_level = state_of_game.carryover_sides.add_child("end_level_data");
		end_level.write(cfg_end_level);
	}

	if (res == DEFEAT) {
		if (resources::persist != NULL)
			resources::persist->end_transaction();
		gui2::show_transient_message(disp.video(),
				    _("Defeat"),
				    _("You have been defeated!")
				    );
	}
	else if(res == VICTORY){
		store_carryover(state_of_game, playcontroller, disp, end_level);
	}

	if (!disp.video().faked() && res != QUIT && end_level.transient.linger_mode)
	{
		try {
			playcontroller.linger();
		} catch(end_level_exception& e) {
			if (e.result == QUIT) {
				return QUIT;
			}
		}
	}

	return res;
}


static LEVEL_RESULT playmp_scenario(const config& game_config,
		const config* level, display& disp, game_state& state_of_game,
		const config::const_child_itors &story, bool skip_replay,
		bool blindfold_replay, io_type_t& io_type, end_level_data &end_level)
{
	const int ticks = SDL_GetTicks();
	int num_turns = (*level)["turns"].to_int(-1);

	config init_level = *level;
	do_carryover_WML(init_level, state_of_game);
	team_init(init_level, state_of_game);
	clear_carryover_WML(state_of_game);

	playmp_controller playcontroller(init_level, state_of_game, ticks, num_turns,
		game_config, disp.video(), skip_replay, blindfold_replay, io_type == IO_SERVER);
	LEVEL_RESULT res = playcontroller.play_scenario(story, skip_replay);
	end_level = playcontroller.get_end_level_data_const();

	if (state_of_game.carryover_sides.has_child("end_level_data")) {
		config& cfg_end_level = state_of_game.carryover_sides.child("end_level_data");
		end_level.write(cfg_end_level);
	} else {
		config& cfg_end_level = state_of_game.carryover_sides.add_child("end_level_data");
		end_level.write(cfg_end_level);
	}

	//Check if the player started as mp client and changed to host
	if (io_type == IO_CLIENT && playcontroller.is_host())
		io_type = IO_SERVER;

	if (res == DEFEAT) {
		if (resources::persist != NULL)
			resources::persist->end_transaction();
		gui2::show_transient_message(disp.video(),
				    _("Defeat"),
				    _("You have been defeated!")
				    );
	}
	else if(res == VICTORY){
		store_carryover(state_of_game, playcontroller, disp, end_level);
	}
	else if(res == OBSERVER_END){
		state_of_game.carryover_sides_start["next_scenario"] = resources::gamedata->next_scenario();
	}

	if (!disp.video().faked() && res != QUIT) {
		if (!end_level.transient.linger_mode) {
			if(!playcontroller.is_host()) {
				// If we continue without lingering we need to
				// make sure the host uploads the next scenario
				// before we attempt to download it.
				playcontroller.wait_for_upload();
			}
		} else {
			try {
				playcontroller.linger();
			} catch(end_level_exception& e) {
				if (e.result == QUIT) {
					return QUIT;
				}
			}
		}
	}

	return res;
}

LEVEL_RESULT play_game(game_display& disp, game_state& gamestate,
	const config& game_config, io_type_t io_type, bool skip_replay, 
	bool network_game, bool blindfold_replay)
{
	std::string type = gamestate.classification().campaign_type;
	if(type.empty())
		type = "scenario";

	config const* scenario = NULL;

	// 'starting_pos' will contain the position we start the game from.
	config starting_pos;

	carryover_info sides = carryover_info(gamestate.carryover_sides_start);

	// Do we have any snapshot data?
	// yes => this must be a savegame
	// no  => we are starting a fresh scenario
	if (!gamestate.snapshot.child("side") || !recorder.at_end())
	{
		gamestate.classification().completion = "running";
		// Campaign or Multiplayer?
		// If the gamestate already contains a starting_pos,
		// then we are starting a fresh multiplayer game.
		// Otherwise this is the start of a campaign scenario.
		if(gamestate.replay_start()["id"].empty() == false) {
			starting_pos = gamestate.replay_start();
			scenario = &starting_pos;

			if(gamestate.replay_start()["random_seed"] != gamestate.carryover_sides_start["random_seed"]){
				sides = carryover_info(gamestate.replay_start());
			}

		} else {
			//reload of the scenario, as starting_pos contains carryover information only
			LOG_G << "loading scenario: '" << sides.next_scenario() << "'\n";
			scenario = &game_config.find_child(type, "id", sides.next_scenario());

			if(!*scenario){
				scenario = NULL;
			}
			LOG_G << "scenario found: " << (scenario != NULL ? "yes" : "no") << "\n";

		}
	} else {
		// This game was started from a savegame
		LOG_G << "loading snapshot...\n";
		starting_pos = gamestate.replay_start();
		scenario = &gamestate.snapshot;
		// When starting wesnoth --multiplayer there might be
		// no variables which leads to a segfault
		if (const config &vars = gamestate.snapshot.child("variables")) {
			sides.set_variables(vars);
		}
		sides.get_wml_menu_items().set_menu_items(gamestate.snapshot);
		// Replace game label with that from snapshot
		if (!gamestate.snapshot["label"].empty()){
			gamestate.classification().label = gamestate.snapshot["label"].str();
		}
	}

	gamestate.carryover_sides_start = sides.to_config();

	while(scenario != NULL) {
		// If we are a multiplayer client, tweak the controllers
		// (actually, moved to server. do we still need this starting_pos thing?)
		if(io_type == IO_CLIENT) {
			if(scenario != &starting_pos) {
				starting_pos = *scenario;
				scenario = &starting_pos;
			}
		}

		config::const_child_itors story = scenario->child_range("story");
		//TODO: remove once scenario in carryover_info/gamedata is confirmed
//		gamestate.classification().next_scenario = (*scenario)["next_scenario"].str();

		bool save_game_after_scenario = true;

		LEVEL_RESULT res = VICTORY;
		end_level_data end_level;

		try {
			// Preserve old label eg. replay
			if (gamestate.classification().label.empty()) {
				if (gamestate.classification().abbrev.empty())
					gamestate.classification().label = (*scenario)["name"].str();
				else {
					gamestate.classification().label = gamestate.classification().abbrev;
					gamestate.classification().label.append("-");
					gamestate.classification().label.append((*scenario)["name"]);
				}
			}

			// If the entire scenario should be randomly generated
			if((*scenario)["scenario_generation"] != "") {
				generate_scenario(scenario);
			}
			std::string map_data = (*scenario)["map_data"];
			if(map_data.empty() && (*scenario)["map"] != "") {
				map_data = read_map((*scenario)["map"]);
			}

			// If the map should be randomly generated
			if(map_data.empty() && (*scenario)["map_generation"] != "") {
				generate_map(scenario);
			}

			sound::empty_playlist();

			//add the variables to the starting_pos unless they are already there
			const config &wmlvars = gamestate.replay_start().child("variables");
			if (!wmlvars || wmlvars.empty()){
				gamestate.replay_start().clear_children("variables");
				gamestate.replay_start().add_child("variables", gamestate.carryover_sides_start.child_or_empty("variables"));
			}

			switch (io_type){
			case IO_NONE:
				res = playsingle_scenario(game_config, scenario, disp, gamestate, story, skip_replay, end_level);
				break;
			case IO_SERVER:
			case IO_CLIENT:
				res = playmp_scenario(game_config, scenario, disp, gamestate, story, skip_replay, blindfold_replay, io_type, end_level);
				break;
			}
		} catch(game::load_game_failed& e) {
			gui2::show_error_message(disp.video(), _("The game could not be loaded: ") + e.message);
			return QUIT;
		} catch(game::game_error& e) {
			gui2::show_error_message(disp.video(), _("Error while playing the game: ") + e.message);
			return QUIT;
		} catch(incorrect_map_format_error& e) {
			gui2::show_error_message(disp.video(), std::string(_("The game map could not be loaded: ")) + e.message);
			return QUIT;
		} catch(config::error& e) {
			std::cerr << "caught config::error...\n";
			gui2::show_error_message(disp.video(), _("Error while reading the WML: ") + e.message);
			return QUIT;
		} catch(twml_exception& e) {
			e.show(disp);
			return QUIT;
		}

		// Save-management options fire on game end.
		// This means: (a) we have a victory, or
		// or (b) we're multiplayer live, in which
		// case defeat is also game end.  Someday,
		// if MP campaigns ever work again, we might
		// need to change this test.
		if (res == VICTORY || (io_type != IO_NONE && res == DEFEAT)) {
			if (preferences::delete_saves())
				savegame::clean_saves(gamestate.classification().label);

			if (preferences::save_replays() && end_level.replay_save) {
				savegame::replay_savegame save(gamestate, preferences::save_compression_format());
				save.save_game_automatic(disp.video(), true);
			}
		}

		recorder.clear();
		gamestate.replay_data.clear();
		gamestate.replay_start().clear();

		// On DEFEAT, QUIT, or OBSERVER_END, we're done now
		if (res != VICTORY)
		{
			if (res != OBSERVER_END || gamestate.carryover_sides_start["next_scenario"].empty()) {
				gamestate.snapshot = config();
				return res;
			}

			const int dlg_res = gui2::show_message(disp.video(), _("Game Over"),
				_("This scenario has ended. Do you want to continue the campaign?"),
				gui2::tmessage::yes_no_buttons);

			if(dlg_res == gui2::twindow::CANCEL) {
				gamestate.snapshot = config();
				return res;
			}
		}

		// Continue without saving is like a victory,
		// but the save game dialog isn't displayed
		if (!end_level.prescenario_save)
			save_game_after_scenario = false;

		//TODO: remove once scenario in carryover_info/gamedata is confirmed
		// Switch to the next scenario.
		//gamestate.classification().scenario = gamestate.classification().next_scenario;

		sides = carryover_info(gamestate.carryover_sides_start);
		sides.rng().rotate_random();
		gamestate.carryover_sides_start = sides.to_config();

		if (io_type == IO_CLIENT) {
			// Opens mp::connect dialog to get a new gamestate.
			mp::ui::result wait_res = mp::goto_mp_wait(gamestate, disp,
				game_config, res == OBSERVER_END);
			if (wait_res == mp::ui::QUIT) {
				gamestate.snapshot = config();
				return QUIT;
			}

			starting_pos = gamestate.replay_start();
			gamestate = game_state(starting_pos);
			// Retain carryover_sides_start, as the config from the server
			// doesn't contain it.
			gamestate.carryover_sides_start = sides.to_config();
		} else {
			// Retrieve next scenario data.
			scenario = &game_config.find_child(type, "id",
				gamestate.carryover_sides_start["next_scenario"]);
			if (!*scenario) {
				scenario = NULL;
			} else {
				starting_pos = *scenario;
				scenario = &starting_pos;
			}

			if (io_type == IO_SERVER && scenario != NULL) {
				mp_game_settings& params = gamestate.mp_settings();

				// A hash have to be generated using an unmodified
				// scenario data.
				params.hash = scenario->hash();

				// Apply carryover before passing a scenario data to the
				// mp::connect_engine.
				team_init(starting_pos, gamestate);

				//We don't merge WML until start of next scenario, but if we want to allow user to disable MP ui in transition,
				//then we have to move "allow_new_game" attribute over now.
				bool allow_new_game_flag = (*scenario)["allow_new_game"].to_bool(true);

				if (gamestate.carryover_sides_start.child_or_empty("end_level_data").child_or_empty("next_scenario_settings").has_attribute("allow_new_game")) {
					allow_new_game_flag = gamestate.carryover_sides_start.child_or_empty("end_level_data").child("next_scenario_settings")["allow_new_game"].to_bool();
				}

				params.scenario_data = *scenario;
				params.mp_scenario = (*scenario)["id"].str();
				params.mp_scenario_name = (*scenario)["name"].str();
				params.num_turns = (*scenario)["turns"].to_int(-1);
				params.saved_game = false;
				params.use_map_settings =
					(*scenario)["force_lock_settings"].to_bool();

				mp::connect_engine_ptr
					connect_engine(new mp::connect_engine(disp, gamestate,
						params, !network_game, false));

				if (allow_new_game_flag || (game_config::debug && network::nconnections() == 0)) {
					// Opens mp::connect dialog to allow users to
					// make an adjustments for scenario.
					mp::ui::result connect_res = mp::goto_mp_connect(disp,
						*connect_engine, game_config, params.name);
					if (connect_res == mp::ui::QUIT) {
						return QUIT;
					}
				} else {
					// Start the next scenario immediately.
					connect_engine->
						start_game(mp::connect_engine::FORCE_IMPORT_USERS);
				}

				starting_pos = gamestate.replay_start();
				scenario = &starting_pos;

				// TODO: move this code to mp::connect_engine
				// in order to send generated data to the network
				// before starting the game.
				//
				// If the entire scenario should be randomly generated
				/*if((*scenario)["scenario_generation"] != "") {
					generate_scenario(scenario);
				}

				std::string map_data = (*scenario)["map_data"];
				if(map_data.empty() && (*scenario)["map"] != "") {
					map_data = read_map((*scenario)["map"]);
				}

				// If the map should be randomly generated
				if(map_data.empty() && (*scenario)["map_generation"] != "") {
					generate_map(scenario);
				}*/
			}
		}

		if(scenario != NULL) {
			// Update the label
			if (gamestate.classification().abbrev.empty())
				gamestate.classification().label = (*scenario)["name"].str();
			else {
				gamestate.classification().label = gamestate.classification().abbrev;
				gamestate.classification().label.append("-");
				gamestate.classification().label.append((*scenario)["name"]);
			}

			// If this isn't the last scenario, then save the game
			if(save_game_after_scenario) {

				// For multiplayer, we want the save
				// to contain the starting position.
				// For campaigns however, this is the
				// start-of-scenario save and the
				// starting position needs to be empty,
				// to force a reload of the scenario config.

				savegame::scenariostart_savegame save(gamestate, preferences::save_compression_format());

				save.save_game_automatic(disp.video());
			}

		}
		gamestate.snapshot = config();
	}

	if (!gamestate.carryover_sides_start["next_scenario"].empty() && gamestate.carryover_sides_start["next_scenario"] != "null") {
		std::string message = _("Unknown scenario: '$scenario|'");
		utils::string_map symbols;
		symbols["scenario"] = gamestate.carryover_sides_start["next_scenario"];
		message = utils::interpolate_variables_into_string(message, &symbols);
		gui2::show_error_message(disp.video(), message);
		return QUIT;
	}

	if (gamestate.classification().campaign_type == "scenario"){
		if (preferences::delete_saves())
			savegame::clean_saves(gamestate.classification().label);
	}
	return VICTORY;
}

