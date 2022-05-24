/*
	Copyright (C) 2007 - 2022
	by Mark de Wever <koraq@xs4all.nl>
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
#include "sdl/texture.hpp"
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
				 const SDL_Rect& view_bounds,
				 wfl::map_formula_callable& variables)
{
	/**
	 * @todo formulas are now recalculated every draw cycle which is a bit silly
	 * unless there has been a resize. So to optimize we should use an extra
	 * flag or do the calculation in a separate routine.
	 */

	const unsigned x1 = x1_(variables) - view_bounds.x;
	const unsigned y1 = y1_(variables) - view_bounds.y;
	const unsigned x2 = x2_(variables) - view_bounds.x;
	const unsigned y2 = y2_(variables) - view_bounds.y;

	DBG_GUI_D << "Line: draw from " << x1 << ',' << y1 << " to " << x2 << ',' << y2
			  << " within bounds {" << view_bounds.x << ", " << view_bounds.y
			  << ", " << view_bounds.w << ", " << view_bounds.h << "}.\n";

	// @todo FIXME respect the thickness.

	// lock the surface
	surface_lock locker(canvas);

	draw_line(canvas, renderer, color_(variables), x1, y1, x2, y2);
}

/***** ***** ***** Base class for rectangular shapes ***** ***** *****/

rect_bounded_shape::rect_bounded_shape(const config& cfg)
	: shape(cfg)
	, x_(cfg["x"])
	, y_(cfg["y"])
	, w_(cfg["w"])
	, h_(cfg["h"])
{
}

rect_bounded_shape::calculated_rects rect_bounded_shape::calculate_rects(const SDL_Rect& view_bounds, wfl::map_formula_callable& variables) const
{
	// Formulas are recalculated every draw cycle, even if there hasn't been a resize.

	const unsigned x = x_(variables);
	const unsigned y = y_(variables);
	const unsigned w = w_(variables);
	const unsigned h = h_(variables);

	const auto dst_on_widget = sdl::create_rect(x, y, w, h);

	SDL_Rect clip_on_widget;
	if(!SDL_IntersectRect(&dst_on_widget, &view_bounds, &clip_on_widget)) {
		DBG_GUI_D << "Text: Clipping view_bounds resulted in an empty intersection, nothing to do.\n";
		return {true, dst_on_widget, {}, {}, {}, {}};
	}

	auto unclipped_around_viewport = dst_on_widget;
	unclipped_around_viewport.x -= view_bounds.x;
	unclipped_around_viewport.y -= view_bounds.y;

	auto clip_in_shape = clip_on_widget;
	clip_in_shape.x -= x;
	clip_in_shape.y -= y;

	auto dst_in_viewport = clip_on_widget;
	dst_in_viewport.x -= view_bounds.x;
	dst_in_viewport.y -= view_bounds.y;

	DBG_GUI_D << "Calculate_rects: from " << x << ',' << y << " width " << w << " height " << h << "\n"
			  << " view_bounds {" << view_bounds.x << ", " << view_bounds.y << ", "
			  << view_bounds.w << ", " << view_bounds.h << "}.\n"
			  << " dst_in_viewport {" << dst_in_viewport.x << ", " << dst_in_viewport.y << ", "
			  << dst_in_viewport.w << ", " << dst_in_viewport.h << "}.\n";

	return {false, dst_on_widget, clip_on_widget, clip_in_shape, unclipped_around_viewport, dst_in_viewport};
}

/***** ***** ***** ***** ***** Rectangle ***** ***** ***** ***** *****/

rectangle_shape::rectangle_shape(const config& cfg)
	: rect_bounded_shape(cfg)
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
					  const SDL_Rect& view_bounds,
					  wfl::map_formula_callable& variables)
{
	const auto rects = calculate_rects(view_bounds, variables);
	if(rects.empty) {
		return;
	}

	surface_lock locker(canvas);

	const color_t fill_color = fill_color_(variables);

	// Fill the background, if applicable
	if(!fill_color.null()) {
		set_renderer_color(renderer, fill_color);
		auto area = rects.unclipped_around_viewport;
		area.x += border_thickness_;
		area.y += border_thickness_;
		area.w -= 2 * border_thickness_;
		area.h -= 2 * border_thickness_;

		SDL_RenderFillRect(renderer, &area);
	}

	// Draw the border
	set_renderer_color(renderer, border_color_(variables));
	for(int i = 0; i < border_thickness_; ++i) {
		auto dimensions = rects.unclipped_around_viewport;
		dimensions.x += i;
		dimensions.y += i;
		dimensions.w -= 2 * i;
		dimensions.h -= 2 * i;

		SDL_RenderDrawRect(renderer, &dimensions);
	}
}

/***** ***** ***** ***** ***** Rounded Rectangle ***** ***** ***** ***** *****/

round_rectangle_shape::round_rectangle_shape(const config& cfg)
	: rect_bounded_shape(cfg)
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
	const SDL_Rect& view_bounds,
	wfl::map_formula_callable& variables)
{
	const auto rects = calculate_rects(view_bounds, variables);
	const int x = rects.unclipped_around_viewport.x;
	const int y = rects.unclipped_around_viewport.y;
	const int w = rects.unclipped_around_viewport.w;
	const int h = rects.unclipped_around_viewport.h;
	const int r = r_(variables);

	DBG_GUI_D << "Rounded Rectangle: draw from " << x << ',' << y << " width " << w
		<< " height "
		<< " within bounds {" << view_bounds.x << ", " << view_bounds.y
		<< ", " << view_bounds.w << ", " << view_bounds.h << "}.\n";

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
				   const SDL_Rect& view_bounds,
				   wfl::map_formula_callable& variables)
{
	/**
	 * @todo formulas are now recalculated every draw cycle which is a bit
	 * silly unless there has been a resize. So to optimize we should use an
	 * extra flag or do the calculation in a separate routine.
	 */

	const int x = x_(variables) - view_bounds.x;
	const int y = y_(variables) - view_bounds.y;
	const unsigned radius = radius_(variables);

	DBG_GUI_D << "Circle: drawn at " << x << ',' << y << " radius " << radius
		<< " within bounds {" << view_bounds.x << ", " << view_bounds.y
		<< ", " << view_bounds.w << ", " << view_bounds.h << "}.\n";

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
				  const SDL_Rect& view_bounds,
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
	const unsigned clip_y = y_(local_variables);

	local_variables.add("clip_x", wfl::variant(clip_x));
	local_variables.add("clip_y", wfl::variant(clip_y));

	// Execute the provided actions for this context.
	wfl::variant(variables.fake_ptr()).execute_variant(actions_formula_.evaluate(local_variables));

	// Copy the data to local variables to avoid overwriting the originals.
	SDL_Rect src_clip = src_clip_;
	surface surf;

	// Test whether we need to scale and do the scaling if needed.
	if ((w == 0) && (h == 0)) {
		surf = image_;
	}
	else { // assert((w != 0) || (h != 0))
		if(w == 0 && resize_mode_ == resize_mode::stretch) {
			DBG_GUI_D << "Image: vertical stretch from " << image_->w << ','
					  << image_->h << " to a height of " << h << ".\n";

			surf = stretch_surface_vertical(image_, h);
			w = image_->w;
		}
		else if(h == 0 && resize_mode_ == resize_mode::stretch) {
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
			if(resize_mode_ == resize_mode::tile) {
				DBG_GUI_D << "Image: tiling from " << image_->w << ','
						  << image_->h << " to " << w << ',' << h << ".\n";

				surf = tile_surface(image_, w, h, false);
			} else if(resize_mode_ == resize_mode::tile_center) {
				DBG_GUI_D << "Image: tiling centrally from " << image_->w << ','
						  << image_->h << " to " << w << ',' << h << ".\n";

				surf = tile_surface(image_, w, h, true);
			} else {
				if(resize_mode_ == resize_mode::stretch) {
					ERR_GUI_D << "Image: failed to stretch image, "
								 "fall back to scaling.\n";
				}

				DBG_GUI_D << "Image: scaling from " << image_->w << ','
						  << image_->h << " to " << w << ',' << h << ".\n";

				if(resize_mode_ == resize_mode::scale_sharp) {
					surf = scale_surface_sharp(image_, w, h);
				} else {
					surf = scale_surface_legacy(image_, w, h);
				}
			}
		}
		src_clip.w = w;
		src_clip.h = h;
	}

	// Calculate the destination, clip it to the view_bounds, and then change both
	// src_clip and dst_on_view_bounds to view_bounds' coordinate system.
	const SDL_Rect dst_on_widget = sdl::create_rect(clip_x, clip_y, src_clip.w, src_clip.h);
	SDL_Rect dst_on_view_bounds;
	if(!SDL_IntersectRect(&dst_on_widget, &view_bounds, &dst_on_view_bounds)) {
		DBG_GUI_D << "Image: completely outside view_bounds\n";
		return;
	}
	dst_on_view_bounds.x -= view_bounds.x;
	dst_on_view_bounds.y -= view_bounds.y;
	src_clip.x += view_bounds.x;
	src_clip.y += view_bounds.y;
	src_clip.w = dst_on_view_bounds.w;
	src_clip.h = dst_on_view_bounds.h;

	if(vertical_mirror_(local_variables)) {
		surf = flip_surface(surf);
	}

	blit_surface(surf, &src_clip, canvas, &dst_on_view_bounds);
}

image_shape::resize_mode image_shape::get_resize_mode(const std::string& resize_mode)
{
	if(resize_mode == "tile") {
		return resize_mode::tile;
	} else if(resize_mode == "tile_center") {
		return resize_mode::tile_center;
	} else if(resize_mode == "stretch") {
		return resize_mode::stretch;
	} else if(resize_mode == "scale_sharp") {
		return resize_mode::scale_sharp;
	} else {
		if(!resize_mode.empty() && resize_mode != "scale") {
			ERR_GUI_E << "Invalid resize mode '" << resize_mode
					  << "' falling back to 'scale'.\n";
		}
		return resize_mode::scale;
	}
}

/***** ***** ***** ***** ***** TEXT ***** ***** ***** ***** *****/

text_shape::text_shape(const config& cfg)
	: rect_bounded_shape(cfg)
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
				 const SDL_Rect& view_bounds,
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

	wfl::map_formula_callable local_variables(variables);
	local_variables.add("text_width", wfl::variant(text_renderer.get_width()));
	local_variables.add("text_height", wfl::variant(text_renderer.get_height()));

	const auto rects = calculate_rects(view_bounds, local_variables);

	if(rects.empty) {
		DBG_GUI_D << "Text: Clipping to view_bounds resulted in an empty intersection, nothing to do.\n";
		return;
	}

	surface& surf = text_renderer.render(rects.clip_in_shape);
	if(surf->w == 0) {
		DBG_GUI_D << "Text: Rendering '" << text
				  << "' resulted in an empty canvas, leave.\n";
		return;
	}

	// Blit the clipped region - this needs a non-const copy of the rect
	auto dst_in_viewport = rects.dst_in_viewport;
	blit_surface(surf, nullptr, canvas, &dst_in_viewport);
}

/***** ***** ***** ***** ***** CANVAS ***** ***** ***** ***** *****/

canvas::canvas()
	: shapes_()
	, blur_depth_(0)
	, w_(0)
	, h_(0)
	, viewport_()
	, view_bounds_{0, 0, 0, 0}
	, variables_()
	, functions_()
	, is_dirty_(true)
{
}

canvas::canvas(canvas&& c) noexcept
	: shapes_(std::move(c.shapes_))
	, blur_depth_(c.blur_depth_)
	, w_(c.w_)
	, h_(c.h_)
	, viewport_(std::move(c.viewport_))
	, view_bounds_(std::move(c.view_bounds_))
	, variables_(c.variables_)
	, functions_(c.functions_)
	, is_dirty_(c.is_dirty_)
{
}

void canvas::draw(const SDL_Rect& area_to_draw, bool force)
{
	log_scope2(log_gui_draw, "Canvas: drawing.");
	if(!viewport_ || !SDL_RectEquals(&view_bounds_, &area_to_draw)) {
		DBG_GUI_D << "Canvas: redrawing because the cached view_bounds no longer fits.\n";
		invalidate_cache();
	} else if(!is_dirty_ && !force) {
		DBG_GUI_D << "Canvas: nothing to draw.\n";
		return;
	}

	if(is_dirty_) {
		get_screen_size_variables(variables_);
		variables_.add("width", wfl::variant(w_));
		variables_.add("height", wfl::variant(h_));
	}

	auto renderer = std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> {nullptr, &SDL_DestroyRenderer};

	if(viewport_ && view_bounds_.w == area_to_draw.w && view_bounds_.h == area_to_draw.h) {
		DBG_GUI_D << "Canvas: use cached canvas.\n";
		renderer.reset(SDL_CreateSoftwareRenderer(viewport_));
		SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 0);
		SDL_RenderClear(renderer.get());
	} else {
		DBG_GUI_D << "Canvas: create new empty canvas.\n";
		viewport_ = surface(area_to_draw.w, area_to_draw.h);
		renderer.reset(SDL_CreateSoftwareRenderer(viewport_));
	}
	view_bounds_ = area_to_draw;
	SDL_SetRenderDrawBlendMode(renderer.get(), SDL_BLENDMODE_BLEND);

	// draw items
	for(auto& shape : shapes_) {
		lg::scope_logger inner_scope_logging_object__(log_gui_draw, "Canvas: draw shape.");

		shape->draw(viewport_, renderer.get(), view_bounds_, variables_);
	}

	SDL_RenderPresent(renderer.get());

	is_dirty_ = false;
}

void canvas::blit(SDL_Rect rect)
{
	// This early-return has to come before the `validate(rect.w <= w_)` check, as during the boost_unit_tests execution
	// the debug_clock widget will have no shapes, 0x0 size, yet be given a larger rect to draw.
	if(shapes_.empty()) {
		DBG_GUI_D << "Canvas: empty (no shapes to draw).\n";
		return;
	}

	CVideo& video = CVideo::get_singleton();

	VALIDATE(rect.w >= 0 && rect.h >= 0, _("Area to draw has negative size"));
	VALIDATE(static_cast<unsigned>(rect.w) <= w_ && static_cast<unsigned>(rect.h) <= h_,
		_("Area to draw is larger than widget size"));

	// If the widget is partly off-screen, this might get called with
	// surf width=1000, height=1000
	// rect={-1, 2, 330, 440}
	//
	// From those, as the first column is off-screen:
	// rect_clipped_to_parent={0, 2, 329, 440}
	// area_to_draw={1, 0, 329, 440}
	SDL_Rect parent {0, 0, video.get_width(), video.get_height()};
	SDL_Rect rect_clipped_to_parent;
	if(!SDL_IntersectRect(&rect, &parent, &rect_clipped_to_parent)) {
		DBG_GUI_D << "Area to draw is completely outside parent.\n";
		return;
	}
	SDL_Rect area_to_draw {
		std::max(0, -rect.x),
		std::max(0, -rect.y),
		rect_clipped_to_parent.w,
		rect_clipped_to_parent.h
	};

	draw(area_to_draw);

	if(blur_depth_) {
		surface s = video.read_pixels_low_res(&rect);
		s = blur_surface(s, blur_depth_);
		video.blit_surface(s, &rect);
	}

	// Currently draw(area_to_draw) will always allocate a viewport_ that exactly matches area_to_draw, which means that
	// scrolling by a single pixel will force a complete redraw. I tested rendering a few of the off-screen lines too,
	// however it didn't seem to be an optimisation - the dirty flag was already set on each such redraw, triggering a
	// complete redraw.
	//
	// If you try to re-add this overdraw, the nullptr below will need to be replaced with
	// {area_to_draw.x - view_bounds_.x, area_to_draw.y - view_bounds_.y, area_to_draw.w, area_to_draw.h};
	assert(area_to_draw.x == view_bounds_.x);
	assert(area_to_draw.y == view_bounds_.y);
	assert(area_to_draw.w == view_bounds_.w);
	assert(area_to_draw.h == view_bounds_.h);
	video.blit_surface(rect.x, rect.y, viewport_);
}

void canvas::parse_cfg(const config& cfg)
{
	log_scope2(log_gui_parse, "Canvas: parsing config.");

	for(const auto shape : cfg.all_children_range())
	{
		const std::string& type = shape.key;
		const config& data = shape.cfg;

		DBG_GUI_P << "Canvas: found shape of the type " << type << ".\n";

		if(type == "line") {
			shapes_.emplace_back(std::make_unique<line_shape>(data));
		} else if(type == "rectangle") {
			shapes_.emplace_back(std::make_unique<rectangle_shape>(data));
		} else if(type == "round_rectangle") {
			shapes_.emplace_back(std::make_unique<round_rectangle_shape>(data));
		} else if(type == "circle") {
			shapes_.emplace_back(std::make_unique<circle_shape>(data));
		} else if(type == "image") {
			shapes_.emplace_back(std::make_unique<image_shape>(data, functions_));
		} else if(type == "text") {
			shapes_.emplace_back(std::make_unique<text_shape>(data));
		} else if(type == "pre_commit") {

			/* note this should get split if more preprocessing is used. */
			for(const auto function : data.all_children_range())
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
	} else {
		auto conditional = [](const std::unique_ptr<shape>& s)->bool { return !s->immutable(); };

		auto iter = std::remove_if(shapes_.begin(), shapes_.end(), conditional);
		shapes_.erase(iter, shapes_.end());
	}
}

/***** ***** ***** ***** ***** SHAPE ***** ***** ***** ***** *****/

} // namespace gui2
