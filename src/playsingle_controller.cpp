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
#include "config_assign.hpp"
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
#include "resources.hpp"
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
	, replay_sender_(*resources::recorder)
	, network_reader_()
	, turn_data_(replay_sender_, network_reader_)
	, end_turn_(END_TURN_NONE)
	, skip_next_turn_(false)
{
	hotkey_handler_.reset(new hotkey_handler(*this, saved_game_)); //upgrade hotkey handler to the sp (whiteboard enabled) version

	
	// game may need to start in linger mode
	linger_ = this->is_regular_game_end();

	ai::game_info ai_info;
	ai::manager::set_ai_info(ai_info);
	ai::manager::add_observer(this) ;

	plugins_context_->set_accessor_string("level_result", boost::bind(&playsingle_controller::describe_result, this));
	plugins_context_->set_accessor_int("turn", boost::bind(&play_controller::turn, this));
}

std::string playsingle_controller::describe_result() const
{
	if(!is_regular_game_end()) {
		return "NONE";
	}
	else if(get_end_level_data_const().is_victory){
		return "VICTORY";
	}
	else {
		return "DEFEAT";
	}
}

playsingle_controller::~playsingle_controller()
{
	ai::manager::remove_observer(this) ;
	ai::manager::clear_ais() ;
}

void playsingle_controller::init_gui(){
	LOG_NG << "Initializing GUI... " << (SDL_GetTicks() - ticks()) << "\n";
	play_controller::init_gui();

	if(gamestate().first_human_team_ != -1) {
		gui_->scroll_to_tile(gamestate().board_.map().starting_position(gamestate().first_human_team_ + 1), game_display::WARP);
	}
	gui_->scroll_to_tile(gamestate().board_.map().starting_position(1), game_display::WARP);

	update_locker lock_display(gui_->video(), is_skipping_replay());
	gui_->draw();
	get_hotkey_command_executor()->set_button_state(*gui_);
	events::raise_draw_event();
}

void playsingle_controller::report_victory(
	std::ostringstream &report, team& t,
	int finishing_bonus_per_turn, int turns_left, int finishing_bonus)
{
	report << "<small>" << _("Remaining gold: ") << utils::half_signed_value(t.gold()) << "</small>";

	if(t.carryover_bonus()) {
		if (turns_left > -1) {
			report << "\n\n<b>" << _("Turns finished early: ") << turns_left << "</b>\n"
				   << "<small>" << _("Early finish bonus: ") << finishing_bonus_per_turn << _(" per turn") << "</small>\n"
				   << "<small>" << _("Total bonus: ") << finishing_bonus << "</small>\n";
		}
		report << "<small>" << _("Total gold: ") << utils::half_signed_value(t.gold() + finishing_bonus) << "</small>";
	}
	if (t.gold() > 0) {
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
	report << "\n" << goldmsg;
}

void playsingle_controller::play_scenario_init(const config& level) {
	// At the beginning of the scenario, save a snapshot as replay_start
	if(saved_game_.replay_start().empty()){
		saved_game_.replay_start() = to_config();
	}
	start_game(level);
	if( saved_game_.classification().random_mode != "" && (network::nconnections() != 0)) {
		// This won't cause errors later but we should notify the user about it in case he didn't knew it.
		gui2::show_transient_message(
			gui_->video(),
			// TODO: find a better title
			_("Game Error"),
			_("This multiplayer game uses an alternative random mode, if you don't know what this message means, then most likely someone is cheating or someone reloaded a corrupt game.")
		);
	}
	return;
}

void playsingle_controller::play_scenario_main_loop()
{
	LOG_NG << "starting main loop\n" << (SDL_GetTicks() - ticks()) << "\n";


	// Avoid autosaving after loading, but still
	// allow the first turn to have an autosave.
	ai_testing::log_game_start();
	if(gamestate().board_.teams().empty())
	{
		ERR_NG << "Playing game with 0 teams." << std::endl;
	}
	while(true) {
		play_turn();
		if (is_regular_game_end()) {
			return;
		}
		player_number_ = 1;
	} //end for loop
}

LEVEL_RESULT playsingle_controller::play_scenario(
	const config::const_child_itors &story, const config& level)
{
	LOG_NG << "in playsingle_controller::play_scenario()...\n";

	// Start music.
	BOOST_FOREACH(const config &m, level.child_range("music")) {
		sound::play_music_config(m);
	}
	sound::commit_music_changes();

	if(!this->is_skipping_replay()) {
		show_story(*gui_, get_scenario_name(), story);
	}
	gui_->labels().read(level);

	// Read sound sources
	assert(soundsources_manager_ != NULL);
	BOOST_FOREACH(const config &s, level.child_range("sound_source")) {
		try {
			soundsource::sourcespec spec(s);
			soundsources_manager_->add(spec);
		} catch (bad_lexical_cast &) {
			ERR_NG << "Error when parsing sound_source config: bad lexical cast." << std::endl;
			ERR_NG << "sound_source config was: " << s.debug() << std::endl;
			ERR_NG << "Skipping this sound source..." << std::endl;
		}
	}
	LOG_NG << "entering try... " << (SDL_GetTicks() - ticks()) << "\n";
	try {
		play_scenario_init(level);
		// clears level config;
		this->saved_game_.remove_snapshot();

		if (!is_regular_game_end() && !linger_) {
			play_scenario_main_loop();
		}
		if (game_config::exit_at_end) {
			exit(0);
		}
		const bool is_victory = get_end_level_data_const().is_victory;

		if(gamestate().gamedata_.phase() <= game_data::PRESTART) {
			sdl::draw_solid_tinted_rectangle(
				0, 0, gui_->video().getx(), gui_->video().gety(), 0, 0, 0, 1.0,
				gui_->video().getSurface()
				);
			update_rect(0, 0, gui_->video().getx(), gui_->video().gety());
		}

		ai_testing::log_game_end();

		const end_level_data& end_level = get_end_level_data_const();
		if (!end_level.transient.custom_endlevel_music.empty()) {
			if (!is_victory) {
				set_defeat_music_list(end_level.transient.custom_endlevel_music);
			} else {
				set_victory_music_list(end_level.transient.custom_endlevel_music);
			}
		}

		if (gamestate().board_.teams().empty())
		{
			//store persistent teams
			saved_game_.set_snapshot(config());

			return LEVEL_RESULT::VICTORY; // this is probably only a story scenario, i.e. has its endlevel in the prestart event
		}
		if(linger_) {
			LOG_NG << "resuming from loaded linger state...\n";
			//as carryover information is stored in the snapshot, we have to re-store it after loading a linger state
			saved_game_.set_snapshot(config());
			if(!is_observer()) {
				persist_.end_transaction();
			}
			return LEVEL_RESULT::VICTORY;
		}
		pump().fire(is_victory ? "victory" : "defeat");
		{ // Block for set_scontext_synced_base
			set_scontext_synced_base sync;
			pump().fire("scenario_end");
		}
		if(end_level.proceed_to_next_level) {
			gamestate().board_.heal_all_survivors();
		}
		if(is_observer()) {
			gui2::show_transient_message(gui_->video(), _("Game Over"), _("The game is over."));
			return LEVEL_RESULT::OBSERVER_END;
		}
		// If we're a player, and the result is victory/defeat, then send
		// a message to notify the server of the reason for the game ending.
		network::send_data(config_of
			("info", config_of
				("type", "termination")
				("condition", "game over")
				("result", is_victory ? "victory" : "defeat")
			));
		// Play victory music once all victory events
		// are finished, if we aren't observers.
		//
		// Some scenario authors may use 'continue'
		// result for something that is not story-wise
		// a victory, so let them use [music] tags
		// instead should they want special music.
		const std::string& end_music = is_victory ? select_victory_music() : select_defeat_music();
		if(end_music.empty() != true) {
			sound::play_music_once(end_music);
		}
		persist_.end_transaction();
		return is_victory ? LEVEL_RESULT::VICTORY : LEVEL_RESULT::DEFEAT;
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

		scoped_savegame_snapshot snapshot(*this);
		savegame::ingame_savegame save(saved_game_, *gui_, preferences::save_compression_format());
		save.save_game_interactive(gui_->video(), _("A network disconnection has occurred, and the game cannot continue. Do you want to save the game?"), gui::YES_NO);
		if(disconnect) {
			throw network::error();
		} else {
			return LEVEL_RESULT::QUIT;
		}
	}

	return LEVEL_RESULT::QUIT;
}

void playsingle_controller::play_idle_loop()
{
	while(!should_return_to_play_side()) {
		play_slice_catch();
		gui_->draw();
		SDL_Delay(10);
	}
}

void playsingle_controller::play_side_impl()
{
	if (!skip_next_turn_) {
		end_turn_ = END_TURN_NONE;
	}
	if((current_team().is_local_human() && current_team().is_proxy_human())) {
		LOG_NG << "is human...\n";
		// If a side is dead end the turn, but play at least side=1's
		// turn in case all sides are dead
		if (gamestate().board_.side_units(player_number_) == 0 && !(gamestate().board_.units().size() == 0 && player_number_ == 1)) {
			end_turn_ = END_TURN_REQUIRED;
		}

		before_human_turn();
		if (end_turn_ == END_TURN_NONE) {
			play_human_turn();
		}
		if ( !player_type_changed_ && !is_regular_game_end()) {
			after_human_turn();
		}
		LOG_NG << "human finished turn...\n";

	} else if(current_team().is_local_ai() || (current_team().is_local_human() && current_team().is_droid())) {
		play_ai_turn();
	} else if(current_team().is_network()) {
		play_network_turn();
	} else if(current_team().is_local_human() && current_team().is_idle()) {
		end_turn_enable(false);
		do_idle_notification();
		before_human_turn();
		if (end_turn_ == END_TURN_NONE) {
			play_idle_loop();
		}
	}
	else {
		// we should have skipped over empty controllers before so this shouldn't be possible
		ERR_NG << "Found invalid side controller " << current_team().controller().to_string() << " (" << current_team().proxy_controller().to_string() << ") for side " << current_team().side() << "\n";
	}
}

void playsingle_controller::before_human_turn()
{
	log_scope("player turn");
	assert(!linger_);
	if(end_turn_ != END_TURN_NONE || is_regular_game_end()) {
		return;
	}

	if(init_side_done_now_) {
		scoped_savegame_snapshot snapshot(*this);
		savegame::autosave_savegame save(saved_game_, *gui_, preferences::save_compression_format());
		save.autosave(game_config::disable_autosave, preferences::autosavemax(), preferences::INFINITE_AUTO_SAVES);
	}

	if(preferences::turn_bell()) {
		sound::play_bell(game_config::sounds::turn_bell);
	}
}

void playsingle_controller::show_turn_dialog(){
	if(preferences::turn_dialog() && !is_regular_game_end() ) {
		blindfold b(*gui_, true); //apply a blindfold for the duration of this dialog
		gui_->redraw_everything();
		gui_->recalculate_minimap();
		std::string message = _("It is now $name|’s turn");
		utils::string_map symbols;
		symbols["name"] = gamestate().board_.teams()[player_number_ - 1].current_player();
		message = utils::interpolate_variables_into_string(message, &symbols);
		gui2::show_transient_message(gui_->video(), "", message);
	}
}

void playsingle_controller::execute_gotos()
{
	if(should_return_to_play_side())
	{
		return;
	}
	try
	{
		menu_handler_.execute_gotos(mouse_handler_, player_number_);
	}
	catch (const return_to_play_side_exception&)
	{
	}
}

void playsingle_controller::play_human_turn() {
	show_turn_dialog();

	if (!preferences::disable_auto_moves()) {
		execute_gotos();
	}

	end_turn_enable(true);
	while(!should_return_to_play_side()) {
		check_objectives();
		play_slice_catch();
		gui_->draw();
	}

}

void playsingle_controller::linger()
{
	LOG_NG << "beginning end-of-scenario linger\n";
	linger_ = true;

	// If we need to set the status depending on the completion state
	// the key to it is here.
	gui_->set_game_mode(game_display::LINGER_SP);

	// change the end-turn button text to its alternate label
	gui_->get_theme().refresh_title2("button-endturn", "title2");
	gui_->invalidate_theme();
	gui_->redraw_everything();

	// End all unit moves
	gamestate().board_.set_all_units_user_end_turn();
	try {
		// Same logic as single-player human turn, but
		// *not* the same as multiplayer human turn.
		end_turn_enable(true);
		end_turn_ = END_TURN_NONE;
		while(end_turn_ == END_TURN_NONE) {
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
	// Clear moves from the GUI.
	gui_->set_route(NULL);
	gui_->unhighlight_reach();
}

void playsingle_controller::play_ai_turn()
{
	LOG_NG << "is ai...\n";

	end_turn_enable(false);
	gui_->recalculate_minimap();

	const cursor::setter cursor_setter(cursor::WAIT);

	// Correct an oddball case where a human could have left delayed shroud
	// updates on before giving control to the AI. (The AI does not bother
	// with the undo stack, so it cannot delay shroud updates.)
	team & cur_team = current_team();
	if ( !cur_team.auto_shroud_updates() ) {
		// We just took control, so the undo stack is empty. We still need
		// to record this change for the replay though.
		synced_context::run_and_store("auto_shroud", replay_helper::get_auto_shroud(true));
	}
	undo_stack_->clear();

	turn_data_.send_data();
	try {
		try {
			ai::manager::play_turn(player_number_);
		}
		catch (return_to_play_side_exception&) {
		}
		catch (fallback_ai_to_human_exception&) {
			current_team().make_human();
			player_type_changed_ = true;
		}
	}
	catch(...) {
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
	gui_->draw();
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
void playsingle_controller::play_network_turn()
{
	// There should be no networked sides in single-player.
	ERR_NG << "Networked team encountered by playsingle_controller." << std::endl;
}


void playsingle_controller::handle_generic_event(const std::string& name){
	if (name == "ai_user_interact"){
		play_slice(false);
	}
}



void playsingle_controller::end_turn(){
	if (linger_)
		end_turn_ = END_TURN_REQUIRED;
	else if (!is_browsing() && menu_handler_.end_turn(player_number_)){
		end_turn_ = END_TURN_REQUIRED;
	}
}

void playsingle_controller::force_end_turn(){
	skip_next_turn_ = true;
	end_turn_ = END_TURN_REQUIRED;
}

void playsingle_controller::check_objectives()
{
	const team &t = gamestate().board_.teams()[gui_->viewing_team()];

	if (!is_regular_game_end() && !is_browsing() && t.objectives_changed()) {
		dialogs::show_objectives(get_scenario_name().str(), t.objectives());
		t.reset_objectives_changed();
	}
}


bool playsingle_controller::is_host() const
{
	return turn_data_.is_host();
}

void playsingle_controller::maybe_linger()
{
	// mouse_handler expects at least one team for linger mode to work.
	assert(is_regular_game_end());
	if (get_end_level_data_const().transient.linger_mode && !gamestate().board_.teams().empty()) {
		linger();
	}
}

void playsingle_controller::sync_end_turn()
{
	//We cannot add [end_turn] to the recorder while executing another action.
	assert(synced_context::synced_state() == synced_context::UNSYNCED);
	if(end_turn_ == END_TURN_REQUIRED && current_team().is_local())
	{
		//TODO: we shodul also send this immideateley.
		resources::recorder->end_turn();
		end_turn_ = END_TURN_SYNCED;
	}

	assert(end_turn_ == END_TURN_SYNCED);
	skip_next_turn_ = false;
}
