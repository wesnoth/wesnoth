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
 *  Handle input via mouse & keyboard, events, schedule commands.
 */

#include "play_controller.hpp"

#include "actions/heal.hpp"
#include "actions/undo.hpp"
#include "actions/vision.hpp"
#include "ai/manager.hpp"
#include "ai/testing.hpp"
#include "display_chat_manager.hpp"
#include "floating_label.hpp"
#include "formula/string_utils.hpp"
#include "game_errors.hpp"
#include "game_events/menu_item.hpp"
#include "game_events/pump.hpp"
#include "game_state.hpp"
#include "gettext.hpp"
#include "gui/dialogs/loading_screen.hpp"
#include "gui/dialogs/message.hpp"      // for show_error_message
#include "gui/dialogs/transient_message.hpp"
#include "hotkey/command_executor.hpp"
#include "hotkey/hotkey_handler.hpp"
#include "log.hpp"
#include "map/label.hpp"
#include "pathfind/teleport.hpp"
#include "preferences/preferences.hpp"
#include "random.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "save_index.hpp"
#include "saved_game.hpp"
#include "savegame.hpp"
#include "scripting/game_lua_kernel.hpp"
#include "scripting/plugins/context.hpp"
#include "sound.hpp"
#include "soundsource.hpp"
#include "statistics.hpp"
#include "synced_context.hpp"
#include "units/types.hpp"
#include "units/unit.hpp"
#include "utils/general.hpp"
#include "video.hpp"
#include "whiteboard/manager.hpp"

#include <functional>

static lg::log_domain log_aitesting("ai/testing");
#define LOG_AIT LOG_STREAM(info, log_aitesting)
// If necessary, this define can be replaced with `#define LOG_AIT std::cout` to restore previous behavior

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)
#define DBG_NG LOG_STREAM(debug, log_engine)
#define ERR_NG LOG_STREAM(err, log_engine)

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)

static lg::log_domain log_enginerefac("enginerefac");
#define LOG_RG LOG_STREAM(info, log_enginerefac)

static lg::log_domain log_engine_enemies("engine/enemies");
#define DBG_EE LOG_STREAM(debug, log_engine_enemies)

/**
 * Copies [scenario] attributes/tags that are not otherwise stored in C++ structs/clases.
 */
static void copy_persistent(const config& src, config& dst)
{
	static const std::set<std::string> attrs {
		"description",
		"name",
		"disallow_recall",
		"experience_modifier",
		"require_scenario",
		"loaded_resources"
	};

	static const std::set<std::string> tags {
		"terrain_graphics",
		"modify_unit_type",
		"lua"
	};

	for(const std::string& attr : attrs) {
		dst[attr] = src[attr];
	}

	for(const std::string& tag : tags) {
		dst.append_children(src, tag);
	}
}

static void clear_resources()
{
	resources::controller = nullptr;
	resources::filter_con = nullptr;
	resources::gameboard = nullptr;
	resources::gamedata = nullptr;
	resources::lua_kernel = nullptr;
	resources::game_events = nullptr;
	resources::persist = nullptr;
	resources::soundsources = nullptr;
	resources::tod_manager = nullptr;
	resources::tunnels = nullptr;
	resources::undo_stack = nullptr;
	resources::recorder = nullptr;
	resources::whiteboard.reset();
	resources::classification = nullptr;
}

play_controller::play_controller(const config& level, saved_game& state_of_game)
	: controller_base()
	, observer()
	, quit_confirmation()
	, ticks_(SDL_GetTicks())
	, gamestate_()
	, level_()
	, saved_game_(state_of_game)
	, tooltips_manager_()
	, whiteboard_manager_()
	, plugins_context_()
	, labels_manager_(new font::floating_label_context())
	, help_manager_(&game_config_)
	, mouse_handler_(nullptr, *this)
	, menu_handler_(nullptr, *this)
	, hotkey_handler_(new hotkey_handler(*this, saved_game_))
	, soundsources_manager_()
	, persist_()
	, gui_()
	, xp_mod_(new unit_experience_accelerator(level["experience_modifier"].to_int(100)))
	, statistics_context_(new statistics_t(state_of_game.statistics()))
	, replay_(new replay(state_of_game.get_replay()))
	, skip_replay_(false)
	, skip_story_(state_of_game.skip_story())
	, did_autosave_this_turn_(true)
	, did_tod_sound_this_turn_(false)
	, map_start_()
	, start_faded_(true)
	, victory_music_()
	, defeat_music_()
	, scope_(hotkey::scope_game)
	, ignore_replay_errors_(false)
	, player_type_changed_(false)
{
	copy_persistent(level, level_);

	for(const config& modify_unit_type : level_.child_range("modify_unit_type")) {
		unit_types.apply_scenario_fix(modify_unit_type);
	}
	resources::controller = this;
	resources::persist = &persist_;
	resources::recorder = replay_.get();

	resources::classification = &saved_game_.classification();

	persist_.start_transaction();

	game_config::add_color_info(game_config_view::wrap(level));

	try {
		init(level);
	} catch(...) {
		DBG_NG << "Caught exception initializing level: " << utils::get_unknown_exception_type();
		clear_resources();
		throw;
	}
}

play_controller::~play_controller()
{
	unit_types.remove_scenario_fixes();
	clear_resources();
}

void play_controller::init(const config& level)
{
	/*
	 * Initilisation currently happens on the following order:
	 * 1) This code, which is executed with the loadingscreen
	 *    From inside the constructor.
	 * 2) The Music is changed.
	 * 3) The Storyscreen is shown.
	 * 4) The Labels are added
	 * 5) The preload event is fired
	 * 6) The prestart event is fired
	 * 7) The gui is activated
	 * 8) The start event is fired
	 */
	gui2::dialogs::loading_screen::display([this, &level]() {
		gui2::dialogs::loading_screen::progress(loading_stage::load_level);

		LOG_NG << "initializing game_state..." << (SDL_GetTicks() - ticks());
		gamestate_.reset(new game_state(level, *this));

		resources::gameboard = &gamestate().board_;
		resources::gamedata = &gamestate().gamedata_;
		resources::tod_manager = &gamestate().tod_manager_;
		resources::filter_con = &gamestate();
		resources::undo_stack = &undo_stack();
		resources::game_events = gamestate().events_manager_.get();
		resources::lua_kernel = gamestate().lua_kernel_.get();

		gamestate_->ai_manager_.add_observer(this);
		gamestate_->init(level, *this);
		resources::tunnels = gamestate().pathfind_manager_.get();

		LOG_NG << "initializing whiteboard..." << (SDL_GetTicks() - ticks());
		gui2::dialogs::loading_screen::progress(loading_stage::init_whiteboard);
		whiteboard_manager_.reset(new wb::manager());
		resources::whiteboard = whiteboard_manager_;

		LOG_NG << "loading units..." << (SDL_GetTicks() - ticks());
		gui2::dialogs::loading_screen::progress(loading_stage::load_units);
		prefs::get().encounter_all_content(gamestate().board_);

		LOG_NG << "initializing theme... " << (SDL_GetTicks() - ticks());
		gui2::dialogs::loading_screen::progress(loading_stage::init_theme);

		LOG_NG << "building terrain rules... " << (SDL_GetTicks() - ticks());
		gui2::dialogs::loading_screen::progress(loading_stage::build_terrain);

		gui_.reset(new game_display(gamestate().board_, whiteboard_manager_, *gamestate().reports_, theme(), level));
		map_start_ = map_location(level.child_or_empty("display").child_or_empty("location"));
		if(start_faded_) {
			gui_->set_fade({0,0,0,255});
			gui_->set_prevent_draw(true);
		}

		// Ensure the loading screen doesn't end up underneath the game display
		gui2::dialogs::loading_screen::raise();

		if(!video::headless()) {
			if(saved_game_.mp_settings().mp_countdown) {
				gui_->get_theme().modify_label("time-icon", _("time left for current turn"));
			} else {
				gui_->get_theme().modify_label("time-icon", _("current local time"));
			}
		}

		gui2::dialogs::loading_screen::progress(loading_stage::init_display);
		mouse_handler_.set_gui(gui_.get());
		menu_handler_.set_gui(gui_.get());

		LOG_NG << "done initializing display... " << (SDL_GetTicks() - ticks());

		LOG_NG << "building gamestate to gui and whiteboard... " << (SDL_GetTicks() - ticks());
		// This *needs* to be created before the show_intro and show_map_scene
		// as that functions use the manager state_of_game
		// Has to be done before registering any events!
		gamestate().set_game_display(gui_.get());
		gui2::dialogs::loading_screen::progress(loading_stage::init_lua);

		init_managers();
		gui2::dialogs::loading_screen::progress(loading_stage::start_game);
		// loadscreen_manager->reset();
		gamestate().gamedata_.set_phase(game_data::PRELOAD);
		gamestate().lua_kernel_->load_game(level);

		plugins_context_.reset(new plugins_context("Game"));
		plugins_context_->set_callback("save_game", [this](const config& cfg) { save_game_auto(cfg["filename"]); }, true);
		plugins_context_->set_callback("save_replay", [this](const config& cfg) { save_replay_auto(cfg["filename"]); }, true);
		plugins_context_->set_callback("quit", [](const config&) { throw_quit_game_exception(); }, false);
		plugins_context_->set_accessor_string("scenario_name", [this](config) { return get_scenario_name(); });
	});
}

void play_controller::reset_gamestate(const config& level, int replay_pos)
{
	// TODO: should we update we update this->level_ with level ?

	resources::gameboard = nullptr;
	resources::gamedata = nullptr;
	resources::tod_manager = nullptr;
	resources::filter_con = nullptr;
	resources::lua_kernel = nullptr;
	resources::game_events = nullptr;
	resources::tunnels = nullptr;
	resources::undo_stack = nullptr;

	gui_->labels().set_team(nullptr);

	/* First destroy the old game state, then create the new one.
	This is necessary to ensure that while the old AI manager is being destroyed,
	all its member objects access the old manager instead of the new. */
	gamestate_.reset();
	gamestate_.reset(new game_state(level, *this));

	resources::gameboard = &gamestate().board_;
	resources::gamedata = &gamestate().gamedata_;
	resources::tod_manager = &gamestate().tod_manager_;
	resources::filter_con = &gamestate();
	resources::undo_stack = &undo_stack();
	resources::game_events = gamestate().events_manager_.get();
	resources::lua_kernel = gamestate().lua_kernel_.get();

	gamestate_->ai_manager_.add_observer(this);
	gamestate_->init(level, *this);
	gamestate().set_game_display(gui_.get());
	resources::tunnels = gamestate().pathfind_manager_.get();

	gui_->reset_reports(*gamestate().reports_);
	gui_->change_display_context(&gamestate().board_);
	saved_game_.get_replay().set_pos(replay_pos);

	gamestate().gamedata_.set_phase(game_data::PRELOAD);
	gamestate().lua_kernel_->load_game(level);
}

void play_controller::init_managers()
{
	LOG_NG << "initializing managers... " << (SDL_GetTicks() - ticks());
	soundsources_manager_.reset(new soundsource::manager(*gui_));

	resources::soundsources = soundsources_manager_.get();
	LOG_NG << "done initializing managers... " << (SDL_GetTicks() - ticks());
}

void play_controller::fire_preload()
{
	// Run initialization scripts, even if loading from a snapshot.
	gamestate().gamedata_.get_variable("turn_number") = static_cast<int>(turn());
	pump().fire("preload");
	gamestate().lua_kernel_->preload_finished();
}

void play_controller::fire_prestart()
{
	// pre-start events must be executed before any GUI operation,
	// as those may cause the display to be refreshed.
	gamestate().gamedata_.set_phase(game_data::PRESTART);

	// Fire these right before prestart events, to catch only the units sides
	// have started with.
	for(const unit& u : get_units()) {
		pump().fire("unit_placed", map_location(u.get_location()));
	}

	pump().fire("prestart");

	// prestart event may modify start turn with WML, reflect any changes.
	gamestate().gamedata_.get_variable("turn_number") = static_cast<int>(turn());
}

void play_controller::refresh_objectives() const
{
	const config cfg("side", gui_->viewing_side());
	gamestate().lua_kernel_->run_wml_action("show_objectives", vconfig(cfg),
		game_events::queued_event("_from_interface", "", map_location(), map_location(), config()));
}

void play_controller::fire_start()
{
	gamestate().gamedata_.set_phase(game_data::START);
	pump().fire("start");

	skip_story_ = false; // Show [message]s from now on even with --campaign-skip-story

	// start event may modify start turn with WML, reflect any changes.
	gamestate().gamedata_.get_variable("turn_number") = static_cast<int>(turn());

	refresh_objectives();
	check_objectives();

	// prestart and start events may modify the initial gold amount,
	// reflect any changes.
	for(team& tm : get_teams()) {
		tm.set_start_gold(tm.gold());
	}

	gamestate().gamedata_.set_phase(game_data::TURN_STARTING_WAITING);
}

void play_controller::init_gui()
{
	gui_->begin_game();
	gui_->update_tod();
}

void play_controller::init_side_begin()
{
	mouse_handler_.set_side(current_side());
	gui_->set_playing_team(std::size_t(current_side() - 1));

	update_viewing_player();

	gamestate().gamedata_.last_selected = map_location::null_location();
}

void play_controller::maybe_do_init_side()
{
	//
	// We do side init only if not done yet for a local side when we are not replaying.
	// For all other sides it is recorded in replay and replay handler has to handle
	// calling do_init_side() functions.
	//
	if(is_during_turn()) {
		// We already executed do_init_side this can for example happe if we reload a game,
		// but also if we changed control of a side during it's turn
		return;
	}

	if(!current_team().is_local()) {
		// We are in a mp game and execute do_init_side as soon as we receive [init_side] from the current player
		// (see replay.cpp)
		return;
	}

	if(is_replay()) {
		// We are in a replay and execute do_init_side as soon as we reach the next [init_side] in the replay data
		// (see replay.cpp)
		return;
	}

	if(current_team().is_idle()) {
		// In this case it can happen that we just gave control of this side to another player so doing init_side
		// could lead to errors since we no longer own this side from the servers perspective.
		// (see playturn.cpp)
		return;
	}

	resources::recorder->init_side();
	do_init_side();
}

void play_controller::do_init_side()
{
	{ // Block for set_scontext_synced
		set_scontext_synced sync;

		synced_context::block_undo();

		log_scope("player turn");
		// In case we might end up calling sync:network during the side turn events,
		// and we don't want do_init_side to be called when a player drops.
		gamestate().gamedata_.set_phase(game_data::TURN_STARTING);
		gamestate_->next_player_number_ = gamestate_->player_number_ + 1;

		const std::string turn_num = std::to_string(turn());
		const std::string side_num = std::to_string(current_side());

		gamestate().gamedata_.get_variable("side_number") = current_side();

		// We might have skipped some sides because they were empty so it is not enough to check for side_num==1
		if(!gamestate().tod_manager_.has_turn_event_fired()) {
			pump().fire("turn_" + turn_num);
			pump().fire("new_turn");
			gamestate().tod_manager_.turn_event_fired();
		}

		pump().fire("side_turn");
		pump().fire("side_" + side_num + "_turn");
		pump().fire("side_turn_" + turn_num);
		pump().fire("side_" + side_num + "_turn_" + turn_num);

		// We want to work out if units for this player should get healed,
		// and the player should get income now.
		// Healing/income happen if it's not the first turn of processing,
		// or if we are loading a game.
		if(turn() > 1) {
			gamestate().board_.new_turn(current_side());
			current_team().new_turn();

			// If the expense is less than the number of villages owned
			// times the village support capacity,
			// then we don't have to pay anything at all
			int expense = gamestate().board_.side_upkeep(current_side()) - current_team().support();
			if(expense > 0) {
				current_team().spend_gold(expense);
			}
		}

		if(do_healing()) {
			calculate_healing(current_side(), !is_skipping_replay());
		}

		// Do healing on every side turn except the very first side turn.
		// (1.14 and earlier did healing whenever turn >= 2.)
		set_do_healing(true);

		// Set resting now after the healing has been done.
		for(unit& patient : resources::gameboard->units()) {
			if(patient.side() == current_side()) {
				patient.set_resting(true);
			}
		}

		// Prepare the undo stack.
		undo_stack().new_side_turn(current_side());

		pump().fire("turn_refresh");
		pump().fire("side_" + side_num + "_turn_refresh");
		pump().fire("turn_" + turn_num + "_refresh");
		pump().fire("side_" + side_num + "_turn_" + turn_num + "_refresh");

		// Make sure vision is accurate.
		actions::clear_shroud(current_side(), true);

		check_victory();
		sync.do_final_checkup();
		gamestate().gamedata_.set_phase(game_data::TURN_PLAYING);
	}

	statistics().reset_turn_stats(gamestate().board_.get_team(current_side()).save_id_or_number());

	init_side_end();

	if(!is_skipping_replay() && current_team().get_scroll_to_leader()) {
		gui_->scroll_to_leader(current_side(), game_display::ONSCREEN, false);
	}
}

void play_controller::init_side_end()
{
	if(	did_tod_sound_this_turn_) {
		did_tod_sound_this_turn_ = true;
		const time_of_day& tod = gamestate().tod_manager_.get_time_of_day();
		sound::play_sound(tod.sounds, sound::SOUND_SOURCES);
	}
	whiteboard_manager_->on_init_side();
}

config play_controller::to_config() const
{
	config cfg = level_;

	cfg["replay_pos"] = saved_game_.get_replay().get_pos();
	gamestate().write(cfg);

	gui_->write(cfg.add_child("display"));

	// Write the soundsources.
	soundsources_manager_->write_sourcespecs(cfg);

	gui_->labels().write(cfg);
	sound::write_music_play_list(cfg);

	if(cfg["replay_pos"].to_int(0) > 0 && cfg["playing_team"].empty()) {
		gui2::show_error_message(_("Trying to create a corrupt file, please report this bug"));
	}

	return cfg;
}

void play_controller::finish_side_turn_events()
{

	{ // Block for set_scontext_synced
		set_scontext_synced sync(1);
		// Also clears the undo stack.
		synced_context::block_undo();

		gamestate().board_.end_turn(current_side());
		const std::string turn_num = std::to_string(turn());
		const std::string side_num = std::to_string(current_side());

		// Clear shroud, in case units had been slowed for the turn.
		actions::clear_shroud(current_side());

		pump().fire("side_turn_end");
		pump().fire("side_" + side_num + "_turn_end");
		pump().fire("side_turn_" + turn_num + "_end");
		pump().fire("side_" + side_num + "_turn_" + turn_num + "_end");
		// This is where we refog, after all of a side's events are done.
		actions::recalculate_fog(current_side());
		check_victory();
		sync.do_final_checkup();
	}
	mouse_handler_.deselect_hex();
}

void play_controller::finish_turn()
{
	set_scontext_synced sync(2);
	const std::string turn_num = std::to_string(turn());
	pump().fire("turn_end");
	pump().fire("turn_" + turn_num + "_end");
	sync.do_final_checkup();
}

bool play_controller::enemies_visible() const
{
	// If we aren't using fog/shroud, this is easy :)
	if(current_team().uses_fog() == false && current_team().uses_shroud() == false) {
		return true;
	}

	// See if any enemies are visible
	for(const unit& u : get_units()) {
		if(current_team().is_enemy(u.side()) && !gui_->fogged(u.get_location())) {
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
	const unsigned int team_num = current_side();
	events::mouse_handler& mousehandler = mouse_handler_;

	switch(menu_handler_.get_textbox().mode()) {
	case gui::TEXTBOX_SEARCH:
		menu_handler_.do_search(str);
		menu_handler_.get_textbox().memorize_command(str);
		menu_handler_.get_textbox().close();
		break;
	case gui::TEXTBOX_MESSAGE:
		menu_handler_.do_speak();
		menu_handler_.get_textbox().close(); // need to close that one after executing do_speak() !
		break;
	case gui::TEXTBOX_COMMAND:
		menu_handler_.get_textbox().memorize_command(str);
		menu_handler_.get_textbox().close();
		menu_handler_.do_command(str);
		break;
	case gui::TEXTBOX_AI:
		menu_handler_.get_textbox().memorize_command(str);
		menu_handler_.get_textbox().close();
		menu_handler_.do_ai_formula(str, team_num, mousehandler);
		break;
	default:
		menu_handler_.get_textbox().close();
		ERR_DP << "unknown textbox mode";
	}
}

void play_controller::textbox_move_vertically(bool up)
{
	if(menu_handler_.get_textbox().active() == false) {
		return;
	}

	if(menu_handler_.get_textbox().mode() == gui::TEXTBOX_MESSAGE
		|| menu_handler_.get_textbox().mode() == gui::TEXTBOX_NONE) {
		// Not handling messages to avoid spam
		return;
	}

	const std::string str = menu_handler_.get_textbox().box()->text();
	const std::vector<std::string>& command_history = menu_handler_.get_textbox().command_history();

	auto prev = std::find(command_history.begin(), command_history.end(), str);

	if (prev != command_history.end())
	{
		if(up) {
			if(prev != command_history.begin()) {
				menu_handler_.get_textbox().box()->set_text(*--prev);
			}
		} else {
			if(++prev != command_history.end()) {
				menu_handler_.get_textbox().box()->set_text(*prev);
			} else {
				menu_handler_.get_textbox().box()->set_text("");
			}
		}
	} else if (up) {
		if(command_history.size() > 0) {
			menu_handler_.get_textbox().box()->set_text(*--prev);
		}
		if(!str.empty()) {
			menu_handler_.get_textbox().memorize_command(str);
		}
	}
}

void play_controller::tab()
{
	gui::TEXTBOX_MODE mode = menu_handler_.get_textbox().mode();

	std::set<std::string> dictionary;
	switch(mode) {
	case gui::TEXTBOX_SEARCH: {
		for(const unit& u : get_units()) {
			const map_location& loc = u.get_location();
			if(!gui_->fogged(loc) && !(get_teams()[gui_->viewing_team()].is_enemy(u.side()) && u.invisible(loc)))
				dictionary.insert(u.name());
		}
		// TODO List map labels
		break;
	}
	case gui::TEXTBOX_COMMAND: {
		std::vector<std::string> commands = menu_handler_.get_commands_list();
		dictionary.insert(commands.begin(), commands.end());
		[[fallthrough]]; // we also want player names from the next case
	}
	case gui::TEXTBOX_MESSAGE: {
		for(const team& t : get_teams()) {
			if(!t.is_empty())
				dictionary.insert(t.current_player());
		}

		// Add observers
		for(const std::string& o : gui_->observers()) {
			dictionary.insert(o);
		}

		// Add nicks who whispered you
		for(const std::string& w : gui_->get_chat_manager().whisperers()) {
			dictionary.insert(w);
		}

		// Add nicks from friendlist
		const std::map<std::string, std::string> friends = prefs::get().get_acquaintances_nice("friend");

		for(std::map<std::string, std::string>::const_iterator iter = friends.begin(); iter != friends.end(); ++iter) {
			dictionary.insert((*iter).first);
		}

		// Exclude own nick from tab-completion.
		// NOTE why ?
		dictionary.erase(prefs::get().login());
		break;
	}

	default:
		ERR_DP << "unknown textbox mode";
	} // switch(mode)

	menu_handler_.get_textbox().tab(dictionary);
}

team& play_controller::current_team()
{
	if(get_teams().size() == 0) {
		throw game::game_error("The scenario has no sides defined");
	}

	assert(gamestate().board_.has_team(current_side()));
	return gamestate().board_.get_team(current_side());
}

const team& play_controller::current_team() const
{
	if(get_teams().size() == 0) {
		throw game::game_error("The scenario has no sides defined");
	}

	assert(gamestate().board_.has_team(current_side()));
	return gamestate().board_.get_team(current_side());
}


events::mouse_handler& play_controller::get_mouse_handler_base()
{
	return mouse_handler_;
}

std::shared_ptr<wb::manager> play_controller::get_whiteboard() const
{
	return whiteboard_manager_;
}

const mp_game_settings& play_controller::get_mp_settings()
{
	return saved_game_.mp_settings();
}

game_classification& play_controller::get_classification()
{
	return saved_game_.classification();
}

game_display& play_controller::get_display()
{
	return *gui_;
}

bool play_controller::have_keyboard_focus()
{
	return !menu_handler_.get_textbox().active();
}

void play_controller::process_focus_keydown_event(const SDL_Event& event)
{
	if(event.key.keysym.sym == SDLK_ESCAPE) {
		menu_handler_.get_textbox().close();
	} else if(event.key.keysym.sym == SDLK_TAB) {
		tab();
	} else if(event.key.keysym.sym == SDLK_UP) {
		textbox_move_vertically(true);
	} else if(event.key.keysym.sym == SDLK_DOWN) {
		textbox_move_vertically(false);
	} else if(event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
		enter_textbox();
	}
}

void play_controller::process_keydown_event(const SDL_Event& event)
{
	if(event.key.keysym.sym == SDLK_TAB) {
		whiteboard_manager_->set_invert_behavior(true);
	}
}

void play_controller::process_keyup_event(const SDL_Event& event)
{
	// If the user has pressed 1 through 9, we want to show
	// how far the unit can move in that many turns
	if(event.key.keysym.sym >= '1' && event.key.keysym.sym <= '9') {
		const int new_path_turns = (event.type == SDL_KEYDOWN) ? event.key.keysym.sym - '1' : 0;

		if(new_path_turns != mouse_handler_.get_path_turns()) {
			mouse_handler_.set_path_turns(new_path_turns);

			const unit_map::iterator u = mouse_handler_.selected_unit();

			if(u.valid()) {
				// if it's not the unit's turn, we reset its moves
				unit_movement_resetter move_reset(*u, u->side() != current_side());

				mouse_handler_.set_current_paths(pathfind::paths(
					*u, false, true, get_teams()[gui_->viewing_team()], mouse_handler_.get_path_turns()));

				gui_->highlight_reach(mouse_handler_.current_paths());
			} else {
				mouse_handler_.select_hex(mouse_handler_.get_selected_hex(), false, false, false);
			}
		}
	} else if(event.key.keysym.sym == SDLK_TAB) {
		CKey keys;
		if(!keys[SDLK_TAB]) {
			whiteboard_manager_->set_invert_behavior(false);
		}
	}
}

replay& play_controller::get_replay()
{
	assert(replay_);
	return *replay_.get();
}

void play_controller::save_game()
{
	// Saving while an event is running isn't supported
	// because it may lead to expired event handlers being saved.
	assert(!gamestate().events_manager_->is_event_running());

	scoped_savegame_snapshot snapshot(*this);
	savegame::ingame_savegame save(saved_game_, prefs::get().save_compression_format());
	save.save_game_interactive("", savegame::savegame::OK_CANCEL);
}

void play_controller::save_game_auto(const std::string& filename)
{
	scoped_savegame_snapshot snapshot(*this);
	savegame::ingame_savegame save(saved_game_, prefs::get().save_compression_format());
	save.save_game_automatic(false, filename);
}

void play_controller::save_replay()
{
	savegame::replay_savegame save(saved_game_, prefs::get().save_compression_format());
	save.save_game_interactive("", savegame::savegame::OK_CANCEL);
}

void play_controller::save_replay_auto(const std::string& filename)
{
	savegame::replay_savegame save(saved_game_, prefs::get().save_compression_format());
	save.save_game_automatic(false, filename);
}

void play_controller::save_map()
{
	menu_handler_.save_map();
}

void play_controller::load_game()
{
	savegame::loadgame load(savegame::save_index_class::default_saves_dir(), saved_game_);
	load.load_game_ingame();
}

void play_controller::undo()
{
	mouse_handler_.deselect_hex();
	undo_stack().undo();
}

void play_controller::redo()
{
	mouse_handler_.deselect_hex();
	undo_stack().redo();
}

bool play_controller::can_undo() const
{
	return is_during_turn() && !is_browsing() && !events::commands_disabled && undo_stack().can_undo();
}

bool play_controller::can_redo() const
{
	return is_during_turn() && !is_browsing() && !events::commands_disabled && undo_stack().can_redo();
}

const std::string& play_controller::select_music(bool victory) const
{
	const std::vector<std::string>& music_list = victory
		? (gamestate_->get_game_data()->get_victory_music().empty()
			? game_config::default_victory_music
			: gamestate_->get_game_data()->get_victory_music())
		: (gamestate_->get_game_data()->get_defeat_music().empty()
			? game_config::default_defeat_music
			: gamestate_->get_game_data()->get_defeat_music());

	if(music_list.empty()) {
		// Since this function returns a reference, we can't return a temporary empty string.
		static const std::string empty_str = "";
		return empty_str;
	}

	return music_list[randomness::rng::default_instance().get_random_int(0, music_list.size() - 1)];
}

void play_controller::check_victory()
{
	if(is_linger_mode()) {
		return;
	}

	if(is_regular_game_end()) {
		return;
	}

	bool continue_level, found_player, found_network_player, invalidate_all;
	std::set<unsigned> not_defeated;

	gamestate().board_.check_victory(
		continue_level,
		found_player,
		found_network_player,
		invalidate_all,
		not_defeated,
		gamestate().remove_from_carryover_on_defeat_
	);

	if(invalidate_all) {
		gui_->invalidate_all();
	}

	if(continue_level) {
		return;
	}

	if(found_player || found_network_player) {
		pump().fire("enemies_defeated");
		if(is_regular_game_end()) {
			return;
		}
	}

	DBG_EE << "victory_when_enemies_defeated: " << gamestate().victory_when_enemies_defeated_;
	DBG_EE << "found_player: " << found_player;
	DBG_EE << "found_network_player: " << found_network_player;

	if(!gamestate().victory_when_enemies_defeated_ && (found_player || found_network_player)) {
		// This level has asked not to be ended by this condition.
		return;
	}

	if(video::headless()) {
		LOG_AIT << "winner: ";
		for(unsigned l : not_defeated) {
			std::string ai = ai::manager::get_singleton().get_active_ai_identifier_for_side(l);
			if(ai.empty())
				ai = "default ai";
			LOG_AIT << l << " (using " << ai << ") ";
		}

		LOG_AIT;
		ai_testing::log_victory(not_defeated);
	}

	DBG_EE << "throwing end level exception...";
	// Also proceed to the next scenario when another player survived.
	end_level_data el_data;
	el_data.transient.reveal_map = reveal_map_default();
	el_data.proceed_to_next_level = found_player || found_network_player;
	el_data.is_victory = found_player;
	set_end_level_data(el_data);
}

void play_controller::process_oos(const std::string& msg) const
{
	if(video::headless()) {
		throw game::game_error(msg);
	}

	if(game_config::ignore_replay_errors) {
		return;
	}

	std::stringstream message;
	message << _("The game is out of sync. It might not make much sense to continue. Do you want to save your game?");
	message << "\n\n" << _("Error details:") << "\n\n" << msg;

	scoped_savegame_snapshot snapshot(*this);
	savegame::oos_savegame save(saved_game_, ignore_replay_errors_);
	save.save_game_interactive(message.str(), savegame::savegame::YES_NO); // can throw quit_game_exception
}

bool play_controller::reveal_map_default() const
{
	return saved_game_.classification().get_tagname() == "multiplayer";
}

void play_controller::update_gui_to_player(const int team_index, const bool observe)
{
	gui_->set_team(team_index, observe);
	gui_->recalculate_minimap();
	gui_->invalidate_all();
}

void play_controller::do_autosave()
{
	scoped_savegame_snapshot snapshot(*this);
	savegame::autosave_savegame save(saved_game_, prefs::get().save_compression_format());
	save.autosave(false, prefs::get().auto_save_max(), pref_constants::INFINITE_AUTO_SAVES);
}

void play_controller::do_consolesave(const std::string& filename)
{
	scoped_savegame_snapshot snapshot(*this);
	savegame::ingame_savegame save(saved_game_, prefs::get().save_compression_format());
	save.save_game_automatic(true, filename);
}

void play_controller::update_savegame_snapshot() const
{
	// note: this writes to level_ if this is not a replay.
	saved_game_.set_snapshot(to_config());
}

game_events::wml_event_pump& play_controller::pump()
{
	return gamestate().events_manager_->pump();
}

int play_controller::get_ticks() const
{
	return ticks_;
}

soundsource::manager* play_controller::get_soundsource_man()
{
	return soundsources_manager_.get();
}

plugins_context* play_controller::get_plugins_context()
{
	return plugins_context_.get();
}

hotkey::command_executor* play_controller::get_hotkey_command_executor()
{
	return hotkey_handler_.get();
}

bool play_controller::is_browsing() const
{
	if(!gamestate().in_phase(game_data::TURN_PLAYING)) {
		return true;
	}

	const team& t = current_team();
	return !t.is_local_human() || !t.is_proxy_human();
}

void play_controller::play_slice_catch()
{
	if(should_return_to_play_side()) {
		return;
	}

	try {
		play_slice();
	} catch(const return_to_play_side_exception&) {
		assert(should_return_to_play_side());
	}
}

void play_controller::start_game()
{
	if(gamestate().in_phase(game_data::PRELOAD)) {
		resources::recorder->add_start_if_not_there_yet();
		resources::recorder->get_next_action();

		set_scontext_synced sync;

		// So that the code knows it can send choices immidiateley
		// todo: im not sure whetrh this is actually needed.
		synced_context::block_undo();
		fire_prestart();
		if(is_regular_game_end()) {
			return;
		}

		for(const team& t : get_teams()) {
			actions::clear_shroud(t.side(), false, false);
		}

		init_gui();
		LOG_NG << "first_time..." << (is_skipping_replay() ? "skipping" : "no skip");

		fire_start();
		if(is_regular_game_end()) {
			return;
		}

		sync.do_final_checkup();
		gui_->recalculate_minimap();

		// Initialize countdown clock.
		for(const team& t : get_teams()) {
			if(saved_game_.mp_settings().mp_countdown) {
				t.set_countdown_time(1000 * saved_game_.mp_settings().mp_countdown_init_time);
			}
		}
		did_autosave_this_turn_ = false;
	} else {
		init_gui();
		gui_->recalculate_minimap();
	}

	check_next_scenario_is_known();
}

/**
 * Find all [endlevel]next_scenario= attributes, and add them to @a result.
 */
static void find_next_scenarios(const config& parent, std::set<std::string>& result) {
	for(const auto& endlevel : parent.child_range("endlevel")) {
		if(endlevel.has_attribute("next_scenario")) {
			result.insert(endlevel["next_scenario"]);
		}
	}
	for(const auto cfg : parent.all_children_range()) {
		find_next_scenarios(cfg.cfg, result);
	}
};

void play_controller::check_next_scenario_is_known() {
	// Which scenarios are reachable from the current one?
	std::set<std::string> possible_next_scenarios;
	possible_next_scenarios.insert(gamestate().gamedata_.next_scenario());

	// Find all "endlevel" tags that could be triggered in events
	config events;
	gamestate().events_manager_->write_events(events);
	find_next_scenarios(events, possible_next_scenarios);

	// Are we looking for [scenario]id=, [multiplayer]id= or [test]id=?
	const auto tagname = saved_game_.classification().get_tagname();

	// Of the possible routes, work out which exist.
	bool possible_this_is_the_last_scenario = false;
	std::vector<std::string> known;
	std::vector<std::string> unknown;
	for(const auto& x : possible_next_scenarios) {
		if(x.empty() || x == "null") {
			possible_this_is_the_last_scenario = true;
			LOG_NG << "This can be the last scenario";
		} else if(utils::contains(x, '$')) {
			// Assume a WML variable will be set to a correct value before the end of the scenario
			known.push_back(x);
			LOG_NG << "Variable value for next scenario '" << x << "'";
		} else if(game_config_.find_child(tagname, "id", x)) {
			known.push_back(x);
			LOG_NG << "Known next scenario '" << x << "'";
		} else {
			unknown.push_back(x);
			ERR_NG << "Unknown next scenario '" << x << "'";
		}
	}

	if(unknown.empty()) {
		// everything's good
		return;
	}

	std::string title = _("Warning: broken campaign branches");
	std::stringstream message;

	message << _n(
		// TRANSLATORS: This is an error that will hopefully only be seen by UMC authors and by players who have already
		// said "okay" to a "loading saves from an old version might not work" dialog.
		"The next scenario is missing, you will not be able to finish this campaign.",
		// TRANSLATORS: This is an error that will hopefully only be seen by UMC authors and by players who have already
		// said "okay" to a "loading saves from an old version might not work" dialog.
		"Some of the possible next scenarios are missing, you might not be able to finish this campaign.",
		unknown.size() + known.size() + (possible_this_is_the_last_scenario ? 1 : 0));
	message << "\n\n";
	message << _n(
		"Please report the following missing scenario to the campaign’s author:\n$unknown_list|",
		"Please report the following missing scenarios to the campaign’s author:\n$unknown_list|",
		unknown.size());
	message << "\n";
	message << _("Once this is fixed, you will need to restart this scenario.");

	std::stringstream unknown_list;
	for(const auto& x : unknown) {
		unknown_list << font::unicode_bullet << " " << x << "\n";
	}
	utils::string_map symbols;
	symbols["unknown_list"] = unknown_list.str();
	auto message_str = utils::interpolate_variables_into_string(message.str(), &symbols);
	ERR_NG << message_str;
	gui2::show_message(title, message_str, gui2::dialogs::message::close_button);
}

bool play_controller::can_use_synced_wml_menu() const
{
	const team& viewing_team = get_teams()[gui_->viewing_team()];
	return gui_->viewing_team() == gui_->playing_team() && !events::commands_disabled && viewing_team.is_local_human()
		&& !is_browsing();
}

std::set<std::string> play_controller::all_players() const
{
	std::set<std::string> res = gui_->observers();
	for(const team& t : get_teams()) {
		if(t.is_human()) {
			res.insert(t.current_player());
		}
	}

	return res;
}

void play_controller::check_time_over()
{
	const bool time_left = gamestate().tod_manager_.next_turn(&gamestate().gamedata_);

	if(!time_left) {
		LOG_NG << "firing time over event...";
		set_scontext_synced_base sync;
		pump().fire("time_over");
		LOG_NG << "done firing time over event...";

		// If turns are added while handling 'time over' event.
		if(gamestate().tod_manager_.is_time_left()) {
			return;
		}

		if(video::headless()) {
			LOG_AIT << "time over (draw)";
			ai_testing::log_draw();
		}

		check_victory();
		if(is_regular_game_end()) {
			return;
		}

		end_level_data e;
		e.transient.reveal_map = reveal_map_default();
		e.proceed_to_next_level = false;
		e.is_victory = false;
		set_end_level_data(e);
	}
}

play_controller::scoped_savegame_snapshot::scoped_savegame_snapshot(const play_controller& controller)
	: controller_(controller)
{
	controller_.update_savegame_snapshot();
}

play_controller::scoped_savegame_snapshot::~scoped_savegame_snapshot()
{
	controller_.saved_game_.remove_snapshot();
}

void play_controller::show_objectives() const
{
	const team& t = get_teams()[gui_->viewing_team()];
	static const std::string no_objectives(_("No objectives available"));
	std::string objectives = utils::interpolate_variables_into_string(t.objectives(), *gamestate_->get_game_data());
	gui2::show_transient_message(get_scenario_name(), (objectives.empty() ? no_objectives : objectives), "", true);
	t.reset_objectives_changed();
}

void play_controller::toggle_skipping_replay()
{
	skip_replay_ = !skip_replay_;
	const std::shared_ptr<gui::button> skip_animation_button = get_display().find_action_button("skip-animation");
	if(skip_animation_button) {
		skip_animation_button->set_check(skip_replay_);
	}
}

bool play_controller::is_during_turn() const
{
	return gamestate().in_phase(game_data::TURN_PLAYING);
}

bool play_controller::is_linger_mode() const
{
	return gamestate().in_phase(game_data::GAME_ENDED);
}

void play_controller::maybe_throw_return_to_play_side() const
{
	if(should_return_to_play_side() && is_during_turn()) {
		throw return_to_play_side_exception();
	}
}
