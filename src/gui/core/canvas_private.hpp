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
/**
 * @ingroup GUICanvasWML
 *
 * Definition of a line.
 * When drawing a line it doesn't get blended on the surface but replaces the pixels instead.
 * A blitting flag might be added later if needed.
 *
 * Keys:
 * Key          |Type                                    |Default  |Description
 * -------------|----------------------------------------|---------|-----------
 * x1           | @ref guivartype_f_unsigned "f_unsigned"|0        |The x coordinate of the startpoint.
 * y1           | @ref guivartype_f_unsigned "f_unsigned"|0        |The y coordinate of the startpoint.
 * x2           | @ref guivartype_f_unsigned "f_unsigned"|0        |The x coordinate of the endpoint.
 * y2           | @ref guivartype_f_unsigned "f_unsigned"|0        |The y coordinate of the endpoint.
 * color        | @ref guivartype_color "color"          |""       |The color of the line.
 * thickness    | @ref guivartype_unsigned "unsigned"    |0        |The thickness of the line; if 0 nothing is drawn.
 * debug        | @ref guivartype_string "string"        |""       |Debug message to show upon creation this message is not stored.
 *
 * Variables:
 * Key                |Type                                    |Description
 * -------------------|----------------------------------------|-----------
 * width              | @ref guivartype_unsigned "unsigned"    |The width of the canvas.
 * height             | @ref guivartype_unsigned "unsigned"    |The height of the canvas.
 * text               | @ref guivartype_t_string "t_string"    |The text to render on the widget.
 * text_maximum_width | @ref guivartype_unsigned "unsigned"    |The maximum width available for the text on the widget.
 * text_maximum_height| @ref guivartype_unsigned "unsigned"    |The maximum height available for the text on the widget.
 * text_wrap_mode     | @ref guivartype_int "int"              |When the text doesn't fit in the available width there are several ways to fix that. This variable holds the best method. (NOTE this is a 'hidden' variable meant to copy state from a widget to its canvas so there's no reason to use this variable and thus its values are not listed and might change without further notice.)
 * text_alignment     | @ref guivartype_h_align "h_align"      |The way the text is aligned inside the canvas.
 *
 * The size variables are copied to the window and will be determined at runtime.
 * This is needed since the main window can be resized and the dialog needs to resize accordingly.
 * The following variables are available:
 * Key            |Type                                |Description
 * ---------------|------------------------------------|-----------
 * screen_width   | @ref guivartype_unsigned "unsigned"|The usable width of the Wesnoth main window.
 * screen_height  | @ref guivartype_unsigned "unsigned"|The usable height of the Wesnoth main window.
 * gamemapx_offset| @ref guivartype_unsigned "unsigned"|The distance between left edge of the screen and the game map.
 * gamemap_width  | @ref guivartype_unsigned "unsigned"|The usable width of the Wesnoth gamemap, if no gamemap shown it's the same value as screen_width.
 * gamemap_height | @ref guivartype_unsigned "unsigned"|The usable height of the Wesnoth gamemap, if no gamemap shown it's the same value as screen_height.
 * mouse_x        | @ref guivartype_unsigned "unsigned"|The x coordinate of the mouse pointer.
 * mouse_y        | @ref guivartype_unsigned "unsigned"|The y coordinate of the mouse pointer.
 * window_width   | @ref guivartype_unsigned "unsigned"|The window width. This value has two meanings during the layout phase. This only applies if automatic placement is not enabled. - When set to 0 it should return the wanted maximum width. If no maximum is wanted it should be set to the '"(screen_width)"'. - When not equal to 0 its value is the best width for the window. When the size should remain unchanged it should be set to '"(window_width)"'.
 * window_height  | @ref guivartype_unsigned "unsigned"|The window height. This value has two meanings during the layout phase. This only applies if automatic placement is not enabled. - When set to 0 it should return the wanted maximum height. If no maximum is wanted it should be set to the '"(screen_height)"'. - When not equal to 0 its value is the best height for the window. When the size should remain unchanged it should be set to '"(window_height)"'.
 *
 * Note when drawing the valid coordinates are:
 * * 0 -> width - 1
 * * 0 -> height -1
 *
 * Drawing outside this area will result in unpredictable results including crashing. (That should be fixed, when encountered.)
 */
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

/**
 * @ingroup GUICanvasWML
 *
 * Class holding common attribute names (for WML) and common implementation (in C++) for shapes
 * placed with the 4 attributes x, y, w and h.
 *
 * Keys:
 * Key                |Type                                    |Default|Description
 * -------------------|----------------------------------------|-------|-----------
 * x                  | @ref guivartype_f_unsigned "f_unsigned"|0      |The x coordinate of the top left corner.
 * y                  | @ref guivartype_f_unsigned "f_unsigned"|0      |The y coordinate of the top left corner.
 * w                  | @ref guivartype_f_unsigned "f_unsigned"|0      |The width of the rectangle.
 * h                  | @ref guivartype_f_unsigned "f_unsigned"|0      |The height of the rectangle.
 */
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

/**
 * @ingroup GUICanvasWML
 *
 * Definition of a rectangle.
 * When drawing a rectangle it doesn't get blended on the surface but replaces the pixels instead.
 * A blitting flag might be added later if needed.
 *
 * Keys:
 * Key                |Type                                    |Default|Description
 * -------------------|----------------------------------------|-------|-----------
 * border_thickness   | @ref guivartype_unsigned "unsigned"    |0      |The thickness of the border if the thickness is zero it's not drawn.
 * border_color       | @ref guivartype_color "color"          |""     |The color of the border if empty it's not drawn.
 * fill_color         | @ref guivartype_color "color"          |""     |The color of the interior if omitted it's not drawn.
 * debug              | @ref guivartype_string "string"        |""     |Debug message to show upon creation this message is not stored.
 *
 * Variables: see line_shape
 */
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

/**
 * @ingroup GUICanvasWML
 *
 * Definition of a rounded rectangle shape.
 * When drawing a rounded rectangle it doesn't get blended on the surface but replaces the pixels instead.
 * A blitting flag might be added later if needed.
 * Key             |Type                                    |Default  |Description
 * ----------------|----------------------------------------|---------|-----------
 * corner_radius   | @ref guivartype_f_unsigned "f_unsigned"|0        |The radius of the rectangle's corners.
 * border_thickness| @ref guivartype_unsigned "unsigned"    |0        |The thickness of the border; if the thickness is zero it's not drawn.
 * border_color    | @ref guivartype_color "color"          |""       |The color of the border; if empty it's not drawn.
 * fill_color      | @ref guivartype_color "color"          |""       |The color of the interior; if omitted it's not drawn.
 * debug           | @ref guivartype_string "string"        |""       |Debug message to show upon creation; this message is not stored.
 */
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

/**
 * @ingroup GUICanvasWML
 *
 * Definition of a circle.
 * When drawing a circle it doesn't get blended on the surface but replaces the pixels instead.
 * A blitting flag might be added later if needed.
 *
 * Keys:
 * Key                |Type                                    |Default|Description
 * -------------------|----------------------------------------|-------|-----------
 * x                  | @ref guivartype_f_unsigned "f_unsigned"|0      |The x coordinate of the center.
 * y                  | @ref guivartype_f_unsigned "f_unsigned"|0      |The y coordinate of the center.
 * radius             | @ref guivartype_f_unsigned "f_unsigned"|0      |The radius of the circle; if 0 nothing is drawn.
 * color              | @ref guivartype_color "color"          |""     |The color of the circle.
 * debug              | @ref guivartype_string "string"        |""     |Debug message to show upon creation this message is not stored.
 *
 * Variables: see line_shape
 *
 * Drawing outside the area will result in unpredictable results including crashing. (That should be fixed, when encountered.)
 */
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

/**
 * @ingroup GUICanvasWML
 *
 * Keys:
 * Key                |Type                                      |Default|Description
 * -------------------|------------------------------------------|-------|-----------
 * x                  | @ref guivartype_f_unsigned "f_unsigned"  |0      |The x coordinate of the top left corner.
 * y                  | @ref guivartype_f_unsigned "f_unsigned"  |0      |The y coordinate of the top left corner.
 * w                  | @ref guivartype_f_unsigned "f_unsigned"  |0      |The width of the image, if not zero the image will be scaled to the desired width.
 * h                  | @ref guivartype_f_unsigned "f_unsigned"  |0      |The height of the image, if not zero the image will be scaled to the desired height.
 * resize_mode        | @ref guivartype_resize_mode "resize_mode"|scale  |Determines how an image is scaled to fit the wanted size.
 * vertical_mirror    | @ref guivartype_f_bool "f_bool"          |false  |Mirror the image over the vertical axis.
 * name               | @ref guivartype_string "string"          |""     |The name of the image.
 * debug              | @ref guivartype_string "string"          |""     |Debug message to show upon creation this message is not stored.
 *
 * Variables:
 * Key                  |Type                                  |Description
 * ---------------------|--------------------------------------|-----------
 * image_width          | @ref guivartype_unsigned "unsigned"  |The width of the image, either the requested width or the natural width of the image. This value can be used to set the x (or y) value of the image. (This means x and y are evaluated after the width and height.)
 * image_height         | @ref guivartype_unsigned "unsigned"  |The height of the image, either the requested height or the natural height of the image. This value can be used to set the y (or x) value of the image. (This means x and y are evaluated after the width and height.)
 * image_original_width | @ref guivartype_unsigned "unsigned"  |The width of the image as stored on disk, can be used to set x or w (also y and h can be set).
 * image_original_height| @ref guivartype_unsigned "unsigned"  |The height of the image as stored on disk, can be used to set y or h (also x and y can be set).
 *
 * Also the general variables are available, see line_shape
 */
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

/**
 * @ingroup GUICanvasWML
 *
 * Key                |Type                                      |Default  |Description
 * -------------------|------------------------------------------|---------|-----------
 * font_family        | @ref guivartype_font_style "font_style"  |"sans"   |The font family used for the text.
 * font_size          | @ref guivartype_f_unsigned "f_unsigned"  |mandatory|The size of the text font.
 * font_style         | @ref guivartype_f_unsigned "f_unsigned"  |""       |The style of the text.
 * text_alignment     | @ref guivartype_f_unsigned "f_unsigned"  |"left"   |The alignment of the text.
 * color              | @ref guivartype_color "color"            |""       |The color of the text.
 * text               | @ref guivartype_f_tstring "f_tstring"    |""       |The text to draw (translatable).
 * text_markup        | @ref guivartype_f_bool "f_bool"          |false    |Can the text have mark-up?
 * text_link_aware    | @ref guivartype_f_bool "f_bool"          |false    |Is the text link aware?
 * text_link_color    | @ref guivartype_string "string"          |"#ffff00"|The color of links in the text.
 * maximum_width      | @ref guivartype_f_int "f_int"            |-1       |The maximum width the text is allowed to be.
 * maximum_height     | @ref guivartype_f_int "f_int"            |-1       |The maximum height the text is allowed to be.
 * debug              | @ref guivartype_string "string"          |""       |Debug message to show upon creation this message is not stored.
 *
 * NOTE alignment could only be done with the formulas, but now with the text_alignment flag as well,
 * older widgets might still use the formulas and not all widgets may expose the text alignment yet and when exposed not use it yet.
 *
 * Variables:
 * Key                |Type                                      |Description
 * -------------------|------------------------------------------|-----------
 * text_width         | @ref guivartype_unsigned "unsigned"      |The width of the rendered text.
 * text_height        | @ref guivartype_unsigned "unsigned"      |The height of the rendered text.
 * Also the general variables are available, see line_shape
 */
class text_shape : public rect_bounded_shape
{
public:
	/**
	 * Constructor.
	 *
	 * @param cfg                 The config object to define the text.
	 */
	explicit text_shape(const config& cfg);

	void draw(wfl::map_formula_callable& variables) override;

private:
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
	typed_formula<int> highlight_start_;
	typed_formula<int> highlight_end_;
	/** The color to be used for highlighting */
	typed_formula<color_t> highlight_color_;
};

}
