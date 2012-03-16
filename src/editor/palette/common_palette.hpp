/* $Id$ */
/*
   Copyright (C) 2003 - 2012 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef COMMON_PALETTES_H_INCLUDED
#define COMMON_PALETTES_H_INCLUDED

namespace editor {

/**
 * Stores the info about the groups in a nice format.
 */
struct item_group
{
	item_group(const config& cfg):
		id(cfg["id"]), name(cfg["name"].t_str()),
		icon(cfg["icon"]), core(cfg["core"].to_bool()) {};

	std::string id;
	t_string name;
	std::string icon;
    bool core;
};

class common_palette {

public:

	virtual ~common_palette() {}

	virtual void set_group(size_t index) = 0;

	virtual size_t active_group_index() = 0;

	virtual const std::vector<item_group>& get_groups() = 0;

	/** Scroll the editor-palette up one step if possible. */
	virtual void scroll_up() = 0;

	/** Scroll the editor-palette down one step if possible. */
	virtual void scroll_down() = 0;

	virtual void swap() = 0;

	virtual void adjust_size() = 0;

	virtual void draw(bool) = 0;
};

}

#endif
