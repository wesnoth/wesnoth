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
 *  Handle input via mouse & keyboard, events, schedule commands.
 */

#include "play_controller.hpp"

#include "actions/create.hpp"
#include "actions/heal.hpp"
#include "actions/undo.hpp"
#include "actions/vision.hpp"
#include "ai/manager.hpp"
#include "ai/testing.hpp"
#include "dialogs.hpp"
#include "display_chat_manager.hpp"
#include "formula_string_utils.hpp"
#include "game_events/manager.hpp"
#include "game_events/menu_item.hpp"
#include "game_events/pump.hpp"
#include "game_preferences.hpp"
#include "hotkey/hotkey_item.hpp"
#include "hotkey_handler.hpp"
#include "map_label.hpp"
#include "gettext.hpp"
#include "halo.hpp"
#include "hotkey/command_executor.hpp"
#include "loadscreen.hpp"
#include "log.hpp"
#include "pathfind/teleport.hpp"
#include "preferences_display.hpp"
#include "replay.hpp"
#include "reports.hpp"
#include "resources.hpp"
#include "savegame.hpp"
#include "saved_game.hpp"
#include "save_blocker.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "scripting/plugins/context.hpp"
#include "sound.hpp"
#include "soundsource.hpp"
#include "statistics.hpp"
#include "synced_context.hpp"
#include "terrain_type_data.hpp"
#include "tooltips.hpp"
#include "unit.hpp"
#include "unit_id.hpp"
#include "unit_types.hpp"
#include "whiteboard/manager.hpp"
#include "wml_exception.hpp"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

static lg::log_domain log_aitesting("aitesting");
#define LOG_AIT LOG_STREAM(info, log_aitesting)
//If necessary, this define can be replaced with `#define LOG_AIT std::cout` to restore previous behavior

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)

static lg::log_domain log_enginerefac("enginerefac");
#define LOG_RG LOG_STREAM(info, log_enginerefac)

static lg::log_domain log_engine_enemies("engine/enemies");
#define DBG_EE LOG_STREAM(debug, log_engine_enemies)

static void clear_resources()
{
	resources::controller = NULL;
	resources::filter_con = NULL;
	resources::gameboard = NULL;
	resources::gamedata = NULL;
	resources::game_events = NULL;
	resources::lua_kernel = NULL;
	resources::persist = NULL;
	resources::screen = NULL;
	resources::soundsources = NULL;
	resources::teams = NULL;
	resources::tod_manager = NULL;
	resources::tunnels = NULL;
	resources::undo_stack = NULL;
	resources::units = NULL;
	resources::whiteboard.reset();


	resources::classification = NULL;
	resources::mp_settings = NULL;

}

play_controller::play_controller(const config& level, saved_game& state_of_game,
		const int ticks, const config& game_config, const tdata_cache & tdata,
		CVideo& video, bool skip_replay)
	: controller_base(game_config, video)
	, observer()
	, savegame_config()
	, gamestate_(level, tdata)
	, level_(level)
	, saved_game_(state_of_game)
	, prefs_disp_manager_()
	, tooltips_manager_()
	, whiteboard_manager_()
	, plugins_context_()
	, labels_manager_()
	, help_manager_(&game_config)
	, mouse_handler_(NULL, *this)
	, menu_handler_(NULL, *this, game_config)
	, hotkey_handler_(new hotkey_handler(*this, saved_game_))
	, soundsources_manager_()
	, persist_()
	, gui_()
	, xp_mod_(new unit_experience_accelerator(level["experience_modifier"].to_int(100)))
	, statistics_context_(new statistics::scenario_context(level["name"]))
	, undo_stack_(new actions::undo_list(level.child("undo_stack")))
	, loading_game_(level["playing_team"].empty() == false)
	, player_number_(1)
	, first_player_(level["playing_team"].to_int() + 1)
	, start_turn_(gamestate_.tod_manager_.turn()) // gamestate_.tod_manager_ constructed above
	, skip_replay_(skip_replay)
	, linger_(false)
	, it_is_a_new_turn_(true)
	, init_side_done_(level["init_side_done"].to_bool(true))
	, ticks_(ticks)
	, victory_when_enemies_defeated_(true)
	, remove_from_carryover_on_defeat_(true)
	, end_level_data_()
	, victory_music_()
	, defeat_music_()
	, scope_()
	, server_request_number_(0)
{
	resources::controller = this;
	resources::gameboard = &gamestate_.board_;
	resources::gamedata = &gamestate_.gamedata_;
	resources::persist = &persist_;
	resources::teams = &gamestate_.board_.teams_;
	resources::tod_manager = &gamestate_.tod_manager_;
	resources::undo_stack = undo_stack_.get();
	resources::units = &gamestate_.board_.units_;
	resources::filter_con = &gamestate_;

	resources::classification = &saved_game_.classification();
	resources::mp_settings = &saved_game_.mp_settings();

	persist_.start_transaction();
	n_unit::id_manager::instance().set_save_id(level_["next_underlying_unit_id"]);

	// Setup victory and defeat music
	set_victory_music_list(level_["victory_music"]);
	set_defeat_music_list(level_["defeat_music"]);

	game_config::add_color_info(level);
	hotkey::deactivate_all_scopes();
	hotkey::set_scope_active(hotkey::SCOPE_GAME);
	try {
		init(video);
	} catch (...) {
		clear_resources();
		throw;
	}
}

play_controller::~play_controller()
{
	hotkey::delete_all_wml_hotkeys();
	clear_resources();
}

void play_controller::init(CVideo& video){
	util::scoped_resource<loadscreen::global_loadscreen_manager*, util::delete_item> scoped_loadscreen_manager;
	loadscreen::global_loadscreen_manager* loadscreen_manager = loadscreen::global_loadscreen_manager::get();
	if (!loadscreen_manager)
	{
		scoped_loadscreen_manager.assign(new loadscreen::global_loadscreen_manager(video));
		loadscreen_manager = scoped_loadscreen_manager.get();
	}

	loadscreen::start_stage("load level");
	recorder.set_skip(false);

	LOG_NG << "initializing game_state..." << (SDL_GetTicks() - ticks_) << std::endl;
	gamestate_.init(ticks_, *this);
	resources::tunnels = gamestate_.pathfind_manager_.get();

	LOG_NG << "initializing whiteboard..." << (SDL_GetTicks() - ticks_) << std::endl;
	whiteboard_manager_.reset(new wb::manager());
	resources::whiteboard = whiteboard_manager_;

	// mouse_handler expects at least one team for linger mode to work.
	if (gamestate_.board_.teams().empty()) end_level_data_.transient.linger_mode = false;

	LOG_NG << "loading units..." << (SDL_GetTicks() - ticks_) << std::endl;
	loadscreen::start_stage("load units");
	preferences::encounter_all_content(gamestate_.board_);

	LOG_NG << "initializing theme... " << (SDL_GetTicks() - ticks_) << std::endl;
	loadscreen::start_stage("init theme");
	const config &theme_cfg = controller_base::get_theme(game_config_, level_["theme"]);

	LOG_NG << "building terrain rules... " << (SDL_GetTicks() - ticks_) << std::endl;
	loadscreen::start_stage("build terrain");
	gui_.reset(new game_display(gamestate_.board_, video, whiteboard_manager_, *gamestate_.reports_, gamestate_.tod_manager_, theme_cfg, level_));
	if (!gui_->video().faked()) {
		if (saved_game_.mp_settings().mp_countdown)
			gui_->get_theme().modify_label("time-icon", _ ("time left for current turn"));
		else
			gui_->get_theme().modify_label("time-icon", _ ("current local time"));
	}

	loadscreen::start_stage("init display");
	mouse_handler_.set_gui(gui_.get());
	menu_handler_.set_gui(gui_.get());
	resources::screen = gui_.get();

	LOG_NG << "done initializing display... " << (SDL_GetTicks() - ticks_) << std::endl;

	LOG_NG << "building gamestate to gui and whiteboard... " << (SDL_GetTicks() - ticks_) << std::endl;
	//loadscreen::start_stage("build events manager & lua");
	// This *needs* to be created before the show_intro and show_map_scene
	// as that functions use the manager state_of_game
	// Has to be done before registering any events!
	gamestate_.bind(whiteboard_manager_.get(), gui_.get());
	resources::lua_kernel=gamestate_.lua_kernel_.get();
	resources::game_events=gamestate_.events_manager_.get();

	if(gamestate_.first_human_team_ != -1) {
		gui_->set_team(gamestate_.first_human_team_);
	}
	else if (is_observer())
	{
		// Find first team that is allowed to be observed.
		// If not set here observer would be without fog until
		// the first turn of observable side
		size_t i;
		for (i=0;i < gamestate_.board_.teams().size();++i)
		{
			if (!gamestate_.board_.teams()[i].get_disallow_observers())
			{
				gui_->set_team(i);
			}
		}
	}

	browse_ = true;

	init_managers();
	loadscreen::global_loadscreen->start_stage("start game");
	loadscreen_manager->reset();

	plugins_context_.reset(new plugins_context("Game"));
	plugins_context_->set_callback("save_game", boost::bind(&play_controller::save_game_auto, this, boost::bind(get_str, _1, "filename" )), true);
	plugins_context_->set_callback("save_replay", boost::bind(&play_controller::save_replay_auto, this, boost::bind(get_str, _1, "filename" )), true);
	plugins_context_->set_callback("quit", boost::bind(&play_controller::force_end_level, this, QUIT), false);
}

void play_controller::init_managers(){
	LOG_NG << "initializing managers... " << (SDL_GetTicks() - ticks_) << std::endl;
	prefs_disp_manager_.reset(new preferences::display_manager(gui_.get()));
	tooltips_manager_.reset(new tooltips::manager(gui_->video()));
	soundsources_manager_.reset(new soundsource::manager(*gui_));

	resources::soundsources = soundsources_manager_.get();
	LOG_NG << "done initializing managers... " << (SDL_GetTicks() - ticks_) << std::endl;
}

void play_controller::fire_preload()
{
	// Run initialization scripts, even if loading from a snapshot.
	gamestate_.gamedata_.set_phase(game_data::PRELOAD);
	gamestate_.lua_kernel_->initialize();
	gamestate_.gamedata_.get_variable("turn_number") = int(turn());
	pump().fire("preload");
}
void play_controller::fire_prestart()
{
	// pre-start events must be executed before any GUI operation,
	// as those may cause the display to be refreshed.
	update_locker lock_display(gui_->video());
	gamestate_.gamedata_.set_phase(game_data::PRESTART);
	pump().fire("prestart");
	check_end_level();
	// prestart event may modify start turn with WML, reflect any changes.
	start_turn_ = turn();
	gamestate_.gamedata_.get_variable("turn_number") = int(start_turn_);
}

void play_controller::fire_start(bool execute){
	if(execute) {
		gamestate_.gamedata_.set_phase(game_data::START);
		pump().fire("start");
		check_end_level();
		// start event may modify start turn with WML, reflect any changes.
		start_turn_ = turn();
		gamestate_.gamedata_.get_variable("turn_number") = int(start_turn_);

		// prestart and start events may modify the initial gold amount,
		// reflect any changes.
		BOOST_FOREACH(team& tm, gamestate_.board_.teams_)
		{
			tm.set_start_gold(tm.gold());
		}

	} else {
		it_is_a_new_turn_ = false;
	}
	if( saved_game_.classification().random_mode != "" && (network::nconnections() != 0))
	{
		std::string mes = _("MP game uses an alternative random mode, if you don't know what this message means, then most likeley someone is cheating or someone reloaded a corrupt game.");
		gui_->get_chat_manager().add_chat_message(
			time(NULL),
			"game_engine",
			0,
			mes,
			events::chat_handler::MESSAGE_PUBLIC,
			preferences::message_bell());
		replay::process_error(mes);
	}
	gamestate_.gamedata_.set_phase(game_data::PLAY);
}

void play_controller::init_gui(){
	gui_->begin_game();
	gui_->update_tod();

	if ( !loading_game_ ) {
		for ( int side = gamestate_.board_.teams().size(); side != 0; --side )
			actions::clear_shroud(side, false, false);
	}
}

possible_end_play_signal play_controller::init_side(bool is_replay){
	log_scope("player turn");
	bool only_visual = loading_game_ && init_side_done_;
	init_side_done_ = false;
	mouse_handler_.set_side(player_number_);

	// If we are observers we move to watch next team if it is allowed
	if (is_observer() && !current_team().get_disallow_observers()) {
		gui_->set_team(size_t(player_number_ - 1));
	}
	gui_->set_playing_team(size_t(player_number_ - 1));

	gamestate_.gamedata_.get_variable("side_number") = player_number_;
	gamestate_.gamedata_.last_selected = map_location::null_location();

	HANDLE_END_PLAY_SIGNAL( maybe_do_init_side(is_replay, only_visual) );

	loading_game_ = false;

	return boost::none;
}

/**
 * Called by turn_info::process_network_data() or init_side() to call do_init_side() if necessary.
 */
void play_controller::maybe_do_init_side(bool is_replay, bool only_visual) {
	/**
	 * We do side init only if not done yet for a local side when we are not replaying.
	 * For all other sides it is recorded in replay and replay handler has to handle
	 * calling do_init_side() functions.
	 **/
	if (is_replay || init_side_done_ || !current_team().is_local()) {
		return;
	}

	if(!only_visual) {
		recorder.init_side();
		set_scontext_synced sync;
		do_init_side(is_replay);
		sync.do_final_checkup();
	}
	else
	{
		do_init_side(is_replay, true);
	}
}

/**
 * Called by replay handler or init_side() to do actual work for turn change.
 */
void play_controller::do_init_side(bool is_replay, bool only_visual) {
	log_scope("player turn");
	//In case we might end up calling sync:network during the side turn events,
	//and we don't want do_init_side to be called when a player drops.
	init_side_done_ = true;

	const std::string turn_num = str_cast(turn());
	const std::string side_num = str_cast(player_number_);

	// If this is right after loading a game we don't need to fire events and such. It was already done before saving.
	if (!only_visual) {
		if(it_is_a_new_turn_)
		{
			pump().fire("turn " + turn_num);
			pump().fire("new turn");
			it_is_a_new_turn_ = false;
		}

		pump().fire("side turn");
		pump().fire("side " + side_num + " turn");
		pump().fire("side turn " + turn_num);
		pump().fire("side " + side_num + " turn " + turn_num);
	}

	if(current_team().is_local_human() && !is_replay) {
		update_gui_to_player(player_number_ - 1);
	}
	// We want to work out if units for this player should get healed,
	// and the player should get income now.
	// Healing/income happen if it's not the first turn of processing,
	// or if we are loading a game.
	if (!only_visual && turn() > 1) {
		gamestate_.board_.new_turn(player_number_);
		current_team().new_turn();

		// If the expense is less than the number of villages owned
		// times the village support capacity,
		// then we don't have to pay anything at all
		int expense = gamestate_.board_.side_upkeep(player_number_) -
			current_team().support();
		if(expense > 0) {
			current_team().spend_gold(expense);
		}

		calculate_healing(player_number_, !skip_replay_);
	}

	if (!only_visual) {
		// Prepare the undo stack.
		undo_stack_->new_side_turn(player_number_);

		pump().fire("turn refresh");
		pump().fire("side " + side_num + " turn refresh");
		pump().fire("turn " + turn_num + " refresh");
		pump().fire("side " + side_num + " turn " + turn_num + " refresh");

		// Make sure vision is accurate.
		actions::clear_shroud(player_number_, true);
	}

	const time_of_day &tod = gamestate_.tod_manager_.get_time_of_day();

	if (player_number_ == first_player_)
		sound::play_sound(tod.sounds, sound::SOUND_SOURCES);

	if (!recorder.is_skipping()){
		gui_->invalidate_all();
	}

	if (!recorder.is_skipping() && !skip_replay_ && current_team().get_scroll_to_leader()){
		gui_->scroll_to_leader(player_number_,game_display::ONSCREEN,false);
	}
	loading_game_ = false;

	check_victory();
	whiteboard_manager_->on_init_side();
}

//builds the snapshot config from its members and their configs respectively
config play_controller::to_config() const
{
	config cfg;

	cfg["init_side_done"] = init_side_done_;
	cfg.merge_attributes(level_);

	gamestate_.write(cfg);

	if(linger_) {
		end_level_data_.write(cfg.add_child("end_level_data"));
	}

	// Write terrain_graphics data in snapshot, too
	BOOST_FOREACH(const config &tg, level_.child_range("terrain_graphics")) {
		cfg.add_child("terrain_graphics", tg);
	}

	gui_->write(cfg.add_child("display"));

	// Preserve the undo stack so that fog/shroud clearing is kept accurate.
	undo_stack_->write(cfg.add_child("undo_stack"));

	//Write the soundsources.
	soundsources_manager_->write_sourcespecs(cfg);

	if(gui_.get() != NULL){
		cfg["playing_team"] = str_cast(gui_->playing_team());
		gui_->labels().write(cfg);
		sound::write_music_play_list(cfg);
	}

	cfg["next_underlying_unit_id"] = str_cast(n_unit::id_manager::instance().get_save_id());
	return cfg;
}

void play_controller::finish_side_turn(){

	whiteboard_manager_->on_finish_side_turn(player_number_);

	gamestate_.board_.end_turn(player_number_);

	// Clear shroud, in case units had been slowed for the turn.
	actions::clear_shroud(player_number_);

	const std::string turn_num = str_cast(turn());
	const std::string side_num = str_cast(player_number_);

	{ //Block for set_scontext_synced
		set_scontext_synced sync(1);
		pump().fire("side turn end");
		pump().fire("side "+ side_num + " turn end");
		pump().fire("side turn " + turn_num + " end");
		pump().fire("side " + side_num + " turn " + turn_num + " end");
		// This is where we refog, after all of a side's events are done.
		actions::recalculate_fog(player_number_);
		sync.do_final_checkup();
	}

	// This implements "delayed map sharing."
	// It is meant as an alternative to shared vision.
	if(current_team().copy_ally_shroud()) {
		gui_->recalculate_minimap();
		gui_->invalidate_all();
	}

	mouse_handler_.deselect_hex();
	n_unit::id_manager::instance().reset_fake();
}

void play_controller::finish_turn()
{
	set_scontext_synced sync(2);
	const std::string turn_num = str_cast(turn());
	pump().fire("turn end");
	pump().fire("turn " + turn_num + " end");
	sync.do_final_checkup();
}

bool play_controller::enemies_visible() const
{
	// If we aren't using fog/shroud, this is easy :)
	if(current_team().uses_fog() == false && current_team().uses_shroud() == false)
		return true;

	// See if any enemies are visible
	BOOST_FOREACH(const unit & u, gamestate_.board_.units()) {
		if (current_team().is_enemy(u.side()) && !gui_->fogged(u.get_location())) {
			return true;
		}
	}

	return false;
}


void play_controller::enter_textbox()
{
	if(menu_handler_.get_textbox().active() == false) {
		return;
	}

	const std::string str = menu_handler_.get_textbox().box()->text();
	const unsigned int team_num = player_number_;
	events::mouse_handler& mousehandler = mouse_handler_;

	switch(menu_handler_.get_textbox().mode()) {
	case gui::TEXTBOX_SEARCH:
		menu_handler_.do_search(str);
		menu_handler_.get_textbox().close(*gui_);
		break;
	case gui::TEXTBOX_MESSAGE:
		menu_handler_.do_speak();
		menu_handler_.get_textbox().close(*gui_);  //need to close that one after executing do_speak() !
		break;
	case gui::TEXTBOX_COMMAND:
		menu_handler_.get_textbox().close(*gui_);
		menu_handler_.do_command(str);
		break;
	case gui::TEXTBOX_AI:
		menu_handler_.get_textbox().close(*gui_);
		menu_handler_.do_ai_formula(str, team_num, mousehandler);
		break;
	default:
		menu_handler_.get_textbox().close(*gui_);
		ERR_DP << "unknown textbox mode" << std::endl;
	}

}

void play_controller::tab()
{
	gui::TEXTBOX_MODE mode = menu_handler_.get_textbox().mode();

	std::set<std::string> dictionary;
	switch(mode) {
	case gui::TEXTBOX_SEARCH:
	{
		BOOST_FOREACH(const unit &u, gamestate_.board_.units()){
			const map_location& loc = u.get_location();
			if(!gui_->fogged(loc) &&
					!(gamestate_.board_.teams()[gui_->viewing_team()].is_enemy(u.side()) && u.invisible(loc)))
				dictionary.insert(u.name());
		}
		//TODO List map labels
		break;
	}
	case gui::TEXTBOX_COMMAND:
	{
		std::vector<std::string> commands = menu_handler_.get_commands_list();
		dictionary.insert(commands.begin(), commands.end());
		// no break here, we also want player names from the next case
	}
	case gui::TEXTBOX_MESSAGE:
	{
		BOOST_FOREACH(const team& t, gamestate_.board_.teams()) {
			if(!t.is_empty())
				dictionary.insert(t.current_player());
		}

		// Add observers
		BOOST_FOREACH(const std::string& o, gui_->observers()){
			dictionary.insert(o);
		}
		//Exclude own nick from tab-completion.
		//NOTE why ?
		dictionary.erase(preferences::login());
		break;
	}

	default:
		ERR_DP << "unknown textbox mode" << std::endl;
	} //switch(mode)

	menu_handler_.get_textbox().tab(dictionary);
}

team& play_controller::current_team()
{
	assert(player_number_ > 0 && player_number_ <= int(gamestate_.board_.teams().size()));
	return gamestate_.board_.teams_[player_number_-1];
}

const team& play_controller::current_team() const
{
	assert(player_number_ > 0 && player_number_ <= int(gamestate_.board_.teams().size()));
	return gamestate_.board_.teams()[player_number_-1];
}

int play_controller::find_human_team_before_current_player() const
{
	if (player_number_ > int(gamestate_.board_.teams().size()))
		return -2;

	for (int i = player_number_-2; i >= 0; --i) {
		if (gamestate_.board_.teams()[i].is_local_human()) {
			return i+1;
		}
	}

	for (int i = gamestate_.board_.teams().size()-1; i > player_number_-1; --i) {
		if (gamestate_.board_.teams()[i].is_local_human()) {
			return i+1;
		}
	}

	return -1;
}

events::mouse_handler& play_controller::get_mouse_handler_base() {
	return mouse_handler_;
}

boost::shared_ptr<wb::manager> play_controller::get_whiteboard() {
	return whiteboard_manager_;
}

const mp_game_settings & play_controller::get_mp_settings() {
	return saved_game_.mp_settings();
}

const game_classification & play_controller::get_classification() {
	return saved_game_.classification();
}

game_display& play_controller::get_display() {
	return *gui_;
}

bool play_controller::have_keyboard_focus()
{
	return !menu_handler_.get_textbox().active();
}

void play_controller::process_focus_keydown_event(const SDL_Event& event)
{
	if(event.key.keysym.sym == SDLK_ESCAPE) {
		menu_handler_.get_textbox().close(*gui_);
	} else if(event.key.keysym.sym == SDLK_TAB) {
		tab();
	} else if(event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
		enter_textbox();
	}
}

void play_controller::process_keydown_event(const SDL_Event& event) {
	if (event.key.keysym.sym == SDLK_TAB) {
		whiteboard_manager_->set_invert_behavior(true);
	}
}

void play_controller::process_keyup_event(const SDL_Event& event) {
	// If the user has pressed 1 through 9, we want to show
	// how far the unit can move in that many turns
	if(event.key.keysym.sym >= '1' && event.key.keysym.sym <= '7') {
		const int new_path_turns = (event.type == SDL_KEYDOWN) ?
		                           event.key.keysym.sym - '1' : 0;

		if(new_path_turns != mouse_handler_.get_path_turns()) {
			mouse_handler_.set_path_turns(new_path_turns);

			const unit_map::iterator u = mouse_handler_.selected_unit();

			if(u.valid()) {
				// if it's not the unit's turn, we reset its moves
				unit_movement_resetter move_reset(*u, u->side() != player_number_);

				mouse_handler_.set_current_paths(pathfind::paths(*u, false,
				                       true, gamestate_.board_.teams_[gui_->viewing_team()],
				                       mouse_handler_.get_path_turns()));

				gui_->highlight_reach(mouse_handler_.current_paths());
			} else {
				mouse_handler_.select_hex(mouse_handler_.get_selected_hex(), false, false, false);
			}

		}
	} else if (event.key.keysym.sym == SDLK_TAB) {
		static CKey keys;
		if (!keys[SDLK_TAB]) {
			whiteboard_manager_->set_invert_behavior(false);
		}
	}
}

void play_controller::save_game(){
	if(save_blocker::try_block()) {
		save_blocker::save_unblocker unblocker;
		update_savegame_snapshot();
		savegame::ingame_savegame save(saved_game_, *gui_, preferences::save_compression_format());
		save.save_game_interactive(gui_->video(), "", gui::OK_CANCEL);
	} else {
		save_blocker::on_unblock(this,&play_controller::save_game);
	}
}

void play_controller::save_game_auto(const std::string & filename) {
	if(save_blocker::try_block()) {
		save_blocker::save_unblocker unblocker;

		update_savegame_snapshot();
		savegame::ingame_savegame save(saved_game_, *gui_, preferences::save_compression_format());
		save.save_game_automatic(gui_->video(), false, filename);
	}
}

void play_controller::save_replay(){
	if(save_blocker::try_block()) {
		save_blocker::save_unblocker unblocker;
		savegame::replay_savegame save(saved_game_, preferences::save_compression_format());
		save.save_game_interactive(gui_->video(), "", gui::OK_CANCEL);
	} else {
		save_blocker::on_unblock(this,&play_controller::save_replay);
	}
}

void play_controller::save_replay_auto(const std::string & filename) {
	if(save_blocker::try_block()) {
		save_blocker::save_unblocker unblocker;
		savegame::replay_savegame save(saved_game_, preferences::save_compression_format());
		save.save_game_automatic(gui_->video(), false, filename);
	}
}

void play_controller::save_map(){
	if(save_blocker::try_block()) {
		save_blocker::save_unblocker unblocker;
		menu_handler_.save_map();
	} else {
		save_blocker::on_unblock(this,&play_controller::save_map);
	}
}

void play_controller::load_game() {
	savegame::loadgame load(*gui_, game_config_, saved_game_);
	load.load_game();
}

void play_controller::undo(){
	mouse_handler_.deselect_hex();
	undo_stack_->undo();
}

void play_controller::redo(){
	mouse_handler_.deselect_hex();
	undo_stack_->redo();
}

bool play_controller::can_undo() const {
	return !linger_ && !browse_ && !events::commands_disabled && undo_stack_->can_undo();
}

bool play_controller::can_redo() const {
	return !linger_ && !browse_ && !events::commands_disabled && undo_stack_->can_redo();
}

namespace {
	static const std::string empty_str = "";
}

const std::string& play_controller::select_victory_music() const
{
	if(victory_music_.empty())
		return empty_str;
	return victory_music_[rand() % victory_music_.size()];
}

const std::string& play_controller::select_defeat_music() const
{
	if(defeat_music_.empty())
		return empty_str;
	return defeat_music_[rand() % defeat_music_.size()];
}


void play_controller::set_victory_music_list(const std::string& list)
{
	victory_music_ = utils::split(list);
	if(victory_music_.empty())
		victory_music_ = utils::split(game_config::default_victory_music);
}

void play_controller::set_defeat_music_list(const std::string& list)
{
	defeat_music_  = utils::split(list);
	if(defeat_music_.empty())
		defeat_music_ = utils::split(game_config::default_defeat_music);
}

void play_controller::check_victory()
{
	if(linger_)
	{
		return;
	}

	check_end_level();

	bool continue_level, found_player, found_network_player, invalidate_all;
	std::set<unsigned> not_defeated;

	gamestate_.board_.check_victory(continue_level, found_player, found_network_player, invalidate_all, not_defeated, remove_from_carryover_on_defeat_);

	if (invalidate_all) {
		gui_->invalidate_all();
	}

	if (continue_level) {
		return ;
	}

	if (found_player || found_network_player) {
		pump().fire("enemies defeated");
		check_end_level();
	}

	DBG_EE << "victory_when_enemies_defeated: " << victory_when_enemies_defeated_ << std::endl;
	DBG_EE << "found_player: " << found_player << std::endl;
	DBG_EE << "found_network_player: " << found_network_player << std::endl;

	if (!victory_when_enemies_defeated_ && (found_player || found_network_player)) {
		// This level has asked not to be ended by this condition.
		return;
	}

	if (non_interactive()) {
		LOG_AIT << "winner: ";
		BOOST_FOREACH(unsigned l, not_defeated) {
			std::string ai = ai::manager::get_active_ai_identifier_for_side(l);
			if (ai.empty()) ai = "default ai";
			LOG_AIT << l << " (using " << ai << ") ";
		}
		LOG_AIT << std::endl;
		ai_testing::log_victory(not_defeated);
	}

	DBG_EE << "throwing end level exception..." << std::endl;
	//Also proceed to the next scenario when another player survived.
	end_level_data_.proceed_to_next_level = found_player || found_network_player;
	throw end_level_exception(found_player ? VICTORY : DEFEAT);
}

void play_controller::process_oos(const std::string& msg) const
{
	if (game_config::ignore_replay_errors) return;

	std::stringstream message;
	message << _("The game is out of sync. It might not make much sense to continue. Do you want to save your game?");
	message << "\n\n" << _("Error details:") << "\n\n" << msg;

	update_savegame_snapshot();
	savegame::oos_savegame save(saved_game_, *gui_);
	save.save_game_interactive(gui_->video(), message.str(), gui::YES_NO); // can throw end_level_exception
}

void play_controller::update_gui_to_player(const int team_index, const bool observe)
{
	gui_->set_team(team_index, observe);
	gui_->recalculate_minimap();
	gui_->invalidate_all();
	gui_->draw(true,true);
}

void play_controller::do_autosave()
{
	update_savegame_snapshot();
	savegame::autosave_savegame save(saved_game_, *gui_, preferences::save_compression_format());
	save.autosave(false, preferences::autosavemax(), preferences::INFINITE_AUTO_SAVES);
}


void play_controller::do_consolesave(const std::string& filename)
{
	update_savegame_snapshot();
	savegame::ingame_savegame save(saved_game_, *gui_, preferences::save_compression_format());
	save.save_game_automatic(gui_->video(), true, filename);
}

void play_controller::update_savegame_snapshot() const
{
	//note: this writes to level_ if this is not a replay.
	this->saved_game_.set_snapshot(to_config());
}

game_events::t_pump & play_controller::pump() {
	return gamestate_.events_manager_->pump();
}

int play_controller::get_ticks() {
	return ticks_;
}

soundsource::manager * play_controller::get_soundsource_man() {
	return soundsources_manager_.get();
}

plugins_context * play_controller::get_plugins_context() {
	return plugins_context_.get();
}

hotkey::command_executor * play_controller::get_hotkey_command_executor() {
	return hotkey_handler_.get();
}
