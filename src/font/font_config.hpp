/*
   Copyright (C) 2016 by Chris Beck<render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef FONT_FONT_CONFIG_HPP_INCLUDED
#define FONT_FONT_CONFIG_HPP_INCLUDED

/***
 * The font::manager initializes cairo and font_config in order to figure out
 * what local fonts to use. It also asks SDL_TTF to initialize itself, via the
 * sdl_ttf raii object.
 */

#include "exceptions.hpp"
#include "font_options.hpp"
#include "sdl_ttf.hpp"

class t_string;

namespace font {

//object which initializes and destroys structures needed for fonts
//
struct manager {
	manager();
	~manager();

	manager(const manager &) = delete;
	manager & operator = (const manager &) = delete;

	/**
	 * Updates the font path, when initialized it sets the fontpath to
	 * game_config::path. When this path is updated, this function should be
	 * called.
	 */
	void update_font_path() const;

	struct error : public game::error {
		error() : game::error("Font initialization failed") {}
	};
private:
	/** Initializes the font path. */
	void init() const;

	/** Deinitializes the font path. */
	void deinit() const;

    /** Initialize sdl_ttf concurrent with font::manager lifetime */
	sdl_ttf sdl_ttf_initializer_;
};

/***
 * load_font_config actually searches the game font path for fonts, and refreshes
 * the set of loaded fonts
 */
bool load_font_config();

/** Returns the currently defined fonts. */
const t_string& get_font_families(family_class fclass = FONT_SANS_SERIF);

/** Test if a font file exists */
bool check_font_file(std::string name);

} // end namespace font

#endif
