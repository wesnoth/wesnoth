/*
	Copyright (C) 2009 - 2024
	by Iris Morelle <shadowm2006@gmail.com>
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

#include "color_range.hpp"
#include "lua_jailbreak_exception.hpp"
#include "sdl/surface.hpp"
#include "sdl/utils.hpp"

#include <functional>
#include <map>
#include <memory>

namespace image {

class modification;

/**
 * A modified priority queue used to order image modifications.
 * The priorities for this queue are to order modifications by priority(),
 * then by the order they are added to the queue.
 */
class modification_queue
{
	// Invariant for this class:
	// At the beginning and end of each member function call, there
	// are no empty vectors in priorities_.
public:
	modification_queue()
		: priorities_()
	{
	}

	bool empty() const  { return priorities_.empty(); }
	void push(std::unique_ptr<modification> mod);
	void pop();
	std::size_t size() const;
	modification * top() const;

private:
	/** Map from a mod's priority() to the mods having that priority. */
	typedef std::map<int, std::vector<std::unique_ptr<modification>>, std::greater<int>> map_type;
	/** Map from a mod's priority() to the mods having that priority. */
	map_type priorities_;
};

/** Base abstract class for an image-path modification */
class modification
{
public:

	/** Exception thrown by the operator() when an error occurs. */
	struct imod_exception final
		: public lua_jailbreak_exception
	{
		/**
		 * Constructor.
		 *
		 * @pre message_stream.str()[message_stream.str().size() - 1] == '\n'
		 *
		 * @param message_stream  Stream with the error message regarding
		 *                        the failed operation.
		 */
		imod_exception(const std::stringstream& message_stream);

		/**
		 * Constructor.
		 *
		 * @pre message[message.size() - 1] == '\n'
		 *
		 * @param message         String with the error message regarding
		 *                        the failed operation.
		 */
		imod_exception(const std::string& message);

		~imod_exception() noexcept {}

		/** The error message regarding the failed operation. */
		const std::string message;

	private:

		IMPLEMENT_LUA_JAILBREAK_EXCEPTION(imod_exception)
	};

	/** Decodes modifications from a modification string */
	static modification_queue decode(const std::string&);

	virtual ~modification() {}

	/** Applies the image-path modification on the specified surface */
	virtual void operator()(surface& src) const = 0;

	/** Specifies the priority of the modification */
	virtual int priority() const { return 0; }
};

/**
 * Recolor (RC/TC/PAL) modification.
 * It is used not only for color-range-based recoloring ("~RC(magenta>teal)")
 * but also for team-color-based color range selection and recoloring
 * ("~TC(3,magenta)") and palette switches ("~PAL(000000,005000 > FFFFFF,FF00FF)").
 */
class rc_modification : public modification
{
public:
	/**
	 * Default constructor.
	 */
	rc_modification()
		: rc_map_()
	{}
	/**
	 * RC-map based constructor.
	 * @param recolor_map The palette switch map.
	 */
	rc_modification(const color_range_map& recolor_map)
		: rc_map_(recolor_map)
	{}
	virtual void operator()(surface& src) const override;

	// The rc modification has a higher priority
	virtual int priority() const override { return 1; }

	bool no_op() const { return rc_map_.empty(); }

	const color_range_map& map() const { return rc_map_;}
	color_range_map& map() { return rc_map_;}

private:
	color_range_map rc_map_;
};

/**
 * Mirror (FL) modification.
 */
class fl_modification : public modification
{
public:
	/**
	 * Constructor.
	 * @param horiz Horizontal mirror flag.
	 * @param vert  Vertical mirror flag.
	 */
	fl_modification(bool horiz = false, bool vert = false)
		: horiz_(horiz)
		, vert_(vert)
	{}
	virtual void operator()(surface& src) const override;

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
 * Rotate (ROTATE) modification.
 */
class rotate_modification : public modification
{
public:
	/**
	 * Constructor.
	 *
	 * @pre zoom >= offset   Otherwise the result will have empty pixels.
	 * @pre offset > 0       Otherwise the procedure will not return.
	 *
	 * @param degrees Amount of rotation (in degrees).
	 *                Positive values are clockwise; negative are counter-clockwise.
	 * @param zoom    The zoom level to calculate the rotation from.
	 *                Greater values result in better results and increased runtime.
	 *                This parameter will be ignored if @a degrees is a multiple of 90.
	 * @param offset  Determines the step size of the scanning of the zoomed source.
	 *                Different offsets can produce better results, try them out.
	 *                Greater values result in decreased runtime.
	 *                This parameter will be ignored if @a degrees is a multiple of 90.
	 *                If @a offset is greater than @a zoom the result will have empty pixels.
	 */
	rotate_modification(int degrees = 90, int zoom = 16, int offset = 8)
		: degrees_(degrees), zoom_(zoom), offset_(offset)
	{}
	virtual void operator()(surface& src) const override;

	bool no_op() const { return degrees_ % 360 == 0; }

private:
	int degrees_;
	int zoom_;
	int offset_;
};

/**
 * Grayscale (GS) modification.
 */
class gs_modification : public modification
{
public:
	virtual void operator()(surface& src) const override;
};

/**
 * Crop transparent padding (CROP_TRANSPARENCY) modification.
 */
class crop_transparency_modification : public modification
{
public:
	virtual void operator()(surface& src) const override;
};

/**
 * Black and white (BW) modification.
 */
class bw_modification : public modification
{
public:
	bw_modification(int threshold): threshold_(threshold) {}
	virtual void operator()(surface& src) const override;
private:
	int threshold_;
};

/**
 * Give to the image a sepia tint (SEPIA)
 */
struct sepia_modification : modification
{
	virtual void operator()(surface& src) const override;
};

/**
 * Make an image negative (NEG)
 */
class negative_modification : public modification
{
public:
	negative_modification(int r, int g, int b): red_(r), green_(g), blue_(b) {}
	virtual void operator()(surface& src) const override;
private:
	int red_, green_, blue_;
};

/**
 * Plot Alpha (Alpha) modification
 */
class plot_alpha_modification : public modification
{
public:
	virtual void operator()(surface& src) const override;
};

/**
 * Wipe Alpha (Wipe_Alpha) modification
 */
class wipe_alpha_modification : public modification
{
public:
	virtual void operator()(surface& src) const override;
};

/**
 * Adjust Alpha (ADJUST_ALPHA) modification
 */
class adjust_alpha_modification : public modification
{
public:
	adjust_alpha_modification(const std::string& formula)
		: formula_(formula)
	{}

	virtual void operator()(surface& src) const override;

private:
	std::string formula_;
};

/**
 * Adjust Channels (CHAN) modification
 */
class adjust_channels_modification : public modification
{
public:
	adjust_channels_modification(const std::vector<std::string>& formulas)
		: formulas_(formulas)
	{
		if(formulas_.empty()) {
			formulas_.push_back("red");
		}
		if(formulas_.size() == 1) {
			formulas_.push_back("green");
		}
		if(formulas_.size() == 2) {
			formulas_.push_back("blue");
		}
		if(formulas_.size() == 3) {
			formulas_.push_back("alpha");
		}
	}

	virtual void operator()(surface& src) const override;

private:
	std::vector<std::string> formulas_;
};

/**
 * Crop (CROP) modification.
 */
class crop_modification : public modification
{
public:
	crop_modification(const SDL_Rect& slice)
		: slice_(slice)
	{}
	virtual void operator()(surface& src) const override;

	const SDL_Rect& get_slice() const
	{
		return slice_;
	}

private:
	SDL_Rect slice_;
};

/**
 * Scale (BLIT) modification.
 */

class blit_modification : public modification
{
public:
	blit_modification(const surface& surf, int x, int y)
		: surf_(surf), x_(x), y_(y)
	{}
	virtual void operator()(surface& src) const override;

	const surface& get_surface() const
	{
		return surf_;
	}

	int get_x() const
	{
		return x_;
	}

	int get_y() const
	{
		return y_;
	}

private:
	surface surf_;
	int x_;
	int y_;
};

/**
 * Mask (MASK) modification.
 */

class mask_modification : public modification
{
public:
	mask_modification(const surface& mask, int x, int y)
		: mask_(mask), x_(x), y_(y)
	{}
	virtual void operator()(surface& src) const override;

	const surface& get_mask() const
	{
		return mask_;
	}

	int get_x() const
	{
		return x_;
	}

	int get_y() const
	{
		return y_;
	}

private:
	surface mask_;
	int x_;
	int y_;
};

/**
 * LIGHT (L) modification.
 */

class light_modification : public modification
{
public:
	light_modification(const surface& surf)
		: surf_(surf)
	{}
	virtual void operator()(surface& src) const override;

	const surface& get_surface() const
	{
		return surf_;
	}

private:
	surface surf_;
};

/**
 * Scaling (SCALE[_INTO], SCALE[_INTO]_SHARP) modifications.
 */
class scale_modification : public modification
{
public:
	// Bit-unique scaling flags
	enum SCALE_FLAGS : uint8_t {
		SCALE_LINEAR          = 0b00000,
		SCALE_SHARP           = 0b00001,
		FIT_TO_SIZE           = 0b00010,
		PRESERVE_ASPECT_RATIO = 0b00100,
		X_BY_FACTOR           = 0b01000,
		Y_BY_FACTOR           = 0b10000,
	};

	scale_modification(point target_size, uint8_t flags)
		: target_size_(target_size)
		, flags_(flags)
	{}

	virtual void operator()(surface& src) const override;

	int get_w() const { return target_size_.x; }
	int get_h() const { return target_size_.y; }

private:
	point target_size_{0,0};

	uint8_t flags_ = SCALE_LINEAR | FIT_TO_SIZE;
};

/**
 * xBRZ scale (xBRZ) modification
 */
class xbrz_modification : public modification
{
public:
	xbrz_modification(int z)
		: z_(z)
	{}

	virtual void operator()(surface& src) const override;

private:
	int z_;
};

/**
 * Opacity (O) modification
 */
class o_modification : public modification
{
public:
	o_modification(float opacity)
		: opacity_(opacity)
	{}
	virtual void operator()(surface& src) const override;

	float get_opacity() const
	{
		return opacity_;
	}

private:
	float opacity_;
};

/**
 * Color-shift (CS, R, G, B) modification.
 */
class cs_modification : public modification
{
public:
	cs_modification(int r, int g, int b)
		: r_(r), g_(g), b_(b)
	{}
	virtual void operator()(surface& src) const override;

	int get_r() const { return r_; }
	int get_g() const { return g_; }
	int get_b() const { return b_; }

private:
	int r_, g_, b_;
};

/**
 * Color blending (BLEND) modification
 */
class blend_modification : public modification
{
public:
	blend_modification(int r, int g, int b, float a)
		: r_(r), g_(g), b_(b), a_(a)
	{}
	virtual void operator()(surface& src) const override;

	int   get_r() const { return r_; }
	int   get_g() const { return g_; }
	int   get_b() const { return b_; }
	float get_a() const { return a_; }

private:
	int r_, g_, b_;
	float a_;
};

/**
 * Gaussian-like blur (BL) modification.
 */
class bl_modification : public modification
{
public:
	bl_modification(int depth)
		: depth_(depth)
	{}
	virtual void operator()(surface& src) const override;

	int get_depth() const
	{
		return depth_;
	}

private:
	int depth_;
};

/**
 * Fill background with a color (BG).
 */
struct background_modification : modification
{
	background_modification(const color_t& c): color_(c) {}
	virtual void operator()(surface& src) const override;

	const color_t& get_color() const
	{
		return color_;
	}

private:
	color_t color_;
};

/**
 * Channel swap (SWAP).
 */
class swap_modification : public modification
{
public:
	swap_modification(channel r, channel g, channel b, channel a): red_(r), green_(g), blue_(b), alpha_(a) {}
	virtual void operator()(surface& src) const override;
private:
	channel red_;
	channel green_;
	channel blue_;
	channel alpha_;
};

} /* end namespace image */
