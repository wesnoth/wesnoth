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
#include "tooltips.hpp"

#include <cstdio>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

namespace {

std::map<int,TTF_Font*> font_table;

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

	TTF_Font* font = open_font("Vera.ttf",size);

	if(font == NULL) return NULL;

	TTF_SetFontStyle(font,TTF_STYLE_NORMAL);

	std::cerr << "Inserting font...\n";
	font_table.insert(std::pair<int,TTF_Font*>(size,font));
	return font;
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
	for(std::map<int,TTF_Font*>::iterator i = font_table.begin();
	    i != font_table.end(); ++i) {
		TTF_CloseFont(i->second);
	}

	TTF_Quit();
}

const SDL_Color NORMAL_COLOUR = {0xDD,0xDD,0xDD,0},
                GOOD_COLOUR   = {0x00,0xFF,0x00,0},
                BAD_COLOUR    = {0xFF,0x00,0x00,0},
                BLACK_COLOUR  = {0x00,0x00,0x00,0},
                DARK_COLOUR   = {0x00,0x00,0x66,0},
                YELLOW_COLOUR = {0xFF,0xFF,0x00,0},
                BUTTON_COLOUR = {0xBC,0xB0,0x88,0};

const SDL_Color& get_side_colour(int side)
{
	side -= 1;

	static const SDL_Color sides[] = { {0xFF,0x00,0x00,0},
	                                   {0x00,0x00,0xFF,0},
	                                   {0x00,0xFF,0x00,0},
	                                   {0xFF,0xFF,0x00,0},
	                                   {0xFF,0x55,0x55,0},
	                                   {0xFF,0x55,0x55,0} };

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
		SDL_BlitSurface(surface,&src,gui->video().getSurface(),&dest);
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
	for(;;) {
		if(i1 != i2) {
			SDL_Color col = colour;
			int sz = size;
			if(use_markup == USE_MARKUP) {
				if(*i1 == '#') {
					col = BAD_COLOUR;
					++i1;
				} else if(*i1 == '@') {
					col = GOOD_COLOUR;
					++i1;
				} else if(*i1 == '{') {
					col = NORMAL_COLOUR;
					++i1;
				} else if(*i1 == '}') {
					col = BLACK_COLOUR;
					++i1;
				} else if(*i1 == '+') {
					sz += 2;
					++i1;
				} else if(*i1 == '-') {
					sz -= 2;
					++i1;
				}

				if(i1 != i2 && *i1 >= 1 && *i1 <= 9) {
					col = get_side_colour(*i1);
					++i1;
				}
			}

			if(i1 != i2) {
				const std::string new_string(i1,i2);
				const SDL_Rect rect =
				    draw_text_line(gui,area,sz,col,new_string,x,y,bg,
				                   use_tooltips);
				if(rect.w > res.w)
					res.w = rect.w;
				res.h += rect.h;
				y += rect.h;
			}
		}

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
	case '#':
	case '@':
		return true;
	default:
		return false;
	}
}

}
