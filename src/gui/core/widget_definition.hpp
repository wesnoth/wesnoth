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

#pragma once

#include "config.hpp"
#include "font/font_options.hpp"
#include "font/text.hpp"
#include "gui/core/linked_group_definition.hpp"

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

/** Base class of a resolution, contains the common keys for a resolution. */
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
	unsigned text_font_size;

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
