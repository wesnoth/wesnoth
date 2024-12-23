/*
	Copyright (C) 2008 - 2024
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

#pragma once

#include "gui/dialogs/modal_dialog.hpp"

class game_launcher;

namespace gui2::dialogs
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
		REDRAW_BACKGROUND = 0, // Needs to be 0, the value of gui2::retval::NONE
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
		// Exit to desktop
		QUIT_GAME,
		// Used to reload all game data
		RELOAD_GAME_DATA,
		// Used to reshow the titlescreen, for example,
		// in the case of a gui2 theme change
		RELOAD_UI,
	};

private:
	virtual const std::string& window_id() const override;

	void init_callbacks();

	void register_button(const std::string& id, hotkey::HOTKEY_COMMAND hk, const std::function<void()>& callback);

	/***** ***** ***** ***** Callbacks ***** ***** ****** *****/

	void on_resize();

	/**
	 * Updates the tip of day widget.
	 *
	 * @param previous            Show the previous tip, else shows the next one.
	 */
	void update_tip(const bool previous);

	/** Updates UI labels that are not t_string after a language change. */
	void update_static_labels();

	/** Shows the debug clock. */
	void show_debug_clock_window();

	/** Shows the gui test window. */
	void show_gui_test_dialog();

	/** Shows the preferences dialog. */
	void show_preferences();

	void hotkey_callback_select_tests();

	void show_achievements();

	void show_community();

	void button_callback_multiplayer();

	void button_callback_cores();

	/** Holds the debug clock dialog. */
	std::unique_ptr<modeless_dialog> debug_clock_;

	game_launcher& game_;

};

} // namespace dialogs
