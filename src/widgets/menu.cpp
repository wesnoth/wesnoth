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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "widgets/menu.hpp"

#include "draw.hpp"
#include "font/sdl_ttf_compat.hpp"
#include "font/standard_colors.hpp"
#include "game_config.hpp"
#include "language.hpp"
#include "lexical_cast.hpp"
#include "picture.hpp"
#include "sdl/rect.hpp"
#include "sdl/texture.hpp"
#include "sound.hpp"
#include "utils/general.hpp"
#include "video.hpp"
#include "wml_separators.hpp"

#include <numeric>

namespace {
	/**
	 * For converting indented_menu_item.indent_level into a width in pixels.
	 * The text size might change, so instead of caching a value pango_line_size
	 * is called repeatedly with this as an argument.
	 */
	const std::string indent_string{"    "};
};

namespace gui {

menu::menu(bool click_selects, int max_height, int max_width, style *menu_style, const bool auto_join)
: scrollarea(auto_join), silent_(false),
  max_height_(max_height), max_width_(max_width),
  max_items_(-1), item_height_(-1),
  selected_(0), click_selects_(click_selects), out_(false),
  previous_button_(true), show_result_(false),
  double_clicked_(false),
  num_selects_(true),
  ignore_next_doubleclick_(false),
  last_was_doubleclick_(false), use_ellipsis_(false)
{
	style_ = (menu_style) ? menu_style : &default_style;
	style_->init();
	fill_items({});
}

menu::~menu()
{
}

void menu::fill_items(const std::vector<indented_menu_item>& items)
{
	for(const auto& itor : items) {
		const std::size_t id = items_.size();
		item_pos_.push_back(id);
		items_.emplace_back(itor, id);
	}

	update_size();
}

void menu::update_scrollbar_grip_height()
{
	set_full_size(items_.size());
	set_shown_size(max_items_onscreen());
}

void menu::update_size()
{
	int h = 0;
	for(std::size_t i = get_position(),
	    i_end = std::min(items_.size(), i + max_items_onscreen());
	    i < i_end; ++i)
		h += get_item_rect(i).h;
	h = std::max(h, height());
	if (max_height_ > 0 && h > (max_height_)) {
		h = max_height_;
	}

	use_ellipsis_ = false;
	int w = widest_row_width();
	if (items_.size() > max_items_onscreen())
		w += scrollbar_width();
	w = std::max(w, width());
	if (max_width_ > 0 && w > (max_width_)) {
		use_ellipsis_ = true;
		w = max_width_;
	}

	update_scrollbar_grip_height();
	set_measurements(w, h);
}

int menu::selection() const
{
	if (selected_ >= items_.size()) {
		return -1;
	}

	return items_[selected_].id;
}

void menu::set_inner_location(const SDL_Rect& /*rect*/)
{
	itemRects_.clear();
	update_scrollbar_grip_height();
}

void menu::set_items(const std::vector<indented_menu_item>& items, std::optional<std::size_t> selected)
{
	const bool scrolled_to_max = (has_scrollbar() && get_position() == get_max_position());
	items_.clear();
	item_pos_.clear();
	itemRects_.clear();
	widest_row_width_.reset();
	//undrawn_items_.clear();
	max_items_ = -1; // Force recalculation of the max items.
	item_height_ = -1; // Force recalculation of the item height.

	if(selected) {
		selected_ = *selected;
	} else {
		selected_ = 0;
	}

	fill_items(items);
	if(scrolled_to_max) {
		set_position(get_max_position());
	}

	update_scrollbar_grip_height();

	adjust_viewport_to_selection();

	queue_redraw();
}

void menu::set_max_height(const int new_max_height)
{
	max_height_ = new_max_height;
	itemRects_.clear();
	max_items_ = -1;
	update_size();
}

void menu::set_max_width(const int new_max_width)
{
	max_width_ = new_max_width;
	itemRects_.clear();
	widest_row_width_.reset();
	update_size();
}

std::size_t menu::max_items_onscreen() const
{
	if(max_items_ != -1) {
		return std::size_t(max_items_);
	}

	const std::size_t max_height = (
			max_height_ == -1
				? (video::game_canvas_size().y * 66) / 100
				: max_height_
		);

	std::vector<int> heights;
	std::size_t n;
	for(n = 0; n != items_.size(); ++n) {
		// The for loop, sort and sum around this are unnecessary, because
		// get_item_height has ignored its argument since Wesnoth 0.6.99.1.
		// It caches and returns the height of the tallest item.
		heights.push_back(get_item_height(n));
	}

	std::sort(heights.begin(),heights.end(),std::greater<int>());
	std::size_t sum = 0;
	for(n = 0; n != items_.size() && sum < max_height; ++n) {
		sum += heights[n];
	}

	if(sum > max_height && n > 1)
		--n;

	return max_items_ = n;
}

void menu::adjust_viewport_to_selection()
{
	if(click_selects_)
		return;
	adjust_position(selected_);
}

void menu::set_selection_pos(std::size_t new_selected, bool silent, SELECTION_MOVE_VIEWPORT move_viewport)
{
	if (new_selected >= items_.size())
		return;

	bool changed = false;
	if (new_selected != selected_) {
		invalidate_row_pos(selected_);
		invalidate_row_pos(new_selected);
		selected_ = new_selected;
		changed = true;
	}

	if(move_viewport == MOVE_VIEWPORT) {
		adjust_viewport_to_selection();
		if(!silent_ && !silent && changed) {
			sound::play_UI_sound(game_config::sounds::menu_select);
		}
	}
}

void menu::move_selection_up(std::size_t dep)
{
	set_selection_pos(selected_ > dep ? selected_ - dep : 0);
}

void menu::move_selection_down(std::size_t dep)
{
	std::size_t nb_items = items_.size();
	set_selection_pos(selected_ + dep >= nb_items ? nb_items - 1 : selected_ + dep);
}

// private function with control over sound and viewport
void menu::move_selection_to(std::size_t id, bool silent, SELECTION_MOVE_VIEWPORT move_viewport)
{
	if(id < item_pos_.size()) {
		set_selection_pos(item_pos_[id], silent, move_viewport);
	}
}

// public function
void menu::move_selection(std::size_t id)
{
	if(id < item_pos_.size()) {
		set_selection_pos(item_pos_[id], true, MOVE_VIEWPORT);
	}
}

// public function
void menu::move_selection_keeping_viewport(std::size_t id)
{
	if(id < item_pos_.size()) {
		set_selection_pos(item_pos_[id], true, NO_MOVE_VIEWPORT);
	}
}

void menu::reset_selection()
{
	set_selection_pos(0, true);
}

void menu::key_press(SDL_Keycode key)
{
	if (!click_selects_) {
		switch(key) {
		case SDLK_UP:
			move_selection_up(1);
			break;
		case SDLK_DOWN:
			move_selection_down(1);
			break;
		case SDLK_PAGEUP:
			move_selection_up(max_items_onscreen());
			break;
		case SDLK_PAGEDOWN:
			move_selection_down(max_items_onscreen());
			break;
		case SDLK_HOME:
			set_selection_pos(0);
			break;
		case SDLK_END:
			set_selection_pos(items_.size() - 1);
			break;
		//case SDLK_RETURN:
		//	double_clicked_ = true;
		//	break;
		default:
			break;
		}
	}

	if (num_selects_ && key >= SDLK_1 && key <= SDLK_9)
		set_selection_pos(key - SDLK_1);
}

bool menu::requires_event_focus(const SDL_Event* event) const
{
	if(!focus_ || height() == 0 || hidden()) {
		return false;
	}
	if(event == nullptr) {
		//when event is not specified, signal that focus may be desired later
		return true;
	}

	if(event->type == SDL_KEYDOWN) {
		SDL_Keycode key = event->key.keysym.sym;
		if (!click_selects_) {
			switch(key) {
			case SDLK_UP:
			case SDLK_DOWN:
			case SDLK_PAGEUP:
			case SDLK_PAGEDOWN:
			case SDLK_HOME:
			case SDLK_END:
				return true;
			default:
				break;
			}
		}
		if (num_selects_ && key >= SDLK_1 && key <= SDLK_9) {
			return true;
		}
	}
	//mouse events are processed regardless of focus
	return false;
}

void menu::handle_event(const SDL_Event& event)
{
	scrollarea::handle_event(event);
	if (height()==0 || hidden())
		return;

	if(event.type == SDL_KEYDOWN) {
		// Only pass key events if we have the focus
		if (focus(&event))
			key_press(event.key.keysym.sym);
	} else if(!mouse_locked() && ((event.type == SDL_MOUSEBUTTONDOWN &&
	         (event.button.button == SDL_BUTTON_LEFT || event.button.button == SDL_BUTTON_RIGHT)) ||
	         event.type == DOUBLE_CLICK_EVENT)) {

		int x = 0;
		int y = 0;
		if(event.type == SDL_MOUSEBUTTONDOWN) {
			x = event.button.x;
			y = event.button.y;
		} else {
			x = reinterpret_cast<std::size_t>(event.user.data1);
			y = reinterpret_cast<std::size_t>(event.user.data2);
		}

		const int item = hit(x,y);
		if(item != -1) {
			set_focus(true);
			move_selection_to(item);

			if(click_selects_) {
				show_result_ = true;
			}

			if(event.type == DOUBLE_CLICK_EVENT) {
				if (ignore_next_doubleclick_) {
					ignore_next_doubleclick_ = false;
				} else {
					double_clicked_ = true;
					last_was_doubleclick_ = true;
					if(!silent_) {
						sound::play_UI_sound(game_config::sounds::button_press);
					}
				}
			} else if (last_was_doubleclick_) {
				// If we have a double click as the next event, it means
				// this double click was generated from a click that
				// already has helped in generating a double click.
				SDL_Event ev;
				SDL_PeepEvents(&ev, 1, SDL_PEEKEVENT, DOUBLE_CLICK_EVENT, DOUBLE_CLICK_EVENT);
				if (ev.type == DOUBLE_CLICK_EVENT) {
					ignore_next_doubleclick_ = true;
				}
				last_was_doubleclick_ = false;
			}
		}

	} else if(!mouse_locked() && event.type == SDL_MOUSEMOTION) {
		if(click_selects_) {
			const int item = hit(event.motion.x,event.motion.y);
			const bool out = (item == -1);
			if (out_ != out) {
				out_ = out;
				invalidate_row_pos(selected_);
			}
			if (item != -1) {
				move_selection_to(item);
			}
		}
	}
}

int menu::process()
{
	if(show_result_) {
		show_result_ = false;
		return selected_;
	} else {
		return -1;
	}
}

bool menu::double_clicked()
{
	bool old = double_clicked_;
	double_clicked_ = false;
	return old;
}

void menu::set_click_selects(bool value)
{
	click_selects_ = value;
}

void menu::set_numeric_keypress_selection(bool value)
{
	num_selects_ = value;
}

void menu::scroll(unsigned int)
{
	itemRects_.clear();
	queue_redraw();
}

SDL_Rect menu::style::item_size(const indented_menu_item& imi) const {
	SDL_Rect res {0,0,0,0};

	res.w = imi.indent_level * font::pango_line_size(indent_string, get_font_size()).first;

	if (!imi.icon.empty()) {
		// Not the first item, add the spacing.
		res.w += 5;

		const texture img = image::get_texture(imi.icon);
		res.w += img.w();
		res.h = std::max<int>(img.h(), res.h);
	}

	if (!imi.text.empty()) {
		// Not the first item, add the spacing.
		res.w += 5;

		const SDL_Rect area {0,0,10000,10000};
		const SDL_Rect font_size =
			font::pango_draw_text(false, area, get_font_size(),
				font::NORMAL_COLOR, imi.text, 0, 0);
		res.w += font_size.w;
		res.h = std::max<int>(font_size.h, res.h);
	}
	return res;
}

void menu::style::draw_row_bg(menu& /*menu_ref*/, const std::size_t /*row_index*/, const SDL_Rect& rect, ROW_TYPE type)
{
	int rgb = 0;
	double alpha = 0.0;

	switch(type) {
	case NORMAL_ROW:
		rgb = normal_rgb_;
		alpha = normal_alpha_;
		break;
	case SELECTED_ROW:
		rgb = selected_rgb_;
		alpha = selected_alpha_;
		break;
	}

	// FIXME: make this clearer
	color_t c((rgb & 0xff0000) >> 16, (rgb & 0xff00) >> 8, rgb & 0xff);
	c.a = 255 * alpha;

	draw::fill(rect, c);
}

void menu::style::draw_row(menu& menu_ref, const std::size_t row_index, const SDL_Rect& rect, ROW_TYPE type)
{
	if(rect.w == 0 || rect.h == 0) {
		return;
	}
	draw_row_bg(menu_ref, row_index, rect, type);

	SDL_Rect minirect = rect;
	minirect.x += thickness_;
	minirect.y += thickness_;
	minirect.w -= 2*thickness_;
	minirect.h -= 2*thickness_;
	menu_ref.draw_row(row_index, minirect, type);
}

int menu::widest_row_width() const
{
	if(!widest_row_width_) {
		int widest = 0;
		for(const auto& row : items_) {
			const SDL_Rect size = style_->item_size(row.fields);
			widest = std::max(widest, size.w);
		}
		// Assume there's text at the end of the item, and add padding accordingly.
		widest_row_width_ = static_cast<int>(widest + style_->get_cell_padding());
	}

	return *widest_row_width_;
}

bool menu::hit_on_indent_or_icon(std::size_t row_index, int x) const
{
	if(row_index >= items_.size()) {
		return false;
	}

	// The virtual method item_size() is overloaded by imgsel_style::item_size(),
	// which adds borders on both sides. Call it twice and remove one side's padding.
	const auto& imi = items_[row_index].fields;
	int width_used_so_far = style_->item_size({imi.indent_level, imi.icon, ""}).w;
	width_used_so_far -= style_->item_size({0, "", ""}).w / 2;

	const SDL_Rect& loc = inner_location();
	if (current_language_rtl()) {
		// inner_location() already takes account of the scrollbar width
		return x > loc.x + loc.w - width_used_so_far;
	}
	return x < loc.x + width_used_so_far;
}

void menu::draw_row(const std::size_t row_index, const SDL_Rect& loc, ROW_TYPE)
{
	//called from style, draws one row's contents in a generic and adaptable way
	const auto& imi = items_[row_index].fields;
	rect area = video::game_canvas();
	bool lang_rtl = current_language_rtl();

	// There's nothing to draw for the indent, just mark the space as used
	int width_used_so_far = imi.indent_level * font::pango_line_size(indent_string, style_->get_font_size()).first;

	if (!imi.icon.empty()) {
		const texture img = image::get_texture(imi.icon);
		int img_w = img.w();
		int img_h = img.h();
		const int remaining_width = max_width_ < 0 ? area.w : std::min<int>(max_width_, loc.w - width_used_so_far);
		if(img && img_w <= remaining_width && loc.y + img_h < area.h) {
			const std::size_t y = loc.y + (loc.h - img_h)/2;
			const std::size_t x = loc.x + (lang_rtl ? loc.w - width_used_so_far - img_w : width_used_so_far);
			draw::blit(img, {int(x), int(y), img_w, img_h});

			// If there wasn't space for the icon, it doesn't get drawn, nor does the width get used.
			// If it is drawn, add 5 pixels of padding.
			width_used_so_far += img_w + 5;
		}
	}

	// Expected to be non-empty, but I guess a unit type could have a blank name
	if (!imi.text.empty()) {
		const auto text_size = font::pango_line_size(imi.text, style_->get_font_size());
		const std::size_t x = loc.x + (lang_rtl ? std::max(0, loc.w - width_used_so_far - text_size.first) : width_used_so_far);
		const std::size_t y = loc.y + (loc.h - text_size.second)/2;
		rect text_loc = loc;
		text_loc.w = loc.w - (width_used_so_far) - 2 * style_->get_thickness();
		text_loc.h = text_size.second;
		font::pango_draw_text(true, text_loc, style_->get_font_size(), font::NORMAL_COLOR, imi.text,
			x, y);
	}
}

void menu::draw_contents()
{
	for(std::size_t i = 0; i != item_pos_.size(); ++i) {
		style_->draw_row(*this,item_pos_[i],get_item_rect(i),
			 (!out_ && item_pos_[i] == selected_) ? SELECTED_ROW : NORMAL_ROW);
	}
}

int menu::hit(int x, int y) const
{
	const SDL_Rect& loc = inner_location();
	if (x >= loc.x  && x < loc.x + loc.w && y >= loc.y && y < loc.y + loc.h) {
		for(std::size_t i = 0; i != items_.size(); ++i) {
			const SDL_Rect& rect = get_item_rect(i);
			if (y >= rect.y && y < rect.y + rect.h)
				return i;
		}
	}

	return -1;
}

SDL_Rect menu::get_item_rect(int item) const
{
	return get_item_rect_internal(item_pos_[item]);
}

SDL_Rect menu::get_item_rect_internal(std::size_t item) const
{
	unsigned int first_item_on_screen = get_position();
	if (item < first_item_on_screen ||
	    item >= first_item_on_screen + max_items_onscreen()) {
		return sdl::empty_rect;
	}

	const std::map<int,SDL_Rect>::const_iterator i = itemRects_.find(item);
	if(i != itemRects_.end())
		return i->second;

	const SDL_Rect& loc = inner_location();

	int y = loc.y;
	if (item != first_item_on_screen) {
		const SDL_Rect& prev = get_item_rect_internal(item-1);
		y = prev.y + prev.h;
	}

	rect res(loc.x, y, loc.w, get_item_height(item));

	const point canvas_size = video::game_canvas_size();

	if(res.x > canvas_size.x) {
		return sdl::empty_rect;
	} else if(res.x + res.w > canvas_size.x) {
		res.w = canvas_size.x - res.x;
	}

	if(res.y > canvas_size.y) {
		return sdl::empty_rect;
	} else if(res.y + res.h > canvas_size.y) {
		res.h = canvas_size.y - res.y;
	}

	//only insert into the cache if the menu's co-ordinates have
	//been initialized
	if (loc.x > 0 && loc.y > 0)
		itemRects_.emplace(item, res);

	return res;
}

std::size_t menu::get_item_height_internal(const indented_menu_item& imi) const
{
	return style_->item_size(imi).h;
}

std::size_t menu::get_item_height(int) const
{
	// This could probably return the height of a single line of Pango text, plus
	// padding. However, keeping compatibility with the current numbers means
	// less unknowns about what the numbers should actually be.
	if(item_height_ != -1)
		return std::size_t(item_height_);

	std::size_t max_height = 0;
	for(const auto& item : items_) {
		max_height = std::max<int>(max_height,get_item_height_internal(item.fields));
	}

	return item_height_ = max_height;
}

void menu::invalidate_row(std::size_t id)
{
	if(id >= items_.size()) {
		return;
	}

	queue_redraw(get_item_rect(id));
}

void menu::invalidate_row_pos(std::size_t pos)
{
	if(pos >= items_.size()) {
		return;
	}

	invalidate_row(items_[pos].id);
}

}
