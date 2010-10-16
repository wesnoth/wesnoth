/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Mark de Wever <koraq@xs4all.nl>
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

namespace gui2 {

class ttitle_screen : public tdialog
{
public:
	ttitle_screen();

	~ttitle_screen();

	CVideo* video() { return video_; }

	/**
	 * Values for the menu-items of the main menu.
	 *
	 * @todo Evaluate the best place for these items.
	 */
	enum tresult {
			  TUTORIAL = 1        /**< Start special campaign 'tutorial' */
			, NEW_CAMPAIGN        /**< Let user select a campaign to play */
			, MULTIPLAYER         /**<
			                       * Play single scenario against humans or AI
			                       */
			, LOAD_GAME
			, GET_ADDONS
#ifndef DISABLE_EDITOR
			, START_MAP_EDITOR
#endif
			, CHANGE_LANGUAGE
			, EDIT_PREFERENCES
			, SHOW_ABOUT          /**< Show credits */
			, QUIT_GAME
			, TIP_PREVIOUS        /**< Show previous tip-of-the-day */
			, TIP_NEXT            /**< Show next tip-of-the-day */
			, SHOW_HELP
			, REDRAW_BACKGROUND   /**<
			                       * Used after an action needing a redraw (ex:
			                       * fullscreen)
			                       */
			, RELOAD_GAME_DATA    /**< Used to reload all game data */
			, NOTHING             /**<
			                       * Default, nothing done, no redraw needed
			                       */
			};

private:
	/** Used in show in order to show child windows. */
	CVideo* video_;

	/** Inherited from tdialog, implemented by REGISTER_WINDOW. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	virtual void post_build(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	/** The progress bar time for the logo. */
	unsigned long logo_timer_id_;

	/**
	 * Updates the tip of day widget.
	 *
	 * @param window              The window being shown.
	 * @param previous            Show the previous tip, else shows the next
	 *                            one.
	 */
	void update_tip(twindow& window, const bool previous);
};

} // namespace gui2

#endif
