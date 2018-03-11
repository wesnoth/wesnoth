/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
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
#include "editor/palette/common_palette.hpp"
#include "editor/palette/tristate_button.hpp"

#include <boost/ptr_container/ptr_vector.hpp>

class location_palette_item;
class location_palette_button;

namespace editor {

class editor_toolkit;

class location_palette : public common_palette {

public:

	location_palette(editor_display &gui, const config& /*cfg*/,
	                 editor_toolkit &toolkit);


	virtual sdl_handler_vector handler_members() override;

	void set_start_item(size_t index) override { items_start_ = index; }

	size_t start_num(void) override { return items_start_; }

	/** Menu expanding for palette group list */
	void expand_palette_groups_menu(std::vector<config>& items, int i) override
	{
		items.erase(items.begin() + i);
	}

	virtual void set_group(size_t /*index*/) override {}
	virtual void next_group() override {}
	virtual void prev_group() override {}
	virtual const std::vector<item_group>& get_groups() const override { static const std::vector<item_group> empty; return empty; }

	virtual void draw() override {
		widget::draw();
	}
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

	void swap() override {}
	bool can_swap() { return false; }

	virtual std::string get_help_string() { return ""; }

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

	/** Scroll the editor-palette to the bottom. */
	void scroll_bottom();

	virtual bool is_selected_item(const std::string& id);

	/** Return the number of items in the palette. */
	int num_items() override;
	/** Return the maximum number of items shown at the same time. */
	int num_visible_items();
protected:

	int item_size_;
	// the height of a row, the size of an item including borders.
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
	editor_toolkit& toolkit_;
	boost::ptr_vector<location_palette_item> buttons_;
	std::unique_ptr<location_palette_button> button_add_;
	std::unique_ptr<location_palette_button> button_delete_;
	std::unique_ptr<location_palette_button> button_goto_;
    int help_handle_;
	editor_display& disp_;
};


} //end namespace editor
