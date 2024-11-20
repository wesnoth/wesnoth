/*
	Copyright (C) 2006 - 2024
	by Jeremy Rosen <jeremy.rosen@enst-bretagne.fr>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/**
 *  @file
 *  Frame for unit's animation sequence.
 */

#pragma once

#include "units/frame_private.hpp"

#include "color.hpp"
#include "drawing_layer.hpp"
#include "halo.hpp"
#include "picture.hpp"
#include "utils/optional_fwd.hpp"

#include <boost/logic/tribool.hpp>

#include <chrono>

class config;

constexpr int get_abs_frame_layer(drawing_layer layer)
{
	return int(layer) - int(drawing_layer::unit_first);
}

/** All parameters from a frame at a given instant */
struct frame_parameters
{
	std::chrono::milliseconds duration{0};

	image::locator image;
	image::locator image_diagonal;

	std::string image_mod;
	std::string halo;

	int halo_x = 0;
	int halo_y = 0;

	std::string halo_mod;
	std::string sound;
	std::string text;

	utils::optional<color_t> text_color;
	utils::optional<color_t> blend_with;

	double blend_ratio = 0.0;
	double highlight_ratio = 1.0;
	double offset = 0.0;
	double submerge = 0.0;

	int x = 0;
	int y = 0;
	int directional_x = 0;
	int directional_y = 0;

	boost::tribool auto_vflip = boost::logic::indeterminate;
	boost::tribool auto_hflip = boost::logic::indeterminate;
	boost::tribool primary_frame = boost::logic::indeterminate;

	int drawing_layer = get_abs_frame_layer(drawing_layer::unit_default);
};

/**
 * Easily build frame parameters with the serialized constructors
 */
class frame_builder
{
public:
	frame_builder();
	frame_builder(const config& cfg, const std::string& frame_string = "");

	/** Allow easy chained modifications. Will raised assert if used after initialization */
	frame_builder& duration(const std::chrono::milliseconds& duration);
	frame_builder& image(const std::string& image, const std::string& image_mod = "");
	frame_builder& image_diagonal(const std::string& image_diagonal, const std::string& image_mod = "");
	frame_builder& sound(const std::string& sound);
	frame_builder& text(const std::string& text, const color_t text_color);
	frame_builder& halo(const std::string& halo, const std::string& halo_x, const std::string& halo_y, const std::string& halo_mod);
	frame_builder& blend(const std::string& blend_ratio, const color_t blend_color);
	frame_builder& highlight(const std::string& highlight);
	frame_builder& offset(const std::string& offset);
	frame_builder& submerge(const std::string& submerge);
	frame_builder& x(const std::string& x);
	frame_builder& y(const std::string& y);
	frame_builder& directional_x(const std::string& directional_x);
	frame_builder& directional_y(const std::string& directional_y);
	frame_builder& auto_vflip(const bool auto_vflip);
	frame_builder& auto_hflip(const bool auto_hflip);
	frame_builder& primary_frame(const bool primary_frame);
	frame_builder& drawing_layer(const std::string& drawing_layer);

private:
	friend class frame_parsed_parameters;

	std::chrono::milliseconds duration_;

	std::string image_;
	std::string image_diagonal_;
	std::string image_mod_;
	std::string halo_;
	std::string halo_x_;
	std::string halo_y_;
	std::string halo_mod_;
	std::string sound_;
	std::string text_;

	utils::optional<color_t> text_color_;
	utils::optional<color_t> blend_with_;

	std::string blend_ratio_;
	std::string highlight_ratio_;
	std::string offset_;
	std::string submerge_;
	std::string x_;
	std::string y_;
	std::string directional_x_;
	std::string directional_y_;

	boost::tribool auto_vflip_;
	boost::tribool auto_hflip_;
	boost::tribool primary_frame_;

	std::string drawing_layer_;
};

/**
 * Keep most parameters in a separate class to simplify the handling of the large
 * number of parameters between the frame level and animation level.
 */
class frame_parsed_parameters
{
public:
	frame_parsed_parameters(const frame_builder& builder = frame_builder(),
		const std::chrono::milliseconds& override_duration = std::chrono::milliseconds{0});

	void override(const std::chrono::milliseconds& duration,
		const std::string& highlight = "",
		const std::string& blend_ratio = "",
		color_t blend_color = {0,0,0},
		const std::string& offset = "",
		const std::string& layer = "",
		const std::string& modifiers = "");

	/** Getters for the different parameters */
	frame_parameters parameters(const std::chrono::milliseconds& current_time) const;

	const std::chrono::milliseconds& duration() const { return duration_; }
	bool does_not_change() const;
	bool need_update() const;

	/** Contents of frame in strings */
	std::vector<std::string> debug_strings() const;

private:
	std::chrono::milliseconds duration_;

	progressive_image image_;
	progressive_image image_diagonal_;

	std::string image_mod_;

	progressive_string halo_;
	progressive_int halo_x_;
	progressive_int halo_y_;

	std::string halo_mod_;
	std::string sound_;
	std::string text_;

	utils::optional<color_t> text_color_;
	utils::optional<color_t> blend_with_;

	progressive_double blend_ratio_;
	progressive_double highlight_ratio_;
	progressive_double offset_;
	progressive_double submerge_;
	progressive_int x_;
	progressive_int y_;
	progressive_int directional_x_;
	progressive_int directional_y_;

	boost::tribool auto_vflip_;
	boost::tribool auto_hflip_;
	boost::tribool primary_frame_;

	progressive_int drawing_layer_;
};

/** Describes a unit's animation sequence. */
class unit_frame
{
public:
	// Constructors
	unit_frame(const frame_builder& builder = frame_builder()) : builder_(builder) {}

	void redraw(const std::chrono::milliseconds& frame_time, bool on_start_time, bool in_scope_of_frame, const map_location& src, const map_location& dst,
		halo::handle& halo_id, halo::manager& halo_man, const frame_parameters& animation_val, const frame_parameters& engine_val) const;

	frame_parameters merge_parameters(const std::chrono::milliseconds& current_time,
		const frame_parameters& animation_val,
		const frame_parameters& engine_val = frame_parameters()) const;

	frame_parameters parameters(const std::chrono::milliseconds& current_time) const
	{
		return builder_.parameters(current_time);
	}

	frame_parameters end_parameters() const
	{
		return builder_.parameters(duration());
	}

	const std::chrono::milliseconds& duration() const
	{
		return builder_.duration();
	}

	bool does_not_change() const
	{
		return builder_.does_not_change();
	}

	bool need_update() const
	{
		return builder_.need_update();
	}

	std::vector<std::string> debug_strings() const
	{
		// Contents of frame in strings
		return builder_.debug_strings();
	}

	std::set<map_location> get_overlaped_hex(const std::chrono::milliseconds& frame_time, const map_location& src, const map_location& dst,
		const frame_parameters& animation_val, const frame_parameters& engine_val) const;

private:
	frame_parsed_parameters builder_;
};
