/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#define GETTEXT_DOMAIN "wesnoth-editor"

#include "gui/dialogs/editor_new_map.hpp"
#include "gui/widgets/integer_selector.hpp"
#include "gui/dialogs/field.hpp"

namespace gui2 {

teditor_new_map::teditor_new_map() :
	map_width_(register_integer("width", false)),
	map_height_(register_integer("height", false))
{
}

void teditor_new_map::set_map_width(int value)
{
	map_width_->set_cache_value(value);
}

int teditor_new_map::map_width() const
{
	return map_width_->get_cache_value();
}

void teditor_new_map::set_map_height(int value)
{
	map_height_->set_cache_value(value);
}

int teditor_new_map::map_height() const
{
	return map_height_->get_cache_value();
}

twindow* teditor_new_map::build_window(CVideo& video)
{
	return build(video, get_id(EDITOR_NEW_MAP));
}

} // namespace gui2
