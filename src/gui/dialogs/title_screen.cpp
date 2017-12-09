/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/title_screen.hpp"

#include "addon/manager_ui.hpp"
#include "formatter.hpp"
#include "game_config.hpp"
#include "game_config_manager.hpp"
#include "game_launcher.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/auxiliary/tips.hpp"
#include "gui/core/timer.hpp"
#include "gui/dialogs/core_selection.hpp"
#include "gui/dialogs/debug_clock.hpp"
#include "gui/dialogs/game_version.hpp"
#include "gui/dialogs/help_browser.hpp"
#include "gui/dialogs/language_selection.hpp"
#include "gui/dialogs/lua_interpreter.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/multiplayer/mp_host_game_prompt.hpp"
#include "gui/dialogs/multiplayer/mp_method_selection.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
#include "log.hpp"
#include "preferences/game.hpp"
//#define DEBUG_TOOLTIP
#ifdef DEBUG_TOOLTIP
#include "gui/dialogs/tooltip.hpp"
#endif
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/multi_page.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "help/help.hpp"
#include "hotkey/hotkey_command.hpp"
#include "video.hpp"
#include "utils/functional.hpp"

#include <algorithm>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
#define WRN_CF LOG_STREAM(warn, log_config)

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_title_screen
 *
 * == Title screen ==
 *
 * This shows the title screen.
 *
 * @begin{table}{dialog_widgets}
 * tutorial & & button & m &
 *         The button to start the tutorial. $
 *
 * campaign & & button & m &
 *         The button to start a campaign. $
 *
 * multiplayer & & button & m &
 *         The button to start multiplayer mode. $
 *
 * load & & button & m &
 *         The button to load a saved game. $
 *
 * editor & & button & m &
 *         The button to start the editor. $
 *
 * addons & & button & m &
 *         The button to start managing the addons. $
 *
 * cores & & button & m &
 *         The button to start managing the cores. $
 *
 * language & & button & m &
 *         The button to select the game language. $
 *
 * credits & & button & m &
 *         The button to show Wesnoth's contributors. $
 *
 * quit & & button & m &
 *         The button to quit Wesnoth. $
 *
 * tips & & multi_page & m &
 *         A multi_page to hold all tips, when this widget is used the area of
 *         the tips doesn't need to be resized when the next or previous button
 *         is pressed. $
 *
 * -tip & & label & o &
 *         Shows the text of the current tip. $
 *
 * -source & & label & o &
 *         The source (the one who's quoted or the book referenced) of the
 *         current tip. $
 *
 * next_tip & & button & m &
 *         The button show the next tip of the day. $
 *
 * previous_tip & & button & m &
 *         The button show the previous tip of the day. $
 *
 * logo & & image & o &
 *         The Wesnoth logo. $
 *
 * revision_number & & styled_widget & o &
 *         A widget to show the version number when the version number is
 *         known. $
 *
 * @end{table}
 */

REGISTER_DIALOG(title_screen)

bool show_debug_clock_button = false;

title_screen::title_screen(game_launcher& game)
	: debug_clock_()
	, game_(game)
{
	set_restore(false);

	// Need to set this in the constructor, pre_show() / post_build() is too late
	set_allow_plugin_skip(false);
}

title_screen::~title_screen()
{
}

using btn_callback = std::function<void()>;

static void register_button(window& win, const std::string& id, hotkey::HOTKEY_COMMAND hk, btn_callback callback)
{
	if(hk != hotkey::HOTKEY_NULL) {
		win.register_hotkey(hk, std::bind(callback));
	}

	connect_signal_mouse_left_click(find_widget<button>(&win, id, false), std::bind(callback));
}

static void launch_lua_console()
{
	gui2::dialogs::lua_interpreter::display(gui2::dialogs::lua_interpreter::APP);
}

#ifdef DEBUG_TOOLTIP
/*
 * This function is used to test the tooltip placement algorithms as
 * described in the »Tooltip placement« section in the GUI2 design
 * document.
 *
 * Use a 1024 x 768 screen size, set the maximum loop iteration to:
 * - 0   to test with a normal tooltip placement.
 * - 30  to test with a larger normal tooltip placement.
 * - 60  to test with a huge tooltip placement.
 * - 150 to test with a borderline to insanely huge tooltip placement.
 * - 180 to test with an insanely huge tooltip placement.
 */
static void debug_tooltip(window& /*window*/, bool& handled, const point& coordinate)
{
	std::string message = "Hello world.";

	for(int i = 0; i < 0; ++i) {
		message += " More greetings.";
	}

	gui2::tip::remove();
	gui2::tip::show("tooltip", message, coordinate);

	handled = true;
}
#endif

void title_screen::pre_show(window& win)
{
	win.set_click_dismiss(false);
	win.set_enter_disabled(true);
	win.set_escape_disabled(true);

#ifdef DEBUG_TOOLTIP
	win.connect_signal<event::SDL_MOUSE_MOTION>(
			std::bind(debug_tooltip, std::ref(win), _3, _5),
			event::dispatcher::front_child);
#endif

	win.connect_signal<event::SDL_VIDEO_RESIZE>(std::bind(&title_screen::on_resize, this, std::ref(win)));

	//
	// General hotkeys
	//
	win.register_hotkey(hotkey::TITLE_SCREEN__RELOAD_WML,
		std::bind(&gui2::window::set_retval, std::ref(win), RELOAD_GAME_DATA, true));

	win.register_hotkey(hotkey::TITLE_SCREEN__TEST,
		std::bind(&title_screen::hotkey_callback_select_tests, this, std::ref(win)));

	win.register_hotkey(hotkey::HOTKEY_FULLSCREEN,
		std::bind(&CVideo::toggle_fullscreen, std::ref(win.video())));

	// A wrapper is needed here since the relevant display function is overloaded, and
	// since the wrapper's signature doesn't exactly match what register_hotkey expects.
	win.register_hotkey(hotkey::LUA_CONSOLE, std::bind(&launch_lua_console));

	//
	// Background and logo images
	//
	if(game_config::images::game_title.empty()) {
		ERR_CF << "No title image defined" << std::endl;
	}

	win.get_canvas(0).set_variable("title_image", wfl::variant(game_config::images::game_title));

	if(game_config::images::game_title_background.empty()) {
		ERR_CF << "No title background image defined" << std::endl;
	}

	win.get_canvas(0).set_variable("background_image", wfl::variant(game_config::images::game_title_background));

	find_widget<image>(&win, "logo-bg", false).set_image(game_config::images::game_logo_background);
	find_widget<image>(&win, "logo", false).set_image(game_config::images::game_logo);

	//
	// Version string
	//
	const std::string version_string = formatter() << ("Version") << " " << game_config::revision;

	if(label* version_label = find_widget<label>(&win, "revision_number", false, false)) {
		version_label->set_label(version_string);
	}

	win.get_canvas(0).set_variable("revision_number", wfl::variant(version_string));

	//
	// Tip-of-the-day browser
	//
	multi_page& tip_pages = find_widget<multi_page>(&win, "tips", false);

	std::vector<game_tip> tips = tip_of_the_day::shuffle(settings::tips);
	if(tips.empty()) {
		WRN_CF << "There are no tips of day available." << std::endl;
	}

	for(const auto& tip : tips)	{
		string_map widget;
		std::map<std::string, string_map> page;

		widget["use_markup"] = "true";

		widget["label"] = tip.text();
		page.emplace("tip", widget);

		widget["label"] = tip.source();
		page.emplace("source", widget);

		tip_pages.add_page(page);
	}

	update_tip(win, true);

	register_button(win, "next_tip", hotkey::TITLE_SCREEN__NEXT_TIP,
		std::bind(&title_screen::update_tip, this, std::ref(win), true));

	register_button(win, "previous_tip", hotkey::TITLE_SCREEN__PREVIOUS_TIP,
		std::bind(&title_screen::update_tip, this, std::ref(win), false));

	//
	// Help
	//
	register_button(win, "help", hotkey::HOTKEY_HELP, []() {
		if(gui2::new_widgets) {
			gui2::dialogs::help_browser::display();
		}

		help::help_manager help_manager(&game_config_manager::get()->game_config());
		help::show_help();
	});

	//
	// About
	//
	register_button(win, "about", hotkey::HOTKEY_NULL, std::bind(&game_version::display<>));

	//
	// Tutorial
	//
	register_button(win, "tutorial", hotkey::TITLE_SCREEN__TUTORIAL, [this, &win]() {
		game_.set_tutorial();
		win.set_retval(LAUNCH_GAME);
	});

	//
	// Campaign
	//
	register_button(win, "campaign", hotkey::TITLE_SCREEN__CAMPAIGN, [this, &win]() {
		try{
			if(game_.new_campaign()) {
				win.set_retval(LAUNCH_GAME);
			}
		} catch (const config::error& e) {
			gui2::show_error_message(e.what());
		}
	});

	//
	// Multiplayer
	//
	register_button(win, "multiplayer", hotkey::TITLE_SCREEN__MULTIPLAYER,
		std::bind(&title_screen::button_callback_multiplayer, this, std::ref(win)));

	//
	// Load game
	//
	register_button(win, "load", hotkey::HOTKEY_LOAD_GAME, [this, &win]() {
		if(game_.load_game()) {
			win.set_retval(LAUNCH_GAME);
		} else {
			game_.clear_loaded_game();
		}
	});

	//
	// Addons
	//
	register_button(win, "addons", hotkey::TITLE_SCREEN__ADDONS, []() {
		// NOTE: we need the help_manager to get access to the Add-ons section in the game help!
		help::help_manager help_manager(&game_config_manager::get()->game_config());

		if(manage_addons()) {
			game_config_manager::get()->reload_changed_game_config();
		}
	});

	//
	// Editor
	//
	register_button(win, "editor", hotkey::TITLE_SCREEN__EDITOR, [&win]() { win.set_retval(MAP_EDITOR); });

	//
	// Cores
	//
	register_button(win, "cores", hotkey::TITLE_SCREEN__CORES,
		std::bind(&title_screen::button_callback_cores, this));

	if(game_config_manager::get()->game_config().child_range("core").size() <= 1) {
		find_widget<button>(&win, "cores", false).set_visible(widget::visibility::invisible);
	}

	//
	// Language
	//
	register_button(win, "language", hotkey::HOTKEY_LANGUAGE, [this, &win]() {
		try {
			if(game_.change_language()) {
				t_string::reset_translations();
				::image::flush_cache();
				on_resize(win);
			}
		} catch(std::runtime_error& e) {
			gui2::show_error_message(e.what());
		}
	});

	//
	// Preferences
	//
	register_button(win, "preferences", hotkey::HOTKEY_PREFERENCES, [this]() { game_.show_preferences(); });

	//
	// Credits
	//
	register_button(win, "credits", hotkey::TITLE_SCREEN__CREDITS, [&win]() { win.set_retval(SHOW_ABOUT); });

	//
	// Quit
	//
	register_button(win, "quit", hotkey::HOTKEY_QUIT_TO_DESKTOP, [&win]() { win.set_retval(QUIT_GAME); });

	//
	// Debug clock
	//
	register_button(win, "clock", hotkey::HOTKEY_NULL,
		std::bind(&title_screen::show_debug_clock_window, this));

	find_widget<button>(&win, "clock", false).set_visible(show_debug_clock_button
		? widget::visibility::visible
		: widget::visibility::invisible);
}

void title_screen::on_resize(window& win)
{
	win.set_retval(REDRAW_BACKGROUND);
}

void title_screen::update_tip(window& win, const bool previous)
{
	multi_page& tips = find_widget<multi_page>(&win, "tips", false);
	if(tips.get_page_count() == 0) {
		return;
	}

	int page = tips.get_selected_page();
	if(previous) {
		if(page <= 0) {
			page = tips.get_page_count();
		}
		--page;
	} else {
		++page;
		if(static_cast<unsigned>(page) >= tips.get_page_count()) {
			page = 0;
		}
	}

	tips.select_page(page);

	/**
	 * @todo Look for a proper fix.
	 *
	 * This dirtying is required to avoid the blurring to be rendered wrong.
	 * Not entirely sure why, but since we plan to move to SDL2 that change
	 * will probably fix this issue automatically.
	 */
	win.set_is_dirty(true);
}

void title_screen::show_debug_clock_window()
{
	assert(show_debug_clock_button);

	if(debug_clock_) {
		debug_clock_.reset(nullptr);
	} else {
		debug_clock_.reset(new debug_clock());
		debug_clock_->show(true);
	}
}

void title_screen::hotkey_callback_select_tests(window& window)
{
	game_config_manager::get()->load_game_config_for_create(false, true);

	std::vector<std::string> options;
	for(const config &sc : game_config_manager::get()->game_config().child_range("test")) {
		if(!sc["is_unit_test"].to_bool(false)) {
			options.emplace_back(sc["id"]);
		}
	}

	std::sort(options.begin(), options.end());

	gui2::dialogs::simple_item_selector dlg(_("Choose Test"), "", options);
	dlg.show();

	int choice = dlg.selected_index();
	if(choice >= 0) {
		game_.set_test(options[choice]);
		window.set_retval(LAUNCH_GAME);
	}
}

void title_screen::button_callback_multiplayer(window& window)
{
	while(true) {
		gui2::dialogs::mp_method_selection dlg;
		dlg.show();

		if(dlg.get_retval() != gui2::window::OK) {
			return;
		}

		const int res = dlg.get_choice();

		if(res == 2 && preferences::mp_server_warning_disabled() < 2) {
			if(!gui2::dialogs::mp_host_game_prompt::execute()) {
				continue;
			}
		}

		switch(res) {
		case 0:
			game_.select_mp_server(preferences::server_list().front().address);
			window.set_retval(MP_CONNECT);
			break;
		case 1:
			game_.select_mp_server("");
			window.set_retval(MP_CONNECT);
			break;
		case 2:
			game_.select_mp_server("localhost");
			window.set_retval(MP_HOST);
			break;
		case 3:
			window.set_retval(MP_LOCAL);
			break;
		}

		return;
	}
}

void title_screen::button_callback_cores()
{
	int current = 0;

	std::vector<config> cores;
	for(const config& core : game_config_manager::get()->game_config().child_range("core")) {
		cores.push_back(core);

		if(core["id"] == preferences::core_id()) {
			current = cores.size() - 1;
		}
	}

	gui2::dialogs::core_selection core_dlg(cores, current);
	if(core_dlg.show()) {
		const std::string& core_id = cores[core_dlg.get_choice()]["id"];

		preferences::set_core_id(core_id);
		game_config_manager::get()->reload_changed_game_config();
	}
}

} // namespace dialogs
} // namespace gui2
