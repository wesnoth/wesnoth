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
 * @file
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
 * in story screen parts.
 */
class floating_image
{
public:
	struct render_input
	{
		SDL_Rect rect;	/**< Corrected rectangle for rendering surf. */
		surface image;	/**< Surface, scaled if required. */
	};

	/**
	 * WML-based constructor.
	 * @param cfg Object corresponding to a [image] block's contents from
	 *            a [part] node.
	 */
	floating_image(const config& cfg);

	/**
	 * Copy constructor.
	 */
	floating_image(const floating_image& fi);

	floating_image& operator=(const floating_image& fi) {
		assign(fi);
		return *this;
	}

	/**
	 * Returns the referential X coordinate of the image.
	 * The actual (corrected) value is determined at render time - see get_render_input().
	 */
	int ref_x() const {
		return x_;
	}

	/**
	 * Returns the referential Y coordinate of the image.
	 * The actual (corrected) value is determined at render time - see get_render_input().
	 */
	int ref_y() const {
		return y_;
	}

	/**
	 * Whether the image should be automatically scaled as much as
	 * the storyscreen background is.
	 */
	bool autoscale() const { return autoscaled_; }

	/**
	 * Whether the image coordinates specify the location of its
	 * center (true) or top-left corner (false).
	 */
	bool centered() const  { return centered_; }

	/**
	 * Delay before displaying, in milliseconds.
	 */
	int display_delay() const { return delay_; }

	/**
	 * Gets a render_input object for use by the rendering code after applying
	 * any geometric transformations required.
	 */
	render_input get_render_input(double scale, SDL_Rect& dst_rect) const;

private:
	std::string file_;
	int x_, y_; // referential (non corrected) x,y
	int delay_;
	bool autoscaled_;
	bool centered_;

	/** Copy constructor and operator=() implementation details. */
	void assign(const floating_image& fi);
};

/**
 * Represents and contains information about a single storyscreen part.
 */
class part
{
public:
	/**
	 * Currently used to indicate where the text block should be placed.
	 * Note that it will always take as much space as it is
	 * possible horizontally.
	 */
	enum BLOCK_LOCATION {
		BLOCK_TOP,		/**< Top of the screen. */
		BLOCK_MIDDLE,	/**< Center of the screen. */
		BLOCK_BOTTOM	/**< Bottom of the screen. This is the default. */
	};
	/**
	 * Currently used to indicate where the page title should be placed.
	 * It always takes as little space (horizontally) as possible,
	 * and it is always placed at the top of the screen.
	 */
	enum TEXT_ALIGNMENT {
		TEXT_LEFT,		/**< Top-left corner. */
		TEXT_CENTERED,	/**< Center on the topmost edge of the screen. */
		TEXT_RIGHT		/**< Top-right corner. */
	};
	/**
	 * Used to signal user actions.
	 */
	enum RESULT {
		NEXT,		/**< Jump to next story part. */
		SKIP,		/**< Skip all story parts for this set. */
		QUIT		/**< Quit game and go back to main menu. */
	};

	/**
	 * Constructs a storyscreen part from a managed WML node.
	 * @param part_cfg Node object which should correspond to a [part] block's contents.
	 */
	part(const vconfig &part_cfg);

	/** Whether the background image should be scaled to fill the screen or not. */
	bool scale_background() const {
		return scale_background_;
	}

	/** Path to the background image. */
	const std::string& background() const {
		return background_file_;
	}

	/** Whether the story screen title should be displayed or not. */
	bool show_title() const {
		return show_title_;
	}

	/** Retrieves the story text itself. */
	const std::string& text() const {
		return text_;
	}

	/** Changes the story text. */
	void set_text(const std::string& text) {
		text_ = text;
	}

	/** Retrieves the story screen title. */
	const std::string& title() const {
		return text_title_;
	}

	/** Changes the story screen title. */
	void set_title(const std::string& title) {
		text_title_ = title;
	}

	/** Retrieves the background music. */
	const std::string& music() const {
		return music_;
	}

	/** Retrieves a one-time-only sound effect. */
	const std::string& sound() const {
		return sound_;
	}

	/** Retrieves the area of the screen on which the story text is displayed. */
	BLOCK_LOCATION story_text_location() const {
		return text_block_loc_;
	}

	/** Retrieves the alignment of the title text against the screen. */
	TEXT_ALIGNMENT title_text_alignment() const {
		return title_alignment_;
	}

	/** Retrieve any associated floating images for this story screen. */
	const std::vector<floating_image>& get_floating_images() const {
		return floating_images_;
	}

private:
	/** Takes care of initializing and branching properties. */
	void resolve_wml(const vconfig &cfg);

	static BLOCK_LOCATION string_tblock_loc(const std::string& s);
	static TEXT_ALIGNMENT string_title_align(const std::string& s);

	bool scale_background_;
	std::string background_file_;

	bool show_title_;
	std::string text_;
	std::string text_title_;
	BLOCK_LOCATION text_block_loc_;
	TEXT_ALIGNMENT title_alignment_;

	std::string music_;
	std::string sound_;

	std::vector<floating_image> floating_images_;
};

} // end namespace storyscreen


#endif /* ! STORYSCREEN_PART_HPP_INCLUDED */
