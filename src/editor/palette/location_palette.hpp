/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "common_palette.hpp"
#include "tristate_button.hpp"

class location_palette_item;
class location_palette_button;

namespace editor {

class location_palette : public common_palette {

public:

	location_palette(editor_display &gui, const config& /*cfg*/, mouse_action** active_mouse_action);


	virtual sdl_handler_vector handler_members();

	void set_start_item(size_t index) { items_start_ = index; }

	size_t start_num(void) { return items_start_; }

	/** Menu expanding for palette group list */
	void expand_palette_groups_menu(std::vector< std::pair<std::string, std::string> >&) override {}
	void expand_palette_groups_menu(std::vector<std::string>&) override {}

	virtual void set_group(size_t /*index*/) override {}
	virtual void next_group() override {}
	virtual void prev_group() override {}
	virtual const std::vector<item_group>& get_groups() const override { static const std::vector<item_group> empty; return empty; }

	virtual void draw() {
		widget::draw();
	}
	virtual void draw_contents();

	/**
	 * Update the size of this widget.
	 *
	 * Use if the size_specs have changed.
	 */
	void adjust_size(const SDL_Rect& target);

	virtual bool scroll_up();
	virtual bool can_scroll_up();
	virtual bool scroll_down();
	virtual bool can_scroll_down();

	void swap() override {}
	bool can_swap() { return false; }

	virtual std::string get_help_string() { return ""; }

	/** Return the currently selected item. */
	const std::string& selected_item() const { return selected_item_; }

	virtual void select_item(const std::string& item_id);
	virtual std::vector<std::string> action_pressed() const override;
	void add_item(const std::string& id);
	~location_palette();
private:

	/** Scroll the editor-palette to the top. */
	void scroll_top();

	/** Scroll the editor-palette to the bottom. */
	void scroll_bottom();

	virtual bool is_selected_item(const std::string& id);

	/** Return the number of items in the palette. */
	int num_items();
	/** Return the maximum number of items shown at the same time. */
	int num_visible_items();

	void hide(bool hidden) override;
protected:

	editor_display &gui_;

	int item_size_;
	// the heigh of a row, the size of an item including borders.
	int item_space_;

private:
	unsigned int palette_y_;
	unsigned int palette_x_;

protected:
	//the current scrolling position
	int items_start_;

private:
	std::string selected_item_;
	std::vector<std::string> items_;
    mouse_action** active_mouse_action_;
	boost::ptr_vector<location_palette_item> buttons_;
	std::unique_ptr<location_palette_button> button_add_;
	std::unique_ptr<location_palette_button> button_delete_;
	std::unique_ptr<location_palette_button> button_goto_;
    int help_handle_;
	editor_display& disp_;
};


} //end namespace editor

