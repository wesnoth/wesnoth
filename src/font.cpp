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

#include "SDL_ttf.h"

#include "config.hpp"
#include "font.hpp"
#include "game_config.hpp"
#include "language.hpp"
#include "log.hpp"
#include "sdl_utils.hpp"
#include "tooltips.hpp"

#include <cstdio>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

namespace {

std::map<int,TTF_Font*> font_table;
std::string font_name = "Vera.ttf";

//SDL_ttf seems to have a problem where TTF_OpenFont will seg fault if
//the font file doesn't exist, so make sure it exists first.
TTF_Font* open_font(const std::string& fname, int size)
{
	std::string name;

	if(game_config::path.empty() == false) {
		name = game_config::path + "/fonts/" + fname;
		std::cerr << "Opening font file: " << name << " ...\n";

		if(read_file(name).empty()) {
			name = "fonts/" + fname;
			std::cerr << "Failed, now trying: " << name << " ...\n";
			if(read_file(name).empty()) {
				std::cerr << "Failed :(\n";
				return NULL;
			}
		}
	} else {
		name = "fonts/" + fname;
		std::cerr << "Opening font file: " << name << " ...\n";

		if(read_file(name).empty()) {
			std::cerr << "Failed :(\n";
			return NULL;
		}
	}

	TTF_Font* font = TTF_OpenFont(name.c_str(),size);
	if(font == NULL) {
		std::cerr << "Could not open font file: " << name << '\n';
	}

	return font;
}

TTF_Font* get_font(int size)
{
	const std::map<int,TTF_Font*>::iterator it = font_table.find(size);
	if(it != font_table.end())
		return it->second;

	TTF_Font* font = open_font(font_name,size);

	if(font == NULL)
		return NULL;

	TTF_SetFontStyle(font,TTF_STYLE_NORMAL);

	std::cerr << "Inserting font...\n";
	font_table.insert(std::pair<int,TTF_Font*>(size,font));
	return font;
}

void clear_fonts()
{
	for(std::map<int,TTF_Font*>::iterator i = font_table.begin(); i != font_table.end(); ++i) {
		TTF_CloseFont(i->second);
	}

	font_table.clear();
}

}

namespace font {

manager::manager()
{
	const int res = TTF_Init();
	if(res < 0) {
		std::cerr << "Could not initialize true type fonts\n";
	} else {
		std::cerr << "Initialized true type fonts\n";
	}
}

manager::~manager()
{
	clear_fonts();
	TTF_Quit();
}

void set_font(const std::string& name)
{
	clear_fonts();
	font_name = name;
	if(font_name == "") {
		font_name = "Vera.ttf";
	}
}

const SDL_Color NORMAL_COLOUR = {0xDD,0xDD,0xDD,0},
                GOOD_COLOUR   = {0x00,0xFF,0x00,0},
                BAD_COLOUR    = {0xFF,0x00,0x00,0},
                BLACK_COLOUR  = {0x00,0x00,0x00,0},
                DARK_COLOUR   = {0x00,0x00,0x66,0},
                YELLOW_COLOUR = {0xFF,0xFF,0x00,0},
                BUTTON_COLOUR = {0xBC,0xB0,0x88,0};

const char LARGE_TEXT='+', SMALL_TEXT='-', GOOD_TEXT='@', BAD_TEXT='#',
           NORMAL_TEXT='{', BLACK_TEXT='}', IMAGE='&', NULL_MARKUP='^';


const SDL_Color& get_side_colour(int side)
{
	side -= 1;

	static const SDL_Color sides[] = { {0xFF,0x00,0x00,0},
	                                   {0x00,0x00,0xFF,0},
	                                   {0x00,0xFF,0x00,0},
	                                   {0xFF,0xFF,0x00,0},
	                                   {0xFF,0x00,0xFF,0},
					   {0x33,0x33,0x33,0},
					   {0x89,0x89,0x89,0},
					   {0xFF,0xFF,0xFF,0},
					   {0x94,0x50,0x27,0},
					   {0x02,0xF5,0xE1,0},
	                                   {0xFF,0x00,0xFF,0} };

	static const size_t nsides = sizeof(sides)/sizeof(*sides);

	if(size_t(side) < nsides) {
		return sides[side];
	} else {
		return BLACK_COLOUR;
	}
}

namespace {
SDL_Surface* render_text(TTF_Font* font,const std::string& str,
						 const SDL_Color& colour)
{
	switch(charset())
	{
	case CHARSET_UTF8:
		return TTF_RenderUTF8_Blended(font,str.c_str(),colour);
	case CHARSET_LATIN1:
		return TTF_RenderText_Blended(font,str.c_str(),colour);
	default:
		std::cerr << "Unrecognized charset\n";
		return NULL;
	}
}

}

SDL_Rect draw_text_line(display* gui, const SDL_Rect& area, int size,
                        const SDL_Color& colour, const std::string& text,
                        int x, int y, SDL_Surface* bg, bool use_tooltips)
{
	TTF_Font* const font = get_font(size);
	if(font == NULL) {
		std::cerr << "Could not get font\n";
		SDL_Rect res;
		res.x = 0; res.y = 0; res.w = 0; res.h = 0;
		return res;
	}

	scoped_sdl_surface surface(render_text(font,text.c_str(),colour));
	if(surface == NULL) {
		std::cerr << "Could not render ttf: '" << text << "'\n";
		SDL_Rect res;
		res.x = 0; res.y = 0; res.w = 0; res.h = 0;
		return res;
	}

	SDL_Rect dest;
	if(x!=-1)
		dest.x = x;
	else
		dest.x = (area.w/2)-(surface->w/2);
	if(y!=-1)
		dest.y = y;
	else
		dest.y = (area.h/2)-(surface->h/2);
	dest.w = surface->w;
	dest.h = surface->h;

	if(dest.x + dest.w > area.x + area.w) {

		if(text.size() > 3) {
			std::string txt = text;
			if(std::count(txt.end()-3,txt.end(),'.') == 3) {
				txt.erase(txt.end()-4);
			} else {
				tooltips::add_tooltip(dest,text);
				std::fill(txt.end()-3,txt.end(),'.');
			}

			return draw_text_line(gui,area,size,colour,txt,x,y,bg,false);
		}

		dest.w = area.x + area.w - dest.x;
	}

	if(dest.y + dest.h > area.y + area.h) {
		dest.h = area.y + area.h - dest.y;
	}

	if(gui != NULL) {
		SDL_Rect src = dest;
		src.x = 0;
		src.y = 0;
		sdl_safe_blit(surface,&src,gui->video().getSurface(),&dest);
	}

	if(use_tooltips) {
		tooltips::add_tooltip(dest,text);
	}

	return dest;
}


SDL_Rect draw_text(display* gui, const SDL_Rect& area, int size,
                   const SDL_Color& colour, const std::string& txt,
                   int x, int y, SDL_Surface* bg, bool use_tooltips,
                   MARKUP use_markup)
{
	//make sure there's always at least a space, so we can ensure
	//that we can return a rectangle for height
	static const std::string blank_text(" ");
	const std::string& text = txt.empty() ? blank_text : txt;

	SDL_Rect res;
	res.x = x;
	res.y = y;
	res.w = 0;
	res.h = 0;

	std::string::const_iterator i1 = text.begin();
	std::string::const_iterator i2 = std::find(i1,text.end(),'\n');
	SDL_Color col = colour;
	int sz = size;
	for(;;) {
		if(i1 != i2) {
			if(use_markup == USE_MARKUP) {
				if(*i1 == '\\') {
					// this must either be a quoted special character or a
					// quoted backslash - either way, remove leading backslash
					++i1;
				} else if(*i1 == BAD_TEXT) {
					col = BAD_COLOUR;
					++i1;
				} else if(*i1 == GOOD_TEXT) {
					col = GOOD_COLOUR;
					++i1;
				} else if(*i1 == NORMAL_TEXT) {
					col = NORMAL_COLOUR;
					++i1;
					continue;
				} else if(*i1 == BLACK_TEXT) {
					col = BLACK_COLOUR;
					++i1;
					continue;
				} else if(*i1 == LARGE_TEXT) {
					sz += 2;
					++i1;
					continue;
				} else if(*i1 == SMALL_TEXT) {
					sz -= 2;
					++i1;
					continue;
				} else if(*i1 == NULL_MARKUP) {
					++i1;
				} else if(*i1 >= 1 && *i1 <= 9) {
					col = get_side_colour(*i1);
					++i1;
					continue;
				}
			}

			if(i1 != i2) {
				std::string new_string(i1,i2);

				config::unescape(new_string);

				const SDL_Rect rect =
				    draw_text_line(gui,area,sz,col,new_string,x,y,bg,
				                   use_tooltips);
				if(rect.w > res.w)
					res.w = rect.w;
				res.h += rect.h;
				y += rect.h;
			}
		}

		col = colour;
		sz = size;

		if(i2 == text.end()) {
			break;
		}

		i1 = i2+1;
		i2 = std::find(i1,text.end(),'\n');
	}

	return res;
}

bool is_format_char(char c)
{
	switch(c) {
	case GOOD_TEXT:
	case BAD_TEXT:
		return true;
	default:
		return false;
	}
}


std::string remove_first_space(const std::string& text)
{
  if (text.length() > 0 && text[0] == ' ') {
    return text.substr(1);
  }
  
  return text;
}


}

namespace font {

  int line_width(const std::string line, int font_size)
  {
    
    TTF_Font* const font = get_font(font_size);
    if(font == NULL) {
      std::cerr << "Could not get font\n";
      return 0;
    }
    int w = 0;
    int h = 0;
  
    switch(charset())
      {
      case CHARSET_UTF8:
	TTF_SizeUTF8(font, line.c_str(), &w, &h);
	break;
      case CHARSET_LATIN1:
	TTF_SizeText(font, line.c_str(), &w, &h);
	break;
      default:
	std::cerr << "Unrecognized charset\n";
      }

    return w;
  }

  
  std::string word_wrap_text(const std::string& unwrapped_text, int font_size, int max_width)
  {
    std::string wrapped_text; // the final result
  
    size_t word_start_pos = 0;
    std::string cur_word; // including start-whitespace
    std::string cur_line; // the whole line so far
  
    for(int c = 0; c < unwrapped_text.length(); c++) {

      // Find the next word
      bool forced_line_break = false;
      if (c == unwrapped_text.length() - 1) {
	cur_word = unwrapped_text.substr(word_start_pos, c + 1 - word_start_pos);
	word_start_pos = c + 1;
      } else if (unwrapped_text[c] == '\n') {
	cur_word = unwrapped_text.substr(word_start_pos, c + 1 - word_start_pos);
	word_start_pos = c + 1;
	forced_line_break = true;
      } else if (unwrapped_text[c] == ' ') {
	cur_word = unwrapped_text.substr(word_start_pos, c - word_start_pos);
	word_start_pos = c;
      } else {
	continue;
      }

      // Test if the line should be wrapped or not
      if (line_width(cur_line + cur_word, font_size) > max_width) {

	if (line_width(cur_word, font_size) > (max_width /*/ 2*/)) {
	  // The last word is too big to fit in a nice way, split it on a char basis
	  for (std::string::iterator i = cur_word.begin(); i != cur_word.end(); ++i) {
	    if (line_width(cur_line + *i, font_size) > max_width) {
	      wrapped_text += cur_line + '\n';
	      cur_line = *i;
	    } else {
	      cur_line += *i;
	    }
	  }
	
	} else {
	  // Split the line on a word basis
	  wrapped_text += cur_line + '\n';
	  cur_line = remove_first_space(cur_word);
	
	}
      } else {
	cur_line += cur_word;
      }

      if (forced_line_break) {
	wrapped_text += cur_line;
	cur_line = "";
	forced_line_break = false;
      }
    }
    
    // Don't forget to add the text left in cur_line
    if (cur_line != "") {
      wrapped_text += cur_line + '\n';
    }

    return wrapped_text;
  }


  SDL_Rect draw_wrapped_text(display* gui, const SDL_Rect& area, int font_size,
			     const SDL_Color& colour, const std::string& text,
			     int x, int y, int max_width, SDL_Surface* bg)
  {
    std::string wrapped_text = word_wrap_text(text, font_size, max_width);
    return font::draw_text(gui, area, font_size, colour, wrapped_text, x, y, bg, false, NO_MARKUP);
  }

}

//floating labels
namespace {

class floating_label
{
public:
	floating_label(const std::string& text, int font_size, const SDL_Color& colour,
	      int xpos, int ypos, int xmove, int ymove, int lifetime, const SDL_Rect& clip_rect)
		  : surf_(NULL), buf_(NULL), text_(text), font_size_(font_size), colour_(colour), xpos_(xpos), ypos_(ypos),
		    xmove_(xmove), ymove_(ymove), lifetime_(lifetime), clip_rect_(clip_rect),
			alpha_change_(-255/lifetime)
	{}

	void move(int xmove, int ymove);

	void draw(SDL_Surface* screen);
	void undraw(SDL_Surface* screen);

	bool expired() const;

	const std::string& text() const;

private:
	shared_sdl_surface surf_, buf_;
	std::string text_;
	int font_size_;
	SDL_Color colour_;
	int xpos_, ypos_, xmove_, ymove_;
	int lifetime_;
	SDL_Rect clip_rect_;
	int alpha_change_;
};

typedef std::map<int,floating_label> label_map;
label_map labels;
int label_id = 0;

bool hide_floating_labels = false;

void floating_label::move(int xmove, int ymove)
{
	xpos_ += xmove;
	ypos_ += ymove;
}

void floating_label::draw(SDL_Surface* screen)
{
	if(surf_ == NULL) {
		TTF_Font* const font = get_font(font_size_);
		if(font == NULL) {
			return;
		}

		surf_.assign(font::render_text(font,text_,colour_));
		if(surf_ == NULL) {
			return;
		}
	}

	if(buf_ == NULL) {
		buf_.assign(SDL_CreateRGBSurface(SDL_SWSURFACE,surf_->w,surf_->h,surf_->format->BitsPerPixel,
		                            surf_->format->Rmask,surf_->format->Gmask,surf_->format->Bmask,surf_->format->Amask));
		if(buf_ == NULL) {
			return;
		}
	}

	if(screen == NULL) {
		return;
	}

	SDL_Rect rect = {xpos_-surf_->w/2,ypos_,surf_->w,surf_->h};
	const clip_rect_setter clip_setter(screen,clip_rect_);
	SDL_BlitSurface(screen,&rect,buf_,NULL);
	SDL_BlitSurface(surf_,NULL,screen,&rect);
	update_rect(rect);
}

void floating_label::undraw(SDL_Surface* screen)
{
	if(screen == NULL || buf_ == NULL) {
		return;
	}

	SDL_Rect rect = {xpos_-surf_->w/2,ypos_,surf_->w,surf_->h};
	const clip_rect_setter clip_setter(screen,clip_rect_);
	SDL_BlitSurface(buf_,NULL,screen,&rect);

	update_rect(rect);

	move(xmove_,ymove_);
	if(lifetime_ > 0) {
		--lifetime_;
		if(surf_ != NULL && alpha_change_ != 0) {
			surf_.assign(adjust_surface_alpha_add(surf_,alpha_change_));
		}
	}
}

bool floating_label::expired() const { return lifetime_ == 0; }

const std::string& floating_label::text() const { return text_; }

}

namespace font {
int add_floating_label(const std::string& text, int font_size, const SDL_Color& colour,
					   int xpos, int ypos, int xmove, int ymove, int lifetime, const SDL_Rect& clip_rect)
{
	labels.insert(std::pair<int,floating_label>(label_id++,floating_label(text,font_size,colour,xpos,ypos,xmove,ymove,lifetime,clip_rect)));
	return label_id-1;
}

void move_floating_label(int handle, int xmove, int ymove)
{
	const label_map::iterator i = labels.find(handle);
	if(i != labels.end()) {
		i->second.move(xmove,ymove);
	}
}

void remove_floating_label(int handle)
{
	const label_map::iterator i = labels.find(handle);
	if(i != labels.end()) {
		labels.erase(i);
	}
}

const std::string& get_floating_label_text(int handle)
{
	const label_map::iterator i = labels.find(handle);
	if(i != labels.end()) {
		return i->second.text();
	} else {
		static const std::string empty_str;
		return empty_str;
	}
}

floating_label_manager::floating_label_manager()
{
}

floating_label_manager::~floating_label_manager()
{
	labels.clear();
}

floating_label_hider::floating_label_hider() : old_(hide_floating_labels)
{
	SDL_Surface* const screen = SDL_GetVideoSurface();
	if(screen != NULL) {
		draw_floating_labels(screen);
	}

	hide_floating_labels = true;
}

floating_label_hider::~floating_label_hider()
{
	hide_floating_labels = old_;

	SDL_Surface* const screen = SDL_GetVideoSurface();
	if(screen != NULL) {
		undraw_floating_labels(screen);
	}
}

void draw_floating_labels(SDL_Surface* screen)
{
	if(hide_floating_labels) {
		return;
	}

	//draw the labels in reverse order, so we can clear them in-order
	//to make sure the draw-undraw order is LIFO
	for(label_map::reverse_iterator i = labels.rbegin(); i != labels.rend(); ++i) {
		i->second.draw(screen);
	}
}

void undraw_floating_labels(SDL_Surface* screen)
{
	if(hide_floating_labels) {
		return;
	}

	for(label_map::iterator i = labels.begin(); i != labels.end(); ) {
		i->second.undraw(screen);
		if(i->second.expired()) {
			labels.erase(i++);
		} else {
			++i;
		}
	}
}

}