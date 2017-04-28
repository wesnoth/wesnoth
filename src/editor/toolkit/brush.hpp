/*
   Copyright (C) 2008 - 2017 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef EDITOR_BRUSH_HPP_INCLUDED
#define EDITOR_BRUSH_HPP_INCLUDED

#include "editor/map/editor_map.hpp"

namespace editor {

/**
 * The brush class represents a single brush -- a set of relative locations around a "hotspot",
 * and related info such as the icon image. It is constructed from WML -- the [brush] tag.
 */
class brush
{
public:
	/**
	 * Construct a default (empty) brush. Note that not even the hotspot is affected by default,
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
	std::set<map_location> project(const map_location& hotspot) const;

	/**
	 * @return the name of this brush
	 */
	const std::string name() const { return name_; }

	/**
	 * @return the image of this brush
	 */
	const std::string id() const { return id_; }

protected:
	/**
	 * The relative locations of the brush
	 */
	std::set<map_location> relative_tiles_;

	std::string name_;
	std::string id_;
};


} //end namespace editor

#endif
