/*
   Copyright (C) 2003-2005 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2015 by Philippe Plantier <ayin@anathas.org>
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

#include "playcampaign.hpp"

#include "carryover.hpp"
#include "game_config.hpp"
#include "game_errors.hpp"
#include "game_preferences.hpp"
#include "generators/map_create.hpp"
#include "generators/map_generator.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/window.hpp"
#include "persist_manager.hpp"
#include "playmp_controller.hpp"
#include "replay_controller.hpp"
#include "log.hpp"
#include "map_exception.hpp"
#include "mp_game_utils.hpp"
#include "multiplayer.hpp"
#include "connect_engine.hpp"
#include "dialogs.hpp"
#include "gettext.hpp"
#include "resources.hpp"
#include "savegame.hpp"
#include "saved_game.hpp"
#include "sound.hpp"
#include "terrain_type_data.hpp"
#include "wml_exception.hpp"
#include "formula_string_utils.hpp"

#include <boost/foreach.hpp>

#define LOG_G LOG_STREAM(info, lg::general)

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

static lg::log_domain log_enginerefac("enginerefac");
#define LOG_RG LOG_STREAM(info, log_enginerefac)

static void show_carryover_message(saved_game& gamestate, playsingle_controller& playcontroller, display& disp, const end_level_data& end_level, const LEVEL_RESULT res)
{
	assert(resources::teams);

	bool has_next_scenario = !resources::gamedata->next_scenario().empty() &&
			resources::gamedata->next_scenario() != "null";
	//maybe this can be the case for scenario that only contain a story and end during the prestart event ?
	if(resources::teams->size() < 1){
		return;
	}

	std::ostringstream report;
	std::string title;

	bool obs = playcontroller.is_observer();

	if (obs) {
		title = _("Scenario Report");
	} else if (res == LEVEL_RESULT::VICTORY) {
		title = _("Victory");
		report << "<b>" << _("You have emerged victorious!") << "</b>\n\n";
	} else {
		title = _("Defeat");
		report <<  _("You have been defeated!") << "\n";
	}

	//We need to write the carryover amount to the team thats why we need non const
	std::vector<team>& teams = *resources::teams;
	int persistent_teams = 0;
	BOOST_FOREACH(const team &t, teams) {
		if (t.persistent()){
			++persistent_teams;
		}
	}

	if (persistent_teams > 0 && ((has_next_scenario && end_level.proceed_to_next_level)||
			gamestate.classification().campaign_type == game_classification::CAMPAIGN_TYPE::TEST))
	{
		gamemap map = playcontroller.get_map_const();
		tod_manager tod = playcontroller.get_tod_manager_const();
		int turns_left = std::max<int>(0, tod.number_of_turns() - tod.turn());
		BOOST_FOREACH(team &t, teams)
		{
			if (!t.persistent() || t.lost())
			{
				continue;
			}
			int finishing_bonus_per_turn = map.villages().size() * t.village_gold() + t.base_income();
			int finishing_bonus = t.carryover_bonus() ? finishing_bonus_per_turn * turns_left : 0;
			t.set_carryover_gold(div100rounded((t.gold() + finishing_bonus) * t.carryover_percentage()));
			if(!t.is_local_human())
			{
				continue;
			}
			if (persistent_teams > 1) {
				report << "\n<b>" << t.current_player() << "</b>\n";
			}

			playcontroller.report_victory(report, t, finishing_bonus_per_turn, turns_left, finishing_bonus);
		}
	}

	if (end_level.transient.carryover_report) {
		gui2::show_transient_message(disp.video(), title, report.str(), "", true);
	}
}

LEVEL_RESULT play_replay(display& disp, saved_game& gamestate, const config& game_config,
		const tdata_cache & tdata, bool is_unit_test)
{
	gamestate.get_replay().set_pos(0);
	// 'starting_pos' will contain the position we start the game from.
	// this call also might expand [scenario] in case thatt there is no replay_start
	const config& starting_pos = gamestate.get_replay_starting_pos();

	try {
		// Preserve old label eg. replay
		if (gamestate.classification().label.empty())
			gamestate.classification().label = starting_pos["name"].str();
		//if (gamestate.abbrev.empty())
		//	gamestate.abbrev = (*scenario)["abbrev"];

		LEVEL_RESULT res = play_replay_level(game_config, tdata, disp.video(), gamestate, is_unit_test);

		return res;
	} catch(game::load_game_failed& e) {
		ERR_NG << std::string(_("The game could not be loaded: ")) + " (game::load_game_failed) " + e.message << std::endl;
		if (is_unit_test) {
			return LEVEL_RESULT::DEFEAT;
		} else {
			gui2::show_error_message(disp.video(), _("The game could not be loaded: ") + e.message);
		}
	
	} catch(quit_game_exception&) {
		LOG_NG << "The replay was aborted\n";
		return LEVEL_RESULT::QUIT;
	} catch(game::game_error& e) {
		ERR_NG << std::string(_("Error while playing the game: ")) + " (game::game_error) " + e.message << std::endl;
		if (is_unit_test) {
			return LEVEL_RESULT::DEFEAT;
		} else {
			gui2::show_error_message(disp.video(), std::string(_("Error while playing the game: ")) + e.message);
		}
	} catch(incorrect_map_format_error& e) {
		ERR_NG << std::string(_("The game map could not be loaded: ")) + " (incorrect_map_format_error) " + e.message << std::endl;
		if (is_unit_test) {
			return LEVEL_RESULT::DEFEAT;
		} else {
			gui2::show_error_message(disp.video(), std::string(_("The game map could not be loaded: ")) + e.message);
		}
	} catch(twml_exception& e) {
		ERR_NG << std::string("WML Exception: ") + e.user_message << std::endl;
		ERR_NG << std::string("Dev Message: ") + e.dev_message << std::endl;
		if (is_unit_test) {
			return LEVEL_RESULT::DEFEAT;
		} else {
			e.show(disp);
		}
	}
	//TODO: when can this happen?
	return LEVEL_RESULT::VICTORY;
}

static LEVEL_RESULT playsingle_scenario(const config& game_config,
		const tdata_cache & tdata,
		display& disp, saved_game& state_of_game,
		const config::const_child_itors &story,
		bool skip_replay, end_level_data &end_level)
{
	const int ticks = SDL_GetTicks();

	LOG_NG << "creating objects... " << (SDL_GetTicks() - ticks) << "\n";
	playsingle_controller playcontroller(state_of_game.get_starting_pos(), state_of_game, ticks, game_config, tdata, disp.video(), skip_replay);
	LOG_NG << "created objects... " << (SDL_GetTicks() - playcontroller.get_ticks()) << "\n";

	LEVEL_RESULT res = playcontroller.play_scenario(story);

	if (res == LEVEL_RESULT::QUIT)
	{
		return LEVEL_RESULT::QUIT;
	}

	end_level = playcontroller.get_end_level_data_const();

	show_carryover_message(state_of_game, playcontroller, disp, end_level, res);
	if(!disp.video().faked())
	{
		playcontroller.maybe_linger();
	}
	state_of_game.set_snapshot(playcontroller.to_config());
	return res;
}


static LEVEL_RESULT playmp_scenario(const config& game_config,
		const tdata_cache & tdata,
		display& disp, saved_game& state_of_game,
		const config::const_child_itors &story, bool skip_replay,
		std::set<std::string>& mp_players, bool blindfold_replay, io_type_t& io_type, end_level_data &end_level)
{
	const int ticks = SDL_GetTicks();

	playmp_controller playcontroller(state_of_game.get_starting_pos(), state_of_game, ticks,
		game_config, tdata, disp.video(), skip_replay, blindfold_replay, io_type == IO_SERVER);
	LEVEL_RESULT res = playcontroller.play_scenario(story);

	//Check if the player started as mp client and changed to host
	if (io_type == IO_CLIENT && playcontroller.is_host())
		io_type = IO_SERVER;

	if (res == LEVEL_RESULT::QUIT)
	{
		return LEVEL_RESULT::QUIT;
	}

	end_level = playcontroller.get_end_level_data_const();

	if(res != LEVEL_RESULT::OBSERVER_END)
	{
		//We need to call this before linger because it prints the defeated/victory message.
		//(we want to see that message before entering the linger mode)
		show_carryover_message(state_of_game, playcontroller, disp, end_level, res);
	}
	if(!disp.video().faked())
	{
		playcontroller.maybe_linger();
	}
	playcontroller.update_savegame_snapshot();
	mp_players = playcontroller.all_players();
	return res;
}

LEVEL_RESULT play_game(game_display& disp, saved_game& gamestate,
	const config& game_config, const tdata_cache & tdata, 
	io_type_t io_type, bool skip_replay,
	bool network_game, bool blindfold_replay, bool is_unit_test)
{
	gamestate.get_replay().set_to_end();

	gamestate.expand_scenario();

	game_classification::CAMPAIGN_TYPE game_type = gamestate.classification().campaign_type;

	while(gamestate.valid())
	{
		LEVEL_RESULT res = LEVEL_RESULT::VICTORY;
		end_level_data end_level;
		std::set<std::string> mp_players;
		try {

			gamestate.expand_random_scenario();
			//In case this an mp scenario reloaded by sp this was not already done yet.
			gamestate.expand_mp_events();

			sound::empty_playlist();

			gamestate.expand_carryover();
			//expand_mp_options must be called after expand_carryover because expand_carryover will to set previous variables if there are already variables in the [scenario]
			gamestate.expand_mp_options();

			config::const_child_itors story = gamestate.get_starting_pos().child_range("story");

#if !defined(ALWAYS_USE_MP_CONTROLLER)
			if (game_type != game_classification::CAMPAIGN_TYPE::MULTIPLAYER) {
				res = playsingle_scenario(game_config, tdata, disp, gamestate, story, skip_replay, end_level);
			} else 
#endif
			{
				res = playmp_scenario(game_config, tdata, disp, gamestate, story, skip_replay, mp_players, blindfold_replay, io_type, end_level);
			}
		} catch(game::load_game_failed& e) {
			gui2::show_error_message(disp.video(), _("The game could not be loaded: ") + e.message);
			return LEVEL_RESULT::QUIT;
		} catch(quit_game_exception&) {
			LOG_NG << "The game was aborted\n";
			return LEVEL_RESULT::QUIT;
		} catch(game::game_error& e) {
			gui2::show_error_message(disp.video(), _("Error while playing the game: ") + e.message);
			return LEVEL_RESULT::QUIT;
		} catch(incorrect_map_format_error& e) {
			gui2::show_error_message(disp.video(), std::string(_("The game map could not be loaded: ")) + e.message);
			return LEVEL_RESULT::QUIT;
		} catch (mapgen_exception& e) {
			gui2::show_error_message(disp.video(), std::string(_("Map generator error: ") + e.message));
		} catch(config::error& e) {
			gui2::show_error_message(disp.video(), _("Error while reading the WML: ") + e.message);
			return LEVEL_RESULT::QUIT;
		} catch(twml_exception& e) {
			e.show(disp);
			return LEVEL_RESULT::QUIT;
		}

		if (is_unit_test) {
			return res;
		}
		if(res == LEVEL_RESULT::QUIT) {
			return res;
		}
		// proceed_to_next_level <=> 'any human side recieved victory'
		// If 'any human side recieved victory' we do the Save-management options
		// Otherwise we are done now
		if(!end_level.proceed_to_next_level) {
			return res;
		}

		if (preferences::delete_saves()) {
			savegame::clean_saves(gamestate.classification().label);
		}
		if (preferences::save_replays() && end_level.replay_save) {
			savegame::replay_savegame save(gamestate, preferences::save_compression_format());
			save.save_game_automatic(disp.video(), true);
		}
		
		gamestate.convert_to_start_save();

		//If there is no next scenario we're done now.
		if(gamestate.get_scenario_id().empty())
		{
			return res;
		}
		else if(res == LEVEL_RESULT::OBSERVER_END)
		{
			// TODO: does it make sense to ask this question if we are currently the host?
			const int dlg_res = gui2::show_message(disp.video(), _("Game Over"),
				_("This scenario has ended. Do you want to continue the campaign?"),
				gui2::tmessage::yes_no_buttons);

			if(dlg_res == gui2::twindow::CANCEL) {
				return res;
			}
		}

		if (io_type == IO_CLIENT) {
			// Opens mp::connect dialog to get a new gamestate.
			mp::ui::result wait_res = mp::goto_mp_wait(gamestate, disp,
				game_config, res == LEVEL_RESULT::OBSERVER_END);
			if (wait_res == mp::ui::QUIT) {
				return LEVEL_RESULT::QUIT;
			}

			//The host should send the complete savegame now that also contains the carryvoer sides start.
		} else {
			// Retrieve next scenario data.
			gamestate.expand_scenario();

			if (gamestate.valid()) {
				//note that although starting_pos is const it might be changed by gamestate.some_non_const_operation()  .
				const config& starting_pos = gamestate.get_starting_pos();

				gamestate.mp_settings().num_turns = starting_pos["turns"].to_int(-1);
				gamestate.mp_settings().saved_game = false;
				gamestate.mp_settings().use_map_settings = starting_pos["force_lock_settings"].to_bool();

				ng::connect_engine_ptr
					connect_engine(new ng::connect_engine(gamestate, !network_game, false, mp_players));

				if (!connect_engine->can_start_game() || (game_config::debug && game_type == game_classification::CAMPAIGN_TYPE::MULTIPLAYER)) {
					// Opens mp::connect dialog to allow users to make an adjustments for scenario.
					mp::ui::result connect_res = mp::goto_mp_connect(disp,
						*connect_engine, game_config, gamestate.mp_settings().name);
					if (connect_res == mp::ui::QUIT) {
						return LEVEL_RESULT::QUIT;
					}
				} else {
					// Start the next scenario immediately.
					connect_engine->start_game();
				}
			}
		}

		if(gamestate.valid()) {
			// Update the label
			gamestate.update_label();

			// If this isn't the last scenario, then save the game
			if(end_level.prescenario_save) {

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
	}

	if (!gamestate.get_scenario_id().empty()) {
		std::string message = _("Unknown scenario: '$scenario|'");
		utils::string_map symbols;
		symbols["scenario"] = gamestate.get_scenario_id();
		message = utils::interpolate_variables_into_string(message, &symbols);
		gui2::show_error_message(disp.video(), message);
		return LEVEL_RESULT::QUIT;
	}

	if (game_type == game_classification::CAMPAIGN_TYPE::SCENARIO){
		if (preferences::delete_saves()) {
			savegame::clean_saves(gamestate.classification().label);
		}
	}
	return LEVEL_RESULT::VICTORY;
}

