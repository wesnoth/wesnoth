/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "config.hpp"
#include "font.hpp"
#include "language.hpp"
#include "menu.hpp"
#include "playlevel.hpp"
#include "language.hpp"
#include "sdl_utils.hpp"
#include "widgets/button.hpp"
#include "widgets/textbox.hpp"

#include <iostream>
#include <numeric>

namespace gui {

void draw_dialog_frame(int x, int y, int w, int h, display& disp)
{
	const int border_size = 6;

	SDL_Surface* const scr = disp.video().getSurface();

	const display::Pixel border_colour = SDL_MapRGB(scr->format,200,0,0);

	draw_solid_tinted_rectangle(x-border_size,y-border_size,
	                            w+border_size,h+border_size,0,0,0,0.6,scr);
	draw_solid_tinted_rectangle(x,y,w+border_size,h+border_size,0,0,0,0.6,scr);

	draw_rectangle(x-border_size,y-border_size,w+border_size,h+border_size,
	               border_colour,scr);
	draw_rectangle(x,y,w+border_size,h+border_size,border_colour,scr);
}

void draw_rectangle(int x, int y, int w, int h, short colour,
                    SDL_Surface* target)
{
	if(x < 0 || y < 0 || x+w >= target->w || y+h >= target->h) {
		std::cerr << "Rectangle has illegal co-ordinates: " << x << "," << y
		          << "," << w << "," << h << "\n";
		return;
	}

	short* top_left = reinterpret_cast<short*>(target->pixels) + target->w*y+x;
	short* top_right = top_left + w;
	short* bot_left = top_left + target->w*h;
	short* bot_right = bot_left + w;

	std::fill(top_left,top_right+1,colour);
	std::fill(bot_left,bot_right+1,colour);
	while(top_left != bot_left) {
		*top_left = colour;
		*top_right = colour;
		top_left += target->w;
		top_right += target->w;
	}
}

void draw_solid_tinted_rectangle(int x, int y, int w, int h,
                                 int r, int g, int b,
                                 double alpha, SDL_Surface* target)
{
	if(x < 0 || y < 0 || x+w >= target->w || y+h >= target->h) {
		std::cerr << "Rectangle has illegal co-ordinates: " << x << "," << y
		          << "," << w << "," << h << "\n";
		return;
	}

	const SDL_PixelFormat* const fmt = target->format;
	short* p = reinterpret_cast<short*>(target->pixels) + target->w*y + x;
	while(h > 0) {
		short* beg = p;
		short* const end = p + w;
		while(beg != end) {
			const int cur_r = ((*beg&fmt->Rmask) >> fmt->Rshift) << fmt->Rloss;
			const int cur_g = ((*beg&fmt->Gmask) >> fmt->Gshift) << fmt->Gloss;
			const int cur_b = ((*beg&fmt->Bmask) >> fmt->Bshift) << fmt->Bloss;

			const int new_r = int(double(cur_r)*(1.0-alpha) + double(r)*alpha);
			const int new_g = int(double(cur_g)*(1.0-alpha) + double(g)*alpha);
			const int new_b = int(double(cur_b)*(1.0-alpha) + double(b)*alpha);

			*beg = ((new_r >> fmt->Rloss) << fmt->Rshift) |
			       ((new_g >> fmt->Gloss) << fmt->Gshift) |
			       ((new_b >> fmt->Bloss) << fmt->Bshift);

			++beg;
		}

		p += target->w;
		--h;
	}
}

} //end namespace gui

namespace {
const size_t max_menu_items = 18;
const size_t menu_font_size = 16;
const size_t menu_cell_padding = 10;

class menu
{
	display* display_;
	int x_, y_;
	std::vector<std::vector<std::string> > items_;
	mutable std::vector<int> column_widths_;

	scoped_sdl_surface buffer_;
	int selected_;
	bool click_selects_;
	bool previous_button_;
	bool drawn_;

	mutable int height_;
	mutable int width_;

	mutable int first_item_on_screen_;
	gui::button uparrow_, downarrow_;

	const std::vector<int>& column_widths() const
	{
		if(column_widths_.empty()) {
			for(size_t row = 0; row != items_.size(); ++row) {
				for(size_t col = 0; col != items_[row].size(); ++col) {
					static const SDL_Rect area =
					                {0,0,display_->x(),display_->y()};

					const SDL_Rect res =
						font::draw_text(NULL,area,menu_font_size,
						          font::NORMAL_COLOUR,items_[row][col],x_,y_);

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

	void draw_item(int item) {
		SDL_Rect rect = get_item_rect(item);
		if(rect.w == 0)
			return;

		if(buffer_.get() != NULL) {
			const int ypos = items_start()+(item-first_item_on_screen_)*rect.h;
			SDL_Rect srcrect = {0,ypos,rect.w,rect.h};
			SDL_BlitSurface(buffer_,&srcrect,
			                display_->video().getSurface(),&rect);
		}

		gui::draw_solid_tinted_rectangle(x_,rect.y,width(),rect.h,
		                                 item == selected_ ? 150:0,0,0,
		                                 0.7,display_->video().getSurface());

		SDL_Rect area = {0,0,display_->x(),display_->y()};

		const std::vector<int>& widths = column_widths();

		int xpos = rect.x;
		for(size_t i = 0; i != items_[item].size(); ++i) {
			font::draw_text(display_,area,menu_font_size,font::NORMAL_COLOUR,
			                items_[item][i],xpos,rect.y);
			xpos += widths[i];
		}
	}

	void draw() {
		drawn_ = true;

		for(size_t i = 0; i != items_.size(); ++i)
			draw_item(i);

		display_->video().flip();
	}

	int hit(int x, int y) const {
		if(x > x_ && x < x_ + width() && y > y_ && y < y_ + height()){
			for(size_t i = 0; i != items_.size(); ++i) {
				const SDL_Rect& rect = get_item_rect(i);
				if(y > rect.y && y < rect.y + rect.h)
					return i;
			}
		}

		return -1;
	}

	mutable std::map<int,SDL_Rect> itemRects_;

	SDL_Rect get_item_rect(int item) const
	{
		const SDL_Rect empty_rect = {0,0,0,0};
		if(item < first_item_on_screen_ ||
		   size_t(item) >= first_item_on_screen_ + max_menu_items) {
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

		SDL_Rect res = font::draw_text(NULL,area,menu_font_size,
			                   font::NORMAL_COLOUR,items_[item][0],x_,y);

		res.w = width();

		//only insert into the cache if the menu's co-ordinates have
		//been initialized
		if(x_ > 0 && y_ > 0)
			itemRects_.insert(std::pair<int,SDL_Rect>(item,res));

		return res;
	}

	int items_start() const
	{
		if(items_.size() > max_menu_items)
			return uparrow_.height();
		else
			return 0;
	}

	int items_end() const
	{
		if(items_.size() > max_menu_items)
			return height() - downarrow_.height();
		else
			return height();
	}

	int items_height() const
	{
		return items_end() - items_start();
	}

public:
	menu(display& disp, const std::vector<std::string>& items,
	     bool click_selects=false)
	        : display_(&disp), x_(0), y_(0), buffer_(NULL),
	          selected_(-1), click_selects_(click_selects),
	          previous_button_(true), drawn_(false), height_(-1), width_(-1),
			  first_item_on_screen_(0),
			  uparrow_(disp,"",gui::button::TYPE_PRESS,"uparrow"),
	          downarrow_(disp,"",gui::button::TYPE_PRESS,"downarrow")
	{
		for(std::vector<std::string>::const_iterator item = items.begin();
		    item != items.end(); ++item) {
			items_.push_back(config::split(*item));

			//make sure there is always at least one item
			if(items_.back().empty())
				items_.back().push_back(" ");
		}
	}

	int height() const {
		if(height_ == -1) {
			height_ = 0;
			for(size_t i = 0; i != items_.size() && i != max_menu_items; ++i) {
				height_ += get_item_rect(i).h;
			}

			if(items_.size() > max_menu_items) {
				height_ += uparrow_.height() + downarrow_.height();
			}
		}

		return height_;
	}

	int width() const {
		if(width_ == -1) {
			const std::vector<int>& widths = column_widths();
			width_ = std::accumulate(widths.begin(),widths.end(),0);
		}

		return width_;
	}

	int selection() const { return selected_; }

	void set_loc(int x, int y) {
		x_ = x;
		y_ = y;

		const int w = width();

		SDL_Rect portion = {x_,y_,w,height()};
		SDL_Surface* const screen = display_->video().getSurface();
		buffer_.assign(get_surface_portion(screen, portion));

		if(items_.size() > max_menu_items) {
			uparrow_.set_x(x_);
			uparrow_.set_y(y_);
			downarrow_.set_x(x_);
			downarrow_.set_y(y_+items_end());
		}
	}

	int process(int x, int y, bool button,bool up_arrow,bool down_arrow,
	            bool page_up, bool page_down, int select_item) {
		if(items_.size() > size_t(max_menu_items)) {
			const bool up = uparrow_.process(x,y,button);
			if(up && first_item_on_screen_ > 0) {
				itemRects_.clear();
				--first_item_on_screen_;

				draw();
			}

			const bool down = downarrow_.process(x,y,button);
			if(down &&
			   size_t(first_item_on_screen_+max_menu_items) < items_.size()) {
				itemRects_.clear();
				++first_item_on_screen_;
				draw();
			}
		}

		if(select_item >= 0 && size_t(select_item) < items_.size()) {
			selected_ = select_item;
			if(selected_ < first_item_on_screen_) {
				itemRects_.clear();
				first_item_on_screen_ = selected_;
			}

			if(size_t(selected_ - first_item_on_screen_) >= max_menu_items) {
				itemRects_.clear();
				first_item_on_screen_ = selected_ - max_menu_items - 1;
			}

			draw();
		}

		if(up_arrow && !click_selects_ && selected_ > 0) {
			--selected_;
			if(selected_ < first_item_on_screen_) {
				itemRects_.clear();
				first_item_on_screen_ = selected_;
			}

			draw();
		}

		if(down_arrow && !click_selects_ && selected_ < int(items_.size()-1)) {
			++selected_;
			if(size_t(selected_ - first_item_on_screen_) == max_menu_items) {
				itemRects_.clear();
				++first_item_on_screen_;
			}

			draw();
		}

		if(page_up && !click_selects_) {
			selected_ -= max_menu_items;
			if(selected_ < 0)
				selected_ = 0;

			itemRects_.clear();
			first_item_on_screen_ = selected_;

			draw();
		}

		if(page_down && !click_selects_) {
			selected_ += max_menu_items;
			if(size_t(selected_) >= items_.size())
				selected_ = items_.size()-1;

			first_item_on_screen_ = selected_ - (max_menu_items-1);
			if(first_item_on_screen_ < 0)
				first_item_on_screen_ = 0;

			itemRects_.clear();

			draw();
		}

		const int starting_selected = selected_;

		const int hit_item = hit(x,y);

		if(click_selects_) {
			selected_ = hit_item;
			if(button && !previous_button_)
				return selected_;
			else {
				if(!drawn_ || selected_ != starting_selected)
					draw();
				previous_button_ = button;
				return -1;
			}
		}

		if(button && hit_item != -1){
			selected_ = hit_item;
		}

		if(selected_ == -1)
			selected_ = 0;

		if(selected_ != starting_selected)
			draw();

		return -1;
	}
};

}

namespace gui
{

int show_dialog(display& disp, SDL_Surface* image,
                const std::string& caption, const std::string& msg,
                DIALOG_TYPE type,
				const std::vector<std::string>* menu_items_ptr,
				const std::vector<unit>* units_ptr,
				const std::string& text_widget_label,
				std::string* text_widget_text)
{
	if(disp.update_locked())
		return -1;

	const resize_lock prevent_resizing;

	const std::vector<std::string>& menu_items =
	   (menu_items_ptr == NULL) ? std::vector<std::string>() : *menu_items_ptr;
	const std::vector<unit>& units =
	   (units_ptr == NULL) ? std::vector<unit>() : *units_ptr;

	static const int message_font_size = 16;
	static const int caption_font_size = 18;

	CVideo& screen = disp.video();
	SDL_Surface* const scr = screen.getSurface();

	SDL_Rect clipRect = { 0, 0, disp.x(), disp.y() };

	const bool use_textbox = text_widget_text != NULL;
	static const std::string default_text_string = "";
	const unsigned int text_box_width = 350;
	textbox text_widget(disp,text_box_width,
	                    use_textbox ? *text_widget_text : default_text_string);

	int text_widget_width = 0;
	int text_widget_height = 0;
	if(use_textbox) {
		text_widget_width =
		 font::draw_text(NULL, clipRect, message_font_size,
		                 font::NORMAL_COLOUR, text_widget_label, 0, 0, NULL).w +
		                            text_widget.width();
		text_widget_height = text_widget.height();
	}

	menu menu_(disp,menu_items,type == MESSAGE);

	const int border_size = 6;
	const short border_colour = 0xF000;
	int nlines = 1;
	int longest_line = 0;
	int cur_line = 0;

	const int max_line_length = 58;

	std::string message = msg;
	for(std::string::iterator message_it = message.begin();
	    message_it != message.end(); ++message_it) {
		if(*message_it == ' ' && cur_line > max_line_length)
			*message_it = '\n';

		if(*message_it == '\n') {
			if(cur_line > longest_line)
				longest_line = cur_line;

			cur_line = 0;

			++nlines;
		} else {
			++cur_line;
		}
	}

	SDL_Rect text_size = { 0, 0, 0, 0 };
	if(!message.empty()) {
		text_size = font::draw_text(NULL, clipRect, message_font_size,
						            font::NORMAL_COLOUR, message, 0, 0, NULL);
	}

	SDL_Rect caption_size = { 0, 0, 0, 0 };
	if(!caption.empty()) {
		caption_size = font::draw_text(NULL, clipRect, caption_font_size,
		                               font::NORMAL_COLOUR,caption,0,0,NULL);
	}

	const std::string* button_list = NULL;
	std::vector<button> buttons;
	switch(type) {
		case MESSAGE:
			break;
		case OK_ONLY: {
			static const std::string thebuttons[] = { "ok_button", "" };
			button_list = thebuttons;
			break;
		}

		case YES_NO: {
			static const std::string thebuttons[] = { "yes_button",
			                                          "no_button", ""};
			button_list = thebuttons;
			break;
		}

		case OK_CANCEL: {
			static const std::string thebuttons[] = { "ok_button",
			                                          "cancel_button",""};
			button_list = thebuttons;
			break;
		}
	}

	const int button_height_padding = 10;
	int button_width_padding = 0;
	int button_heights = 0;
	int button_widths = 0;
	if(button_list != NULL) {
		try {
			while(button_list->empty() == false) {
				buttons.push_back(button(disp,string_table[*button_list]));

				if(buttons.back().height() > button_heights)
					button_heights = buttons.back().height();

				button_widths += buttons.back().width();
				button_width_padding += 5;

				++button_list;
			}

		} catch(button::error&) {
			std::cerr << "error initializing button!\n";
		}
	}

	if(button_heights > 0) {
		button_heights += button_height_padding;
	}

	if(cur_line > longest_line)
		longest_line = cur_line;

	const int left_padding = 10;
	const int right_padding = 10;
	const int image_h_padding = image != NULL ? 10 : 0;
	const int top_padding = 10;
	const int bottom_padding = 10;
	const int padding_width = left_padding + right_padding + image_h_padding;
	const int padding_height = top_padding + bottom_padding;
	const int caption_width = caption_size.w;
	const int image_width = image != NULL ? image->w : 0;
	const int total_image_width = caption_width > image_width ?
	                              caption_width : image_width;
	const int image_height = image != NULL ? image->h : 0;

	int text_width = text_size.w;
	if(menu_.width() > text_width)
		text_width = menu_.width();

	int total_width = total_image_width + text_width +
	                  padding_width;
	if(button_widths + button_width_padding > total_width)
		total_width = button_widths + button_width_padding;

	if(text_widget_width+left_padding+right_padding > total_width)
		total_width = text_widget_width+left_padding+right_padding;

	const int total_height = (image_height+8 > text_size.h ?
	                          image_height+8 : text_size.h) +
	                         padding_height + button_heights + menu_.height() +
							 text_widget_height;

	if(total_width > scr->w - 100 || total_height > scr->h - 100)
		return false;

	int xloc = scr->w/2 - total_width/2;
	int yloc = scr->h/2 - total_height/2;

	int unitx = 0;
	int unity = 0;
	//if we are showing a dialog with unit details, then we have
	//to make more room for it
	if(!units.empty()) {
		xloc += scr->w/10;
		unitx = xloc - 300;
		if(unitx < 10)
			unitx = 10;

		unity = yloc;
	}

	//make sure that the dialog doesn't overlap the right part of the screen
	if(xloc + total_width+border_size >= disp.mapx()-1) {
		xloc = disp.mapx()-(total_width+border_size+2);
		if(xloc < 0)
			return -1;
	}

	const int button_hpadding = total_width - button_widths;
	int button_offset = 0;
	for(size_t button_num = 0; button_num != buttons.size(); ++button_num) {
		const int padding_amount = button_hpadding/(buttons.size()+1);
		buttons[button_num].set_x(xloc + padding_amount*(button_num+1) +
		                          button_offset);
		buttons[button_num].set_y(yloc + total_height - button_heights);
		button_offset += buttons[button_num].width();
	}

	if(menu_.height() > 0)
		menu_.set_loc(xloc+total_image_width+left_padding+image_h_padding,
		              yloc+top_padding+text_size.h);

	draw_dialog_frame(xloc,yloc,total_width,total_height,disp);

	if(image != NULL) {
		const int x = xloc + left_padding;
		const int y = yloc + top_padding;

		disp.blit_surface(x,y,image);

		int center_font = 0;
		if(caption_size.w < image->w) {
			center_font = image->w/2 - caption_size.w/2;
		}

		font::draw_text(&disp, clipRect, caption_font_size,
		                font::NORMAL_COLOUR, caption,
						xloc+left_padding+center_font,
		                yloc+top_padding+image->h-6, NULL);
	}

	if(!units.empty()) {
		const int unitw = 200;
		const int unith = disp.y()/2;
		draw_solid_tinted_rectangle(unitx,unity,unitw,unith,
		                            0,0,0,1.0,scr);
		draw_rectangle(unitx,unity,unitw,unith,border_colour,scr);
	}

	font::draw_text(&disp, clipRect, message_font_size,
	                font::NORMAL_COLOUR, message,
					xloc+total_image_width+left_padding+image_h_padding,
					yloc+top_padding);

	if(use_textbox) {
		const int image_h = image != NULL ? image->h : 0;
		const int text_widget_y = yloc+top_padding+image_h-6+text_size.h;
		text_widget.set_location(xloc + left_padding +
		                         text_widget_width - text_widget.width(),
		                         text_widget_y);
		text_widget.draw();
		font::draw_text(&disp, clipRect, message_font_size,
		                font::NORMAL_COLOUR, text_widget_label,
						xloc + left_padding,text_widget_y);
	}

	screen.flip();

	CKey key;

	bool left_button = true, right_button = true, key_down = true,
	     up_arrow = false, down_arrow = false,
	     page_up = false, page_down = false;

	disp.invalidate_all();

	int cur_selection = -1;

	SDL_Rect unit_details_rect, unit_profile_rect;
	unit_details_rect.w = 0;
	unit_profile_rect.w = 0;

	bool first_time = true;

	for(;;) {
		int mousex, mousey;
		const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);

		const bool new_right_button = mouse_flags&SDL_BUTTON_RMASK;
		const bool new_left_button = mouse_flags&SDL_BUTTON_LMASK;
		const bool new_key_down = key[KEY_SPACE] || key[KEY_ENTER] ||
		                          key[KEY_ESCAPE];

		const bool new_up_arrow = key[KEY_UP];
		const bool new_down_arrow = key[KEY_DOWN];

		const bool new_page_up = key[SDLK_PAGEUP];
		const bool new_page_down = key[SDLK_PAGEDOWN];

		int select_item = -1;
		for(int item = 0; item != 10; ++item) {
			if(key['1' + item])
				select_item = item;
		}


		if(!key_down && key[KEY_ENTER] &&
		   (type == YES_NO || type == OK_CANCEL)) {
			if(menu_.height() == 0) {
				return 0;
			} else {
				return menu_.selection();
			}
		}

		if(menu_.selection() != cur_selection || first_time) {
			cur_selection = menu_.selection();

			int selection = cur_selection;
			if(first_time)
				selection = 0;

			if(size_t(selection) < units.size()) {
				SDL_Surface* const screen = disp.video().getSurface();
				if(unit_details_rect.w > 0) {
					SDL_FillRect(screen,&unit_details_rect,0);
				}

				if(unit_profile_rect.w > 0) {
					SDL_FillRect(screen,&unit_profile_rect,0);
				}

				disp.draw_unit_details(unitx+left_padding,
				   unity+top_padding, gamemap::location(), units[selection],
				   unit_details_rect, unit_profile_rect);
				disp.update_display();
			}
		}

		first_time = false;

		if(menu_.height() > 0) {
			const int res = menu_.process(mousex,mousey,new_left_button,
			                              !up_arrow && new_up_arrow,
										  !down_arrow && new_down_arrow,
			                              !page_up && new_page_up,
			                              !page_down && new_page_down,
			                              select_item);
			if(res != -1)
				return res;
		}

		up_arrow = new_up_arrow;
		down_arrow = new_down_arrow;
		page_up = new_page_up;
		page_down = new_page_down;

		if(use_textbox) {
			text_widget.process();
		}


		if(buttons.empty() && (new_left_button && !left_button ||
		                       new_right_button && !right_button) ||
		   buttons.size() < 2 && new_key_down && !key_down &&
		   menu_.height() == 0)
			break;

		left_button = new_left_button;
		right_button = new_right_button;
		key_down = new_key_down;

		for(std::vector<button>::iterator button_it = buttons.begin();
		    button_it != buttons.end(); ++button_it) {
			if(button_it->process(mousex,mousey,left_button)) {
				if(text_widget_text != NULL && use_textbox)
					*text_widget_text = text_widget.text();

				//if the menu is not used, then return the index of the
				//button pressed, otherwise return the index of the menu
				//item selected if the last button is not pressed, and
				//cancel (-1) otherwise
				if(menu_.height() == 0) {
					return button_it - buttons.begin();
				} else if(buttons.size() <= 1 ||
				       size_t(button_it-buttons.begin()) != buttons.size()-1) {
					return menu_.selection();
				} else {
					return -1;
				}
			}
		}

		pump_events();
	}

	return -1;
}

TITLE_RESULT show_title(display& screen)
{
	const resize_lock prevent_resizing;

	SDL_Surface* const title_surface = screen.getImage("title.png",
	                                                   display::UNSCALED);

	if(title_surface == NULL) {
		std::cerr << "Could not find title image 'title.png'\n";
		return QUIT_GAME;
	}

	const std::string& version_str = string_table["version"] + " " +
	                                 game_config::version;

	const SDL_Rect version_area = draw_text(NULL,screen.screen_area(),10,
	                                    font::NORMAL_COLOUR,version_str,0,0);
	const size_t versiony = screen.y() - version_area.h;

	if(versiony < size_t(screen.y())) {
		draw_text(&screen,screen.screen_area(),10,font::NORMAL_COLOUR,
		          version_str,0,versiony);
	}

	const int x = screen.x()/2 - title_surface->w/2;
	const int y = 100;

	screen.blit_surface(x,y,title_surface);

	button tutorial_button(screen,string_table["tutorial_button"]);
	button new_button(screen,string_table["campaign_button"]);
	button load_button(screen,string_table["load_button"]);
	button multi_button(screen,string_table["multiplayer_button"]);
	button quit_button(screen,string_table["quit_button"]);
	button language_button(screen,string_table["language_button"]);

	const int menu_xbase = screen.x() - (x + tutorial_button.width() + 20);
	const int menu_xincr = 0;
	const int menu_ybase = 120 + title_surface->h;
	const int menu_yincr = 50;
	int bc = 0; //button count
#define BUTTON_XY() (menu_xbase+(bc)*menu_xincr), (menu_ybase+(bc++)*menu_yincr)

	tutorial_button.set_xy(BUTTON_XY());
	new_button.set_xy(BUTTON_XY());
	load_button.set_xy(BUTTON_XY());
	multi_button.set_xy(BUTTON_XY());
	language_button.set_xy(BUTTON_XY());
	quit_button.set_xy(BUTTON_XY());

	tutorial_button.draw();
	new_button.draw();
	load_button.draw();
	multi_button.draw();
	quit_button.draw();
	language_button.draw();
	screen.video().flip();

	CKey key;

	for(;;) {
		int mousex, mousey;
		const int mouse_flags = SDL_GetMouseState(&mousex,&mousey);
		const bool left_button = mouse_flags&SDL_BUTTON_LMASK;

		if(tutorial_button.process(mousex,mousey,left_button))
			return TUTORIAL;

		if(new_button.process(mousex,mousey,left_button))
			return NEW_CAMPAIGN;

		if(load_button.process(mousex,mousey,left_button))
			return LOAD_GAME;

		if(multi_button.process(mousex,mousey,left_button))
			return MULTIPLAYER;

		if(language_button.process(mousex,mousey,left_button))
			return CHANGE_LANGUAGE;

		if(quit_button.process(mousex,mousey,left_button))
			return QUIT_GAME;

		if(key[KEY_ESCAPE])
			return QUIT_GAME;

		pump_events();

		SDL_Delay(20);
	}

	return QUIT_GAME;
}

} //end namespace gui
