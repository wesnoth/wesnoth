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
 * @ingroup GUIWindowDefinitionWML
 *
 * This class implements the title screen.
 *
 * The menu buttons return a result back to the caller with the button pressed.
 * So at the moment it only handles the tips itself.
 *
 * Key               |Type          |Mandatory|Description
 * ------------------|--------------|---------|-----------
 * tutorial          | @ref button  |yes      |The button to start the tutorial.
 * campaign          | @ref button  |yes      |The button to start a campaign.
 * multiplayer       | @ref button  |yes      |The button to start multiplayer mode.
 * load              | @ref button  |yes      |The button to load a saved game.
 * editor            | @ref button  |yes      |The button to start the editor.
 * addons            | @ref button  |yes      |The button to start managing the addons.
 * cores             | @ref button  |yes      |The button to start managing the cores.
 * language          | @ref button  |yes      |The button to select the game language.
 * credits           | @ref button  |yes      |The button to show Wesnoth's contributors.
 * quit              | @ref button  |yes      |The button to quit Wesnoth.
 * tips              | multi_page   |yes      |A multi_page to hold all tips, when this widget is used the area of the tips doesn't need to be resized when the next or previous button is pressed.
 * tip               | @ref label   |no       |Shows the text of the current tip.
 * source            | @ref label   |no       |The source (the one who's quoted or the book referenced) of the current tip.
 * next_tip          | @ref button  |yes      |The button show the next tip of the day.
 * previous_tip      | @ref button  |yes      |The button show the previous tip of the day.
 * logo              | progress_bar |no       |A progress bar to "animate" the Wesnoth logo.
 * revision_number   | control      |no       |A widget to show the version number when the version number is known.
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
		// Show credits
		SHOW_ABOUT,
		// Exit to desktop
		QUIT_GAME,
		// Used to reload all game data
		RELOAD_GAME_DATA,
	};

private:
	virtual const std::string& window_id() const override;

	void init_callbacks();

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

	void hotkey_callback_select_tests();

	void show_achievements();

	void button_callback_multiplayer();

	void button_callback_cores();

	/** Holds the debug clock dialog. */
	std::unique_ptr<modeless_dialog> debug_clock_;

	game_launcher& game_;

};

} // namespace dialogs
