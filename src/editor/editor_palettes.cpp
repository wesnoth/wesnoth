/* $Id$ */
/*
  Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
  Part of the Battle for Wesnoth Project http://www.wesnoth.org/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2
  or at your option any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the COPYING file for more details.
*/

//! @file editor/editor_palettes.cpp
//! Manage the terrain-palette in the editor.

#include "SDL.h"
#include "SDL_keysym.h"

#include "editor_palettes.hpp"
#include "editor_layout.hpp"
#include "../config.hpp"
#include "../sdl_utils.hpp"
#include "../serialization/string_utils.hpp"
#include "../image.hpp"
#include "../reports.hpp"
#include "../gettext.hpp"
#include "../tooltips.hpp"
#include "../util.hpp"
#include "../video.hpp"

#include <cassert>

namespace map_editor {

bool is_invalid_terrain(t_translation::t_terrain c) {
	return (c == t_translation::VOID_TERRAIN || c == t_translation::FOGGED);
}

terrain_group::terrain_group(const config& cfg, display& gui):
	id(cfg["id"]), name(cfg["name"]),
	button(gui.video(), "", gui::button::TYPE_CHECK, cfg["icon"])
{
}

terrain_palette::terrain_palette(display &gui, const size_specs &sizes,
								 const gamemap &map, const config& cfg)
	: gui::widget(gui.video()), size_specs_(sizes), gui_(gui), tstart_(0),
	  checked_group_btn_(0), map_(map),
	  top_button_(gui.video(), "", gui::button::TYPE_PRESS, "uparrow-button"),
	  bot_button_(gui.video(), "", gui::button::TYPE_PRESS, "downarrow-button")
{

	// Get the available terrains temporary in terrains_
	terrains_ = map_.get_terrain_list();
	terrains_.erase(std::remove_if(terrains_.begin(), terrains_.end(), is_invalid_terrain),
					terrains_.end());

	// Get the available groups and add them to the structure
    const config::child_list& groups = cfg.get_children("editor_group");
	config::child_list::const_iterator g_itor = groups.begin();
	for(; g_itor != groups.end(); ++ g_itor) {
		terrain_groups_.push_back(terrain_group(**g_itor, gui));

		// By default the 'all'-button is pressed
		if(terrain_groups_.back().id == "all") {
			terrain_groups_.back().button.set_check(true);
			checked_group_btn_ = &terrain_groups_.back().button;
		}
	}
	// The rest of the code assumes this is a valid pointer
	assert(checked_group_btn_ != 0);

	// Add the groups for all terrains to the map
	t_translation::t_list::const_iterator t_itor = terrains_.begin();
	for(; t_itor != terrains_.end(); ++t_itor) {
		// Add the terrain to the requested groups
		const std::vector<std::string>& key =
			utils::split(map_.get_terrain_info(*t_itor).editor_group());

		for(std::vector<std::string>::const_iterator k_itor = key.begin();
				k_itor != key.end(); ++k_itor)
		 {
			terrain_map_[*k_itor].push_back(*t_itor);
		}

		// Add the terrain to the default group
		terrain_map_["all"].push_back(*t_itor);
	}

	// Set the default group
	terrains_ = terrain_map_["all"];

	if(terrains_.empty()) {
		std::cerr << "No terrain found.\n";
	}
	else {
		selected_fg_terrain_ = terrains_[0];
		selected_bg_terrain_ = terrains_[0];
	}
	update_report();
	//adjust_size();
}

void terrain_palette::adjust_size() {

	scroll_top();
	const size_t button_height = 24;
	const size_t button_palette_padding = 8;

	// Values for the group buttons fully hardcoded for now
	//! @todo will be fixed later
	const size_t group_button_height   = 24;
	const size_t group_button_padding  =  2;
	const size_t group_buttons_per_row =  5;

	// Determine number of theme button rows
	size_t group_rows = terrain_groups_.size() / group_buttons_per_row;
	if(terrain_groups_.size() % group_buttons_per_row != 0) {
		++group_rows;
	}
	const size_t group_height = group_rows * (group_button_height + group_button_padding);

	SDL_Rect rect = { size_specs_.palette_x, size_specs_.palette_y, size_specs_.palette_w, size_specs_.palette_h };
	set_location(rect);
	top_button_y_ = size_specs_.palette_y + group_height ;
	button_x_ = size_specs_.palette_x + size_specs_.palette_w/2 - button_height/2;
	terrain_start_ = top_button_y_ + button_height + button_palette_padding;
	const size_t space_for_terrains = size_specs_.palette_h - (button_height + button_palette_padding) * 2 - group_height;
	rect.y = terrain_start_;
	rect.h = space_for_terrains;
	bg_register(rect);
	const unsigned terrains_fitting =
		static_cast<unsigned> (space_for_terrains / size_specs_.terrain_space) *
		size_specs_.terrain_width;
	const unsigned total_terrains = num_terrains();
	nterrains_ = minimum<int>(terrains_fitting, total_terrains);
	bot_button_y_ = size_specs_.palette_y + (nterrains_ / size_specs_.terrain_width) * size_specs_.terrain_space + \
		button_palette_padding * size_specs_.terrain_width + button_height + group_height;
	top_button_.set_location(button_x_, top_button_y_);
	bot_button_.set_location(button_x_, bot_button_y_);

	size_t top = size_specs_.palette_y;
	size_t left = size_specs_.palette_x - 8;
	for(size_t i = 0; i < terrain_groups_.size(); ++i) {
		terrain_groups_[i].button.set_location(left, top);
		if(i % group_buttons_per_row == (group_buttons_per_row - 1)) {
			left = size_specs_.palette_x - 8;
			top += group_button_height + group_button_padding;
		} else {
			left += group_button_height + group_button_padding;
		}
	}

	set_dirty();
}

void terrain_palette::set_dirty(bool dirty) {
	widget::set_dirty(dirty);
	if (dirty) {
		top_button_.set_dirty();
		bot_button_.set_dirty();
		for(size_t i = 0; i < terrain_groups_.size(); ++i) {
			terrain_groups_[i].button.set_dirty();
		}
	}
}

void terrain_palette::scroll_down() {
	if(tstart_ + nterrains_ + size_specs_.terrain_width <= num_terrains()) {
		tstart_ += size_specs_.terrain_width;
		bg_restore();
		set_dirty();
	}
	else if (tstart_ + nterrains_ + (num_terrains() % size_specs_.terrain_width) <= num_terrains()) {
		tstart_ += num_terrains() % size_specs_.terrain_width;
		bg_restore();
		set_dirty();
	}
}

void terrain_palette::scroll_up() {
	unsigned int decrement = size_specs_.terrain_width;
	if (tstart_ + nterrains_ == num_terrains() && num_terrains() % size_specs_.terrain_width != 0) {
		decrement = num_terrains() % size_specs_.terrain_width;
	}
	if(tstart_ >= decrement) {
		bg_restore();
		set_dirty();
		tstart_ -= decrement;
	}
}

void terrain_palette::scroll_top() {
	tstart_ = 0;
	bg_restore();
	set_dirty();
}

void terrain_palette::scroll_bottom() {
	unsigned int old_start = num_terrains();
	while (old_start != tstart_) {
		old_start = tstart_;
		scroll_down();
	}
}

void terrain_palette::set_group(const std::string& id)
{
	terrains_ = terrain_map_[id];
	if(terrains_.empty()) {
		std::cerr << "No terrain found.\n";
	}
	scroll_top();
}

t_translation::t_terrain terrain_palette::selected_fg_terrain() const
{
	return selected_fg_terrain_;
}

t_translation::t_terrain terrain_palette::selected_bg_terrain() const
{
	return selected_bg_terrain_;
}

void terrain_palette::select_fg_terrain(t_translation::t_terrain terrain)
{
	if (selected_fg_terrain_ != terrain) {
		set_dirty();
		selected_fg_terrain_ = terrain;
		update_report();
	}
}

void terrain_palette::select_bg_terrain(t_translation::t_terrain terrain)
{
	if (selected_bg_terrain_ != terrain) {
		set_dirty();
		selected_bg_terrain_ = terrain;
		update_report();
	}
}

/**
 * After the language is changed, the selected terrains needs an update.
 */
void terrain_palette::update_selected_terrains(void)
{
	set_dirty();
	update_report();
}

std::string terrain_palette::get_terrain_string(const t_translation::t_terrain t)
{
	std::stringstream str;
	const std::string& name = map_.get_terrain_info(t).name();
	const t_translation::t_list& underlying = map_.underlying_union_terrain(t);
	str << name;
	if(underlying.size() != 1 || underlying[0] != t) {
		str << " (";
		for(t_translation::t_list::const_iterator i = underlying.begin();
			i != underlying.end(); ++i) {

			str << map_.get_terrain_info(*i).name();
			if(i+1 != underlying.end()) {
				str << ",";
			}
		}
		str << ")";
	}
	return str.str();
}

void terrain_palette::left_mouse_click(const int mousex, const int mousey) {
	int tselect = tile_selected(mousex, mousey);
	if(tselect >= 0) {
		select_fg_terrain(terrains_[tstart_+tselect]);
		gui_.invalidate_game_status();
	}
}

void terrain_palette::right_mouse_click(const int mousex, const int mousey) {
	int tselect = tile_selected(mousex, mousey);
	if(tselect >= 0) {
		select_bg_terrain(terrains_[tstart_+tselect]);
		gui_.invalidate_game_status();
	}
}

size_t terrain_palette::num_terrains() const {
	return terrains_.size();
}

void terrain_palette::draw() {
	draw(false);
}

void terrain_palette::handle_event(const SDL_Event& event) {
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
	}
	if (mouse_button_event.type == SDL_MOUSEBUTTONUP) {
		if (mouse_button_event.button == SDL_BUTTON_LEFT) {
		}
	}
}

void terrain_palette::draw(bool force) {

	if (top_button_.pressed()) {
		scroll_up();
	}
	if (bot_button_.pressed()) {
		scroll_down();
	}
	for(size_t i = 0; i < terrain_groups_.size(); ++i) {
		if(terrain_groups_[i].button.pressed()) {
			if(&terrain_groups_[i].button == checked_group_btn_) {
				checked_group_btn_->set_check(true);
			} else {
				checked_group_btn_->set_check(false);
				checked_group_btn_ = &terrain_groups_[i].button;
				set_group(terrain_groups_[i].id);
			}
			break;
		}
	}
	if (!dirty() && !force) {
		return;
	}
	unsigned int starting = tstart_;
	unsigned int ending = starting + nterrains_;
	SDL_Surface* const screen = gui_.video().getSurface();
	if(ending > num_terrains()){
		ending = num_terrains();
	}
	const SDL_Rect &loc = location();
	int y = terrain_start_;
	for(unsigned int counter = starting; counter < ending; counter++){
		const t_translation::t_terrain terrain = terrains_[counter];
		const std::string filename = "terrain/" + map_.get_terrain_info(terrain).editor_image() + ".png";
		surface image(image::get_image(filename));
		if(image == NULL) {
			std::cerr << "image for terrain " << counter << ": '" << filename << "' not found\n";
			return;
		}

		if(static_cast<unsigned>(image->w) != size_specs_.terrain_size ||
			static_cast<unsigned>(image->h) != size_specs_.terrain_size) {

			image.assign(scale_surface(image,
				size_specs_.terrain_size, size_specs_.terrain_size));
		}

		const int counter_from_zero = counter - starting;
		SDL_Rect dstrect;
		dstrect.x = loc.x + (counter_from_zero % size_specs_.terrain_width) * size_specs_.terrain_space;
		dstrect.y = y;
		dstrect.w = image->w;
		dstrect.h = image->h;

		SDL_BlitSurface(image, NULL, screen, &dstrect);
		SDL_Surface* const screen = gui_.video().getSurface();
		Uint32 color;
		if (terrain == selected_bg_terrain() && terrain == selected_fg_terrain()) {
			color = SDL_MapRGB(screen->format,0xFF,0x00,0xFF);
		}
		else if (terrain == selected_bg_terrain()) {
			color = SDL_MapRGB(screen->format,0x00,0x00,0xFF);
		}
		else if (terrain == selected_fg_terrain()) {
			color = SDL_MapRGB(screen->format,0xFF,0x00,0x00);
		}
		else {
			color = SDL_MapRGB(screen->format,0x00,0x00,0x00);
		}
		draw_rectangle(dstrect.x, dstrect.y, image->w, image->h, color, screen);
		if (counter_from_zero % size_specs_.terrain_width == size_specs_.terrain_width - 1)
			y += size_specs_.terrain_space;
	}
	update_rect(loc);
	set_dirty(false);
}

int terrain_palette::tile_selected(const int x, const int y) const {
	for(unsigned int i = 0; i != nterrains_; i++) {
		const int px = size_specs_.palette_x + (i % size_specs_.terrain_width) * size_specs_.terrain_space;
		const int py = terrain_start_ + (i / size_specs_.terrain_width) * size_specs_.terrain_space;
		const int pw = size_specs_.terrain_space;
		const int ph = size_specs_.terrain_space;

		if(x > px && x < px + pw && y > py && y < py + ph) {
			return i;
		}
	}
	return -1;
}

void terrain_palette::update_report() {
	const std::string msg = std::string(_("FG")) + ": "
		+ get_terrain_string(selected_fg_terrain()) + "\n"
		+ std::string(_("BG")) +
		": " + get_terrain_string(selected_bg_terrain());
	gui_.set_report_content(reports::SELECTED_TERRAIN, msg);
}

void terrain_palette::load_tooltips()
{
	for(size_t i = 0; i < terrain_groups_.size(); ++i) {
		const std::string& text = terrain_groups_[i].name;
		if(text !="") {
			const SDL_Rect tooltip_rect = terrain_groups_[i].button.location();
			tooltips::add_tooltip(tooltip_rect, text);
		}
	}
}

// void terrain_palette::bg_backup() {
//	restorer_ = surface_restorer(&gui_.video(), get_rect());
// }

// void terrain_palette::bg_restore() {
//	restorer_.restore();
// }

brush_bar::brush_bar(display &gui, const size_specs &sizes)
	: gui::widget(gui.video()), size_specs_(sizes), gui_(gui), selected_(0), total_brush_(3),
	  size_(30) {
	adjust_size();
}

void brush_bar::adjust_size() {// TODO
	set_location(size_specs_.brush_x, size_specs_.brush_y);
	set_measurements(size_ * total_brush_ + (total_brush_ - 1) * size_specs_.brush_padding, size_);
	set_dirty();
}

unsigned int brush_bar::selected_brush_size() {
	return selected_ + 1;
}

void brush_bar::select_brush_size(int new_size) {
	assert(new_size > 0 && new_size <= total_brush_);
	selected_ = new_size - 1;
}

void brush_bar::left_mouse_click(const int mousex, const int mousey) {
	int index = selected_index(mousex, mousey);
	if(index >= 0) {
		if (static_cast<unsigned>(index) != selected_) {
			set_dirty();
			selected_ = index;
		}
	}
}

void brush_bar::handle_event(const SDL_Event& event) {
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
	}
}

void brush_bar::draw() {
	draw(false);
}

void brush_bar::draw(bool force) {
	if (!dirty() && !force) {
		return;
	}
	const SDL_Rect loc = location();
	int x = loc.x;
	// Everything will be redrawn even though only one little part may
	// have changed, but that happens so seldom so we'll settle with this.
	SDL_Surface* const screen = gui_.video().getSurface();
	for (int i = 1; i <= total_brush_; i++) {
		std::stringstream filename;
		filename << "editor/brush-" << i << ".png";
		surface image(image::get_image(filename.str()));
		if (image == NULL) {
			std::cerr << "Image " << filename.str() << " not found." << std::endl;
			continue;
		}
		if (static_cast<unsigned>(image->w) != size_ ||
				static_cast<unsigned>(image->h) != size_) {

			image.assign(scale_surface(image, size_, size_));
		}
		SDL_Rect dstrect;
		dstrect.x = x;
		dstrect.y = size_specs_.brush_y;
		dstrect.w = image->w;
		dstrect.h = image->h;
		SDL_BlitSurface(image, NULL, screen, &dstrect);
		const Uint32 color = static_cast<unsigned>(i) == selected_brush_size() ?
			SDL_MapRGB(screen->format,0xFF,0x00,0x00) :
			SDL_MapRGB(screen->format,0x00,0x00,0x00);
		draw_rectangle(dstrect.x, dstrect.y, image->w, image->h, color, screen);
		x += image->w + size_specs_.brush_padding;
	}
	update_rect(loc);
	set_dirty(false);
}

int brush_bar::selected_index(const int x, const int y) const {
	const int bar_x = size_specs_.brush_x;
	const int bar_y = size_specs_.brush_y;

	if ((x < bar_x || static_cast<unsigned>(x) > bar_x + size_ * total_brush_ +
			total_brush_ * size_specs_.brush_padding) ||
		    (y < bar_y || static_cast<unsigned>(y) > bar_y + size_)) {

		return -1;
	}

	for(int i = 0; i < total_brush_; i++) {
		const int px = bar_x + size_ * i + i * size_specs_.brush_padding;

		if(x >= px && static_cast<unsigned>(x) <= px + size_ &&
				y >= bar_y && static_cast<unsigned>(y) <= bar_y + size_) {

			return i;
		}
	}
	return -1;
}

} // end namespace map_editor

