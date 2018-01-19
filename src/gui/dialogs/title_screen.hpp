/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gui/dialogs/modal_dialog.hpp"

class game_launcher;

namespace gui2
{
namespace dialogs
{

class modeless_dialog;

/** Do we wish to show the button for the debug clock. */
extern bool show_debug_clock_button;

/**
 * This class implements the title screen.
 *
 * The menu buttons return a result back to the caller with the button pressed.
 * So at the moment it only handles the tips itself.
 */
class title_screen : public modal_dialog
{
public:
	title_screen(game_launcher& game);

	~title_screen();

	/**
	 * Values for actions which leave the title screen.
	 * Actions that merely show a dialog are not included here.
	 */
	enum result {
		// Window was resized, so needs redrawing
		REDRAW_BACKGROUND = 0, // Needs to be 0, the value of gui2::window::NONE
		// Start playing a single-player game, such as the tutorial or a campaign
		LAUNCH_GAME,
		// Connect to an MP server
		MP_CONNECT,
		// Host an MP game
		MP_HOST,
		// Start a local MP game
		MP_LOCAL,
		// Start the map/scenario editor
		MAP_EDITOR,
		// Show credits
		SHOW_ABOUT,
		// Exit to desktop
		QUIT_GAME,
		// Used to reload all game data
		RELOAD_GAME_DATA,
	};

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/***** ***** ***** ***** Callbacks ***** ***** ****** *****/

	void on_resize(window& window);

	/**
	 * Updates the tip of day widget.
	 *
	 * @param window              The window being shown.
	 * @param previous            Show the previous tip, else shows the next one.
	 */
	void update_tip(window& window, const bool previous);

	/** Shows the debug clock. */
	void show_debug_clock_window();

	void hotkey_callback_select_tests(window& window);

	void button_callback_multiplayer(window& window);

	void button_callback_cores();

	/** Holds the debug clock dialog. */
	std::unique_ptr<modeless_dialog> debug_clock_;

	game_launcher& game_;

};

} // namespace dialogs
} // namespace gui2
