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
#include "cursor.hpp"
#include "display.hpp"
#include "font.hpp"
#include "network.hpp"
#include "unit.hpp"
#include "video.hpp"

#include "widgets/button.hpp"

#include "SDL.h"

#include <string>
#include <vector>

namespace gui
{

extern const int ButtonHPadding;
extern const int ButtonVPadding;

bool in_dialog();

struct dialog_manager : private cursor::setter, private font::floating_label_hider {
	dialog_manager();
	~dialog_manager();

private:
	bool reset_to;
};

void draw_dialog_frame(int x, int y, int w, int h, display& disp, const std::string* dialog_style=NULL, surface_restorer* restorer=NULL);

void draw_dialog_background(int x, int y, int w, int h, display& disp, const std::string& dialog_style);

void draw_rectangle(int x, int y, int w, int h, Uint16 colour, SDL_Surface* tg);

void draw_solid_tinted_rectangle(int x, int y, int w, int h,
                                 int r, int g, int b,
								 double alpha, SDL_Surface* target);

//given the location of a dialog, will draw its title.
//Returns the area the title takes up
SDL_Rect draw_dialog_title(int x, int y, display* disp, const std::string& text);

//function to draw a dialog on the screen. x,y,w,h give the dimensions of the client area
//of the dialog. 'title' is the title of the dialog. The title will be displayed at the
//top of the dialog above the client area. 'dialog_style' if present gives the style of
//the dialog to use.
//'buttons' contains pointers to standard dialog buttons such as 'ok' and 'cancel' that
//will appear on the dialog. If present, they will be located at the bottom of the dialog,
//below the client area.
//if 'restorer' is present, it will be set to a restorer that will reset the screen area
//to its original state after the dialog is drawn.
void draw_dialog(int x, int y, int w, int h, display& disp, const std::string& title,
                 const std::string* dialog_style=NULL, std::vector<button*>* buttons=NULL,
                 surface_restorer* restorer=NULL);

class dialog_action
{
public:
	virtual ~dialog_action() {}

	virtual int do_action() = 0;

	enum { CONTINUE_DIALOG=-2 };
};

class dialog_button_action
{
public:
	virtual ~dialog_button_action() {}

	enum RESULT { DELETE_ITEM, NO_EFFECT };

	virtual RESULT button_pressed(int menu_selection) = 0;
};

struct dialog_button
{
	dialog_button(dialog_button_action* handler, const std::string& label) : handler(handler), label(label)
	{}

	dialog_button_action* handler;
	std::string label;
};

enum { ESCAPE_DIALOG=-3 };

enum DIALOG_TYPE { MESSAGE, OK_ONLY, YES_NO, OK_CANCEL, CANCEL_ONLY, CLOSE_ONLY };

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
				const std::string* dialog_style=NULL,
				std::vector<dialog_button>* buttons=NULL
			 );

network::connection network_data_dialog(display& disp, const std::string& msg, config& cfg, network::connection connection_num=0);

enum TITLE_RESULT { TUTORIAL, NEW_CAMPAIGN, MULTIPLAYER, LOAD_GAME,
                    CHANGE_LANGUAGE, EDIT_PREFERENCES, SHOW_ABOUT, QUIT_GAME };

TITLE_RESULT show_title(display& screen);

void check_quit(display& screen);

}

#endif
