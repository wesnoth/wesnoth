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
#ifndef BUTTON_H_INCLUDED
#define BUTTON_H_INCLUDED

#include "SDL.h"

#include "../sdl_utils.hpp"

#include <string>
#include <vector>
#include <string>

class display;

namespace gui {

class button
{
public:
	struct error {};

	enum TYPE { TYPE_PRESS, TYPE_CHECK };

	button(display& disp, const std::string& label, TYPE type=TYPE_PRESS,
	       std::string button_image="");

	void set_check(bool check);
	bool checked() const;

	void draw();

	void set_x(int val);
	void set_y(int val);
	void set_xy(int valx, int valy);
	void set_label(const std::string& val);

	int width() const;
	int height() const;

	bool process(int mousex, int mousey, bool button);

	void backup_background();
	void hide();

private:
	surface_restorer restorer_;
	std::string label_;
	display* display_;
	shared_sdl_surface image_, pressedImage_, activeImage_, pressedActiveImage_;
	int x_, y_, w_, h_;
	SDL_Rect textRect_;

	bool button_;

	enum STATE { UNINIT, NORMAL, ACTIVE, PRESSED, PRESSED_ACTIVE };
	STATE state_;

	TYPE type_;

	bool hit(int x, int y) const;
}; //end class button

}

#endif
