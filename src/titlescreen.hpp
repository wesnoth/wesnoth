/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
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

namespace gui {

//! Values for the menu-items of the main menu.
enum TITLE_RESULT { TUTORIAL = 0,		//!< Start special campaign 'tutorial'
					NEW_CAMPAIGN,		//!< Let user select a campaign to play
					MULTIPLAYER,		//!< Play single scenario against humans or AI
					LOAD_GAME, GET_ADDONS,
                    CHANGE_LANGUAGE, EDIT_PREFERENCES,
					SHOW_ABOUT,			//!< Show credits
					QUIT_GAME,
					TIP_PREVIOUS,	//!< Show previous tip-of-the-day
					TIP_NEXT,		//!< Show next tip-of-the-day
					SHOW_HELP,
					BEG_FOR_UPLOAD		//!< Ask user for permission to upload game-stats as feedback
				  };

//! Show titlepage with logo & background, menu-buttons and tip-of-day.
TITLE_RESULT show_title(game_display& screen, config& tips_of_day);
}

#endif
