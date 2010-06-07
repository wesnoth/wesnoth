/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Ignacio R. Morelle <shadowm2006@gmail.com>
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
 * Storyscreen parts rendering interface.
 * @todo Translate relevant parts to GUI2.
 */

#ifndef STORYSCREEN_RENDER_HPP_INCLUDED
#define STORYSCREEN_RENDER_HPP_INCLUDED

#include "key.hpp"
#include "storyscreen/part.hpp"
// #include "widgets/button.hpp"

class display;
class CVideo;

namespace gui { class button; }

namespace storyscreen {

/**
 * Storyscreen part user interface.
 * This works on the assumption, like the old one, that the screen
 * cannot be resized while we are at this. More specifically, it is
 * assumed that the screen dimensions remain constant between the
 * constructor call, and the destruction of the objects.
 */
class part_ui
{
public:
	/** Storyscreen result. */
	enum RESULT {
		NEXT,	/**< The user pressed the go-next button. */
		BACK,	/**< The user pressed the go-back button. */
		FIRST,	/**< The user pressed the go-first button. */
		LAST,	/**< The user pressed the go-last button. */
		QUIT	/**< The user selected quit. */
	};

	/**
	 * Constructor.
	 * @param p A storyscreen::part with the required information and parameters.
	 * @param disp Display.
	 * @param next_button Next button. Shouldn't be destroyed before the part_ui object.
	 * @param skip_button Skip button. Shouldn't be destroyed before the part_ui object.
	 */
        part_ui(part& p, display& disp, 
                gui::button& next_button,  gui::button& back_button, 
                gui::button& first_button, gui::button& last_button,
		gui::button& play_button);

	/**
	 * Render and display the storyscreen, process and return user input.
	 */
	RESULT show();

private:
	part& p_;
	display& disp_;
	CVideo& video_; // convenience, it's currently obtained from disp_
	CKey keys_;     // convenience

	gui::button& next_button_;
	gui::button& back_button_;
	gui::button& first_button_;
	gui::button& last_button_;
	gui::button& play_button_;

	RESULT ret_;

	double scale_factor_;

	// The background is aspect-corrected when scaled, so the image doesn't
	// look distorted. base_rect_ has the actual area occupied by the background.
	SDL_Rect base_rect_;

	surface background_;
	std::vector< floating_image::render_input > imgs_;
	bool has_background_;

	int text_x_, text_y_, buttons_x_, buttons_y_;

	/** Constructor implementation details. */
	void prepare_background();
	/** Constructor implementation details. */
	void prepare_geometry();
	/** Constructor implementation details. */
	void prepare_floating_images();

	void render_background();
	void render_title_box();
	void render_story_box();
	void render_story_box_borders(SDL_Rect&);
	/**
	 * Renders all floating images in sequence.
	 * @return 'true' if the user interrupted the operation; 'false' otherwise.
	 */
	bool render_floating_images();

	void wait_for_input();
};

} // end namespace storyscreen

#endif /* !STORYSCREEN_RENDER_HPP_INCLUDED */
