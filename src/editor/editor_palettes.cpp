/*
  Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
  Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the COPYING file for more details.
*/

#include "editor_palettes.hpp"
#include "editor_layout.hpp"
#include "../show_dialog.hpp"
#include "../image.hpp"


namespace map_editor {

bool is_invalid_terrain(char c) {
	return c == ' ' || c == '~';
}

terrain_palette::terrain_palette(display &gui, const size_specs &sizes,
								 const gamemap &map)
	: size_specs_(sizes), gui_(gui), tstart_(0), map_(map), invalid_(true) {

	terrains_ = map_.get_terrain_precedence();
	terrains_.erase(std::remove_if(terrains_.begin(), terrains_.end(), is_invalid_terrain),
					terrains_.end());
	if(terrains_.empty()) {
		std::cerr << "No terrain found.\n";
	}
	else {
		selected_terrain_ = terrains_[0];
	}
}

void terrain_palette::scroll_down() {
	if(tstart_ + size_specs_.nterrains + 2 <= num_terrains()) {
		tstart_ += 2;
		invalid_ = true;
	}
	else if (tstart_ + size_specs_.nterrains + 1 <= num_terrains()) {
		tstart_ += 1;
		invalid_ = true;
	}
}

void terrain_palette::scroll_up() {
	unsigned int decrement = 2;
	if (tstart_ + size_specs_.nterrains == num_terrains()
		&& terrains_.size() % 2 != 0) {
		decrement = 1;
	}
	if(tstart_ >= decrement) {
		invalid_ = true;
		tstart_ -= decrement;
	}
}

gamemap::TERRAIN terrain_palette::selected_terrain() const {
	return selected_terrain_;
}

void terrain_palette::select_terrain(gamemap::TERRAIN terrain) {
	if (selected_terrain_ != terrain) {
		invalid_ = true;
		selected_terrain_ = terrain;
	}
}

void terrain_palette::left_mouse_click(const int mousex, const int mousey) {
	int tselect = tile_selected(mousex, mousey);
	if(tselect >= 0) {
		select_terrain(terrains_[tstart_+tselect]);
	}
}

size_t terrain_palette::num_terrains() const {
	return terrains_.size();
}

void terrain_palette::draw(bool force) {
	if (!invalid_ && !force) {
		return;
	}
	size_t x = gui_.mapx() + size_specs_.palette_x;
	size_t y = size_specs_.palette_y;

	unsigned int starting = tstart_;
	unsigned int ending = starting + size_specs_.nterrains;

	SDL_Rect invalid_rect;
	invalid_rect.x = x;
	invalid_rect.y = y;
	invalid_rect.w = 0;
	invalid_rect.w = size_specs_.terrain_space * 2;
	invalid_rect.h = (size_specs_.nterrains / 2) * size_specs_.terrain_space;
	// Everything will be redrawn even though only one little part may
	// have changed, but that happens so seldom so we'll settle with
	// this.
	SDL_Surface* const screen = gui_.video().getSurface();
	SDL_BlitSurface(surf_, NULL, screen, &invalid_rect);
	surf_.assign(get_surface_portion(screen, invalid_rect));
	if(ending > num_terrains()){
		ending = num_terrains();
	}
	for(unsigned int counter = starting; counter < ending; counter++){
		const gamemap::TERRAIN terrain = terrains_[counter];
		const std::string filename = "terrain/" +
			map_.get_terrain_info(terrain).default_image() + ".png";
		scoped_sdl_surface image(image::get_image(filename, image::UNSCALED));
		if(image->w != size_specs_.terrain_size || image->h != size_specs_.terrain_size) {
			image.assign(scale_surface(image, size_specs_.terrain_size,
									   size_specs_.terrain_size));
		}
		if(image == NULL) {
			std::cerr << "image for terrain '" << counter << "' not found\n";
			return;
		}
		const int counter_from_zero = counter - starting;
		SDL_Rect dstrect;
		dstrect.x = x + (counter_from_zero % 2 != 0 ? size_specs_.terrain_space : 0);
		dstrect.y = y;
		dstrect.w = image->w;
		dstrect.h = image->h;

		SDL_BlitSurface(image, NULL, screen, &dstrect);
		gui::draw_rectangle(dstrect.x, dstrect.y, image->w, image->h,
							terrain == selected_terrain() ? 0xF000:0 , screen);
	
		if (counter_from_zero % 2 != 0)
			y += size_specs_.terrain_space;
	}
	update_rect(invalid_rect);
	invalid_ = false;
}

int terrain_palette::tile_selected(const int x, const int y) const {
	for(unsigned int i = 0; i != size_specs_.nterrains; i++) {
		const int px = gui_.mapx() + size_specs_.palette_x +
			(i % 2 != 0 ? size_specs_.terrain_space : 0);
		const int py = size_specs_.palette_y + (i / 2) * size_specs_.terrain_space;
		const int pw = size_specs_.terrain_space;
		const int ph = size_specs_.terrain_space;

		if(x > px && x < px + pw && y > py && y < py + ph) {
			return i;
		}
	}
	return -1;
}


brush_bar::brush_bar(display &gui, const size_specs &sizes)
	: size_specs_(sizes), gui_(gui), selected_(0), total_brush_(3),
	  size_(30), invalid_(true) {}

unsigned int brush_bar::selected_brush_size() {
	return selected_ + 1;
}
		
void brush_bar::left_mouse_click(const int mousex, const int mousey) {
	int index = selected_index(mousex, mousey);
	if(index >= 0) {
		if (index != selected_) {
			invalid_ = true;
			selected_ = index;
		}
	}
}

void brush_bar::draw(bool force) {
	if (!invalid_ && !force) {
		return;
	}
	size_t x = gui_.mapx() + size_specs_.brush_x;
	size_t y = size_specs_.brush_y;

	SDL_Rect invalid_rect;
	invalid_rect.x = x;
	invalid_rect.y = y;
	invalid_rect.w = size_ * total_brush_;
	invalid_rect.h = size_;
	// Everything will be redrawn even though only one little part may
	// have changed, but that happens so seldom so we'll settle with
	// this.
	SDL_Surface* const screen = gui_.video().getSurface();
	SDL_BlitSurface(surf_, NULL, screen, &invalid_rect);
	surf_.assign(get_surface_portion(screen, invalid_rect));

	for (int i = 1; i <= total_brush_; i++) {
		std::stringstream filename;
		filename << "editor/brush-" << i << ".png";
		scoped_sdl_surface image(image::get_image(filename.str(), image::UNSCALED));
		if (image == NULL) {
			std::cerr << "Image " << filename.str() << " not found." << std::endl;
			continue;
		}
		if (image->w != size_ || image->h != size_) {
			image.assign(scale_surface(image, size_, size_));
		}
		SDL_Rect dstrect;
		dstrect.x = x;
		dstrect.y = size_specs_.brush_y;
		dstrect.w = image->w;
		dstrect.h = image->h;
		SDL_BlitSurface(image, NULL, screen, &dstrect);
		gui::draw_rectangle(dstrect.x, dstrect.y, image->w, image->h,
				    (i == selected_brush_size()) ? 0xF000:0 , screen);
		x += image->w;
	}
	update_rect(invalid_rect);
	invalid_ = false;
}

int brush_bar::selected_index(const int x, const int y) const {
	const int bar_x = gui_.mapx() + size_specs_.brush_x;
	const int bar_y = size_specs_.brush_y;

	if ((x < bar_x || x > bar_x + size_ * total_brush_)
	    || (y < bar_y || y > bar_y + size_)) {
		return -1;
	}

	for(unsigned int i = 0; i < total_brush_; i++) {
		const int px = bar_x + size_ * i;

		if(x >= px && x <= px + size_ && y >= bar_y && y <= bar_y + size_) {
			return i;
		}
	}
	return -1;
}

}
