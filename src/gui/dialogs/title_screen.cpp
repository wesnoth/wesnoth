/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

#include "game_config.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/timer.hpp"
#include "gui/core/tips.hpp"
#include "gui/dialogs/debug_clock.hpp"
#include "gui/dialogs/game_version.hpp"
#include "gui/dialogs/language_selection.hpp"
#include "gui/dialogs/lua_interpreter.hpp"
//#define DEBUG_TOOLTIP
#ifdef DEBUG_TOOLTIP
#include "gui/dialogs/tip.hpp"
#endif
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/multi_page.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "utils/foreach.hpp"
#include "video.hpp"

#include <boost/bind.hpp>

#include <algorithm>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
#define WRN_CF LOG_STREAM(warn, log_config)

static lg::log_domain log_general("general");
#define ERR_GEN LOG_STREAM(err, log_general)

namespace gui2
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
 * logo & & progress_bar & o &
 *         A progress bar to "animate" the Wesnoth logo. $
 *
 * revision_number & & control & o &
 *         A widget to show the version number when the version number is
 *         known. $
 *
 * @end{table}
 */

REGISTER_DIALOG(title_screen)

bool show_debug_clock_button = false;

static bool hotkey(twindow& window, const ttitle_screen::tresult result)
{
	window.set_retval(static_cast<twindow::tretval>(result));

	return true;
}

ttitle_screen::ttitle_screen() : debug_clock_(nullptr)
{
}

ttitle_screen::~ttitle_screen()
{
	delete debug_clock_;
}

static bool fullscreen(CVideo& video)
{
	video.set_fullscreen(!preferences::fullscreen());


	return true;
}

static bool launch_lua_console(twindow & window)
{
	gui2::tlua_interpreter::display(window.video(), gui2::tlua_interpreter::APP);
	return true;
}

void ttitle_screen::post_build(twindow& window)
{
	/** @todo Should become a title screen hotkey. */
	window.register_hotkey(
			hotkey::TITLE_SCREEN__RELOAD_WML,
			boost::bind(&hotkey, boost::ref(window), RELOAD_GAME_DATA));

	window.register_hotkey(hotkey::HOTKEY_FULLSCREEN,
			boost::bind(fullscreen, boost::ref(window.video())));

	window.register_hotkey(
			hotkey::HOTKEY_LANGUAGE,
			boost::bind(&hotkey, boost::ref(window), CHANGE_LANGUAGE));

	window.register_hotkey(hotkey::HOTKEY_LOAD_GAME,
						   boost::bind(&hotkey, boost::ref(window), LOAD_GAME));

	window.register_hotkey(hotkey::HOTKEY_HELP,
						   boost::bind(&hotkey, boost::ref(window), SHOW_HELP));

	window.register_hotkey(
			hotkey::HOTKEY_PREFERENCES,
			boost::bind(&hotkey, boost::ref(window), EDIT_PREFERENCES));

	boost::function<void()> next_tip_wrapper = boost::bind(
			&ttitle_screen::update_tip, this, boost::ref(window), true);

	window.register_hotkey(
			hotkey::TITLE_SCREEN__NEXT_TIP,
			boost::bind(function_wrapper<bool, boost::function<void()> >,
						true,
						next_tip_wrapper));

	boost::function<void()> previous_tip_wrapper = boost::bind(
			&ttitle_screen::update_tip, this, boost::ref(window), false);

	window.register_hotkey(
			hotkey::TITLE_SCREEN__PREVIOUS_TIP,
			boost::bind(function_wrapper<bool, boost::function<void()> >,
						true,
						previous_tip_wrapper));

	window.register_hotkey(hotkey::TITLE_SCREEN__TUTORIAL,
						   boost::bind(&hotkey, boost::ref(window), TUTORIAL));

	window.register_hotkey(
			hotkey::TITLE_SCREEN__CAMPAIGN,
			boost::bind(&hotkey, boost::ref(window), NEW_CAMPAIGN));

	window.register_hotkey(
			hotkey::TITLE_SCREEN__MULTIPLAYER,
			boost::bind(&hotkey, boost::ref(window), MULTIPLAYER));

	window.register_hotkey(
			hotkey::TITLE_SCREEN__ADDONS,
			boost::bind(&hotkey, boost::ref(window), GET_ADDONS));

	window.register_hotkey(hotkey::TITLE_SCREEN__CORES,
						   boost::bind(&hotkey, boost::ref(window), CORES));

	window.register_hotkey(
			hotkey::TITLE_SCREEN__EDITOR,
			boost::bind(&hotkey, boost::ref(window), START_MAP_EDITOR));

	window.register_hotkey(
			hotkey::TITLE_SCREEN__CREDITS,
			boost::bind(&hotkey, boost::ref(window), SHOW_ABOUT));

	window.register_hotkey(hotkey::HOTKEY_QUIT_TO_DESKTOP,
						   boost::bind(&hotkey, boost::ref(window), QUIT_GAME));

	window.register_hotkey(
			hotkey::LUA_CONSOLE,
			boost::bind(&launch_lua_console, boost::ref(window)));
}

#ifdef DEBUG_TOOLTIP
static void
debug_tooltip(twindow& window, bool& handled, const tpoint& coordinate)
{
	std::string message = "Hello world.";
	/*
	 * This function is used to test the tooltip placement algorithms as
	 * described in the »Tooltip placement« section in the GUI2 design
	 * document.
	 *
	 * Use a 1024 x 768 screen size, set the maximum loop iteration to:
	 * - 0  to test with a normal tooltip placement.
	 * - 30 to test with a larger normal tooltip placement.
	 * - 60 to test with a huge tooltip placement.
	 * - 150 to test with a borderline to insanely huge tooltip placement.
	 * - 180 to test with an insanely huge tooltip placement.
	 */
	for(int i = 0; i < 0; ++i) {
		message += " More greetings.";
	}
	gui2::tip::remove();
	gui2::tip::show(window.video(), "tooltip", message, coordinate);
	handled = true;
}
#endif

void ttitle_screen::pre_show(twindow& window)
{
	set_restore(false);
	window.set_click_dismiss(false);
	window.set_enter_disabled(true);
	window.set_escape_disabled(true);

#ifdef DEBUG_TOOLTIP
	window.connect_signal<event::SDL_MOUSE_MOTION>(
			boost::bind(debug_tooltip, boost::ref(window), _3, _5),
			event::tdispatcher::front_child);
#endif

	/**** Set the version number ****/
	if(tcontrol* control
	   = find_widget<tcontrol>(&window, "revision_number", false, false)) {

		control->set_label(_("Version ") + game_config::revision);
	}
	window.canvas()[0].set_variable(
			"revision_number",
			variant(_("Version") + std::string(" ") + game_config::revision));

	/**** Set the tip of the day ****/
	tmulti_page& tip_pages = find_widget<tmulti_page>(&window, "tips", false);

	std::vector<ttip> tips(settings::get_tips());
	if(tips.empty()) {
		WRN_CF << "There are not tips of day available." << std::endl;
	}

	FOREACH(const AUTO & tip, tips)
	{

		string_map widget;
		std::map<std::string, string_map> page;

		widget["label"] = tip.text();
		widget["use_markup"] = "true";
		page["tip"] = widget;

		widget["label"] = tip.source();
		widget["use_markup"] = "true";
		page["source"] = widget;

		tip_pages.add_page(page);
	}

	update_tip(window, true);

	connect_signal_mouse_left_click(
			find_widget<tbutton>(&window, "next_tip", false),
			boost::bind(&ttitle_screen::update_tip,
						this,
						boost::ref(window),
						true));

	connect_signal_mouse_left_click(
			find_widget<tbutton>(&window, "previous_tip", false),
			boost::bind(&ttitle_screen::update_tip,
						this,
						boost::ref(window),
						false));

	if(game_config::images::game_title.empty()) {
		ERR_CF << "No title image defined" << std::endl;
	} else {
		window.canvas()[0].set_variable(
				"title_image", variant(game_config::images::game_title));
	}

	if(game_config::images::game_title_background.empty()) {
		ERR_CF << "No title background image defined" << std::endl;
	} else {
		window.canvas()[0].set_variable(
				"background_image",
				variant(game_config::images::game_title_background));
	}

	/***** Logo *****/
	find_widget<timage>(&window, "logo", false).set_image("misc/logo.png");

	/***** About dialog button *****/
	tbutton& about = find_widget<tbutton>(&window, "about", false);
	connect_signal_mouse_left_click(
			about,
			boost::bind(&tgame_version::display, boost::ref(window.video())));

	/***** Set the clock button. *****/
	tbutton& clock = find_widget<tbutton>(&window, "clock", false);
	clock.set_visible(show_debug_clock_button ? twidget::tvisible::visible
											  : twidget::tvisible::invisible);

	connect_signal_mouse_left_click(
			clock,
			boost::bind(&ttitle_screen::show_debug_clock_window,
						this,
						boost::ref(window.video())));
}

void ttitle_screen::update_tip(twindow& window, const bool previous)
{
	tmulti_page& tips = find_widget<tmulti_page>(&window, "tips", false);
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
	window.set_is_dirty(true);
}

void ttitle_screen::show_debug_clock_window(CVideo& video)
{
	assert(show_debug_clock_button);

	if(debug_clock_) {
		delete debug_clock_;
		debug_clock_ = nullptr;
	} else {
		debug_clock_ = new tdebug_clock();
		debug_clock_->show(video, true);
	}
}

} // namespace gui2
