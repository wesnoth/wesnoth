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

#include "wesconfig.h"
#include "global.hpp"

#include "font.hpp"
#include "filesystem.hpp"
#include "game_config.hpp"
#include "language.hpp"
#include "log.hpp"
#include "sdl_utils.hpp"
#include "team.hpp"
#include "tooltips.hpp"
#include "util.hpp"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <stack>
#include <string>

#define LOG_FT lg::info(lg::display)
#define WRN_FT lg::warn(lg::display)
#define ERR_FT lg::err(lg::display)

namespace {

std::map<int,TTF_Font*> font_table;
std::string font_name = "Vera.ttf";

//SDL_ttf seems to have a problem where TTF_OpenFont will seg fault if
//the font file doesn't exist, so make sure it exists first.
TTF_Font* open_font(const std::string& fname, int size)
{
	std::string name;

	LOG_FT << "Opening font '" << fname << "' ...\n";
#ifndef USE_ZIPIOS
	if(game_config::path.empty() == false) {
		name = game_config::path + "/fonts/" + fname;
		LOG_FT << "Trying file '" << name << "' ...\n";

		if (!file_exists(name)) {
			name = "fonts/" + fname;
			WRN_FT << "Failed opening '" << name << "'; now trying '" << name << "' ...\n";
			if (!file_exists(name)) {
				ERR_FT << "Failed opening font: " << fname << "\n";
				return NULL;
			}
		}
	} else {
		name = "fonts/" + fname;
		LOG_FT << "Trying file '" << name << "' ...\n";

		if(!file_exists(name)) {
			ERR_FT << "Failed opening font '" << fname << "'\n";
			return NULL;
		}
	}

	LOG_FT << "Opening font file '" << name << "', font size is " << size << "\n";

	TTF_Font* font = TTF_OpenFont(name.c_str(),size);
#else
	std::string tmp = read_file("fonts/" + fname);
	if (tmp.empty()) {
		ERR_FT << "Failed opening font file '" << fname << "'\n";
		return NULL;
	}
	// the following statement would leak memory if fonts were closed
	std::string *s = new std::string(tmp);
	SDL_RWops* ops = SDL_RWFromMem((void*)s->c_str(), s->size());
	TTF_Font* font = TTF_OpenFontRW(ops, 0, size);
#endif
	if(font == NULL) {
		ERR_FT << "Failed opening font file '" << name << "'\n";
		return NULL;
	}

	LOG_FT << "Opened font okay\n";

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

	LOG_FT << "Inserting font...\n";
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

		//according to the SDL_ttf documentation, combinations of styles may cause
		//SDL_ttf to segfault. We work around this here by disallowing combinations
		//of styles
		if((style&TTF_STYLE_UNDERLINE) != 0) {
			//style = TTF_STYLE_NORMAL; //TTF_STYLE_UNDERLINE;
			style = TTF_STYLE_UNDERLINE;
		} else if((style&TTF_STYLE_BOLD) != 0) {
			style = TTF_STYLE_BOLD;
		} else if((style&TTF_STYLE_ITALIC) != 0) {
			//style = TTF_STYLE_NORMAL; //TTF_STYLE_ITALIC;
			style = TTF_STYLE_ITALIC;
		} 

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
		ERR_FT << "Could not initialize true type fonts\n";
		throw error();
	} else {
		LOG_FT << "Initialized true type fonts\n";
	}
}

manager::~manager()
{
	clear_fonts();
	TTF_Quit();
}

void set_font()
{
	clear_fonts();
	font_name = _("Vera.ttf");
}

const SDL_Color NORMAL_COLOUR = {0xDD,0xDD,0xDD,0},
                GOOD_COLOUR   = {0x00,0xFF,0x00,0},
                BAD_COLOUR    = {0xFF,0x00,0x00,0},
                BLACK_COLOUR  = {0x00,0x00,0x00,0},
                DARK_COLOUR   = {0x00,0x00,0x66,0},
                YELLOW_COLOUR = {0xFF,0xFF,0x00,0},
                BUTTON_COLOUR = {0xBC,0xB0,0x88,0},
                TITLE_COLOUR  = {0xBC,0xB0,0x88,0};

const char LARGE_TEXT='*', SMALL_TEXT='`', GOOD_TEXT='@', BAD_TEXT='#',
           NORMAL_TEXT='{', BLACK_TEXT='}', BOLD_TEXT='~', IMAGE='&', NULL_MARKUP='^';

namespace {

static const size_t max_text_line_width = 4096;

class text_surface
{
public:
	text_surface(TTF_Font *font, std::string const &str, SDL_Color color, int style);
	text_surface(TTF_Font *font, SDL_Color color, int style);
	void set_text(std::string const &str);
	bool operator==(text_surface const &t) const { return hash_ == t.hash_ && equal(t); }
	bool operator!=(text_surface const &t) const { return hash_ != t.hash_ || !equal(t); }
	size_t width() const;
	size_t height() const;
	surface const &get_surface() const;
private:
	int hash_;
	TTF_Font *font_; // no font should ever be closed
	std::string str_;
	SDL_Color color_;
	int style_;
	mutable int w_, h_;
	mutable surface surf_;
	void hash();
	void measure() const;
	bool equal(text_surface const &t) const;
};

text_surface::text_surface(TTF_Font *font, std::string const &str, SDL_Color color, int style)
  : font_(font), str_(str), color_(color), style_(style), w_(-1), h_(-1)
{
	hash();
}

text_surface::text_surface(TTF_Font *font, SDL_Color color, int style)
  : hash_(0), font_(font), color_(color), style_(style), w_(-1), h_(-1)
{}

void text_surface::set_text(std::string const &str)
{
	str_ = str;
	hash();
}

void text_surface::hash()
{
	int h = 0;
	for(std::string::const_iterator it = str_.begin(), it_end = str_.end(); it != it_end; ++it)
		h = ((h << 9) | (h >> (sizeof(int) * 8 - 9))) ^ (*it);
	hash_ = h;
}

bool text_surface::equal(text_surface const &t) const
{
	return font_ == t.font_
		&& color_.r == t.color_.r && color_.g == t.color_.g && color_.b == t.color_.b
		&& style_ == t.style_
		&& str_ == t.str_;
}

void text_surface::measure() const
{
	font_style_setter const style_setter(font_, style_);
	TTF_SizeUTF8(font_, str_.c_str(), &w_, &h_);
}

size_t text_surface::width() const
{
	if (w_ == -1)
		measure();
	return w_;
}

size_t text_surface::height() const
{
	if (h_ == -1)
		measure();
	return h_;
}

surface const &text_surface::get_surface() const
{
	if (!surf_.null())
		return surf_;

	// Impose a maximal number of characters for a text line. Do now draw
	// any text longer that that, to prevent a SDL buffer overflow
	if (width() > max_text_line_width)
		return surf_;

	// Validate the UTF-8 string: workaround a SDL_TTF bug that makes it
	// crash when used with an invalid UTF-8 string
	wide_string ws = string_to_wstring(str_);

	for(wide_string::const_iterator itor = ws.begin(); itor != ws.end(); ++itor) {
		int minx, miny, maxx, maxy, advance;	

		if(TTF_GlyphMetrics(font_, *itor, &minx, &maxx, &miny, &maxy, &advance) != 0 ) {
			std::cerr << "glyph with strange size: " << *itor << "(" << char(*itor) << ") " << minx << ", " << maxx  << ", " << miny << ", " << maxy << ", " << advance << "\n";
		}
	}

	std::string fixed_str = wstring_to_string(string_to_wstring(str_));

	font_style_setter const style_setter(font_, style_);
	surf_ = surface(TTF_RenderUTF8_Blended(font_, fixed_str.c_str(), color_));
	return surf_;
}

class text_cache
{
public:
	static text_surface &find(text_surface const &t);
private:
	typedef std::list< text_surface > text_list;
	static text_list cache_;
};

text_cache::text_list text_cache::cache_;

text_surface &text_cache::find(text_surface const &t)
{
	static size_t lookup_ = 0, hit_ = 0;
	text_list::iterator it_bgn = cache_.begin(), it_end = cache_.end();
	text_list::iterator it = std::find(it_bgn, it_end, t);
	if (it != it_end) {
		cache_.splice(it_bgn, cache_, it);
		++hit_;
	} else {
		if (cache_.size() >= 50)
			cache_.pop_back();
		cache_.push_front(t);
	}
	if (++lookup_ % 1000 == 0) {
		LOG_FT << "Text cache: " << lookup_ << " lookups, " << (hit_ / 10) << "% hits\n";
		hit_ = 0;
	}
	return cache_.front();
}

surface render_text(TTF_Font* font,const std::string& text, const SDL_Color& colour, int style)
{
	if (font == NULL)
		return surface();

	// XXX Changed by erl, to not strip when rendering text. Works everywhere?
	const std::vector<std::string> lines = utils::split(text, '\n', utils::REMOVE_EMPTY);
	std::vector<surface> surfaces;
	surfaces.reserve(lines.size());
	size_t width = 0, height = 0;
	text_surface txt_surf(font, colour, style);

	for(std::vector< std::string >::const_iterator ln = lines.begin(), ln_end = lines.end(); ln != ln_end; ++ln) {
		if (!ln->empty()) {
			txt_surf.set_text(*ln);
			surface const &res = text_cache::find(txt_surf).get_surface();

			if (!res.null()) {
				surfaces.push_back(res);
				width = maximum<size_t>(res->w,width);
				height += res->h;
			}
		}
	}

	if (surfaces.empty()) {
		return surface();
	} else if (surfaces.size() == 1) {
		return surfaces.front();
	} else {

		surface res(create_compatible_surface(surfaces.front(),width,height));
		if (res.null())
			return res;

		size_t ypos = 0;
		for(std::vector< surface >::const_iterator i = surfaces.begin(),
		    i_end = surfaces.end(); i != i_end; ++i) {
			SDL_SetAlpha(*i, 0, 0); // direct blit without alpha blending
			SDL_Rect dstrect = {0, ypos, 0, 0};
			SDL_BlitSurface(*i, NULL, res, &dstrect);
			ypos += (*i)->h;
		}

		return res;
	}
}

//function which will parse the markup tags at the front of a string
std::string::const_iterator parse_markup(std::string::const_iterator i1, std::string::const_iterator i2,
										 int* font_size, SDL_Color* colour, int* style)
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
		case BOLD_TEXT:
			*style |= TTF_STYLE_BOLD;
			break;
		case NULL_MARKUP:
			return i1+1;
		default:
			if(*i1 >= 1 && *i1 <= 9) {
				*colour = team::get_side_colour(*i1);
				break;
			}

			return i1;
		}

		++i1;
	}

	return i1;
}

}


surface get_rendered_text(const std::string& str, int size, const SDL_Color& colour, int style)
{
	TTF_Font* const font = get_font(size);
	if(font == NULL) {
		ERR_FT << "Could not get font for size " << size << "\n";
		return NULL;
	}

	return render_text(font, str, colour, style);
}


SDL_Rect draw_text_line(surface gui_surface, const SDL_Rect& area, int size,
		   const SDL_Color& colour, const std::string& text,
		   int x, int y, bool use_tooltips, int style)
{
	
	TTF_Font* const font = get_font(size);
	if(font == NULL) {
		ERR_FT << "Could not get font for size " << size << "\n";
		SDL_Rect res = {0,0,0,0};
		return res;
	}

	const std::string etext = make_text_ellipsis(text, size, area.w);

	if (gui_surface.null()) {
		text_surface const &u = text_cache::find(text_surface(font, text, colour, style));
		SDL_Rect res = {0, 0, u.width(), u.height()};
		return res;
	}

	surface surface(render_text(font,etext,colour,style));
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
		dest.w = area.x + area.w - dest.x;
	}

	if(dest.y + dest.h > area.y + area.h) {
		dest.h = area.y + area.h - dest.y;
	}

	if(gui_surface != NULL) {
		SDL_Rect src = dest;
		src.x = 0;
		src.y = 0;
		SDL_BlitSurface(surface,&src,gui_surface,&dest);
	}

	if(use_tooltips) {
		tooltips::add_tooltip(dest,text);
	}

	return dest;
}

SDL_Rect draw_text_line(display* gui, const SDL_Rect& area, int size,
                        const SDL_Color& colour, const std::string& text,
                        int x, int y, bool use_tooltips, int style)
{
	surface surface;
	
	if(gui == NULL) {
		surface = NULL;
	} else {
		surface = gui->video().getSurface();
	}
	
	return draw_text_line(surface, area, size, colour, text, x, y, use_tooltips, style);
}

SDL_Rect text_area(const std::string& text, int size, int style)
{
	const SDL_Rect area = {0,0,10000,10000};
	return draw_text(NULL, area, size, font::NORMAL_COLOUR, text, 0, 0, false, style);
}

SDL_Rect draw_text(display* gui, const SDL_Rect& area, int size,
                   const SDL_Color& colour, const std::string& txt,
                   int x, int y, bool use_tooltips, int style)
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
		int text_style = style;

		i1 = parse_markup(i1,i2,&sz,&col,&text_style);

		if(i1 != i2) {
			std::string new_string(i1,i2);

			utils::unescape(new_string);

			const SDL_Rect rect = draw_text_line(gui, area, sz, col, new_string, x, y, use_tooltips, text_style);
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

	int line_width(const std::string line, int font_size, int style)
	{
    
		TTF_Font* const font = get_font(font_size);
		if(font == NULL) {
			ERR_FT << "Could not get font for size " << font_size << "\n";
			return 0;
		}
		int w = 0;
		int h = 0;
  
		font_style_setter style_setter(font, style);
		TTF_SizeUTF8(font, line.c_str(), &w, &h);

		return w;
	}

  
	std::string word_wrap_text(const std::string& unwrapped_text, int font_size, int max_width)
	{
		//std::cerr << "Wrapping word " << unwrapped_text << "\n";
		
		std::string wrapped_text; // the final result
  
		size_t word_start_pos = 0;
		std::string cur_word; // including start-whitespace
		std::string cur_line; // the whole line so far
  
		for(size_t c = 0; c < unwrapped_text.length(); c++) {

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
			     int x, int y, int max_width)
	{
		std::string wrapped_text = word_wrap_text(text, font_size, max_width);
		return font::draw_text(gui, area, font_size, colour, wrapped_text, x, y, false);
	}

}

//floating labels
namespace {

class floating_label
{
public:
	floating_label(const std::string& text, int font_size, const SDL_Color& colour, const SDL_Color& bgcolour,
			double xpos, double ypos, double xmove, double ymove, int lifetime, const SDL_Rect& clip_rect,
			font::ALIGN align, int border_size, bool scroll_with_map)
		: surf_(NULL), buf_(NULL), foreground_(NULL), text_(text), font_size_(font_size), colour_(colour),
		bgcolour_(bgcolour), bgalpha_(bgcolour.unused), xpos_(xpos), ypos_(ypos),
		xmove_(xmove), ymove_(ymove), lifetime_(lifetime), clip_rect_(clip_rect),
		alpha_change_(-255/lifetime), visible_(true), align_(align), border_(border_size), scroll_(scroll_with_map)
	{}

	void move(double xmove, double ymove);

	void draw(surface screen);
	void undraw(surface screen);

	surface create_surface();

	bool expired() const;

	const std::string& text() const;

	void show(bool value);

	bool scroll() const { return scroll_; }

private:

	int xpos(size_t width) const;

	surface surf_, buf_, foreground_;
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
	bool scroll_;
};

typedef std::map<int,floating_label> label_map;
label_map labels;
int label_id = 1;

std::stack<std::set<int> > label_contexts;

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

surface floating_label::create_surface()
{
	if (surf_.null()) {
		foreground_ = font::render_text(get_font(font_size_), text_, colour_, 0);

		if(foreground_ == NULL) {
			return NULL;
		}

		//if the surface has to be created onto some kind of background, then do that here
		if(bgalpha_ != 0) {
			surface tmp(create_compatible_surface(foreground_,foreground_->w+border_*2,foreground_->h+border_*2));
			if(tmp == NULL) {
				return NULL;
			}

			SDL_FillRect(tmp,NULL,SDL_MapRGB(tmp->format,bgcolour_.r,bgcolour_.g,bgcolour_.b));
			if(bgalpha_ != 255) {
				tmp.assign(adjust_surface_alpha_add(tmp,bgalpha_ - 255));
				if(tmp == NULL) {
					return NULL;
				}
			}

			surf_.assign(tmp);
		} else {
			surface background = font::render_text(get_font(font_size_), text_, font::BLACK_COLOUR, 0);
			background = blur_surface(background,4);
			background = adjust_surface_alpha(background, ftofxp(4.0));

			surf_.assign(background);
		}

	}

	return surf_;
}

void floating_label::draw(surface screen)
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

	update_rect(rect);
}

void floating_label::undraw(surface screen)
{
	if(screen == NULL || buf_ == NULL) {
		return;
	}

	SDL_Rect rect = {xpos(surf_->w),int(ypos_),surf_->w,surf_->h};
	const clip_rect_setter clip_setter(screen,clip_rect_);
	SDL_BlitSurface(buf_,NULL,screen,&rect);

	update_rect(rect);

	move(xmove_,ymove_);
	if(lifetime_ > 0) {
		--lifetime_;
		if(alpha_change_ != 0 && (xmove_ != 0.0 || ymove_ != 0.0)) {
			if (!surf_.null()) {
				surf_.assign(adjust_surface_alpha_add(surf_,alpha_change_));
			}
			if (!foreground_.null()) {
				foreground_.assign(adjust_surface_alpha_add(foreground_,alpha_change_));
			}
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
		const SDL_Color* bg_colour, int border_size, LABEL_SCROLL_MODE scroll_mode)
{
	if(label_contexts.empty()) {
		return 0;
	}

	if(lifetime <= 0) {
		lifetime = -1;
	}

	SDL_Color bg = {0,0,0,0};
	if(bg_colour != NULL) {
		bg = *bg_colour;
	}

	++label_id;
	labels.insert(std::pair<int,floating_label>(label_id,floating_label(text,font_size,colour,bg,xpos,ypos,xmove,ymove,lifetime,clip_rect,align,border_size,scroll_mode == ANCHOR_LABEL_MAP)));
	label_contexts.top().insert(label_id);
	return label_id;
}

void move_floating_label(int handle, double xmove, double ymove)
{
	const label_map::iterator i = labels.find(handle);
	if(i != labels.end()) {
		i->second.move(xmove,ymove);
	}
}

void scroll_floating_labels(double xmove, double ymove)
{
	for(label_map::iterator i = labels.begin(); i != labels.end(); ++i) {
		if(i->second.scroll()) {
			i->second.move(xmove,ymove);
		}
	}
}

void remove_floating_label(int handle)
{
	const label_map::iterator i = labels.find(handle);
	if(i != labels.end()) {
		if(label_contexts.empty() == false) {
			label_contexts.top().erase(i->first);
		}

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
		const surface surf = i->second.create_surface();
		if(surf != NULL) {
			SDL_Rect rect = {0,0,surf->w,surf->h};
			return rect;
		}
	}

	return empty_rect;
}

floating_label_context::floating_label_context()
{
	surface const screen = SDL_GetVideoSurface();
	if(screen != NULL) {
		draw_floating_labels(screen);
	}

	label_contexts.push(std::set<int>());
}

floating_label_context::~floating_label_context()
{
	const std::set<int>& labels = label_contexts.top();
	for(std::set<int>::const_iterator i = labels.begin(); i != labels.end(); ) {
		remove_floating_label(*i++);
	}

	label_contexts.pop();

	surface const screen = SDL_GetVideoSurface();
	if(screen != NULL) {
		undraw_floating_labels(screen);
	}
}

void draw_floating_labels(surface screen)
{
	if(label_contexts.empty()) {
		return;
	}

	const std::set<int>& context = label_contexts.top();

	//draw the labels in the order they were added, so later added labels (likely to be tooltips)
	//are displayed over earlier added labels.
	for(label_map::iterator i = labels.begin(); i != labels.end(); ++i) {
		if(context.count(i->first) > 0) {
			i->second.draw(screen);
		}
	}
}

void undraw_floating_labels(surface screen)
{
	if(label_contexts.empty()) {
		return;
	}

	std::set<int>& context = label_contexts.top();

	//undraw labels in reverse order, so that a LIFO process occurs, and the screen is restored
	//into the exact state it started in.
	for(label_map::reverse_iterator i = labels.rbegin(); i != labels.rend(); ++i) {
		if(context.count(i->first) > 0) {
			i->second.undraw(screen);
		}
	}

	//remove expired labels
	for(label_map::iterator j = labels.begin(); j != labels.end(); ) {
		if(context.count(j->first) > 0 && j->second.expired()) {
			context.erase(j->first);
			labels.erase(j++);
		} else {
			j++;
		}
	}
}

}
