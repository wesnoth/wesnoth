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

#include "draw.hpp"
#include "font/text.hpp"
#include "formatter.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/typed_formula.hpp"
#include "gui/core/log.hpp"
#include "gui/widgets/helper.hpp"
#include "picture.hpp"
#include "sdl/point.hpp"
#include "sdl/rect.hpp"
#include "sdl/texture.hpp"
#include "sdl/utils.hpp" // blur_surface
#include "video.hpp" // read_pixels_low_res, only used for blurring
#include "wml_exception.hpp"

namespace gui2
{

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
		DBG_GUI_P << "Line: found debug message '" << debug << "'.";
	}
}

void line_shape::draw(wfl::map_formula_callable& variables)
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

	DBG_GUI_D << "Line: draw from " << x1 << ',' << y1 << " to " << x2 << ',' << y2 << ".";

	// @todo FIXME respect the thickness.

	draw::line(x1, y1, x2, y2, color_(variables));
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
		DBG_GUI_P << "Rectangle: found debug message '" << debug << "'.";
	}
}

void rectangle_shape::draw(wfl::map_formula_callable& variables)
{
	const int x = x_(variables);
	const int y = y_(variables);
	const int w = w_(variables);
	const int h = h_(variables);

	const color_t fill_color = fill_color_(variables);

	// Fill the background, if applicable
	if(!fill_color.null()) {
		DBG_GUI_D << "fill " << fill_color;
		draw::set_color(fill_color);

		const SDL_Rect area {
			x +  border_thickness_,
			y +  border_thickness_,
			w - (border_thickness_ * 2),
			h - (border_thickness_ * 2)
		};

		draw::fill(area);
	}

	const color_t border_color = border_color_(variables);

	// Draw the border
	draw::set_color(border_color);
	DBG_GUI_D << "border thickness " << border_thickness_
		<< ", colour " << border_color;
	for(int i = 0; i < border_thickness_; ++i) {
		const SDL_Rect dimensions {
			x + i,
			y + i,
			w - (i * 2),
			h - (i * 2)
		};

		draw::rect(dimensions);
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
		DBG_GUI_P << "Rounded Rectangle: found debug message '" << debug << "'.";
	}
}

void round_rectangle_shape::draw(wfl::map_formula_callable& variables)
{
	const int x = x_(variables);
	const int y = y_(variables);
	const int w = w_(variables);
	const int h = h_(variables);
	const int r = r_(variables);

	DBG_GUI_D << "Rounded Rectangle: draw from " << x << ',' << y << " width " << w << " height " << h << ".";

	const color_t fill_color = fill_color_(variables);

	// Fill the background, if applicable
	if(!fill_color.null() && w && h) {
		draw::set_color(fill_color);

		draw::fill(rect{x + r,                 y + border_thickness_, w - r                 * 2, r - border_thickness_ + 1});
		draw::fill(rect{x + border_thickness_, y + r + 1,             w - border_thickness_ * 2, h - r * 2});
		draw::fill(rect{x + r,                 y - r + h + 1,         w - r                 * 2, r - border_thickness_});

		draw::disc(x + r,     y + r,     r, 0xc0);
		draw::disc(x + w - r, y + r,     r, 0x03);
		draw::disc(x + r,     y + h - r, r, 0x30);
		draw::disc(x + w - r, y + h - r, r, 0x0c);
	}

	const color_t border_color = border_color_(variables);

	// Draw the border
	draw::set_color(border_color);

	for(int i = 0; i < border_thickness_; ++i) {
		draw::line(x + r, y + i,     x + w - r, y + i);
		draw::line(x + r, y + h - i, x + w - r, y + h - i);

		draw::line(x + i,     y + r, x + i,     y + h - r);
		draw::line(x + w - i, y + r, x + w - i, y + h - r);

		draw::circle(x + r,     y + r,     r - i, 0xc0);
		draw::circle(x + w - r, y + r,     r - i, 0x03);
		draw::circle(x + r,     y + h - r, r - i, 0x30);
		draw::circle(x + w - r, y + h - r, r - i, 0x0c);
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
		DBG_GUI_P << "Circle: found debug message '" << debug << "'.";
	}
}

void circle_shape::draw(wfl::map_formula_callable& variables)
{
	/**
	 * @todo formulas are now recalculated every draw cycle which is a bit
	 * silly unless there has been a resize. So to optimize we should use an
	 * extra flag or do the calculation in a separate routine.
	 */

	const int x = x_(variables);
	const int y = y_(variables);
	const unsigned radius = radius_(variables);

	DBG_GUI_D << "Circle: drawn at " << x << ',' << y << " radius " << radius << ".";

	const color_t fill_color = fill_color_(variables);
	if(!fill_color.null() && radius) {
		draw::disc(x, y, radius, fill_color);
	}

	const color_t border_color = border_color_(variables);
	for(unsigned int i = 0; i < border_thickness_; i++) {
		draw::circle(x, y, radius - i, border_color);
	}
}

/***** ***** ***** ***** ***** IMAGE ***** ***** ***** ***** *****/

image_shape::image_shape(const config& cfg, wfl::action_function_symbol_table& functions)
	: shape(cfg)
	, x_(cfg["x"])
	, y_(cfg["y"])
	, w_(cfg["w"])
	, h_(cfg["h"])
	, image_name_(cfg["name"])
	, resize_mode_(get_resize_mode(cfg["resize_mode"]))
	, mirror_(cfg.get_old_attribute("mirror", "vertical_mirror", "image"))
	, actions_formula_(cfg["actions"], &functions)
{
	const std::string& debug = (cfg["debug"]);
	if(!debug.empty()) {
		DBG_GUI_P << "Image: found debug message '" << debug << "'.";
	}
}

void image_shape::dimension_validation(unsigned value, const std::string& name, const std::string& key)
{
	const int as_int = static_cast<int>(value);

	VALIDATE_WITH_DEV_MESSAGE(as_int >= 0, _("Image doesn't fit on canvas."),
		formatter() << "Image '" << name << "', " << key << " = " << as_int << "."
	);
}

void image_shape::draw(wfl::map_formula_callable& variables)
{
	DBG_GUI_D << "Image: draw.";

	/**
	 * @todo formulas are now recalculated every draw cycle which is a  bit
	 * silly unless there has been a resize. So to optimize we should use an
	 * extra flag or do the calculation in a separate routine.
	 */
	const std::string& name = image_name_(variables);

	if(name.empty()) {
		DBG_GUI_D << "Image: formula returned no value, will not be drawn.";
		return;
	}

	// Texture filtering mode must be set on texture creation,
	// so check whether we need smooth scaling or not here.
	image::scale_quality scale_quality = image::scale_quality::nearest;
	if (resize_mode_ == resize_mode::stretch
		|| resize_mode_ == resize_mode::scale)
	{
		scale_quality = image::scale_quality::linear;
	}
	texture tex = image::get_texture(image::locator(name), scale_quality);

	if(!tex) {
		ERR_GUI_D << "Image: '" << name << "' not found and won't be drawn.";
		return;
	}

	wfl::map_formula_callable local_variables(variables);
	local_variables.add("image_original_width", wfl::variant(tex.w()));
	local_variables.add("image_original_height", wfl::variant(tex.h()));

	int w = w_(local_variables);
	dimension_validation(w, name, "w");

	int h = h_(local_variables);
	dimension_validation(h, name, "h");

	local_variables.add("image_width", wfl::variant(w ? w : tex.w()));
	local_variables.add("image_height", wfl::variant(h ? h : tex.h()));

	const int x = x_(local_variables);
	const int y = y_(local_variables);

	// used in gui/dialogs/story_viewer.cpp
	local_variables.add("clip_x", wfl::variant(x));
	local_variables.add("clip_y", wfl::variant(y));

	// Execute the provided actions for this context.
	wfl::variant(variables.fake_ptr()).execute_variant(actions_formula_.evaluate(local_variables));

	// If w or h is 0, assume it means the whole image.
	if (!w) { w = tex.w(); }
	if (!h) { h = tex.h(); }

	const SDL_Rect dst_rect { x, y, w, h };

	// What to do with the image depends on whether we need to tile it or not.
	switch(resize_mode_) {
	case resize_mode::tile:
		draw::tiled(tex, dst_rect, false, mirror_(variables));
		break;
	case resize_mode::tile_center:
		draw::tiled(tex, dst_rect, true, mirror_(variables));
		break;
	case resize_mode::tile_highres:
		draw::tiled_highres(tex, dst_rect, false, mirror_(variables));
		break;
	case resize_mode::stretch:
		// Stretching is identical to scaling in terms of handling.
		// Is this intended? That's what previous code was doing.
	case resize_mode::scale:
		// Filtering mode is set on texture creation above.
		// Handling is otherwise identical to sharp scaling.
	case resize_mode::scale_sharp:
		if(mirror_(variables)) {
			draw::flipped(tex, dst_rect);
		} else {
			draw::blit(tex, dst_rect);
		}
		break;
	default:
		ERR_GUI_D << "Image: unrecognized resize mode.";
		break;
	}
}

image_shape::resize_mode image_shape::get_resize_mode(const std::string& resize_mode)
{
	if(resize_mode == "tile") {
		return resize_mode::tile;
	} else if(resize_mode == "tile_center") {
		return resize_mode::tile_center;
	} else if(resize_mode == "tile_highres") {
		return resize_mode::tile_highres;
	} else if(resize_mode == "stretch") {
		return resize_mode::stretch;
	} else if(resize_mode == "scale_sharp") {
		return resize_mode::scale_sharp;
	} else {
		if(!resize_mode.empty() && resize_mode != "scale") {
			ERR_GUI_E << "Invalid resize mode '" << resize_mode << "' falling back to 'scale'.";
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
		DBG_GUI_P << "Text: found debug message '" << debug << "'.";
	}
}

void text_shape::draw(wfl::map_formula_callable& variables)
{
	assert(variables.has_key("text"));

	// We first need to determine the size of the text which need the rendered
	// text. So resolve and render the text first and then start to resolve
	// the other formulas.
	const t_string text = text_(variables);

	if(text.empty()) {
		DBG_GUI_D << "Text: no text to render, leave.";
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
	const auto [tw, th] = text_renderer.get_size();

	// Translate text width and height back to draw-space, rounding up.
	local_variables.add("text_width", wfl::variant(tw));
	local_variables.add("text_height", wfl::variant(th));

	const int x = x_(local_variables);
	const int y = y_(local_variables);
	const int w = w_(local_variables);
	const int h = h_(local_variables);
	rect dst_rect{x, y, w, h};

	texture tex = text_renderer.render_and_get_texture();
	if(!tex) {
		DBG_GUI_D << "Text: Rendering '" << text << "' resulted in an empty canvas, leave.";
		return;
	}

	dst_rect.w = std::min(dst_rect.w, tex.w());
	dst_rect.h = std::min(dst_rect.h, tex.h());

	draw::blit(tex, dst_rect);
}

/***** ***** ***** ***** ***** CANVAS ***** ***** ***** ***** *****/

canvas::canvas()
	: shapes_()
	, blur_depth_(0)
	, w_(0)
	, h_(0)
	, variables_()
	, functions_()
{
}

canvas::canvas(canvas&& c) noexcept
	: shapes_(std::move(c.shapes_))
	, blur_depth_(c.blur_depth_)
	, w_(c.w_)
	, h_(c.h_)
	, variables_(c.variables_)
	, functions_(c.functions_)
{
}

void canvas::draw()
{
	// This early-return has to come before the `validate(rect.w <= w_)` check, as during the boost_unit_tests execution
	// the debug_clock widget will have no shapes, 0x0 size, yet be given a larger rect to draw.
	if(shapes_.empty()) {
		DBG_GUI_D << "Canvas: empty (no shapes to draw).";
		return;
	}

	// Note: this doesn't update if whatever is underneath changes.
	if(blur_depth_ && !blur_texture_) {
		// Cache a blurred image of whatever is underneath.
		SDL_Rect rect = draw::get_viewport();
		surface s = video::read_pixels_low_res(&rect);
		s = blur_surface(s, blur_depth_);
		blur_texture_ = texture(s);
	}

	// Draw blurred background.
	// TODO: hwaccel - this should be able to be removed at some point with shaders
	if(blur_depth_ && blur_texture_) {
		draw::blit(blur_texture_);
	}

	// Draw items
	for(auto& shape : shapes_) {
		const lg::scope_logger inner_scope_logging_object__{log_gui_draw, "Canvas: draw shape."};
		shape->draw(variables_);
	}
}

void canvas::parse_cfg(const config& cfg)
{
	log_scope2(log_gui_parse, "Canvas: parsing config.");

	for(const auto shape : cfg.all_children_range())
	{
		const std::string& type = shape.key;
		const config& data = shape.cfg;

		DBG_GUI_P << "Canvas: found shape of the type " << type << ".";

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
							  << " of an invalid type " << type << ".";
				}
			}

		} else {
			ERR_GUI_P << "Canvas: found a shape of an invalid type " << type
					  << ".";

			assert(false);
		}
	}
}

void canvas::update_size_variables()
{
	get_screen_size_variables(variables_);
	variables_.add("width", wfl::variant(w_));
	variables_.add("height", wfl::variant(h_));
}

void canvas::set_size(const point& size)
{
	w_ = size.x;
	h_ = size.y;
	update_size_variables();
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
