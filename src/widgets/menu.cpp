//disable the very annoying VC++ warning 4786
#ifdef WIN32
#pragma warning(disable:4786)
#endif

#include "menu.hpp"

#include "../font.hpp"
#include "../sdl_utils.hpp"
#include "../show_dialog.hpp"
#include "../util.hpp"
#include "../video.hpp"

#include <numeric>

namespace {
const size_t menu_font_size = font::SIZE_NORMAL;
const size_t menu_cell_padding = font::SIZE_NORMAL * 3/5;
}

namespace gui {

menu::menu(display& disp, const std::vector<std::string>& items,
           bool click_selects, int max_height, int max_width)
        : max_height_(max_height), max_width_(max_width), max_items_(-1), item_height_(-1),
	  display_(&disp), x_(0), y_(0), cur_help_(-1,-1), help_string_(-1), buffer_(NULL),
	  selected_(0), click_selects_(click_selects),
	  previous_button_(true), drawn_(false), show_result_(false),
	  height_(-1), width_(-1), first_item_on_screen_(0),
	  uparrow_(disp,"",gui::button::TYPE_PRESS,"uparrow-button"),
	  downarrow_(disp,"",gui::button::TYPE_PRESS,"downarrow-button"),
	  double_clicked_(false),
	  scrollbar_(disp,this),
	  num_selects_(true),
	  ignore_next_doubleclick_(false),
	  last_was_doubleclick_(false)
{
	fill_items(items, true);
}

void menu::fill_items(const std::vector<std::string>& items, bool strip_spaces)
{
	for(std::vector<std::string>::const_iterator item = items.begin();
	    item != items.end(); ++item) {
		items_.push_back(config::quoted_split(*item, ',', !strip_spaces));

		//make sure there is always at least one item
		if(items_.back().empty())
			items_.back().push_back(" ");

		//if the first character in an item is an asterisk,
		//it means this item should be selected by default
		std::string& first_item = items_.back().front();
		if(first_item.empty() == false && first_item[0] == DEFAULT_ITEM) {
			selected_ = items_.size()-1;
			first_item.erase(first_item.begin());
		}
	}

	create_help_strings();
}

void menu::create_help_strings()
{
	help_.clear();
	for(std::vector<std::vector<std::string> >::iterator i = items_.begin(); i != items_.end(); ++i) {
		help_.resize(help_.size()+1);
		for(std::vector<std::string>::iterator j = i->begin(); j != i->end(); ++j) {
			if(std::find(j->begin(),j->end(),static_cast<char>(HELP_STRING_SEPERATOR)) == j->end()) {
				help_.back().push_back("");
			} else {
				const std::vector<std::string>& items = config::split(*j,HELP_STRING_SEPERATOR,0);
				if(items.size() >= 2) {
					*j = items.front();
					help_.back().push_back(items.back());
				} else {
					help_.back().push_back("");
				}
			}
		}
	}
}

void menu::update_scrollbar_grip_height()
{
	int h = scrollbar_.height();
	size_t nb_items = items_.size(), max_items = max_items_onscreen();
	int new_height = nb_items > max_items  ? h * max_items / nb_items : h;
	int min_height = scrollbar_.get_minimum_grip_height();

	if (new_height < min_height) 
		new_height = min_height;
	scrollbar_.set_grip_height(new_height);
}

int menu::update_scrollbar_position()
{
	size_t nb_items = items_.size(), max_items = max_items_onscreen();
	int new_scrollpos;
	if (nb_items > max_items) {
		int max_scroll_position = scrollbar_.height() - scrollbar_.get_grip_height();
		size_t last_top_idx = nb_items - max_items;
		new_scrollpos = (first_item_on_screen_ * max_scroll_position + last_top_idx - 1) / last_top_idx;
	} else
		new_scrollpos = 0;
	scrollbar_.set_grip_position(new_scrollpos);
	return new_scrollpos;
}

int menu::height() const
{
	if(height_ == -1) {
		height_ = 0;
		for(size_t i = first_item_on_screen_; i != items_.size()
				&& i != first_item_on_screen_ + max_items_onscreen(); ++i) {
			height_ += get_item_rect(i).h;
		}
	}
	return height_;
}

int menu::width() const
{
	if(width_ == -1) {
		const std::vector<int>& widths = column_widths();
		width_ = std::accumulate(widths.begin(),widths.end(),0);
		if(show_scrollbar()) {
			width_ += scrollbar_.get_max_width();
		}
	}
	return width_;
}

int menu::item_area_width() const {
	return width() - scrollbar_.get_width();
}

int menu::selection() const { return selected_; }

void menu::set_loc(int x, int y)
{
	x_ = x;
	y_ = y;

	const int w = width();

	SDL_Rect portion = {x_,y_,w,height()};
	surface const screen = display_->video().getSurface();
	buffer_.assign(get_surface_portion(screen, portion));

	if(show_scrollbar()) {
		const int menu_width = width() - scrollbar_.get_max_width();

		scrollbar_.enable(true);
		int scr_width = scrollbar_.get_width();
		
		SDL_Rect scroll_rect = {x_ + menu_width, y_+uparrow_.height(),
								scr_width, 
								height()-downarrow_.height()-uparrow_.height()};
		scrollbar_.set_location(scroll_rect);
		update_scrollbar_grip_height();

		uparrow_.set_location(x_ + menu_width,y_);
		downarrow_.set_location(x_+ menu_width,scrollbar_.location().y + scrollbar_.location().h);

	}
}

void menu::set_width(int w)
{
	width_ = w;
	set_loc(x_, y_);
	itemRects_.clear();
}

SDL_Rect menu::get_rect() const
{
	SDL_Rect r = {x_, y_, max_width_ < 0 ? width_ : max_width_,
				  max_height_ < 0 ? height_ : max_height_};
	return r;
}

void menu::redraw()
{
	if(x_ == 0 && y_ == 0) {
		return;
	}

	draw();
	uparrow_.draw();
	downarrow_.draw();
	scrollbar_.redraw();
}

void menu::change_item(int pos1, int pos2,std::string str)
{
	items_[pos1][pos2] = str;
	undrawn_items_.insert(pos1);
}

void menu::erase_item(size_t index)
{
	size_t nb_items = items_.size();
	if (index >= nb_items)
		return;

	clear_item(nb_items - 1);
	items_.erase(items_.begin() + index);
	if (selected_ >= nb_items - 1)
		selected_ = nb_items - 2;

	update_scrollbar_grip_height();
	adjust_viewport_to_selection();
	update_scrollbar_position();
	itemRects_.clear();
	drawn_ = false;
}

void menu::set_items(const std::vector<std::string>& items, bool strip_spaces,
					 bool keep_viewport) {
	items_.clear();
	itemRects_.clear();
	column_widths_.clear();
	undrawn_items_.clear();
	height_ = -1; // Force recalculation of the height.
	width_ = -1; // Force recalculation of the width.
	max_items_ = -1; // Force recalculation of the max items.
	item_height_ = -1; // Force recalculation of the item height.
	// Scrollbar and buttons will be reenabled if they are needed.
	scrollbar_.enable(false);
	uparrow_.hide(true);
	downarrow_.hide(true);
	selected_ = 0;
	fill_items(items, strip_spaces);
	if (!keep_viewport)
		first_item_on_screen_ = 0;
	set_loc(x_, y_); // Force some more updating.
	adjust_viewport_to_selection();
	update_scrollbar_position();
	drawn_ = false;
}

void menu::set_max_height(const int new_max_height) {
	max_height_ = new_max_height;
}

void menu::set_max_width(const int new_max_width) {
	max_width_ = new_max_width;
}

size_t menu::max_items_onscreen() const
{
	if(max_items_ != -1) {
		return size_t(max_items_);
	}

	const size_t max_height = max_height_ == -1 ? (display_->y()*66)/100 : max_height_;
	std::vector<int> heights;
	size_t n;
	for(n = 0; n != items_.size(); ++n) {
		heights.push_back(get_item_height(n));
	}

	std::sort(heights.begin(),heights.end(),std::greater<int>());
	size_t sum = 0;
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

	size_t nb_items = items_.size(), max_items = max_items_onscreen();
	size_t new_first_item = first_item_on_screen_;

	if (selected_ < first_item_on_screen_)
		new_first_item = selected_;
	else if (selected_ >= first_item_on_screen_ + max_items)
		new_first_item = selected_ - (max_items - 1);
	if (nb_items <= max_items)
		new_first_item = 0;
	else if (new_first_item > nb_items - max_items)
		new_first_item = nb_items - max_items;

	move_viewport(new_first_item);
}

void menu::move_selection_up(size_t dep)
{
	move_selection(selected_ > dep ? selected_ - dep : 0);
}

void menu::move_selection_down(size_t dep)
{
	size_t nb_items = items_.size();
	move_selection(selected_ + dep >= nb_items ? nb_items - 1 : selected_ + dep);
}

void menu::move_selection(size_t new_selected)
{
	if (new_selected == selected_ || new_selected >= items_.size())
		return;

	undrawn_items_.insert(selected_);
	undrawn_items_.insert(new_selected);
	selected_ = new_selected;
	adjust_viewport_to_selection();
}

void menu::move_viewport_up(size_t dep)
{
	move_viewport(first_item_on_screen_ > dep ? first_item_on_screen_ - dep : 0);
}

void menu::move_viewport_down(size_t dep)
{
	size_t nb_items = items_.size(), max_items = max_items_onscreen();
	if (nb_items <= max_items)
		return;
	size_t last_top_idx = nb_items - max_items;
	move_viewport(first_item_on_screen_ + dep >= last_top_idx ? last_top_idx : first_item_on_screen_ + dep);
}

void menu::move_viewport(size_t new_first_item)
{
	if (new_first_item == first_item_on_screen_)
		return;

	first_item_on_screen_ = new_first_item;
	update_scrollbar_position();
	itemRects_.clear();
	drawn_ = false;
}

void menu::key_press(SDLKey key)
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
		default:
			break;
		}
	}

	if (num_selects_ && key >= SDLK_1 && key <= SDLK_9)
		move_selection(key - SDLK_1);
}

void menu::handle_event(const SDL_Event& event)
{
	if(event.type == SDL_KEYDOWN) {
		key_press(event.key.keysym.sym);
	} else if(event.type == SDL_MOUSEBUTTONDOWN &&
	          event.button.button == SDL_BUTTON_LEFT ||
			  event.type == DOUBLE_CLICK_EVENT) {
		int x = 0;
		int y = 0;
		if(event.type == SDL_MOUSEBUTTONDOWN) {
			x = event.button.x;
			y = event.button.y;
		} else {
			x = (int)event.user.data1;
			y = (int)event.user.data2;
		}

		const int item = hit(x,y);
		if(item != -1) {
			move_selection(item);

			if(click_selects_) {
				show_result_ = true;
			}

			if(event.type == DOUBLE_CLICK_EVENT) {
				if (ignore_next_doubleclick_) {
					ignore_next_doubleclick_ = false;
				}
				else {
					double_clicked_ = true;
					last_was_doubleclick_ = true;
				}
			}
			else if (last_was_doubleclick_) {
				// If we have a double click as the next event, it means
				// this double click was generated from a click that
				// already has helped in generating a double click.
				SDL_Event ev;
				SDL_PeepEvents(&ev, 1, SDL_PEEKEVENT,
							   SDL_EVENTMASK(DOUBLE_CLICK_EVENT));
				if (ev.type == DOUBLE_CLICK_EVENT) {
					ignore_next_doubleclick_ = true;
				}
				last_was_doubleclick_ = false;
			}
		}
	} else if(event.type == SDL_MOUSEMOTION && click_selects_) { 
		const int item = hit(event.motion.x,event.motion.y);
		if (item != -1)
			move_selection(item);
	} else if(event.type == SDL_MOUSEBUTTONDOWN 
			&& event.button.button == SDL_BUTTON_WHEELDOWN) {
		move_viewport_down(1);
	} else if(event.type == SDL_MOUSEBUTTONDOWN
			&& event.button.button == SDL_BUTTON_WHEELUP) {
		move_viewport_up(1);
	}
}

int menu::process(int x, int y, bool button, bool, bool, bool, bool, int)
{
	int max_scroll_position = scrollbar_.height() - scrollbar_.get_grip_height();
	size_t nb_items = items_.size(), max_items = max_items_onscreen();
	if (nb_items > max_items) {
		if (uparrow_.process(x, y, button))
			move_viewport_up(1);
		if (downarrow_.process(x, y, button))
			move_viewport_down(1);

		scrollbar_.process();
		int scroll_position = scrollbar_.get_grip_position();
		size_t last_top_idx = nb_items - max_items;
		size_t new_first_item = (scroll_position * last_top_idx) / max_scroll_position;
		move_viewport(new_first_item >= last_top_idx ? last_top_idx : new_first_item);
		int groove = scrollbar_.groove_clicked();
		if (groove == -1)
			move_viewport_up(max_items);
		else if (groove == 1)
			move_viewport_down(max_items);
	}

	// update enabled/disabled status for up/down buttons and the scrollbar
	bool enable_scrollbar = nb_items > max_items;
	scrollbar_.enable(enable_scrollbar);
	uparrow_.hide(!enable_scrollbar);
	downarrow_.hide(!enable_scrollbar);
	if (enable_scrollbar) {
		uparrow_.enable(first_item_on_screen_ != 0);
		downarrow_.enable(first_item_on_screen_ < nb_items - max_items);
	}

	if (!drawn_)
		draw();

	if(show_result_) {
		show_result_ = false;
		return selected_;
	} else {
		return -1;
	}
}

bool menu::show_scrollbar() const
{
	return items_.size() > max_items_onscreen();
}

bool menu::double_clicked()
{
	bool old = double_clicked_;
	double_clicked_ = false;
	return old;
}

void menu::set_numeric_keypress_selection(bool value)
{
	num_selects_ = value;
}

void menu::scroll(int)
{
}

namespace {
	const char ImagePrefix = '&';

	SDL_Rect item_size(const std::string& item) {
		SDL_Rect res = {0,0,0,0};
		std::vector<std::string> img_text_items = config::split(item, menu::IMG_TEXT_SEPERATOR);
		for (std::vector<std::string>::const_iterator it = img_text_items.begin();
			 it != img_text_items.end(); it++) {
			if (res.w > 0 || res.h > 0) {
				// Not the first item, add the spacing.
				res.w += 5;
			}
			const std::string str = *it;
			if(str.empty() == false && str[0] == ImagePrefix) {
				const std::string image_name(str.begin()+1,str.end());
				surface const img = image::get_image(image_name,image::UNSCALED);
				if(img != NULL) {
					res.w += img->w;
					res.h = maximum<int>(img->h, res.h);
				}
			} else {
				const SDL_Rect area = {0,0,10000,10000};
				const SDL_Rect font_size =
					font::draw_text(NULL,area,menu_font_size,font::NORMAL_COLOUR,str,0,0);
				res.w += font_size.w;
				res.h = maximum<int>(font_size.h, res.h);
			}
		}
		return res;
	}
}

const std::vector<int>& menu::column_widths() const
{
	if(column_widths_.empty()) {
		for(size_t row = 0; row != items_.size(); ++row) {
			for(size_t col = 0; col != items_[row].size(); ++col) {
				const SDL_Rect res = item_size(items_[row][col]);

				if(col == column_widths_.size()) {
					column_widths_.push_back(res.w + menu_cell_padding);
				} else if(res.w > column_widths_[col] - menu_cell_padding) {
					column_widths_[col] = res.w + menu_cell_padding;
				}
			}
		}
	}

	return column_widths_;
}

void menu::clear_item(int item)
{
	SDL_Rect rect = get_item_rect(item);
	if(rect.w == 0) {
		return;
	}

	if(buffer_.get() != NULL) {
		const int ypos = (item-first_item_on_screen_)*rect.h;
		SDL_Rect srcrect = {0,ypos,rect.w,rect.h};
		SDL_Rect dstrect = rect;
		SDL_BlitSurface(buffer_,&srcrect,
		                display_->video().getSurface(),&dstrect);
	}
}

void menu::draw_item(int item)
{
	SDL_Rect rect = get_item_rect(item);
	if(rect.w == 0) {
		return;
	}

	clear_item(item);
	gui::draw_solid_tinted_rectangle(x_,rect.y,width()-scrollbar_.get_width(),rect.h,
	                                 item == selected_ ? 150:0,0,0,
	                                 item == selected_ ? 0.6 : 0.2,
	                                 display_->video().getSurface());

	SDL_Rect area = display_->screen_area();

	const std::vector<int>& widths = column_widths();

	int xpos = rect.x;
	for(size_t i = 0; i != items_[item].size(); ++i) {
		const int last_x = xpos;
		std::string str = items_[item][i];
		std::vector<std::string> img_text_items = config::split(str, IMG_TEXT_SEPERATOR);
		for (std::vector<std::string>::const_iterator it = img_text_items.begin();
			 it != img_text_items.end(); it++) {
			str = *it;
			if(str.empty() == false && str[0] == ImagePrefix) {
				const std::string image_name(str.begin()+1,str.end());
				surface const img = image::get_image(image_name,image::UNSCALED);
				const int max_width = max_width_ < 0 ? display_->x() :
					minimum<int>(max_width_, display_->x() - xpos);
				if(img != NULL && (xpos - rect.x) + img->w < max_width
				   && rect.y+img->h < display_->y()) {
					const size_t y = rect.y + (rect.h - img->h)/2;
					display_->blit_surface(xpos,y,img);
					xpos += img->w + 5;
				}
			} else {
				const std::string to_show = max_width_ > -1 ? 
					font::make_text_ellipsis(str, menu_font_size,
											 max_width_ - (xpos - rect.x)
											 - scrollbar_.get_width()) : str;
				const SDL_Rect& text_size = font::text_area(str,menu_font_size);
				const size_t y = rect.y + (rect.h - text_size.h)/2;
				font::draw_text(display_,area,menu_font_size,font::NORMAL_COLOUR,to_show,xpos,y);
				xpos += text_size.w + 5;
			}
		}
		xpos = last_x + widths[i];
	}
}

void menu::draw()
{
	if(x_ == 0 && y_ == 0 || drawn_ && undrawn_items_.empty()) {
		return;
	}

	if(drawn_) {
		for(std::set<size_t>::const_iterator i = undrawn_items_.begin(); i != undrawn_items_.end(); ++i) {
			if(*i < items_.size()) {
				draw_item(*i);
				update_rect(get_item_rect(*i));
			}
		}

		undrawn_items_.clear();
		return;
	}

	undrawn_items_.clear();
	drawn_ = true;

	for(size_t i = 0; i != items_.size(); ++i)
		draw_item(i);

	update_rect(x_,y_,width(),height());
}

int menu::hit(int x, int y) const
{
	if(x > x_  && x < x_ + width() - scrollbar_.get_width() && 
	   y > y_ && y < y_ + height()) {
		for(size_t i = 0; i != items_.size(); ++i) {
			const SDL_Rect& rect = get_item_rect(i);
			if(y > rect.y && y < rect.y + rect.h)
				return i;
		}
	}

	return -1;
}

std::pair<int,int> menu::hit_cell(int x, int y) const
{
	if(x > x_  && x < x_ + width() - scrollbar_.get_width() && 
	   y > y_ && y < y_ + height()) {
		for(size_t i = 0; i != items_.size(); ++i) {
			const SDL_Rect& rect = get_item_rect(i);
			if(y > rect.y && y < rect.y + rect.h) {
				const std::vector<int>& widths = column_widths();
				for(std::vector<int>::const_iterator w = widths.begin(); w != widths.end(); ++w) {
					x -= *w;
					if(x <= x_) {
						return std::pair<int,int>(int(i),int(w-widths.begin()));
					}
				}
			}
		}
	}

	return std::pair<int,int>(-1,-1);
}

SDL_Rect menu::get_item_rect(int item) const
{
	const SDL_Rect empty_rect = {0,0,0,0};
	if(item < first_item_on_screen_ ||
	   size_t(item) >= first_item_on_screen_ + max_items_onscreen()) {
		return empty_rect;
	}

	const std::map<int,SDL_Rect>::const_iterator i = itemRects_.find(item);
	if(i != itemRects_.end())
		return i->second;

	int y = y_;
	if(item != first_item_on_screen_) {
		const SDL_Rect& prev = get_item_rect(item-1);
		y = prev.y + prev.h;
	}

	const SDL_Rect screen_area = display_->screen_area();

	SDL_Rect res = {x_, y,
		            width() - scrollbar_.get_width(),
					get_item_height(item)};

	if(res.x > screen_area.w) {
		return empty_rect;
	} else if(res.x + res.w > screen_area.w) {
		res.w = screen_area.w - res.x;
	}

	if(res.y > screen_area.h) {
		return empty_rect;
	} else if(res.y + res.h > screen_area.h) {
		res.h = screen_area.h - res.y;
	}

	//only insert into the cache if the menu's co-ordinates have
	//been initialized
	if(x_ > 0 && y_ > 0)
		itemRects_.insert(std::pair<int,SDL_Rect>(item,res));

	return res;
}

size_t menu::get_item_height_internal(int item) const
{
	size_t res = 0;
	for(size_t n = 0; n != items_[item].size(); ++n) {
		SDL_Rect rect = item_size(items_[item][n]);
		res = maximum<int>(rect.h,res);
	}

	return res;
}

size_t menu::get_item_height(int) const
{
	if(item_height_ != -1)
		return size_t(item_height_);

	size_t max_height = 0;
	for(size_t n = 0; n != items_.size(); ++n) {
		max_height = maximum<int>(max_height,get_item_height_internal(n));
	}

	return item_height_ = max_height;
}

void menu::process_help_string(int mousex, int mousey)
{
	const std::pair<int,int> loc = hit_cell(mousex,mousey);
	if(loc == cur_help_) {
		return;
	} else if(loc.first == -1) {
		display_->clear_help_string(help_string_);
		help_string_ = -1;
	} else {
		if(help_string_ != -1) {
			display_->clear_help_string(help_string_);
			help_string_ = -1;
		}

		if(size_t(loc.first) < help_.size()) {
			const std::vector<std::string>& row = help_[loc.first];
			if(size_t(loc.second) < row.size()) {
				const std::string& help = row[loc.second];
				if(help.empty() == false) {
					//std::cerr << "setting help string from menu to '" << help << "'\n";
					help_string_ = display_->set_help_string(help);
				}
			}
		}
	}

	cur_help_ = loc;
}

}
