#include "menu.hpp"

#include "../font.hpp"
#include "../sdl_utils.hpp"
#include "../show_dialog.hpp"
#include "../util.hpp"
#include "../video.hpp"

#include <numeric>

namespace {
const size_t menu_font_size = 14;
const size_t menu_cell_padding = 12;
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
		  double_clicked_(false)
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

int menu::height() const
{
	if(height_ == -1) {
		height_ = 0;
		for(size_t i = 0; i != items_.size() && i != max_items_onscreen(); ++i) {
			height_ += get_item_rect(i).h;
		}

		if(items_.size() > max_items_onscreen()) {
			height_ += uparrow_.height() + downarrow_.height();
		}
	}

	return height_;
}

int menu::width() const
{
	if(width_ == -1) {
		const std::vector<int>& widths = column_widths();
		width_ = std::accumulate(widths.begin(),widths.end(),0);
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

	if(items_.size() > max_items_onscreen()) {
		uparrow_.set_x(x_);
		uparrow_.set_y(y_);
		downarrow_.set_x(x_);
		downarrow_.set_y(y_+items_end());
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
	draw();
	uparrow_.draw();
	downarrow_.draw();
}

void menu::change_item(int pos1, int pos2,std::string str)
{
 items_[pos1][pos2] = str;
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
	} else if(selected_ >= first_item_on_screen_ + int(max_items_onscreen())) {
		first_item_on_screen_ = selected_ - (max_items_onscreen() - 1);
		itemRects_.clear();
	}
}

void menu::key_press(SDLKey key)
{
	switch(key) {
		case SDLK_UP: {
			if(!click_selects_ && selected_ > 0) {
				--selected_;
				calculate_position();
				drawn_ = false;
			}

			break;
		}

		case SDLK_DOWN: {
			if(!click_selects_ && selected_ < int(items_.size())-1) {
				++selected_;
				calculate_position();
				drawn_ = false;
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

	if(key >= SDLK_1 && key <= SDLK_9) {
		const int pos = key - SDLK_1;
		if(size_t(pos) < items_.size()) {
			selected_ = pos;
			calculate_position();
			drawn_ = false;
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
			selected_ = item;
			drawn_ = false;

			if(click_selects_) {
				show_result_ = true;
			}

			if(event.type == DOUBLE_CLICK_EVENT) {
				double_clicked_ = true;
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
	if(items_.size() > max_items_onscreen()) {
		const bool up = uparrow_.process(x,y,button);
		if(up && first_item_on_screen_ > 0) {
			--first_item_on_screen_;
			itemRects_.clear();
			drawn_ = false;
		}
		const bool down = downarrow_.process(x,y,button);
		if(down && first_item_on_screen_ + max_items_onscreen() < items_.size()) {
			++first_item_on_screen_;
			itemRects_.clear();
			drawn_ = false;
		}
	}

	if(!drawn_) {
		draw();
	}

	if(show_result_) {
		return selected_;
	} else {
		return -1;
	}
}

bool menu::double_clicked() const
{
	return double_clicked_;
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

void menu::draw_item(int item)
{
	SDL_Rect rect = get_item_rect(item);
	if(rect.w == 0) {
		return;
	}

	if(buffer_.get() != NULL) {
		const int ypos = items_start()+(item-first_item_on_screen_)*rect.h;
		SDL_Rect srcrect = {0,ypos,rect.w,rect.h};
		SDL_Rect dstrect = rect;
		SDL_BlitSurface(buffer_,&srcrect,
		                display_->video().getSurface(),&dstrect);
	}

	gui::draw_solid_tinted_rectangle(x_,rect.y,width(),rect.h,
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
			const SDL_Rect& text_size = font::draw_text(NULL,area,menu_font_size,font::NORMAL_COLOUR,str,xpos,rect.y);
			const size_t y = rect.y + (rect.h - text_size.h)/2;
			font::draw_text(display_,area,menu_font_size,font::NORMAL_COLOUR,str,xpos,y);
		}
		xpos += widths[i];
	}
}

void menu::draw()
{
	drawn_ = true;

	// update enabled/disabled status for up/down buttons
	if(items_.size() > max_items_onscreen()) {
		if(first_item_on_screen_ == 0) {
			uparrow_.enable(false);
			uparrow_.hide();
		} else {
			uparrow_.enable(true);
		}

		if(first_item_on_screen_ >= items_.size() - max_items_onscreen()) {
			downarrow_.enable(false);
			downarrow_.hide();
		} else {
			downarrow_.enable(true);
		}
	}

	for(size_t i = 0; i != items_.size(); ++i)
		draw_item(i);

	update_rect(x_,y_,width(),height());
}

int menu::hit(int x, int y) const
{
	if(x > x_ && x < x_ + width() && y > y_ && y < y_ + height()){
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

	int y = y_ + items_start();
	if(item != first_item_on_screen_) {
		const SDL_Rect& prev = get_item_rect(item-1);
		y = prev.y + prev.h;
	}

	static const SDL_Rect area = {0,0,display_->x(),display_->y()};

	SDL_Rect res = {x_,y,width(),get_item_height(item)};

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

int menu::items_start() const
{
	if(items_.size() > max_items_onscreen())
		return uparrow_.height();
	else
		return 0;
}

int menu::items_end() const
{
	if(items_.size() > max_items_onscreen())
		return height() - downarrow_.height();
	else
		return height();
}

int menu::items_height() const
{
	return items_end() - items_start();
}

}
