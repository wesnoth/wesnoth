/*
  Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
  Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the COPYING file for more details.
*/

#include "SDL.h"
#include "SDL_keysym.h"

#include "editor_palettes.hpp"
#include "editor_layout.hpp"
#include "../sdl_utils.hpp"
#include "../show_dialog.hpp"
#include "../image.hpp"
#include "../reports.hpp"
#include "../language.hpp"
#include "../util.hpp"


namespace map_editor {

bool is_invalid_terrain(char c) {
	return c == ' ' || c == '~';
}

terrain_palette::terrain_palette(display &gui, const size_specs &sizes,
								 const gamemap &map)
	: gui::widget(gui), size_specs_(sizes), gui_(gui), tstart_(0), map_(map),
	  top_button_(gui, "", gui::button::TYPE_PRESS, "uparrow-button"),
	  bot_button_(gui, "", gui::button::TYPE_PRESS, "downarrow-button") {

	terrains_ = map_.get_terrain_precedence();
	terrains_.erase(std::remove_if(terrains_.begin(), terrains_.end(), is_invalid_terrain),
					terrains_.end());
	if(terrains_.empty()) {
		std::cerr << "No terrain found.\n";
	}
	else {
		selected_fg_terrain_ = terrains_[0];
		selected_bg_terrain_ = terrains_[0];
	}
	update_report();
	adjust_size();
}

void terrain_palette::adjust_size() {
	scroll_top();
	const size_t button_height = 24;
	const size_t button_palette_padding = 8;
	set_location(size_specs_.palette_x, size_specs_.palette_y);
	set_width(size_specs_.palette_w);
	set_height(size_specs_.palette_h);
	top_button_y_ = size_specs_.palette_y;
	button_x_ = size_specs_.palette_x + size_specs_.palette_w/2 - button_height/2;
	terrain_start_ = top_button_y_ + button_height + button_palette_padding;
	const size_t space_for_terrains = size_specs_.palette_h -
		(button_height + button_palette_padding) * 2;
	const unsigned terrains_fitting =
		(unsigned)(space_for_terrains / size_specs_.terrain_space) * 2;
	const unsigned total_terrains = num_terrains();
	nterrains_ = minimum<int>(terrains_fitting, total_terrains);
	bot_button_y_ = size_specs_.palette_y + (nterrains_ / 2) * size_specs_.terrain_space +
		button_palette_padding * 2 + button_height;
	top_button_.set_location(button_x_, top_button_y_);
	bot_button_.set_location(button_x_, bot_button_y_);
	top_button_.set_dirty();
	bot_button_.set_dirty();
	set_dirty();
}

void terrain_palette::scroll_down() {
	if(tstart_ + nterrains_ + 2 <= num_terrains()) {
		tstart_ += 2;
		set_dirty();
	}
	else if (tstart_ + nterrains_ + 1 <= num_terrains()) {
		tstart_ += 1;
		set_dirty();
	}
}

void terrain_palette::scroll_up() {
	unsigned int decrement = 2;
	if (tstart_ + nterrains_ == num_terrains()
		&& num_terrains() % 2 != 0) {
		decrement = 1;
	}
	if(tstart_ >= decrement) {
		set_dirty();
		tstart_ -= decrement;
	}
}

void terrain_palette::scroll_top() {
	tstart_ = 0;
	set_dirty();
}

void terrain_palette::scroll_bottom() {
	unsigned int old_start = num_terrains();
	while (old_start != tstart_) {
		old_start = tstart_;
		scroll_down();
	}
}

gamemap::TERRAIN terrain_palette::selected_fg_terrain() const {
	return selected_fg_terrain_;
}

gamemap::TERRAIN terrain_palette::selected_bg_terrain() const {
	return selected_bg_terrain_;
}

void terrain_palette::select_fg_terrain(gamemap::TERRAIN terrain) {
	if (selected_fg_terrain_ != terrain) {
		set_dirty();
		selected_fg_terrain_ = terrain;
	}
}

void terrain_palette::select_bg_terrain(gamemap::TERRAIN terrain) {
	if (selected_bg_terrain_ != terrain) {
		set_dirty();
		selected_bg_terrain_ = terrain;
	}
}

std::string terrain_palette::get_terrain_string(const gamemap::TERRAIN t) {
	std::stringstream str;
	const std::string& name = map_.terrain_name(t);
	const std::vector<std::string>& underlying_names =
		map_.underlying_terrain_name(t);
	str << translate_string(name);
	if(underlying_names.size() != 1 || underlying_names.front() != name) {
		str << " (";
		for(std::vector<std::string>::const_iterator i = underlying_names.begin();
			i != underlying_names.end(); ++i) {
			str << translate_string(*i);
			if(i+1 != underlying_names.end()) {
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
		update_report();
		gui_.invalidate_game_status();
	}
}

void terrain_palette::right_mouse_click(const int mousex, const int mousey) {
	int tselect = tile_selected(mousex, mousey);
	if(tselect >= 0) {
		select_bg_terrain(terrains_[tstart_+tselect]);
		update_report();
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
			if (!focus()) {
				set_focus(true);
			}
		}
		// If the mouse is outside, remove focus.
		else {
			if (focus()) {
				set_focus(false);
			}
		}
	}
	if (!focus()) {
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
		const gamemap::TERRAIN terrain = terrains_[counter];
		const std::string filename = "terrain/" +
			map_.get_terrain_info(terrain).default_image() + ".png";
		scoped_sdl_surface image(image::get_image(filename, image::UNSCALED));
		if((unsigned)image->w != size_specs_.terrain_size
		   || (unsigned)image->h != size_specs_.terrain_size) {
			image.assign(scale_surface(image, size_specs_.terrain_size,
									   size_specs_.terrain_size));
		}
		if(image == NULL) {
			std::cerr << "image for terrain '" << counter << "' not found\n";
			return;
		}
		const int counter_from_zero = counter - starting;
		SDL_Rect dstrect;
		dstrect.x = loc.x + (counter_from_zero % 2 != 0 ? size_specs_.terrain_space : 0);
		dstrect.y = y;
		dstrect.w = image->w;
		dstrect.h = image->h;

		SDL_BlitSurface(image, NULL, screen, &dstrect);
		if (terrain == selected_bg_terrain() && terrain == selected_fg_terrain()) {
			gui::draw_rectangle(dstrect.x, dstrect.y, image->w, image->h, 0xF0F0, screen);
		}
		else if (terrain == selected_bg_terrain()) {
			gui::draw_rectangle(dstrect.x, dstrect.y, image->w, image->h, 0x00F0, screen);
		}
		else if (terrain == selected_fg_terrain()) {
			gui::draw_rectangle(dstrect.x, dstrect.y, image->w, image->h, 0xF000, screen);
		}
		else {
			gui::draw_rectangle(dstrect.x, dstrect.y, image->w, image->h, 0, screen);
		}
		if (counter_from_zero % 2 != 0)
			y += size_specs_.terrain_space;
	}
	update_rect(loc);
	set_dirty(false);
}
int terrain_palette::tile_selected(const int x, const int y) const {
	for(unsigned int i = 0; i != nterrains_; i++) {
		const int px = size_specs_.palette_x +
			(i % 2 != 0 ? size_specs_.terrain_space : 0);
		const int py = terrain_start_ + (i / 2) * size_specs_.terrain_space;
		const int pw = size_specs_.terrain_space;
		const int ph = size_specs_.terrain_space;

		if(x > px && x < px + pw && y > py && y < py + ph) {
			return i;
		}
	}
	return -1;
}

void terrain_palette::update_report() {
	const std::string msg = translate_string("edit_foreground_short") + ": "
		+ get_terrain_string(selected_fg_terrain()) + "\n"
		+ translate_string("edit_background_short") +
		": " + get_terrain_string(selected_bg_terrain());
	reports::set_report_content(reports::SELECTED_TERRAIN, msg);
}

brush_bar::brush_bar(display &gui, const size_specs &sizes)
	: gui::widget(gui), size_specs_(sizes), gui_(gui), selected_(0), total_brush_(3),
	  size_(30) {
	adjust_size();
}

void brush_bar::adjust_size() {
	set_location(size_specs_.brush_x, size_specs_.brush_y);
	set_width(size_ * total_brush_);
	set_height(size_);
	set_dirty();
}

unsigned int brush_bar::selected_brush_size() {
	return selected_ + 1;
}
		
void brush_bar::left_mouse_click(const int mousex, const int mousey) {
	int index = selected_index(mousex, mousey);
	if(index >= 0) {
		if ((unsigned)index != selected_) {
			set_dirty();
			selected_ = index;
		}
	}
}


void brush_bar::handle_event(const SDL_Event& event) {
	if (event.type == SDL_MOUSEMOTION) {
		// If the mouse is inside the palette, give it focus.
		if (point_in_rect(event.button.x, event.button.y, location())) {
			if (!focus()) {
				set_focus(true);
			}
		}
		// If the mouse is outside, remove focus.
		else {
			if (focus()) {
				set_focus(false);
			}
		}
	}
	if (!focus()) {
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
	// have changed, but that happens so seldom so we'll settle with
	// this.
	SDL_Surface* const screen = gui_.video().getSurface();
	for (int i = 1; i <= total_brush_; i++) {
		std::stringstream filename;
		filename << "editor/brush-" << i << ".png";
		scoped_sdl_surface image(image::get_image(filename.str(), image::UNSCALED));
		if (image == NULL) {
			std::cerr << "Image " << filename.str() << " not found." << std::endl;
			continue;
		}
		if ((unsigned)image->w != size_ || (unsigned)image->h != size_) {
			image.assign(scale_surface(image, size_, size_));
		}
		SDL_Rect dstrect;
		dstrect.x = x;
		dstrect.y = size_specs_.brush_y;
		dstrect.w = image->w;
		dstrect.h = image->h;
		SDL_BlitSurface(image, NULL, screen, &dstrect);
		gui::draw_rectangle(dstrect.x, dstrect.y, image->w, image->h,
				    ((unsigned)i == selected_brush_size()) ? 0xF000:0 , screen);
		x += image->w;
	}
	update_rect(loc);
	set_dirty(false);
}

int brush_bar::selected_index(const int x, const int y) const {
	const int bar_x = size_specs_.brush_x;
	const int bar_y = size_specs_.brush_y;

	if ((x < bar_x || (unsigned)x > bar_x + size_ * total_brush_)
	    || (y < bar_y || (unsigned)y > bar_y + size_)) {
		return -1;
	}

	for(int i = 0; i < total_brush_; i++) {
		const int px = bar_x + size_ * i;

		if(x >= px && (unsigned)x <= px + size_ && y >= bar_y && (unsigned)y <= bar_y + size_) {
			return i;
		}
	}
	return -1;
}

}
