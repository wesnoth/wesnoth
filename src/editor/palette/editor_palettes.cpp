/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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

#include "config_assign.hpp"
#include "gettext.hpp"
#include "font/text_formatting.hpp"
#include "tooltips.hpp"
#include "overlay.hpp"
#include "filesystem.hpp"
#include "units/types.hpp"

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
template sdl_handler_vector editor_palette<t_translation::terrain_code>::handler_members();
template sdl_handler_vector editor_palette<unit_type>::handler_members();
template sdl_handler_vector editor_palette<overlay>::handler_members();

template<class Item>
void editor_palette<Item>::expand_palette_groups_menu(std::vector<config>& items)
{
	for (unsigned int i = 0; i < items.size(); ++i) {
		if (items[i]["id"] == "editor-palette-groups") {
			items.erase(items.begin() + i);

			std::vector<config> groups;
			const std::vector<item_group>& item_groups = get_groups();

			for (size_t mci = 0; mci < item_groups.size(); ++mci) {
				std::string groupname = item_groups[mci].name;
				if (groupname.empty()) {
					groupname = _("(Unknown Group)");
				}
				std::stringstream i_ss;
				i_ss << item_groups[mci].icon;
				if (mci == active_group_index()) {

					if (filesystem::file_exists(i_ss.str() + "_30-pressed.png" ) ) {
						i_ss << "_30-pressed.png";
					} else {
						i_ss << "_30.png~CS(70,70,0)";
					}

				} else {
					i_ss << "_30.png";
				}

				groups.emplace_back(config_of
					("label", groupname)
					("icon", i_ss.str())
				);
			}

			items.insert(items.begin() + i, groups.begin(), groups.end());
			break;
		}
	}
}
template void editor_palette<t_translation::terrain_code>::expand_palette_groups_menu(std::vector<config>& items);
template void editor_palette<unit_type>::expand_palette_groups_menu(std::vector<config>& items);
template void editor_palette<overlay>::expand_palette_groups_menu(std::vector<config>& items);

template<class Item>
bool editor_palette<Item>::scroll_up()
{
	int decrement = item_width_;
	if (items_start_ + num_visible_items() == num_items() && num_items() % item_width_ != 0) {
		decrement = num_items() % item_width_;
	}
	if(items_start_ >= decrement) {
		items_start_ -= decrement;
		draw();
		return true;
	}
	return false;
}
template bool editor_palette<t_translation::terrain_code>::scroll_up();
template bool editor_palette<unit_type>::scroll_up();
template bool editor_palette<overlay>::scroll_up();

template<class Item>
void editor_palette<Item>::expand_palette_groups_menu(std::vector< std::pair< std::string, std::string> >& items)
{
	const std::vector<item_group>& item_groups = get_groups();

	for (size_t mci = 0; mci < item_groups.size(); ++mci) {
		std::string groupname = item_groups[mci].name;
		if (groupname.empty()) {
			groupname = _("(Unknown Group)");
		}
		const std::string& img = item_groups[mci].icon;
		items.push_back(std::pair<std::string, std::string>( img, groupname));
	}
}
template void editor_palette<t_translation::terrain_code>::expand_palette_groups_menu(std::vector< std::pair< std::string, std::string> >& items);
template void editor_palette<unit_type>::expand_palette_groups_menu(std::vector< std::pair< std::string, std::string> >& items);
template void editor_palette<overlay>::expand_palette_groups_menu(std::vector< std::pair< std::string, std::string> >& items);

template<class Item>
bool editor_palette<Item>::can_scroll_up()
{
	return (items_start_ != 0);
}
template bool editor_palette<t_translation::terrain_code>::can_scroll_up();
template bool editor_palette<unit_type>::can_scroll_up();
template bool editor_palette<overlay>::can_scroll_up();

template<class Item>
bool editor_palette<Item>::can_scroll_down()
{
	return (items_start_ + nitems_ + item_width_ <= num_items());
}
template bool editor_palette<t_translation::terrain_code>::can_scroll_down();
template bool editor_palette<unit_type>::can_scroll_down();
template bool editor_palette<overlay>::can_scroll_down();

template<class Item>
bool editor_palette<Item>::scroll_down()
{
	bool end_reached = (!(items_start_ + nitems_ + item_width_ <= num_items()));
	bool scrolled = false;

	// move downwards
	if(!end_reached) {
		items_start_ += item_width_;
		scrolled = true;
	}
	else if (items_start_ + nitems_ + (num_items() % item_width_) <= num_items()) {
		items_start_ += num_items() % item_width_;
		scrolled = true;
	}
	set_dirty(scrolled);
	draw();
	return scrolled;
}
template bool editor_palette<t_translation::terrain_code>::scroll_down();
template bool editor_palette<unit_type>::scroll_down();
template bool editor_palette<overlay>::scroll_down();

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
				//palette_menu_button->set_label(group.name);
				palette_menu_button->set_tooltip_string(group.name);
				palette_menu_button->set_overlay(group.icon);
			}
		}
	}
	assert(found);

	active_group_ = id;

	if(active_group().empty()) {
		ERR_ED << "No items found in group with the id: '" << id << "'." << std::endl;
	}
}
template void editor_palette<t_translation::terrain_code>::set_group(const std::string& id);
template void editor_palette<unit_type>::set_group(const std::string& id);
template void editor_palette<overlay>::set_group(const std::string& id);

template<class Item>
void editor_palette<Item>::set_group(size_t index)
{
	assert(groups_.size() > index);
	set_group(groups_[index].id);
}
template void editor_palette<t_translation::terrain_code>::set_group(size_t index);
template void editor_palette<unit_type>::set_group(size_t index);
template void editor_palette<overlay>::set_group(size_t index);

template<class Item>
size_t editor_palette<Item>::active_group_index()
{
	assert(!active_group_.empty());

	for (size_t i = 0 ; i < groups_.size(); i++) {
		if (groups_[i].id == active_group_)
			return i;
	}

	return static_cast<size_t>(-1);
}
template size_t editor_palette<t_translation::terrain_code>::active_group_index();
template size_t editor_palette<unit_type>::active_group_index();
template size_t editor_palette<overlay>::active_group_index();

template<class Item>
void editor_palette<Item>::adjust_size(const SDL_Rect& target)
{
	palette_x_ = target.x;
	palette_y_ = target.y;
	const int space_for_items = target.h;
	const int items_fitting = (space_for_items / item_space_) * item_width_;
	nitems_ = std::min(items_fitting, nmax_items_);
	if (num_visible_items() != nitems_) {
		buttons_.resize(nitems_, gui::tristate_button(gui_.video(), this));
	}
	set_location(target);
	set_dirty(true);
	gui_.video().clear_help_string(help_handle_);
	help_handle_ = gui_.video().set_help_string(get_help_string());
}
template void editor_palette<t_translation::terrain_code>::adjust_size(const SDL_Rect& target);
template void editor_palette<unit_type>::adjust_size(const SDL_Rect& target);
template void editor_palette<overlay>::adjust_size(const SDL_Rect& target);

template<class Item>
void editor_palette<Item>::select_fg_item(const std::string& item_id)
{
	if (selected_fg_item_ != item_id) {
		selected_fg_item_ = item_id;
		set_dirty();
	}
	gui_.video().clear_help_string(help_handle_);
	help_handle_ = gui_.video().set_help_string(get_help_string());
}
template void editor_palette<t_translation::terrain_code>::select_fg_item(const std::string& terrain_id);
template void editor_palette<unit_type>::select_fg_item(const std::string& unit_id);
template void editor_palette<overlay>::select_fg_item(const std::string& unit_id);

template<class Item>
void editor_palette<Item>::select_bg_item(const std::string& item_id)
{
	if (selected_bg_item_ != item_id) {
		selected_bg_item_ = item_id;
		set_dirty();
	}
	gui_.video().clear_help_string(help_handle_);
	help_handle_ = gui_.video().set_help_string(get_help_string());
}
template void editor_palette<t_translation::terrain_code>::select_bg_item(const std::string& terrain_id);
template void editor_palette<unit_type>::select_bg_item(const std::string& unit_id);
template void editor_palette<overlay>::select_bg_item(const std::string& unit_id);

template<class Item>
void editor_palette<Item>::swap()
{
	std::swap(selected_fg_item_, selected_bg_item_);
	select_fg_item(selected_fg_item_);
	select_bg_item(selected_bg_item_);
	set_dirty();
}
template void editor_palette<t_translation::terrain_code>::swap();
template void editor_palette<unit_type>::swap();
template void editor_palette<overlay>::swap();

template<class Item>
int editor_palette<Item>::num_items()
{
	return group_map_[active_group_].size();
}
template int editor_palette<t_translation::terrain_code>::num_items();
template int editor_palette<unit_type>::num_items();
template int editor_palette<overlay>::num_items();

template<class Item>
bool editor_palette<Item>::is_selected_fg_item(const std::string& id)
{
	return selected_fg_item_ == id;
}
template bool editor_palette<t_translation::terrain_code>::is_selected_fg_item(const std::string& id);
template bool editor_palette<unit_type>::is_selected_fg_item(const std::string& id);
template bool editor_palette<overlay>::is_selected_fg_item(const std::string& id);

template<class Item>
bool editor_palette<Item>::is_selected_bg_item(const std::string& id)
{
	return selected_bg_item_ == id;
}
template bool editor_palette<t_translation::terrain_code>::is_selected_bg_item(const std::string& id);
template bool editor_palette<unit_type>::is_selected_bg_item(const std::string& id);
template bool editor_palette<overlay>::is_selected_bg_item(const std::string& id);

template<class Item>
void editor_palette<Item>::draw_contents()
{
	toolkit_.set_mouseover_overlay(gui_);

	std::shared_ptr<gui::button> palette_menu_button = gui_.find_menu_button("menu-editor-terrain");
	if (palette_menu_button) {

		t_string& name = groups_[active_group_index()].name;
		std::string& icon = groups_[active_group_index()].icon;

		palette_menu_button->set_tooltip_string(name);
		palette_menu_button->set_overlay(icon);
	}

	unsigned int y = palette_y_;
	unsigned int x = palette_x_;
	int starting = items_start_;
	int ending = std::min<int>(starting + nitems_, num_items());

	std::shared_ptr<gui::button> upscroll_button = gui_.find_action_button("upscroll-button-editor");
	if (upscroll_button)
		upscroll_button->enable(starting != 0);
	std::shared_ptr<gui::button> downscroll_button = gui_.find_action_button("downscroll-button-editor");
	if (downscroll_button)
		downscroll_button->enable(ending != num_items());


	int counter = starting;
	for (int i = 0, size = num_visible_items(); i < size ; ++i) {
		//TODO check if the conditions still hold for the counter variable
		//for (unsigned int counter = starting; counter < ending; counter++)

		gui::tristate_button& tile = buttons_[i];

		tile.hide(true);

		if (i >= ending) continue;

		const std::string item_id = active_group()[counter];
		//typedef std::map<std::string, Item> item_map_wurscht;
		typename item_map::iterator item = item_map_.find(item_id);

		surface item_image(nullptr);
		std::stringstream tooltip_text;
		draw_item((*item).second, item_image, tooltip_text);

		bool is_core = non_core_items_.find(get_id((*item).second)) == non_core_items_.end();
		if (!is_core) {
			tooltip_text << " "
					<< font::span_color(font::BAD_COLOR)
			<< _("(non-core)") << "\n"
			<< _("Will not work in game without extra care.")
			<< "</span>";
		}

		const int counter_from_zero = counter - starting;
		SDL_Rect dstrect;
		dstrect.x = x + (counter_from_zero % item_width_) * item_space_;
		dstrect.y = y;
		dstrect.w = item_size_ + 2;
		dstrect.h = item_size_ + 2;

		tile.set_location(dstrect);
		tile.set_tooltip_string(tooltip_text.str());
		tile.set_item_image(item_image);
		tile.set_item_id(item_id);

//		if (get_id((*item).second) == selected_bg_item_
//				&& get_id((*item).second) == selected_fg_item_) {
//			tile.set_pressed(gui::tristate_button::BOTH);
//		} else if (get_id((*item).second) == selected_bg_item_) {
//			tile.set_pressed(gui::tristate_button::RIGHT);
//		} else if (get_id((*item).second) == selected_fg_item_) {
//			tile.set_pressed(gui::tristate_button::LEFT);
//		} else {
//			tile.set_pressed(gui::tristate_button::NONE);
//		}

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
		tile.draw();

		// Adjust location
		if (counter_from_zero % item_width_ == item_width_ - 1)
			y += item_space_;
		counter++;
	}
}
template void editor_palette<t_translation::terrain_code>::draw_contents();
template void editor_palette<unit_type>::draw_contents();
template void editor_palette<overlay>::draw_contents();


} // end namespace editor
