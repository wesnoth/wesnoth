/*
	Copyright (C) 2003 - 2025
	by David White <dave@whitevine.net>
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

#include "gui/core/top_level_drawable.hpp"
#include "sdl/rect.hpp"

#include <string>

namespace tooltips {

class manager : public gui2::top_level_drawable
{
public:
	manager();
	~manager();
	// TLD interface
	virtual void layout() override;
	virtual bool expose(const rect& region) override;
	virtual rect screen_location() override;
};

void clear_tooltips();
void clear_tooltips(const rect& rect);
int  add_tooltip(const rect& rect, const std::string& message, const std::string& action ="");
bool update_tooltip(int id, const rect& rect, const std::string& message);
void remove_tooltip(int id);
void process(int mousex, int mousey);

// Check if we clicked on a tooltip having an action.
// If it is, then execute the action and return true
// (only possible action are opening help page for the moment)
bool click(int mousex, int mousey);

}
