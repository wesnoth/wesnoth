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

#include "global.hpp"

#include "actions/vision.hpp"
#include "game_end_exceptions.hpp"
#include "game_events/handlers.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "map_label.hpp"
#include "replay.hpp"
#include "random_new_deterministic.hpp"
#include "replay_controller.hpp"
#include "resources.hpp"
#include "savegame.hpp"
#include "synced_context.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)

static lg::log_domain log_replay("replay");
#define DBG_REPLAY LOG_STREAM(debug, log_replay)
#define LOG_REPLAY LOG_STREAM(info, log_replay)

LEVEL_RESULT play_replay_level(const config& game_config,
		const config* level, CVideo& video, game_state& state_of_game)
{
	try{
		const int ticks = SDL_GetTicks();
		int num_turns = (*level)["turns"].to_int(-1);

		config init_level = *level;
		carryover_info sides(state_of_game.carryover_sides);
		sides.transfer_to(init_level);
		state_of_game.carryover_sides = sides.to_config();

		DBG_NG << "creating objects... " << (SDL_GetTicks() - ticks) << "\n";
		replay_controller replaycontroller(init_level, state_of_game, ticks, num_turns, game_config, video);
		DBG_NG << "created objects... " << (SDL_GetTicks() - replaycontroller.get_ticks()) << "\n";
		const events::command_disabler disable_commands;

		//replay event-loop
		for (;;){
			replaycontroller.play_slice();
		}
	}
	catch(end_level_exception&){
		DBG_NG << "play_replay_level: end_level_exception\n";
	}

	return VICTORY;
}

replay_controller::replay_controller(const config& level,
		game_state& state_of_game, const int ticks, const int num_turns,
		const config& game_config, CVideo& video) :
	play_controller(level, state_of_game, ticks, num_turns, game_config, video, false),
	teams_start_(teams_),
	gamestate_start_(gamestate_),
	units_start_(units_),
	tod_manager_start_(level, num_turns),
	current_turn_(1),
	is_playing_(false),
	show_everything_(false),
	show_team_(state_of_game.classification().campaign_type == "multiplayer" ? 0 : 1)
{
	tod_manager_start_ = tod_manager_;

	// Our parent class correctly detects that we are loading a game. However,
	// we are not loading mid-game, so from here on, treat this as not loading
	// a game. (Allows turn_1 et al. events to fire at the correct time.)
	loading_game_ = false;

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

	gui_->scroll_to_leader(units_, player_number_, display::WARP);
	update_locker lock_display((*gui_).video(),false);
	for(std::vector<team>::iterator t = teams_.begin(); t != teams_.end(); ++t) {
		t->reset_objectives_changed();
	}

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
	const config &theme_cfg = get_theme(game_config_, level_["theme"]);
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

void replay_controller::update_replay_ui()
{
	//check if we have all buttons - if someone messed with theme then some buttons may be missing
	//if any of the buttons is missing, we just disable every one
	if(!replay_ui_has_all_buttons()) {
		gui::button *play_b = play_button(), *stop_b = stop_button(),
		            *reset_b = reset_button(), *play_turn_b = play_turn_button(),
		            *play_side_b = play_side_button();

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
}

void replay_controller::replay_ui_playback_should_stop()
{
	if(!replay_ui_has_all_buttons())
		return;

	if(!recorder.at_end()) {
		play_button()->enable(true);
		reset_button()->enable(true);
		play_turn_button()->enable(true);
		play_side_button()->enable(true);

		play_button()->release();
		play_turn_button()->release();
		play_side_button()->release();
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

	gui_->clear_chat_messages();
	is_playing_ = false;
	player_number_ = 1;
	current_turn_ = 1;
	it_is_a_new_turn_ = true;
	skip_replay_ = false;
	tod_manager_= tod_manager_start_;
	recorder.start_replay();
	recorder.set_skip(false);
	units_ = units_start_;
	gamestate_ = gamestate_start_;
	teams_ = teams_start_;
	if (events_manager_ ){
		// NOTE: this double reset is required so that the new
		// instance of game_events::manager isn't created before the
		// old manager is actually destroyed (triggering an assertion
		// failure)
		events_manager_.reset();
		events_manager_.reset(new game_events::manager(level_));
	}

	gui_->labels().read(level_);

	resources::gamedata->rng().seed_random(level_["random_seed"], level_["random_calls"]);
	statistics::fresh_stats();
	set_victory_when_enemies_defeated(level_["victory_when_enemies_defeated"].to_bool(true));

	// Add era events for MP game.
	if (const config &era_cfg = level_.child("era")) {
		game_events::add_events(era_cfg.child_range("event"), "era_events");
	}

	// Scenario initialization. (c.f. playsingle_controller::play_scenario())
	fire_preload();
	if(true){ //block for set_scontext_synced
		config* pstart = recorder.get_next_action();
		assert(pstart->has_child("start"));
		/*
			use this after recorder.add_synced_command
			because set_scontext_synced sets the checkup to the last added command
		*/
		set_scontext_synced sync;
		
		fire_prestart(true);
		init_gui();
		fire_start(true);
	}
	// Since we did not fire the start event, it_is_a_new_turn_ has the wrong value.
	it_is_a_new_turn_ = true;
	update_gui();

	reset_replay_ui();
}

void replay_controller::stop_replay(){
	is_playing_ = false;
}

void replay_controller::replay_next_turn(){
	is_playing_ = true;
	replay_ui_playback_should_start();

	play_turn();

 	if (!skip_replay_ || !is_playing_){
		gui_->scroll_to_leader(units_, player_number_,game_display::ONSCREEN,false);
	}

	replay_ui_playback_should_stop();
}

void replay_controller::replay_next_side(){
	is_playing_ = true;
	replay_ui_playback_should_start();

	play_side(player_number_ - 1, false);

	if (!skip_replay_ || !is_playing_) {
		gui_->scroll_to_leader(units_, player_number_,game_display::ONSCREEN,false);
	}

	replay_ui_playback_should_stop();
}

void replay_controller::process_oos(const std::string& msg) const
{
	if (game_config::ignore_replay_errors) return;

	std::stringstream message;
	message << _("The replay is corrupt/out of sync. It might not make much sense to continue. Do you want to save the game?");
	message << "\n\n" << _("Error details:") << "\n\n" << msg;

	savegame::oos_savegame save(to_config());
	save.save_game_interactive(resources::screen->video(), message.str(), gui::YES_NO); // can throw end_level_exception
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
	recorder.set_skip(skip_replay_);
}

//move all sides till stop/end
void replay_controller::play_replay(){

	if (recorder.at_end()){
		//shouldn't actually happen
		return;
	}

	try{
		is_playing_ = true;
		replay_ui_playback_should_start();

		DBG_REPLAY << "starting main loop\n" << (SDL_GetTicks() - ticks_) << "\n";
		for(; !recorder.at_end() && is_playing_; first_player_ = 1) {
			play_turn();
		}

		if (!is_playing_) {
			gui_->scroll_to_leader(units_, player_number_,game_display::ONSCREEN,false);
		}
	}
	catch(end_level_exception& e){
		if (e.result == QUIT) throw;
	}

	replay_ui_playback_should_stop();
}

//make all sides move, then stop
void replay_controller::play_turn(){

	LOG_REPLAY << "turn: " << current_turn_ << "\n";

	gui_->new_turn();
	gui_->invalidate_game_status();
	events::raise_draw_event();

	bool last_team = false;

	while ( (!last_team) && (!recorder.at_end()) && is_playing_ ){
		last_team = static_cast<size_t>(player_number_) == teams_.size();
		play_side(player_number_ - 1, false);
		play_slice();
	}
}

//make only one side move
void replay_controller::play_side(const unsigned int /*team_index*/, bool){

	DBG_REPLAY << "Status turn number: " << turn() << "\n";
	DBG_REPLAY << "Replay_Controller turn number: " << current_turn_ << "\n";
	DBG_REPLAY << "Player number: " << player_number_ << "\n";

	try{
		// If a side is empty skip over it.
		bool has_end_turn = true;
		if (!current_team().is_empty()) {
			statistics::reset_turn_stats(current_team().save_id());

			play_controller::init_side(player_number_ - 1, true);

			DBG_REPLAY << "doing replay " << player_number_ << "\n";
			// if have reached the end we don't want to execute finish_side_turn and finish_turn
			// becasue we might not have enough data to execute them (like advancements during turn_end for example)
			// !has_end_turn == we reached the end of teh replay without finding and end turn tag.
			has_end_turn = do_replay(player_number_);
			if(!has_end_turn)
			{
				return;
			}
			finish_side_turn();
		}

		player_number_++;

		if (static_cast<size_t>(player_number_) > teams_.size()) {
			//during the orginal game player_number_ would also be teams_.size(),
			player_number_ = teams_.size();
			finish_turn();
			tod_manager_.next_turn();
			it_is_a_new_turn_ = true;
			player_number_ = 1;
			current_turn_++;
			gui_->new_turn();
		}

		// This is necessary for replays in order to show possible movements.
		BOOST_FOREACH(unit &u, units_) {
			if (u.side() == player_number_) {
				u.new_turn();
			}
		}

		update_teams();
		update_gui();
	}
	catch(end_level_exception& e){
		//VICTORY/DEFEAT end_level_exception shall not return to title screen
		if (e.result != VICTORY && e.result != DEFEAT) throw;
	}
}

void replay_controller::update_teams(){

	int next_team = player_number_;
	if(static_cast<size_t>(next_team) > teams_.size()) {
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

void replay_controller::update_gui(){
	(*gui_).recalculate_minimap();
	(*gui_).redraw_minimap();
	(*gui_).invalidate_all();
	events::raise_draw_event();
	(*gui_).draw();
}

void replay_controller::preferences(){
	play_controller::preferences();
	update_gui();
}

void replay_controller::show_statistics(){
	menu_handler_.show_statistics(gui_->playing_team()+1);
}

void replay_controller::handle_generic_event(const std::string& name){

	if( name == "completely_redrawn" ) {
		update_replay_ui();

		gui::button* skip_animation_button = gui_->find_action_button("skip-animation");
		if(skip_animation_button) {
			skip_animation_button->set_check(skip_replay_);
		}
	} else {
		rebuild_replay_theme();
	}
}

bool replay_controller::can_execute_command(const hotkey::hotkey_command& cmd, int index) const
{
	hotkey::HOTKEY_COMMAND command = cmd.id;
	bool result = play_controller::can_execute_command(cmd,index);

	switch(command) {

	//commands we can always do
	case hotkey::HOTKEY_REPLAY_RESET:
	case hotkey::HOTKEY_REPLAY_SHOW_EVERYTHING:
	case hotkey::HOTKEY_REPLAY_SHOW_EACH:
	case hotkey::HOTKEY_REPLAY_SHOW_TEAM1:
	case hotkey::HOTKEY_REPLAY_SKIP_ANIMATION:
	case hotkey::HOTKEY_SAVE_GAME:
	case hotkey::HOTKEY_SAVE_REPLAY:
	case hotkey::HOTKEY_CHAT_LOG:
		return true;

	//commands we only can do before the end of the replay
	case hotkey::HOTKEY_REPLAY_PLAY:
	case hotkey::HOTKEY_REPLAY_STOP:
	case hotkey::HOTKEY_REPLAY_NEXT_TURN:
	case hotkey::HOTKEY_REPLAY_NEXT_SIDE:
		if(recorder.at_end()) {
			return false;
		} else {
			return true;
		}

	default:
		return result;
	}
}
