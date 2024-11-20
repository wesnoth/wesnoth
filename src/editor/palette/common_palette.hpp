/*
	Copyright (C) 2003 - 2024
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

#include "config.hpp"
#include "tstring.hpp"
#include "widgets/widget.hpp"

struct SDL_Rect;

namespace editor {

/**
 * Stores the info about the groups in a nice format.
 */
struct item_group
{
	item_group(const config& cfg)
		: id(cfg["id"])
		, name(cfg["name"].t_str())
		, icon(cfg["icon"])
		, core(cfg["core"].to_bool())
	{}

	std::string id;
	t_string name;
	std::string icon;
	bool core;
};


class common_palette  : public gui::widget {

public:

	common_palette() : gui::widget(true) {}

	virtual ~common_palette() {}

	//event handling

	virtual sdl_handler_vector handler_members() { return sdl_handler_vector(); }


	/** Scroll the editor-palette up one step if possible. */
	virtual bool scroll_up() = 0;
	virtual bool can_scroll_up() = 0;

	/** Scroll the editor-palette down one step if possible. */
	virtual bool scroll_down() = 0;
	virtual bool can_scroll_down() = 0;

	//drawing
	virtual void adjust_size(const SDL_Rect& target) = 0;

	//group
	virtual void set_group(std::size_t index) = 0;
	virtual void next_group() = 0;
	virtual void prev_group() = 0;
	virtual const std::vector<item_group>& get_groups() const = 0;

	/** Menu expanding for palette group list */
	virtual void expand_palette_groups_menu(std::vector<config>& items, int i) = 0;

	//item
	virtual std::size_t num_items() = 0;
	virtual std::size_t start_num() = 0;
	virtual void set_start_item(std::size_t index) = 0;

	/** Whether the hotkey system should the enable GUI button connected to swap(). */
	virtual bool supports_swap() { return true; }
	/**
	 * For tools which support fg and bg items, exchange the two items. Typically, fg and bg mean
	 * that they're placed by left or right mouse clicks, respectively.
	 *
	 * There's a mismatch between class structure and responsibilities here, as part of the UX isn't
	 * part of the palette. The tool decides what right-click does. Even for the scenery item tool,
	 * where right-click deletes an item, fg and bg provide a way to switch between two items.
	 */
	virtual void swap() = 0;

	virtual std::vector<std::string> action_pressed() const { return std::vector<std::string>(); }
};

// a palette containing tristtate buttons.
class tristate_palette : public common_palette
{
public:
	tristate_palette()
		: common_palette()
	{
	}
	virtual void select_fg_item(const std::string& item_id) = 0;
	virtual void select_bg_item(const std::string& item_id) = 0;
};

}
