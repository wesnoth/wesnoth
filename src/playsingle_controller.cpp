/* $Id$ */

/*
   Copyright (C) 2006 - 2011 by Joerg Hinrichs <joerg.hinrichs@alice-dsl.de>
   wesnoth playlevel Copyright (C) 2003 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file playsingle_controller.cpp
 *  Logic for single-player game.
 */

#include "playsingle_controller.hpp"

#include "actions.hpp"
#include "ai/manager.hpp"
#include "ai/game_info.hpp"
#include "ai/testing.hpp"
#include "dialogs.hpp"
#include "foreach.hpp"
#include "game_end_exceptions.hpp"
#include "game_events.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "log.hpp"
#include "map_label.hpp"
#include "marked-up_text.hpp"
#include "playturn.hpp"
#include "resources.hpp"
#include "savegame.hpp"
#include "sound.hpp"
#include "formula_string_utils.hpp"
#include "events.hpp"
#include "save_blocker.hpp"
#include "soundsource.hpp"
#include "storyscreen/interface.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)

playsingle_controller::playsingle_controller(const config& level,
		game_state& state_of_game, const int ticks, const int num_turns,
		const config& game_config, CVideo& video, bool skip_replay) :
	play_controller(level, state_of_game, ticks, num_turns, game_config, video, skip_replay),
	cursor_setter(cursor::NORMAL),
	data_backlog_(),
	textbox_info_(),
	replay_sender_(recorder),
	end_turn_(false),
	player_type_changed_(false),
	replaying_(false),
	turn_over_(false),
	skip_next_turn_(false),
	level_result_(NONE)
{
	// game may need to start in linger mode
	if (state_of_game.classification().completion == "victory" || state_of_game.classification().completion == "defeat")
	{
		LOG_NG << "Setting linger mode.\n";
		browse_ = linger_ = true;
	}

	ai::game_info ai_info(*gui_,map_,units_,teams_, tod_manager_, gamestate_);
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

	if(first_human_team_ != -1) {
		gui_->scroll_to_tile(map_.starting_position(first_human_team_ + 1), game_display::WARP);
	}
	gui_->scroll_to_tile(map_.starting_position(1), game_display::WARP);

	update_locker lock_display(gui_->video(),recorder.is_skipping());
	events::raise_draw_event();
	gui_->draw();
	for(std::vector<team>::iterator t = teams_.begin(); t != teams_.end(); ++t) {
		::clear_shroud(t - teams_.begin() + 1);
	}
}

void playsingle_controller::recruit(){
	if (!browse_)
		menu_handler_.recruit(browse_, player_number_, mouse_handler_.get_last_hex());
}

void playsingle_controller::repeat_recruit(){
	if (!browse_)
		menu_handler_.repeat_recruit(player_number_, mouse_handler_.get_last_hex());
}

void playsingle_controller::recall(){
	if (!browse_)
		menu_handler_.recall(player_number_, mouse_handler_.get_last_hex());
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
	if ((level_result_ == NONE) || linger_)
	{
		team &t = teams_[gui_->viewing_team()];
		if (!browse_ && t.objectives_changed()) {
			dialogs::show_objectives(level_, t.objectives());
			t.reset_objectives_changed();
		}
		return;
	}
	throw end_level_exception(level_result_);
}

void playsingle_controller::rename_unit(){
	menu_handler_.rename_unit(mouse_handler_);
}

void playsingle_controller::create_unit(){
	menu_handler_.create_unit(mouse_handler_);
}

void playsingle_controller::change_side(){
	menu_handler_.change_side(mouse_handler_);
}

void playsingle_controller::label_terrain(bool team_only){
	menu_handler_.label_terrain(mouse_handler_, team_only);
}

void playsingle_controller::continue_move(){
	menu_handler_.continue_move(mouse_handler_, player_number_);
}

void playsingle_controller::add_waypoint(){
	if (!browse_)
		mouse_handler_.add_waypoint(mouse_handler_.get_last_hex());
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
	menu_handler_.custom_command(mouse_handler_, player_number_);
}

void playsingle_controller::ai_formula(){
	menu_handler_.ai_formula();
}

void playsingle_controller::clear_messages(){
	menu_handler_.clear_messages();
}

#ifdef USRCMD2
void playsingle_controller::user_command_2(){
	menu_handler_.user_command_2();
}

void playsingle_controller::user_command_3(){
	menu_handler_.user_command_3();
}
#endif

void playsingle_controller::report_victory(
	std::ostringstream &report, int player_gold, int remaining_gold,
	int finishing_bonus_per_turn, int turns_left, int finishing_bonus)
{
	end_level_data &end_level = get_end_level_data();
	report << _("Remaining gold: ")
		   << remaining_gold << "\n";
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
		       << (remaining_gold + finishing_bonus);
	}
	if (remaining_gold > 0) {
		report << '\n' << _("Carry over percentage: ") << end_level.carryover_percentage;
	}
	if(end_level.carryover_add) {
		report << "\n<b>" << _("Bonus Gold: ") << player_gold <<"</b>";
	} else {
		report << "\n<b>" << _("Retained Gold: ") << player_gold << "</b>";
	}

	std::string goldmsg;
	utils::string_map symbols;

	symbols["gold"] = lexical_cast_default<std::string>(player_gold);

	// Note that both strings are the same in english, but some languages will
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

LEVEL_RESULT playsingle_controller::play_scenario(
	const config::const_child_itors &story,
	bool skip_replay)
{
	LOG_NG << "in playsingle_controller::play_scenario()...\n";

	// Start music.
	BOOST_FOREACH (const config &m, level_.child_range("music")) {
		sound::play_music_config(m);
	}
	sound::commit_music_changes();

	if(!skip_replay) {
		BOOST_FOREACH (const config &s, story) {
			show_storyscreen(*gui_, vconfig(s, true), level_["name"]);
		}
	}
	gui_->labels().read(level_);

	// Find a list of 'items' (i.e. overlays) on the level, and add them
	BOOST_FOREACH (const config &overlay, level_.child_range("item"))
	{
		gui_->add_overlay(
			map_location(overlay, resources::state_of_game),
			overlay["image"], overlay["halo"], overlay["team_name"],
			utils::string_bool(overlay["visible_in_fog"], true));
	}

	// Read sound sources
	assert(soundsources_manager_ != NULL);
	BOOST_FOREACH (const config &s, level_.child_range("sound_source")) {
		soundsource::sourcespec spec(s);
		soundsources_manager_->add(spec);
	}

	set_victory_when_enemies_defeated(
		utils::string_bool(level_["victory_when_enemies_defeated"], true));
	end_level_data &end_level = get_end_level_data();
	end_level.carryover_percentage = lexical_cast_default<int>(
		level_["carryover_percentage"], game_config::gold_carryover_percentage);
	end_level.carryover_add = utils::string_bool(level_["carryover_add"]);

	LOG_NG << "entering try... " << (SDL_GetTicks() - ticks_) << "\n";
	try {
		fire_prestart(!loading_game_);
		init_gui();

		LOG_NG << "first_time..." << (recorder.is_skipping() ? "skipping" : "no skip") << "\n";

		fire_start(!loading_game_);
		gui_->recalculate_minimap();

		replaying_ = (recorder.at_end() == false);

		LOG_NG << "starting main loop\n" << (SDL_GetTicks() - ticks_) << "\n";

		// Initialize countdown clock.
		std::vector<team>::iterator t;
		for(t = teams_.begin(); t != teams_.end(); ++t) {
			if (gamestate_.mp_settings().mp_countdown && !loading_game_ ){
				t->set_countdown_time(1000 * gamestate_.mp_settings().mp_countdown_init_time);
			}
		}

		// if we loaded a save file in linger mode, skip to it.
		if (linger_) {
			//determine the bonus gold handling for this scenario
			const config end_cfg = level_.child_or_empty("endlevel");
			end_level.carryover_percentage = lexical_cast_default<int>(
				level_["carryover_percentage"],
				game_config::gold_carryover_percentage);
			end_level.carryover_add = utils::string_bool(level_["carryover_add"]);
			end_level.gold_bonus = utils::string_bool(end_cfg["bonus"], true);
			end_level.carryover_report = false;
			throw end_level_exception(SKIP_TO_LINGER);
		}

		// Avoid autosaving after loading, but still
		// allow the first turn to have an autosave.
		bool save = !loading_game_;
		ai_testing::log_game_start();
		for(; ; first_player_ = 1) {
			play_turn(save);
			save = true;
		} //end for loop

	} catch(game::load_game_exception& lge) {
		// Loading a new game is effectively a quit.
		//
		if (lge.game != "") {
			gamestate_ = game_state();
		}

		throw lge;
	} catch (end_level_exception &end_level_exn) {
		ai_testing::log_game_end();
		LEVEL_RESULT end_level_result = end_level_exn.result;
		if (!end_level.custom_endlevel_music.empty()) {
			if (end_level_result == DEFEAT) {
				set_defeat_music_list(end_level.custom_endlevel_music);
			} else {
				set_victory_music_list(end_level.custom_endlevel_music);
			}
		}

		if (teams_.empty())
		{
			//store persistent teams
			gamestate_.snapshot = config();
			store_recalls();

			return VICTORY; // this is probably only a story scenario, i.e. has its endlevel in the prestart event
		}
		const bool obs = is_observer();
		if (game_config::exit_at_end) {
			exit(0);
		}
		if (end_level_result == DEFEAT || end_level_result == VICTORY)
		{
			gamestate_.classification().completion = (end_level_exn.result == VICTORY) ? "victory" : "defeat";
			// If we're a player, and the result is victory/defeat, then send
			// a message to notify the server of the reason for the game ending.
			if (!obs) {
				config cfg;
				config& info = cfg.add_child("info");
				info["type"] = "termination";
				info["condition"] = "game over";
				info["result"] = gamestate_.classification().completion;
				network::send_data(cfg, 0, true);
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
			gamestate_.classification().completion = "defeat";

			game_events::fire("defeat");

			if (!obs) {
				const std::string& defeat_music = select_defeat_music();
				if(defeat_music.empty() != true)
					sound::play_music_once(defeat_music);

				return DEFEAT;
			} else {
				return QUIT;
			}
		}
		else if (end_level_result == VICTORY)
		{
			gamestate_.classification().completion =
				!end_level.linger_mode ? "running" : "victory";
			game_events::fire("victory");

			//
			// Play victory music once all victory events
			// are finished, if we aren't observers.
			//
			// Some scenario authors may use 'continue'
			// result for something that is not story-wise
			// a victory, so let them use [music] tags
			// instead should they want special music.
			//
			if (!obs && end_level.linger_mode) {
				const std::string& victory_music = select_victory_music();
				if(victory_music.empty() != true)
					sound::play_music_once(victory_music);
			}

			// Add all the units that survived the scenario.
			LOG_NG << "Add units that survived the scenario to the recall list.\n";
			for(unit_map::iterator un = units_.begin(); un != units_.end(); ++un) {

				if(teams_[un->second.side()-1].persistent()) {
					LOG_NG << "Added unit " << un->second.id() << ", " << un->second.name() << "\n";
					un->second.new_turn();
					un->second.new_scenario();
					teams_[un->second.side()-1].recall_list().push_back(un->second);
				}
			}
			//store all units that survived (recall list for the next scenario) in snapshot
			gamestate_.snapshot = config();
			store_recalls();
			//store gold and report victory
			store_gold(obs);
			return VICTORY;
		}
		else if (end_level_result == SKIP_TO_LINGER)
		{
			LOG_NG << "resuming from loaded linger state...\n";
			//as carryover information is stored in the snapshot, we have to re-store it after loading a linger state
			gamestate_.snapshot = config();
			store_recalls();
			store_gold();
			return VICTORY;
		}
	} // end catch
	catch(network::error& e) {
		bool disconnect = false;
		if(e.socket) {
			e.disconnect();
			disconnect = true;
		}

		savegame::game_savegame save(gamestate_, *gui_, to_config(), preferences::compress_saves());
		/** @todo FIXME: remove the odd linebreak after the string freeze */
		save.save_game_interactive(gui_->video(), _("A network disconnection has occurred, and the game\ncannot continue. Do you want to save the game?"), gui::YES_NO);
		if(disconnect) {
			throw network::error();
		} else {
			return QUIT;
		}
	}

	return QUIT;
}

void playsingle_controller::play_turn(bool save)
{
	gui_->new_turn();
	gui_->invalidate_game_status();
	events::raise_draw_event();

	LOG_NG << "turn: " << turn() << "\n";

	if(non_interactive())
		std::cout << "Turn " << turn() << ":" << std::endl;


	for (player_number_ = first_player_; player_number_ <= int(teams_.size()); ++player_number_)
	{
		// If a side is empty skip over it.
		if (current_team().is_empty()) continue;
		try {
			save_blocker blocker;
			init_side(player_number_ - 1);
		} catch (end_turn_exception) {
			if (current_team().is_network() == false) {
				turn_info turn_data(player_number_, replay_sender_, undo_stack_);
				recorder.end_turn();
				turn_data.sync_network();
			}
			continue;
		}

		if (replaying_) {
			LOG_NG << "doing replay " << player_number_ << "\n";
			replaying_ = ::do_replay(player_number_);
			LOG_NG << "result of replay: " << (replaying_?"true":"false") << "\n";
		} else {
			// If a side is dead end the turn.
			if (current_team().is_human() && side_units(units_, player_number_) == 0)
			{
				turn_info turn_data(player_number_, replay_sender_, undo_stack_);
				recorder.end_turn();
				turn_data.sync_network();
				continue;
			}
			ai_testing::log_turn_start(player_number_);
			play_side(player_number_, save);
		}

		finish_side_turn();

		if(non_interactive()) {
			std::cout << " Player " << player_number_ << ": " <<
				current_team().villages().size() << " Villages" <<
				std::endl;
			ai_testing::log_turn_end(player_number_);
		}

		check_victory();

		//if loading a savegame, network turns might not have reset this yet
		loading_game_ = false;
	}

	// Time has run out
	check_time_over();

	finish_turn();
}

void playsingle_controller::play_side(const unsigned int team_index, bool save)
{
	//check for team-specific items in the scenario
	gui_->parse_team_overlays();

	//flag used when we fallback from ai and give temporarily control to human
	bool temporary_human = false;
	do {
		// Although this flag is used only in this method,
		// it has to be a class member since derived classes
		// rely on it
		player_type_changed_ = false;
		if (!skip_next_turn_)
			end_turn_ = false;


		statistics::reset_turn_stats(teams_[team_index - 1].save_id());

		if(current_team().is_human() || temporary_human) {
			LOG_NG << "is human...\n";
			try{
				before_human_turn(save);
				play_human_turn();
				after_human_turn();
			} catch(end_turn_exception& end_turn) {
				if (end_turn.redo == team_index) {
					player_type_changed_ = true;
					// If new controller is not human,
					// reset gui to prev human one
					if (!teams_[team_index-1].is_human()) {
						browse_ = true;
						int t = find_human_team_before(team_index);
						if (t > 0) {
							gui_->set_team(t-1);
							gui_->recalculate_minimap();
							gui_->invalidate_all();
							gui_->draw(true,true);
						}
					}
				}
			}

			if(game_config::debug)
				game_display::clear_debug_highlights();

			LOG_NG << "human finished turn...\n";
		} else if(current_team().is_ai()) {
			try {
				play_ai_turn();
			} catch(fallback_ai_to_human_exception&) {
				//give control to human for the rest of this turn
				player_type_changed_ = true;
				temporary_human = true;
			}
		}
	} while (player_type_changed_);
	// Keep looping if the type of a team (human/ai/networked)
	// has changed mid-turn
	skip_next_turn_ = false;
}

void playsingle_controller::before_human_turn(bool save)
{
	log_scope("player turn");
	browse_ = false;
	linger_ = false;

	gui_->set_team(player_number_ - 1);
	gui_->recalculate_minimap();
	gui_->invalidate_all();
	gui_->draw(true,true);

	ai::manager::raise_turn_started();

	if (save) {
		savegame::autosave_savegame save(gamestate_, *gui_, to_config(), preferences::compress_saves());
		save.autosave(game_config::disable_autosave, preferences::autosavemax(), preferences::INFINITE_AUTO_SAVES);
	}

	if(preferences::turn_bell()) {
		sound::play_bell(game_config::sounds::turn_bell);
	}
}

void playsingle_controller::show_turn_dialog(){
	if(preferences::turn_dialog() && (level_result_ == NONE) ) {
		std::string message = _("It is now $name|'s turn");
		utils::string_map symbols;
		symbols["name"] = teams_[player_number_ - 1].current_player();
		message = utils::interpolate_variables_into_string(message, &symbols);
		gui2::show_transient_message(gui_->video(), "", message);
	}
}

void playsingle_controller::execute_gotos(){
	menu_handler_.execute_gotos(mouse_handler_, player_number_);
}

void playsingle_controller::play_human_turn() {
	show_turn_dialog();
	execute_gotos();

	gui_->enable_menu("endturn", true);
	while(!end_turn_) {
		play_slice();
		check_end_level();
		gui_->draw();
	}
}
struct set_completion
{
	set_completion(game_state& state, const std::string& completion) :
		state_(state), completion_(completion)
	{
	}
	~set_completion()
	{
		state_.classification().completion = completion_;
	}
	private:
	game_state& state_;
	const std::string completion_;
};

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
	set_completion setter(gamestate_,"running");

	// change the end-turn button text to its alternate label
	gui_->get_theme().refresh_title2("button-endturn", "title2");
	gui_->invalidate_theme();
	gui_->redraw_everything();

	// End all unit moves
	for (unit_map::iterator u = units_.begin(); u != units_.end(); ++u) {
		u->second.set_user_end_turn(true);
	}
	try {
		// Same logic as single-player human turn, but
		// *not* the same as multiplayer human turn.
		gui_->enable_menu("endturn", true);
		while(!end_turn_) {
			// Reset the team number to make sure we're the right team.
			player_number_ = first_player_;
			play_slice();
			gui_->draw();
		}
	} catch(game::load_game_exception& lge) {
		// Loading a new game is effectively a quit.
		if (lge.game != "") {
			gamestate_ = game_state();
		}

		throw lge;
	}

	// revert the end-turn button text to its normal label
	gui_->get_theme().refresh_title2("button-endturn", "title");
	gui_->invalidate_theme();
	gui_->redraw_everything();
	gui_->set_game_mode(game_display::RUNNING);

	LOG_NG << "ending end-of-scenario linger\n";
}

void playsingle_controller::end_turn_record()
{
	if (!turn_over_)
	{
		turn_over_ = true;
		recorder.end_turn();
	}
}
void playsingle_controller::end_turn_record_unlock()
{
	turn_over_ = false;
}

void playsingle_controller::after_human_turn(){
	browse_ = true;
	end_turn_record();
	end_turn_record_unlock();
	menu_handler_.clear_undo_stack(player_number_);

	if(teams_[player_number_-1].uses_fog()) {
		// needed because currently fog is only recalculated when a hex is /un/covered
		recalculate_fog(player_number_);
	}

	gui_->set_route(NULL);
	gui_->unhighlight_reach();
}

void playsingle_controller::play_ai_turn(){
	LOG_NG << "is ai...\n";
	gui_->enable_menu("endturn", false);
	browse_ = true;
	gui_->recalculate_minimap();

	const cursor::setter cursor_setter(cursor::WAIT);

	turn_info turn_data(player_number_, replay_sender_, undo_stack_);

	try {
		ai::manager::play_turn(player_number_);
	} catch (end_turn_exception&) {
	}
	recorder.end_turn();
	turn_data.sync_network();

	gui_->recalculate_minimap();
	::clear_shroud(player_number_);
	gui_->invalidate_unit();
	gui_->invalidate_game_status();
	gui_->invalidate_all();
	gui_->draw();
	gui_->delay(100);
}

void playsingle_controller::handle_generic_event(const std::string& name){
	if (name == "ai_user_interact"){
		play_slice(false);
	}
	if (end_turn_){
		throw end_turn_exception();
	}
}

void playsingle_controller::check_time_over(){
	bool b = next_turn();
	if(!b) {

		LOG_NG << "firing time over event...\n";
		game_events::fire("time over");
		LOG_NG << "done firing time over event...\n";
		//if turns are added while handling 'time over' event
		if (tod_manager_.is_time_left()) {
			return;
		}

		if(non_interactive()) {
			std::cout << "time over (draw)\n";
			ai_testing::log_draw();
		}

		check_end_level();
		throw end_level_exception(DEFEAT);
	}
}

void playsingle_controller::store_recalls() {
	std::set<std::string> side_ids;
	std::vector<team>::iterator i;
	for(i=teams_.begin(); i!=teams_.end(); ++i) {
		side_ids.insert(i->save_id());
		if (i->persistent()) {
			config& new_side = gamestate_.snapshot.add_child("side");
			new_side["save_id"] = i->save_id();
			new_side["name"] = i->current_player();
			std::stringstream can_recruit;
			std::copy(i->recruits().begin(),i->recruits().end(),std::ostream_iterator<std::string>(can_recruit,","));
			std::string can_recruit_str = can_recruit.str();
			// Remove the trailing comma
			if(can_recruit_str.empty() == false) {
				can_recruit_str.resize(can_recruit_str.size()-1);
			}
			new_side["previous_recruits"] = can_recruit_str;
			LOG_NG << "stored side in snapshot:\n" << new_side["save_id"] << std::endl;
			//add the units of the recall list
			BOOST_FOREACH(const unit& u, i->recall_list()) {
				config& new_unit = new_side.add_child("unit");
				u.write(new_unit);
			}
		}
	}
	//add any players from starting_pos that do not have a team in the current scenario
	BOOST_FOREACH (const config* player_cfg, gamestate_.starting_pos.get_children("player")) {
		if (side_ids.count((*player_cfg)["save_id"]) == 0) {
			LOG_NG << "stored inactive side in snapshot:\n" << (*player_cfg)["save_id"] << std::endl;
			gamestate_.snapshot.add_child("side", (*player_cfg));
		}
	}
}

void playsingle_controller::store_gold(bool obs)
{
	bool has_next_scenario = !gamestate_.classification().next_scenario.empty() &&
		gamestate_.classification().next_scenario != "null";

	std::ostringstream report;
	std::string title;

	if (obs) {
		title = _("Scenario Report");
	} else {
		title = _("Victory");
		report << "<b>" << _("You have emerged victorious!") << "</b>\n\n";
	}

	int persistent_teams = 0;
	BOOST_FOREACH (const team &t, teams_) {
		if (t.persistent()) ++persistent_teams;
	}

	end_level_data &end_level = get_end_level_data();

	if (persistent_teams > 0 && (has_next_scenario ||
		gamestate_.classification().campaign_type == "test"))
	{
		int finishing_bonus_per_turn =
			map_.villages().size() * game_config::village_income +
			game_config::base_income;
		int turns_left = std::max<int>(0, number_of_turns() - turn());
		int finishing_bonus = (end_level.gold_bonus && turns_left > -1) ?
			finishing_bonus_per_turn * turns_left : 0;
		BOOST_FOREACH (const team &t, teams_)
		{
			if (!t.persistent()) continue;
			int carryover_gold = ((t.gold() + finishing_bonus) * end_level.carryover_percentage) / 100;
			config::child_itors side_range = gamestate_.snapshot.child_range("side");
			config::child_iterator side_it = side_range.first;

			// Check if this side already exists in the snapshot.
			while (side_it != side_range.second) {
				if ((*side_it)["save_id"] == t.save_id()) {
					(*side_it)["gold"] = str_cast<int>(carryover_gold);
					(*side_it)["gold_add"] = end_level.carryover_add ? "yes" : "no";
					(*side_it)["colour"] = t.colour();
					(*side_it)["current_player"] = t.current_player();
					(*side_it)["name"] = t.name();
					break;
				}
				++side_it;
			}

			// If it doesn't, add a new child.
			if (side_it == side_range.second) {
				config &new_side = gamestate_.snapshot.add_child("side");
				new_side["save_id"] = t.save_id();
				new_side["gold"] = str_cast<int>(carryover_gold);
				new_side["gold_add"] = end_level.carryover_add ? "yes" : "no";
				new_side["colour"] = t.colour();
				new_side["current_player"] = t.current_player();
				new_side["name"] = t.name();
			}

			// Only show the report for ourselves.
			if (!t.is_human()) continue;

			if (persistent_teams > 1) {
				report << "\n<b>" << t.current_player() << "</b>\n";
			}

			report_victory(report, carryover_gold, t.gold(), finishing_bonus_per_turn, turns_left, finishing_bonus);
		}
	}

	if (end_level.carryover_report) {
		gui2::show_transient_message(gui_->video(), title, report.str(), true);
	}
}

bool playsingle_controller::can_execute_command(hotkey::HOTKEY_COMMAND command, int index) const
{
	bool res = true;
	switch (command){
		case hotkey::HOTKEY_ADD_WAYPOINT:
		case hotkey::HOTKEY_UNIT_HOLD_POSITION:
		case hotkey::HOTKEY_END_UNIT_TURN:
		case hotkey::HOTKEY_RECRUIT:
		case hotkey::HOTKEY_REPEAT_RECRUIT:
		case hotkey::HOTKEY_RECALL:
			return !browse_ && !linger_ && !events::commands_disabled;
		case hotkey::HOTKEY_ENDTURN:
			return (!browse_ || linger_) && !events::commands_disabled;

		case hotkey::HOTKEY_DELAY_SHROUD:
			return !linger_ && (teams_[gui_->viewing_team()].uses_fog() || teams_[gui_->viewing_team()].uses_shroud())
			&& !events::commands_disabled;
		case hotkey::HOTKEY_UPDATE_SHROUD:
			return !linger_
				&& player_number_ == gui_->viewing_side()
				&& !events::commands_disabled
				&& teams_[gui_->viewing_team()].auto_shroud_updates() == false;

		// Commands we can only do if in debug mode
		case hotkey::HOTKEY_CREATE_UNIT:
		case hotkey::HOTKEY_CHANGE_SIDE:
			return !events::commands_disabled && game_config::debug && map_.on_board(mouse_handler_.get_last_hex());

		case hotkey::HOTKEY_LABEL_TEAM_TERRAIN:
		case hotkey::HOTKEY_LABEL_TERRAIN:
			res = !events::commands_disabled && map_.on_board(mouse_handler_.get_last_hex())
				&& !gui_->shrouded(mouse_handler_.get_last_hex())
				&& !is_observer();
			break;

		case hotkey::HOTKEY_CONTINUE_MOVE: {
			if(browse_ || events::commands_disabled)
				return false;

			if( (menu_handler_.current_unit(mouse_handler_) != units_.end())
				&& (menu_handler_.current_unit(mouse_handler_)->second.move_interrupted()))
				return true;
			const unit_map::const_iterator i = units_.find(mouse_handler_.get_selected_hex());
			if (i == units_.end()) return false;
			return i->second.move_interrupted();
		}
		default: return play_controller::can_execute_command(command, index);
	}
	return res;
}
