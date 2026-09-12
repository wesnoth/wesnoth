/*
	Copyright (C) 2003 - 2025
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "color.hpp"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

using color_mapping = std::unordered_map<color_t, color_t>;

/**
 * Representing the "team color" information, this C++ class contains a subset
 * of WML's [color_range] tag. The other content of the [color_range] tags is
 * parsed by the game_config class, and the data is held by several globals
 * declared in game_config.hpp.
 *
 * The main definition is made of three reference RGB colors, used for
 * calculating conversions from a source/key palette with the external
 * generate_color_mapping() method.
 *
 *   1) The average shade of a unit's team-color portions
 *      (default: gray #808080)
 *   2) The maximum highlight shade of a unit's team-color portions
 *      (default: white)
 *   3) The minimum shadow shade of a unit's team-color portions
 *      (default: black)
 *
 * Some further colors are included because this mixes the responsibilities of
 * being the "team color" class.
 *
 *   4) A color with high contrast to terrain colors, so that it can
 *      be used for the markers on the mini-map.
 *      (default: same as the provided average shade, or gray #808080)
 *   5) A color with high contrast to dark backgrounds, used for text.
 */
class color_range
{
public:
	/**
	* Constructor, which expects four reference RGB colors.
	* @param mid Average color shade.
	* @param max Maximum (highlight) color shade
	* @param min Minimum color shade
	* @param rep High-contrast to terrain
	* @param font High-contrast to a dark background
	*/
	color_range(color_t mid, color_t max, color_t min, color_t rep, color_t font)
		: mid_(mid)
		, max_(max)
		, min_(min)
		, rep_(rep)
		, font_color_(font)
	{}

	/**
	* Constructor, matching the WML tag's split of the rgb attribute in one argument,
    * and the color for UI text in a different argument.
	*/
	color_range(const std::vector<color_t>& v, const std::optional<color_t>& font)
		: mid_(v.size()     ? v[0] : color_t(128, 128, 128))
		, max_(v.size() > 1 ? v[1] : color_t(255, 255, 255))
		, min_(v.size() > 2 ? v[2] : color_t(0  , 0  , 0  ))
		, rep_(v.size() > 3 ? v[3] : mid_)
		, font_color_(font ? *font : mid_)
	{}

	/** Default constructor. */
	color_range()
		: mid_(128, 128, 128)
		, max_(255, 255, 255)
		, min_()
		, rep_(128, 128, 128)
		, font_color_(mid_)
	{}

	/** Average color shade. */
	color_t mid() const { return mid_; }

	/** Maximum color shade. */
	color_t max() const { return max_; }

	/** Minimum color shade. */
	color_t min() const { return min_; }

	/** High-contrast against typical terrain colors, intended for the minimap markers. */
	color_t rep() const { return rep_; }

	/** High-contrast to dark backgrounds, intended for the text in the chat log, etc. */
	color_t font_color() const { return font_color_; }

	bool operator==(const color_range& b) const
	{
		return mid_ == b.mid() && max_ == b.max() && min_ == b.min()
			&& rep_ == b.rep() && font_color_ == b.font_color();
	}

	/** Return a string describing the color range for debug output. */
	std::string debug() const;

private:
	color_t mid_ , max_ , min_ , rep_, font_color_;
};

/**
 * Creates a reference color palette from a color range.
 */
std::vector<color_t> generate_reference_palette(const color_range& cr);

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
color_mapping generate_color_mapping(const color_range& new_rgb, const std::vector<color_t>& old_rgb);
