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

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor/palette/editor_palettes.hpp"

#include "gettext.hpp"
#include "serialization/markup.hpp"
#include "overlay.hpp"
#include "filesystem.hpp"

#include "editor/toolkit/editor_toolkit.hpp"

namespace editor {

template<class Item>
sdl_handler_vector editor_palette<Item>::handler_members()
{
	sdl_handler_vector h;
	for (gui::widget& b : buttons_) {
		h.push_back(&b);
	}
	return h;
}

template<class Item>
void editor_palette<Item>::expand_palette_groups_menu(std::vector<config>& items, int i)
{
	auto pos = items.erase(items.begin() + i);

	std::vector<config> groups;
	const std::vector<item_group>& item_groups = get_groups();

	for (std::size_t mci = 0; mci < item_groups.size(); ++mci) {
		std::string groupname = item_groups[mci].name;
		if (groupname.empty()) {
			groupname = _("(Unknown Group)");
		}
		std::string img = item_groups[mci].icon + "_30";
		if (mci == active_group_index()) {
			std::string pressed_img = img + "-pressed.png";
			if(filesystem::get_binary_file_location("images", pressed_img).has_value()) {
				img = pressed_img;
			} else {
				img += ".png~CS(70,70,0)";
			}
		} else {
			img += ".png";
		}

		groups.emplace_back(
			"label", groupname,
			"icon", img
		);
	}

	items.insert(pos, groups.begin(), groups.end());
}

template<class Item>
bool editor_palette<Item>::scroll_up()
{
	bool scrolled = false;
	if(can_scroll_up()) {
		// This should only be reachable with items_start_ being a multiple of columns_, but guard against underflow anyway.
		if(items_start_ < columns_) {
			items_start_ = 0;
		} else {
			items_start_ -= columns_;
		}
		scrolled = true;
		set_dirty(true);
	}
	return scrolled;
}

template<class Item>
bool editor_palette<Item>::can_scroll_up()
{
	return (items_start_ != 0);
}

template<class Item>
bool editor_palette<Item>::can_scroll_down()
{
	return (items_start_ + buttons_.size() < num_items());
}

template<class Item>
bool editor_palette<Item>::scroll_down()
{
	bool scrolled = false;
	if(can_scroll_down()) {
		items_start_ += columns_;
		scrolled = true;
		set_dirty(true);
	}
	return scrolled;
}

template<class Item>
void editor_palette<Item>::set_group(const std::string& id)
{
	assert(!id.empty());

	bool found = false;
	for (const item_group& group : groups_) {
		if (group.id == id) {
			found = true;
			std::shared_ptr<gui::button> palette_menu_button = gui_.find_menu_button("menu-editor-terrain");
			if (palette_menu_button) {
				palette_menu_button->set_tooltip_string(group.name);
				palette_menu_button->set_overlay(group.icon);
			}
		}
	}
	assert(found);

	active_group_ = id;

	if(active_group().empty()) {
		ERR_ED << "No items found in group with the id: '" << id << "'.";
	}
}

template<class Item>
void editor_palette<Item>::set_group(std::size_t index)
{
	assert(groups_.size() > index);
	set_group(groups_[index].id);
}

template<class Item>
std::size_t editor_palette<Item>::active_group_index()
{
	assert(!active_group_.empty());

	for (std::size_t i = 0 ; i < groups_.size(); i++) {
		if (groups_[i].id == active_group_)
			return i;
	}

	return static_cast<std::size_t>(-1);
}

template<class Item>
void editor_palette<Item>::adjust_size(const SDL_Rect& target)
{
	// The number of columns is passed to the constructor, and isn't changed afterwards. It's likely to be
	// exactly 4, but will always be a small number which makes the next cast reasonable.
	const int items_fitting = static_cast<int>((target.h / item_space_) * columns_);

	// This might be called while the palette is not visible onscreen.
	// If that happens, no items will fit and we'll have a negative number here.
	// Just skip it in that case.
	//
	// New items can be added via the add_item function, so this creates as
	// many buttons as can fit, even if there aren't yet enough items to need
	// that many buttons.
	if(items_fitting > 0) {
		const auto buttons_needed = static_cast<std::size_t>(items_fitting);
		if(buttons_.size() != buttons_needed) {
			buttons_.resize(buttons_needed, gui::tristate_button(this));
		}
	}

	// Update button locations and sizes. Needs to be done even if the number of buttons hasn't changed,
	// because adjust_size() also handles moving left and right when the window's width is changed.
	SDL_Rect dstrect;
	dstrect.w = item_size_ + 2;
	dstrect.h = item_size_ + 2;
	for(std::size_t i = 0; i < buttons_.size(); ++i) {
		dstrect.x = target.x + static_cast<int>(i % columns_) * item_space_;
		dstrect.y = target.y + static_cast<int>(i / columns_) * item_space_;
		buttons_[i].set_location(dstrect);
	}

	set_location(target);
	set_dirty(true);
	gui_.set_help_string(get_help_string());
}

template<class Item>
void editor_palette<Item>::select_fg_item(const std::string& item_id)
{
	if (selected_fg_item_ != item_id) {
		selected_fg_item_ = item_id;
		set_dirty();
	}
	gui_.set_help_string(get_help_string());
}

template<class Item>
void editor_palette<Item>::select_bg_item(const std::string& item_id)
{
	if (selected_bg_item_ != item_id) {
		selected_bg_item_ = item_id;
		set_dirty();
	}
	gui_.set_help_string(get_help_string());
}

template<class Item>
void editor_palette<Item>::swap()
{
	std::swap(selected_fg_item_, selected_bg_item_);
	select_fg_item(selected_fg_item_);
	select_bg_item(selected_bg_item_);
	set_dirty();
}

template<class Item>
std::size_t editor_palette<Item>::num_items()
{
	return group_map_[active_group_].size();
}


template<class Item>
void editor_palette<Item>::hide(bool hidden)
{
	widget::hide(hidden);

	if (!hidden) {
		gui_.set_help_string(get_help_string());
	} else {
		gui_.clear_help_string();
	}

	for (gui::widget& w : buttons_) {
		w.hide(hidden);
	}
}


template<class Item>
bool editor_palette<Item>::is_selected_fg_item(const std::string& id)
{
	return selected_fg_item_ == id;
}

template<class Item>
bool editor_palette<Item>::is_selected_bg_item(const std::string& id)
{
	return selected_bg_item_ == id;
}

template<class Item>
void editor_palette<Item>::layout()
{
	if (!dirty()) {
		return;
	}

	toolkit_.set_mouseover_overlay(gui_);

	std::shared_ptr<gui::button> palette_menu_button = gui_.find_menu_button("menu-editor-terrain");
	if(palette_menu_button) {
		t_string& name = groups_[active_group_index()].name;
		std::string& icon = groups_[active_group_index()].icon;

		palette_menu_button->set_tooltip_string(name);
		palette_menu_button->set_overlay(icon);
	}

	// The hotkey system will automatically enable and disable the buttons when it runs, but it doesn't
	// get triggered when handling mouse-wheel scrolling. Therefore duplicate that functionality here.
	std::shared_ptr<gui::button> upscroll_button = gui_.find_action_button("upscroll-button-editor");
	if(upscroll_button)
		upscroll_button->enable(can_scroll_up());
	std::shared_ptr<gui::button> downscroll_button = gui_.find_action_button("downscroll-button-editor");
	if(downscroll_button)
		downscroll_button->enable(can_scroll_down());

	for(std::size_t i = 0; i < buttons_.size(); ++i) {
		const auto item_index = items_start_ + i;
		gui::tristate_button& tile = buttons_[i];

		tile.hide(true);

		// If we've scrolled to the end of the list, or if there aren't many items, leave the button hidden
		if(item_index >= num_items()) {
			continue;
		}

		const std::string item_id = active_group()[item_index];
		typename item_map::iterator item = item_map_.find(item_id);

		texture item_base, item_overlay;
		std::stringstream tooltip_text;
		setup_item((*item).second, item_base, item_overlay, tooltip_text);
		bool is_core = non_core_items_.find(get_id((*item).second)) == non_core_items_.end();
		if (!is_core) {
			tooltip_text << " "
			<< _("(non-core)") << "\n"
			<< _("Will not work in game without extra care.");
			tile.set_tooltip_string(markup::span_color(font::BAD_COLOR, tooltip_text.str()));
		} else {
			tile.set_tooltip_string(tooltip_text.str());
		}

		tile.set_item_image(item_base, item_overlay);
		tile.set_item_id(item_id);

		if (is_selected_bg_item(get_id(item->second))
				&& is_selected_fg_item(get_id(item->second))) {
			tile.set_pressed(gui::tristate_button::BOTH);
		} else if (is_selected_bg_item(get_id(item->second))) {
			tile.set_pressed(gui::tristate_button::RIGHT);
		} else if (is_selected_fg_item(get_id(item->second))) {
			tile.set_pressed(gui::tristate_button::LEFT);
		} else {
			tile.set_pressed(gui::tristate_button::NONE);
		}

		tile.set_dirty(true);
		tile.hide(false);
	}

	set_dirty(false);
}

template<class Item>
void editor_palette<Item>::draw_contents()
{
	// This is unnecessary as every GUI1 widget is a TLD.
	//for(std::size_t i = 0; i < buttons_.size(); ++i) {
	//	gui::tristate_button& tile = buttons_[i];
	//	tile.draw();
	//}
}

// Force compilation of the following template instantiations
template class editor_palette<t_translation::terrain_code>;
template class editor_palette<const unit_type&>;
template class editor_palette<overlay>;

} // end namespace editor
