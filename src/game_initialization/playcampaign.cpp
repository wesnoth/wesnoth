/*
	Copyright (C) 2005 - 2022
	by Philippe Plantier <ayin@anathas.org>
	Copyright (C) 2003 - 2005 by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "game_initialization/playcampaign.hpp"

#include "carryover.hpp"
#include "formula/string_utils.hpp"
#include "game_config.hpp"
#include "game_errors.hpp"
#include "game_initialization/connect_engine.hpp"
#include "game_initialization/mp_game_utils.hpp"
#include "game_initialization/multiplayer.hpp"
#include "generators/map_create.hpp"
#include "generators/map_generator.hpp"
#include "gettext.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/outro.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/retval.hpp"
#include "log.hpp"
#include "map/exception.hpp"
#include "map/map.hpp"
#include "persist_manager.hpp"
#include "playmp_controller.hpp"
#include "preferences/game.hpp"
#include "saved_game.hpp"
#include "savegame.hpp"
#include "sound.hpp"
#include "video.hpp"
#include "wesnothd_connection.hpp"
#include "wml_exception.hpp"

#define LOG_G LOG_STREAM(info, lg::general)

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

static lg::log_domain log_enginerefac("enginerefac");
#define LOG_RG LOG_STREAM(info, log_enginerefac)

void campaign_controller::show_carryover_message(
	playsingle_controller& playcontroller, const end_level_data& end_level, const level_result::type res)
{
	// We need to write the carryover amount to the team that's why we need non const
	std::vector<team>& teams = playcontroller.get_teams();

	// maybe this can be the case for scenario that only contain a story and end during the prestart event ?
	if(teams.size() < 1) {
		return;
	}

	std::ostringstream report;
	std::string title;

	bool obs = playcontroller.is_observer();

	if(obs) {
		title = _("Scenario Report");
	} else if(res == level_result::type::victory) {
		title = _("Victory");
		report << "<b>" << _("You have emerged victorious!") << "</b>";
	} else {
		title = _("Defeat");
		report << _("You have been defeated!");
	}

	const std::string& next_scenario = playcontroller.gamestate().get_game_data()->next_scenario();
	const bool has_next_scenario = !next_scenario.empty() && next_scenario != "null";

	int persistent_teams = 0;
	for(const team& t : teams) {
		if(t.persistent()) {
			++persistent_teams;
		}
	}

	if(persistent_teams > 0 && ((has_next_scenario && end_level.proceed_to_next_level) || state_.classification().is_test())) {
		const gamemap& map = playcontroller.get_map();
		const tod_manager& tod = playcontroller.get_tod_manager();

		const int turns_left = std::max<int>(0, tod.number_of_turns() - tod.turn());
		for(team& t : teams) {
			if(!t.persistent() || t.lost()) {
				continue;
			}

			const int finishing_bonus_per_turn = map.villages().size() * t.village_gold() + t.base_income();
			const int finishing_bonus = t.carryover_bonus() * finishing_bonus_per_turn * turns_left;

			t.set_carryover_gold(div100rounded((t.gold() + finishing_bonus) * t.carryover_percentage()));

			if(!t.is_local_human()) {
				continue;
			}

			if(persistent_teams > 1) {
				report << "\n\n<b>" << t.side_name() << "</b>";
			}

			report << "<small>\n" << _("Remaining gold: ") << utils::half_signed_value(t.gold()) << "</small>";

			if(t.carryover_bonus() != 0) {
				if(turns_left > -1) {
					report << "\n\n<b>" << _("Turns finished early: ") << turns_left << "</b>\n"
						<< "<small>" << _("Early finish bonus: ") << finishing_bonus_per_turn << _(" per turn") << "</small>\n"
						<< "<small>" << _("Total bonus: ") << finishing_bonus << "</small>\n";
				}

				report << "<small>" << _("Total gold: ") << utils::half_signed_value(t.gold() + finishing_bonus) << "</small>";
			}

			if(t.gold() > 0) {
				report << "\n<small>" << _("Carryover percentage: ") << t.carryover_percentage() << "</small>";
			}

			if(t.carryover_add()) {
				report << "\n\n<big><b>" << _("Bonus gold: ") << utils::half_signed_value(t.carryover_gold()) << "</b></big>";
			} else {
				report << "\n\n<big><b>" << _("Retained gold: ") << utils::half_signed_value(t.carryover_gold()) << "</b></big>";
			}

			std::string goldmsg;
			utils::string_map symbols;

			symbols["gold"] = lexical_cast_default<std::string>(t.carryover_gold());

			// Note that both strings are the same in English, but some languages will
			// want to translate them differently.
			if(t.carryover_add()) {
				if(t.carryover_gold() > 0) {
					goldmsg = VNGETTEXT(
						"You will start the next scenario with $gold on top of the defined minimum starting gold.",
						"You will start the next scenario with $gold on top of the defined minimum starting gold.",
						t.carryover_gold(), symbols
					);

				} else {
					goldmsg = VNGETTEXT(
						"You will start the next scenario with the defined minimum starting gold.",
						"You will start the next scenario with the defined minimum starting gold.",
						t.carryover_gold(), symbols
					);
				}
			} else {
				goldmsg = VNGETTEXT(
					"You will start the next scenario with $gold or its defined minimum starting gold, "
					"whichever is higher.",
					"You will start the next scenario with $gold or its defined minimum starting gold, "
					"whichever is higher.",
					t.carryover_gold(), symbols
				);
			}

			// xgettext:no-c-format
			report << "\n" << goldmsg;
		}
	}

	if(end_level.transient.carryover_report) {
		gui2::show_transient_message(title, report.str(), "", true);
	}
}

level_result::type campaign_controller::playsingle_scenario(end_level_data &end_level)
{
	const config& starting_point = is_replay_
		? state_.get_replay_starting_point()
		: state_.get_starting_point();

	playsingle_controller playcontroller(starting_point, state_, false);

	LOG_NG << "created objects... " << (SDL_GetTicks() - playcontroller.get_ticks());
	if(is_replay_) {
		playcontroller.enable_replay(is_unit_test_);
	}

	level_result::type res = playcontroller.play_scenario(starting_point);
	if(res == level_result::type::quit) {
		return level_result::type::quit;
	}

	if(!is_unit_test_) {
		is_replay_ = false;
	}

	if(is_replay_) {
		return res;
	}

	end_level = playcontroller.get_end_level_data();
	show_carryover_message(playcontroller, end_level, res);

	if(!video::headless()) {
		playcontroller.maybe_linger();
	}

	state_.set_snapshot(playcontroller.to_config());
	return res;
}

level_result::type campaign_controller::playmp_scenario(end_level_data &end_level)
{
	playmp_controller playcontroller(state_.get_starting_point(), state_, mp_info_);
	level_result::type res = playcontroller.play_scenario(state_.get_starting_point());

	// Check if the player started as mp client and changed to host
	if(res == level_result::type::quit) {
		return level_result::type::quit;
	}

	end_level = playcontroller.get_end_level_data();

	if(res != level_result::type::observer_end) {
		// We need to call this before linger because it prints the defeated/victory message.
		//(we want to see that message before entering the linger mode)
		show_carryover_message(playcontroller, end_level, res);
	}

	playcontroller.maybe_linger();
	playcontroller.update_savegame_snapshot();

	if(mp_info_) {
		mp_info_->connected_players = playcontroller.all_players();
		mp_info_->skip_replay = false;
		mp_info_->skip_replay_blindfolded = false;
	}

	return res;
}

level_result::type campaign_controller::play_game()
{
	if(is_replay_) {
		state_.get_replay().set_pos(0);
	} else {
		state_.get_replay().set_to_end();
	}

	state_.expand_scenario();

	while(state_.valid()) {
		level_result::type res = level_result::type::victory;
		end_level_data end_level;

		try {
			state_.expand_random_scenario();
			// In case this an mp scenario reloaded by sp this was not already done yet.
			state_.expand_mp_events();

			sound::empty_playlist();

			state_.expand_carryover();

			// expand_mp_options must be called after expand_carryover because expand_carryover will to set previous
			// variables if there are already variables in the [scenario]
			state_.expand_mp_options();

#if !defined(ALWAYS_USE_MP_CONTROLLER)
			if(!state_.classification().is_multiplayer() || is_replay_) {
				res = playsingle_scenario(end_level);
				if(is_replay_) {
					return res;
				}
			} else
#endif
			{
				res = playmp_scenario(end_level);
			}
		} catch(const leavegame_wesnothd_error&) {
			LOG_NG << "The game was remotely ended";
			return level_result::type::quit;
		} catch(const game::load_game_failed& e) {
			gui2::show_error_message(_("The game could not be loaded: ") + e.message);
			return level_result::type::quit;
		} catch(const quit_game_exception&) {
			LOG_NG << "The game was aborted";
			return level_result::type::quit;
		} catch(const game::game_error& e) {
			gui2::show_error_message(_("Error while playing the game: ") + e.message);
			return level_result::type::quit;
		} catch(const incorrect_map_format_error& e) {
			gui2::show_error_message(_("The game map could not be loaded: ") + e.message);
			return level_result::type::quit;
		} catch(const mapgen_exception& e) {
			gui2::show_error_message(_("Map generator error: ") + e.message);
		} catch(const config::error& e) {
			gui2::show_error_message(_("Error while reading the WML: ") + e.message);
			return level_result::type::quit;
		} catch(const wml_exception& e) {
			e.show();
			return level_result::type::quit;
		}

		if(is_unit_test_) {
			return res;
		}

		if(res == level_result::type::quit) {
			return res;
		}

		// proceed_to_next_level <=> 'any human side received victory'
		// If 'any human side received victory' we do the Save-management options
		// Otherwise we are done now
		if(!end_level.proceed_to_next_level) {
			return res;
		}

		if(preferences::delete_saves()) {
			savegame::clean_saves(state_.classification().label);
		}

		if(preferences::save_replays() && end_level.replay_save) {
			savegame::replay_savegame save(state_, preferences::save_compression_format());
			save.save_game_automatic(true);
		}

		state_.convert_to_start_save();

		// If there is no next scenario we're done now.
		if(state_.get_scenario_id().empty()) {
			// Don't show The End for multiplayer scenarios.
			if(res == level_result::type::victory && !state_.classification().is_normal_mp_game()) {
				preferences::add_completed_campaign(
					state_.classification().campaign, state_.classification().difficulty);

				if(state_.classification().end_credits) {
					gui2::dialogs::outro::display(state_.classification());
				}
			}

			return res;
		} else if(res == level_result::type::observer_end && mp_info_ && !mp_info_->is_host) {
			const int dlg_res = gui2::show_message(_("Game Over"),
				_("This scenario has ended. Do you want to continue the campaign?"),
				gui2::dialogs::message::yes_no_buttons);

			if(dlg_res == gui2::retval::CANCEL) {
				return res;
			}
		}

		if(mp_info_ && !mp_info_->is_host) {
			// Opens join game dialog to get a new gamestate.
			if(!mp::goto_mp_wait(res == level_result::type::observer_end)) {
				return level_result::type::quit;
			}

			// The host should send the complete savegame now that also contains the carryover sides start.
		} else {
			// clear previous game content information
			// otherwise it keeps getting appended for each scenario resulting in incorrect data being sent to the server to be stored
			state_.mp_settings().addons.clear();
			// Retrieve next scenario data.
			state_.expand_scenario();

			if(state_.valid()) {
				// note that although starting_pos is const it might be changed by gamestate.some_non_const_operation()
				const config& starting_pos = state_.get_starting_point();

				const bool is_mp = state_.classification().is_normal_mp_game();
				state_.mp_settings().num_turns = starting_pos["turns"].to_int(-1);

				if(state_.mp_settings().saved_game == saved_game_mode::type::midgame) {
					state_.mp_settings().saved_game = saved_game_mode::type::scenaro_start;
				}

				state_.mp_settings().use_map_settings = starting_pos["force_lock_settings"].to_bool(!is_mp);

				ng::connect_engine connect_engine(state_, false, mp_info_);

				if(!connect_engine.can_start_game() || (game_config::debug && state_.classification().is_multiplayer())) {
					// Opens staging dialog to allow users to make an adjustments for scenario.
					if(!mp::goto_mp_staging(connect_engine)) {
						return level_result::type::quit;
					}
				} else {
					// Start the next scenario immediately.
					connect_engine.start_game();
				}
			}
		}

		if(state_.valid()) {
			// Update the label
			state_.update_label();

			// If this isn't the last scenario, then save the game
			if(end_level.prescenario_save) {
				// For multiplayer, we want the save to contain the starting position.
				// For campaigns however, this is the start-of-scenario save and the
				// starting position needs to be empty, to force a reload of the scenario config.
				savegame::scenariostart_savegame save(state_, preferences::save_compression_format());
				save.save_game_automatic();
			}
		}
	}

	if(!state_.get_scenario_id().empty()) {
		utils::string_map symbols;
		symbols["scenario"] = state_.get_scenario_id();

		std::string message = _("Unknown scenario: '$scenario|'");
		message = utils::interpolate_variables_into_string(message, &symbols);

		gui2::show_error_message(message);
		return level_result::type::quit;
	}

	if(state_.classification().is_scenario()) {
		if(preferences::delete_saves()) {
			savegame::clean_saves(state_.classification().label);
		}
	}

	return level_result::type::victory;
}
