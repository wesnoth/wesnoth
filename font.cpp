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
#include "SDL_ttf.h"

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
	if(read_file(fname).empty())
		return NULL;

	return TTF_OpenFont(fname.c_str(),size);
}

TTF_Font* get_font(int size)
{
	const std::map<int,TTF_Font*>::iterator it = font_table.find(size);
	if(it != font_table.end())
		return it->second;

	TTF_Font* font = NULL;
	std::cerr << "opening font file...\n";

#ifdef WESNOTH_PATH
	font = open_font(std::string(WESNOTH_PATH) + "/images/misc/Vera.ttf",size);
#endif

	if(font == NULL) {
		font = open_font("images/misc/Vera.ttf",size);
	}

	if(font == NULL) {
		std::cerr << "Could not open font file\n";
		return font;
	}

	TTF_SetFontStyle(font,TTF_STYLE_NORMAL);

	std::cerr << "inserting font...\n";
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

SDL_Rect draw_text_line(display* gui, const SDL_Rect& area, int size,
                        COLOUR colour, const std::string& text, int x, int y,
						SDL_Surface* bg)
{
	static const SDL_Color colours[] =
	//     neutral         good          bad
	    { {0xFF,0xFF,0,0}, {0,0xFF,0,0}, {0xFF,0,0,0} };

	const SDL_Color& col = colours[colour];
	TTF_Font* const font = get_font(size);
	if(font == NULL) {
		std::cerr << "Could not get font\n";
		SDL_Rect res;
		res.x = 0; res.y = 0; res.w = 0; res.h = 0;
		return res;
	}
	
	SDL_Surface* const surface = TTF_RenderText_Blended(font,text.c_str(),col);
	if(surface == NULL) {
		std::cerr << "Could not render ttf: '" << text << "'\n";
		SDL_Rect res;
		res.x = 0; res.y = 0; res.w = 0; res.h = 0;
		return res;
	}

	SDL_Rect dest;
	dest.x = x;
	dest.y = y;
	dest.w = surface->w;
	dest.h = surface->h;
	
	if(dest.x + dest.w > area.x + area.w) {
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

	SDL_FreeSurface(surface);

	return dest;
}


SDL_Rect draw_text(display* gui, const SDL_Rect& area, int size,
                   COLOUR colour, const std::string& txt, int x, int y,
                   SDL_Surface* bg)
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
			COLOUR col = NORMAL_COLOUR;
			int sz = size;
			if(*i1 == '#') {
				col = BAD_COLOUR;
				++i1;
			} else if(*i1 == '@') {
				col = GOOD_COLOUR;
				++i1;
			} else if(*i1 == '+') {
				sz += 2;
				++i1;
			} else if(*i1 == '-') {
				sz -= 2;
				++i1;
			}
			
			if(i1 != i2) {
				const std::string new_string(i1,i2);
				const SDL_Rect rect =
				    draw_text_line(gui,area,sz,col,new_string,x,y,bg);
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

}
