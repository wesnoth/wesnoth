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

#include "global.hpp"

#include "replay_controller.hpp"

#include "carryover.hpp"
#include "actions/vision.hpp"
#include "display_chat_manager.hpp"
#include "game_end_exceptions.hpp"
#include "game_errors.hpp" //needed to be thrown
#include "game_events/manager.hpp"
#include "game_events/pump.hpp"
#include "gettext.hpp"
#include "hotkey_handler_replay.hpp"
#include "log.hpp"
#include "map_label.hpp"
#include "mouse_handler_base.hpp"
#include "replay.hpp"
#include "random_new_deterministic.hpp"
#include "resources.hpp"
#include "savegame.hpp"
#include "saved_game.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "statistics.hpp"
#include "synced_context.hpp"
#include "unit_id.hpp"
#include "whiteboard/manager.hpp"

#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)

static lg::log_domain log_replay("replay");
#define DBG_REPLAY LOG_STREAM(debug, log_replay)
#define LOG_REPLAY LOG_STREAM(info, log_replay)
#define ERR_REPLAY LOG_STREAM(err, log_replay)

void play_replay_level_main_loop(replay_controller & replaycontroller, bool & is_unit_test);

LEVEL_RESULT play_replay_level(const config& game_config, const tdata_cache & tdata,
		CVideo& video, saved_game& state_of_game, bool is_unit_test)
{
	const int ticks = SDL_GetTicks();

	DBG_NG << "creating objects... " << (SDL_GetTicks() - ticks) << std::endl;

	boost::scoped_ptr<replay_controller> rc;

	const events::command_disabler disable_commands;

	rc.reset(new replay_controller(state_of_game.get_replay_starting_pos(), state_of_game, ticks, game_config, tdata, video));
	DBG_NG << "created objects... " << (SDL_GetTicks() - rc->get_ticks()) << std::endl;

	//replay event-loop
	play_replay_level_main_loop(*rc, is_unit_test);
	if(rc->is_regular_game_end())
	{
	//	return rc->get_end_level_data_const().is_victory ? VICTORY : DEFEAT;
		//The replay contained the whole scenario, returns VICTORY regardless of the original outcome.
		return VICTORY;
	}
	else
	{
		//The replay was finished without reaching the scenario end.
		return QUIT;
	}
}

void play_replay_level_main_loop(replay_controller & replaycontroller, bool & is_unit_test) {
	if (is_unit_test) {
		return replaycontroller.try_run_to_completion();
	}

	for (;;) {
		//Quits by quit_level_exception
		replaycontroller.play_slice();
	}
}

void replay_controller::try_run_to_completion() {
	for (;;) {
		play_slice();
		if (resources::recorder->at_end()) {
			return;
		} else {
			if (!is_playing_) {
				play_replay();
			}
		}
	}
}

replay_controller::replay_controller(const config& level,
		saved_game& state_of_game, const int ticks,
		const config& game_config, 
		const tdata_cache & tdata, CVideo& video)
	: play_controller(level, state_of_game, ticks, game_config, tdata, video, false)
	, gameboard_start_(gamestate_.board_)
	, tod_manager_start_(level)
	, is_playing_(false)
	, show_everything_(false)
	, show_team_(state_of_game.classification().campaign_type == game_classification::MULTIPLAYER ? 0 : 1)
{
	hotkey_handler_.reset(new hotkey_handler(*this, saved_game_)); //upgrade hotkey handler to the replay controller version

	// Our game_data correctly detects that we are loading a game. However,
	// we are not loading mid-game, so from here on, treat this as not loading
	// a game. (Allows turn_1 et al. events to fire at the correct time.)
	init();
	reset_replay();
}

replay_controller::~replay_controller()
{
	//YogiHH
	//not absolutely sure if this is needed, but it makes me feel a lot better ;-)
	//feel free to delete this if it is not necessary
	gui_->get_theme().theme_reset_event().detach_handler(this);
	gui_->complete_redraw_event().detach_handler(this);
}

void replay_controller::init(){
	DBG_REPLAY << "in replay_controller::init()...\n";
	
	last_replay_action = REPLAY_FOUND_END_MOVE;
	//guarantee the cursor goes back to 'normal' at the end of the level
	const cursor::setter cursor_setter(cursor::NORMAL);
	init_replay_display();
}

void replay_controller::init_gui(){
	DBG_NG << "Initializing GUI... " << (SDL_GetTicks() - ticks_) << "\n";
	play_controller::init_gui();

	if (show_team_)
		gui_->set_team(show_team_ - 1, show_everything_);
	else
		gui_->set_team(0, show_everything_);

	gui_->scroll_to_leader(player_number_, display::WARP);
	update_locker lock_display((*gui_).video(),false);
	BOOST_FOREACH(const team & t, gamestate_.board_.teams()) {
		t.reset_objectives_changed();
	}
	get_hotkey_command_executor()->set_button_state(*gui_); 
	update_replay_ui();
}

void replay_controller::init_replay_display(){
	DBG_REPLAY << "initializing replay-display... " << (SDL_GetTicks() - ticks_) << "\n";

	rebuild_replay_theme();
	gui_->get_theme().theme_reset_event().attach_handler(this);
	gui_->complete_redraw_event().attach_handler(this);
	DBG_REPLAY << "done initializing replay-display... " << (SDL_GetTicks() - ticks_) << "\n";
}

void replay_controller::rebuild_replay_theme()
{
	const config &theme_cfg = controller_base::get_theme(game_config_, level_["theme"]);
	if (const config &res = theme_cfg.child("resolution"))
	{
		if (const config &replay_theme_cfg = res.child("replay"))
			gui_->get_theme().modify(replay_theme_cfg);
		gui_->get_theme().modify_label("time-icon", _ ("current local time"));
		//Make sure we get notified if the theme is redrawn completely. That way we have
		//a chance to restore the replay controls of the theme as well.
		gui_->invalidate_theme();
	}
}

gui::button* replay_controller::play_button()
{
	return gui_->find_action_button("button-playreplay");
}

gui::button* replay_controller::stop_button()
{
	return gui_->find_action_button("button-stopreplay");
}

gui::button* replay_controller::reset_button()
{
	return gui_->find_action_button("button-resetreplay");
}

gui::button* replay_controller::play_turn_button()
{
	return gui_->find_action_button("button-nextturn");
}

gui::button* replay_controller::play_side_button()
{
	return gui_->find_action_button("button-nextside");
}

gui::button* replay_controller::play_move_button()
{
	return gui_->find_action_button("button-nextmove");
}

void replay_controller::update_replay_ui()
{
	//check if we have all buttons - if someone messed with theme then some buttons may be missing
	//if any of the buttons is missing, we just disable every one
	if(!replay_ui_has_all_buttons()) {
		gui::button *play_b = play_button(), *stop_b = stop_button(),
		            *reset_b = reset_button(), *play_turn_b = play_turn_button(),
		            *play_side_b = play_side_button(), *play_move_b = play_move_button();

		if(play_b) {
			play_b->enable(false);
		}

		if(stop_b) {
			stop_b->enable(false);
		}

		if(reset_b) {
			reset_b->enable(false);
		}

		if(play_turn_b) {
			play_turn_b->enable(false);
		}

		if(play_side_b) {
			play_side_b->enable(false);
		}

		if (play_move_b) {
			play_move_b->enable(false);
		} 
	}
}

void replay_controller::replay_ui_playback_should_start()
{
	if(!replay_ui_has_all_buttons())
		return;

	play_button()->enable(false);
	reset_button()->enable(false);
	play_turn_button()->enable(false);
	play_side_button()->enable(false);
	play_move_button()->enable(false);
}

void replay_controller::replay_ui_playback_should_stop()
{
	if(!replay_ui_has_all_buttons())
		return;

	if(!resources::recorder->at_end()) {
		play_button()->enable(true);
		reset_button()->enable(true);
		play_turn_button()->enable(true);
		play_side_button()->enable(true);
		play_move_button()->enable(true);

		play_button()->release();
		play_turn_button()->release();
		play_side_button()->release();
		play_move_button()->release();
	} else {
		reset_button()->enable(true);
		stop_button()->enable(false);
	}

	if(!is_playing_) {
		//user interrupted
		stop_button()->release();
	}
}

void replay_controller::reset_replay_ui()
{
	if(!replay_ui_has_all_buttons())
		return;

	play_button()->enable(true);
	stop_button()->enable(true);
	reset_button()->enable(true);
	play_turn_button()->enable(true);
	play_side_button()->enable(true);
}


void replay_controller::reset_replay()
{
	DBG_REPLAY << "replay_controller::reset_replay\n";

	gui_->get_chat_manager().clear_chat_messages();
	is_playing_ = false;
	player_number_ = level_["playing_team"].to_int() + 1;
	it_is_a_new_turn_ = level_["it_is_a_new_turn"].to_bool(true);
	init_side_done_ = level_["init_side_done"].to_bool(false);
	skip_replay_ = false;
	gamestate_.tod_manager_= tod_manager_start_;
	resources::recorder->start_replay();
	gamestate_.board_ = gameboard_start_;
	gui_->change_display_context(&gamestate_.board_); //this doesn't change the pointer value, but it triggers the gui to update the internal terrain builder object,
						   //idk what the consequences of not doing that are, but its probably a good idea to do it, esp. if layout
						   //of game_board changes in the future


	/*if (events_manager_ ){
		// NOTE: this double reset is required so that the new
		// instance of game_events::manager isn't created before the
		// old manager is actually destroyed (triggering an assertion
		// failure)
		events_manager_.reset();
		events_manager_.reset(new game_events::manager(level_));
	}*/

	gamestate_.events_manager_.reset();
	resources::game_events=NULL;
	gamestate_.lua_kernel_.reset();
	resources::lua_kernel=NULL;
	gamestate_.lua_kernel_.reset(new game_lua_kernel(level_, &gui_->video(), gamestate_, *this, *gamestate_.reports_));
	gamestate_.lua_kernel_->set_game_display(gui_.get());
	resources::lua_kernel=gamestate_.lua_kernel_.get();
	gamestate_.game_events_resources_->lua_kernel = resources::lua_kernel;
	gamestate_.events_manager_.reset(new game_events::manager(level_, gamestate_.game_events_resources_));
	resources::game_events=gamestate_.events_manager_.get();

	gui_->labels().read(level_);

	*resources::gamedata = game_data(level_);
	n_unit::id_manager::instance().set_save_id(level_["next_underlying_unit_id"]);
	statistics::fresh_stats();

	gui_->needs_rebuild(true);
	gui_->maybe_rebuild();

	// Scenario initialization. (c.f. playsingle_controller::play_scenario())
	start_game();
	update_gui();

	reset_replay_ui();
}

void replay_controller::stop_replay()
{
	is_playing_ = false;
}

void replay_controller::replay_next_turn()
{
	is_playing_ = true;
	replay_ui_playback_should_start();

	play_turn();

	if (!is_skipping_replay() || !is_playing_) {
		gui_->scroll_to_leader(player_number_,game_display::ONSCREEN,false);
	}

	replay_ui_playback_should_stop();
}

void replay_controller::replay_next_move_or_side(bool one_move)
{
	is_playing_ = true;
	replay_ui_playback_should_start();
	
	play_move_or_side(one_move);
	while (current_team().is_empty()) {
		play_move_or_side(one_move);
	}

	if ( (!is_skipping_replay() || !is_playing_) && (last_replay_action == REPLAY_FOUND_END_TURN) ){
		gui_->scroll_to_leader(player_number_,game_display::ONSCREEN,false);
	}

	replay_ui_playback_should_stop();
}

void replay_controller::replay_next_side()
{
	return replay_next_move_or_side(false);
}

void replay_controller::replay_next_move()
{
	return replay_next_move_or_side(true);
}


void replay_controller::process_oos(const std::string& msg) const
{
	if (game_config::ignore_replay_errors) {
		return;
	}

	std::stringstream message;
	message << _("The replay is corrupt/out of sync. It might not make much sense to continue. Do you want to save the game?");
	message << "\n\n" << _("Error details:") << "\n\n" << msg;

	if (non_interactive()) {
		throw game::game_error(message.str());
	} else {
		update_savegame_snapshot();
		savegame::oos_savegame save(saved_game_, *gui_);
		save.save_game_interactive(resources::screen->video(), message.str(), gui::YES_NO); // can throw end_level_exception
	}
}

void replay_controller::replay_show_everything(){
	show_everything_ = true;
	show_team_ = 0;
	update_teams();
	update_gui();
}

void replay_controller::replay_show_each(){
	show_everything_ = false;
	show_team_ = 0;
	update_teams();
	update_gui();
}

void replay_controller::replay_show_team1(){
	show_everything_ = false;
	show_team_ = 1;
	update_teams();
	update_gui();
}

void replay_controller::replay_skip_animation(){
	skip_replay_ = !skip_replay_;
}

//move all sides till stop/end
void replay_controller::play_replay()
{

	if (resources::recorder->at_end())
	{
	}

	is_playing_ = true;
	replay_ui_playback_should_start();

	play_replay_main_loop();

	if (!is_playing_) {
		gui_->scroll_to_leader(player_number_,game_display::ONSCREEN,false);
	}

	replay_ui_playback_should_stop();
}

void replay_controller::play_replay_main_loop()
{
	DBG_REPLAY << "starting main loop\n" << (SDL_GetTicks() - ticks_) << "\n";
	while(!resources::recorder->at_end() && is_playing_) {
		play_turn();
	}
}

//make all sides move, then stop
void replay_controller::play_turn()
{

	LOG_REPLAY << "turn: " << turn() << "\n";

	gui_->new_turn();
	gui_->invalidate_game_status();
	events::raise_draw_event();

	bool last_team = false;

	while ( (!last_team) && (!resources::recorder->at_end()) && is_playing_ ){
		last_team = static_cast<size_t>(player_number_) == gamestate_.board_.teams().size();
		play_side();
		play_slice();
	}
}


void replay_controller::play_side() {
	return play_move_or_side(false);
}

void replay_controller::play_move() {
	return play_move_or_side(true);
}

//make only one side move
void replay_controller::play_move_or_side(bool one_move) {
	
	DBG_REPLAY << "Status turn number: " << turn() << "\n";
	DBG_REPLAY << "Player number: " << player_number_ << "\n";

	// If a side is empty skip over it.
	if (!current_team().is_empty()) {
		statistics::reset_turn_stats(current_team().save_id());

		if (last_replay_action == REPLAY_FOUND_END_TURN) {
			play_controller::init_side_begin(true);
		}

		DBG_REPLAY << "doing replay " << player_number_ << "\n";
		// if have reached the end we don't want to execute finish_side_turn and finish_turn
		// because we might not have enough data to execute them (like advancements during turn_end for example)
		
		last_replay_action = do_replay(one_move);
		if(last_replay_action != REPLAY_FOUND_END_TURN) {
			//We reached the end of the replay without finding an end turn tag.
			return;
		}

		finish_side_turn();
	}

	player_number_++;

	if (static_cast<size_t>(player_number_) > gamestate_.board_.teams().size()) {
		//during the orginal game player_number_ would also be gamestate_.board_.teams().size(),
		player_number_ = gamestate_.board_.teams().size();
		finish_turn();
		bool is_time_left = gamestate_.tod_manager_.next_turn(*resources::gamedata);
		if(!is_time_left) {
			set_scontext_synced_base sync;
			pump().fire("time over");
		}
		it_is_a_new_turn_ = true;
		player_number_ = 1;
		gui_->new_turn();
	}

	// This is necessary for replays in order to show possible movements.
	// But it causes OOS with the original game since the units have wrong movement_left value during side turn events.
	// gamestate_.board_.new_turn(player_number_);

	update_teams();
	update_gui();
}

void replay_controller::update_teams()
{

	int next_team = player_number_;
	if(static_cast<size_t>(next_team) > gamestate_.board_.teams().size()) {
		next_team = 1;
	}

	if ( show_team_ == 0 ) {
		gui_->set_team(next_team - 1, show_everything_);
	} else {
		gui_->set_team(show_team_ - 1, show_everything_);
	}

	gui_->set_playing_team(next_team - 1);
	gui_->invalidate_all();
}

void replay_controller::update_gui()
{
	(*gui_).recalculate_minimap();
	(*gui_).redraw_minimap();
	(*gui_).invalidate_all();
	events::raise_draw_event();
	(*gui_).draw();
}

void replay_controller::handle_generic_event(const std::string& name)
{

	if( name == "completely_redrawn" ) {
		update_replay_ui();

		gui::button* skip_animation_button = gui_->find_action_button("skip-animation");
		if(skip_animation_button) {
			skip_animation_button->set_check(is_skipping_replay());
		}
	} else {
		rebuild_replay_theme();
	}
}

bool replay_controller::recorder_at_end() {
	return resources::recorder->at_end();
}
