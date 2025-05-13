/*
	Copyright (C) 2025 - 2025
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

#include <pango/pango-layout.h>
#include <utility>

struct color_t;

namespace font
{
/** Helper class to encapsulate the management of a PangoAttrList. */
class attribute_list
{
public:
	attribute_list()
		: attributes_(pango_attr_list_new())
	{
	}

	attribute_list(attribute_list&& o)
		: attributes_(std::exchange(o.attributes_, nullptr))
	{
	}

	~attribute_list()
	{
		if(attributes_) {
			pango_attr_list_unref(attributes_);
		}
	}

	attribute_list(const attribute_list&) = delete;
	attribute_list& operator=(const attribute_list&) = delete;

	attribute_list& operator=(attribute_list&& o)
	{
		attributes_ = std::exchange(o.attributes_, nullptr);
		return *this;
	};

	void insert(PangoAttribute* attr)
	{
		pango_attr_list_insert(attributes_, attr);
	}

	void modify(PangoAttribute* attr)
	{
		pango_attr_list_change(attributes_, attr);
	}

	void apply_to(PangoLayout* layout) const
	{
		pango_layout_set_attributes(layout, attributes_);
	}

	void splice_into(PangoAttrList* target) const
	{
		pango_attr_list_splice(target, attributes_, 0, 0);
	}

private:
	PangoAttrList* attributes_;
};

//
// The following free functions are thin wrappers around the corresponding
// pango_attr_new methods. For more details, refer to the Pango docs.
//

/**
 * Add Pango font weight attribute to a specific portion of text. This changes the font weight
 * of the corresponding part of the text.
 *
 * @param list                The attribute list to which to append this attribute.
 * @param offset_start        Byte index of the cursor where font weight change starts
 * @param offset_end          Byte index of the cursor where font weight change ends
 * @param weight              Pango font weight
 */
void add_attribute_weight(attribute_list& list, unsigned offset_start, unsigned offset_end, PangoWeight weight);

/**
 * Add Pango font style attribute to a specific portion of text, used to set italic/oblique text
 *
 * @param list                The attribute list to which to append this attribute.
 * @param offset_start        Byte index of the cursor where font style change starts
 * @param offset_end          Byte index of the cursor where font style change ends
 * @param style               Pango font style (normal/italic/oblique)
 */
void add_attribute_style(attribute_list& list, unsigned offset_start, unsigned offset_end, PangoStyle style);

/**
 * Add Pango underline attribute to a specific portion of text. This adds an underline to the
 * corresponding part of the text.
 *
 * @param list                The attribute list to which to append this attribute.
 * @param offset_start        Byte index of the cursor where underline starts
 * @param offset_end          Byte index of the cursor where underline change ends
 * @param underline           Pango underline style
 */
void add_attribute_underline(attribute_list& list, unsigned offset_start, unsigned offset_end, PangoUnderline underline);

/**
 * Add Pango fg color attribute to a specific portion of text. This changes the foreground
 * color of the corresponding part of the text.
 *
 * @param list                The attribute list to which to append this attribute.
 * @param offset_start        Byte index of the cursor where color change starts
 * @param offset_end          Byte index of the cursor where color change ends
 * @param color               Foreground color
 */
void add_attribute_fg_color(attribute_list& list, unsigned offset_start, unsigned offset_end, const color_t& color);

/**
 * Mark a specific portion of text for highlighting. Used for selection box.
 * BGColor is set in set_text(), this just marks the area to be colored.
 * Markup not used because the user may enter their own markup or special characters
 *
 * @param list                The attribute list to which to append this attribute.
 * @param offset_start        Byte index of the cursor where selection/highlight starts
 * @param offset_end          Byte index of the cursor where selection/highlight ends
 * @param color               Highlight/Background color
 */
void add_attribute_bg_color(attribute_list& list, unsigned offset_start, unsigned offset_end, const color_t& color);

/**
 * Add Pango font size attribute to a specific portion of text. This changes the font size
 * of the corresponding part of the text.
 *
 * @param list                The attribute list to which to append this attribute.
 * @param offset_start        Byte index of the cursor where size change starts
 * @param offset_end          Byte index of the cursor where size change ends
 * @param size                Font size
 */
void add_attribute_size(attribute_list& list, unsigned offset_start, unsigned offset_end, int size);

/**
 * Add Pango font family attribute to a specific portion of text. This changes
 * the font family of the corresponding part of the text.
 *
 * @param list                The attribute list to which to append this attribute.
 * @param offset_start        Byte index of the cursor where size change starts
 * @param offset_end          Byte index of the cursor where size change ends
 * @param family              The font family
 */
void add_attribute_font_family(attribute_list& list, unsigned offset_start, unsigned offset_end, font::family_class family);

/**
 * Add Pango line height attribute to a specific portion of text. This changes
 * the line height of the corresponding part of the text.
 *
 * @param list                The attribute list to which to append this attribute.
 * @param offset_start        Byte index of the cursor where size change starts
 * @param offset_end          Byte index of the cursor where size change ends
 * @param factor              The line height factor, i.e., `new_line_height/old_line_height`
 */
void add_attribute_line_height(attribute_list& list, unsigned offset_start, unsigned offset_end, const double factor);

/**
 * Add Pango shape attribute to a specific portion of text. This replaces
 * the text within the start and end offsets with an inline image.
 * The caller should ensure that the portion of text between the given offset
 * range is filled with dummy characters, such as
 *
 * @param list                The attribute list to which to append this attribute.
 * @param offset_start        Byte index of the cursor where size change starts
 * @param offset_end          Byte index of the cursor where size change ends
 * @param image_path         Path to the image to be shown inline
 */
void add_attribute_image_shape(attribute_list& list, unsigned offset_start, unsigned offset_end, const std::string& image_path);

} // namespace font
