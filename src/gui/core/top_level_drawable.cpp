/*
	Copyright (C) 2022 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "gui/core/top_level_drawable.hpp"

#include "draw_manager.hpp"

namespace gui2
{

top_level_drawable::top_level_drawable()
{
	draw_manager::register_drawable(this);
}

top_level_drawable::~top_level_drawable()
{
	draw_manager::deregister_drawable(this);
}

top_level_drawable::top_level_drawable(const top_level_drawable&)
{
	draw_manager::register_drawable(this);
}

top_level_drawable& top_level_drawable::operator=(const top_level_drawable&)
{
	draw_manager::register_drawable(this);
	return *this;
}

top_level_drawable::top_level_drawable(top_level_drawable&&)
{
	draw_manager::register_drawable(this);
}

top_level_drawable& top_level_drawable::operator=(top_level_drawable&&)
{
	draw_manager::register_drawable(this);
	return *this;
}

} // namespace gui2
