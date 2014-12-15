/*
   Copyright (C) 2006 - 2014 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
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
#include "soundsource.hpp"
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
		CVideo& video, bool skip_replay) :
	play_controller(level, state_of_game, ticks, game_config, tdata, video, skip_replay),
	cursor_setter(cursor::NORMAL),
	textbox_info_(),
	replay_sender_(recorder),
	network_reader_(),
	turn_data_(replay_sender_, network_reader_),
	end_turn_(false),
	player_type_changed_(false),
	replaying_(false),
	skip_next_turn_(false),
	do_autosaves_(false),
	level_result_(NONE)
{
	// game may need to start in linger mode
	if (state_of_game.classification().completion == "victory" || state_of_game.classification().completion == "defeat")
	{
		LOG_NG << "Setting linger mode.\n";
		browse_ = linger_ = true;
	}

	ai::game_info ai_info;
	ai::manager::set_ai_info(ai_info);
	ai::manager::add_observer(this) ;
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
	set_button_state(*gui_);
	events::raise_draw_event();
	gui_->draw();
}

void playsingle_controller::recruit(){
	if (!browse_)
		menu_handler_.recruit(player_number_, mouse_handler_.get_last_hex());
	else if (whiteboard_manager_->is_active())
		menu_handler_.recruit(gui_->viewing_side(), mouse_handler_.get_last_hex());
}

void playsingle_controller::repeat_recruit(){
	if (!browse_)
		menu_handler_.repeat_recruit(player_number_, mouse_handler_.get_last_hex());
	else if (whiteboard_manager_->is_active())
		menu_handler_.repeat_recruit(gui_->viewing_side(), mouse_handler_.get_last_hex());
}

void playsingle_controller::recall(){
	if (!browse_)
		menu_handler_.recall(player_number_, mouse_handler_.get_last_hex());
	else if (whiteboard_manager_->is_active())
		menu_handler_.recall(gui_->viewing_side(), mouse_handler_.get_last_hex());
}

void playsingle_controller::toggle_shroud_updates(){
	menu_handler_.toggle_shroud_updates(gui_->viewing_team()+1);
}

void playsingle_controller::update_shroud_now(){
	menu_handler_.update_shroud_now(gui_->viewing_team()+1);
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
	get_end_level_data().proceed_to_next_level = (level_result_ == VICTORY);
	throw end_level_exception(level_result_);
}

void playsingle_controller::rename_unit(){
	menu_handler_.rename_unit();
}

void playsingle_controller::create_unit(){
	menu_handler_.create_unit(mouse_handler_);
}

void playsingle_controller::change_side(){
	menu_handler_.change_side(mouse_handler_);
}

void playsingle_controller::kill_unit(){
	menu_handler_.kill_unit(mouse_handler_);
}

void playsingle_controller::label_terrain(bool team_only){
	menu_handler_.label_terrain(mouse_handler_, team_only);
}

void playsingle_controller::clear_labels(){
	menu_handler_.clear_labels();
}

void playsingle_controller::continue_move(){
	menu_handler_.continue_move(mouse_handler_, player_number_);
}

void playsingle_controller::unit_hold_position(){
	if (!browse_)
		menu_handler_.unit_hold_position(mouse_handler_, player_number_);
}

void playsingle_controller::end_unit_turn(){
	if (!browse_)
		menu_handler_.end_unit_turn(mouse_handler_, player_number_);
}

void playsingle_controller::user_command(){
	menu_handler_.user_command();
}

void playsingle_controller::custom_command(){
	menu_handler_.custom_command();
}

void playsingle_controller::ai_formula(){
	menu_handler_.ai_formula();
}

void playsingle_controller::clear_messages(){
	menu_handler_.clear_messages();
}

void playsingle_controller::whiteboard_toggle() {
	whiteboard_manager_->set_active(!whiteboard_manager_->is_active());

	if (whiteboard_manager_->is_active()) {
		std::string hk = hotkey::get_names(hotkey::hotkey_command::get_command_by_command(hotkey::HOTKEY_WB_TOGGLE).command);
		utils::string_map symbols;
		symbols["hotkey"] = hk;

		gui_->announce(_("Planning mode activated!") + std::string("\n") + vgettext("(press $hotkey to deactivate)", symbols), font::NORMAL_COLOR);
	} else {
		gui_->announce(_("Planning mode deactivated!"), font::NORMAL_COLOR);
	}
	//@todo Stop printing whiteboard help in the chat once we have better documentation/help
	whiteboard_manager_->print_help_once();
}

void playsingle_controller::whiteboard_execute_action(){
	whiteboard_manager_->contextual_execute();
}

void playsingle_controller::whiteboard_execute_all_actions(){
	whiteboard_manager_->execute_all_actions();
}

void playsingle_controller::whiteboard_delete_action(){
	whiteboard_manager_->contextual_delete();
}

void playsingle_controller::whiteboard_bump_up_action()
{
	whiteboard_manager_->contextual_bump_up_action();
}

void playsingle_controller::whiteboard_bump_down_action()
{
	whiteboard_manager_->contextual_bump_down_action();
}

void playsingle_controller::whiteboard_suppose_dead()
{
	unit* curr_unit;
	map_location loc;
	{ wb::future_map future; //start planned unit map scope
		curr_unit = &*menu_handler_.current_unit();
		loc = curr_unit->get_location();
	} // end planned unit map scope
	whiteboard_manager_->save_suppose_dead(*curr_unit,loc);
}

void playsingle_controller::report_victory(
	std::ostringstream &report, int player_gold, int remaining_gold,
	int finishing_bonus_per_turn, int turns_left, int finishing_bonus)
{
	const end_level_data &end_level = get_end_level_data_const();
	report << _("Remaining gold: ")
		   << utils::half_signed_value(remaining_gold) << "\n";
	if(end_level.gold_bonus) {
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
		       << utils::half_signed_value(remaining_gold + finishing_bonus);
	}
	if (remaining_gold > 0) {
		report << '\n' << _("Carry over percentage: ") << end_level.carryover_percentage;
	}
	if(end_level.carryover_add) {
		report << "\n<b>" << _("Bonus Gold: ") << utils::half_signed_value(player_gold) <<"</b>";
	} else {
		report << "\n<b>" << _("Retained Gold: ") << utils::half_signed_value(player_gold) << "</b>";
	}

	std::string goldmsg;
	utils::string_map symbols;

	symbols["gold"] = lexical_cast_default<std::string>(player_gold);

	// Note that both strings are the same in English, but some languages will
	// want to translate them differently.
	if(end_level.carryover_add) {
		if(player_gold > 0) {
			goldmsg = vngettext(
					"You will start the next scenario with $gold "
					"on top of the defined minimum starting gold.",
					"You will start the next scenario with $gold "
					"on top of the defined minimum starting gold.",
					player_gold, symbols);

		} else {
			goldmsg = vngettext(
					"You will start the next scenario with "
					"the defined minimum starting gold.",
					"You will start the next scenario with "
					"the defined minimum starting gold.",
					player_gold, symbols);
		}
	} else {
		goldmsg = vngettext(
			"You will start the next scenario with $gold "
			"or its defined minimum starting gold, "
			"whichever is higher.",
			"You will start the next scenario with $gold "
			"or its defined minimum starting gold, "
			"whichever is higher.",
			player_gold, symbols);
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
	end_level.carryover_percentage = level_["carryover_percentage"].to_int(game_config::gold_carryover_percentage);
	end_level.carryover_add = level_["carryover_add"].to_bool();

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
				events_manager_->pump().fire("defeat");

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
				events_manager_->pump().fire("victory");

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

		if(current_team().is_local_human() || temporary_human) {
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

		} else if(current_team().is_local_ai()) {
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
		} else if(current_team().is_idle()) {
			end_turn_enable(false);
			do_idle_notification();

			possible_end_play_signal signal = before_human_turn();

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
	set_button_state(*gui_);
}

hotkey::ACTION_STATE playsingle_controller::get_action_state(hotkey::HOTKEY_COMMAND command, int index) const
{
	switch(command) {
	case hotkey::HOTKEY_WB_TOGGLE:
		return whiteboard_manager_->is_active() ? hotkey::ACTION_ON : hotkey::ACTION_OFF;
	default:
		return play_controller::get_action_state(command, index);
	}
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
		events_manager_->pump().fire("time over");
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

bool playsingle_controller::can_execute_command(const hotkey::hotkey_command& cmd, int index) const
{
	hotkey::HOTKEY_COMMAND command = cmd.id;
	bool res = true;
	switch (command){

		case hotkey::HOTKEY_WML:
			//code mixed from play_controller::show_menu and code here
			return (gui_->viewing_team() == gui_->playing_team()) && !events::commands_disabled && gamestate_.board_.teams()[gui_->viewing_team()].is_local_human() && !linger_ && !browse_;
		case hotkey::HOTKEY_UNIT_HOLD_POSITION:
		case hotkey::HOTKEY_END_UNIT_TURN:
			return !browse_ && !linger_ && !events::commands_disabled;
		case hotkey::HOTKEY_RECRUIT:
		case hotkey::HOTKEY_REPEAT_RECRUIT:
		case hotkey::HOTKEY_RECALL:
			return (!browse_ || whiteboard_manager_->is_active()) && !linger_ && !events::commands_disabled;
		case hotkey::HOTKEY_ENDTURN:
			return (!browse_ || linger_) && !events::commands_disabled;

		case hotkey::HOTKEY_DELAY_SHROUD:
			return !linger_ && (gamestate_.board_.teams()[gui_->viewing_team()].uses_fog() || gamestate_.board_.teams()[gui_->viewing_team()].uses_shroud())
			&& !events::commands_disabled;
		case hotkey::HOTKEY_UPDATE_SHROUD:
			return !linger_
				&& player_number_ == gui_->viewing_side()
				&& !events::commands_disabled
				&& gamestate_.board_.teams()[gui_->viewing_team()].auto_shroud_updates() == false;

		// Commands we can only do if in debug mode
		case hotkey::HOTKEY_CREATE_UNIT:
		case hotkey::HOTKEY_CHANGE_SIDE:
		case hotkey::HOTKEY_KILL_UNIT:
			return !events::commands_disabled && game_config::debug && gamestate_.board_.map().on_board(mouse_handler_.get_last_hex());

		case hotkey::HOTKEY_CLEAR_LABELS:
			res = !is_observer();
			break;
		case hotkey::HOTKEY_LABEL_TEAM_TERRAIN:
		case hotkey::HOTKEY_LABEL_TERRAIN: {
			const terrain_label *label = gui_->labels().get_label(mouse_handler_.get_last_hex());
			res = !events::commands_disabled && gamestate_.board_.map().on_board(mouse_handler_.get_last_hex())
				&& !gui_->shrouded(mouse_handler_.get_last_hex())
				&& !is_observer()
				&& (!label || !label->immutable());
			break;
		}
		case hotkey::HOTKEY_CONTINUE_MOVE: {
			if(browse_ || events::commands_disabled)
				return false;

			if( (menu_handler_.current_unit().valid())
				&& (menu_handler_.current_unit()->move_interrupted()))
				return true;
			const unit_map::const_iterator i = gamestate_.board_.units().find(mouse_handler_.get_selected_hex());
			if (!i.valid()) return false;
			return i->move_interrupted();
		}
		case hotkey::HOTKEY_WB_TOGGLE:
			return !is_observer();
		case hotkey::HOTKEY_WB_EXECUTE_ACTION:
		case hotkey::HOTKEY_WB_EXECUTE_ALL_ACTIONS:
			return whiteboard_manager_->can_enable_execution_hotkeys();
		case hotkey::HOTKEY_WB_DELETE_ACTION:
			return whiteboard_manager_->can_enable_modifier_hotkeys();
		case hotkey::HOTKEY_WB_BUMP_UP_ACTION:
		case hotkey::HOTKEY_WB_BUMP_DOWN_ACTION:
			return whiteboard_manager_->can_enable_reorder_hotkeys();
		case hotkey::HOTKEY_WB_SUPPOSE_DEAD:
		{
			//@todo re-enable this once we figure out a decent UI for suppose_dead
			return false;
		}

		default: return play_controller::can_execute_command(cmd, index);
	}
	return res;
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
