/*
   Copyright (C) 2007 - 2018 by Mark de Wever <koraq@xs4all.nl>
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
 * @file
 * Implementation of canvas.hpp.
 */

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/core/canvas.hpp"
#include "gui/core/canvas_private.hpp"

#include "font/text.hpp"
#include "formatter.hpp"
#include "gettext.hpp"
#include "picture.hpp"

#include "gui/auxiliary/typed_formula.hpp"
#include "gui/core/log.hpp"
#include "gui/widgets/helper.hpp"
#include "sdl/rect.hpp"
#include "video.hpp"
#include "wml_exception.hpp"

namespace gui2
{

namespace
{

/***** ***** ***** ***** ***** DRAWING PRIMITIVES ***** ***** ***** ***** *****/

static void set_renderer_color(SDL_Renderer* renderer, color_t color)
{
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

/**
 * Draws a line on a surface.
 *
 * @pre                   The caller needs to make sure the entire line fits on
 *                        the @p surface.
 * @pre                   @p x2 >= @p x1
 * @pre                   The @p surface is locked.
 *
 * @param canvas          The canvas to draw upon, the caller should lock the
 *                        surface before calling.
 * @param color           The color of the line to draw.
 * @param x1              The start x coordinate of the line to draw.
 * @param y1              The start y coordinate of the line to draw.
 * @param x2              The end x coordinate of the line to draw.
 * @param y2              The end y coordinate of the line to draw.
 */
static void draw_line(surface& canvas,
					  SDL_Renderer* renderer,
					  color_t color,
					  unsigned x1,
					  unsigned y1,
					  const unsigned x2,
					  unsigned y2)
{
	unsigned w = canvas->w;

	DBG_GUI_D << "Shape: draw line from " << x1 << ',' << y1 << " to " << x2
			  << ',' << y2 << " canvas width " << w << " canvas height "
			  << canvas->h << ".\n";

	assert(static_cast<int>(x1) < canvas->w);
	assert(static_cast<int>(x2) < canvas->w);
	assert(static_cast<int>(y1) < canvas->h);
	assert(static_cast<int>(y2) < canvas->h);

	set_renderer_color(renderer, color);

	if(x1 == x2 && y1 == y2) {
		// Handle single-pixel lines properly
		SDL_RenderDrawPoint(renderer, x1, y1);
	} else {
		SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
	}
}

/**
 * Draws a circle on a surface.
 *
 * @pre                   The circle must fit on the canvas.
 * @pre                   The @p surface is locked.
 *
 * @param canvas          The canvas to draw upon, the caller should lock the
 *                        surface before calling.
 * @param color           The border color of the circle to draw.
 * @param x_center        The x coordinate of the center of the circle to draw.
 * @param y_center        The y coordinate of the center of the circle to draw.
 * @param radius          The radius of the circle to draw.
 * @tparam octants        A bitfield indicating which octants to draw, starting at twelve o'clock and moving clockwise.
 */
template<unsigned int octants = 0xff>
static void draw_circle(surface& canvas,
						SDL_Renderer* renderer,
						color_t color,
						const int x_center,
						const int y_center,
						const int radius)
{
	unsigned w = canvas->w;

	DBG_GUI_D << "Shape: draw circle at " << x_center << ',' << y_center
			  << " with radius " << radius << " canvas width " << w
			  << " canvas height " << canvas->h << ".\n";

	if(octants & 0x0f) assert((x_center + radius) < canvas->w);
	if(octants & 0xf0) assert((x_center - radius) >= 0);
	if(octants & 0x3c) assert((y_center + radius) < canvas->h);
	if(octants & 0xc3) assert((y_center - radius) >= 0);

	set_renderer_color(renderer, color);

	// Algorithm based on
	// http://de.wikipedia.org/wiki/Rasterung_von_Kreisen#Methode_von_Horn
	// version of 2011.02.07.
	int d = -static_cast<int>(radius);
	int x = radius;
	int y = 0;

	std::vector<SDL_Point> points;

	while(!(y > x)) {
		if(octants & 0x04) points.push_back({x_center + x, y_center + y});
		if(octants & 0x02) points.push_back({x_center + x, y_center - y});
		if(octants & 0x20) points.push_back({x_center - x, y_center + y});
		if(octants & 0x40) points.push_back({x_center - x, y_center - y});

		if(octants & 0x08) points.push_back({x_center + y, y_center + x});
		if(octants & 0x01) points.push_back({x_center + y, y_center - x});
		if(octants & 0x10) points.push_back({x_center - y, y_center + x});
		if(octants & 0x80) points.push_back({x_center - y, y_center - x});

		d += 2 * y + 1;
		++y;
		if(d > 0) {
			d += -2 * x + 2;
			--x;
		}
	}

	SDL_RenderDrawPoints(renderer, points.data(), points.size());
}

/**
 * Draws a filled circle on a surface.
 *
 * @pre                   The circle must fit on the canvas.
 * @pre                   The @p surface is locked.
 *
 * @param canvas          The canvas to draw upon, the caller should lock the
 *                        surface before calling.
 * @param color           The fill color of the circle to draw.
 * @param x_center        The x coordinate of the center of the circle to draw.
 * @param y_center        The y coordinate of the center of the circle to draw.
 * @param radius          The radius of the circle to draw.
 * @tparam octants        A bitfield indicating which octants to draw, starting at twelve o'clock and moving clockwise.
 */
template<unsigned int octants = 0xff>
static void fill_circle(surface& canvas,
						SDL_Renderer* renderer,
						color_t color,
						const int x_center,
						const int y_center,
						const int radius)
{
	unsigned w = canvas->w;

	DBG_GUI_D << "Shape: draw filled circle at " << x_center << ',' << y_center
			  << " with radius " << radius << " canvas width " << w
			  << " canvas height " << canvas->h << ".\n";

	if(octants & 0x0f) assert((x_center + radius) < canvas->w);
	if(octants & 0xf0) assert((x_center - radius) >= 0);
	if(octants & 0x3c) assert((y_center + radius) < canvas->h);
	if(octants & 0xc3) assert((y_center - radius) >= 0);

	set_renderer_color(renderer, color);

	int d = -static_cast<int>(radius);
	int x = radius;
	int y = 0;

	while(!(y > x)) {
		// I use the formula of Bresenham's line algorithm to determine the boundaries of a segment.
		// The slope of the line is always 1 or -1 in this case.
		if(octants & 0x04) SDL_RenderDrawLine(renderer, x_center + x,     y_center + y + 1, x_center + y + 1, y_center + y + 1); // x2 - 1 = y2 - (y_center + 1) + x_center
		if(octants & 0x02) SDL_RenderDrawLine(renderer, x_center + x,     y_center - y,     x_center + y + 1, y_center - y);     // x2 - 1 = y_center - y2 + x_center
		if(octants & 0x20) SDL_RenderDrawLine(renderer, x_center - x - 1, y_center + y + 1, x_center - y - 2, y_center + y + 1); // x2 + 1 = (y_center + 1) - y2 + (x_center - 1)
		if(octants & 0x40) SDL_RenderDrawLine(renderer, x_center - x - 1, y_center - y,     x_center - y - 2, y_center - y);     // x2 + 1 = y2 - y_center + (x_center - 1)

		if(octants & 0x08) SDL_RenderDrawLine(renderer, x_center + y,     y_center + x + 1, x_center + y,     y_center + y + 1); // y2 = x2 - x_center + (y_center + 1)
		if(octants & 0x01) SDL_RenderDrawLine(renderer, x_center + y,     y_center - x,     x_center + y,     y_center - y);     // y2 = x_center - x2 + y_center
		if(octants & 0x10) SDL_RenderDrawLine(renderer, x_center - y - 1, y_center + x + 1, x_center - y - 1, y_center + y + 1); // y2 = (x_center - 1) - x2 + (y_center + 1)
		if(octants & 0x80) SDL_RenderDrawLine(renderer, x_center - y - 1, y_center - x,     x_center - y - 1, y_center - y);     // y2 = x2 - (x_center - 1) + y_center

		d += 2 * y + 1;
		++y;
		if(d > 0) {
			d += -2 * x + 2;
			--x;
		}
	}
}

} // namespace


/***** ***** ***** ***** ***** LINE ***** ***** ***** ***** *****/

line_shape::line_shape(const config& cfg)
	: shape(cfg)
	, x1_(cfg["x1"])
	, y1_(cfg["y1"])
	, x2_(cfg["x2"])
	, y2_(cfg["y2"])
	, color_(cfg["color"])
	, thickness_(cfg["thickness"])
{
	const std::string& debug = (cfg["debug"]);
	if(!debug.empty()) {
		DBG_GUI_P << "Line: found debug message '" << debug << "'.\n";
	}
}

void line_shape::draw(surface& canvas,
				 SDL_Renderer* renderer,
				 wfl::map_formula_callable& variables)
{
	/**
	 * @todo formulas are now recalculated every draw cycle which is a bit silly
	 * unless there has been a resize. So to optimize we should use an extra
	 * flag or do the calculation in a separate routine.
	 */

	const unsigned x1 = x1_(variables);
	const unsigned y1 = y1_(variables);
	const unsigned x2 = x2_(variables);
	const unsigned y2 = y2_(variables);

	DBG_GUI_D << "Line: draw from " << x1 << ',' << y1 << " to " << x2 << ','
			  << y2 << " canvas size " << canvas->w << ',' << canvas->h
			  << ".\n";

	VALIDATE(static_cast<int>(x1) < canvas->w
			 && static_cast<int>(x2) < canvas->w
			 && static_cast<int>(y1) < canvas->h
			 && static_cast<int>(y2) < canvas->h,
			 _("Line doesn't fit on canvas."));

	// @todo FIXME respect the thickness.

	// lock the surface
	surface_lock locker(canvas);

	draw_line(canvas, renderer, color_(variables), x1, y1, x2, y2);
}

/***** ***** ***** ***** ***** Rectangle ***** ***** ***** ***** *****/

rectangle_shape::rectangle_shape(const config& cfg)
	: shape(cfg)
	, x_(cfg["x"])
	, y_(cfg["y"])
	, w_(cfg["w"])
	, h_(cfg["h"])
	, border_thickness_(cfg["border_thickness"])
	, border_color_(cfg["border_color"], color_t::null_color())
	, fill_color_(cfg["fill_color"], color_t::null_color())
{
	// Check if a raw color string evaluates to a null color.
	if(!border_color_.has_formula() && border_color_().null()) {
		border_thickness_ = 0;
	}

	const std::string& debug = (cfg["debug"]);
	if(!debug.empty()) {
		DBG_GUI_P << "Rectangle: found debug message '" << debug << "'.\n";
	}
}

void rectangle_shape::draw(surface& canvas,
					  SDL_Renderer* renderer,
					  wfl::map_formula_callable& variables)
{
	/**
	 * @todo formulas are now recalculated every draw cycle which is a  bit
	 * silly unless there has been a resize. So to optimize we should use an
	 * extra flag or do the calculation in a separate routine.
	 */
	const int x = x_(variables);
	const int y = y_(variables);
	const int w = w_(variables);
	const int h = h_(variables);

	DBG_GUI_D << "Rectangle: draw from " << x << ',' << y << " width " << w
			  << " height " << h << " canvas size " << canvas->w << ','
			  << canvas->h << ".\n";

	VALIDATE(x     <  canvas->w
	      && x + w <= canvas->w
	      && y     <  canvas->h
	      && y + h <= canvas->h, _("Rectangle doesn't fit on canvas."));

	surface_lock locker(canvas);

	const color_t fill_color = fill_color_(variables);

	// Fill the background, if applicable
	if(!fill_color.null() && w && h) {
		set_renderer_color(renderer, fill_color);

		SDL_Rect area {
			x +  border_thickness_,
			y +  border_thickness_,
			w - (border_thickness_ * 2),
			h - (border_thickness_ * 2)
		};

		SDL_RenderFillRect(renderer, &area);
	}

	// Draw the border
	for(int i = 0; i < border_thickness_; ++i) {
		SDL_Rect dimensions {
			x + i,
			y + i,
			w - (i * 2),
			h - (i * 2)
		};

		set_renderer_color(renderer, border_color_(variables));

		SDL_RenderDrawRect(renderer, &dimensions);
	}
}

/***** ***** ***** ***** ***** Rounded Rectangle ***** ***** ***** ***** *****/

round_rectangle_shape::round_rectangle_shape(const config& cfg)
	: shape(cfg)
	, x_(cfg["x"])
	, y_(cfg["y"])
	, w_(cfg["w"])
	, h_(cfg["h"])
	, r_(cfg["corner_radius"])
	, border_thickness_(cfg["border_thickness"])
	, border_color_(cfg["border_color"], color_t::null_color())
	, fill_color_(cfg["fill_color"], color_t::null_color())
{
	// Check if a raw color string evaluates to a null color.
	if(!border_color_.has_formula() && border_color_().null()) {
		border_thickness_ = 0;
	}

	const std::string& debug = (cfg["debug"]);
	if(!debug.empty()) {
		DBG_GUI_P << "Rounded Rectangle: found debug message '" << debug << "'.\n";
	}
}

void round_rectangle_shape::draw(surface& canvas,
	SDL_Renderer* renderer,
	wfl::map_formula_callable& variables)
{
	/**
	 * @todo formulas are now recalculated every draw cycle which is a  bit
	 * silly unless there has been a resize. So to optimize we should use an
	 * extra flag or do the calculation in a separate routine.
	 */
	const int x = x_(variables);
	const int y = y_(variables);
	const int w = w_(variables);
	const int h = h_(variables);
	const int r = r_(variables);

	DBG_GUI_D << "Rounded Rectangle: draw from " << x << ',' << y << " width " << w
		<< " height " << h << " canvas size " << canvas->w << ','
		<< canvas->h << ".\n";

	VALIDATE(x     <  canvas->w
		&& x + w <= canvas->w
		&& y     <  canvas->h
		&& y + h <= canvas->h, _("Rounded Rectangle doesn't fit on canvas."));

	surface_lock locker(canvas);

	const color_t fill_color = fill_color_(variables);

	// Fill the background, if applicable
	if(!fill_color.null() && w && h) {
		set_renderer_color(renderer, fill_color);
		static const int count = 3;
		SDL_Rect area[count] {
			{x + r,                 y + border_thickness_, w - r                 * 2, r - border_thickness_ + 1},
			{x + border_thickness_, y + r + 1,             w - border_thickness_ * 2, h - r * 2},
			{x + r,                 y - r + h + 1,         w - r                 * 2, r - border_thickness_},
		};

		SDL_RenderFillRects(renderer, area, count);

		fill_circle<0xc0>(canvas, renderer, fill_color, x + r,     y + r,     r);
		fill_circle<0x03>(canvas, renderer, fill_color, x + w - r, y + r,     r);
		fill_circle<0x30>(canvas, renderer, fill_color, x + r,     y + h - r, r);
		fill_circle<0x0c>(canvas, renderer, fill_color, x + w - r, y + h - r, r);
	}

	const color_t border_color = border_color_(variables);

	// Draw the border
	for(int i = 0; i < border_thickness_; ++i) {
		set_renderer_color(renderer, border_color);

		SDL_RenderDrawLine(renderer, x + r, y + i,     x + w - r, y + i);
		SDL_RenderDrawLine(renderer, x + r, y + h - i, x + w - r, y + h - i);

		SDL_RenderDrawLine(renderer, x + i,     y + r, x + i,     y + h - r);
		SDL_RenderDrawLine(renderer, x + w - i, y + r, x + w - i, y + h - r);

		draw_circle<0xc0>(canvas, renderer, border_color, x + r,     y + r,     r - i);
		draw_circle<0x03>(canvas, renderer, border_color, x + w - r, y + r,     r - i);
		draw_circle<0x30>(canvas, renderer, border_color, x + r,     y + h - r, r - i);
		draw_circle<0x0c>(canvas, renderer, border_color, x + w - r, y + h - r, r - i);
	}
}

/***** ***** ***** ***** ***** CIRCLE ***** ***** ***** ***** *****/

circle_shape::circle_shape(const config& cfg)
	: shape(cfg)
	, x_(cfg["x"])
	, y_(cfg["y"])
	, radius_(cfg["radius"])
	, border_color_(cfg["border_color"])
	, fill_color_(cfg["fill_color"])
	, border_thickness_(cfg["border_thickness"].to_int(1))
{
	const std::string& debug = (cfg["debug"]);
	if(!debug.empty()) {
		DBG_GUI_P << "Circle: found debug message '" << debug << "'.\n";
	}
}

void circle_shape::draw(surface& canvas,
				   SDL_Renderer* renderer,
				   wfl::map_formula_callable& variables)
{
	/**
	 * @todo formulas are now recalculated every draw cycle which is a bit
	 * silly unless there has been a resize. So to optimize we should use an
	 * extra flag or do the calculation in a separate routine.
	 */

	const unsigned x = x_(variables);
	const unsigned y = y_(variables);
	const unsigned radius = radius_(variables);

	DBG_GUI_D << "Circle: drawn at " << x << ',' << y << " radius " << radius
			  << " canvas size " << canvas->w << ',' << canvas->h << ".\n";

	VALIDATE_WITH_DEV_MESSAGE(
			static_cast<int>(x - radius) >= 0,
			_("Circle doesn't fit on canvas."),
			formatter() << "x = " << x << ", radius = " << radius);

	VALIDATE_WITH_DEV_MESSAGE(
			static_cast<int>(y - radius) >= 0,
			_("Circle doesn't fit on canvas."),
			formatter() << "y = " << y << ", radius = " << radius);

	VALIDATE_WITH_DEV_MESSAGE(
			static_cast<int>(x + radius) < canvas->w,
			_("Circle doesn't fit on canvas."),
			formatter() << "x = " << x << ", radius = " << radius
						 << "', canvas width = " << canvas->w << ".");

	VALIDATE_WITH_DEV_MESSAGE(
			static_cast<int>(y + radius) < canvas->h,
			_("Circle doesn't fit on canvas."),
			formatter() << "y = " << y << ", radius = " << radius
						 << "', canvas height = " << canvas->h << ".");

	// lock the surface
	surface_lock locker(canvas);

	const color_t fill_color = fill_color_(variables);
	if(!fill_color.null() && radius) {
		fill_circle(canvas, renderer, fill_color, x, y, radius);
	}

	const color_t border_color = border_color_(variables);
	for(unsigned int i = 0; i < border_thickness_; i++) {
		draw_circle(canvas, renderer, border_color, x, y, radius - i);
	}
}

/***** ***** ***** ***** ***** IMAGE ***** ***** ***** ***** *****/

image_shape::image_shape(const config& cfg, wfl::action_function_symbol_table& functions)
	: shape(cfg)
	, x_(cfg["x"])
	, y_(cfg["y"])
	, w_(cfg["w"])
	, h_(cfg["h"])
	, src_clip_()
	, image_()
	, image_name_(cfg["name"])
	, resize_mode_(get_resize_mode(cfg["resize_mode"]))
	, vertical_mirror_(cfg["vertical_mirror"])
	, actions_formula_(cfg["actions"], &functions)
{
	const std::string& debug = (cfg["debug"]);
	if(!debug.empty()) {
		DBG_GUI_P << "Image: found debug message '" << debug << "'.\n";
	}
}

void image_shape::dimension_validation(unsigned value, const std::string& name, const std::string& key)
{
	const int as_int = static_cast<int>(value);

	VALIDATE_WITH_DEV_MESSAGE(as_int >= 0, _("Image doesn't fit on canvas."),
		formatter() << "Image '" << name << "', " << key << " = " << as_int << "."
	);
}

void image_shape::draw(surface& canvas,
				  SDL_Renderer* /*renderer*/,
				  wfl::map_formula_callable& variables)
{
	DBG_GUI_D << "Image: draw.\n";

	/**
	 * @todo formulas are now recalculated every draw cycle which is a  bit
	 * silly unless there has been a resize. So to optimize we should use an
	 * extra flag or do the calculation in a separate routine.
	 */
	const std::string& name = image_name_(variables);

	if(name.empty()) {
		DBG_GUI_D << "Image: formula returned no value, will not be drawn.\n";
		return;
	}

	/*
	 * The locator might return a different surface for every call so we can't
	 * cache the output, also not if no formula is used.
	 */
	surface tmp(image::get_image(image::locator(name)));

	if(!tmp) {
		ERR_GUI_D << "Image: '" << name << "' not found and won't be drawn." << std::endl;
		return;
	}

	image_ = tmp;
	assert(image_);
	src_clip_ = {0, 0, image_->w, image_->h};

	wfl::map_formula_callable local_variables(variables);
	local_variables.add("image_original_width", wfl::variant(image_->w));
	local_variables.add("image_original_height", wfl::variant(image_->h));

	unsigned w = w_(local_variables);
	dimension_validation(w, name, "w");

	unsigned h = h_(local_variables);
	dimension_validation(h, name, "h");

	local_variables.add("image_width", wfl::variant(w ? w : image_->w));
	local_variables.add("image_height", wfl::variant(h ? h : image_->h));

	const unsigned clip_x = x_(local_variables);
	dimension_validation(clip_x, name, "x");

	const unsigned clip_y = y_(local_variables);
	dimension_validation(clip_y, name, "y");

	local_variables.add("clip_x", wfl::variant(clip_x));
	local_variables.add("clip_y", wfl::variant(clip_y));

	// Execute the provided actions for this context.
	wfl::variant(variables.fake_ptr()).execute_variant(actions_formula_.evaluate(local_variables));

	// Copy the data to local variables to avoid overwriting the originals.
	SDL_Rect src_clip = src_clip_;
	SDL_Rect dst_clip = sdl::create_rect(clip_x, clip_y, 0, 0);
	surface surf;

	// Test whether we need to scale and do the scaling if needed.
	if ((w == 0) && (h == 0)) {
		surf = image_;
	}
	else { // assert((w != 0) || (h != 0))
		if(w == 0 && resize_mode_ == stretch) {
			DBG_GUI_D << "Image: vertical stretch from " << image_->w << ','
					  << image_->h << " to a height of " << h << ".\n";

			surf = stretch_surface_vertical(image_, h);
			w = image_->w;
		}
		else if(h == 0 && resize_mode_ == stretch) {
			DBG_GUI_D << "Image: horizontal stretch from " << image_->w
					  << ',' << image_->h << " to a width of " << w
					  << ".\n";

			surf = stretch_surface_horizontal(image_, w);
			h = image_->h;
		}
		else {
			if(w == 0) {
				w = image_->w;
			}
			if(h == 0) {
				h = image_->h;
			}
			if(resize_mode_ == tile) {
				DBG_GUI_D << "Image: tiling from " << image_->w << ','
						  << image_->h << " to " << w << ',' << h << ".\n";

				surf = tile_surface(image_, w, h, false);
			} else if(resize_mode_ == tile_center) {
				DBG_GUI_D << "Image: tiling centrally from " << image_->w << ','
						  << image_->h << " to " << w << ',' << h << ".\n";

				surf = tile_surface(image_, w, h, true);
			} else {
				if(resize_mode_ == stretch) {
					ERR_GUI_D << "Image: failed to stretch image, "
								 "fall back to scaling.\n";
				}

				DBG_GUI_D << "Image: scaling from " << image_->w << ','
						  << image_->h << " to " << w << ',' << h << ".\n";

				surf = scale_surface_legacy(image_, w, h);
			}
		}
		src_clip.w = w;
		src_clip.h = h;
	}

	if(vertical_mirror_(local_variables)) {
		surf = flip_surface(surf);
	}

	blit_surface(surf, &src_clip, canvas, &dst_clip);
}

image_shape::resize_mode image_shape::get_resize_mode(const std::string& resize_mode)
{
	if(resize_mode == "tile") {
		return image_shape::tile;
	} else if(resize_mode == "tile_center") {
		return image_shape::tile_center;
	} else if(resize_mode == "stretch") {
		return image_shape::stretch;
	} else {
		if(!resize_mode.empty() && resize_mode != "scale") {
			ERR_GUI_E << "Invalid resize mode '" << resize_mode
					  << "' falling back to 'scale'.\n";
		}
		return image_shape::scale;
	}
}

/***** ***** ***** ***** ***** TEXT ***** ***** ***** ***** *****/

text_shape::text_shape(const config& cfg)
	: shape(cfg)
	, x_(cfg["x"])
	, y_(cfg["y"])
	, w_(cfg["w"])
	, h_(cfg["h"])
	, font_family_(font::str_to_family_class(cfg["font_family"]))
	, font_size_(cfg["font_size"])
	, font_style_(decode_font_style(cfg["font_style"]))
	, text_alignment_(cfg["text_alignment"])
	, color_(cfg["color"])
	, text_(cfg["text"])
	, text_markup_(cfg["text_markup"], false)
	, link_aware_(cfg["text_link_aware"], false)
	, link_color_(cfg["text_link_color"], color_t::from_hex_string("ffff00"))
	, maximum_width_(cfg["maximum_width"], -1)
	, characters_per_line_(cfg["text_characters_per_line"])
	, maximum_height_(cfg["maximum_height"], -1)
{
	if(!font_size_.has_formula()) {
		VALIDATE(font_size_(), _("Text has a font size of 0."));
	}

	const std::string& debug = (cfg["debug"]);
	if(!debug.empty()) {
		DBG_GUI_P << "Text: found debug message '" << debug << "'.\n";
	}
}

void text_shape::draw(surface& canvas,
				 SDL_Renderer* /*renderer*/,
				 wfl::map_formula_callable& variables)
{
	assert(variables.has_key("text"));

	// We first need to determine the size of the text which need the rendered
	// text. So resolve and render the text first and then start to resolve
	// the other formulas.
	const t_string text = text_(variables);

	if(text.empty()) {
		DBG_GUI_D << "Text: no text to render, leave.\n";
		return;
	}

	font::pango_text& text_renderer = font::get_text_renderer();

	text_renderer
		.set_link_aware(link_aware_(variables))
		.set_link_color(link_color_(variables))
		.set_text(text, text_markup_(variables));

	text_renderer.set_family_class(font_family_)
		.set_font_size(font_size_(variables))
		.set_font_style(font_style_)
		.set_alignment(text_alignment_(variables))
		.set_foreground_color(color_(variables))
		.set_maximum_width(maximum_width_(variables))
		.set_maximum_height(maximum_height_(variables), true)
		.set_ellipse_mode(variables.has_key("text_wrap_mode")
				? static_cast<PangoEllipsizeMode>(variables.query_value("text_wrap_mode").as_int())
				: PANGO_ELLIPSIZE_END)
		.set_characters_per_line(characters_per_line_);

	surface& surf = text_renderer.render();
	if(surf->w == 0) {
		DBG_GUI_D << "Text: Rendering '" << text
				  << "' resulted in an empty canvas, leave.\n";
		return;
	}

	wfl::map_formula_callable local_variables(variables);
	local_variables.add("text_width", wfl::variant(surf->w));
	local_variables.add("text_height", wfl::variant(surf->h));
	/*
		std::cerr << "Text: drawing text '" << text
			<< " maximum width " << maximum_width_(variables)
			<< " maximum height " << maximum_height_(variables)
			<< " text width " << surf->w
			<< " text height " << surf->h;
	*/
	// TODO: formulas are now recalculated every draw cycle which is a
	// bit silly unless there has been a resize. So to optimize we should
	// use an extra flag or do the calculation in a separate routine.

	const unsigned x = x_(local_variables);
	const unsigned y = y_(local_variables);
	const unsigned w = w_(local_variables);
	const unsigned h = h_(local_variables);

	DBG_GUI_D << "Text: drawing text '" << text << "' drawn from " << x << ','
			  << y << " width " << w << " height " << h << " canvas size "
			  << canvas->w << ',' << canvas->h << ".\n";

	VALIDATE(static_cast<int>(x) < canvas->w && static_cast<int>(y) < canvas->h,
			 _("Text doesn't start on canvas."));

	// A text might be to long and will be clipped.
	if(surf->w > static_cast<int>(w)) {
		WRN_GUI_D << "Text: text is too wide for the "
					 "canvas and will be clipped.\n";
	}

	if(surf->h > static_cast<int>(h)) {
		WRN_GUI_D << "Text: text is too high for the "
					 "canvas and will be clipped.\n";
	}

	SDL_Rect dst = sdl::create_rect(x, y, canvas->w, canvas->h);
	blit_surface(surf, nullptr, canvas, &dst);
}

/***** ***** ***** ***** ***** CANVAS ***** ***** ***** ***** *****/

canvas::canvas()
	: shapes_()
	, drawn_shapes_()
	, blur_depth_(0)
	, w_(0)
	, h_(0)
	, canvas_()
	, renderer_(nullptr)
	, variables_()
	, functions_()
	, is_dirty_(true)
{
}

canvas::canvas(canvas&& c) noexcept
	: shapes_(std::move(c.shapes_))
	, drawn_shapes_(std::move(c.drawn_shapes_))
	, blur_depth_(c.blur_depth_)
	, w_(c.w_)
	, h_(c.h_)
	, canvas_(std::move(c.canvas_))
	, renderer_(std::exchange(c.renderer_, nullptr))
	, variables_(c.variables_)
	, functions_(c.functions_)
	, is_dirty_(c.is_dirty_)
{
}

canvas::~canvas()
{
	if(renderer_)
		SDL_DestroyRenderer(renderer_);
}

void canvas::draw(const bool force)
{
	log_scope2(log_gui_draw, "Canvas: drawing.");
	if(!is_dirty_ && !force) {
		DBG_GUI_D << "Canvas: nothing to draw.\n";
		return;
	}

	if(is_dirty_) {
		get_screen_size_variables(variables_);
		variables_.add("width", wfl::variant(w_));
		variables_.add("height", wfl::variant(h_));
	}

	if(canvas_) {
		DBG_GUI_D << "Canvas: use cached canvas.\n";
	} else {
		// create surface
		DBG_GUI_D << "Canvas: create new empty canvas.\n";
		canvas_ = surface(w_, h_);
	}

	if(renderer_) {
		SDL_DestroyRenderer(renderer_);
	}

	renderer_ = SDL_CreateSoftwareRenderer(canvas_);
	SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

	// draw items
	for(auto& shape : shapes_) {
		lg::scope_logger inner_scope_logging_object__(log_gui_draw, "Canvas: draw shape.");

		shape->draw(canvas_, renderer_, variables_);
	}

	// The shapes have been drawn and the draw result has been cached. Clear the list.
	std::copy(shapes_.begin(), shapes_.end(), std::back_inserter(drawn_shapes_));
	shapes_.clear();

	SDL_RenderPresent(renderer_);

	is_dirty_ = false;
}

void canvas::blit(surface& surf, SDL_Rect rect)
{
	draw();

	if(blur_depth_) {
		/*
		 * If the surf is the video surface the blurring seems to stack, this
		 * can be seen in the title screen. So also use the not 32 bpp method
		 * for this situation.
		 */
		if(surf != CVideo::get_singleton().getSurface() && surf.is_neutral()) {
			blur_surface(surf, rect, blur_depth_);
		} else {
			// Can't directly blur the surface if not 32 bpp.
			SDL_Rect r = rect;
			surface s = get_surface_portion(surf, r);
			s = blur_surface(s, blur_depth_);
			sdl_blit(s, nullptr, surf, &r);
		}
	}

	sdl_blit(canvas_, nullptr, surf, &rect);
}

void canvas::parse_cfg(const config& cfg)
{
	log_scope2(log_gui_parse, "Canvas: parsing config.");

	for(const auto & shape : cfg.all_children_range())
	{
		const std::string& type = shape.key;
		const config& data = shape.cfg;

		DBG_GUI_P << "Canvas: found shape of the type " << type << ".\n";

		if(type == "line") {
			shapes_.emplace_back(std::make_shared<line_shape>(data));
		} else if(type == "rectangle") {
			shapes_.emplace_back(std::make_shared<rectangle_shape>(data));
		} else if(type == "round_rectangle") {
			shapes_.emplace_back(std::make_shared<round_rectangle_shape>(data));
		} else if(type == "circle") {
			shapes_.emplace_back(std::make_shared<circle_shape>(data));
		} else if(type == "image") {
			shapes_.emplace_back(std::make_shared<image_shape>(data, functions_));
		} else if(type == "text") {
			shapes_.emplace_back(std::make_shared<text_shape>(data));
		} else if(type == "pre_commit") {

			/* note this should get split if more preprocessing is used. */
			for(const auto & function : data.all_children_range())
			{

				if(function.key == "blur") {
					blur_depth_ = function.cfg["depth"];
				} else {
					ERR_GUI_P << "Canvas: found a pre commit function"
							  << " of an invalid type " << type << ".\n";
				}
			}

		} else {
			ERR_GUI_P << "Canvas: found a shape of an invalid type " << type
					  << ".\n";

			assert(false);
		}
	}
}

void canvas::clear_shapes(const bool force)
{
	if(force) {
		shapes_.clear();
		drawn_shapes_.clear();
	} else {
		auto conditional = [](const shape_ptr s)->bool { return !s->immutable(); };

		auto iter = std::remove_if(shapes_.begin(), shapes_.end(), conditional);
		shapes_.erase(iter, shapes_.end());

		iter = std::remove_if(drawn_shapes_.begin(), drawn_shapes_.end(), conditional);
		drawn_shapes_.erase(iter, drawn_shapes_.end());
	}
}

void canvas::invalidate_cache()
{
	canvas_ = nullptr;

	if(shapes_.empty()) {
		shapes_.swap(drawn_shapes_);
	} else {
		std::copy(drawn_shapes_.begin(), drawn_shapes_.end(), std::inserter(shapes_, shapes_.begin()));
		drawn_shapes_.clear();
	}
}

/***** ***** ***** ***** ***** SHAPE ***** ***** ***** ***** *****/

} // namespace gui2
