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

/** @file image_function.hpp */

#ifndef IMAGE_FUNCTION_HPP_INCLUDED
#define IMAGE_FUNCTION_HPP_INCLUDED

#include "sdl_utils.hpp"
#include <map>

namespace image {
/**
 * Base abstract class for an image-path function.
 * It actually just enforces the operator()() protocol.
 */
class function_base
{
public:
	virtual ~function_base() {}
	/**
	 * Applies the image-path function on the specified surface.
	 */
	virtual surface operator()(const surface& src) const = 0;
};

/**
 * Recolor (RC/TC/PAL) function.
 * It is used not only for color-range-based recoloring ("~RC(magenta>teal)")
 * but also for team-color-based color range selection and recoloring
 * ("~TC(3,magenta)") and palette switches ("~PAL(000000,005000 > FFFFFF,FF00FF)").
 */
class rc_function : public function_base
{
public:
	/**
	 * Default constructor.
	 */
	rc_function()
		: rc_map_()
	{}
	/**
	 * RC-map based constructor.
	 * @param recolor_map The palette switch map.
	 */
	rc_function(const std::map<Uint32, Uint32>& recolor_map)
		: rc_map_(recolor_map)
	{}
	virtual surface operator()(const surface& src) const;

	bool no_op() const { return rc_map_.empty(); }

	const std::map<Uint32, Uint32>& map() const { return rc_map_;}
	std::map<Uint32, Uint32>& map() { return rc_map_;}

private:
	std::map<Uint32, Uint32> rc_map_;
};

/**
 * Mirror (FL) function.
 */
class fl_function : public function_base
{
public:
	/**
	 * Constructor.
	 * @param horiz Horizontal mirror flag.
	 * @param vert  Vertical mirror flag.
	 */
	fl_function(bool horiz = false, bool vert = false)
		: horiz_(horiz)
		, vert_(vert)
	{}
	virtual surface operator()(const surface& src) const;

	void set_horiz(bool val)  { horiz_ = val; }
	void set_vert(bool val)   { vert_ = val; }
	bool get_horiz() const    { return horiz_; }
	bool get_vert() const     { return vert_; }
	/** Toggle horizontal mirror flag.
	 *  @return The new flag state after toggling. */
	bool toggle_horiz()       { return((horiz_ = !horiz_)); }
	/** Toggle vertical mirror flag.
	 *  @return The new flag state after toggling. */
	bool toggle_vert()        { return((vert_ = !vert_)); }

	bool no_op() const { return ((!horiz_) && (!vert_)); }

private:
	bool horiz_;
	bool vert_;
};

/**
 * Grayscale (GS) function.
 */
class gs_function : public function_base
{
public:
	gs_function() {}
	virtual surface operator()(const surface& src) const;
};

/**
 * Crop (CROP) function.
 */
class crop_function : public function_base
{
public:
	crop_function(const SDL_Rect& slice)
		: slice_(slice)
	{}
	virtual surface operator()(const surface& src) const;

private:
	SDL_Rect slice_;
};

/**
 * Scale (SCALE) function.
 */
class scale_function : public function_base
{
public:
	scale_function(int width, int height)
		: w_(width), h_(height)
	{}
	virtual surface operator()(const surface& src) const;

private:
	int w_, h_;
};

/**
 * Opacity (O) function
 */
class o_function : public function_base
{
public:
	o_function(float opacity)
		: opacity_(opacity)
	{}
	virtual surface operator()(const surface& src) const;

private:
	float opacity_;
};

/**
 * Color-shift (CS, R, G, B) function.
 */
class cs_function : public function_base
{
public:
	cs_function(int r, int g, int b)
		: r_(r), g_(g), b_(b)
	{}
	virtual surface operator()(const surface& src) const;

private:
	int r_, g_, b_;
};

/**
 * Gaussian-like blur (BL) function.
 */
class bl_function : public function_base
{
public:
	bl_function(int depth)
		: depth_(depth)
	{}
	virtual surface operator()(const surface& src) const;

private:
	int depth_;
};

/**
 * Overlay with ToD brightening (BRIGHTEN).
 */
struct brighten_function : function_base
{
	virtual surface operator()(const surface &src) const;
};

} /* end namespace image */

#endif /* !defined(IMAGE_FUNCTION_HPP_INCLUDED) */
