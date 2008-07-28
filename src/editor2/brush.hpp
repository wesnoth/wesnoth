/* $Id$ */
/*
   Copyright (C) 2008 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef EDITOR2_BRUSH_HPP_INCLUDED
#define EDITOR2_BRUSH_HPP_INCLUDED

#include "editor_map.hpp"

#include "../config.hpp"

#include <set>

namespace editor2 {

class brush
{
public:
	/**
	 * Construct a default (empty) brush. Note that not even the hotspot is affected by default
	 */
	brush();
	
	/**
	 * Construct a brush object from config
	 */
	explicit brush(const config& cfg);
	
	/**
	 * Add a location to the brush. If it already exists nothing will change.
	 */
	void add_relative_location(int relative_x, int relative_y);

	/**
	 * Get a set of locations affected (i.e. under the brush) when the center (hotspot)
	 * is in given location
	 */
	std::set<gamemap::location> project(const gamemap::location& hotspot) const;
	
protected:
	std::set<gamemap::location> relative_tiles_;
};


} //end namespace editor2

#endif
