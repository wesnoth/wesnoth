/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#pragma once

#include "color.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

//These macros interfere with MS VC++
#ifdef _MSC_VER
	#undef max
	#undef min
#endif

using color_range_map = std::unordered_map<color_t, color_t>;

/**
 * A color range definition is made of four reference RGB colors, used
 * for calculating conversions from a source/key palette.
 *
 *   1) The average shade of a unit's team-color portions
 *      (default: gray #808080)
 *   2) The maximum highlight shade of a unit's team-color portions
 *      (default: white)
 *   3) The minimum shadow shade of a unit's team-color portions
 *      (default: black)
 *   4) A plain high-contrast color, used for the markers on the mini-map
 *      (default: same as the provided average shade, or gray #808080)
 *
 * The first three reference colors are used for converting a source palette
 * with the external recolor_range() method.
 */
class color_range
{
public:
	/**
	* Constructor, which expects four reference RGB colors.
	* @param mid Average color shade.
	* @param max Maximum (highlight) color shade
	* @param min Minimum color shade
	* @param rep High-contrast reference color
	*/
	color_range(color_t mid, color_t max = {255, 255, 255}, color_t min = {0, 0, 0}, color_t rep = {128, 128, 128})
		: mid_(mid)
		, max_(max)
		, min_(min)
		, rep_(rep)
	{}

	/**
	* Constructor, which expects four reference RGB colors.
	* @param v STL vector with the four reference colors in order.
	*/
	color_range(const std::vector<color_t>& v)
		: mid_(v.size()     ? v[0] : color_t(128, 128, 128))
		, max_(v.size() > 1 ? v[1] : color_t(255, 255, 255))
		, min_(v.size() > 2 ? v[2] : color_t(0  , 0  , 0  ))
		, rep_(v.size() > 3 ? v[3] : mid_)
	{}

	/** Default constructor. */
	color_range()
		: mid_(128, 128, 128)
		, max_(255, 255, 255)
		, min_()
		, rep_(128, 128, 128)
	{}

	/** Average color shade. */
	color_t mid() const { return mid_; }

	/** Maximum color shade. */
	color_t max() const { return max_; }

	/** Minimum color shade. */
	color_t min() const { return min_; }

	/** High-contrast shade, intended for the minimap markers. */
	color_t rep() const { return rep_; }

	bool operator==(const color_range& b) const
	{
		return mid_ == b.mid() && max_ == b.max() && min_ == b.min() && rep_ == b.rep();
	}

	bool operator<(const color_range& b) const
	{
		if(mid_ != b.mid()) { return mid_.to_rgba_bytes() < b.mid().to_rgba_bytes(); }
		if(max_ != b.max()) { return max_.to_rgba_bytes() < b.max().to_rgba_bytes(); }
		if(min_ != b.min()) { return min_.to_rgba_bytes() < b.min().to_rgba_bytes(); }

		return rep_.to_rgba_bytes() < b.rep().to_rgba_bytes();
	}

	/** Return a string describing the color range for debug output. */
	std::string debug() const;

private:
	color_t mid_ , max_ , min_ , rep_;
};

/**
 * Creates a reference color palette from a color range.
 */
std::vector<color_t> palette(const color_range& cr);

/**
 * Converts a source palette using the specified color_range object.
 * This holds the main interface for range-based team coloring. The output is used with the recolor_image()
*  method to do the actual recoloring.
 *
 * @param        new_rgb Specifies parameters for the conversion.
 * @param        old_rgb Source palette.
 *
 * @return       A STL map of colors, with the keys being source palette elements, and the values
 *               are the result of applying the color range conversion on it.
 */
color_range_map recolor_range(const color_range& new_rgb, const std::vector<color_t>& old_rgb);
