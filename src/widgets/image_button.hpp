/* $Id: button.hpp 7396 2005-07-02 21:37:20Z ott $ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef IMAGE_BUTTON_H_INCLUDED
#define IMAGE_BUTTON_H_INCLUDED

#include "SDL.h"

#include "widget.hpp"

#include "../sdl_utils.hpp"

#include <string>
#include <vector>
#include <string>

namespace gui {

class image_button : public widget
{
public:
	struct error {};

	enum SPACE_CONSUMPTION { DEFAULT_SPACE, MINIMUM_SPACE };

	image_button(CVideo& video, std::string button_image="", SPACE_CONSUMPTION spacing=DEFAULT_SPACE);

	virtual ~image_button() {}

	bool pressed();

	void enable(bool new_val);
	bool enabled() const;

protected:
	virtual void handle_event(const SDL_Event& event);
	virtual void mouse_motion(const SDL_MouseMotionEvent& event);
	virtual void mouse_down(const SDL_MouseButtonEvent& event);
	virtual void mouse_up(const SDL_MouseButtonEvent& event);
	virtual void draw_contents();

private:

	void calculate_size();

	surface image_, pressedImage_, activeImage_, pressedActiveImage_;
	SDL_Rect textRect_;

	bool button_;

	enum STATE { UNINIT, NORMAL, ACTIVE, PRESSED, PRESSED_ACTIVE };
	STATE state_;

	bool enabled_;

	bool pressed_;

	SPACE_CONSUMPTION spacing_;

	int base_height_, base_width_;

	bool hit(int x, int y) const;
}; //end class button

}

#endif
