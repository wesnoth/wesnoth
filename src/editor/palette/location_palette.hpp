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

#include "editor/editor_display.hpp"
#include "editor/palette/common_palette.hpp"

class location_palette_item;
class location_palette_button;
class game_config_view;
namespace editor {

class editor_toolkit;

/**
 * List of starting locations and location ids. Shows a single-column list, with buttons to add
 * new items to the list and to jump to that location on the map.
 */
class location_palette : public common_palette {

public:

	location_palette(editor_display &gui, editor_toolkit &toolkit);


	virtual sdl_handler_vector handler_members() override;

	void set_start_item(std::size_t index) override { items_start_ = index; }

	std::size_t start_num(void) override { return items_start_; }

	/** Menu expanding for palette group list */
	void expand_palette_groups_menu(std::vector<config>& items, int i) override
	{
		items.erase(items.begin() + i);
	}

	virtual void set_group(std::size_t /*index*/) override {}
	virtual void next_group() override {}
	virtual void prev_group() override {}
	virtual const std::vector<item_group>& get_groups() const override { static const std::vector<item_group> empty; return empty; }

	/** Called by draw_manager to validate layout before drawing. */
	virtual void layout() override;
	/** Called by widget::draw() */
	virtual void draw_contents() override;

	/**
	 * Update the size of this widget.
	 *
	 * Use if the size_specs have changed.
	 */
	void adjust_size(const SDL_Rect& target) override;

	virtual bool scroll_up() override;
	virtual bool can_scroll_up() override;
	virtual bool scroll_down() override;
	virtual bool can_scroll_down() override;

	virtual void swap() override {}
	virtual bool supports_swap() override { return false; }

	std::string get_help_string() const { return ""; }

	/** Return the currently selected item. */
	const std::string& selected_item() const { return selected_item_; }

	virtual void select_item(const std::string& item_id);
	virtual std::vector<std::string> action_pressed() const override;
	void add_item(const std::string& id);
	~location_palette();
	void hide(bool hidden) override;

private:
	/** Scroll the editor-palette to the top. */
	void scroll_top();

	virtual bool is_selected_item(const std::string& id);

	/** Return the number of items in the palette. */
	std::size_t num_items() override;
	/**
	 * Return the number of GUI elements that can show items. Some of these may be hidden, if there
	 * are more of them than items to show, or if the palette has been scrolled to the bottom.
	 */
	std::size_t num_visible_items();
protected:

	int item_size_;
	// the height of a row, the size of an item including borders.
	int item_space_;

protected:
	// the current scrolling position
	std::size_t items_start_;

private:
	std::string selected_item_;
	std::vector<std::string> items_;
	editor_toolkit& toolkit_;
	std::vector<location_palette_item> buttons_;
	std::unique_ptr<location_palette_button> button_add_;
	std::unique_ptr<location_palette_button> button_delete_;
	std::unique_ptr<location_palette_button> button_goto_;
	editor_display& disp_;
};


} //end namespace editor
