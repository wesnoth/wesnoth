/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef MENU_HPP_INCLUDED
#define MENU_HPP_INCLUDED

#include "display.hpp"
#include "SDL.h"
#include "unit.hpp"
#include "video.hpp"

#include <string>
#include <vector>

namespace gui
{

void draw_dialog_frame(int x, int y, int w, int h, display& disp);

void draw_rectangle(int x, int y, int w, int h, short colour, SDL_Surface* tg);

void draw_solid_tinted_rectangle(int x, int y, int w, int h,
                                 int r, int g, int b,
								 double alpha, SDL_Surface* target);

class dialog_action
{
public:
	virtual ~dialog_action() {}

	virtual int do_action() = 0;

	enum { CONTINUE_DIALOG=-2 };
};

enum DIALOG_TYPE { MESSAGE, OK_ONLY, YES_NO, OK_CANCEL, CANCEL_ONLY };

//if a menu is given, then returns -1 if the dialog was cancelled, and the
//index of the selection otherwise. If no menu is given, returns the index
//of the button that was pressed
int show_dialog(display& screen, SDL_Surface* image,
                const std::string& caption, const std::string& message,
                DIALOG_TYPE type=MESSAGE,
                const std::vector<std::string>* menu_items=NULL,
                const std::vector<unit>* units=NULL,
				const std::string& text_widget_label="",
				std::string* text_widget_text=NULL,
                dialog_action* action=NULL
			 );

enum TITLE_RESULT { TUTORIAL, NEW_CAMPAIGN, MULTIPLAYER, LOAD_GAME, QUIT_GAME,
                    CHANGE_LANGUAGE };

TITLE_RESULT show_title(display& screen);

void check_quit(display& screen);

}

#endif
