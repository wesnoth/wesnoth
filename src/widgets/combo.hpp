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
#ifndef COMBO_H_INCLUDED
#define COMBO_H_INCLUDED

#include "SDL.h"

#include "button.hpp"
#include "../display.hpp"

namespace gui {

class combo
{
public:
	combo(display& disp, const std::vector<std::string>& items);
	void draw();

	void set_x(int val);
	void set_y(int val);
	void set_xy(int valx, int valy);
	void set_selected(int val);

	int width() const;
	int height() const;
	int selected() const;

	void process(int mousex, int mousey, bool button);

private:
	std::vector<std::string> items_;
	int selected_;
	display *display_;
	gui::button button_;


}; //end class combo

}

#endif
