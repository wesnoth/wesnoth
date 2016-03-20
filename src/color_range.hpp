/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
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

#ifndef COLOR_RANGE_H_INCLUDED
#define COLOR_RANGE_H_INCLUDED

//These macros interfere with MS VC++
#ifdef _MSC_VER
	#undef max
	#undef min
#endif

#include "global.hpp"
#include <map>
#include <string>
#include <vector>

#include <SDL_types.h>

/* Convert comma separated string into rgb values.
 * Return false and empty result on error.
 */
bool string2rgb(const std::string& s, std::vector<Uint32>& result);

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
  color_range(Uint32 mid , Uint32 max = 0x00FFFFFF , Uint32 min = 0x00000000 , Uint32 rep = 0x00808080):mid_(mid),max_(max),min_(min),rep_(rep){}

  /**
   * Constructor, which expects four reference RGB colors.
   * @param v STL vector with the four reference colors in order.
   */
  color_range(const std::vector<Uint32>& v)
    : mid_(v.size()     ? v[0] : 0x00808080),
      max_(v.size() > 1 ? v[1] : 0x00FFFFFF),
      min_(v.size() > 2 ? v[2] : 0x00000000),
      rep_(v.size() > 3 ? v[3] : mid_)
  {
  }

  /** Default constructor. */
  color_range() : mid_(0x00808080), max_(0x00FFFFFF), min_(0x00000000), rep_(0x00808080) {}

  /** Average color shade. */
  Uint32 mid() const{return(mid_);}
  /** Maximum color shade. */
  Uint32 max() const{return(max_);}
  /** Minimum color shade. */
  Uint32 min() const{return(min_);}
  /** High-contrast shade, intended for the minimap markers. */
  Uint32 rep() const{return(rep_);}

  bool operator<(const color_range& b) const
  {
    if(mid_ != b.mid()) return(mid_ < b.mid());
    if(max_ != b.max()) return(max_ < b.max());
    if(min_ != b.min()) return(min_ < b.min());
    return(rep_ < b.rep());
  }

  bool operator==(const color_range& b) const
  {
    return(mid_ == b.mid() && max_ == b.max() && min_ == b.min() && rep_ == b.rep());
  }

  int index() const; // the default team index for this color, or 0 for none

  /** Return a string describing the color range for debug output. */
  std::string debug() const;

private:
  Uint32 mid_ , max_ , min_ , rep_;
};

/**
 * Creates a reference color palette from a color range.
 */
std::vector<Uint32> palette(color_range cr);

/**
 * Converts a source palette using the specified color_range object.
 * This holds the main interface for range-based team coloring. The output is
 * used with the recolor_image() method to do the actual recoloring.
 * @param new_rgb Specifies parameters for the conversion.
 * @param old_rgb Source palette.
 * @return A STL map of colors, with the keys being source palette elements, and the values
 *         are the result of applying the color range conversion on it.
 */
std::map<Uint32, Uint32> recolor_range(const color_range& new_rgb, const std::vector<Uint32>& old_rgb);

/**
 * Converts a color value to WML text markup syntax for highlighting.
 * For example, 0x00CC00CC becomes "<204,0,204>".
 */
std::string rgb2highlight(Uint32 rgb);

/**
 * Converts a color value to WML text markup syntax for highlighting.
 * For example, 0x00CC00CC becomes "#CC00CC".
 */
std::string rgb2highlight_pango(Uint32 rgb);
#endif
