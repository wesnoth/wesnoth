/*
	Copyright (C) 2008 - 2024
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

#pragma once

#include "font/font_options.hpp"
#include "color.hpp"
#include "sdl/surface.hpp"
#include "sdl/texture.hpp"
#include "serialization/string_utils.hpp"

#include <pango/pangocairo.h>


#include <functional>
#include <memory>
#include <string>
#include <vector>

/***
 * Note: This is the cairo-pango code path, not the SDL_TTF code path.
 */

struct point;

namespace font {

/** Flush the rendered text cache. */
void flush_texture_cache();

// add background color and also font markup.

/**
 * Text class.
 *
 * This class represents text which is rendered using Pango.
 *
 * It takes text, as a utf-8 std::string, plus formatting options including
 * font and color. It provides a surface object which holds the rendered text.
 *
 * Besides this, it can do some additional calculations using the font layout.
 *
 * It can take an index into the text, and convert it to pixel coordinates,
 * so that if we want to draw a cursor in an editbox, we know where to draw it.
 *
 * It can also take a pixel coordinate with respect to the text layout, and
 * translate it back to an index into the original text. This is useful if the
 * user clicks on the text, and we want to know where to move the cursor.
 *
 * The get_token method takes a pixel coordinate, which we assume represents a
 * click position, and gets the corresponding "token" from the string. The default
 * token delimiters are whitespace " \n\r\t". So, this returns the "word" that the
 * user clicked on.
 *
 * Finally, the get_link method represents special support for hyperlinks in text.
 * A token "looks like a link" if it begins "http://" or "https://".
 * If a text has link_aware enabled, then any such token is rendered with an
 * underline and in a special color, see `link_color`.
 * The get_link method calls get_token and further checks if the clicked token
 * looks like a link.
 *
 * This class stores the text to draw and uses pango with the cairo backend to
 * render the text. See http://pango.org for more info.
 *
 */
class pango_text
{
public:
	pango_text();

	pango_text(const pango_text&) = delete;
	pango_text& operator=(const pango_text&) = delete;

	/**
	 * Returns the cached texture, or creates a new one otherwise.
	 *
	 * texture::w() and texture::h() methods will return the expected
	 * width and height of the texture in draw space. This may differ
	 * from the real value returned by texture::get_info().
	 *
	 * In almost all cases, use w() and h() to get the size of the
	 * rendered text for drawing.
	 */
	texture render_and_get_texture();

private:
	/**
	 * Wrapper around render_surface which sets texture::w() and texture::h()
	 * in the same way that render_and_get_texture does.
	 *
	 * The viewport rect is interpreted at the scale of render-space, not
	 * drawing-space. This function has only been made private to preserve
	 * the drawing-space encapsulation.
	 */
	texture render_texture(const SDL_Rect& viewport);

	/**
	 * Returns the rendered text.
	 *
	 * The viewport rect is interpreted at the scale of render-space, not
	 * drawing-space. This function has only been made private to preserve
	 * the drawing-space encapsulation.
	 *
	 * @param viewport Only this area needs to be drawn - the returned
	 * surface's origin will correspond to viewport.x and viewport.y, the
	 * width and height will be at least viewport.w and viewport.h (although
	 * they may be larger).
	 */
	surface render_surface(const SDL_Rect& viewport);

public:
	/** Returns the size of the text, in drawing coordinates. */
	point get_size();

	/** Has the text been truncated? This happens if it exceeds max width or height. */
	bool is_truncated() const;

	/**
	 * Inserts UTF-8 text.
	 *
	 * @param offset              The position to insert the text.
	 * @param text                The UTF-8 text to insert.
	 *
	 * @returns                   The number of characters inserted.
	 */
	unsigned insert_text(const unsigned offset, const std::string& text);

	/***** ***** ***** ***** Font flags ***** ***** ***** *****/

	// NOTE: these values must be powers of 2 in order to be bit-unique
	enum FONT_STYLE {
		STYLE_NORMAL = 0,
		STYLE_BOLD = 1,
		STYLE_ITALIC = 2,
		STYLE_UNDERLINE = 4,
	};

	/***** ***** ***** ***** Query details ***** ***** ***** *****/

	/**
	 * Returns the maximum glyph height of a font, in drawing coordinates.
	 *
	 * @returns                       The height of the tallest possible glyph for the selected
	 *                                font. More specifically, the result is the sum of the maximum
	 *                                ascent and descent lengths.
	 */
	int get_max_glyph_height() const;

	/**
	 * Gets the location for the cursor, in drawing coordinates.
	 *
	 * @param column              The column character index of the cursor.
	 * @param line                The line character index of the cursor.
	 *
	 * @returns                   The position of the top of the cursor. It the
	 *                            requested location is out of range 0,0 is
	 *                            returned.
	 */
	point get_cursor_position(
		const unsigned column, const unsigned line = 0) const;

	/**
	 * Gets the location for the cursor, in drawing coordinates.
	 *
	 * @param offset              The column byte index of the cursor.
	 *
	 * @returns                   The position of the top of the cursor. It the
	 *                            requested location is out of range 0,0 is
	 *                            returned.
	 */
	point get_cursor_pos_from_index(const unsigned offset) const;

	/**
	 * Get maximum length.
	 *
	 * @returns                   The maximum length of the text. The length of text
	 *                            should not exceed this value.
	 */
	std::size_t get_maximum_length() const;

	/**
	 * Gets the largest collection of characters, including the token at position,
	 * and not including any characters from the delimiters set.
	 *
	 * @param position            The pixel position in the text area.
	 * @param delimiters
	 *
	 * @returns                   The token containing position, and none of the
	 * 			      delimiter characters. If position is out of bounds,
	 *			      it returns the empty string.
	 */
	std::string get_token(const point & position, const char * delimiters = " \n\r\t") const;

	/**
	 * Checks if position points to a character in a link in the text, returns it
	 * if so, empty string otherwise. Link-awareness must be enabled to get results.
	 * @param position            The pixel position in the text area.
	 *
	 * @returns                   The link if one is found, the empty string otherwise.
	 */
	std::string get_link(const point & position) const;

	/**
	 * Gets the column of line of the character at the position.
	 *
	 * @param position            The pixel position in the text area.
	 *
	 * @returns                   A point with the x value the column and the y
	 *                            value the line of the character found (or last
	 *                            character if not found.
	 */
	point get_column_line(const point& position) const;

	/**
	 * Retrieves a list of strings with contents for each rendered line.
	 *
	 * This method is not const because it requires rendering the text.
	 *
	 * @note This is only intended for renderer implementation details. This
	 *       is a rather expensive function because it copies everything at
	 *       least once.
	 */
	std::vector<std::string> get_lines() const;

	/**
	 * Get a specific line from the pango layout
	 *
	 * @param index    the line number of the line to retrieve
	 *
	 * @returns        the PangoLayoutLine* corresponding to line number index
	 */
	PangoLayoutLine* get_line(int index);

	/**
	 * Given a byte index, find out at which line the corresponding character
	 * is located.
	 *
	 * @param offset   the byte index
	 *
	 * @returns        the line number corresponding to the given index
	 */
	int get_line_num_from_offset(const unsigned offset);

	/**
	 * Get number of lines in the text.
	 *
	 * @returns                   The number of lines in the text.
	 *
	 */
	unsigned get_lines_count() const { return pango_layout_get_line_count(layout_.get()); };

	/**
	 * Gets the length of the text in bytes.
	 *
	 * The text set is UTF-8 so the length of the string might not be the length
	 * of the text.
	 */
	std::size_t get_length() const { return length_; }

	/**
	 * Sets the text to render.
	 *
	 * @param text                The text to render.
	 * @param markedup            Should the text be rendered with pango
	 *                            markup. If the markup is invalid it's
	 *                            rendered as text without markup.
	 *
	 * @returns                   The status, if rendered as markup and the
	 *                            markup contains errors, false is returned
	 *                            else true.
	 */
	bool set_text(const std::string& text, const bool markedup);

	/***** ***** ***** ***** Setters / getters ***** ***** ***** *****/

	const std::string& text() const { return text_; }

	pango_text& set_family_class(font::family_class fclass);

	pango_text& set_font_size(unsigned font_size);

	pango_text& set_font_style(const FONT_STYLE font_style);

	pango_text& set_foreground_color(const color_t& color);

	pango_text& set_maximum_width(int width);

	pango_text& set_characters_per_line(const unsigned characters_per_line);

	pango_text& set_maximum_height(int height, bool multiline);

	pango_text& set_ellipse_mode(const PangoEllipsizeMode ellipse_mode);

	pango_text& set_alignment(const PangoAlignment alignment);

	pango_text& set_maximum_length(const std::size_t maximum_length);

	bool link_aware() const { return link_aware_; }

	pango_text& set_link_aware(bool b);

	pango_text& set_link_color(const color_t& color);

	pango_text& set_add_outline(bool do_add);

	/**
	* Mark a specific portion of text for highlighting. Used for selection box.
	* BGColor is set in set_text(), this just marks the area to be colored.
	* Markup not used because the user may enter their own markup or special characters
	* @param start_offset        Column offset of the cursor where selection/highlight starts
 	* @param end_offset          Column offset of the cursor where selection/highlight ends
 	* @param color               Highlight color
	*/
	void set_highlight_area(const unsigned start_offset, const unsigned end_offset, const color_t& color);

	void add_attribute_weight(const unsigned start_offset, const unsigned end_offset, PangoWeight weight);
	void add_attribute_style(const unsigned start_offset, const unsigned end_offset, PangoStyle style);
	void add_attribute_underline(const unsigned start_offset, const unsigned end_offset, PangoUnderline underline);
	void add_attribute_fg_color(const unsigned start_offset, const unsigned end_offset, const color_t& color);
	void add_attribute_size(const unsigned start_offset, const unsigned end_offset, int size);
	void add_attribute_font_family(const unsigned start_offset, const unsigned end_offset, std::string family);

	/** Clear all attributes */
	void clear_attribute_list();

private:

	/***** ***** ***** *****  Pango variables ***** ***** ***** *****/
	std::unique_ptr<PangoContext, std::function<void(void*)>> context_;
	std::unique_ptr<PangoLayout, std::function<void(void*)>> layout_;
	mutable PangoRectangle rect_;

	/** The text to draw (stored as UTF-8). */
	std::string text_;

	/** Does the text contain pango markup? If different render routines must be used. */
	bool markedup_text_;

	/** Are hyperlinks in the text marked-up, and will get_link return them. */
	bool link_aware_;

	/**
	 * The color to render links in.
	 *
	 * Links are formatted using pango &lt;span> as follows:
	 *
	 * &lt;span underline="single" color=" + link_color_ + ">
	 */
	color_t link_color_;

	/** The font family class used. */
	font::family_class font_class_;

	/** The font size to draw. */
	unsigned font_size_;

	/** The style of the font, this is an orred mask of the font flags. */
	FONT_STYLE font_style_;

	/** The foreground color. */
	color_t foreground_color_;

	/** Whether to add an outline effect. */
	bool add_outline_;

	/**
	 * The maximum width of the text.
	 *
	 * Values less or equal to 0 mean no maximum and are internally stored as
	 * -1, since that's the value pango uses for it.
	 *
	 * See @ref characters_per_line_.
	 */
	int maximum_width_;

	/**
	 * The number of characters per line.
	 *
	 * This can be used as an alternative of @ref maximum_width_. The user can
	 * select a number of characters on a line for wrapping. When the value is
	 * non-zero it determines the maximum width based on the average character
	 * width.
	 *
	 * If both @ref maximum_width_ and @ref characters_per_line_ are set the
	 * minimum of the two will be the maximum.
	 *
	 * @note Long lines are often harder to read, setting this value can
	 * automatically wrap on a number of characters regardless of the font
	 * size. Often 66 characters is considered the optimal value for a one
	 * column text.
	 */
	unsigned characters_per_line_;

	/**
	 * The maximum height of the text.
	 *
	 * Values less or equal to 0 mean no maximum and are internally stored as
	 * -1, since that's the value pango uses for it.
	 */
	int maximum_height_;

	/** The way too long text is shown depends on this mode. */
	PangoEllipsizeMode ellipse_mode_;

	/** The alignment of the text. */
	PangoAlignment alignment_;

	/** The maximum length of the text. */
	std::size_t maximum_length_;

	/**
	 * The text has two dirty states:
	 * - The setting of the state and the size calculations.
	 * - The rendering of the surface.
	 */

	/** The dirty state of the calculations. */
	mutable bool calculation_dirty_;

	/** Length of the text. */
	mutable std::size_t length_;

	unsigned attribute_start_offset_;
	unsigned attribute_end_offset_;
	color_t	highlight_color_;

	/**
	 * Global pango attribute list. All attributes in this list
	 * will be applied one by one to the text
	 */
	PangoAttrList* global_attribute_list_;

	/** The pixel scale, used to render high-DPI text. */
	int pixel_scale_;

	/** Recalculates the text layout. */
	void recalculate() const;

	/** Calculates surface size. */
	PangoRectangle calculate_size(PangoLayout& layout) const;

	/**
	 * Equivalent to create_surface(viewport), where the viewport's top-left is
	 * at (0,0) and the area is large enough to contain the full text.
	 *
	 * The top-left of the viewport will be at (0,0), regardless of the values
	 * of x and y in the rect_ member variable. If the x or y co-ordinates are
	 * non-zero, then x columns and y rows of blank space are included in the
	 * amount of memory allocated.
	 */
	surface create_surface();

	/**
	 * Renders the text to a surface that uses surface_buffer_ as its data store,
	 * the buffer will be allocated or reallocated as necessary.
	 *
	 * The surface's origin will correspond to viewport.x and viewport.y, the
	 * width and height will be at least viewport.w and viewport.h (although
	 * they may be larger).
	 *
	 * @param viewport The area to draw, which can be a subset of the text. This
	 * rectangle's coordinates use render-space's scale.
	 */
	surface create_surface(const SDL_Rect& viewport);

	/**
	 * This is part of create_surface(viewport). The separation is a legacy
	 * from workarounds to the size limits of cairo_surface_t.
	 */
	void render(PangoLayout& layout, const SDL_Rect& viewport, const unsigned stride);

	/**
	 * Buffer to store the image on.
	 *
	 * We use a cairo surface to draw on this buffer and then use the buffer as
	 * data source for the SDL_Surface. This means the buffer needs to be stored
	 * in the object, since SDL_Surface doesn't own its buffer.
	 */
	mutable std::vector<uint8_t> surface_buffer_;

	/**
	 * Sets the markup'ed text.
	 *
	 * It tries to set the text as markup. If the markup is invalid it will try
	 * a bit harder to recover from the errors and still set the markup.
	 *
	 * @param text                The text to set as markup.
	 * @param layout
	 *
	 * @returns                   Whether the markup was set or an
	 *                            unrecoverable error occurred and the text is
	 *                            set as plain text with an error message.
	 */
	bool set_markup(std::string_view text, PangoLayout& layout);

	bool validate_markup(std::string_view text, char** raw_text, std::string& semi_escaped) const;

	static void copy_layout_properties(PangoLayout& src, PangoLayout& dst);

	std::string format_links(std::string_view text) const;

	/**
	 * Adjust a texture's draw-width and height according to pixel scale.
	 *
	 * As fonts are rendered at output-scale, we need to do this just
	 * before returning the rendered texture. These attributes are stored
	 * as part of the returned texture object.
	 */
	texture with_draw_scale(const texture& t) const;

	/** Scale the given render-space size to draw-space, rounding up. */
	int to_draw_scale(int s) const;

	/** Scale the given render-space point to draw-space, rounding up. */
	point to_draw_scale(const point& p) const;

	/** Update pixel scale, if necessary. */
	void update_pixel_scale();
};

/**
 * Returns a reference to a static pango_text object.
 *
 * Since the class is essentially a render pipeline, there's no need for individual
 * areas of the game to own their own renderers. Not to mention it isn't a trivial
 * class; constructing one is likely to be expensive.
 */
pango_text& get_text_renderer();

/**
 * Returns the maximum glyph height of a font, in pixels.
 *
 * @param size                    Desired font size in pixels.
 * @param fclass                  Font family to use for measurement.
 * @param style                   Font style to select the correct variant for measurement.
 *
 * @returns                       The height of the tallest possible glyph for the selected
 *                                font. More specifically, the result is the sum of the maximum
 *                                ascent and descent lengths.
 */
int get_max_height(unsigned size, font::family_class fclass = font::FONT_SANS_SERIF, pango_text::FONT_STYLE style = pango_text::STYLE_NORMAL);

/* Returns the default line spacing factor
 * For now hardcoded here */
constexpr float get_line_spacing_factor() { return 1.3f; };

} // namespace font
