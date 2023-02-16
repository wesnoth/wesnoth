/*
	Copyright (C) 2003 - 2022
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

namespace gui {

menu::basic_sorter::basic_sorter()
	: alpha_sort_()
	, numeric_sort_()
	, id_sort_()
	, redirect_sort_()
	, pos_sort_()
{
	set_id_sort(-1);
}

menu::basic_sorter& menu::basic_sorter::set_alpha_sort(int column)
{
	alpha_sort_.insert(column);
	return *this;
}

menu::basic_sorter& menu::basic_sorter::set_numeric_sort(int column)
{
	numeric_sort_.insert(column);
	return *this;
}

menu::basic_sorter& menu::basic_sorter::set_id_sort(int column)
{
	id_sort_.insert(column);
	return *this;
}

menu::basic_sorter& menu::basic_sorter::set_redirect_sort(int column, int to)
{
	if(column != to) {
		redirect_sort_.emplace(column, to);
	}

	return *this;
}

menu::basic_sorter& menu::basic_sorter::set_position_sort(int column, const std::vector<int>& pos)
{
	pos_sort_[column] = pos;
	return *this;
}

bool menu::basic_sorter::column_sortable(int column) const
{
	const std::map<int,int>::const_iterator redirect = redirect_sort_.find(column);
	if(redirect != redirect_sort_.end()) {
		return column_sortable(redirect->second);
	}

	return alpha_sort_.count(column) == 1 || numeric_sort_.count(column) == 1 ||
		   pos_sort_.count(column) == 1 || id_sort_.count(column) == 1;
}

bool menu::basic_sorter::less(int column, const item& row1, const item& row2) const
{
	const std::map<int,int>::const_iterator redirect = redirect_sort_.find(column);
	if(redirect != redirect_sort_.end()) {
		return less(redirect->second,row1,row2);
	}

	if(id_sort_.count(column) == 1) {
		return row1.id < row2.id;
	}

	if(column < 0 || column >= int(row2.fields.size())) {
		return false;
	}

	if(column >= int(row1.fields.size())) {
		return true;
	}

	const std::string& item1 = row1.fields[column];
	const std::string& item2 = row2.fields[column];

	if(alpha_sort_.count(column) == 1) {
		std::string::const_iterator begin1 = item1.begin(), end1 = item1.end(),
		                            begin2 = item2.begin(), end2 = item2.end();
		while(begin1 != end1 && is_wml_separator(*begin1)) {
			++begin1;
		}

		while(begin2 != end2 && is_wml_separator(*begin2)) {
			++begin2;
		}

		return std::lexicographical_compare(begin1,end1,begin2,end2,utils::chars_less_insensitive);
	} else if(numeric_sort_.count(column) == 1) {
		int val_1 = lexical_cast_default<int>(item1, 0);
		int val_2 = lexical_cast_default<int>(item2, 0);

		return val_1 > val_2;
	}

	const std::map<int,std::vector<int>>::const_iterator itor = pos_sort_.find(column);
	if(itor != pos_sort_.end()) {
		const std::vector<int>& pos = itor->second;
		if(row1.id >= pos.size()) {
			return false;
		}

		if(row2.id >= pos.size()) {
			return true;
		}

		return pos[row1.id] < pos[row2.id];
	}

	return false;
}

menu::menu(const std::vector<std::string>& items,
		bool click_selects, int max_height, int max_width,
		const sorter* sorter_obj, style *menu_style, const bool auto_join)
: scrollarea(auto_join), silent_(false),
  max_height_(max_height), max_width_(max_width),
  max_items_(-1), item_height_(-1),
  heading_height_(-1),
  selected_(0), click_selects_(click_selects), out_(false),
  previous_button_(true), show_result_(false),
  double_clicked_(false),
  num_selects_(true),
  ignore_next_doubleclick_(false),
  last_was_doubleclick_(false), use_ellipsis_(false),
  sorter_(sorter_obj), sortby_(-1), sortreversed_(false), highlight_heading_(-1)
{
	style_ = (menu_style) ? menu_style : &default_style;
	style_->init();
	fill_items(items, true);
}

menu::~menu()
{
}

void menu::fill_items(const std::vector<std::string>& items, bool strip_spaces)
{
	for(std::vector<std::string>::const_iterator itor = items.begin();
	    itor != items.end(); ++itor) {

		if(itor->empty() == false && (*itor)[0] == HEADING_PREFIX) {
			heading_ = utils::quoted_split(itor->substr(1),COLUMN_SEPARATOR, !strip_spaces);
			continue;
		}

		const std::size_t id = items_.size();
		item_pos_.push_back(id);
		const item new_item(utils::quoted_split(*itor, COLUMN_SEPARATOR, !strip_spaces),id);
		items_.push_back(new_item);

		//make sure there is always at least one item
		if(items_.back().fields.empty()) {
			items_.back().fields.push_back(" ");
		}

		//if the first character in an item is an asterisk,
		//it means this item should be selected by default
		std::string& first_item = items_.back().fields.front();
		if(first_item.empty() == false && first_item[0] == DEFAULT_ITEM) {
			selected_ = id;
			first_item.erase(first_item.begin());
		}
	}

	if(sortby_ >= 0) {
		do_sort();
	}
	update_size();
}

namespace {

class sort_func
{
public:
	sort_func(const menu::sorter& pred, int column) : pred_(&pred), column_(column)
	{}

	bool operator()(const menu::item& a, const menu::item& b) const
	{
		return pred_->less(column_,a,b);
	}

private:
	const menu::sorter* pred_;
	int column_;
};

}

void menu::do_sort()
{
	if(sorter_ == nullptr || sorter_->column_sortable(sortby_) == false) {
		return;
	}

	const int selectid = selection();

	std::stable_sort(items_.begin(), items_.end(), sort_func(*sorter_, sortby_));
	if (sortreversed_)
		std::reverse(items_.begin(), items_.end());

	recalculate_pos();

	if(selectid >= 0 && selectid < int(item_pos_.size())) {
		move_selection_to(selectid, true, NO_MOVE_VIEWPORT);
	}

	queue_redraw();
}

void menu::recalculate_pos()
{
	std::size_t sz = items_.size();
	item_pos_.resize(sz);
	for(std::size_t i = 0; i != sz; ++i)
		item_pos_[items_[i].id] = i;
	assert_pos();
}

void menu::assert_pos()
{
	std::size_t sz = items_.size();
	assert(item_pos_.size() == sz);
	for(std::size_t n = 0; n != sz; ++n) {
		assert(item_pos_[n] < sz && n == items_[item_pos_[n]].id);
	}
}

void menu::update_scrollbar_grip_height()
{
	set_full_size(items_.size());
	set_shown_size(max_items_onscreen());
}

void menu::update_size()
{
	int h = heading_height();
	for(std::size_t i = get_position(),
	    i_end = std::min(items_.size(), i + max_items_onscreen());
	    i < i_end; ++i)
		h += get_item_rect(i).h;
	h = std::max(h, height());
	if (max_height_ > 0 && h > (max_height_)) {
		h = max_height_;
	}

	use_ellipsis_ = false;
	const std::vector<int>& widths = column_widths();
	int w = std::accumulate(widths.begin(), widths.end(), 0);
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

void menu::change_item(int pos1, int pos2,const std::string& str)
{
	if(pos1 < 0 || pos1 >= int(item_pos_.size()) ||
		pos2 < 0 || pos2 >= int(items_[item_pos_[pos1]].fields.size())) {
		return;
	}

	items_[item_pos_[pos1]].fields[pos2] = str;
	queue_redraw();
}

void menu::erase_item(std::size_t index)
{
	std::size_t nb_items = items_.size();
	if (index >= nb_items)
		return;
	--nb_items;

	clear_item(nb_items);

	// fix ordered positions of items
	std::size_t pos = item_pos_[index];
	item_pos_.erase(item_pos_.begin() + index);
	items_.erase(items_.begin() + pos);
	for(std::size_t i = 0; i != nb_items; ++i) {
		std::size_t &n1 = item_pos_[i], &n2 = items_[i].id;
		if (n1 > pos) --n1;
		if (n2 > index) --n2;
	}
	assert_pos();

	if (selected_ >= nb_items)
		selected_ = nb_items - 1;

	update_scrollbar_grip_height();
	adjust_viewport_to_selection();
	itemRects_.clear();
	queue_redraw();
}

void menu::set_heading(const std::vector<std::string>& heading)
{
	itemRects_.clear();
	column_widths_.clear();

	heading_ = heading;
	max_items_ = -1;

	queue_redraw();
}

void menu::set_items(const std::vector<std::string>& items, bool strip_spaces, bool keep_viewport)
{

	const bool scrolled_to_max = (has_scrollbar() && get_position() == get_max_position());
	items_.clear();
	item_pos_.clear();
	itemRects_.clear();
	column_widths_.clear();
	//undrawn_items_.clear();
	max_items_ = -1; // Force recalculation of the max items.
	item_height_ = -1; // Force recalculation of the item height.

	if (!keep_viewport || selected_ >= items.size()) {
		selected_ = 0;
	}

	fill_items(items, strip_spaces);
	if(!keep_viewport) {
		set_position(0);
	} else if(scrolled_to_max) {
		set_position(get_max_position());
	}

	update_scrollbar_grip_height();

	if(!keep_viewport) {
		adjust_viewport_to_selection();
	}
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
	column_widths_.clear();
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
		) - heading_height();

	std::vector<int> heights;
	std::size_t n;
	for(n = 0; n != items_.size(); ++n) {
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


		if(sorter_ != nullptr) {
			const int heading = hit_heading(x,y);
			if(heading >= 0 && sorter_->column_sortable(heading)) {
				sort_by(heading);
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

		const int heading_item = hit_heading(event.motion.x,event.motion.y);
		if(heading_item != highlight_heading_) {
			highlight_heading_ = heading_item;
			invalidate_heading();
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

void menu::set_sorter(sorter *s)
{
	if(sortby_ >= 0) {
		//clear an existing sort
		sort_by(-1);
	}
	sorter_ = s;
	sortreversed_ = false;
	sortby_ = -1;
}

void menu::sort_by(int column)
{
	const bool already_sorted = (column == sortby_);

	if(already_sorted) {
		if(sortreversed_ == false) {
			sortreversed_ = true;
		} else {
			sortreversed_ = false;
			sortby_ = -1;
		}
	} else {
		sortby_ = column;
		sortreversed_ = false;
	}

	do_sort();
	itemRects_.clear();
	queue_redraw();
}

SDL_Rect menu::style::item_size(const std::string& item) const {
	SDL_Rect res {0,0,0,0};
	std::vector<std::string> img_text_items = utils::split(item, IMG_TEXT_SEPARATOR, utils::REMOVE_EMPTY);
	for (std::vector<std::string>::const_iterator it = img_text_items.begin();
		 it != img_text_items.end(); ++it) {
		if (res.w > 0 || res.h > 0) {
			// Not the first item, add the spacing.
			res.w += 5;
		}
		const std::string str = *it;
		if (!str.empty() && str[0] == IMAGE_PREFIX) {
			const std::string image_name(str.begin()+1,str.end());
			const point image_size = image::get_size(image_name);
			if (image_size.x && image_size.y) {
				int w = image_size.x;
				int h = image_size.y;
				adjust_image_bounds(w, h);
				res.w += w;
				res.h = std::max<int>(h, res.h);
			}
		}
		else {
			const SDL_Rect area {0,0,10000,10000};
			const SDL_Rect font_size =
				font::pango_draw_text(false, area, get_font_size(),
					font::NORMAL_COLOR, str, 0, 0);
			res.w += font_size.w;
			res.h = std::max<int>(font_size.h, res.h);
		}
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
	case HEADING_ROW:
		rgb = heading_rgb_;
		alpha = heading_alpha_;
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
	if(type != HEADING_ROW) {
		minirect.x += thickness_;
		minirect.y += thickness_;
		minirect.w -= 2*thickness_;
		minirect.h -= 2*thickness_;
	}
	menu_ref.draw_row(row_index, minirect, type);
}



void menu::column_widths_item(const std::vector<std::string>& row, std::vector<int>& widths) const
{
	for(std::size_t col = 0; col != row.size(); ++col) {
		const SDL_Rect res = style_->item_size(row[col]);
		std::size_t text_trailing_space = (item_ends_with_image(row[col])) ? 0 : style_->get_cell_padding();

		if(col == widths.size()) {
			widths.push_back(res.w + text_trailing_space);
		} else if(static_cast<std::size_t>(res.w) > widths[col] - text_trailing_space) {
			widths[col] = res.w + text_trailing_space;
		}
	}
}

bool menu::item_ends_with_image(const std::string& item) const
{
	std::string::size_type pos = item.find_last_of(IMG_TEXT_SEPARATOR);
	pos = (pos == std::string::npos) ? 0 : pos+1;
	return(item.size() > pos && item.at(pos) == IMAGE_PREFIX);
}

const std::vector<int>& menu::column_widths() const
{
	if(column_widths_.empty()) {
		column_widths_item(heading_,column_widths_);
		for(std::size_t row = 0; row != items_.size(); ++row) {
			column_widths_item(items_[row].fields,column_widths_);
		}
	}

	return column_widths_;
}

void menu::clear_item(int item)
{
	SDL_Rect rect = get_item_rect(item);
	if (rect.w == 0)
		return;
	queue_redraw(rect);
}

void menu::draw_row(const std::size_t row_index, const SDL_Rect& loc, ROW_TYPE type)
{
	//called from style, draws one row's contents in a generic and adaptable way
	const std::vector<std::string>& row = (type == HEADING_ROW) ? heading_ : items_[row_index].fields;
	rect area = video::game_canvas();
	rect column = inner_location();
	const std::vector<int>& widths = column_widths();
	bool lang_rtl = current_language_rtl();
	int dir = (lang_rtl) ? -1 : 1;

	int xpos = loc.x;
	if(lang_rtl) {
		xpos += loc.w;
	}

	for(std::size_t i = 0; i != row.size(); ++i) {

		if(lang_rtl) {
			xpos -= widths[i];
		}
		if(type == HEADING_ROW) {
			rect draw_rect {xpos, loc.y, widths[i], loc.h };

			if(highlight_heading_ == int(i)) {
				draw::fill(draw_rect, {255,255,255,77});
			} else if(sortby_ == int(i)) {
				draw::fill(draw_rect, {255,255,255,26});
			}
		}

		const int last_x = xpos;
		column.w = widths[i];
		std::string str = row[i];
		std::vector<std::string> img_text_items = utils::split(str, IMG_TEXT_SEPARATOR, utils::REMOVE_EMPTY);
		for (std::vector<std::string>::const_iterator it = img_text_items.begin();
			 it != img_text_items.end(); ++it) {
			str = *it;
			if (!str.empty() && str[0] == IMAGE_PREFIX) {
				const std::string image_name(str.begin()+1,str.end());
				const texture img = image::get_texture(image_name);
				int img_w = img.w();
				int img_h = img.h();
				style_->adjust_image_bounds(img_w, img_h);
				const int remaining_width = max_width_ < 0 ? area.w :
				std::min<int>(max_width_, ((lang_rtl)? xpos - loc.x : loc.x + loc.w - xpos));
				if(img && img_w <= remaining_width
				&& loc.y + img_h < area.h) {
					const std::size_t y = loc.y + (loc.h - img_h)/2;
					const std::size_t w = img_w + 5;
					const std::size_t x = xpos + ((lang_rtl) ? widths[i] - w : 0);
					draw::blit(img, {int(x), int(y), img_w, img_h});
					if(!lang_rtl)
						xpos += w;
					column.w -= w;
				}
			} else {
				column.x = xpos;

				const auto text_size = font::pango_line_size(str, style_->get_font_size());
				const std::size_t y = loc.y + (loc.h - text_size.second)/2;
				const std::size_t padding = 2;
				rect text_loc = column;
				text_loc.w = loc.w - (xpos - loc.x) - 2 * style_->get_thickness();
				text_loc.h = text_size.second;
				font::pango_draw_text(true, text_loc, style_->get_font_size(), font::NORMAL_COLOR, str,
					(type == HEADING_ROW ? xpos+padding : xpos), y);

				if(type == HEADING_ROW && sortby_ == int(i)) {
					const texture sort_tex(image::get_texture(sortreversed_ ? "buttons/sliders/slider_arrow_blue.png" :
					                                   "buttons/sliders/slider_arrow_blue.png~ROTATE(180)"));
					if(sort_tex && sort_tex.w() <= widths[i] && sort_tex.h() <= loc.h) {
						const int sort_x = xpos + widths[i] - sort_tex.w() - padding;
						const int sort_y = loc.y + loc.h/2 - sort_tex.h()/2;
						SDL_Rect dest = {sort_x, sort_y, sort_tex.w(), sort_tex.h()};
						draw::blit(sort_tex, dest);
					}
				}

				xpos += dir * (text_size.first + 5);
			}
		}
		if(lang_rtl)
			xpos = last_x;
		else
			xpos = last_x + widths[i];
	}
}

void menu::draw_contents()
{
	SDL_Rect heading_rect = inner_location();
	heading_rect.h = heading_height();
	style_->draw_row(*this,0,heading_rect,HEADING_ROW);

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

int menu::hit_column(int x) const
{
	const std::vector<int>& widths = column_widths();
	int j = -1, j_end = widths.size();
	for(x -= location().x; x >= 0; x -= widths[j]) {
		if(++j == j_end) {
			return -1;
		}
	}
	return j;
}

int menu::hit_heading(int x, int y) const
{
	const std::size_t height = heading_height();
	const SDL_Rect& loc = inner_location();
	if(y >= loc.y && static_cast<std::size_t>(y) < loc.y + height) {
		return hit_column(x);
	} else {
		return -1;
	}
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

	int y = loc.y + heading_height();
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

std::size_t menu::get_item_height_internal(const std::vector<std::string>& item) const
{
	std::size_t res = 0;
	for(std::vector<std::string>::const_iterator i = item.begin(); i != item.end(); ++i) {
		SDL_Rect rect = style_->item_size(*i);
		res = std::max<int>(rect.h,res);
	}

	return res;
}

std::size_t menu::heading_height() const
{
	if(heading_height_ == -1) {
		heading_height_ = int(get_item_height_internal(heading_));
	}

	return std::min<unsigned int>(heading_height_,max_height_);
}

std::size_t menu::get_item_height(int) const
{
	if(item_height_ != -1)
		return std::size_t(item_height_);

	std::size_t max_height = 0;
	for(std::size_t n = 0; n != items_.size(); ++n) {
		max_height = std::max<int>(max_height,get_item_height_internal(items_[n].fields));
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

void menu::invalidate_heading()
{
	rect heading_rect = inner_location();
	heading_rect.h = heading_height();
	queue_redraw(heading_rect);
}

}
