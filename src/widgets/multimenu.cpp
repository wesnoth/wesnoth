/*
   Copyright (C) 2015 by Boldizsár Lipka <lipkab@zoho.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "multimenu.hpp"
#include "image.hpp"
#include "video.hpp"

namespace gui {
	multimenu::multimenu(CVideo &video, const std::vector<std::string> &items, bool click_selects, int max_height,
						 int max_width, const menu::sorter *sorter_obj, menu::style *menu_style, const bool auto_join) :
		menu(video, items, click_selects, max_height, max_width, sorter_obj, menu_style, auto_join),
		active_items_(items.size(), false) {}

	void multimenu::draw_row(const size_t row_index, const SDL_Rect &rect, menu::ROW_TYPE type) {
		surface img = image::get_image(active_items_[row_index]
									   ? "buttons/checkbox-pressed.png"
									   : "buttons/checkbox.png");
		blit_surface(img, NULL, video().getSurface(), &rect);
		SDL_Rect newrect = {
				Sint16 (rect.x + img->w + 2),
				rect.y,
				Uint16 (rect.w - img->w - 2),
				rect.h
		};
		menu::draw_row(row_index, newrect, type);
	}

	void multimenu::handle_event(const SDL_Event &event) {
		if (event.type == SDL_MOUSEBUTTONDOWN) {
			int hit_box = hit_checkbox(event.button.x, event.button.y);
			if (hit_box != -1) {
				active_items_[hit_box] = !active_items_[hit_box];
				invalidate_row_pos(hit_box);
				last_changed_ = hit_box;
				return;
			}
		}
		menu::handle_event(event);
	}

	int multimenu::hit_checkbox(int x, int y) const {
		int cb_width = image::get_image("buttons/checkbox-pressed.png")->w;
		return x > inner_location().x + cb_width ? -1 : hit(x, y);
	}

	void multimenu::erase_item(size_t index) {
		active_items_.erase(active_items_.begin() + index);
		menu::erase_item(index);
		last_changed_ = -1;
	}

	void multimenu::set_items(const std::vector<std::string> &items, bool strip_spaces, bool keep_viewport) {
		active_items_.resize(items.size());
		std::fill(active_items_.begin(), active_items_.end(), false);
		last_changed_ = -1;
		menu::set_items(items, strip_spaces, keep_viewport);
	}

	void multimenu::set_active(size_t index, bool active) {
		active_items_[index] = active;
		invalidate_row_pos(index);
	}

	int multimenu::last_changed() {
		int result = last_changed_;
		last_changed_ = -1;
		return result;
	}
}
