/*
	Copyright (C) 2006 - 2024
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
#include "carryover_show_gold.hpp"
#include "formula/string_utils.hpp"
#include "game_end_exceptions.hpp"
#include "game_events/pump.hpp"
#include "gettext.hpp"
#include "gui/dialogs/story_viewer.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "hotkey/hotkey_handler_sp.hpp"
#include "log.hpp"
#include "map/label.hpp"
#include "map/map.hpp"
#include "preferences/preferences.hpp"
#include "replay_controller.hpp"
#include "replay_helper.hpp"
#include "resources.hpp"
#include "saved_game.hpp"
#include "savegame.hpp"
#include "scripting/plugins/context.hpp"
#include "sound.hpp"
#include "soundsource.hpp"
#include "synced_context.hpp"
#include "video.hpp"
#include "wesnothd_connection_error.hpp"
#include "whiteboard/manager.hpp"


static lg::log_domain log_aitesting("ai/testing");
#define LOG_AIT LOG_STREAM(info, log_aitesting)
// If necessary, this define can be replaced with `#define LOG_AIT std::cout` to restore previous behavior

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)

static lg::log_domain log_enginerefac("enginerefac");
#define LOG_RG LOG_STREAM(info, log_enginerefac)

playsingle_controller::playsingle_controller(const config& level, saved_game& state_of_game)
	: play_controller(level, state_of_game)
	, cursor_setter_(cursor::NORMAL)
	, end_turn_requested_(false)
	, ai_fallback_(false)
	, replay_controller_()
{
	// upgrade hotkey handler to the sp (whiteboard enabled) version
	hotkey_handler_ = std::make_unique<hotkey_handler>(*this, saved_game_);


	plugins_context_->set_accessor_string("level_result", std::bind(&playsingle_controller::describe_result, this));
	plugins_context_->set_accessor_int("turn", std::bind(&play_controller::turn, this));
}

///Defined here to reduce file includes.
playsingle_controller::~playsingle_controller() = default;


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
	// If we are retarting replay from linger mode.
	update_gui_linger();
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

		int scroll_team = find_viewing_side();
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

void playsingle_controller::play_scenario_init(const config& level)
{
	gui_->labels().read(level);
	update_viewing_player();

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

	// At the beginning of the scenario, save a snapshot as replay_start
	if(saved_game_.replay_start().empty()) {
		saved_game_.replay_start() = to_config();
	}

	fire_preload();
	gamestate().gamedata_.set_phase(game_data::read_phase(level));

	start_game();
	gamestate_->player_number_ = skip_empty_sides(gamestate_->player_number_).side_num;

	if(!get_teams().empty()) {
		init_side_begin();
		if(gamestate().in_phase(game_data::TURN_PLAYING)) {
			init_side_end();
		}
	}

	if(!saved_game_.classification().random_mode.empty() && is_networked_mp()) {
		// This won't cause errors later but we should notify the user about it in case he didn't knew it.
		gui2::show_transient_message(
			// TODO: find a better title
			_("Game Error"),
			_("This multiplayer game uses an alternative random mode, if you don't know what this message means, then "
			  "most likely someone is cheating or someone reloaded a corrupt game."));
	}
}

playsingle_controller::ses_result playsingle_controller::skip_empty_sides(int side_num)
{
	const int sides = static_cast<int>(get_teams().size());
	const int max = side_num + sides;

	for (; side_num != max; ++side_num) {
		int side_num_mod = modulo(side_num, sides, 1);
		if(!gamestate().board_.get_team(side_num_mod).is_empty()) {
			return { side_num_mod, side_num_mod != side_num };
		}
	}
	return { side_num, true };
}

void playsingle_controller::play_some()
{
	//TODO: Its still unclear to me when end_turn_requested_ should be reset, i guess the idea is
	//      in particular that in rare cases when the player looses control at the same time
	//      as he presses "end turn" and then regains control back, the "end turn" should be discarded?
	//One of the main reasonsy why this is here is probably also that play_controller has no access to it.
	end_turn_requested_ = gamestate().gamedata_.end_turn_forced();

	assert(is_regular_game_end() || gamestate().in_phase(game_data::TURN_STARTING_WAITING, game_data::TURN_PLAYING, game_data::TURN_ENDED, game_data::GAME_ENDED));

	if (!is_regular_game_end() && gamestate().in_phase(game_data::TURN_STARTING_WAITING, game_data::TURN_PLAYING)) {
		play_side();
		assert(is_regular_game_end() || gamestate().in_phase(game_data::TURN_ENDED));
	}

	if (!is_regular_game_end() && gamestate().in_phase(game_data::TURN_ENDED)) {
		finish_side_turn();
	}

	if (is_regular_game_end() && !gamestate().in_phase(game_data::GAME_ENDED)) {
		gamestate().gamedata_.set_phase(game_data::GAME_ENDING);
		do_end_level();
		gamestate().gamedata_.set_phase(game_data::GAME_ENDED);
	}

	if (gamestate().in_phase(game_data::GAME_ENDED)) {
		end_turn_requested_ = !get_end_level_data().transient.linger_mode || get_teams().empty() || video::headless();
		maybe_linger();
	}
}

void playsingle_controller::play_side()
{
	do {
		if(std::find_if(get_teams().begin(), get_teams().end(), [](const team& t) { return !t.is_empty(); }) == get_teams().end()){
			throw game::game_error("The scenario has no (non-empty) sides defined");
		}
		update_viewing_player();

		maybe_do_init_side();
		if(is_regular_game_end()) {
			return;
		}
		// This flag can be set by derived classes (in overridden functions).
		player_type_changed_ = false;


		play_side_impl();

		if(is_regular_game_end()) {
			return;
		}
	} while(player_type_changed_);

	// Keep looping if the type of a team (human/ai/networked) has changed mid-turn
	sync_end_turn();
}

void playsingle_controller::finish_side_turn()
{
	if(is_regular_game_end()) {
		return;
	}

	/// Make a copy, since the [end_turn] was already sent to to server any changes to
	//  next_player_number by wml would cause OOS otherwise.
	int next_player_number_temp = gamestate_->next_player_number_;
	whiteboard_manager_->on_finish_side_turn(current_side());

	finish_side_turn_events();
	if(is_regular_game_end()) {
		return;
	}

	auto [next_player_number, new_turn]  = skip_empty_sides(next_player_number_temp);

	if(new_turn) {
		finish_turn();
		if(is_regular_game_end()) {
			return;
		}
		// Time has run out
		check_time_over();
		if(is_regular_game_end()) {
			return;
		}
		did_tod_sound_this_turn_ = false;
	}

	gamestate_->player_number_ = next_player_number;
	if(current_team().is_empty()) {
		// We don't support this case (turn end events emptying the next sides controller) since the server cannot handle it.
		throw game::game_error("Empty side after new turn events");
	}

	if(new_turn) {
		whiteboard_manager_->on_gamestate_change();
		gui_->new_turn();
		gui_->invalidate_game_status();
	}
	gamestate().gamedata_.set_phase(game_data::TURN_STARTING_WAITING);
	gamestate().gamedata_.set_end_turn_forced(false);
	did_autosave_this_turn_ = false;
	end_turn_requested_ = false;
	init_side_begin();
}

void playsingle_controller::play_scenario_main_loop()
{
	LOG_NG << "starting main loop\n" << (SDL_GetTicks() - ticks());

	ai_testing::log_game_start();
	while(!(gamestate().in_phase(game_data::GAME_ENDED) && end_turn_requested_ )) {
		try {
			play_some();
		} catch(const reset_gamestate_exception& ex) {
			boost::dynamic_bitset<> local_players;
			local_players.resize(get_teams().size(), true);
			// Preserve side controllers, because we won't get the side controoller updates again when replaying.
			for(std::size_t i = 0; i < local_players.size(); ++i) {
				local_players[i] = get_teams()[i].is_local();
			}

			if(ex.stats_) {
				// "Back to turn"
				get_saved_game().statistics().read(*ex.stats_);
			} else {
				// "Reset Replay To start"
				get_saved_game().statistics().clear_current_scenario();
			}

			reset_gamestate(*ex.level, (*ex.level)["replay_pos"]);

			for(std::size_t i = 0; i < local_players.size(); ++i) {
				resources::gameboard->teams()[i].set_local(local_players[i]);
			}

			// TODO: we currently don't set the music to the initial playlist, should we?

			play_scenario_init(*ex.level);

			if(replay_controller_ == nullptr) {
				replay_controller_ = std::make_unique<replay_controller>(*this, false, ex.level, [this]() { on_replay_end(false); });
			}

			if(ex.start_replay) {
				replay_controller_->play_replay();
			}
		}
	} // end for loop
}

void playsingle_controller::do_end_level()
{
	if(game_config::exit_at_end) {
		exit(0);
	}
	const bool is_victory = get_end_level_data().is_victory;

	ai_testing::log_game_end();

	const end_level_data& end_level = get_end_level_data();

	if(get_teams().empty()) {
		// this is probably only a story scenario, i.e. has its endlevel in the prestart event
		return;
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
	carryover_show_gold(gamestate(), is_observer() || is_replay(), is_observer(), saved_game_.classification().is_test());

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


	try {
		play_scenario_init(level);
		// clears level config (the intention was probably just to save some ram),
		// Note: this might clear 'level', so don't use level after this.
		saved_game_.remove_snapshot();

		play_scenario_main_loop();

		// TODO: would it be better if the is_networked_mp() check was done in is_observer() ?
		if(is_networked_mp() && is_observer()) {
			return level_result::type::observer_end;
		}
		return level_result::get_enum(get_end_level_data().test_result).value_or(get_end_level_data().is_victory ? level_result::type::victory : level_result::type::defeat);
	} catch(const savegame::load_game_exception&) {
		// Loading a new game is effectively a quit.
		saved_game_.clear();
		throw;
	} catch(const wesnothd_error& e) {
		scoped_savegame_snapshot snapshot(*this);
		savegame::ingame_savegame save(saved_game_, prefs::get().save_compression_format());
		if(e.message == "") {
			save.save_game_interactive(
				_("A network disconnection has occurred, and the game cannot continue. Do you want to save the game?"),
				savegame::savegame::YES_NO);
		} else {
			save.save_game_interactive(
				_("This game has been ended.\nReason: ") + e.message + _("\nDo you want to save the game?"),
				savegame::savegame::YES_NO);
		}

		if(dynamic_cast<const ingame_wesnothd_error*>(&e) || dynamic_cast<const leavegame_wesnothd_error*>(&e)) {
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
	if(replay_controller_.get() != nullptr) {
		replay_controller_->play_side_impl();

		if(player_type_changed_) {
			replay_controller_.reset();
		}
	} else if((current_team().is_local_human() && current_team().is_proxy_human())) {
		LOG_NG << "is human...";
		// If a side is dead end the turn, but play at least side=1's
		// turn in case all sides are dead
		if(gamestate().board_.side_units(current_side()) == 0 && !(get_units().empty() && current_side() == 1)) {
			require_end_turn();
		}


		if(!end_turn_requested_) {
			before_human_turn();
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

		if( gamestate().in_phase(game_data::TURN_PLAYING, game_data::TURN_STARTING_WAITING)) {
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
	assert(!is_linger_mode());
	if(!gamestate().in_phase(game_data::TURN_PLAYING) || is_regular_game_end()) {
		return;
	}

	if(!did_autosave_this_turn_ && !game_config::disable_autosave && prefs::get().auto_save_max() > 0) {
		did_autosave_this_turn_ = true;
		scoped_savegame_snapshot snapshot(*this);
		savegame::autosave_savegame save(saved_game_, prefs::get().save_compression_format());
		save.autosave(game_config::disable_autosave, prefs::get().auto_save_max(), pref_constants::INFINITE_AUTO_SAVES);
	}

	if(prefs::get().turn_bell()) {
		sound::play_bell(game_config::sounds::turn_bell);
	}
}

void playsingle_controller::show_turn_dialog()
{
	if(prefs::get().turn_dialog() && !is_regular_game_end()) {
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

	if(!prefs::get().disable_auto_moves()) {
		execute_gotos();
	}

	end_turn_enable(true);

	while(!should_return_to_play_side() && !end_turn_requested_) {
		check_objectives();
		play_slice_catch();
	}
}

void playsingle_controller::update_gui_linger()
{
	if(is_linger_mode()) {
		// If we need to set the status depending on the completion state
		// the key to it is here.
		gui_->set_game_mode(game_display::LINGER);
		// change the end-turn button text from "End Turn" to "End Scenario"
		gui_->get_theme().refresh_title2("button-endturn", "title2");

		if(get_end_level_data().transient.reveal_map) {
			// Change the view of all players and observers
			// to see the whole map regardless of shroud and fog.
			update_gui_to_player(gui_->viewing_team(), true);
		}
	} else {
		gui_->set_game_mode(game_display::RUNNING);
		// change the end-turn button text from "End Scenario" to "End Turn"
		gui_->get_theme().refresh_title2("button-endturn", "title");
	}
	// Also checcks whether the button can be pressed.
	gui_->queue_rerender();
}

void playsingle_controller::linger()
{
	LOG_NG << "beginning end-of-scenario linger";

	// Make all of the able-to-move units' orbs consistently red
	gamestate().board_.set_all_units_user_end_turn();

	update_gui_linger();

	try {
		if(replay_controller_.get() != nullptr) {
			replay_controller_->play_side_impl();
			if(player_type_changed_) {
				replay_controller_.reset();
			}
		}
		while(!end_turn_requested_) {
			play_slice();
		}
	} catch(const savegame::load_game_exception&) {
		// Loading a new game is effectively a quit.
		saved_game_.clear();
		throw;
	}

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
		DBG_NG << "Caught exception playing ai turn: " << utils::get_unknown_exception_type();
		throw;
	}

	if(!should_return_to_play_side()) {
		require_end_turn();
	}
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
	if(is_linger_mode()) {
		end_turn_requested_ = true;
	} else if(!is_browsing() && menu_handler_.end_turn(current_side())) {
		require_end_turn();
	}
}

void playsingle_controller::force_end_turn()
{
	gamestate().gamedata_.set_end_turn_forced(true);
	end_turn_requested_ = true;
}

void playsingle_controller::require_end_turn()
{
	end_turn_requested_ = true;
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
	linger();
	end_turn_requested_ = true;
}

void playsingle_controller::sync_end_turn()
{
	// We cannot add [end_turn] to the recorder while executing another action.
	assert(synced_context::synced_state() == synced_context::UNSYNCED);

	if(!gamestate().in_phase(game_data::TURN_ENDED)) {
		assert(end_turn_requested_);
		assert(current_team().is_local());
		assert(gamestate().in_phase(game_data::TURN_PLAYING));
		// TODO: we should also send this immediately.
		resources::recorder->end_turn(gamestate_->next_player_number_);
		gamestate().gamedata_.set_phase(game_data::TURN_ENDED);

	}


	assert(gamestate().in_phase(game_data::TURN_ENDED));

	if(ai_fallback_) {
		current_team().make_ai();
		ai_fallback_ = false;
	}
}

bool playsingle_controller::is_team_visible(int team_num, bool observer) const
{
	const team& t = gamestate().board_.get_team(team_num);
	if(observer) {
		return !t.get_disallow_observers() && !t.is_empty();
	} else {
		return t.is_local_human() && !t.is_idle();
	}
}

int playsingle_controller::find_viewing_side() const
{
	const int num_teams = get_teams().size();
	const bool observer = is_observer();

	for(int i = 0; i < num_teams; i++) {
		const int team_num = modulo(current_side() + i, num_teams, 1);
		if(is_team_visible(team_num, observer)) {
			return team_num;
		}
	}

	return 0;
}

void playsingle_controller::update_viewing_player()
{
	if(replay_controller_ && replay_controller_->is_controlling_view()) {
		replay_controller_->update_viewing_player();
	} else if(int side_num = find_viewing_side()) {
		if(side_num != gui_->viewing_side() || gui_->show_everything()) {
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
		true,
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
	} else if(gamestate().in_phase(game_data::TURN_ENDED)) {
		return true;
	} else if((gamestate().in_phase(game_data::TURN_STARTING_WAITING) || end_turn_requested_) && replay_controller_.get() == 0 && current_team().is_local() && !current_team().is_idle()) {
		// When we are a locally controlled side and havent done init_side yet also return to play_side
		return true;
	} else {
		return false;
	}
}

void playsingle_controller::on_replay_end(bool is_unit_test)
{
	if(is_networked_mp()) {
		// we are using the "Back to turn (replay)" feature
		// And have reached the current gamestate: end the replay and continue normally.
		set_player_type_changed();
	} else if(is_unit_test) {
		replay_controller_->return_to_play_side();
		if(!is_regular_game_end()) {
			end_level_data e;
			e.proceed_to_next_level = false;
			e.is_victory = false;
			set_end_level_data(e);
		}
	} else {
		replay_controller_->stop_replay();
	}
}

