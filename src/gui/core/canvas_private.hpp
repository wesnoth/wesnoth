/*
Copyright (C) 2007 - 2017 by Mark de Wever <koraq@xs4all.nl>
Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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

namespace gui2 {

/** Definition of a line shape. */
class line_shape : public canvas::shape {
public:
	/**
	 * Constructor.
	 *
	 * @param cfg                 The config object to define the line see
	 *                            http://www.wesnoth.org/wiki/GUICanvasWML#Line
	 *                            for more information.
	 */
	explicit line_shape(const config& cfg);

	/** Implement shape::draw(). */
	void draw(
			const int canvas_w,
			const int canvas_h,
			SDL_Renderer* renderer,
			wfl::map_formula_callable& variables) override;

private:
	typed_formula<unsigned> x1_, /**< The start x coordinate of the line. */
			y1_,			/**< The start y coordinate of the line. */
			x2_,			/**< The end x coordinate of the line. */
			y2_;			/**< The end y coordinate of the line. */

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

/** Definition of a rectangle shape. */
class rectangle_shape : public canvas::shape {
public:
	/**
	 * Constructor.
	 *
	 * @param cfg                 The config object to define the rectangle see
	 *                            http://www.wesnoth.org/wiki/GUICanvasWML#Rectangle
	 *                            for more information.
	 */
	explicit rectangle_shape(const config& cfg);

	/** Implement shape::draw(). */
	void draw(
			const int canvas_w,
			const int canvas_h,
			SDL_Renderer* renderer,
			wfl::map_formula_callable& variables) override;

private:
	typed_formula<int> x_, /**< The x coordinate of the rectangle. */
			y_,			   /**< The y coordinate of the rectangle. */
			w_,			   /**< The width of the rectangle. */
			h_;			   /**< The height of the rectangle. */

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

/** Definition of a rounded rectangle shape. */
class round_rectangle_shape : public canvas::shape {
public:
	/**
	 * Constructor.
	 *
	 * @param cfg                 The config object to define the round rectangle see
	 *                            http://www.wesnoth.org/wiki/GUICanvasWML#Rounded_Rectangle
	 *                            for more information.
	 */
	explicit round_rectangle_shape(const config& cfg);

	/** Implement shape::draw(). */
	void draw(
			const int canvas_w,
			const int canvas_h,
			SDL_Renderer* renderer,
			wfl::map_formula_callable& variables) override;
private:
	typed_formula<int> x_, /**< The x coordinate of the rectangle. */
			y_,			   /**< The y coordinate of the rectangle. */
			w_,			   /**< The width of the rectangle. */
			h_,			   /**< The height of the rectangle. */
			r_;			   /**< The radius of the corners. */

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

/** Definition of a circle shape. */
class circle_shape : public canvas::shape {
public:
	/**
	 * Constructor.
	 *
	 * @param cfg                 The config object to define the circle see
	 *                            http://www.wesnoth.org/wiki/GUICanvasWML#Circle
	 *                            for more information.
	 */
	explicit circle_shape(const config& cfg);

	/** Implement shape::draw(). */
	void draw(
			const int canvas_w,
			const int canvas_h,
			SDL_Renderer* renderer,
			wfl::map_formula_callable& variables) override;

private:
	typed_formula<unsigned> x_, /**< The center x coordinate of the circle. */
			y_,			   /**< The center y coordinate of the circle. */
			radius_;	   /**< The radius of the circle. */

	/** The border color of the circle. */
	typed_formula<color_t> border_color_, fill_color_; /**< The fill color of the circle. */

	/** The border thickness of the circle. */
	unsigned int border_thickness_;
};

/** Definition of an image shape. */
class image_shape : public canvas::shape {
public:
	/**
	 * Constructor.
	 *
	 * @param cfg                 The config object to define the image see
	 *                            http://www.wesnoth.org/wiki/GUICanvasWML#Image
	 *                            for more information.
	 */
	image_shape(const config& cfg, wfl::action_function_symbol_table& functions);

	/** Implement shape::draw(). */
	void draw(
			const int canvas_w,
			const int canvas_h,
			SDL_Renderer* renderer,
			wfl::map_formula_callable& variables) override;
private:
	typed_formula<unsigned> x_, /**< The x coordinate of the image. */
			y_,			   /**< The y coordinate of the image. */
			w_,			   /**< The width of the image. */
			h_;			   /**< The height of the image. */

	/**
	 * The image texture. Since formulas may return different values each draw cycle, this is reassigned
	 * each time, so this is mostly here to avoid constantly allocating a new textures.
	 */
	texture image_;

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
	enum resize_mode {
		scale,
		stretch,
		tile,
		tile_center,
	};

	/** Converts a string to a resize mode. */
	resize_mode get_resize_mode(const std::string& resize_mode);

	/** The resize mode for an image. */
	resize_mode resize_mode_;

	/** Mirror the image over the vertical axis. */
	typed_formula<bool> vertical_mirror_;

	// TODO: use a typed_formula?
	wfl::formula actions_formula_;

	static void dimension_validation(unsigned value, const std::string& name, const std::string& key);
};

/** Definition of a text shape. */
class text_shape : public canvas::shape {
public:
	/**
	 * Constructor.
	 *
	 * @param cfg                 The config object to define the text see
	 *                            http://www.wesnoth.org/wiki/GUICanvasWML#Text
	 *                            for more information.
	 */
	explicit text_shape(const config& cfg);

	/** Implement shape::draw(). */
	void draw(
			const int canvas_w,
			const int canvas_h,
			SDL_Renderer* renderer,
			wfl::map_formula_callable& variables) override;

private:
	typed_formula<unsigned> x_, /**< The x coordinate of the text. */
			y_,			   /**< The y coordinate of the text. */
			w_,			   /**< The width of the text. */
			h_;			   /**< The height of the text. */

	/** The text font family. */
	font::family_class font_family_;

	/** The font size of the text. */
	unsigned font_size_;

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
};

}
