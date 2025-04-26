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

#include "font/attributes.hpp"
#include "font/cairo.hpp"
#include "font/font_config.hpp"

#include "color.hpp"
#include "gui/core/log.hpp"
#include "picture.hpp"
#include "preferences/preferences.hpp"
#include "sdl/surface.hpp"
#include "tstring.hpp"
#include "video.hpp"

#include <memory>

namespace font
{
namespace
{
/**
 * Private helper class to manage a single PangoAttribute.
 *
 * This object owns its attribute until relinquished to an attribute_list
 * by calling @ref add_to or @ref modify_in.
 */
class attribute
{
public:
	attribute(PangoAttribute* attr, unsigned offset_start, unsigned offset_end)
		: value_(attr, &pango_attribute_destroy)
	{
		attr->start_index = offset_start;
		attr->end_index = offset_end;
	}

	void add_to(font::attribute_list& list)
	{
		list.insert(value_.release());
	}

	void modify_in(font::attribute_list& list)
	{
		list.modify(value_.release());
	}

private:
	std::unique_ptr<PangoAttribute, void(*)(PangoAttribute*)> value_;
};
} // anon namespace

void add_attribute_size(attribute_list& list, unsigned offset_start, unsigned offset_end, int size)
{
	// TODO: we shouldn't be doing scaling stuff here...
	size = prefs::get().font_scaled(size) * video::get_pixel_scale();

	attribute attr {
		pango_attr_size_new_absolute(PANGO_SCALE * size),
		offset_start, offset_end
	};

	DBG_GUI_D << "attribute: size";
	DBG_GUI_D << "attribute start: " << offset_start << " end : " << offset_end;

	attr.add_to(list);
}

void add_attribute_weight(attribute_list& list, unsigned offset_start, unsigned offset_end, PangoWeight weight)
{
	attribute attr {
		pango_attr_weight_new(weight),
		offset_start, offset_end
	};

	DBG_GUI_D << "attribute: weight";
	DBG_GUI_D << "attribute start: " << offset_start << " end : " << offset_end;

	attr.add_to(list);
}

void add_attribute_style(attribute_list& list, unsigned offset_start, unsigned offset_end, PangoStyle style)
{
	attribute attr {
		pango_attr_style_new(style),
		offset_start, offset_end
	};

	DBG_GUI_D << "attribute: style";
	DBG_GUI_D << "attribute start: " << offset_start << " end : " << offset_end;

	attr.add_to(list);
}

void add_attribute_underline(attribute_list& list, unsigned offset_start, unsigned offset_end, PangoUnderline underline)
{
	attribute attr {
		pango_attr_underline_new(underline),
		offset_start, offset_end
	};

	DBG_GUI_D << "attribute: underline";
	DBG_GUI_D << "attribute start: " << offset_start << " end : " << offset_end;

	attr.add_to(list);
}

void add_attribute_fg_color(attribute_list& list, unsigned offset_start, unsigned offset_end, const color_t& color)
{
	auto [col_r, col_g, col_b] = color.to_pango_format();

	attribute attr {
		pango_attr_foreground_new(col_r, col_g, col_b),
		offset_start, offset_end
	};

	DBG_GUI_D << "attribute: fg color";
	DBG_GUI_D << "attribute start: " << offset_start << " end : " << offset_end;
	DBG_GUI_D << "color: " << col_r << "," << col_g << "," << col_b;

	attr.add_to(list);
}

void add_attribute_bg_color(attribute_list& list, unsigned offset_start, unsigned offset_end, const color_t& color)
{
	auto [col_r, col_g, col_b] = color.to_pango_format();

	attribute attr {
		pango_attr_background_new(col_r, col_g, col_b),
		offset_start, offset_end
	};

	DBG_GUI_D << "highlight start: " << offset_start << "end : " << offset_end;
	DBG_GUI_D << "highlight color: " << col_r << "," << col_g << "," << col_b;

	attr.modify_in(list);
}

void add_attribute_font_family(attribute_list& list, unsigned offset_start, unsigned offset_end, font::family_class family)
{
	const t_string& family_name = get_font_families(family);

	attribute attr {
		pango_attr_family_new(family_name.c_str()),
		offset_start, offset_end
	};

	DBG_GUI_D << "attribute: font family";
	DBG_GUI_D << "attribute start: " << offset_start << " end : " << offset_end;
	DBG_GUI_D << "font family: " << family_name;

	attr.add_to(list);
}

void add_attribute_line_height(attribute_list& list, unsigned offset_start, unsigned offset_end, const double factor)
{
	attribute attr {
		pango_attr_line_height_new(factor),
		offset_start, offset_end
	};

	DBG_GUI_D << "attribute: line height (relative)";
	DBG_GUI_D << "attribute start: " << offset_start << " end : " << offset_end;
	DBG_GUI_D << "factor: " << factor;

	attr.add_to(list);
}

void add_attribute_image_shape(attribute_list& list, unsigned offset_start, unsigned offset_end, const std::string& image_path)
{
	surface surf = ::image::get_surface(image_path);
	int scale = video::get_pixel_scale();
	auto cairo_surface = cairo::create_surface(
		reinterpret_cast<uint8_t*>(surf->pixels), point(surf->w, surf->h));
	PangoRectangle bounds {
		0,
		-PANGO_SCALE * surf->h * scale,
		PANGO_SCALE * surf->w * scale,
		PANGO_SCALE * surf->h * scale
	};

	attribute attr {
		pango_attr_shape_new_with_data(&bounds, &bounds, cairo_surface.release(), nullptr, nullptr),
		offset_start, offset_end
	};

	DBG_GUI_D << "attribute: shape";
	DBG_GUI_D << "attribute start: " << offset_start << " end : " << offset_end;
	DBG_GUI_D << "image path: " << image_path;

	attr.add_to(list);
}

} // namespace font
