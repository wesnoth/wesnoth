#include "menu.hpp"

#include "../font.hpp"
#include "../sdl_utils.hpp"
#include "../show_dialog.hpp"
#include "../util.hpp"
#include "../video.hpp"

#include <numeric>

namespace {
const size_t menu_font_size = 14;
const size_t menu_cell_padding = 10;
}

namespace gui {

menu::menu(display& disp, const std::vector<std::string>& items,
           bool click_selects, int max_height)
        : max_height_(max_height), max_items_(-1), item_height_(-1),
		  display_(&disp), x_(0), y_(0), buffer_(NULL),
          selected_(click_selects ? -1:0), click_selects_(click_selects),
          previous_button_(true), drawn_(false), show_result_(false),
          height_(-1), width_(-1), first_item_on_screen_(0),
		  uparrow_(disp,"",gui::button::TYPE_PRESS,"uparrow-button"),
          downarrow_(disp,"",gui::button::TYPE_PRESS,"downarrow-button"),
		  scrollbar_(disp,this),  scrollbar_height_(0),
		  double_clicked_(false), num_selects_(true),
		  ignore_next_doubleclick_(false),
		  last_was_doubleclick_(false)
{
	for(std::vector<std::string>::const_iterator item = items.begin();
	    item != items.end(); ++item) {
		items_.push_back(config::quoted_split(*item,',',false));

		//make sure there is always at least one item
		if(items_.back().empty())
			items_.back().push_back(" ");

		//if the first character in an item is an asterisk,
		//it means this item should be selected by default
		std::string& first_item = items_.back().front();
		if(first_item.empty() == false && first_item[0] == '*') {
			selected_ = items_.size()-1;
			first_item.erase(first_item.begin());
		}
	}
}

// The scrollbar height depends on the number of visible items versus
void menu::set_scrollbar_height()
{
	int buttons_height = downarrow_.height() + uparrow_.height();
	float pos_percent = (float)max_items_onscreen()/(float)items_.size();
	scrollbar_height_ = (int)(pos_percent * (height()-buttons_height));

	int min_height = scrollbar_.get_minimum_grip_height();

	if (scrollbar_height_ < min_height) 
		scrollbar_height_ = min_height;
	if (scrollbar_height_ > height()) {
		std::cerr << "Strange. For some reason I want my scrollbar" << 
			      " to be larger than me!\n\n";
		std::cerr << "pos_percent=" << pos_percent << " height()=" << height()
				  << std::endl;
		scrollbar_height_ = height() - buttons_height;
	}
					
	scrollbar_.set_grip_height(scrollbar_height_);
}

int menu::height() const
{
	if(height_ == -1) {
		height_ = 0;
		for(size_t i = 0; i != items_.size() && i != max_items_onscreen(); ++i) {
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

int menu::selection() const { return selected_; }

void menu::set_loc(int x, int y)
{
	x_ = x;
	y_ = y;

	const int w = width();

	SDL_Rect portion = {x_,y_,w,height()};
	SDL_Surface* const screen = display_->video().getSurface();
	buffer_.assign(get_surface_portion(screen, portion));

	if(show_scrollbar()) {
		const int menu_width = width() - scrollbar_.get_max_width();

		scrollbar_.enable(true);
		int scr_width = scrollbar_.get_width();
		
		SDL_Rect scroll_rect = {x_ + menu_width, y_+uparrow_.height(),
								scr_width, 
								height()-downarrow_.height()-uparrow_.height()};
		scrollbar_.set_location(scroll_rect);
		set_scrollbar_height();

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
	if(index < items_.size()) {
		clear_item(items_.size()-1);
		items_.erase(items_.begin() + index);
		itemRects_.clear();
		if(size_t(selected_) >= items_.size()) {
			selected_ = int(items_.size()-1);
		}

		calculate_position();

		drawn_ = false;
	}
}

void menu::set_items(const std::vector<std::string>& items) {
	items_.clear();
	itemRects_.clear();
	column_widths_.clear();
	undrawn_items_.clear();
	height_ = -1; // Force recalculation of the height.
	width_ = -1; // Force recalculation of the width.
	max_items_ = -1; // Force recalculation of the max items.
	// Scrollbar and buttons will be reanabled if they are needed.
	scrollbar_.enable(false);
	uparrow_.hide(true);
	downarrow_.hide(true);
	first_item_on_screen_ = 0;
	selected_ = click_selects_ ? -1:0;
	for (std::vector<std::string>::const_iterator item = items.begin();
		 item != items.end(); ++item) {
		items_.push_back(config::quoted_split(*item,',',false));

		//make sure there is always at least one item
		if(items_.back().empty())
			items_.back().push_back(" ");

		//if the first character in an item is an asterisk,
		//it means this item should be selected by default
		std::string& first_item = items_.back().front();
		if(first_item.empty() == false && first_item[0] == '*') {
			selected_ = items_.size()-1;
			first_item.erase(first_item.begin());
		}
	}
	set_loc(x_, y_); // Force some more updating.
	calculate_position();
	drawn_ = false;
}

void menu::set_max_height(const int new_max_height) {
	max_height_ = new_max_height;
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

void menu::calculate_position()
{
	if(click_selects_)
		return;

	if(selected_ < first_item_on_screen_) {
		first_item_on_screen_ = selected_;
		itemRects_.clear();
		drawn_ = false;
	}
	
	if(selected_ >= first_item_on_screen_ + int(max_items_onscreen())) {
		first_item_on_screen_ = selected_ - (max_items_onscreen() - 1);
		itemRects_.clear();
		drawn_ = false;
	}
}

void menu::key_press(SDLKey key)
{
	switch(key) {
		case SDLK_UP: {
			if(!click_selects_ && selected_ > 0) {
				--selected_;
				calculate_position();
				undrawn_items_.insert(selected_);
				undrawn_items_.insert(selected_+1);
			}

			break;
		}

		case SDLK_DOWN: {
			if(!click_selects_ && selected_ < int(items_.size())-1) {
				++selected_;
				calculate_position();
				undrawn_items_.insert(selected_);
				undrawn_items_.insert(selected_-1);
			}

			break;
		}

		case SDLK_PAGEUP: {
			if(!click_selects_) {
				selected_ -= max_items_onscreen();
				if(selected_ < 0)
					selected_ = 0;
				
				calculate_position();
				drawn_ = false;
			}

			break;
		}

		case SDLK_PAGEDOWN: {
			if(!click_selects_) {
				selected_ += max_items_onscreen();
				if(selected_ >= int(items_.size()))
					selected_ = int(items_.size())-1;

				calculate_position();
				drawn_ = false;
			}

			break;
		}

		default:
			break;
	}

	if(key >= SDLK_1 && key <= SDLK_9 && num_selects_) {
		const int pos = key - SDLK_1;
		if(size_t(pos) < items_.size()) {
			undrawn_items_.insert(selected_);
			selected_ = pos;
			calculate_position();
			undrawn_items_.insert(selected_);

		}
	}
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
			undrawn_items_.insert(selected_);
			selected_ = item;
			undrawn_items_.insert(selected_);

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
		if(item != selected_) {
			selected_ = item;
			drawn_ = false;
		}	
	} else if(event.type == SDL_MOUSEBUTTONDOWN 
			&& event.button.button == SDL_BUTTON_WHEELDOWN) {
			if(first_item_on_screen_+max_items_onscreen()<items_.size()) {
				++first_item_on_screen_;
				if (selected_<first_item_on_screen_)
					selected_=first_item_on_screen_;
				itemRects_.clear();
				drawn_ = false;
			}
	} else if(event.type ==	SDL_MOUSEBUTTONDOWN
			&& event.button.button == SDL_BUTTON_WHEELUP) {
			if(first_item_on_screen_>0) {
				--first_item_on_screen_;
				if (selected_>=first_item_on_screen_+int(max_items_onscreen()))
					selected_=first_item_on_screen_+max_items_onscreen()-1;
				itemRects_.clear();
				drawn_ = false;
			}
	}
}

int menu::process(int x, int y, bool button,bool up_arrow,bool down_arrow,
                  bool page_up, bool page_down, int select_item)
{
	static int last_scroll_position = 0;
	bool scroll_in_use = false;
	const int last_top_idx = items_.size() - max_items_onscreen();
	int max_scroll_position = scrollbar_.location().h-scrollbar_height_;
	if(show_scrollbar()) {
		const bool up = uparrow_.process(x,y,button);
		if(up && first_item_on_screen_ > 0) {
			--first_item_on_screen_;
			itemRects_.clear();
			drawn_ = false;
		}
		const bool down = downarrow_.process(x,y,button);
		if(down && first_item_on_screen_ < last_top_idx) {
			++first_item_on_screen_;
			itemRects_.clear();
			drawn_ = false;
		}

		scrollbar_.process();
		int scroll_position = scrollbar_.get_grip_position();
		int new_first_item; 
		if (scroll_position != last_scroll_position) {
			last_scroll_position = scroll_position;
			if (scroll_position == 0) {
				
				new_first_item = 0;
			}
			else if (scroll_position > (max_scroll_position)) {
				new_first_item = last_top_idx;
			}
			else {
				new_first_item = scroll_position * last_top_idx /
									max_scroll_position;
			}
			if (new_first_item != first_item_on_screen_) {
				scroll_in_use = true;
				first_item_on_screen_ = new_first_item;
				itemRects_.clear();
				drawn_ = false;
			}
		}
		else {
			int groove = scrollbar_.groove_clicked();
			if (groove == -1) {
				first_item_on_screen_ -= max_items_onscreen();
				if (first_item_on_screen_ <= 0) {
					first_item_on_screen_ = 0;
				}
				itemRects_.clear();
				drawn_ = false;
			}
			else if (groove == 1) {
				first_item_on_screen_ += max_items_onscreen();
				if (first_item_on_screen_ > last_top_idx) {
					first_item_on_screen_ = last_top_idx;
				}
				itemRects_.clear();
				drawn_ = false;
			}
		}
	}

	if(!drawn_) {
		if (!scroll_in_use && show_scrollbar()) {
			int new_scrollpos = 
				(first_item_on_screen_ * max_scroll_position)
					 / last_top_idx;
			scrollbar_.set_grip_position(new_scrollpos);
			last_scroll_position = new_scrollpos;
		}
		draw();
	}

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

void menu::scroll(int pos)
{
}

namespace {
	const char ImagePrefix = '&';

	SDL_Rect item_size(const std::string& item) {
		SDL_Rect res = {0,0,0,0};
		if(item.empty() == false && item[0] == ImagePrefix) {
			const std::string image_name(item.begin()+1,item.end());
			SDL_Surface* const img = image::get_image(image_name,image::UNSCALED);
			if(img != NULL) {
				res.w = img->w;
				res.h = img->h;
			}
		} else {
			const SDL_Rect area = {0,0,10000,10000};
			res = font::draw_text(NULL,area,menu_font_size,font::NORMAL_COLOUR,item,0,0);
		}

		return res;
	}
}

const std::vector<int>& menu::column_widths() const
{
	if(column_widths_.empty()) {
		for(size_t row = 0; row != items_.size(); ++row) {
			for(size_t col = 0; col != items_[row].size(); ++col) {
				static const SDL_Rect area = {0,0,display_->x(),display_->y()};

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
		const std::string& str = items_[item][i];
		if(str.empty() == false && str[0] == ImagePrefix) {
			const std::string image_name(str.begin()+1,str.end());
			SDL_Surface* const img = image::get_image(image_name,image::UNSCALED);
			if(img != NULL && xpos+img->w < display_->x() && rect.y+img->h < display_->y()) {
				display_->blit_surface(xpos,rect.y,img);
			}

		} else {
			const SDL_Rect& text_size = font::text_area(str,menu_font_size);
			const size_t y = rect.y + (rect.h - text_size.h)/2;
			font::draw_text(display_,area,menu_font_size,font::NORMAL_COLOUR,str,xpos,y);
		}
		xpos += widths[i];
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

	// update enabled/disabled status for up/down buttons
	if(show_scrollbar()) {
		uparrow_.hide(first_item_on_screen_ == 0);
		downarrow_.hide(first_item_on_screen_ >= items_.size() - max_items_onscreen());
	}

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

size_t menu::get_item_height(int item) const
{
	if(item_height_ != -1)
		return size_t(item_height_);

	size_t max_height = 0;
	for(size_t n = 0; n != items_.size(); ++n) {
		max_height = maximum<int>(max_height,get_item_height_internal(n));
	}

	return item_height_ = max_height;
}

}
