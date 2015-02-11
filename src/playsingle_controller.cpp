/*
   Copyright (C) 2006 - 2015 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playlevel Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "ai/game_info.hpp"
#include "ai/testing.hpp"
#include "dialogs.hpp"
#include "display_chat_manager.hpp"
#include "game_end_exceptions.hpp"
#include "game_events/manager.hpp"
#include "game_events/pump.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "hotkey_handler_sp.hpp"
#include "log.hpp"
#include "map_label.hpp"
#include "marked-up_text.hpp"
#include "playturn.hpp"
#include "random_new_deterministic.hpp"
#include "replay_helper.hpp"
#include "savegame.hpp"
#include "sound.hpp"
#include "synced_context.hpp"
#include "formula_string_utils.hpp"
#include "events.hpp"
#include "save_blocker.hpp"
#include "scripting/plugins/context.hpp"
#include "soundsource.hpp"
#include "statistics.hpp"
#include "storyscreen/interface.hpp"
#include "unit.hpp"
#include "unit_animation.hpp"
#include "util.hpp"
#include "whiteboard/manager.hpp"
#include "hotkey/hotkey_item.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_aitesting("aitesting");
#define LOG_AIT LOG_STREAM(info, log_aitesting)
//If necessary, this define can be replaced with `#define LOG_AIT std::cout` to restore previous behavior

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)

static lg::log_domain log_enginerefac("enginerefac");
#define LOG_RG LOG_STREAM(info, log_enginerefac)

playsingle_controller::playsingle_controller(const config& level,
		saved_game& state_of_game, const int ticks,
		const config& game_config, const tdata_cache & tdata,
		CVideo& video, bool skip_replay)
	: play_controller(level, state_of_game, ticks, game_config, tdata, video, skip_replay)
	, cursor_setter(cursor::NORMAL)
	, textbox_info_()
	, replay_sender_(recorder)
	, network_reader_()
	, turn_data_(replay_sender_, network_reader_)
	, end_turn_(false)
	, player_type_changed_(false)
	, replaying_(false)
	, skip_next_turn_(false)
	, do_autosaves_(false)
	, level_result_(NONE)
{
	hotkey_handler_.reset(new hotkey_handler(*this, saved_game_)); //upgrade hotkey handler to the sp (whiteboard enabled) version

	// game may need to start in linger mode
	if (state_of_game.classification().completion == "victory" || state_of_game.classification().completion == "defeat")
	{
		LOG_NG << "Setting linger mode.\n";
		browse_ = linger_ = true;
	}

	ai::game_info ai_info;
	ai::manager::set_ai_info(ai_info);
	ai::manager::add_observer(this) ;

	plugins_context_->set_accessor_string("level_result", boost::bind(&LEVEL_RESULT_to_string, level_result_));
	plugins_context_->set_accessor_int("turn", boost::bind(&play_controller::turn, this));
}

playsingle_controller::~playsingle_controller()
{
	ai::manager::remove_observer(this) ;
	ai::manager::clear_ais() ;
}

void playsingle_controller::init_gui(){
	LOG_NG << "Initializing GUI... " << (SDL_GetTicks() - ticks_) << "\n";
	play_controller::init_gui();

	if(gamestate_.first_human_team_ != -1) {
		gui_->scroll_to_tile(gamestate_.board_.map().starting_position(gamestate_.first_human_team_ + 1), game_display::WARP);
	}
	gui_->scroll_to_tile(gamestate_.board_.map().starting_position(1), game_display::WARP);

	update_locker lock_display(gui_->video(),recorder.is_skipping());
	get_hotkey_command_executor()->set_button_state(*gui_);
	events::raise_draw_event();
	gui_->draw();
}

void playsingle_controller::report_victory(
	std::ostringstream &report, team& t,
	int finishing_bonus_per_turn, int turns_left, int finishing_bonus)
{
	report << _("Remaining gold: ")
		<< utils::half_signed_value(t.gold()) << "\n";
	if(t.carryover_bonus()) {
		if (turns_left > -1) {
			report << _("Early finish bonus: ")
				   << finishing_bonus_per_turn
				   << " " << _("per turn") << "\n"
				   << "<b>" << _("Turns finished early: ")
				   << turns_left << "</b>\n"
				   << _("Bonus: ")
				   << finishing_bonus << "\n";
		}
		report << _("Gold: ")
		       << utils::half_signed_value(t.gold() + finishing_bonus);
	}
	if (t.gold() > 0) {
		report << '\n' << _("Carry over percentage: ") << t.carryover_percentage();
	}
	if(t.carryover_add()) {
		report << "\n<b>" << _("Bonus Gold: ") << utils::half_signed_value(t.carryover_gold()) <<"</b>";
	} else {
		report << "\n<b>" << _("Retained Gold: ") << utils::half_signed_value(t.carryover_gold()) << "</b>";
	}

	std::string goldmsg;
	utils::string_map symbols;

	symbols["gold"] = lexical_cast_default<std::string>(t.carryover_gold());

	// Note that both strings are the same in English, but some languages will
	// want to translate them differently.
	if(t.carryover_add()) {
		if(t.carryover_gold() > 0) {
			goldmsg = vngettext(
					"You will start the next scenario with $gold "
					"on top of the defined minimum starting gold.",
					"You will start the next scenario with $gold "
					"on top of the defined minimum starting gold.",
					t.carryover_gold(), symbols);

		} else {
			goldmsg = vngettext(
					"You will start the next scenario with "
					"the defined minimum starting gold.",
					"You will start the next scenario with "
					"the defined minimum starting gold.",
					t.carryover_gold(), symbols);
		}
	} else {
		goldmsg = vngettext(
			"You will start the next scenario with $gold "
			"or its defined minimum starting gold, "
			"whichever is higher.",
			"You will start the next scenario with $gold "
			"or its defined minimum starting gold, "
			"whichever is higher.",
			t.carryover_gold(), symbols);
	}

	// xgettext:no-c-format
	report << '\n' << goldmsg;
}

boost::optional<LEVEL_RESULT> playsingle_controller::play_scenario_init(end_level_data & /*eld*/, bool & past_prestart) {
	// At the beginning of the scenario, save a snapshot as replay_start
	if(saved_game_.replay_start().empty()){
		saved_game_.replay_start() = to_config();
	}

	try {
		fire_preload();
	} catch (end_level_exception & e) {
		return e.result;
	} catch (end_turn_exception &) {
		assert(false && "caugh end_turn exception in a bad place... terminating.");
		std::terminate();
	}

	replaying_ = (recorder.at_end() == false);
	assert(!replaying_);

	if(!loading_game_ )
	{
		if(replaying_)
		{
			//can this codepath be reached ?
			//note this when we are entering an mp game and see the 'replay' of the game
			//this path is not reached because we receive the replay later
			config* pstart = recorder.get_next_action();
			assert(pstart->has_child("start"));
		}
		else
		{
			assert(recorder.empty());
			recorder.add_start();
			recorder.get_next_action();
		}
		//we can only use a set_scontext_synced with a non empty recorder.
		set_scontext_synced sync;

		try {
			fire_prestart();
		} catch (end_level_exception & e) {
			return e.result;
		} catch (end_turn_exception &) {
			assert(false && "caugh end_turn exception in a bad place... terminating.");
			std::terminate();
		}


		init_gui();
		past_prestart = true;
		LOG_NG << "first_time..." << (recorder.is_skipping() ? "skipping" : "no skip") << "\n";

		events::raise_draw_event();
		try {
			fire_start(true);
		} catch (end_level_exception & e) {
			return e.result;
		} catch (end_turn_exception &) {
			assert(false && "caugh end_turn exception in a bad place... terminating.");
			std::terminate();
		}
		sync.do_final_checkup();
		gui_->recalculate_minimap();
	}
	else
	{
		init_gui();
		past_prestart = true;
		events::raise_draw_event();
		try {
			fire_start(false);
		} catch (end_level_exception & e) {
			return e.result;
		} catch (end_turn_exception &) {
			assert(false && "caugh end_turn exception in a bad place... terminating.");
			std::terminate();
		}
		gui_->recalculate_minimap();
	}
	return boost::none;
}

LEVEL_RESULT playsingle_controller::play_scenario_main_loop(end_level_data & end_level, bool & /*past_prestart*/) {
	LOG_NG << "starting main loop\n" << (SDL_GetTicks() - ticks_) << "\n";

	// Initialize countdown clock.
	std::vector<team>::const_iterator t;
	for(t = gamestate_.board_.teams().begin(); t != gamestate_.board_.teams().end(); ++t) {
		if (saved_game_.mp_settings().mp_countdown && !loading_game_ ){
			t->set_countdown_time(1000 * saved_game_.mp_settings().mp_countdown_init_time);
		}
	}

	// if we loaded a save file in linger mode, skip to it.
	if (linger_) {
		//determine the bonus gold handling for this scenario
		end_level.read(level_.child_or_empty("end_level_data"));
		end_level.transient.carryover_report = false;
		end_level.transient.disabled = true;
		return SKIP_TO_LINGER;
		//throw end_level_exception(SKIP_TO_LINGER);
	}

	// Avoid autosaving after loading, but still
	// allow the first turn to have an autosave.
	do_autosaves_ = !loading_game_;
	ai_testing::log_game_start();
	if(gamestate_.board_.teams().empty())
	{
		ERR_NG << "Playing game with 0 teams." << std::endl;
	}
	for(; ; first_player_ = 1) {
		possible_end_play_signal signal = play_turn();

		if (signal) {
			switch (boost::apply_visitor( get_signal_type(), *signal )) {
				case END_LEVEL:
					return boost::apply_visitor( get_result(), *signal );
				case END_TURN:
					assert(false && "end turn signal propagated to playsingle_controller::play_scenario_main_loop, terminating");
					std::terminate();
			}
		}

		do_autosaves_ = true;
	} //end for loop
}

LEVEL_RESULT playsingle_controller::play_scenario(
	const config::const_child_itors &story,
	bool skip_replay)
{
	LOG_NG << "in playsingle_controller::play_scenario()...\n";

	// Start music.
	BOOST_FOREACH(const config &m, level_.child_range("music")) {
		sound::play_music_config(m);
	}
	sound::commit_music_changes();

	if(!skip_replay) {
		show_story(*gui_, level_["name"], story);
	}
	gui_->labels().read(level_);

	// Read sound sources
	assert(soundsources_manager_ != NULL);
	BOOST_FOREACH(const config &s, level_.child_range("sound_source")) {
		try {
			soundsource::sourcespec spec(s);
			soundsources_manager_->add(spec);
		} catch (bad_lexical_cast &) {
			ERR_NG << "Error when parsing sound_source config: bad lexical cast." << std::endl;
			ERR_NG << "sound_source config was: " << s.debug() << std::endl;
			ERR_NG << "Skipping this sound source..." << std::endl;
		}
	}

	set_victory_when_enemies_defeated(level_["victory_when_enemies_defeated"].to_bool(true));
	set_remove_from_carryover_on_defeat(level_["remove_from_carryover_on_defeat"].to_bool(true));
	end_level_data &end_level = get_end_level_data();

	bool past_prestart = false;

	LOG_NG << "entering try... " << (SDL_GetTicks() - ticks_) << "\n";
	try {
		boost::optional<LEVEL_RESULT> signal = play_scenario_init(end_level, past_prestart);

		if (!signal) {
			signal = play_scenario_main_loop(end_level, past_prestart);
		}

		assert(signal); //play_scenario_main_loop always returns a LEVEL_RESULT
		{
			LEVEL_RESULT end_level_result = *signal;

			if(!past_prestart) {
				sdl::draw_solid_tinted_rectangle(
					0, 0, gui_->video().getx(), gui_->video().gety(), 0, 0, 0, 1.0,
					gui_->video().getSurface()
				);
				update_rect(0, 0, gui_->video().getx(), gui_->video().gety());
			}

			ai_testing::log_game_end();
			if (!end_level.transient.custom_endlevel_music.empty()) {
				if (end_level_result == DEFEAT) {
					set_defeat_music_list(end_level.transient.custom_endlevel_music);
				} else {
					set_victory_music_list(end_level.transient.custom_endlevel_music);
				}
			}

			if (gamestate_.board_.teams().empty())
			{
				//store persistent teams
				saved_game_.set_snapshot(config());

				return VICTORY; // this is probably only a story scenario, i.e. has its endlevel in the prestart event
			}
			const bool obs = is_observer();
			if (game_config::exit_at_end) {
				exit(0);
			}
			if (end_level_result == DEFEAT || end_level_result == VICTORY)
			{
				saved_game_.classification().completion = (end_level_result == VICTORY) ? "victory" : "defeat";
				// If we're a player, and the result is victory/defeat, then send
				// a message to notify the server of the reason for the game ending.
				if (!obs) {
					config cfg;
					config& info = cfg.add_child("info");
					info["type"] = "termination";
					info["condition"] = "game over";
					info["result"] = saved_game_.classification().completion;
					network::send_data(cfg, 0);
				} else {
					gui2::show_transient_message(gui_->video(),_("Game Over"),
										_("The game is over."));
					return OBSERVER_END;
				}
			}

			if (end_level_result == QUIT) {
				return QUIT;
			}
			else if (end_level_result == DEFEAT)
			{
				saved_game_.classification().completion = "defeat";
				pump().fire("defeat");

				if (!obs) {
					const std::string& defeat_music = select_defeat_music();
					if(defeat_music.empty() != true)
						sound::play_music_once(defeat_music);
						persist_.end_transaction();
					return DEFEAT;
				} else {
					return QUIT;
				}
			}
			else if (end_level_result == VICTORY)
			{
				saved_game_.classification().completion =

				!end_level.transient.linger_mode ? "running" : "victory";
				pump().fire("victory");

				//
				// Play victory music once all victory events
				// are finished, if we aren't observers.
				//
				// Some scenario authors may use 'continue'
				// result for something that is not story-wise
				// a victory, so let them use [music] tags
				// instead should they want special music.
				//
				if (!obs && end_level.transient.linger_mode) {
					const std::string& victory_music = select_victory_music();
					if(victory_music.empty() != true)
						sound::play_music_once(victory_music);
				}

				LOG_NG << "Healing survived units\n";
				gamestate_.board_.heal_all_survivors();

				saved_game_.remove_snapshot();
				if(!is_observer()) {
					persist_.end_transaction();
				}

				return VICTORY;
			}
			else if (end_level_result == SKIP_TO_LINGER)
			{
				LOG_NG << "resuming from loaded linger state...\n";
				//as carryover information is stored in the snapshot, we have to re-store it after loading a linger state
				saved_game_.set_snapshot(config());
				if(!is_observer()) {
					persist_.end_transaction();
				}
				return VICTORY;
			}
		} //end if
	} catch(const game::load_game_exception &) {
		// Loading a new game is effectively a quit.
		//
		if ( game::load_game_exception::game != "" ) {
			saved_game_ = saved_game();
		}
		throw;
	} catch(network::error& e) {
		bool disconnect = false;
		if(e.socket) {
			e.disconnect();
			disconnect = true;
		}

		savegame::ingame_savegame save(saved_game_, *gui_, to_config(), preferences::save_compression_format());
		save.save_game_interactive(gui_->video(), _("A network disconnection has occurred, and the game cannot continue. Do you want to save the game?"), gui::YES_NO);
		if(disconnect) {
			throw network::error();
		} else {
			return QUIT;
		}
	}

	return QUIT;
}

possible_end_play_signal playsingle_controller::play_turn()
{
	whiteboard_manager_->on_gamestate_change();
	gui_->new_turn();
	gui_->invalidate_game_status();
	events::raise_draw_event();

	LOG_NG << "turn: " << turn() << "\n";

	if(non_interactive()) {
		LOG_AIT << "Turn " << turn() << ":" << std::endl;
	}

	for (player_number_ = first_player_; player_number_ <= int(gamestate_.board_.teams().size()); ++player_number_)
	{
		// If a side is empty skip over it.
		if (current_team().is_empty()) continue;

		possible_end_play_signal signal;
		{
			save_blocker blocker;
			signal = init_side();
		}

		if (signal) {
			switch (boost::apply_visitor(get_signal_type(), *signal)) {
				case END_TURN:
					if (current_team().is_network() == false) {
						turn_data_.send_data();
						recorder.end_turn();
						turn_data_.sync_network();
					}
					continue;
				case END_LEVEL:
					return signal;
			}
		}

		if (replaying_) {
			LOG_NG << "doing replay " << player_number_ << "\n";
			HANDLE_END_PLAY_SIGNAL ( replaying_ = ::do_replay() == REPLAY_FOUND_END_TURN );
			LOG_NG << "result of replay: " << (replaying_?"true":"false") << "\n";
		} else {
			ai_testing::log_turn_start(player_number_);
			PROPOGATE_END_PLAY_SIGNAL ( play_side() );
		}

		finish_side_turn();

		if(non_interactive()) {
			LOG_AIT << " Player " << player_number_ << ": " <<
				current_team().villages().size() << " Villages" <<
				std::endl;
			ai_testing::log_turn_end(player_number_);
		}

		HANDLE_END_PLAY_SIGNAL ( check_victory() );

		//if loading a savegame, network turns might not have reset this yet
		loading_game_ = false;
	}
	//If the loop exits due to the last team having been processed,
	//player_number_ will be 1 too high
	if(player_number_ > static_cast<int>(gamestate_.board_.teams().size()))
		player_number_ = gamestate_.board_.teams().size();

	finish_turn();

	// Time has run out
	PROPOGATE_END_PLAY_SIGNAL ( check_time_over() );
	return boost::none;
}

possible_end_play_signal playsingle_controller::play_idle_loop()
{
	while(!end_turn_) {
		HANDLE_END_PLAY_SIGNAL( play_slice() );
		gui_->draw();
		SDL_Delay(10);
	}
	return boost::none;
}

possible_end_play_signal playsingle_controller::play_side()
{
	//check for team-specific items in the scenario
	gui_->parse_team_overlays();

	HANDLE_END_PLAY_SIGNAL( maybe_do_init_side(false) );

	//flag used when we fallback from ai and give temporarily control to human
	bool temporary_human = false;
	do {
		// This flag can be set by derived classes (in overridden functions).
		player_type_changed_ = false;
		if (!skip_next_turn_)
			end_turn_ = false;

		statistics::reset_turn_stats(gamestate_.board_.teams()[player_number_ - 1].save_id());

		if((current_team().is_local_human() && current_team().is_proxy_human()) || temporary_human) {
			LOG_NG << "is human...\n";
			temporary_human = false;
			// If a side is dead end the turn, but play at least side=1's
			// turn in case all sides are dead
			if (gamestate_.board_.side_units(player_number_) != 0
				|| (gamestate_.board_.units().size() == 0 && player_number_ == 1))
			{
				possible_end_play_signal signal = before_human_turn();

				if (!signal) {
					signal = play_human_turn();
				}

				if (signal) {
					switch (boost::apply_visitor(get_signal_type(), *signal)) {
						case END_LEVEL:
							return signal;
						case END_TURN:
							if (int(boost::apply_visitor(get_redo(),*signal)) == player_number_) {
								player_type_changed_ = true;
								// If new controller is not human,
								// reset gui to prev human one
								if (!gamestate_.board_.teams()[player_number_-1].is_local_human()) {
									browse_ = true;
									int s = find_human_team_before_current_player();
									if (s <= 0)
										s = gui_->playing_side();
									update_gui_to_player(s-1);
								}
							}

					}
				}
			}

			// Ending the turn commits all moves.
			undo_stack_->clear();
			if ( !player_type_changed_ )
				after_human_turn();
			LOG_NG << "human finished turn...\n";

		} else if(current_team().is_local_ai() || (current_team().is_local_human() && current_team().is_droid())) {
			try {
				play_ai_turn();
			} catch(fallback_ai_to_human_exception&) {
				// Give control to a human for this turn.
				player_type_changed_ = true;
				temporary_human = true;
			} catch (end_level_exception & e) { //Don't know at the moment if these two are possible but can't hurt to add
				return possible_end_play_signal(e.to_struct());
			} catch (end_turn_exception & e) {
				return possible_end_play_signal(e.to_struct());
			}
			if(!player_type_changed_)
			{
				recorder.end_turn();
			}

		} else if(current_team().is_network()) {
			PROPOGATE_END_PLAY_SIGNAL( play_network_turn() );
		} else if(current_team().is_local_human() && current_team().is_idle()) {
			end_turn_enable(false);
			do_idle_notification();

			possible_end_play_signal signal = before_human_turn();
			browse_ = true;

			if (!signal) {
				signal = play_idle_loop();
			}

			if (signal) {
				switch (boost::apply_visitor(get_signal_type(), *signal)) {
					case END_LEVEL:
						return signal;
					case END_TURN:
						LOG_NG << "Escaped from idle state with exception!" << std::endl;
						if (int(boost::apply_visitor(get_redo(), *signal)) == player_number_) {
							player_type_changed_ = true;
							// If new controller is not human,
							// reset gui to prev human one
							if (!gamestate_.board_.teams()[player_number_-1].is_local_human()) {
								browse_ = true;
								int s = find_human_team_before_current_player();
								if (s <= 0)
									s = gui_->playing_side();
								update_gui_to_player(s-1);
							}
							else {
								//This side was previously not human controlled.
								update_gui_to_player(player_number_ - 1);
							}
						}
				}
			}
		}
		else {
			assert(current_team().is_empty()); // Do nothing.
		}

	} while (player_type_changed_);
	// Keep looping if the type of a team (human/ai/networked)
	// has changed mid-turn
	skip_next_turn_ = false;
	return boost::none;
}

possible_end_play_signal playsingle_controller::before_human_turn()
{
	log_scope("player turn");
	browse_ = false;
	linger_ = false;

	HANDLE_END_PLAY_SIGNAL( ai::manager::raise_turn_started() ); //This line throws exception from here: https://github.com/wesnoth/wesnoth/blob/ac96a2b91b3276e20b682210617cf87d1e0d366a/src/playsingle_controller.cpp#L954

	if(do_autosaves_ && level_result_ == NONE) {
		savegame::autosave_savegame save(saved_game_, *gui_, to_config(), preferences::save_compression_format());
		save.autosave(game_config::disable_autosave, preferences::autosavemax(), preferences::INFINITE_AUTO_SAVES);
	}

	if(preferences::turn_bell() && level_result_ == NONE) {
		sound::play_bell(game_config::sounds::turn_bell);
	}
	return boost::none;
}

void playsingle_controller::show_turn_dialog(){
	if(preferences::turn_dialog() && (level_result_ == NONE) ) {
		blindfold b(*gui_, true); //apply a blindfold for the duration of this dialog
		gui_->redraw_everything();
		gui_->recalculate_minimap();
		std::string message = _("It is now $name|’s turn");
		utils::string_map symbols;
		symbols["name"] = gamestate_.board_.teams()[player_number_ - 1].current_player();
		message = utils::interpolate_variables_into_string(message, &symbols);
		gui2::show_transient_message(gui_->video(), "", message);
	}
}

void playsingle_controller::execute_gotos(){
	menu_handler_.execute_gotos(mouse_handler_, player_number_);
}

possible_end_play_signal playsingle_controller::play_human_turn() {
	show_turn_dialog();

	if (!preferences::disable_auto_moves()) {
		HANDLE_END_PLAY_SIGNAL(execute_gotos());
	}

	end_turn_enable(true);
	while(!end_turn_) {
		HANDLE_END_PLAY_SIGNAL( play_slice() );
		HANDLE_END_PLAY_SIGNAL( check_end_level() );
		gui_->draw();
	}

	return boost::none;
}

void playsingle_controller::linger()
{
	LOG_NG << "beginning end-of-scenario linger\n";
	browse_ = true;
	linger_ = true;

	// If we need to set the status depending on the completion state
	// the key to it is here.
	gui_->set_game_mode(game_display::LINGER_SP);

	// this is actually for after linger mode is over -- we don't
	// want to stay stuck in linger state when the *next* scenario
	// is over.
	set_completion setter(saved_game_,"running");

	// change the end-turn button text to its alternate label
	gui_->get_theme().refresh_title2("button-endturn", "title2");
	gui_->invalidate_theme();
	gui_->redraw_everything();

	// End all unit moves
	gamestate_.board_.set_all_units_user_end_turn();
	try {
		// Same logic as single-player human turn, but
		// *not* the same as multiplayer human turn.
		end_turn_enable(true);
		end_turn_ = false;
		while(!end_turn_) {
			// Reset the team number to make sure we're the right team.
			player_number_ = first_player_;
			play_slice();
			gui_->draw();
		}
	} catch(const game::load_game_exception &) {
		// Loading a new game is effectively a quit.
		if ( game::load_game_exception::game != "" ) {
			saved_game_ = saved_game();
		}
		throw;
	}

	// revert the end-turn button text to its normal label
	gui_->get_theme().refresh_title2("button-endturn", "title");
	gui_->invalidate_theme();
	gui_->redraw_everything();
	gui_->set_game_mode(game_display::RUNNING);

	LOG_NG << "ending end-of-scenario linger\n";
}

void playsingle_controller::end_turn_enable(bool enable)
{
	gui_->enable_menu("endturn", enable);
	get_hotkey_command_executor()->set_button_state(*gui_);
}


void playsingle_controller::after_human_turn()
{
	// Mark the turn as done.
	browse_ = true;
	if (!linger_)
	{
		recorder.end_turn();
	}

	// Clear moves from the GUI.
	gui_->set_route(NULL);
	gui_->unhighlight_reach();
}

void playsingle_controller::play_ai_turn(){
	LOG_NG << "is ai...\n";
	end_turn_enable(false);
	browse_ = true;
	gui_->recalculate_minimap();

	const cursor::setter cursor_setter(cursor::WAIT);

	// Correct an oddball case where a human could have left delayed shroud
	// updates on before giving control to the AI. (The AI does not bother
	// with the undo stack, so it cannot delay shroud updates.)
	team & cur_team = current_team();
	if ( !cur_team.auto_shroud_updates() ) {
		// We just took control, so the undo stack is empty. We still need
		// to record this change for the replay though.
		synced_context::run_in_synced_context("auto_shroud", replay_helper::get_auto_shroud(true));
	}

	turn_data_.send_data();
	try {
		try {
			ai::manager::play_turn(player_number_);
		} 
		catch (end_turn_exception&) {
		}
	} 
	catch(...) {
		turn_data_.sync_network();
		throw;
	}
	turn_data_.sync_network();
	gui_->recalculate_minimap();
	gui_->invalidate_unit();
	gui_->invalidate_game_status();
	gui_->invalidate_all();
	gui_->draw();
	gui_->delay(100);
}


/**
 * Will handle sending a networked notification in descendent classes.
 */
void playsingle_controller::do_idle_notification()
{
	gui_->get_chat_manager().add_chat_message(time(NULL), "Wesnoth", 0,
		"This side is in an idle state. To proceed with the game, the host must assign it to another controller.",
		events::chat_handler::MESSAGE_PUBLIC, false);
}

/**
 * Will handle networked turns in descendent classes.
 */
possible_end_play_signal playsingle_controller::play_network_turn()
{
	// There should be no networked sides in single-player.
	ERR_NG << "Networked team encountered by playsingle_controller." << std::endl;
	return boost::none;
}


void playsingle_controller::handle_generic_event(const std::string& name){
	if (name == "ai_user_interact"){
		play_slice(false);
	}
	if (end_turn_){
		throw end_turn_exception();
	}
}

possible_end_play_signal playsingle_controller::check_time_over(){
	bool b = gamestate_.tod_manager_.next_turn(gamestate_.gamedata_);
	it_is_a_new_turn_ = true;
	if(!b) {

		LOG_NG << "firing time over event...\n";
		pump().fire("time over");
		LOG_NG << "done firing time over event...\n";
		//if turns are added while handling 'time over' event
		if (gamestate_.tod_manager_.is_time_left()) {
			return boost::none;
		}

		if(non_interactive()) {
			LOG_AIT << "time over (draw)\n";
			ai_testing::log_draw();
		}

		HANDLE_END_PLAY_SIGNAL( check_victory() );

		get_end_level_data().proceed_to_next_level = false;

		end_level_struct els = {DEFEAT};
		return possible_end_play_signal (els);
		//throw end_level_exception(DEFEAT);
	}
	return boost::none;
}

void playsingle_controller::end_turn(){
	if (linger_)
		end_turn_ = true;
	else if (!browse_){
		browse_ = true;
		end_turn_ = menu_handler_.end_turn(player_number_);
		browse_ = end_turn_;
	}
}

void playsingle_controller::force_end_turn(){
	skip_next_turn_ = true;
	end_turn_ = true;
}

void playsingle_controller::check_end_level()
{
	if (level_result_ == NONE || linger_)
	{
		const team &t = gamestate_.board_.teams()[gui_->viewing_team()];
		if (!browse_ && t.objectives_changed()) {
			dialogs::show_objectives(level_, t.objectives());
			t.reset_objectives_changed();
		}
		return;
	}
	throw end_level_exception(level_result_);
}


bool playsingle_controller::is_host() const
{
	return turn_data_.is_host();
}

void playsingle_controller::maybe_linger()
{
	//Make sure [end_level_data] gets writen into the snapshot even when skipping linger mode.
	linger_ = true;
	if (get_end_level_data_const().transient.linger_mode) {
		linger();
	}
}
