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

#pragma once

#include "config.hpp"
#include "font/font_options.hpp"
#include "font/text.hpp"
#include "gui/core/linked_group_definition.hpp"
#include "gui/auxiliary/typed_formula.hpp"

#include <vector>

namespace gui2
{

/**
 * Contains the state info for a resolution.
 *
 * At the moment all states are the same so there is no need to use
 * inheritance. If that is needed at some point the containers should contain
 * pointers
 */
struct state_definition
{
	explicit state_definition(const config& cfg);

	config canvas_cfg_;
};

/**
 * Base class of a resolution, contains the common keys for a resolution.
 *
 * Depending on the resolution a widget can look different. Resolutions are evaluated in order of appearance.
 * The window_width and window_height are the upper limit this resolution is valid for.
 * When one of the sizes gets above the limit, the next resolution is selected.
 * There's one special case where both values are 0. This resolution always matches.
 * (Resolution definitions behind that one will never be picked.)
 * This resolution can be used as upper limit or if there's only one resolution.
 *
 * The default (and also minimum) size of a button is determined by two items, the wanted default size and the size needed for the text.
 * The size of the text differs per used widget so needs to be determined per button.
 *
 * Container widgets like panels and windows have other rules for their sizes.
 * Their sizes are based on the size of their children (and the border they need themselves).
 * It's wise to set all sizes to 0 for these kind of widgets.
 *
 * Key              |Type                                |Default  |Description
 * -----------------|------------------------------------|---------|-------------
 * window_width     | @ref guivartype_unsigned "unsigned"|0        |Width of the application window.
 * window_height    | @ref guivartype_unsigned "unsigned"|0        |Height of the application window.
 * min_width        | @ref guivartype_unsigned "unsigned"|0        |The minimum width of the widget.
 * min_height       | @ref guivartype_unsigned "unsigned"|0        |The minimum height of the widget.
 * default_width    | @ref guivartype_unsigned "unsigned"|0        |The default width of the widget.
 * default_height   | @ref guivartype_unsigned "unsigned"|0        |The default height of the widget.
 * max_width        | @ref guivartype_unsigned "unsigned"|0        |TThe maximum width of the widget.
 * max_height       | @ref guivartype_unsigned "unsigned"|0        |The maximum height of the widget.
 * text_extra_width | @ref guivartype_unsigned "unsigned"|0        |The extra width needed to determine the minimal size for the text.
 * text_extra_height| @ref guivartype_unsigned "unsigned"|0        |The extra height needed to determine the minimal size for the text.
 * text_font_family | font_family                        |""       |The font family, needed to determine the minimal size for the text.
 * text_font_size   | @ref guivartype_unsigned "unsigned"|0        |The font size, which needs to be used to determine the minimal size for the text.
 * text_font_style  | font_style                         |""       |The font style, which needs to be used to determine the minimal size for the text.
 * state            | @ref guivartype_section "section"  |mandatory|Every widget has one or more state sections. Note they aren't called state but state_xxx the exact names are listed per widget.
 */
struct resolution_definition
{
	explicit resolution_definition(const config& cfg);

	unsigned window_width;
	unsigned window_height;

	unsigned min_width;
	unsigned min_height;

	unsigned default_width;
	unsigned default_height;

	unsigned max_width;
	unsigned max_height;

	std::vector<linked_group_definition> linked_groups;

	unsigned text_extra_width;
	unsigned text_extra_height;
	typed_formula<unsigned> text_font_size;

	font::family_class text_font_family;
	font::pango_text::FONT_STYLE text_font_style;

	std::vector<state_definition> state;
};

typedef std::shared_ptr<resolution_definition>
resolution_definition_ptr;

typedef std::shared_ptr<const resolution_definition>
resolution_definition_const_ptr;

struct styled_widget_definition
{
	explicit styled_widget_definition(const config& cfg);

	template<class T>
	void load_resolutions(const config& cfg)
	{
		for(const config& resolution : cfg.child_range("resolution")) {
			resolutions.emplace_back(std::make_shared<T>(resolution));
		}
	}

	std::string id;
	t_string description;

	std::vector<resolution_definition_ptr> resolutions;
};

typedef std::shared_ptr<styled_widget_definition> styled_widget_definition_ptr;

} // namespace gui2
