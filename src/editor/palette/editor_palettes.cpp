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

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor_palettes.hpp"

#include "foreach.hpp"
#include "gettext.hpp"
#include "marked-up_text.hpp"
#include "tooltips.hpp"

#include "editor/action/mouse/mouse_action.hpp"

namespace editor {

template<class Item>
void editor_palette<Item>::set_group(const std::string& id)
{
	assert(!id.empty());

	bool found = false;
	foreach (const item_group& group, groups_) {
		if (group.id == id)
			found = true;
	}
	assert(found);

	active_group_ = id;

	if(active_group().empty()) {
		ERR_ED << "No items found in group with the id: '" << id << "'.\n";
	}
	gui_.set_terrain_report(active_group_report());
	scroll_top();
}
template void editor_palette<t_translation::t_terrain>::set_group(const std::string& id);
template void editor_palette<unit_type>::set_group(const std::string& id);
template void editor_palette<void*>::set_group(const std::string& id);

template<class Item>
void editor_palette<Item>::set_group(size_t index)
{
	assert(groups_.size() > index);
	set_group(groups_[index].id);
}
template void editor_palette<t_translation::t_terrain>::set_group(size_t index);
template void editor_palette<unit_type>::set_group(size_t index);
template void editor_palette<void*>::set_group(size_t index);

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
template size_t editor_palette<void*>::active_group_index();

template<class Item>
const config editor_palette<Item>::active_group_report()
{
	config cfg;
	config& report = cfg.add_child("element");
	for (size_t i = 0 ; i < groups_.size(); i++) {
		if (groups_[i].id == active_group_) {
			std::string groupname = groups_[i].name;
			report["image"] = "buttons/" + groups_[i].icon + ".png";
			report["tooltip"] = groupname;
		}
	}
	return cfg;
}
template const config editor_palette<t_translation::t_terrain>::active_group_report();
template const config editor_palette<unit_type>::active_group_report();
template const config editor_palette<void*>::active_group_report();

template<class Item>
void editor_palette<Item>::adjust_size()
{
	scroll_top();

	SDL_Rect rect = create_rect(size_specs_.palette_x
			, size_specs_.palette_y
			, size_specs_.palette_w
			, size_specs_.palette_h);

	set_location(rect);
	palette_start_ = size_specs_.palette_y;
	const size_t space_for_items = size_specs_.palette_h;
//TODO
//	rect.y = items_start_;
//	rect.h = space_for_items;
	bg_register(rect);
	const unsigned items_fitting =
		static_cast<unsigned> (space_for_items / item_space_) *
		item_width_;
	nitems_ = std::min<int>(items_fitting, nmax_items_);

	set_dirty();
}
template void editor_palette<t_translation::t_terrain>::adjust_size();
template void editor_palette<unit_type>::adjust_size();
template void editor_palette<void*>::adjust_size();

template<class Item>
void editor_palette<Item>::scroll_down()
{
	//TODO
	SDL_Rect rect = create_rect(size_specs_.palette_x
			, size_specs_.palette_y
			, size_specs_.palette_w
			, size_specs_.palette_h);

	if(items_start_ + nitems_ + item_width_ <= num_items()) {
		items_start_ += item_width_;
		bg_restore(rect);
		set_dirty();
	}
	else if (items_start_ + nitems_ + (num_items() % item_width_) <= num_items()) {
		items_start_ += num_items() % item_width_;
		bg_restore(rect);
		set_dirty();
	}
}
template void editor_palette<t_translation::t_terrain>::scroll_down();
template void editor_palette<unit_type>::scroll_down();
template void editor_palette<void*>::scroll_down();

template<class Item>
void editor_palette<Item>::scroll_up()
{
	//TODO
	SDL_Rect rect = create_rect(size_specs_.palette_x
			, size_specs_.palette_y
			, size_specs_.palette_w
			, size_specs_.palette_h);

	unsigned int decrement = item_width_;
	//TODO here is a bug.
	if (items_start_ + nitems_ == num_items() && num_items() % item_width_ != 0) {
		decrement = num_items() % item_width_;
	}
	if(items_start_ >= decrement) {
		bg_restore(rect);
		set_dirty();
		items_start_ -= decrement;
	}
}
template void editor_palette<t_translation::t_terrain>::scroll_up();
template void editor_palette<unit_type>::scroll_up();
template void editor_palette<void*>::scroll_up();

template<class Item>
void editor_palette<Item>::scroll_top()
{
	//TODO
	SDL_Rect rect = create_rect(size_specs_.palette_x
			, size_specs_.palette_y
			, size_specs_.palette_w
			, size_specs_.palette_h);

	items_start_ = 0;
	bg_restore(rect);
	set_dirty();
}
//TODO
//template void editor_palette<t_translation::t_terrain>::scroll_top();

template<class Item>
void editor_palette<Item>::scroll_bottom()
{
	unsigned int old_start = num_items();
	while (old_start != items_start_) {
		old_start = items_start_;
		scroll_down();
	}
}
template void editor_palette<t_translation::t_terrain>::scroll_bottom();
template void editor_palette<unit_type>::scroll_bottom();
template void editor_palette<void*>::scroll_bottom();

template<class Item>
void editor_palette<Item>::select_fg_item(std::string item_id)
{
	if (selected_fg_item_ != item_id) {
		set_dirty();
		selected_fg_item_ = item_id;
		//TODO
		//update_report();
	}
}
template void editor_palette<t_translation::t_terrain>::select_fg_item(std::string terrain_id);
template void editor_palette<unit_type>::select_fg_item(std::string unit_id);
template void editor_palette<void*>::select_fg_item(std::string unit_id);

template<class Item>
void editor_palette<Item>::select_bg_item(std::string item_id)
{
	if (selected_bg_item_ != item_id) {
		set_dirty();
		selected_bg_item_ = item_id;
		//TODO
		//update_report();
	}
}
template void editor_palette<t_translation::t_terrain>::select_bg_item(std::string terrain_id);
template void editor_palette<unit_type>::select_bg_item(std::string unit_id);
template void editor_palette<void*>::select_bg_item(std::string unit_id);

template<class Item>
void editor_palette<Item>::swap()
{
	std::swap(selected_fg_item_, selected_bg_item_);
	set_dirty();
	//TODO
	//update_report();
}
template void editor_palette<t_translation::t_terrain>::swap();
template void editor_palette<unit_type>::swap();
template void editor_palette<void*>::swap();

template<class Item>
void editor_palette<Item>::left_mouse_click(const int mousex, const int mousey)
{
	int tselect = tile_selected(mousex, mousey);
	if(tselect >= 0 && (items_start_+tselect) < active_group().size()) {
		select_fg_item(active_group()[items_start_+tselect]);
		gui_.invalidate_game_status();
	}
}
//TODO
//template void editor_palette<t_translation::t_terrain>::left_mouse_click(const int mousex, const int mousey);


template<class Item>
void editor_palette<Item>::right_mouse_click(const int mousex, const int mousey)
{
	int tselect = tile_selected(mousex, mousey);
	if(tselect >= 0 && (items_start_+tselect) < active_group().size()) {
		select_bg_item(active_group()[items_start_+tselect]);
		gui_.invalidate_game_status();
	}
}
//TODO
//template void editor_palette<t_translation::t_terrain>::right_mouse_click(const int mousex, const int mousey);

template<class Item>
size_t editor_palette<Item>::num_items()
{
	size_t size = group_map_[active_group_].size();
	return size;
}
//TODO
//template size_t editor_palette<t_translation::t_terrain>::num_items();

template<class Item>
void editor_palette<Item>::handle_event(const SDL_Event& event) {

	if ((**active_mouse_action_).get_palette() != this) {
		return;
	}

	if (event.type == SDL_MOUSEMOTION) {
		// If the mouse is inside the palette, give it focus.
		if (point_in_rect(event.button.x, event.button.y, location())) {
			if (!focus(&event)) {
				set_focus(true);
			}
		}
		// If the mouse is outside, remove focus.
		else {
			if (focus(&event)) {
				set_focus(false);
			}
		}
	}
	if (!focus(&event)) {
		return;
	}
	int mousex, mousey;
	SDL_GetMouseState(&mousex,&mousey);
	const SDL_MouseButtonEvent mouse_button_event = event.button;
	if (mouse_button_event.type == SDL_MOUSEBUTTONDOWN) {
		if (mouse_button_event.button == SDL_BUTTON_LEFT) {
			left_mouse_click(mousex, mousey);
		}
		if (mouse_button_event.button == SDL_BUTTON_RIGHT) {
			right_mouse_click(mousex, mousey);
		}
		if (mouse_button_event.button == SDL_BUTTON_WHEELUP) {
			scroll_up();
		}
		if (mouse_button_event.button == SDL_BUTTON_WHEELDOWN) {
			scroll_down();
		}
		if (mouse_button_event.button == SDL_BUTTON_WHEELLEFT) {
			set_group( (active_group_index() -1) % (groups_.size() -1));
			gui_.set_terrain_report(active_group_report());
		}
		if (mouse_button_event.button == SDL_BUTTON_WHEELRIGHT) {
			set_group( (active_group_index() +1) % (groups_.size() -1));
			gui_.set_terrain_report(active_group_report());
		}
	}
	if (mouse_button_event.type == SDL_MOUSEBUTTONUP) {
		if (mouse_button_event.button == SDL_BUTTON_LEFT) {
		}
	}
}
template void editor_palette<t_translation::t_terrain>::handle_event(const SDL_Event& event);
template void editor_palette<unit_type>::handle_event(const SDL_Event& event);
template void editor_palette<void*>::handle_event(const SDL_Event& event);

template<class Item>
void editor_palette<Item>::draw(bool force)
{
	if ( (!dirty() && !force) || ((**active_mouse_action_).get_palette() != this) ) {
		return;
	}

	unsigned int starting = items_start_;
	unsigned int ending = starting + nitems_;

	if(ending > num_items() ){
		ending = num_items();
	}
	const SDL_Rect &loc = location();
	int y = palette_start_;
	SDL_Rect palrect;
	palrect.x = loc.x;
	palrect.y = palette_start_;
	palrect.w = size_specs_.palette_w;
	palrect.h = size_specs_.palette_h;
	tooltips::clear_tooltips(palrect);
	for(unsigned int counter = starting; counter < ending; counter++){

		const int counter_from_zero = counter - starting;
		SDL_Rect dstrect;
		dstrect.x = loc.x + (counter_from_zero % item_width_) * item_space_;
		dstrect.y = y;
		dstrect.w = item_size_;
		dstrect.h = item_size_;

		// Reset the tile background
		bg_restore(dstrect);

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
	update_rect(loc);
	set_dirty(false);
}
template void editor_palette<t_translation::t_terrain>::draw(bool force);
template void editor_palette<unit_type>::draw(bool force);
template void editor_palette<void*>::draw(bool force);

template<class Item>
int editor_palette<Item>::tile_selected(const int x, const int y) const
{
	for(unsigned int i = 0; i != nitems_; i++) {
		const int px = size_specs_.palette_x + (i % item_width_) * item_space_;
		const int py = palette_start_ + (i / item_width_) * item_space_;
		const int pw = item_space_;
		const int ph = item_space_;

		if(x > px && x < px + pw && y > py && y < py + ph) {
			return i;
		}
	}
	return -1;
}


} // end namespace editor
