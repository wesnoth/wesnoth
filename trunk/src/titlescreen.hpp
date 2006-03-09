/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef TITLE_HPP_INCLUDED
#define TITLE_HPP_INCLUDED

class config;
class display;

namespace gui {

enum TITLE_RESULT { TUTORIAL = 0, NEW_CAMPAIGN, MULTIPLAYER, LOAD_GAME,
                    CHANGE_LANGUAGE, EDIT_PREFERENCES, BEG_FOR_UPLOAD, SHOW_ABOUT, QUIT_GAME, TITLE_CONTINUE };

TITLE_RESULT show_title(display& screen, config& tips_of_day, int* ntip);

}

#endif
