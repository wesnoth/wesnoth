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
 * @file storyscreen/page.hpp
 * This code is work in progress, and the interfaces may change.
 * It is supposed to completely replace the old story screens code
 * at intro.cpp, introducing new WML conventions while at it.
 */

#ifndef STORYSCREEN_PAGE_HPP_INCLUDED
#define STORYSCREEN_PAGE_HPP_INCLUDED

#include <string>
#include <vector>

class config;
class vconfig;
class game_state;

namespace storyscreen {

/**
 * Represents and contains informations about image labels used
 * in story screen pages.
 */
class floating_image
{
public:
	floating_image(const config& cfg);

	/** Returns the referential X coordinate of the image. */
	int ref_x() const { return x_; }
	/** Returns the referential Y coordinate of the image. */
	int ref_y() const { return y_; }

	bool autoscale() const { return autoscaled_; }
	bool centered() const  { return centered_; }

	/** Delay before displaying, in milliseconds. */
	int display_delay() const { return delay_; }

private:
	floating_image();

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

	page(game_state& state_of_game, const vconfig& page_cfg);

	bool scale_background() const { return scale_background_; }
	const std::string& background() const { return background_file_; }

	bool show_title() const { return show_title_; }
	const std::string& text() const { return text_; }
	const std::string& title() const { return text_title_; }

private:
	page();

	void resolve_wml(const vconfig& page_cfg);

	static TEXT_BLOCK_LOCATION string_tblock_loc(const std::string& s);

	bool scale_background_;
	std::string background_file_;

	bool show_title_;
	std::string text_;
	std::string text_title_;
	TEXT_BLOCK_LOCATION text_block_loc_;

	std::string music_;

	std::vector<floating_image> floating_images_;
};

} // end namespace storyscreen


#endif /* ! STORYSCREEN_PAGE_HPP_INCLUDED */
