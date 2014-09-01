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
#include "game_events/handlers.hpp"
#include "game_events/menu_item.hpp"
#include "game_events/pump.hpp"
#include "game_preferences.hpp"
#include "hotkey/hotkey_item.hpp"
#include "map_label.hpp"
#include "gettext.hpp"
#include "halo.hpp"
#include "loadscreen.hpp"
#include "log.hpp"
#include "pathfind/teleport.hpp"
#include "preferences_display.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "savegame.hpp"
#include "saved_game.hpp"
#include "save_blocker.hpp"
#include "scripting/lua.hpp"
#include "sound.hpp"
#include "soundsource.hpp"
#include "synced_context.hpp"
#include "tooltips.hpp"
#include "unit.hpp"
#include "unit_id.hpp"
#include "whiteboard/manager.hpp"
#include "wmi_pager.hpp"
#include "wml_exception.hpp"

#include <boost/foreach.hpp>

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
		const int ticks, const config& game_config,
		CVideo& video, bool skip_replay) :
	controller_base(ticks, game_config, video),
	observer(),
	savegame_config(),
	gamestate_(level, game_config),
	level_(level),
	saved_game_(state_of_game),
	prefs_disp_manager_(),
	tooltips_manager_(),
	events_manager_(),
	labels_manager_(),
	help_manager_(&game_config),
	mouse_handler_(NULL, gamestate_.board_),
	menu_handler_(NULL, gamestate_.board_, level, game_config),
	soundsources_manager_(),
	persist_(),
	gui_(),
	statistics_context_(level["name"]),
	undo_stack_(new actions::undo_list(level.child("undo_stack"))),
	whiteboard_manager_(),
	loading_game_(level["playing_team"].empty() == false),
	player_number_(1),
	first_player_(level["playing_team"].to_int() + 1),
	start_turn_(gamestate_.tod_manager_.turn()), // gamestate_.tod_manager_ constructed above
	skip_replay_(skip_replay),
	linger_(false),
	it_is_a_new_turn_(true),
	init_side_done_(level["init_side_done"].to_bool(true)),
	savenames_(),
	wml_commands_(),
	wml_command_pager_(new wmi_pager()),
	victory_when_enemies_defeated_(true),
	remove_from_carryover_on_defeat_(true),
	end_level_data_(),
	victory_music_(),
	defeat_music_(),
	scope_()
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

	// This *needs* to be created before the show_intro and show_map_scene
	// as that functions use the manager state_of_game
	// Has to be done before registering any events!
	events_manager_.reset(new game_events::manager(level_));

	LOG_NG << "initializing game_state..." << (SDL_GetTicks() - ticks_) << std::endl;
	gamestate_.init(ticks_);
	resources::tunnels = gamestate_.pathfind_manager_.get();

	// mouse_handler expects at least one team for linger mode to work.
	if (gamestate_.board_.teams().empty()) end_level_data_.transient.linger_mode = false;

	LOG_NG << "loading units..." << (SDL_GetTicks() - ticks_) << std::endl;
	loadscreen::start_stage("load units");
	preferences::encounter_all_content(gamestate_.board_);

	LOG_NG << "initializing theme... " << (SDL_GetTicks() - ticks_) << std::endl;
	loadscreen::start_stage("init theme");
	const config &theme_cfg = get_theme(game_config_, level_["theme"]);

	LOG_NG << "initializing whiteboard..." << (SDL_GetTicks() - ticks_) << std::endl;
	whiteboard_manager_.reset(new wb::manager());
	resources::whiteboard = whiteboard_manager_;

	LOG_NG << "building terrain rules... " << (SDL_GetTicks() - ticks_) << std::endl;
	loadscreen::start_stage("build terrain");
	gui_.reset(new game_display(gamestate_.board_, video, whiteboard_manager_, gamestate_.tod_manager_, theme_cfg, level_));
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
#if 0
	// [era] and [modification] childs don't exist anymore in level_
	// the events are now added in saved_game::expand_mp_events
	// add era & mod events for MP game
	if (const config &era_cfg = level_.child("era")) {
		game_events::add_events(era_cfg.child_range("event"), "era_events");
	}

	if (level_.child_or_empty("modification")) {
		BOOST_FOREACH (const config& mod_cfg, level_.child_range("modification")) {
			game_events::add_events(mod_cfg.child_range("event"),
									"mod_" + mod_cfg["id"].str() + "_events");
			BOOST_FOREACH (const config::any_child& var_cfg, mod_cfg.child("variables").all_children_range()) {
				gamestate_.gamedata_.add_variable_cfg(var_cfg.key, var_cfg.cfg);
			}
		}
	}
#endif
	loadscreen::global_loadscreen->start_stage("start game");
	loadscreen_manager->reset();
}

void play_controller::init_managers(){
	LOG_NG << "initializing managers... " << (SDL_GetTicks() - ticks_) << std::endl;
	prefs_disp_manager_.reset(new preferences::display_manager(gui_.get()));
	tooltips_manager_.reset(new tooltips::manager(gui_->video()));
	soundsources_manager_.reset(new soundsource::manager(*gui_));

	resources::soundsources = soundsources_manager_.get();
	LOG_NG << "done initializing managers... " << (SDL_GetTicks() - ticks_) << std::endl;
}

void play_controller::objectives(){
	menu_handler_.objectives(gui_->viewing_team()+1);
}

void play_controller::show_statistics(){
	menu_handler_.show_statistics(gui_->viewing_team()+1);
}

void play_controller::unit_list(){
	menu_handler_.unit_list();
}

void play_controller::status_table(){
	menu_handler_.status_table();
}

void play_controller::save_game(){
	if(save_blocker::try_block()) {
		save_blocker::save_unblocker unblocker;
		savegame::ingame_savegame save(saved_game_, *gui_, to_config(), preferences::save_compression_format());
		save.save_game_interactive(gui_->video(), "", gui::OK_CANCEL);
	} else {
		save_blocker::on_unblock(this,&play_controller::save_game);
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

void play_controller::save_map(){
	if(save_blocker::try_block()) {
		save_blocker::save_unblocker unblocker;
		menu_handler_.save_map();
	} else {
		save_blocker::on_unblock(this,&play_controller::save_map);
	}
}

void play_controller::load_game(){
	savegame::loadgame load(*gui_, game_config_, saved_game_);
	load.load_game();
}

void play_controller::preferences(){
	menu_handler_.preferences();
}

void play_controller::left_mouse_click(){
	int x = gui_->get_location_x(gui_->mouseover_hex());
	int y = gui_->get_location_y(gui_->mouseover_hex());

	SDL_MouseButtonEvent event;

	event.button = 1;
	event.x = x + 30;
	event.y = y + 30;
	event.which = 0;
	event.state = SDL_PRESSED;

	mouse_handler_.mouse_press(event, false);
}

void play_controller::select_and_action() {
	mouse_handler_.select_or_action(browse_);
}

void play_controller::move_action(){
	mouse_handler_.move_action(browse_);
}

void play_controller::deselect_hex(){
	mouse_handler_.deselect_hex();
}
void play_controller::select_hex(){
	mouse_handler_.select_hex(gui_->mouseover_hex(), false);
}

void play_controller::right_mouse_click(){
	int x = gui_->get_location_x(gui_->mouseover_hex());
	int y = gui_->get_location_y(gui_->mouseover_hex());

	SDL_MouseButtonEvent event;

	event.button = 3;
	event.x = x + 30;
	event.y = y + 30;
	event.which = 0;
	event.state = SDL_PRESSED;

	mouse_handler_.mouse_press(event, true);
}


void play_controller::cycle_units(){
	mouse_handler_.cycle_units(browse_);
}

void play_controller::cycle_back_units(){
	mouse_handler_.cycle_back_units(browse_);
}

void play_controller::show_chat_log(){
	menu_handler_.show_chat_log();
}

void play_controller::show_help(){
	menu_handler_.show_help();
}

void play_controller::undo(){
	mouse_handler_.deselect_hex();
	undo_stack_->undo();
}

void play_controller::redo(){
	mouse_handler_.deselect_hex();
	undo_stack_->redo();
}

void play_controller::show_enemy_moves(bool ignore_units){
	menu_handler_.show_enemy_moves(ignore_units, player_number_);
}

void play_controller::goto_leader(){
	menu_handler_.goto_leader(player_number_);
}

void play_controller::unit_description(){
	menu_handler_.unit_description();
}

void play_controller::terrain_description(){
	menu_handler_.terrain_description(mouse_handler_);
}

void play_controller::toggle_ellipses(){
	menu_handler_.toggle_ellipses();
}

void play_controller::toggle_grid(){
	menu_handler_.toggle_grid();
}

void play_controller::search(){
	menu_handler_.search();
}

void play_controller::fire_preload()
{
	// Run initialization scripts, even if loading from a snapshot.
	gamestate_.gamedata_.set_phase(game_data::PRELOAD);
	resources::lua_kernel->initialize();
	gamestate_.gamedata_.get_variable("turn_number") = int(turn());
	game_events::fire("preload");
}
void play_controller::fire_prestart()
{
	// pre-start events must be executed before any GUI operation,
	// as those may cause the display to be refreshed.
	update_locker lock_display(gui_->video());
	gamestate_.gamedata_.set_phase(game_data::PRESTART);
	game_events::fire("prestart");
	check_end_level();
	// prestart event may modify start turn with WML, reflect any changes.
	start_turn_ = turn();
	gamestate_.gamedata_.get_variable("turn_number") = int(start_turn_);
	// prestart event may modify the initial gold amount, reflect any changes.
	BOOST_FOREACH(team& tm, gamestate_.board_.teams_)
	{
		tm.set_start_gold(tm.gold());
	}
}

void play_controller::fire_start(bool execute){
	if(execute) {
		gamestate_.gamedata_.set_phase(game_data::START);
		game_events::fire("start");
		check_end_level();
		// start event may modify start turn with WML, reflect any changes.
		start_turn_ = turn();
		gamestate_.gamedata_.get_variable("turn_number") = int(start_turn_);
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

	if(!only_visual){
		recorder.init_side();
		set_scontext_synced sync;
		do_init_side(is_replay);
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
	//and we dont want do_init_side to be called when a player drops.
	init_side_done_ = true;

	const std::string turn_num = str_cast(turn());
	const std::string side_num = str_cast(player_number_);

	// If this is right after loading a game we don't need to fire events and such. It was already done before saving.
	if (!only_visual) {
		if(it_is_a_new_turn_)
		{
			game_events::fire("turn " + turn_num);
			game_events::fire("new turn");
			it_is_a_new_turn_ = false;
		}

		game_events::fire("side turn");
		game_events::fire("side " + side_num + " turn");
		game_events::fire("side turn " + turn_num);
		game_events::fire("side " + side_num + " turn " + turn_num);
	}

	if(current_team().is_human() && !is_replay) {
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

		game_events::fire("turn refresh");
		game_events::fire("side " + side_num + " turn refresh");
		game_events::fire("turn " + turn_num + " refresh");
		game_events::fire("side " + side_num + " turn " + turn_num + " refresh");

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

	cfg.merge_with(gamestate_.to_config());

	if(linger_) {
		config endlevel;
		end_level_data_.write(endlevel);
		cfg.add_child("end_level_data", endlevel);
	}

	// Write terrain_graphics data in snapshot, too
	BOOST_FOREACH(const config &tg, level_.child_range("terrain_graphics")) {
		cfg.add_child("terrain_graphics", tg);
	}

	config display;
	gui_->write(display);
	cfg.add_child("display", display);

	// Preserve the undo stack so that fog/shroud clearing is kept accurate.
	undo_stack_->write(cfg.add_child("undo_stack"));

	//Write the game events.
	game_events::write_events(cfg);


	if(gui_.get() != NULL){
		cfg["playing_team"] = str_cast(gui_->playing_team());
		gui_->labels().write(cfg);
		sound::write_music_play_list(cfg);
	}

	cfg["next_underlying_unit_id"] = str_cast(n_unit::id_manager::instance().get_save_id());
	//TODO: i am not sure whether the next line is needed.
	cfg.merge_attributes(saved_game_.classification().to_config());
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
		game_events::fire("side turn end");
		game_events::fire("side "+ side_num + " turn end");
		game_events::fire("side turn " + turn_num + " end");
		game_events::fire("side " + side_num + " turn " + turn_num + " end");
	}
	// This is where we refog, after all of a side's events are done.
	actions::recalculate_fog(player_number_);

	// This implements "delayed map sharing."
	// It is meant as an alternative to shared vision.
	if(current_team().copy_ally_shroud()) {
		gui_->recalculate_minimap();
		gui_->invalidate_all();
	}

	mouse_handler_.deselect_hex();
	n_unit::id_manager::instance().reset_fake();
	game_events::pump();
}

void play_controller::finish_turn()
{
	set_scontext_synced sync(2);
	const std::string turn_num = str_cast(turn());
	game_events::fire("turn end");
	game_events::fire("turn " + turn_num + " end");
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

bool play_controller::execute_command(const hotkey::hotkey_command& cmd, int index)
{
	hotkey::HOTKEY_COMMAND command = cmd.id;
	if(index >= 0) {
		unsigned i = static_cast<unsigned>(index);
		if(i < savenames_.size() && !savenames_[i].empty()) {
			// Load the game by throwing load_game_exception
			throw game::load_game_exception(savenames_[i],false,false,false,"");

		} else if ( i < wml_commands_.size()  &&  wml_commands_[i] ) {
			if (!wml_command_pager_->capture(*wml_commands_[i])) {
				wml_commands_[i]->fire_event(mouse_handler_.get_last_hex());
			} else { //relaunch the menu
				show_menu(get_display().get_theme().context_menu()->items(),last_context_menu_x_,last_context_menu_y_,true, get_display());
			}
			return true;
		}
	}
	int prefixlen = wml_menu_hotkey_prefix.length();
	if(command == hotkey::HOTKEY_WML && cmd.command.compare(0, prefixlen, wml_menu_hotkey_prefix) == 0)
	{
		std::string name = cmd.command.substr(prefixlen);
		const map_location& hex = mouse_handler_.get_last_hex();

		gamestate_.gamedata_.get_wml_menu_items().fire_item(name, hex);
		/// @todo Shouldn't the function return at this point?
	}
	return command_executor::execute_command(cmd, index);
}

bool play_controller::can_execute_command(const hotkey::hotkey_command& cmd, int index) const
{
	if(index >= 0) {
		unsigned i = static_cast<unsigned>(index);
		if((i < savenames_.size() && !savenames_[i].empty())
		|| (i < wml_commands_.size() && wml_commands_[i])) {
			return true;
		}
	}
	switch(cmd.id) {

	// Commands we can always do:
	case hotkey::HOTKEY_LEADER:
	case hotkey::HOTKEY_CYCLE_UNITS:
	case hotkey::HOTKEY_CYCLE_BACK_UNITS:
	case hotkey::HOTKEY_ZOOM_IN:
	case hotkey::HOTKEY_ZOOM_OUT:
	case hotkey::HOTKEY_ZOOM_DEFAULT:
	case hotkey::HOTKEY_FULLSCREEN:
	case hotkey::HOTKEY_SCREENSHOT:
	case hotkey::HOTKEY_MAP_SCREENSHOT:
	case hotkey::HOTKEY_ACCELERATED:
	case hotkey::HOTKEY_SAVE_MAP:
	case hotkey::HOTKEY_TOGGLE_ELLIPSES:
	case hotkey::HOTKEY_TOGGLE_GRID:
	case hotkey::HOTKEY_MOUSE_SCROLL:
	case hotkey::HOTKEY_ANIMATE_MAP:
	case hotkey::HOTKEY_STATUS_TABLE:
	case hotkey::HOTKEY_MUTE:
	case hotkey::HOTKEY_PREFERENCES:
	case hotkey::HOTKEY_OBJECTIVES:
	case hotkey::HOTKEY_UNIT_LIST:
	case hotkey::HOTKEY_STATISTICS:
	case hotkey::HOTKEY_QUIT_GAME:
	case hotkey::HOTKEY_SEARCH:
	case hotkey::HOTKEY_HELP:
	case hotkey::HOTKEY_USER_CMD:
	case hotkey::HOTKEY_CUSTOM_CMD:
	case hotkey::HOTKEY_AI_FORMULA:
	case hotkey::HOTKEY_CLEAR_MSG:
	case hotkey::HOTKEY_SELECT_HEX:
	case hotkey::HOTKEY_DESELECT_HEX:
	case hotkey::HOTKEY_MOVE_ACTION:
	case hotkey::HOTKEY_SELECT_AND_ACTION:
	case hotkey::HOTKEY_MINIMAP_CODING_TERRAIN:
	case hotkey::HOTKEY_MINIMAP_CODING_UNIT:
	case hotkey::HOTKEY_MINIMAP_DRAW_UNITS:
	case hotkey::HOTKEY_MINIMAP_DRAW_TERRAIN:
	case hotkey::HOTKEY_MINIMAP_DRAW_VILLAGES:
	case hotkey::HOTKEY_NULL:
	case hotkey::HOTKEY_SAVE_REPLAY:
		return true;

	// Commands that have some preconditions:
	case hotkey::HOTKEY_SAVE_GAME:
		return !events::commands_disabled;

	case hotkey::HOTKEY_SHOW_ENEMY_MOVES:
	case hotkey::HOTKEY_BEST_ENEMY_MOVES:
		return !linger_ && enemies_visible();

	case hotkey::HOTKEY_LOAD_GAME:
		return network::nconnections() == 0; // Can only load games if not in a network game

	case hotkey::HOTKEY_CHAT_LOG:
		return network::nconnections() > 0;

	case hotkey::HOTKEY_REDO:
		return !linger_ && undo_stack_->can_redo() && !events::commands_disabled && !browse_;
	case hotkey::HOTKEY_UNDO:
		return !linger_ && undo_stack_->can_undo() && !events::commands_disabled && !browse_;

	case hotkey::HOTKEY_UNIT_DESCRIPTION:
		return menu_handler_.current_unit().valid();

	case hotkey::HOTKEY_TERRAIN_DESCRIPTION:
		return mouse_handler_.get_last_hex().valid();

	case hotkey::HOTKEY_RENAME_UNIT:
		return !events::commands_disabled &&
			menu_handler_.current_unit().valid() &&
			!(menu_handler_.current_unit()->unrenamable()) &&
			menu_handler_.current_unit()->side() == gui_->viewing_side() &&
			gamestate_.board_.teams()[menu_handler_.current_unit()->side() - 1].is_human();

	default:
		return false;
	}
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
		if (gamestate_.board_.teams()[i].is_human()) {
			return i+1;
		}
	}

	for (int i = gamestate_.board_.teams().size()-1; i > player_number_-1; --i) {
		if (gamestate_.board_.teams()[i].is_human()) {
			return i+1;
		}
	}

	return -1;
}

void play_controller::slice_before_scroll() {
	soundsources_manager_->update();
}

events::mouse_handler& play_controller::get_mouse_handler_base() {
	return mouse_handler_;
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

static void trim_items(std::vector<std::string>& newitems) {
	if (newitems.size() > 5) {
		std::vector<std::string> subitems;
		subitems.push_back(newitems[0]);
		subitems.push_back(newitems[1]);
		subitems.push_back(newitems[newitems.size() / 3]);
		subitems.push_back(newitems[newitems.size() * 2 / 3]);
		subitems.push_back(newitems.back());
		newitems = subitems;
	}
}

void play_controller::expand_autosaves(std::vector<std::string>& items)
{
	const compression::format comp_format =
		preferences::save_compression_format();

	savenames_.clear();
	for (unsigned int i = 0; i < items.size(); ++i) {
		if (items[i] == "AUTOSAVES") {
			items.erase(items.begin() + i);
			std::vector<std::string> newitems;
			std::vector<std::string> newsaves;
			for (unsigned int turn = this->turn(); turn != 0; turn--) {
				std::string name = saved_game_.classification().label + "-" + _("Auto-Save") + lexical_cast<std::string>(turn);
				if (savegame::save_game_exists(name, comp_format)) {
					newsaves.push_back(
						name + compression::format_extension(comp_format));
					newitems.push_back(_("Back to Turn ") + lexical_cast<std::string>(turn));
				}
			}

			const std::string& start_name = saved_game_.classification().label;
			if(savegame::save_game_exists(start_name, comp_format)) {
				newsaves.push_back(
					start_name + compression::format_extension(comp_format));
				newitems.push_back(_("Back to Start"));
			}

			// Make sure list doesn't get too long: keep top two,
			// midpoint and bottom.
			trim_items(newitems);
			trim_items(newsaves);

			items.insert(items.begin()+i, newitems.begin(), newitems.end());
			savenames_.insert(savenames_.end(), newsaves.begin(), newsaves.end());
			break;
		}
		savenames_.push_back("");
	}
}
///replaces "wml" in @items with all active wml menu items for the current field
void play_controller::expand_wml_commands(std::vector<std::string>& items)
{
	wml_commands_.clear();
	for (unsigned int i = 0; i < items.size(); ++i) {
		if (items[i] == "wml") {
			std::vector<std::string> newitems;

			// Replace this placeholder entry with available menu items.
			items.erase(items.begin() + i);
			wml_command_pager_->update_ref(&gamestate_.gamedata_.get_wml_menu_items());
			wml_command_pager_->get_items(mouse_handler_.get_last_hex(),
			                                         wml_commands_, newitems);
			items.insert(items.begin()+i, newitems.begin(), newitems.end());
			// End the "for" loop.
			break;
		}
		// Pad the commands with null pointers (keeps the indices of items and
		// wml_commands_ synced).
		wml_commands_.push_back(const_item_ptr());
	}
}

void play_controller::show_menu(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu, display& disp)
{
	if (context_menu)
	{
		last_context_menu_x_ = xloc;
		last_context_menu_y_ = yloc;
	}

	std::vector<std::string> items = items_arg;
	const hotkey::hotkey_command* cmd;
	std::vector<std::string>::iterator i = items.begin();
	while(i != items.end()) {
		if (*i == "AUTOSAVES") {
			// Autosave visibility is similar to LOAD_GAME hotkey
			cmd = &hotkey::hotkey_command::get_command_by_command(hotkey::HOTKEY_LOAD_GAME);
		} else {
			cmd = &hotkey::get_hotkey_command(*i);
		}
		// Remove WML commands if they would not be allowed here
		if(*i == "wml") {
			if(!context_menu || gui_->viewing_team() != gui_->playing_team()
			|| events::commands_disabled || !gamestate_.board_.teams()[gui_->viewing_team()].is_human()
			|| (linger_ && !game_config::debug)){
				i = items.erase(i);
				continue;
			}
		// Remove commands that can't be executed or don't belong in this type of menu
		} else if(!can_execute_command(*cmd)
			|| (context_menu && !in_context_menu(cmd->id))) {
			i = items.erase(i);
			continue;
		}
		++i;
	}

	// Add special non-hotkey items to the menu and remember their indices
	expand_autosaves(items);
	expand_wml_commands(items);

	if(items.empty())
		return;

	command_executor::show_menu(items, xloc, yloc, context_menu, disp);
}

bool play_controller::in_context_menu(hotkey::HOTKEY_COMMAND command) const
{
	switch(command) {
	// Only display these if the mouse is over a castle or keep tile
	case hotkey::HOTKEY_RECRUIT:
	case hotkey::HOTKEY_REPEAT_RECRUIT:
	case hotkey::HOTKEY_RECALL: {
		// last_hex_ is set by mouse_events::mouse_motion
		const map_location & last_hex = mouse_handler_.get_last_hex();
		const int viewing_side = gui_->viewing_side();

		// A quick check to save us having to create the future map and
		// possibly loop through all units.
		if ( !gamestate_.board_.map().is_keep(last_hex)  &&
		     !gamestate_.board_.map().is_castle(last_hex) )
			return false;

		wb::future_map future; /* lasts until method returns. */

		unit_map::const_iterator leader = gamestate_.board_.units().find(last_hex);
		if ( leader != gamestate_.board_.units().end() )
			return leader->can_recruit()  &&  leader->side() == viewing_side  &&
			       can_recruit_from(*leader);
		else
			// Look for a leader who can recruit on last_hex.
			for ( leader = gamestate_.board_.units().begin(); leader != gamestate_.board_.units().end(); ++leader) {
				if ( leader->can_recruit()  &&  leader->side() == viewing_side  &&
				     can_recruit_on(*leader, last_hex) )
					return true;
			}
		// No leader found who can recruit at last_hex.
		return false;
	}
	default:
		return true;
	}
}

std::string play_controller::get_action_image(hotkey::HOTKEY_COMMAND command, int index) const
{
	if(index >= 0 && index < static_cast<int>(wml_commands_.size())) {
		const const_item_ptr wmi = wml_commands_[index];
		if ( wmi ) {
			return wmi->image();
		}
	}
	return command_executor::get_action_image(command, index);
}

hotkey::ACTION_STATE play_controller::get_action_state(hotkey::HOTKEY_COMMAND command, int /*index*/) const
{
	switch(command) {

	case hotkey::HOTKEY_MINIMAP_DRAW_VILLAGES:
		return (preferences::minimap_draw_villages()) ? hotkey::ACTION_ON : hotkey::ACTION_OFF;
	case hotkey::HOTKEY_MINIMAP_CODING_UNIT:
		return (preferences::minimap_movement_coding()) ? hotkey::ACTION_ON : hotkey::ACTION_OFF;
	case hotkey::HOTKEY_MINIMAP_CODING_TERRAIN:
		return (preferences::minimap_terrain_coding()) ? hotkey::ACTION_ON : hotkey::ACTION_OFF;
	case hotkey::HOTKEY_MINIMAP_DRAW_UNITS:
		return (preferences::minimap_draw_units()) ? hotkey::ACTION_ON : hotkey::ACTION_OFF;
	case hotkey::HOTKEY_MINIMAP_DRAW_TERRAIN:
		return (preferences::minimap_draw_terrain()) ? hotkey::ACTION_ON : hotkey::ACTION_OFF;
	case hotkey::HOTKEY_ZOOM_DEFAULT:
		return (gui_->get_zoom_factor() == 1.0) ? hotkey::ACTION_ON : hotkey::ACTION_OFF;
	case hotkey::HOTKEY_DELAY_SHROUD:
		return gamestate_.board_.teams()[gui_->viewing_team()].auto_shroud_updates() ? hotkey::ACTION_OFF : hotkey::ACTION_ON;
	default:
		return hotkey::ACTION_STATELESS;
	}
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
	std::set<unsigned> not_defeated;

	BOOST_FOREACH( const unit & i , gamestate_.board_.units())
	{
		DBG_EE << "Found a unit: " << i.id() << " on side " << i.side() << std::endl;
		const team& tm = gamestate_.board_.teams()[i.side()-1];
		DBG_EE << "That team's defeat condition is: " << lexical_cast<std::string> (tm.defeat_condition()) << std::endl;
		if (i.can_recruit() && tm.defeat_condition() == team::NO_LEADER) {
			not_defeated.insert(i.side());
		} else if (tm.defeat_condition() == team::NO_UNITS) {
			not_defeated.insert(i.side());
		}
	}

	BOOST_FOREACH(team& tm, gamestate_.board_.teams_)
	{
		if(tm.defeat_condition() == team::NEVER)
		{
			not_defeated.insert(tm.side());
		}
		// Clear villages for teams that have no leader and
		// mark side as lost if it should be removed from carryover.
		if (not_defeated.find(tm.side()) == not_defeated.end())
		{
			tm.clear_villages();
			// invalidate_all() is overkill and expensive but this code is
			// run rarely so do it the expensive way.
			gui_->invalidate_all();

			if (remove_from_carryover_on_defeat_)
			{
				tm.set_lost();
			}
		}
	}

	check_end_level();

	bool found_player = false;
	bool found_network_player = false;

	for (std::set<unsigned>::iterator n = not_defeated.begin(); n != not_defeated.end(); ++n) {
		size_t side = *n - 1;

		DBG_EE << "Side " << (side+1) << " is a not-defeated team" << std::endl;

		std::set<unsigned>::iterator m(n);
		for (++m; m != not_defeated.end(); ++m) {
			if (gamestate_.board_.teams()[side].is_enemy(*m)) {
				return;
			}
			DBG_EE << "Side " << (side+1) << " and " << *m << " are not enemies." << std::endl;
		}

		if (gamestate_.board_.teams()[side].is_human()) {
			found_player = true;
		}

		if (gamestate_.board_.teams()[side].is_network()) {
			found_network_player = true;
		}
	}

	if (found_player || found_network_player) {
		game_events::fire("enemies defeated");
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

	savegame::oos_savegame save(saved_game_, *gui_, to_config());
	save.save_game_interactive(gui_->video(), message.str(), gui::YES_NO); // can throw end_level_exception
}

//this should be at the end of the file but it caused merging problems there.
const std::string play_controller::wml_menu_hotkey_prefix = "wml_menu:";


void play_controller::update_gui_to_player(const int team_index, const bool observe)
{
	gui_->set_team(team_index, observe);
	gui_->recalculate_minimap();
	gui_->invalidate_all();
	gui_->draw(true,true);
}

void play_controller::toggle_accelerated_speed()
{
	preferences::set_turbo(!preferences::turbo());

	if (preferences::turbo())
	{
		utils::string_map symbols;
		symbols["hk"] = hotkey::get_names(hotkey::hotkey_command::get_command_by_command(hotkey::HOTKEY_ACCELERATED).command);
		gui_->announce(_("Accelerated speed enabled!"), font::NORMAL_COLOR);
		gui_->announce("\n" + vgettext("(press $hk to disable)", symbols), font::NORMAL_COLOR);
	}
	else
	{
		gui_->announce(_("Accelerated speed disabled!"), font::NORMAL_COLOR);
	}
}

void play_controller::do_autosave()
{
	savegame::autosave_savegame save(saved_game_, *gui_, to_config(), preferences::save_compression_format());
	save.autosave(false, preferences::autosavemax(), preferences::INFINITE_AUTO_SAVES);
}


void play_controller::do_consolesave(const std::string& filename)
{
	savegame::ingame_savegame save(saved_game_, *gui_,
	                               to_config(), preferences::save_compression_format());
	save.save_game_automatic(gui_->video(), true, filename);
}


