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

#ifndef SHOW_DIALOG_HPP_INCLUDED
#define SHOW_DIALOG_HPP_INCLUDED

#include "config.hpp"
#include "display.hpp"
#include "network.hpp"
#include "unit.hpp"
#include "video.hpp"

#include "SDL.h"

#include <string>
#include <vector>

namespace gui
{

bool in_dialog();

struct dialog_manager {
	dialog_manager();
	~dialog_manager();

private:
	bool reset_to;
};

void draw_dialog_frame(int x, int y, int w, int h, display& disp, const std::string* dialog_style=NULL);

void draw_dialog_background(int x, int y, int w, int h, display& disp, const std::string& dialog_style);

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

struct check_item {
	check_item(const std::string& label, bool checked) : label(label), checked(checked) {}
	std::string label;
	bool checked;
};

//function to chop up one long string of text into lines
size_t text_to_lines(std::string& text, size_t max_length);

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
                dialog_action* action=NULL,
				std::vector<check_item>* options=NULL, int xloc=-1, int yloc=-1,
				const std::string* dialog_style=NULL
			 );

network::connection network_data_dialog(display& disp, const std::string& msg, config& cfg, network::connection connection_num=0);

enum TITLE_RESULT { TUTORIAL, NEW_CAMPAIGN, MULTIPLAYER, LOAD_GAME,
                    CHANGE_LANGUAGE, EDIT_PREFERENCES, SHOW_ABOUT, QUIT_GAME };

TITLE_RESULT show_title(display& screen);

void check_quit(display& screen);

}

#endif
