/*
	Copyright (C) 2007 - 2024
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

#include "gui/core/canvas.hpp"
#include "gui/auxiliary/typed_formula.hpp"

namespace gui2
{

class line_shape : public canvas::shape
{
public:
	/**
	 * Constructor.
	 *
	 * @param cfg                 The config object to define the line.
	 */
	explicit line_shape(const config& cfg);

	void draw(wfl::map_formula_callable& variables) override;

private:
	typed_formula<unsigned> x1_; /**< The start x coordinate of the line. */
	typed_formula<unsigned> y1_; /**< The start y coordinate of the line. */
	typed_formula<unsigned> x2_; /**< The end x coordinate of the line. */
	typed_formula<unsigned> y2_; /**< The end y coordinate of the line. */

	/** The color of the line. */
	typed_formula<color_t> color_;

	/**
	 * The thickness of the line.
	 *
	 * if the value is odd the x and y are the middle of the line.
	 * if the value is even the x and y are the middle of a line
	 * with width - 1. (0 is special case, does nothing.)
	 */
	unsigned thickness_;
};

class rect_bounded_shape : public canvas::shape
{
protected:
	/**
	 * Constructor.
	 *
	 * @param cfg                 The config object to define the rectangle.
	 */
	explicit rect_bounded_shape(const config& cfg)
		: shape(cfg)
		, x_(cfg["x"])
		, y_(cfg["y"])
		, w_(cfg["w"])
		, h_(cfg["h"])
	{
	}

	typed_formula<int> x_; /**< The x coordinate of the rectangle. */
	typed_formula<int> y_; /**< The y coordinate of the rectangle. */
	typed_formula<int> w_; /**< The width of the rectangle. */
	typed_formula<int> h_; /**< The height of the rectangle. */
};

class rectangle_shape : public rect_bounded_shape
{
public:
	/**
	 * Constructor.
	 *
	 * @param cfg                 The config object to define the rectangle.
	 */
	explicit rectangle_shape(const config& cfg);

	void draw(wfl::map_formula_callable& variables) override;

private:
	/**
	 * Border thickness.
	 *
	 * If 0 the fill color is used for the entire widget.
	 */
	int border_thickness_;

	/**
	 * The border color of the rectangle.
	 *
	 * If the color is fully transparent the border isn't drawn.
	 */
	typed_formula<color_t> border_color_;

	/**
	* The border color of the rectangle.
	*
	* If the color is fully transparent the rectangle won't be filled.
	*/
	typed_formula<color_t> fill_color_;
};

class round_rectangle_shape : public rect_bounded_shape
{
public:
	/**
	 * Constructor.
	 *
	 * @param cfg                 The config object to define the round rectangle.
	 */
	explicit round_rectangle_shape(const config& cfg);

	void draw(wfl::map_formula_callable& variables) override;

private:
	typed_formula<int> r_; /**< The radius of the corners. */

	/**
	 * Border thickness.
	 *
	 * If 0 the fill color is used for the entire widget.
	 */
	int border_thickness_;

	/**
	 * The border color of the rounded rectangle.
	 *
	 * If the color is fully transparent the border isn't drawn.
	 */
	typed_formula<color_t> border_color_;

	/**
	 * The border color of the rounded rectangle.
	 *
	 * If the color is fully transparent the rounded rectangle won't be filled.
	 */
	typed_formula<color_t> fill_color_;
};

class circle_shape : public canvas::shape
{
public:
	/**
	 * Constructor.
	 *
	 * @param cfg                 The config object to define the circle.
	 */
	explicit circle_shape(const config& cfg);

	void draw( wfl::map_formula_callable& variables) override;

private:
	typed_formula<unsigned> x_; /**< The center x coordinate of the circle. */
	typed_formula<unsigned> y_; /**< The center y coordinate of the circle. */

	/** The radius of the circle. */
	typed_formula<unsigned> radius_;

	/** The border color of the circle. */
	typed_formula<color_t> border_color_;

	/** The fill color of the circle. */
	typed_formula<color_t> fill_color_;

	/** The border thickness of the circle. */
	unsigned int border_thickness_;
};

class image_shape : public canvas::shape
{
public:
	/**
	 * Constructor.
	 *
	 * @param cfg                 The config object to define the image.
	 * @param functions           WFL functions to execute.
	 */
	image_shape(const config& cfg, wfl::action_function_symbol_table& functions);

	void draw(wfl::map_formula_callable& variables) override;

private:
	typed_formula<unsigned> x_; /**< The x coordinate of the image. */
	typed_formula<unsigned> y_; /**< The y coordinate of the image. */
	typed_formula<unsigned> w_; /**< The width of the image. */
	typed_formula<unsigned> h_; /**< The height of the image. */

	/**
	 * Name of the image.
	 *
	 * This value is only used when the image name is a formula. If it isn't a
	 * formula the image will be loaded in the constructor. If it's a formula it
	 * will be loaded every draw cycles. This allows 'changing' images.
	 */
	typed_formula<std::string> image_name_;

	/**
	 * Determines the way an image will be resized.
	 *
	 * If the image is smaller is needed it needs to resized, how is determined
	 * by the value of this enum.
	 */
	enum class resize_mode {
		scale,
		scale_sharp,
		stretch,
		tile,
		tile_center,
		tile_highres,
	};

	/** Converts a string to a resize mode. */
	resize_mode get_resize_mode(const std::string& resize_mode);

	/** The resize mode for an image. */
	resize_mode resize_mode_;

	/** Mirror the image over the vertical axis. */
	typed_formula<bool> mirror_;

	// TODO: use a typed_formula?
	wfl::formula actions_formula_;

	static void dimension_validation(unsigned value, const std::string& name, const std::string& key);
};

class text_shape : public rect_bounded_shape
{
public:
	/**
	 * Constructor.
	 *
	 * @param cfg                 The config object to define the text.
	 * @param functions           WFL functions to execute.
	 */
	explicit text_shape(const config& cfg, wfl::action_function_symbol_table& functions);

	void draw(wfl::map_formula_callable& variables) override;

private:
	/** the source config */
	config cfg_;

	/** The text font family. */
	font::family_class font_family_;

	/** The font size of the text. */
	typed_formula<unsigned> font_size_;

	/** The style of the text. */
	font::pango_text::FONT_STYLE font_style_;

	/** The alignment of the text. */
	typed_formula<PangoAlignment> text_alignment_;

	/** The color of the text. */
	typed_formula<color_t> color_;

	/** The text to draw. */
	typed_formula<t_string> text_;

	/** The text markup switch of the text. */
	typed_formula<bool> text_markup_;

	/** The link aware switch of the text. */
	typed_formula<bool> link_aware_;

	/** The link color of the text. */
	typed_formula<color_t> link_color_;

	/** The maximum width for the text. */
	typed_formula<int> maximum_width_;

	/** The number of characters per line. */
	unsigned characters_per_line_;

	/** The maximum height for the text. */
	typed_formula<int> maximum_height_;

	/** Start and end offsets for highlight */
	std::string highlight_start_;
	std::string highlight_end_;

	/** The color to be used for highlighting */
	typed_formula<color_t> highlight_color_;

	/** Generic start and end offsets for various attributes */
	std::string attr_start_;
	std::string attr_end_;

	/**
	 * The attribute type
	 * Possible values :
	 *  color/foreground, bgcolor/background, font_size/size,
	 *  bold, italic, underline
	 * The first three require extra data
	 * the color for the first two, and font size for the last
	 */
	std::string attr_name_;

	/** extra data for the attribute, if any */
	std::string attr_data_;

	/** Whether to apply a text outline. */
	typed_formula<bool> outline_;

	/** Any extra WFL actions to execute. */
	wfl::formula actions_formula_;
};

}
