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
#include "util.hpp"

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

	std::cerr << "opening font '" << name << "' " << size << "\n";

	TTF_Font* font = TTF_OpenFont(name.c_str(),size);
	if(font == NULL) {
		std::cerr << "Could not open font file: " << name << '\n';
	}

	std::cerr << "opened font okay\n";

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

struct font_style_setter
{
	font_style_setter(TTF_Font* font, int style) : font_(font), old_style_(0)
	{
		if(style == 0) {
			style = TTF_STYLE_NORMAL;
		}

		old_style_ = TTF_GetFontStyle(font_);
		TTF_SetFontStyle(font_,style);
	}

	~font_style_setter()
	{
		TTF_SetFontStyle(font_,old_style_);
	}

private:
	TTF_Font* font_;
	int old_style_;
};

}

namespace font {

manager::manager()
{
	const int res = TTF_Init();
	if(res == -1) {
		std::cerr << "Could not initialize true type fonts\n";
		throw error();
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

const char LARGE_TEXT='*', SMALL_TEXT='`', GOOD_TEXT='@', BAD_TEXT='#',
           NORMAL_TEXT='{', BLACK_TEXT='}', IMAGE='&', NULL_MARKUP='^';


const SDL_Color& get_side_colour(int side)
{
	side -= 1;

	static const SDL_Color sides[] = { {0xFF,0x00,0x00,0},
	                                   {0x00,0x00,0xFF,0},
	                                   {0x00,0xFF,0x00,0},
	                                   {0xFF,0xFF,0x00,0},
	                                   {0xFF,0x00,0xFF,0},
					   {0xFF,0x7F,0x00,0},
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

SDL_Rect text_size(TTF_Font* font, const std::string& str, const SDL_Color& colour, int style)
{
	const font_style_setter style_setter(font,style);
	SDL_Rect res = {0,0,0,0};
	int w = 0, h = 0;

	TTF_SizeUTF8(font,str.c_str(),&w,&h);

	res.w = size_t(w);
	res.h = size_t(h);

	return res;
}

SDL_Surface* render_text_internal(TTF_Font* font,const std::string& str,
						 const SDL_Color& colour, int style)
{
	const font_style_setter style_setter(font,style);

	return TTF_RenderUTF8_Blended(font,str.c_str(),colour);

}

SDL_Surface* render_text(TTF_Font* font,const std::string& text, const SDL_Color& colour, int style)
{
	const std::vector<std::string> lines = config::split(text,'\n');
	std::vector<shared_sdl_surface> surfaces;
	size_t width = 0, height = 0;
	for(std::vector<std::string>::const_iterator ln = lines.begin(); ln != lines.end(); ++ln) {
		if(*ln != "" && font != NULL) {
			shared_sdl_surface res(render_text_internal(font,*ln,colour,0));

			if(res != NULL) {
				surfaces.push_back(res);
				width = maximum<size_t>(res->w,width);
				height += res->h;
			}
		}
	}

	if(surfaces.empty()) {
		return NULL;
	} else if(surfaces.size() == 1) {
		sdl_add_ref(surfaces.front());
		return surfaces.front();
	} else {

		shared_sdl_surface res(create_compatible_surface(surfaces.front(),width,height));
		if(res == NULL) {
			return NULL;
		}

		size_t ypos = 0;
		for(std::vector<shared_sdl_surface>::const_iterator i = surfaces.begin(); i != surfaces.end(); ++i) {
			SDL_SetAlpha(*i,0,0);
			SDL_Rect dstrect = {0,ypos,(*i)->w,(*i)->h};
			SDL_BlitSurface(*i,NULL,res,&dstrect);
			ypos += (*i)->h;
		}

		sdl_add_ref(res);
		return res;
	}
}

//function which will parse the markup tags at the front of a string
std::string::const_iterator parse_markup(std::string::const_iterator i1, std::string::const_iterator i2,
										 int* font_size, SDL_Color* colour)
{
	if(font_size == NULL || colour == NULL) {
		return i1;
	}

	while(i1 != i2) {
		switch(*i1) {
		case '\\':
			// this must either be a quoted special character or a
			// quoted backslash - either way, remove leading backslash
			break;
		case BAD_TEXT:
			*colour = BAD_COLOUR;
			break;
		case GOOD_TEXT:
			*colour = GOOD_COLOUR;
			break;
		case NORMAL_TEXT:
			*colour = NORMAL_COLOUR;
			break;
		case BLACK_TEXT:
			*colour = BLACK_COLOUR;
			break;
		case LARGE_TEXT:
			*font_size += 2;
			break;
		case SMALL_TEXT:
			*font_size -= 2;
			break;
		case NULL_MARKUP:
			return i1+1;
		default:
			if(*i1 >= 1 && *i1 <= 9) {
				*colour = get_side_colour(*i1);
				break;
			}

			return i1;
		}

		++i1;
	}

	return i1;
}

}


SDL_Surface* get_rendered_text(const std::string& str, int size, const SDL_Color& colour, int style)
{
	TTF_Font* const font = get_font(size);
	if(font == NULL) {
		std::cerr << "Could not get font\n";
		return NULL;
	}

	SDL_Surface *res = render_text(font,str,colour,style);
	if(res == NULL) {
		return NULL;
	}
	return res;
}


SDL_Rect draw_text_line(SDL_Surface *gui_surface, const SDL_Rect& area, int size,
		   const SDL_Color& colour, const std::string& text,
		   int x, int y, SDL_Surface* bg, bool use_tooltips, int style)
{
	
	TTF_Font* const font = get_font(size);
	if(font == NULL) {
		std::cerr << "Could not get font\n";
		SDL_Rect res = {0,0,0,0};
		return res;
	}

	const std::string etext = make_text_ellipsis(text, size, area.w);

	if(gui_surface == NULL) {
		return text_size(font,etext,colour,style);
	}

	scoped_sdl_surface surface(render_text(font,etext,colour,style));
	if(surface == NULL) {
		SDL_Rect res = {0,0,0,0};
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

	if(line_width(text, size) > area.w) {
		tooltips::add_tooltip(dest,text);
	}
	
	if(dest.x + dest.w > area.x + area.w) {
		std::cerr << "Error: text with ellipsis does not fit !\n";
		dest.w = area.x + area.w - dest.x;
	}

	if(dest.y + dest.h > area.y + area.h) {
		dest.h = area.y + area.h - dest.y;
	}

	if(gui_surface != NULL) {
		SDL_Rect src = dest;
		src.x = 0;
		src.y = 0;
		sdl_safe_blit(surface,&src,gui_surface,&dest);
	}

	if(use_tooltips) {
		tooltips::add_tooltip(dest,text);
	}

	return dest;
}

SDL_Rect draw_text_line(display* gui, const SDL_Rect& area, int size,
                        const SDL_Color& colour, const std::string& text,
                        int x, int y, SDL_Surface* bg, bool use_tooltips, int style)
{
	SDL_Surface* surface;
	
	if(gui == NULL) {
		surface = NULL;
	} else {
		surface = gui->video().getSurface();
	}
	
	return draw_text_line(surface, area, size, colour, text, x, y, bg, use_tooltips, style);
}

SDL_Rect text_area(const std::string& text, int size, int style)
{
	const SDL_Rect area = {0,0,10000,10000};
	return draw_text(NULL,area,size,font::NORMAL_COLOUR,text,0,0,NULL,false,USE_MARKUP,style);
}

SDL_Rect draw_text(display* gui, const SDL_Rect& area, int size,
                   const SDL_Color& colour, const std::string& txt,
                   int x, int y, SDL_Surface* bg, bool use_tooltips,
                   MARKUP use_markup, int style)
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
	for(;;) {
		SDL_Color col = colour;
		int sz = size;

		i1 = parse_markup(i1,i2,&sz,&col);

		if(i1 != i2) {
			std::string new_string(i1,i2);

			config::unescape(new_string);

			const SDL_Rect rect = draw_text_line(gui,area,sz,col,new_string,x,y,bg,use_tooltips,style);
			if(rect.w > res.w) {
				res.w = rect.w;
			}

			res.h += rect.h;
			y += rect.h;
		}

		if(i2 == text.end()) {
			break;
		}

		i1 = i2+1;
		i2 = std::find(i1,text.end(),'\n');
	}

	return res;
}

int get_max_height(int size)
{
	TTF_Font* const font = get_font(size);
	return TTF_FontHeight(font);
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
  
		TTF_SizeUTF8(font, line.c_str(), &w, &h);

		return w;
	}

  
	std::string word_wrap_text(const std::string& unwrapped_text, int font_size, int max_width)
	{
		std::cerr << "Wrapping word " << unwrapped_text << "\n";
		
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
					std::vector<std::string> split_word = split_utf8_string(cur_word);
		
					for (std::vector<std::string>::iterator i = split_word.begin(); i != split_word.end(); ++i) {
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

	std::string make_text_ellipsis(const std::string &text, int font_size, int max_width)
	{
		static const std::string ellipsis = "...";
		
		if(line_width(text, font_size) <= max_width)
			return text;
		if(line_width(ellipsis, font_size) > max_width)
			return "";
		
		std::vector<std::string> characters = split_utf8_string(text);
		std::string current_substring = "";

		for(std::vector<std::string>::const_iterator itor = characters.begin(); itor != characters.end(); ++itor) {
			if (line_width(current_substring + *itor + ellipsis, font_size ) > max_width) {
				return current_substring + ellipsis;
			}
			
			current_substring += *itor;
		}

		return text; // Should not happen
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
	floating_label(const std::string& text, int font_size, const SDL_Color& colour, const SDL_Color& bgcolour,
		double xpos, double ypos, double xmove, double ymove, int lifetime, const SDL_Rect& clip_rect, font::ALIGN align,
		int border_size)
		  : surf_(NULL), buf_(NULL), foreground_(NULL), text_(text), font_size_(font_size), colour_(colour), bgcolour_(bgcolour), bgalpha_(bgcolour.unused), xpos_(xpos), ypos_(ypos),
		    xmove_(xmove), ymove_(ymove), lifetime_(lifetime), clip_rect_(clip_rect),
			alpha_change_(-255/lifetime), visible_(true), align_(align), border_(border_size)
	{}

	void move(double xmove, double ymove);

	void draw(SDL_Surface* screen);
	void undraw(SDL_Surface* screen);

	SDL_Surface* create_surface();

	bool expired() const;

	const std::string& text() const;

	void show(bool value);

private:

	int xpos(size_t width) const;

	shared_sdl_surface surf_, buf_, foreground_;
	std::string text_;
	int font_size_;
	SDL_Color colour_, bgcolour_;
	int bgalpha_;
	double xpos_, ypos_, xmove_, ymove_;
	int lifetime_;
	SDL_Rect clip_rect_;
	int alpha_change_;
	bool visible_;
	font::ALIGN align_;
	int border_;
};

typedef std::map<int,floating_label> label_map;
label_map labels;
int label_id = 1;

bool hide_floating_labels = false;

void floating_label::move(double xmove, double ymove)
{
	xpos_ += xmove;
	ypos_ += ymove;
}

int floating_label::xpos(size_t width) const
{
	int xpos = int(xpos_);
	if(align_ == font::CENTER_ALIGN) {
		xpos -= width/2;
	} else if(align_ == font::RIGHT_ALIGN) {
		xpos -= width;
	}

	return xpos;
}

SDL_Surface* floating_label::create_surface()
{
	if(surf_ == NULL) {
		std::cerr << "creating surface...\n";

		const std::vector<std::string> lines = config::split(text_,'\n');
		std::vector<shared_sdl_surface> surfaces;
		for(std::vector<std::string>::const_iterator ln = lines.begin(); ln != lines.end(); ++ln) {
			SDL_Color colour = colour_;
			int size = font_size_;
			const std::string::const_iterator i1 = font::parse_markup(ln->begin(),ln->end(),&size,&colour);
			const std::string str(i1,ln->end());

			TTF_Font* const font = get_font(size);

			if(str != "" && font != NULL) {
				surfaces.push_back(shared_sdl_surface(font::render_text(font,str,colour,0)));
			}
		}

		if(surfaces.empty()) {
			return NULL;
		} else if(surfaces.size() == 1) {
			surf_.assign(surfaces.front());
		} else {
			size_t width = 0, height = 0;
			std::vector<shared_sdl_surface>::const_iterator i;
			for(i = surfaces.begin(); i != surfaces.end(); ++i) {
				width = maximum<size_t>((*i)->w,width);
				height += (*i)->h;
			}

			const SDL_PixelFormat* const format = surfaces.front()->format;
			surf_.assign(create_compatible_surface(surfaces.front(),width,height));

			size_t ypos = 0;
			for(i = surfaces.begin(); i != surfaces.end(); ++i) {
				SDL_SetAlpha(*i,0,0);
				SDL_Rect dstrect = {0,ypos,(*i)->w,(*i)->h};
				SDL_BlitSurface(*i,NULL,surf_,&dstrect);
				ypos += (*i)->h;
			}
		}

		if(surf_ == NULL) {
			return NULL;
		}

		//if the surface has to be created onto some kind of background, then do that here
		if(bgalpha_ != 0) {
			std::cerr << "creating bg...\n";
			shared_sdl_surface tmp(create_compatible_surface(surf_,surf_->w+border_*2,surf_->h+border_*2));
			if(tmp == NULL) {
				return NULL;
			}

			SDL_FillRect(tmp,NULL,SDL_MapRGB(tmp.get()->format,bgcolour_.r,bgcolour_.g,bgcolour_.b));
			if(bgalpha_ != 255) {
				tmp.assign(adjust_surface_alpha_add(tmp.get(),bgalpha_ - 255));
				if(tmp == NULL) {
					return NULL;
				}
			}

			foreground_.assign(surf_);
			surf_.assign(tmp);
		}
	}

	return surf_;
}

void floating_label::draw(SDL_Surface* screen)
{
	if(!visible_) {
		buf_.assign(NULL);
		return;
	}

	create_surface();
	if(surf_ == NULL) {
		return;
	}

	if(buf_ == NULL) {
		buf_.assign(create_compatible_surface(surf_));
		if(buf_ == NULL) {
			return;
		}
	}

	if(screen == NULL) {
		return;
	}

	SDL_Rect rect = {xpos(surf_->w),int(ypos_),surf_->w,surf_->h};
	const clip_rect_setter clip_setter(screen,clip_rect_);
	SDL_BlitSurface(screen,&rect,buf_,NULL);
	SDL_BlitSurface(surf_,NULL,screen,&rect);

	if(foreground_ != NULL) {
		SDL_Rect rect = {xpos(surf_->w)+border_,int(ypos_)+border_,foreground_->w,foreground_->h};
		SDL_BlitSurface(foreground_,NULL,screen,&rect);
	}

	std::cerr << "update\n";
	update_rect(rect);
	std::cerr << "done\n";
}

void floating_label::undraw(SDL_Surface* screen)
{
	if(screen == NULL || buf_ == NULL) {
		return;
	}

	std::cerr << "undraw....\n";

	SDL_Rect rect = {xpos(surf_->w),int(ypos_),surf_->w,surf_->h};
	const clip_rect_setter clip_setter(screen,clip_rect_);
	SDL_BlitSurface(buf_,NULL,screen,&rect);

	update_rect(rect);

	std::cerr << "done undraw\n";

	move(xmove_,ymove_);
	if(lifetime_ > 0) {
		--lifetime_;
		if(surf_ != NULL && alpha_change_ != 0 && (xmove_ != 0.0 || ymove_ != 0.0)) {
			surf_.assign(adjust_surface_alpha_add(surf_,alpha_change_));
		}
	}
}

bool floating_label::expired() const { return lifetime_ == 0; }

const std::string& floating_label::text() const { return text_; }

void floating_label::show(bool value) { visible_ = value; }

}

namespace font {
int add_floating_label(const std::string& text, int font_size, const SDL_Color& colour,
					   double xpos, double ypos, double xmove, double ymove, int lifetime, const SDL_Rect& clip_rect, ALIGN align,
					   const SDL_Color* bg_colour, int border_size)
{
	if(lifetime <= 0) {
		lifetime = -1;
	}

	SDL_Color bg = {0,0,0,0};
	if(bg_colour != NULL) {
		bg = *bg_colour;
	}

	labels.insert(std::pair<int,floating_label>(label_id++,floating_label(text,font_size,colour,bg,xpos,ypos,xmove,ymove,lifetime,clip_rect,align,border_size)));
	return label_id-1;
}

void move_floating_label(int handle, double xmove, double ymove)
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

void show_floating_label(int handle, bool value)
{
	const label_map::iterator i = labels.find(handle);
	if(i != labels.end()) {
		i->second.show(value);
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

SDL_Rect get_floating_label_rect(int handle)
{
	static const SDL_Rect empty_rect = {0,0,0,0};
	const label_map::iterator i = labels.find(handle);
	if(i != labels.end()) {
		const SDL_Surface* const surf = i->second.create_surface();
		if(surf != NULL) {
			SDL_Rect rect = {0,0,surf->w,surf->h};
			return rect;
		}
	}

	return empty_rect;
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

	//draw the labels in the order they were added, so later added labels (likely to be tooltips)
	//are displayed over earlier added labels.
	for(label_map::iterator i = labels.begin(); i != labels.end(); ++i) {
		i->second.draw(screen);
	}
}

void undraw_floating_labels(SDL_Surface* screen)
{
	if(hide_floating_labels) {
		return;
	}

	//undraw labels in reverse order, so that a LIFO process occurs, and the screen is restored
	//into the exact state it started in.
	for(label_map::reverse_iterator i = labels.rbegin(); i != labels.rend(); ++i) {
		i->second.undraw(screen);
	}

	//remove expired labels
	for(label_map::iterator j = labels.begin(); j != labels.end(); ) {
		if(j->second.expired()) {
			labels.erase(j++);
		} else {
			j++;
		}
	}
}

}
