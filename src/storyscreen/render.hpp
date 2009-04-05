/* $Id$ */
/*
   Copyright (C) 2009 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file storyscreen/render.hpp
 * Storyscreen pages rendering interface.
 * @todo Translate relevant parts to GUI2.
 */

#ifndef STORYSCREEN_RENDER_HPP_INCLUDED
#define STORYSCREEN_RENDER_HPP_INCLUDED

#include "key.hpp"
#include "storyscreen/page.hpp"
// #include "widgets/button.hpp"

class display;
class CVideo;

namespace gui { class button; }

namespace storyscreen {

/**
 * Storyscreen page user interface.
 * This works on the assumption, like the old one, that the screen
 * cannot be resized while we are at this. More specifically, it is
 * assumed that the screen dimensions remain constant between the
 * constructor call, and the destruction of the objects.
 */
class page_ui
{
public:
	enum RESULT { NEXT, BACK, SKIP, QUIT };

	page_ui(page& p, display& disp, gui::button& next_button, gui::button& skip_button);
	RESULT show();

private:
	page& p_;
	display& disp_;
	CVideo& video_;
	CKey keys_;
	gui::button& next_button_;
	gui::button& skip_button_;

	RESULT ret_;

	double scale_factor_;
	SDL_Rect base_rect_;

	surface background_;
	std::vector< floating_image::render_input > imgs_;
	bool has_background_;

	int text_x_, text_y_, buttons_x_, buttons_y_;

	void render_background();
	void render_title_box();
	bool render_floating_images();
	void render_text_box();
};

} // end namespace storyscreen

#endif /* !STORYSCREEN_RENDER_HPP_INCLUDED */
