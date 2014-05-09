/*
   Copyright (C) 2008 - 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_TITLE_SCREEN_HPP_INCLUDED
#define GUI_DIALOGS_TITLE_SCREEN_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

namespace gui2
{

class tpopup;

/** Do we wish to show the button for the debug clock. */
extern bool show_debug_clock_button;

/**
 * This class implements the title screen.
 *
 * The menu buttons return a result back to the caller with the button pressed.
 * So at the moment it only handles the tips itself.
 *
 * @todo Evaluate whether we can handle more buttons in this class.
 */
class ttitle_screen : public tdialog
{
public:
	ttitle_screen();

	~ttitle_screen();

	/**
	 * Values for the menu-items of the main menu.
	 *
	 * @todo Evaluate the best place for these items.
	 */
	enum tresult {
		TUTORIAL = 1 /**< Start special campaign 'tutorial' */
		,
		NEW_CAMPAIGN /**< Let user select a campaign to play */
		,
		DOWNLOAD_CAMPAIGN,
		MULTIPLAYER /**<
					 * Play single scenario against humans or AI
					 */
		,
		LOAD_GAME,
		GET_ADDONS,
		CORES,
		START_MAP_EDITOR,
		CHANGE_LANGUAGE,
		EDIT_PREFERENCES,
		SHOW_ABOUT /**< Show credits */
		,
		QUIT_GAME,
		TIP_PREVIOUS /**< Show previous tip-of-the-day */
		,
		TIP_NEXT /**< Show next tip-of-the-day */
		,
		SHOW_HELP,
		REDRAW_BACKGROUND /**<
						   * Used after an action needing a redraw (ex:
						   * fullscreen)
						   */
		,
		RELOAD_GAME_DATA /**< Used to reload all game data */
		,
		NOTHING /**<
				 * Default, nothing done, no redraw needed
				 */
	};

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	virtual void post_build(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** The progress bar time for the logo. */
	size_t logo_timer_id_;

	/** Holds the debug clock dialog. */
	tpopup* debug_clock_;

	/**
	 * Updates the tip of day widget.
	 *
	 * @param window              The window being shown.
	 * @param previous            Show the previous tip, else shows the next
	 *                            one.
	 */
	void update_tip(twindow& window, const bool previous);

	/** Shows the debug clock. */
	void show_debug_clock_window(CVideo& video);
};

} // namespace gui2

#endif
