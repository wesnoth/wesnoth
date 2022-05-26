/*
	Copyright (C) 2008 - 2022
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "filesystem.hpp"
#include "formula/string_utils.hpp"
#include "game_config.hpp"
#include "game_config_manager.hpp"
#include "game_launcher.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/auxiliary/tips.hpp"
#include "gui/core/timer.hpp"
#include "gui/dialogs/core_selection.hpp"
#include "gui/dialogs/debug_clock.hpp"
#include "gui/dialogs/game_version_dialog.hpp"
#include "gui/dialogs/help_browser.hpp"
#include "gui/dialogs/language_selection.hpp"
#include "gui/dialogs/lua_interpreter.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/multiplayer/mp_host_game_prompt.hpp"
#include "gui/dialogs/multiplayer/mp_method_selection.hpp"
#include "gui/dialogs/preferences_dialog.hpp"
#include "gui/dialogs/screenshot_notification.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
#include "language.hpp"
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
#include "sdl/surface.hpp"
#include "sdl/utils.hpp"
#include "video.hpp"

#include <algorithm>
#include <functional>

#include <boost/algorithm/string/erase.hpp>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
#define WRN_CF LOG_STREAM(warn, log_config)

namespace gui2::dialogs
{

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

	auto b = find_widget<button>(&win, id, false, false);
	if(b != nullptr)
	{
		connect_signal_mouse_left_click(*b, std::bind(callback));
	}
}

static void launch_lua_console()
{
	gui2::dialogs::lua_interpreter::display(gui2::dialogs::lua_interpreter::APP);
}

static void make_screenshot()
{
	surface screenshot = CVideo::get_singleton().read_pixels();
	if(screenshot) {
		std::string filename = filesystem::get_screenshot_dir() + "/" + _("Screenshot") + "_";
		filename = filesystem::get_next_filename(filename, ".jpg");
		gui2::dialogs::screenshot_notification::display(filename, screenshot);
	}
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
			std::bind(debug_tooltip, std::ref(win), std::placeholders::_3, std::placeholders::_5),
			event::dispatcher::front_child);
#endif

	win.connect_signal<event::SDL_VIDEO_RESIZE>(std::bind(&title_screen::on_resize, this));

	//
	// General hotkeys
	//
	win.register_hotkey(hotkey::TITLE_SCREEN__RELOAD_WML,
		std::bind(&gui2::window::set_retval, std::ref(win), RELOAD_GAME_DATA, true));

	win.register_hotkey(hotkey::TITLE_SCREEN__TEST,
		std::bind(&title_screen::hotkey_callback_select_tests, this));

	// A wrapper is needed here since the relevant display function is overloaded, and
	// since the wrapper's signature doesn't exactly match what register_hotkey expects.
	win.register_hotkey(hotkey::LUA_CONSOLE, std::bind(&launch_lua_console));

	win.register_hotkey(hotkey::HOTKEY_SCREENSHOT, std::bind(&make_screenshot));

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
	const std::string& version_string = VGETTEXT("Version $version", {{ "version", game_config::revision }});

	if(label* version_label = find_widget<label>(&win, "revision_number", false, false)) {
		version_label->set_label(version_string);
	}

	win.get_canvas(0).set_variable("revision_number", wfl::variant(version_string));

	//
	// Tip-of-the-day browser
	//
	multi_page* tip_pages = find_widget<multi_page>(&win, "tips", false, false);

	if(tip_pages != nullptr) {
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

			tip_pages->add_page(page);
		}

		update_tip(true);
	}

	register_button(win, "next_tip", hotkey::TITLE_SCREEN__NEXT_TIP,
		std::bind(&title_screen::update_tip, this, true));

	register_button(win, "previous_tip", hotkey::TITLE_SCREEN__PREVIOUS_TIP,
		std::bind(&title_screen::update_tip, this, false));

	//
	// Help
	//
	register_button(win, "help", hotkey::HOTKEY_HELP, []() {
		if(gui2::new_widgets) {
			gui2::dialogs::help_browser::display();
		}

		help::show_help();
	});

	//
	// About
	//
	register_button(win, "about", hotkey::HOTKEY_NULL, std::bind(&game_version::display<>));

	//
	// Campaign
	//
	register_button(win, "campaign", hotkey::TITLE_SCREEN__CAMPAIGN, [this, &win]() {
		try{
			if(game_.new_campaign()) {
				// Suspend drawing of the title screen,
				// so it doesn't flicker in between loading screens.
				win.set_suspend_drawing(true);
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
		std::bind(&title_screen::button_callback_multiplayer, this));

	//
	// Load game
	//
	register_button(win, "load", hotkey::HOTKEY_LOAD_GAME, [this, &win]() {
		if(game_.load_game()) {
			// Suspend drawing of the title screen,
			// so it doesn't flicker in between loading screens.
			win.set_suspend_drawing(true);
			win.set_retval(LAUNCH_GAME);
		}
	});

	//
	// Addons
	//
	register_button(win, "addons", hotkey::TITLE_SCREEN__ADDONS, [&win]() {
		if(manage_addons()) {
			win.set_retval(RELOAD_GAME_DATA);
		}
	});

	//
	// Editor
	//
	register_button(win, "editor", hotkey::TITLE_SCREEN__EDITOR, [&win]() { win.set_retval(MAP_EDITOR); });

	//
	// Cores
	//
	win.register_hotkey(hotkey::TITLE_SCREEN__CORES,
		std::bind(&title_screen::button_callback_cores, this));

	//
	// Language
	//
	register_button(win, "language", hotkey::HOTKEY_LANGUAGE, [this]() {
		try {
			if(game_.change_language()) {
				on_resize();
			}
		} catch(const std::runtime_error& e) {
			gui2::show_error_message(e.what());
		}
	});

	if(auto* lang_button = find_widget<button>(&win, "language", false, false); lang_button) {
		const auto& locale = translation::get_effective_locale_info();
		// Just assume everything is UTF-8 (it should be as long as we're called Wesnoth)
		// and strip the charset from the Boost locale identifier.
		const auto& boost_name = boost::algorithm::erase_first_copy(locale.name(), ".UTF-8");
		const auto& langs = get_languages(true);

		auto lang_def = std::find_if(langs.begin(), langs.end(), [&](language_def const& lang) {
			return lang.localename == boost_name;
		});

		if(lang_def != langs.end()) {
			lang_button->set_label(lang_def->language.str());
		} else if(boost_name == "c" || boost_name == "C") {
			// HACK: sometimes System Default doesn't match anything on the list. If you fork
			// Wesnoth and change the neutral language to something other than US English, you
			// want to change this too.
			lang_button->set_label("English (US)");
		} else {
			// If somehow the locale doesn't match a known translation, use the
			// locale identifier as a last resort
			lang_button->set_label(boost_name);
		}
	}

	//
	// Preferences
	//
	register_button(win, "preferences", hotkey::HOTKEY_PREFERENCES, []() {
		gui2::dialogs::preferences_dialog::display();
	});

	//
	// Credits
	//
	register_button(win, "credits", hotkey::TITLE_SCREEN__CREDITS, [&win]() { win.set_retval(SHOW_ABOUT); });

	//
	// Quit
	//
	register_button(win, "quit", hotkey::HOTKEY_QUIT_TO_DESKTOP, [&win]() { win.set_retval(QUIT_GAME); });
	// A sanity check, exit immediately if the .cfg file didn't have a "quit" button.
	find_widget<button>(&win, "quit", false, true);

	//
	// Debug clock
	//
	register_button(win, "clock", hotkey::HOTKEY_NULL,
		std::bind(&title_screen::show_debug_clock_window, this));

	auto clock = find_widget<button>(&win, "clock", false, false);
	if(clock) {
		clock->set_visible(show_debug_clock_button ? widget::visibility::visible : widget::visibility::invisible);
	}
}

void title_screen::on_resize()
{
	set_retval(REDRAW_BACKGROUND);
}

void title_screen::update_tip(const bool previous)
{
	multi_page* tip_pages = find_widget<multi_page>(get_window(), "tips", false, false);
	if(tip_pages == nullptr) {
		return;
	}
	if(tip_pages->get_page_count() == 0) {
		return;
	}

	int page = tip_pages->get_selected_page();
	if(previous) {
		if(page <= 0) {
			page = tip_pages->get_page_count();
		}
		--page;
	} else {
		++page;
		if(static_cast<unsigned>(page) >= tip_pages->get_page_count()) {
			page = 0;
		}
	}

	tip_pages->select_page(page);

	/**
	 * @todo Look for a proper fix.
	 *
	 * This dirtying is required to avoid the blurring to be rendered wrong.
	 * Not entirely sure why, but since we plan to move to SDL2 that change
	 * will probably fix this issue automatically.
	 */
	get_window()->set_is_dirty(true);
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

void title_screen::hotkey_callback_select_tests()
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
		set_retval(LAUNCH_GAME);
	}
}

void title_screen::button_callback_multiplayer()
{
	while(true) {
		gui2::dialogs::mp_method_selection dlg;
		dlg.show();

		if(dlg.get_retval() != gui2::retval::OK) {
			return;
		}

		const auto res = dlg.get_choice();

		if(res == decltype(dlg)::choice::HOST && preferences::mp_server_warning_disabled() < 2) {
			if(!gui2::dialogs::mp_host_game_prompt::execute()) {
				continue;
			}
		}

		switch(res) {
		case decltype(dlg)::choice::JOIN:
			game_.select_mp_server(preferences::builtin_servers_list().front().address);
			get_window()->set_retval(MP_CONNECT);
			break;
		case decltype(dlg)::choice::CONNECT:
			game_.select_mp_server("");
			get_window()->set_retval(MP_CONNECT);
			break;
		case decltype(dlg)::choice::HOST:
			game_.select_mp_server("localhost");
			get_window()->set_retval(MP_HOST);
			break;
		case decltype(dlg)::choice::LOCAL:
			get_window()->set_retval(MP_LOCAL);
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
		get_window()->set_retval(RELOAD_GAME_DATA);
	}
}

} // namespace dialogs
