/*
    Copyright (C) 2024
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

#include "gui/widgets/widget.hpp"
#include "scripting/lua_ptr.hpp"
#include "utils/optional_fwd.hpp"

#include <string>

namespace gui2 {

class menu_item : public enable_lua_ptr<menu_item>
{
public:

	menu_item(const config& cfg, widget* parent = nullptr);

	config get_config() const;

	/** If present, column 1 will have a toggle button. The value indicates its initial state. */
	utils::optional<bool> checkbox;

	/** If no checkbox is present, the icon at this path will be shown in column 1. */
	std::string icon;

	/** If present, column 2 will display the image at this path. */
	utils::optional<std::string> image;

	/** If no image is present, this text will be shown in column 2. */
	t_string label;

	/** If present, this text will be shown in column 3. */
	utils::optional<t_string> details;

	/** Tooltip text for the entire row. */
	t_string tooltip;

	/** Canonical value for the selected item */
	std::string value;

	widget* get_parent() const
	{
		return parent_;
	}

private:
	widget* parent_;
};

}; // namespace gui2
