/*
   Copyright (C) 2003 - 2013 by David White <dave@whitevine.net>
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

#include "editor_palettes.hpp"

#include "gettext.hpp"
#include "marked-up_text.hpp"
#include "tooltips.hpp"

#include "editor/action/mouse/mouse_action.hpp"

#include "wml_separators.hpp"

#include <boost/foreach.hpp>

namespace editor {

template<class Item>
void editor_palette<Item>::expand_palette_groups_menu(std::vector<std::string>& items)
{
	for (unsigned int i = 0; i < items.size(); ++i) {
		if (items[i] == "editor-palette-groups") {
			items.erase(items.begin() + i);

			std::vector<std::string> groups;
			const std::vector<item_group>& item_groups = get_groups();

			for (size_t mci = 0; mci < item_groups.size(); ++mci) {
				std::string groupname = item_groups[mci].name;
				if (groupname.empty()) {
					groupname = _("(Unknown Group)");
				}
				std::string img = item_groups[mci].icon;
				std::stringstream str;
				//TODO
				//std::string postfix = ".png"; //(toolkit_->active_group_index() == mci) ? "-pressed.png" : ".png";
				//str << IMAGE_PREFIX << "buttons/" << img << postfix << COLUMN_SEPARATOR << groupname;
				str << IMAGE_PREFIX << img << COLUMN_SEPARATOR << groupname;
				groups.push_back(str.str());
			}
			items.insert(items.begin() + i, groups.begin(), groups.end());
			break;
		}
	}
}
template void editor_palette<t_translation::t_terrain>::expand_palette_groups_menu(std::vector<std::string>& items);
template void editor_palette<unit_type>::expand_palette_groups_menu(std::vector<std::string>& items);

template<class Item>
bool editor_palette<Item>::left_mouse_click(const int mousex, const int mousey)
{
	int tselect = tile_selected(mousex, mousey);
	if(tselect >= 0 && (items_start_+tselect) < active_group().size()) {
		select_fg_item(active_group()[items_start_+tselect]);
		return true;
	}
	return false;
}
template bool editor_palette<t_translation::t_terrain>::left_mouse_click(const int mousex, const int mousey);
template bool editor_palette<unit_type>::left_mouse_click(const int mousex, const int mousey);

template<class Item>
bool editor_palette<Item>::right_mouse_click(const int mousex, const int mousey)
{
	int tselect = tile_selected(mousex, mousey);
	if(tselect >= 0 && (items_start_+tselect) < active_group().size()) {
		select_bg_item(active_group()[items_start_+tselect]);
		return true;
	}
	return false;
}
template bool editor_palette<t_translation::t_terrain>::right_mouse_click(const int mousex, const int mousey);
template bool editor_palette<unit_type>::right_mouse_click(const int mousex, const int mousey);

template<class Item>
bool editor_palette<Item>::scroll_up()
{
	unsigned int decrement = item_width_;
	if (items_start_ + nitems_ == num_items() && num_items() % item_width_ != 0) {
		decrement = num_items() % item_width_;
	}
	if(items_start_ >= decrement) {
		items_start_ -= decrement;
		return true;
	}
	return false;
}
template bool editor_palette<t_translation::t_terrain>::scroll_up();
template bool editor_palette<unit_type>::scroll_up();

template<class Item>
bool editor_palette<Item>::can_scroll_up()
{
	return (items_start_ != 0);
}
template bool editor_palette<t_translation::t_terrain>::can_scroll_up();
template bool editor_palette<unit_type>::can_scroll_up();

template<class Item>
bool editor_palette<Item>::can_scroll_down()
{
	return (items_start_ + nitems_ + item_width_ <= num_items());
}
template bool editor_palette<t_translation::t_terrain>::can_scroll_down();
template bool editor_palette<unit_type>::can_scroll_down();

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
	draw(scrolled);
	return scrolled;
}
template bool editor_palette<t_translation::t_terrain>::scroll_down();
template bool editor_palette<unit_type>::scroll_down();

template<class Item>
void editor_palette<Item>::set_group(const std::string& id)
{
	assert(!id.empty());

	bool found = false;
	BOOST_FOREACH(const item_group& group, groups_) {
		if (group.id == id)
			found = true;
	}
	assert(found);

	active_group_ = id;

	if(active_group().empty()) {
		ERR_ED << "No items found in group with the id: '" << id << "'.\n";
	}
	gui_.set_palette_report(active_group_report());
}
template void editor_palette<t_translation::t_terrain>::set_group(const std::string& id);
template void editor_palette<unit_type>::set_group(const std::string& id);

template<class Item>
void editor_palette<Item>::set_group(size_t index)
{
	assert(groups_.size() > index);
	set_group(groups_[index].id);
}
template void editor_palette<t_translation::t_terrain>::set_group(size_t index);
template void editor_palette<unit_type>::set_group(size_t index);

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
template size_t editor_palette<t_translation::t_terrain>::active_group_index();
template size_t editor_palette<unit_type>::active_group_index();

template<class Item>
const config editor_palette<Item>::active_group_report()
{
	config cfg;
	config& report = cfg.add_child("element");
	for (size_t i = 0 ; i < groups_.size(); i++) {
		if (groups_[i].id == active_group_) {
			std::string groupname = groups_[i].name;
			report["image"] = groups_[i].icon;
			report["tooltip"] = groupname;
		}
	}
	return cfg;
}
template const config editor_palette<t_translation::t_terrain>::active_group_report();
template const config editor_palette<unit_type>::active_group_report();

template<class Item>
void editor_palette<Item>::adjust_size(const SDL_Rect& target)
{
	palette_x_ = target.x;
	palette_y_ = target.y;
	const size_t space_for_items = target.h;
	const unsigned items_fitting =
		static_cast<unsigned> (space_for_items / item_space_) *
		item_width_;
	nitems_ = std::min<int>(items_fitting, nmax_items_);
}
template void editor_palette<t_translation::t_terrain>::adjust_size(const SDL_Rect& target);
template void editor_palette<unit_type>::adjust_size(const SDL_Rect& target);

template<class Item>
void editor_palette<Item>::select_fg_item(const std::string& item_id)
{
	if (selected_fg_item_ != item_id) {
		selected_fg_item_ = item_id;
	}
}
template void editor_palette<t_translation::t_terrain>::select_fg_item(const std::string& terrain_id);
template void editor_palette<unit_type>::select_fg_item(const std::string& unit_id);

template<class Item>
void editor_palette<Item>::select_bg_item(const std::string& item_id)
{
	if (selected_bg_item_ != item_id) {
		selected_bg_item_ = item_id;
	}
}
template void editor_palette<t_translation::t_terrain>::select_bg_item(const std::string& terrain_id);
template void editor_palette<unit_type>::select_bg_item(const std::string& unit_id);

template<class Item>
void editor_palette<Item>::swap()
{
	std::swap(selected_fg_item_, selected_bg_item_);
}
template void editor_palette<t_translation::t_terrain>::swap();
template void editor_palette<unit_type>::swap();

template<class Item>
size_t editor_palette<Item>::num_items()
{
	const size_t size = group_map_[active_group_].size();
	return size;
}
template size_t editor_palette<t_translation::t_terrain>::num_items();
template size_t editor_palette<unit_type>::num_items();

template<class Item>
void editor_palette<Item>::draw(bool dirty)
{
	if (!dirty) return;
	unsigned int y = palette_y_;
	unsigned int x = palette_x_;
	unsigned int starting = items_start_;
	unsigned int ending = starting + nitems_;
	if(ending > num_items() ){
		ending = num_items();
	}

	gui::button* upscroll_button = gui_.find_button("upscroll-button-editor");
	upscroll_button->enable(starting != 0);
	gui::button* downscroll_button = gui_.find_button("downscroll-button-editor");
	downscroll_button->enable(ending != num_items());

	for(unsigned int counter = starting; counter < ending; counter++){

		const int counter_from_zero = counter - starting;
		SDL_Rect dstrect;
		dstrect.x = x + (counter_from_zero % item_width_) * item_space_;
		dstrect.y = y;
		dstrect.w = item_size_;
		dstrect.h = item_size_;

		std::stringstream tooltip_text;

		const std::string item_id = active_group()[counter];

		typedef std::map<std::string, Item> item_map_wurscht;

		typename item_map_wurscht::iterator item = item_map_.find(item_id);

		draw_item(dstrect, (*item).second, tooltip_text);

		surface screen = gui_.video().getSurface();
		Uint32 color;

		if (get_id((*item).second) == selected_bg_item_
				&& get_id((*item).second) == selected_fg_item_) {
			color = SDL_MapRGB(screen->format,0xFF,0x00,0xFF);
		}
		else if (get_id((*item).second) == selected_bg_item_) {
			color = SDL_MapRGB(screen->format,0x00,0x00,0xFF);
		}
		else if (get_id((*item).second) == selected_fg_item_) {
			color = SDL_MapRGB(screen->format,0xFF,0x00,0x00);
		}
		else {
			color = SDL_MapRGB(screen->format,0x00,0x00,0x00);
		}

		draw_rectangle(dstrect.x, dstrect.y, dstrect.w, dstrect.h, color, screen);
		/* TODO The call above overdraws the border of the terrain image.
			   The following call is doing better but causing other drawing glitches.
			   draw_rectangle(dstrect.x -1, dstrect.y -1, image->w +2, image->h +2, color, screen);
		 */

		bool is_core = non_core_items_.find(get_id((*item).second)) == non_core_items_.end();
		if (!is_core) {
			tooltip_text << " "
					<< font::span_color(font::BAD_COLOR)
			<< _("(non-core)") << "\n"
			<< _("Will not work in game without extra care.")
			<< "</span>";
		}
		tooltips::add_tooltip(dstrect, tooltip_text.str());
		if (counter_from_zero % item_width_ == item_width_ - 1)
			y += item_space_;
	}
}
template void editor_palette<t_translation::t_terrain>::draw(bool);
template void editor_palette<unit_type>::draw(bool);

template<class Item>
int editor_palette<Item>::tile_selected(const int x, const int y) const
{
	for(unsigned int i = 0; i != nitems_; i++) {
		const int px = palette_x_ + (i % item_width_) * item_space_;
		const int py = palette_y_ + (i / item_width_) * item_space_;
		const int pw = item_space_;
		const int ph = item_space_;

		if(x > px && x < px + pw && y > py && y < py + ph) {
			return i;
		}
	}
	return -1;
}


} // end namespace editor
