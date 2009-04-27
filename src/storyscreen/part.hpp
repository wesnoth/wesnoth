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
 * @file storyscreen/part.hpp
 * Storyscreen parts and floating images representation.
 */

#ifndef STORYSCREEN_PART_HPP_INCLUDED
#define STORYSCREEN_PART_HPP_INCLUDED

#include <string>
#include <utility>
#include <vector>

#include "sdl_utils.hpp"

class config;
class vconfig;
class display;
class game_state;

namespace storyscreen {

/**
 * Represents and contains informations about image labels used
 * in story screen pages.
 */
class floating_image
{
public:
	struct render_input
	{
		SDL_Rect rect;	/**< Corrected rectangle for rendering surf. */
		surface image;	/**< Surface, scaled if required. */
	};

	floating_image();
	floating_image(const config& cfg);
	floating_image(const floating_image& fi);

	void assign(const floating_image& fi);

	floating_image& operator=(const floating_image& fi) {
		assign(fi);
		return *this;
	}

	/** Returns the referential X coordinate of the image. */
	int ref_x() const { return x_; }
	/** Returns the referential Y coordinate of the image. */
	int ref_y() const { return y_; }

	bool autoscale() const { return autoscaled_; }
	bool centered() const  { return centered_; }

	/** Delay before displaying, in milliseconds. */
	int display_delay() const { return delay_; }

	/** Render. */
	render_input get_render_input(double scale, SDL_Rect& dst_rect) const;

private:
	std::string file_;
	int x_, y_; // referential (non corrected) x,y
	int delay_;
	bool autoscaled_;
	bool centered_;
};

/**
 * Represents and contains information about a single storyscreen page.
 */
class page
{
public:
	enum TEXT_BLOCK_LOCATION {
		TOP,
		MIDDLE,
		BOTTOM
	};
	enum TITLE_ALIGNMENT {
		LEFT, CENTERED, RIGHT
	};
	enum RESULT {
		NEXT,
		SKIP,
		QUIT
	};

	page(game_state& state_of_game, const vconfig& page_cfg);

	bool scale_background() const { return scale_background_; }
	const std::string& background() const { return background_file_; }

	bool show_title() const { return show_title_; }
	const std::string& text() const { return text_; }
	const std::string& title() const { return text_title_; }
	const std::string& music() const { return music_; }

	void set_text(const std::string& text) { text_ = text; }
	void set_title(const std::string& title) { text_title_ = title; }

	const std::vector<floating_image> get_floating_images() const {
		return floating_images_;
	}

private:
	page();

	void resolve_wml(const vconfig& cfg, game_state& gamestate);

	static TEXT_BLOCK_LOCATION string_tblock_loc(const std::string& s);
	static TITLE_ALIGNMENT string_title_align(const std::string& s);

	bool scale_background_;
	std::string background_file_;

	bool show_title_;
	std::string text_;
	std::string text_title_;
	TEXT_BLOCK_LOCATION text_block_loc_;
	TITLE_ALIGNMENT title_alignment_;

	std::string music_;

	std::vector<floating_image> floating_images_;

	friend class page_ui;
};

} // end namespace storyscreen


#endif /* ! STORYSCREEN_PART_HPP_INCLUDED */
