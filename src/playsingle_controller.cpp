/*
	Copyright (C) 2006 - 2022
	by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
	Copyright (C) 2003 by David White <dave@whitevine.net>
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
 *  Logic for single-player game.
 */

#include "playsingle_controller.hpp"

#include "actions/undo.hpp"
#include "ai/manager.hpp"
#include "ai/testing.hpp"
#include "display_chat_manager.hpp"
#include "events.hpp"
#include "formula/string_utils.hpp"
#include "game_end_exceptions.hpp"
#include "game_events/pump.hpp"
#include "gettext.hpp"
#include "gui/dialogs/story_viewer.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "hotkey/hotkey_handler_sp.hpp"
#include "hotkey/hotkey_item.hpp"
#include "log.hpp"
#include "map/label.hpp"
#include "map/map.hpp"
#include "playturn.hpp"
#include "preferences/game.hpp"
#include "random_deterministic.hpp"
#include "replay_helper.hpp"
#include "resources.hpp"
#include "savegame.hpp"
#include "scripting/plugins/context.hpp"
#include "sound.hpp"
#include "soundsource.hpp"
#include "statistics.hpp"
#include "synced_context.hpp"
#include "units/unit.hpp"
#include "video.hpp"
#include "wesnothd_connection_error.hpp"
#include "whiteboard/manager.hpp"

#include <boost/dynamic_bitset.hpp>

static lg::log_domain log_aitesting("ai/testing");
#define LOG_AIT LOG_STREAM(info, log_aitesting)
// If necessary, this define can be replaced with `#define LOG_AIT std::cout` to restore previous behavior

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)

static lg::log_domain log_enginerefac("enginerefac");
#define LOG_RG LOG_STREAM(info, log_enginerefac)

playsingle_controller::playsingle_controller(const config& level, saved_game& state_of_game, bool skip_replay)
	: play_controller(level, state_of_game, skip_replay, true) // start faded
	, cursor_setter_(cursor::NORMAL)
	, textbox_info_()
	, replay_sender_(*resources::recorder)
	, network_reader_([this](config& cfg) { return receive_from_wesnothd(cfg); })
	, turn_data_(replay_sender_, network_reader_)
	, end_turn_(END_TURN_NONE)
	, skip_next_turn_(false)
	, ai_fallback_(false)
	, replay_controller_()
{
	// upgrade hotkey handler to the sp (whiteboard enabled) version
	hotkey_handler_ = std::make_unique<hotkey_handler>(*this, saved_game_);

	// game may need to start in linger mode
	linger_ = is_regular_game_end();

	plugins_context_->set_accessor_string("level_result", std::bind(&playsingle_controller::describe_result, this));
	plugins_context_->set_accessor_int("turn", std::bind(&play_controller::turn, this));
}

std::string playsingle_controller::describe_result() const
{
	if(!is_regular_game_end()) {
		return "NONE";
	} else if(get_end_level_data().is_victory) {
		return "VICTORY";
	} else {
		return "DEFEAT";
	}
}

void playsingle_controller::init_gui()
{
	LOG_NG << "Initializing GUI... " << (SDL_GetTicks() - ticks());
	play_controller::init_gui();

	// Scroll to the starting position of the first team. If there is a
	// human team, use that team; otherwise use team 1. If the map defines
	// a starting position for the selected team, scroll to that tile. Note
	// this often does not matter since many scenario start with messages,
	// which will usually scroll to the speaker. Also note that the map
	// does not necessarily define the starting positions. While usually
	// best to use the map, the scenarion may explicitly set the positions,
	// overriding those found in the map (if any).
	if(map_start_.valid()) {
		gui_->scroll_to_tile(map_start_, game_display::WARP, false);
		LOG_NG << "Found good stored ui location " << map_start_;
	} else {
		int scroll_team = gamestate().first_human_team_ + 1;
		if(scroll_team == 0) {
			scroll_team = 1;
		}

		map_location loc(get_map().starting_position(scroll_team));
		if((loc.x >= 0) && (loc.y >= 0)) {
			gui_->scroll_to_tile(loc, game_display::WARP);
			LOG_NG << "Found bad stored ui location " << map_start_ << " using side starting location " << loc;
		} else {
			LOG_NG << "Found bad stored ui location";
		}
	}

	// Fade in
	gui_->set_prevent_draw(false);
	gui_->queue_repaint();
	if(!video::headless() && !video::testing()) {
		gui_->fade_to({0,0,0,0}, 500);
	} else {
		gui_->set_fade({0,0,0,0});
	}

	get_hotkey_command_executor()->set_button_state();
}

void playsingle_controller::play_scenario_init()
{
	// At the beginning of the scenario, save a snapshot as replay_start
	if(saved_game_.replay_start().empty()) {
		saved_game_.replay_start() = to_config();
	}

	start_game();

	if(!saved_game_.classification().random_mode.empty() && is_networked_mp()) {
		// This won't cause errors later but we should notify the user about it in case he didn't knew it.
		gui2::show_transient_message(
			// TODO: find a better title
			_("Game Error"),
			_("This multiplayer game uses an alternative random mode, if you don't know what this message means, then "
			  "most likely someone is cheating or someone reloaded a corrupt game."));
	}

	return;
}

void playsingle_controller::play_scenario_main_loop()
{
	LOG_NG << "starting main loop\n" << (SDL_GetTicks() - ticks());

	ai_testing::log_game_start();
	if(get_teams().empty()) {
		ERR_NG << "Playing game with 0 teams.";
	}

	while(true) {
		try {
			play_turn();
			if(is_regular_game_end()) {
				turn_data_.send_data();
				return;
			}
		} catch(const reset_gamestate_exception& ex) {
			//
			// TODO:
			//
			// The MP replay feature still doesn't work properly (causes OOS)
			// because:
			//
			// 1) The undo stack is not reset along with the gamestate (fixed).
			// 2) The server_request_number_ is not reset along with the
			//    gamestate (fixed).
			// 3) chat and other unsynced actions are inserted in the middle of
			//    the replay bringing the replay_pos in unorder (fixed).
			// 4) untracked changes in side controllers are lost when resetting
			//    gamestate (fixed).
			// 5) The game should have a stricter check for whether the loaded
			//    game is actually a parent of this game.
			// 6) If an action was undone after a game was saved it can cause
			//    OOS if the undone action is in the snapshot of the saved
			//    game (luckily this is never the case for autosaves).
			//
			boost::dynamic_bitset<> local_players;
			local_players.resize(get_teams().size(), true);
			// Preserve side controllers, because we won't get the side controoller updates again when replaying.
			for(std::size_t i = 0; i < local_players.size(); ++i) {
				local_players[i] = get_teams()[i].is_local();
			}

			if(ex.start_replay) {
				// MP "Back to turn"
				statistics::read_stats(*ex.stats_);
			} else {
				// SP replay
				statistics::reset_current_scenario();
			}

			reset_gamestate(*ex.level, (*ex.level)["replay_pos"]);

			for(std::size_t i = 0; i < local_players.size(); ++i) {
				resources::gameboard->teams()[i].set_local(local_players[i]);
			}

			play_scenario_init();

			if(replay_controller_ == nullptr) {
				replay_controller_ = std::make_unique<replay_controller>(*this, false, ex.level, [this]() { on_replay_end(false); });
			}

			if(ex.start_replay) {
				replay_controller_->play_replay();
			}
		}
	} // end for loop
}

level_result::type playsingle_controller::play_scenario(const config& level)
{
	LOG_NG << "in playsingle_controller::play_scenario()...";

	// Start music.
	for(const config& m : level.child_range("music")) {
		sound::play_music_config(m, true);
	}

	sound::commit_music_changes();

	if(!is_skipping_replay() && !is_skipping_story()) {
		// Combine all the [story] tags into a single config. Handle this here since
		// storyscreen::controller doesn't have a default constructor.
		config cfg;
		for(const auto& iter : level.child_range("story")) {
			cfg.append_children(iter);
		}

		if(!cfg.empty()) {
			gui2::dialogs::story_viewer::display(get_scenario_name(), cfg);
		}
	}

	gui_->labels().read(level);

	// Read sound sources
	assert(soundsources_manager_ != nullptr);
	for(const config& s : level.child_range("sound_source")) {
		try {
			soundsource::sourcespec spec(s);
			soundsources_manager_->add(spec);
		} catch(const bad_lexical_cast&) {
			ERR_NG << "Error when parsing sound_source config: bad lexical cast.";
			ERR_NG << "sound_source config was: " << s.debug();
			ERR_NG << "Skipping this sound source...";
		}
	}

	LOG_NG << "entering try... " << (SDL_GetTicks() - ticks());

	try {
		play_scenario_init();
		// clears level config;
		saved_game_.remove_snapshot();

		if(!is_regular_game_end() && !linger_) {
			play_scenario_main_loop();
		}

		if(game_config::exit_at_end) {
			exit(0);
		}
		const bool is_victory = get_end_level_data().is_victory;

		//if(gamestate().gamedata_.phase() <= game_data::PRESTART) {
		//	video::clear_screen();
		//}

		ai_testing::log_game_end();

		const end_level_data& end_level = get_end_level_data();

		if(get_teams().empty()) {
			// store persistent teams
			saved_game_.set_snapshot(config());

			// this is probably only a story scenario, i.e. has its endlevel in the prestart event
			return level_result::type::victory;
		}

		if(linger_) {
			LOG_NG << "resuming from loaded linger state...";
			// as carryover information is stored in the snapshot, we have to re-store it after loading a linger state
			saved_game_.set_snapshot(config());
			if(!is_observer()) {
				persist_.end_transaction();
			}

			return level_result::type::victory;
		}

		pump().fire(is_victory ? "local_victory" : "local_defeat");

		{ // Block for set_scontext_synced_base
			set_scontext_synced_base sync;
			pump().fire(end_level.proceed_to_next_level ? level_result::victory : level_result::defeat);
			pump().fire("scenario_end");
		}

		if(end_level.proceed_to_next_level) {
			gamestate().board_.heal_all_survivors();
		}

		if(is_observer()) {
			gui2::show_transient_message(_("Game Over"), _("The game is over."));
			return level_result::type::observer_end;
		}

		// If we're a player, and the result is victory/defeat, then send
		// a message to notify the server of the reason for the game ending.
		send_to_wesnothd(config {
			"info", config {
				"type", "termination",
				"condition", "game over",
				"result", is_victory ? level_result::victory : level_result::defeat,
			},
		});

		// Play victory music once all victory events
		// are finished, if we aren't observers and the
		// carryover dialog isn't disabled.
		//
		// Some scenario authors may use 'continue'
		// result for something that is not story-wise
		// a victory, so let them use [music] tags
		// instead should they want special music.
		const std::string& end_music = select_music(is_victory);
		if((!is_victory || end_level.transient.carryover_report) && !end_music.empty()) {
			sound::empty_playlist();
			sound::play_music_once(end_music);
		}

		persist_.end_transaction();

		level_result::type res = level_result::get_enum(end_level.test_result).value_or(level_result::type::test_invalid);
		if(res == level_result::type::result_not_set) {
			return is_victory ? level_result::type::victory : level_result::type::defeat;
		} else {
			return res;
		}
	} catch(const savegame::load_game_exception&) {
		// Loading a new game is effectively a quit.
		saved_game_.clear();
		throw;
	} catch(const wesnothd_error& e) {
		scoped_savegame_snapshot snapshot(*this);
		savegame::ingame_savegame save(saved_game_, preferences::save_compression_format());
		if(e.message == "") {
			save.save_game_interactive(
				_("A network disconnection has occurred, and the game cannot continue. Do you want to save the game?"),
				savegame::savegame::YES_NO);
		} else {
			save.save_game_interactive(
				_("This game has been ended.\nReason: ") + e.message + _("\nDo you want to save the game?"),
				savegame::savegame::YES_NO);
		}

		if(dynamic_cast<const ingame_wesnothd_error*>(&e)) {
			return level_result::type::quit;
		} else {
			throw;
		}
	}
}

void playsingle_controller::play_idle_loop()
{
	while(!should_return_to_play_side()) {
		play_slice_catch();
		SDL_Delay(10);
	}
}

void playsingle_controller::play_side_impl()
{
	if(!skip_next_turn_) {
		end_turn_ = END_TURN_NONE;
	}

	if(replay_controller_.get() != nullptr) {
		init_side_done_now_ = false;

		REPLAY_RETURN res = replay_controller_->play_side_impl();
		if(res == REPLAY_FOUND_END_TURN) {
			end_turn_ = END_TURN_SYNCED;
		}

		if(player_type_changed_) {
			replay_controller_.reset();
		}
	} else if((current_team().is_local_human() && current_team().is_proxy_human())) {
		LOG_NG << "is human...";
		// If a side is dead end the turn, but play at least side=1's
		// turn in case all sides are dead
		if(gamestate().board_.side_units(current_side()) == 0 && !(get_units().empty() && current_side() == 1)) {
			end_turn_ = END_TURN_REQUIRED;
		}

		before_human_turn();

		if(end_turn_ == END_TURN_NONE) {
			play_human_turn();
		}

		if(!player_type_changed_ && !is_regular_game_end()) {
			after_human_turn();
		}

		LOG_NG << "human finished turn...";
	} else if(current_team().is_local_ai() || (current_team().is_local_human() && current_team().is_droid())) {
		play_ai_turn();
	} else if(current_team().is_network()) {
		play_network_turn();
	} else if(current_team().is_local_human() && current_team().is_idle()) {
		end_turn_enable(false);
		do_idle_notification();
		before_human_turn();

		if(end_turn_ == END_TURN_NONE) {
			play_idle_loop();
		}
	} else {
		// we should have skipped over empty controllers before so this shouldn't be possible
		ERR_NG << "Found invalid side controller " << side_controller::get_string(current_team().controller()) << " ("
			   << side_proxy_controller::get_string(current_team().proxy_controller()) << ") for side " << current_team().side();
	}
}

void playsingle_controller::before_human_turn()
{
	log_scope("player turn");
	assert(!linger_);
	if(end_turn_ != END_TURN_NONE || is_regular_game_end()) {
		return;
	}

	if(init_side_done_now_ && !game_config::disable_autosave && preferences::autosavemax() > 0) {
		scoped_savegame_snapshot snapshot(*this);
		savegame::autosave_savegame save(saved_game_, preferences::save_compression_format());
		save.autosave(game_config::disable_autosave, preferences::autosavemax(), preferences::INFINITE_AUTO_SAVES);
	}

	if(preferences::turn_bell()) {
		sound::play_bell(game_config::sounds::turn_bell);
	}
}

void playsingle_controller::show_turn_dialog()
{
	if(preferences::turn_dialog() && !is_regular_game_end()) {
		blindfold b(*gui_, true); // apply a blindfold for the duration of this dialog
		gui_->queue_rerender();
		std::string message = _("It is now $name|’s turn");
		utils::string_map symbols;
		symbols["name"] = gamestate().board_.get_team(current_side()).side_name();
		message = utils::interpolate_variables_into_string(message, &symbols);
		gui2::show_transient_message("", message);
	}
}

void playsingle_controller::execute_gotos()
{
	if(should_return_to_play_side()) {
		return;
	}

	try {
		menu_handler_.execute_gotos(mouse_handler_, current_side());
	} catch(const return_to_play_side_exception&) {
	}
}

void playsingle_controller::play_human_turn()
{
	show_turn_dialog();

	if(!preferences::disable_auto_moves()) {
		execute_gotos();
	}

	end_turn_enable(true);

	while(!should_return_to_play_side()) {
		check_objectives();
		play_slice_catch();
	}
}

void playsingle_controller::linger()
{
	LOG_NG << "beginning end-of-scenario linger";
	linger_ = true;

	// If we need to set the status depending on the completion state
	// the key to it is here.
	gui_->set_game_mode(game_display::LINGER);

	// Make all of the able-to-move units' orbs consistently red
	gamestate().board_.set_all_units_user_end_turn();

	// change the end-turn button text to its alternate label
	gui_->get_theme().refresh_title2("button-endturn", "title2");
	gui_->queue_rerender();

	try {
		// Same logic as single-player human turn, but
		// *not* the same as multiplayer human turn.
		end_turn_enable(true);
		end_turn_ = END_TURN_NONE;
		while(end_turn_ == END_TURN_NONE) {
			play_slice();
		}
	} catch(const savegame::load_game_exception&) {
		// Loading a new game is effectively a quit.
		saved_game_.clear();
		throw;
	}

	// revert the end-turn button text to its normal label
	gui_->get_theme().refresh_title2("button-endturn", "title");
	gui_->queue_rerender();
	gui_->set_game_mode(game_display::RUNNING);

	LOG_NG << "ending end-of-scenario linger";
}

void playsingle_controller::end_turn_enable(bool /*enable*/)
{
	// TODO: this is really only needed to refresh the visual state of the buttons on each turn.
	// It looks like we can put it in play_side_impl, but it definitely should be removed from
	// here since other code already takes care of actually enabling/disabling the end turn button.
	get_hotkey_command_executor()->set_button_state();
}

void playsingle_controller::after_human_turn()
{
	// Clear moves from the GUI.
	gui_->set_route(nullptr);
	gui_->unhighlight_reach();
}

void playsingle_controller::play_ai_turn()
{
	LOG_NG << "is ai...";

	end_turn_enable(false);
	gui_->recalculate_minimap();

	const cursor::setter cursor_setter(cursor::WAIT);

	// Correct an oddball case where a human could have left delayed shroud
	// updates on before giving control to the AI. (The AI does not bother
	// with the undo stack, so it cannot delay shroud updates.)
	team& cur_team = current_team();
	if(!cur_team.auto_shroud_updates()) {
		// We just took control, so the undo stack is empty. We still need
		// to record this change for the replay though.
		synced_context::run_and_store("auto_shroud", replay_helper::get_auto_shroud(true));
	}

	undo_stack().clear();
	turn_data_.send_data();

	try {
		try {
			if(!should_return_to_play_side()) {
				ai::manager::get_singleton().play_turn(current_side());
			}
		} catch(const return_to_play_side_exception&) {
		} catch(const fallback_ai_to_human_exception&) {
			current_team().make_human();
			player_type_changed_ = true;
			ai_fallback_ = true;
		}
	} catch(...) {
		turn_data_.sync_network();
		throw;
	}

	if(!should_return_to_play_side()) {
		end_turn_ = END_TURN_REQUIRED;
	}

	turn_data_.sync_network();
	gui_->recalculate_minimap();
	gui_->invalidate_unit();
	gui_->invalidate_game_status();
	gui_->invalidate_all();
}

/**
 * Will handle sending a networked notification in descendent classes.
 */
void playsingle_controller::do_idle_notification()
{
	gui_->get_chat_manager().add_chat_message(std::time(nullptr), "Wesnoth", 0,
		"This side is in an idle state. To proceed with the game, the host must assign it to another controller.",
		events::chat_handler::MESSAGE_PUBLIC, false);
}

/**
 * Will handle networked turns in descendent classes.
 */
void playsingle_controller::play_network_turn()
{
	// There should be no networked sides in single-player.
	ERR_NG << "Networked team encountered by playsingle_controller.";
}

void playsingle_controller::handle_generic_event(const std::string& name)
{
	if(name == "ai_user_interact") {
		play_slice(false);
	}
}

void playsingle_controller::end_turn()
{
	if(linger_) {
		end_turn_ = END_TURN_REQUIRED;
	} else if(!is_browsing() && menu_handler_.end_turn(current_side())) {
		end_turn_ = END_TURN_REQUIRED;
	}
}

void playsingle_controller::force_end_turn()
{
	skip_next_turn_ = true;
	end_turn_ = END_TURN_REQUIRED;
}

void playsingle_controller::check_objectives()
{
	if(!get_teams().empty()) {
		const team& t = get_teams()[gui_->viewing_team()];

		if(!is_regular_game_end() && !is_browsing() && t.objectives_changed()) {
			show_objectives();
		}
	}
}

void playsingle_controller::maybe_linger()
{
	// mouse_handler expects at least one team for linger mode to work.
	assert(is_regular_game_end());
	if(get_end_level_data().transient.linger_mode && !get_teams().empty()) {
		linger();
	}
}

void playsingle_controller::sync_end_turn()
{
	// We cannot add [end_turn] to the recorder while executing another action.
	assert(synced_context::synced_state() == synced_context::UNSYNCED);

	if(end_turn_ == END_TURN_REQUIRED && current_team().is_local()) {
		// TODO: we should also send this immediately.
		resources::recorder->end_turn(gamestate_->next_player_number_);
		end_turn_ = END_TURN_SYNCED;
	}

	assert(end_turn_ == END_TURN_SYNCED);
	skip_next_turn_ = false;

	if(ai_fallback_) {
		current_team().make_ai();
		ai_fallback_ = false;
	}
}

void playsingle_controller::update_viewing_player()
{
	if(replay_controller_ && replay_controller_->is_controlling_view()) {
		replay_controller_->update_viewing_player();
	} else if(int side_num = play_controller::find_last_visible_team()) {
		// Update viewing team in case it has changed during the loop.
		if(side_num != gui_->viewing_side()) {
			update_gui_to_player(side_num - 1);
		}
	}
}

void playsingle_controller::reset_replay()
{
	if(replay_controller_ && replay_controller_->allow_reset_replay()) {
		replay_controller_->stop_replay();
		throw reset_gamestate_exception(replay_controller_->get_reset_state(), {}, false);
	} else {
		ERR_NG << "received invalid reset replay";
	}
}

void playsingle_controller::enable_replay(bool is_unit_test)
{
	replay_controller_ = std::make_unique<replay_controller>(
		*this,
 		gamestate().has_human_sides(),
		std::make_shared<config>(saved_game_.get_replay_starting_point()),
		std::bind(&playsingle_controller::on_replay_end, this, is_unit_test)
	);

	if(is_unit_test) {
		replay_controller_->play_replay();
	}
}

bool playsingle_controller::should_return_to_play_side() const
{
	if(player_type_changed_ || is_regular_game_end()) {
		return true;
	} else if(end_turn_ == END_TURN_NONE || replay_controller_.get() != 0 || current_team().is_network()) {
		return false;
	} else {
		return true;
	}
}

void playsingle_controller::on_replay_end(bool is_unit_test)
{
	if(is_networked_mp()) {
		set_player_type_changed();
	} else if(is_unit_test) {
		replay_controller_->return_to_play_side();
		if(!is_regular_game_end()) {
			end_level_data e;
			e.proceed_to_next_level = false;
			e.is_victory = false;
			set_end_level_data(e);
		}
	}
}
