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

#include "config.hpp"
#include "gui/auxiliary/menu_item.hpp"

namespace gui2 {
menu_item::menu_item(const config& cfg, widget* parent)
	: enable_lua_ptr<menu_item>(this)
	, checkbox()
	, icon(cfg["icon"].str())
	, image()
	, label(cfg["label"].t_str())
	, details()
	, tooltip(cfg["tooltip"].t_str())
	, value(cfg["value"].str())
	, parent_(parent)
{
	// Checkboxes take precedence in column 1
	if(cfg.has_attribute("checkbox")) {
		checkbox = cfg["checkbox"].to_bool(false);
	}

	// Images take precedence in column 2
	if(cfg.has_attribute("image")) {
		image = cfg["image"].str();
	}

	if(cfg.has_attribute("details")) {
		details = cfg["details"].t_str();
	}
}

config menu_item::get_config() const
{
	config cfg;
	if(checkbox) {
		cfg["checkbox"] = checkbox.value();
	}
	if(details) {
		cfg["details"] = details.value();
	}
	cfg["icon"] = icon;
	if(image) {
		cfg["image"] = image.value();
	}
	cfg["label"] = label;
	cfg["tooltip"] = tooltip;
	cfg["value"] = value;
	return cfg;
}

}; // namespace gui2
