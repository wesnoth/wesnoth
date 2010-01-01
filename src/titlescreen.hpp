/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef TITLE_HPP_INCLUDED
#define TITLE_HPP_INCLUDED

class config;
class game_display;

/** Read the file with the tips-of-the-day. */
void read_tips_of_day(config& tips_of_day);

/** Go to the next tips-of-the-day */
void next_tip_of_day(config& tips_of_day, bool reverse = false);

/** Return the text for one of the tips-of-the-day. */
const config* get_tip_of_day(config& tips_of_day);

namespace gui {

/**
 * Values for the menu-items of the main menu.
 *
 * The code assumes TUTORIAL is the first item.
 * The values are also used as the button retour values, where 0 means no
 * automatic value so we need to avoid 0.
 */
enum TITLE_RESULT { TUTORIAL = 1,		/**< Start special campaign 'tutorial' */
					NEW_CAMPAIGN,		/**< Let user select a campaign to play */
					MULTIPLAYER,		/**< Play single scenario against humans or AI */
					LOAD_GAME, GET_ADDONS,
#ifndef DISABLE_EDITOR
					START_MAP_EDITOR,
#endif
                    CHANGE_LANGUAGE, EDIT_PREFERENCES,
					SHOW_ABOUT,			/**< Show credits */
					QUIT_GAME,
					TIP_PREVIOUS,		/**< Show previous tip-of-the-day */
					TIP_NEXT,			/**< Show next tip-of-the-day */
					SHOW_HELP,
					BEG_FOR_UPLOAD,		/**< Ask user for permission to upload game-stats as feedback */
					REDRAW_BACKGROUND,	/**< Used after an action needing a redraw (ex: fullscreen) */
					RELOAD_GAME_DATA,	/**< Used to reload all game data */
					NOTHING				/**< Default, nothing done, no redraw needed */
				  };

/** Mark the titlescreen background as dirty */
void set_background_dirty();

/**
 *  Show titlepage with logo and background, menu-buttons and tip-of-the-day.
 *
 *  After the page is shown, this routine waits
 *  for the user to click one of the menu-buttons,
 *  or a keypress.
 *
 *  @param	screen			display object
 *  @param	tips_of_day		list of tips
 *
 *  @return	the value of the menu-item the user has choosen.
 *  @retval	see @ref TITLE_RESULT for possible values
 */
TITLE_RESULT show_title(game_display& screen, config& tips_of_day);
}

#endif
