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
#include "editor/palette/tristate_button.hpp"

namespace editor {

class editor_toolkit;

template<class Item>
class editor_palette : public tristate_palette {

public:

	editor_palette(editor_display &gui, int item_size, std::size_t columns, editor_toolkit &toolkit)
		: tristate_palette()
		, groups_()
		, gui_(gui)
		, item_size_(item_size)
//TODO avoid magic number
		, item_space_(item_size + 3)
		, columns_(columns)
		, group_map_()
		, item_map_()
		, items_start_(0)
		, non_core_items_()
		, active_group_()
		, selected_fg_item_()
		, selected_bg_item_()
		, toolkit_(toolkit)
		, buttons_()
	{
	}

	virtual sdl_handler_vector handler_members() override;

	void set_start_item(std::size_t index) override { items_start_ = index; }

	std::size_t start_num(void) override { return items_start_; }

	/** Menu expanding for palette group list */
	void expand_palette_groups_menu(std::vector<config>& items, int i) override;

	void set_group(std::size_t index) override;

	const std::vector<item_group>& get_groups() const override { return groups_; }

	/** Called by draw_manager to validate layout before drawing. */
	virtual void layout() override;
	/** Called by widget::draw() */
	virtual void draw_contents() override;

	void next_group() override {
		set_group( (active_group_index() +1) % (groups_.size()) );
	}
	void prev_group() override {
		set_group( (active_group_index() -1) % (groups_.size()) );
	}

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

	void swap() override;

	virtual std::string get_help_string() const = 0;

	/** Return the currently selected foreground/background item. */
	const Item& selected_fg_item() const { return item_map_.find(selected_fg_item_)->second; }
	const Item& selected_bg_item() const { return item_map_.find(selected_bg_item_)->second; }

private:

	std::size_t active_group_index();

	/** Setup item image and tooltip. */
	virtual void setup_item(
		const Item& item,
		texture& item_base_image,
		texture& item_overlay_image,
		std::stringstream& tooltip
	) = 0;

	virtual const std::string& get_id(const Item& item) = 0;

	/** Setup the internal data structure. */
	virtual void setup(const game_config_view& cfg) = 0;

	virtual const std::string& active_group_id() {return active_group_;}

	virtual bool is_selected_fg_item(const std::string& id);
	virtual bool is_selected_bg_item(const std::string& id);

	/** Return the number of items in the currently-active group. */
	std::size_t num_items() override;

	void hide(bool hidden) override;

protected:
	/**
	 * Sets a group active id
	 *
	 * This can result in no visible
	 * selected items.
	 */
	virtual void set_group(const std::string& id);
	const std::vector<std::string>& active_group() { return group_map_[active_group_]; }

	/** Select a foreground item. */
	virtual void select_fg_item(const std::string& item_id) override;
	virtual void select_bg_item(const std::string& item_id) override;

	/**
	 * The editor_groups as defined in editor-groups.cfg.
	 *
	 * Note the user must make sure the id's here are the same as the
	 * editor_group in terrain.cfg.
	 */
	std::vector<item_group> groups_;

	editor_display &gui_;

	/**
	 * Both the width and the height of the square buttons.
	 *
	 * This is a size measured in pixels, and should match the type of
	 * SDL_rect.w and SDL_rect.h.
	 */
	int item_size_;
	/**
	 * item_size_ plus some padding.
	 */
	int item_space_;

	/**
	 * Number of items per row.
	 */
	std::size_t columns_;

protected:
	std::map<std::string, std::vector<std::string>> group_map_;

	typedef std::map<std::string, Item> item_map;
	item_map item_map_;
	/**
	 * Index of the item at the top-left of the visible area, used for scrolling up and down.
	 */
	std::size_t items_start_;
	std::set<std::string> non_core_items_;

private:
	std::string active_group_;
	std::string selected_fg_item_;
	std::string selected_bg_item_;

	editor_toolkit& toolkit_;
	std::vector<gui::tristate_button> buttons_;
};


} //end namespace editor
