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

/**
 * Manage the brush bar in the editor.
 * Note: this is a near-straight rip from the old editor.
 */

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "brush_bar.hpp"

#include "sound.hpp"


namespace editor {

brush_bar::brush_bar(editor_display &gui, std::vector<brush>& brushes, brush** the_brush)
	: gui::widget(gui.video()), gui_(gui),
	  selected_(0), brushes_(brushes), the_brush_(the_brush),
	  size_(30) {
	adjust_size();
}

void brush_bar::adjust_size() {
	set_location(gui_.brush_bar_area());
	//TODO
	//set_location(size_specs_.brush_x -1, size_specs_.brush_y -1);
	set_measurements(size_ * brushes_.size() + (brushes_.size() -1) * (brush_padding_) + 2, (size_ +2));
	set_dirty();
}

unsigned int brush_bar::selected_brush_size() {
	return selected_;
}

void brush_bar::left_mouse_click(const int mousex, const int mousey) {
	sound::play_UI_sound(game_config::sounds::button_press);
	int index = selected_index(mousex, mousey);
	if(index >= 0) {
		if (static_cast<size_t>(index) != selected_) {
			set_dirty();
			selected_ = index;
			*the_brush_ = &brushes_[index];
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
	surface screen = gui_.video().getSurface();
	for (size_t i = 0; i < brushes_.size(); i++) {
		std::string filename = brushes_[i].image();
		surface image(image::get_image(filename));
		if (image == NULL) {
			ERR_ED << "Image " << filename << " not found." << std::endl;
			continue;
		}
		if (static_cast<unsigned>(image->w) != size_
		|| static_cast<unsigned>(image->h) != size_) {
			image.assign(scale_surface(image, size_, size_));
		}
		SDL_Rect dstrect = create_rect(x, gui_.brush_bar_area().y, image->w, image->h);
		sdl_blit(image, NULL, screen, &dstrect);
		const Uint32 color = i == selected_brush_size() ?
			SDL_MapRGB(screen->format,0xFF,0x00,0x00) :
			SDL_MapRGB(screen->format,0x00,0x00,0x00);
		//TODO fendrin
		//draw_rectangle(dstrect.x -1, dstrect.y -1, image->w +2, image->h+2, color, screen);
		draw_rectangle(dstrect.x, dstrect.y, image->w, image->h, color, screen);
		x += image->w + brush_padding_;
	}
	update_rect(loc);
	set_dirty(false);
}

int brush_bar::selected_index(int x, int y) const {
	const int bar_x = gui_.brush_bar_area().x;
	const int bar_y = gui_.brush_bar_area().y;

	if ((x < bar_x || static_cast<size_t>(x) > bar_x + size_ * brushes_.size() +
	                  brushes_.size() * brush_padding_) ||
	    (y < bar_y || static_cast<size_t>(y) > bar_y + size_))
	{
		return -1;
	}

	for(size_t i = 0; i <  brushes_.size(); i++) {
		int px = bar_x + size_ * i + i * brush_padding_;
		if (x >= px && x <= px + static_cast<int>(size_) && y >= bar_y && y <= bar_y + static_cast<int>(size_)) {
			return i;
		}
	}
	return -1;
}

}
