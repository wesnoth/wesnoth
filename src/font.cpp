/* $Id$ */
/* vim:set encoding=utf-8: */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "config.hpp"
#include "filesystem.hpp"
#include "font.hpp"
#include "game_config.hpp"
#include "log.hpp"
#include "sdl_utils.hpp"
#include "tooltips.hpp"
#include "util.hpp"
#include "video.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "serialization/string_utils.hpp"
#include "marked-up_text.hpp"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <stack>
#include <string>

#define DBG_FT LOG_STREAM(debug, display)
#define LOG_FT LOG_STREAM(info, display)
#define WRN_FT LOG_STREAM(warn, display)
#define ERR_FT LOG_STREAM(err, display)

#ifdef	HAVE_FRIBIDI
#include <fribidi/fribidi.h>

#else

#endif

namespace {

// Signed int. Negative values mean "no subset".
typedef int subset_id;

struct font_id
{
	font_id(subset_id subset, int size) : subset(subset), size(size) {};
	bool operator==(const font_id& o) const
	{
		return subset == o.subset && size == o.size;
	};
	bool operator<(const font_id& o) const
	{
		return subset < o.subset || (subset == o.subset && size < o.size);
	};

	subset_id subset;
	int size;
};

std::map<font_id, TTF_Font*> font_table;
std::vector<std::string> font_names;

struct text_chunk
{
	text_chunk(subset_id subset) : subset(subset) {}
	text_chunk(subset_id subset, std::string const & text) : subset(subset), text(text) {}
	text_chunk(subset_id subset, ucs2_string const & ucs2_text) : subset(subset), ucs2_text(ucs2_text) {}
	text_chunk(subset_id subset, std::string const & text, ucs2_string const & ucs2_text) : subset(subset), text(text), ucs2_text(ucs2_text) {}

	bool operator==(text_chunk const & t) const { return subset == t.subset && ucs2_text == t.ucs2_text; }
	bool operator!=(text_chunk const & t) const { return !operator==(t); }

	subset_id subset;
	//FIXME if we don't need the utf8 here remove it
	std::string text;
	ucs2_string ucs2_text;
};

std::vector<subset_id> font_map;

//cache sizes of small text
typedef std::map<std::string,SDL_Rect> line_size_cache_map;

//map of styles -> sizes -> cache
std::map<int,std::map<int,line_size_cache_map> > line_size_cache;

}

//Splits the UTF-8 text into text_chunks using the same font.
static std::vector<text_chunk> split_text(std::string const & utf8_text) {
	text_chunk current_chunk(0);
	std::vector<text_chunk> chunks;

	if (utf8_text.empty())
		return chunks;

	try {
		utils::utf8_iterator ch(utf8_text);
		if(size_t(*ch) < font_map.size() && font_map[size_t(*ch)] >= 0) {
			current_chunk.subset = font_map[size_t(*ch)];
		}
		for(utils::utf8_iterator end = utils::utf8_iterator::end(utf8_text); ch != end; ++ch) {
			if(size_t(*ch) < font_map.size() &&
					font_map[size_t(*ch)] >= 0 &&
					font_map[size_t(*ch)] != current_chunk.subset) {
				//null-terminate ucs2_text so we can pass it to SDL_ttf later
				current_chunk.ucs2_text.push_back(0);
				chunks.push_back(current_chunk);
				current_chunk.text = "";
				current_chunk.ucs2_text.clear();
				current_chunk.subset = font_map[size_t(*ch)];
			}
			current_chunk.ucs2_text.push_back(static_cast<Uint16>(*ch));
			current_chunk.text.append(ch.substr().first, ch.substr().second);
		}
		if (!current_chunk.text.empty()) {
			chunks.push_back(current_chunk);
		}
	}
	catch(utils::invalid_utf8_exception e) {
		WRN_FT << "Invalid UTF-8 string: \"" << utf8_text << "\"\n";
	}
	return chunks;
}

static TTF_Font* open_font(const std::string& fname, int size)
{
	std::string name;
	if(!game_config::path.empty()) {
		name = game_config::path + "/fonts/" + fname;
		if(!file_exists(name)) {
			name = "fonts/" + fname;
			if(!file_exists(name)) {
				name = fname;
				if(!file_exists(name)) {
					ERR_FT << "Failed opening font: '" << name << "': No such file or directory\n";
					return NULL;
				}
			}
		}

	} else {
		name = "fonts/" + fname;
		if(!file_exists(name)) {
			if(!file_exists(fname)) {
				ERR_FT << "Failed opening font: '" << name << "': No such file or directory\n";
				return NULL;
			}
			name = fname;
		}
	}

	TTF_Font* font = TTF_OpenFont(name.c_str(),size);
	if(font == NULL) {
		ERR_FT << "Failed opening font: TTF_OpenFont: " << TTF_GetError() << "\n";
		return NULL;
	}

	return font;
}

static TTF_Font* get_font(font_id id)
{
	const std::map<font_id, TTF_Font*>::iterator it = font_table.find(id);
	if(it != font_table.end())
		return it->second;

	if(id.subset < 0 || size_t(id.subset) >= font_names.size())
		return NULL;

	TTF_Font* font = open_font(font_names[id.subset], id.size);

	if(font == NULL)
		return NULL;

	TTF_SetFontStyle(font,TTF_STYLE_NORMAL);

	LOG_FT << "Inserting font...\n";
	font_table.insert(std::pair<font_id,TTF_Font*>(id, font));
	return font;
}

static void clear_fonts()
{
	for(std::map<font_id,TTF_Font*>::iterator i = font_table.begin(); i != font_table.end(); ++i) {
		TTF_CloseFont(i->second);
	}

	font_table.clear();
	font_names.clear();
	font_map.clear();
	line_size_cache.clear();
}

namespace {

struct font_style_setter
{
	font_style_setter(TTF_Font* font, int style) : font_(font), old_style_(0)
	{
		if(style == 0) {
			style = TTF_STYLE_NORMAL;
		}

		old_style_ = TTF_GetFontStyle(font_);

		// I thought I had killed this. Now that we ship SDL_TTF, we
		// should fix the bug directly in SDL_ttf instead of disabling
		// features. -- Ayin 25/2/2005
#if 0
		//according to the SDL_ttf documentation, combinations of
		//styles may cause SDL_ttf to segfault. We work around this
		//here by disallowing combinations of styles

		if((style&TTF_STYLE_UNDERLINE) != 0) {
			//style = TTF_STYLE_NORMAL; //TTF_STYLE_UNDERLINE;
			style = TTF_STYLE_UNDERLINE;
		} else if((style&TTF_STYLE_BOLD) != 0) {
			style = TTF_STYLE_BOLD;
		} else if((style&TTF_STYLE_ITALIC) != 0) {
			//style = TTF_STYLE_NORMAL; //TTF_STYLE_ITALIC;
			style = TTF_STYLE_ITALIC;
		}
#endif

		TTF_SetFontStyle(font_, style);
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

//structure used to describe a font, and the subset of the Unicode character
//set it covers.
struct subset_descriptor
{
	std::string name;
	std::vector<std::pair<size_t, size_t> > present_codepoints;
};

//sets the font list to be used.
static void set_font_list(const std::vector<subset_descriptor>& fontlist)
{
	clear_fonts();
	font_map.reserve(0x10000);

	std::vector<subset_descriptor>::const_iterator itor;
	for(itor = fontlist.begin(); itor != fontlist.end(); ++itor) {
		// Insert fonts only if the font file exists
		if(game_config::path.empty() == false) {
			if(!file_exists(game_config::path + "/fonts/" + itor->name)) {
				if(!file_exists("fonts/" + itor->name)) {
					if(!file_exists(itor->name)) {
					WRN_FT << "Failed opening font file '" << itor->name << "': No such file or directory\n";
					continue;
					}
				}
			}
		} else {
			if(!file_exists("fonts/" + itor->name)) {
				if(!file_exists(itor->name)) {
					WRN_FT << "Failed opening font file '" << itor->name << "': No such file or directory\n";
					continue;
				}
			}
		}
		const subset_id subset = font_names.size();
		font_names.push_back(itor->name);

		std::vector<std::pair<size_t,size_t> >::const_iterator cp_range;
		for(cp_range = itor->present_codepoints.begin();
				cp_range != itor->present_codepoints.end(); ++cp_range) {

			size_t cp_max = maximum<size_t>(cp_range->first, cp_range->second);
			if(cp_max >= font_map.size()) {
				font_map.resize(cp_max+1, -1);
			}
			for(size_t cp = cp_range->first; cp <= cp_range->second; ++cp) {
				if(font_map[cp] < 0)
					font_map[cp] = subset;
			}
		}
	}
}

const SDL_Color NORMAL_COLOUR = {0xDD,0xDD,0xDD,0},
                GRAY_COLOUR   = {0x77,0x77,0x77,0},
                LOBBY_COLOUR  = {0xBB,0xBB,0xBB,0},
                GOOD_COLOUR   = {0x00,0xFF,0x00,0},
                BAD_COLOUR    = {0xFF,0x00,0x00,0},
                BLACK_COLOUR  = {0x00,0x00,0x00,0},
                YELLOW_COLOUR = {0xFF,0xFF,0x00,0},
                BUTTON_COLOUR = {0xBC,0xB0,0x88,0},
                STONED_COLOUR = {0xA0,0xA0,0xA0,0},
                TITLE_COLOUR  = {0xBC,0xB0,0x88,0},
				LABEL_COLOUR  = {0x6B,0x8C,0xFF,0},
				BIGMAP_COLOUR = {0xFF,0xFF,0xFF,0};
const SDL_Color DISABLED_COLOUR = inverse(STONED_COLOUR);

namespace {

static const size_t max_text_line_width = 4096;

class text_surface
{
public:
	text_surface(std::string const &str, int size, SDL_Color color, int style);
	text_surface(int size, SDL_Color color, int style);
	void set_text(std::string const &str);

	void measure() const;
	size_t width() const;
	size_t height() const;
#ifdef	HAVE_FRIBIDI
	bool is_rtl() const { return is_rtl_; }	// Right-To-Left alignment
#endif
	std::vector<surface> const & get_surfaces() const;

	bool operator==(text_surface const &t) const {
		return hash_ == t.hash_ && font_size_ == t.font_size_
			&& color_ == t.color_ && style_ == t.style_ && str_ == t.str_;
	}
	bool operator!=(text_surface const &t) const { return !operator==(t); }
private:
	int hash_;
	int font_size_;
	SDL_Color color_;
	int style_;
	mutable int w_, h_;
	std::string str_;
	mutable bool initialized_;
	mutable std::vector<text_chunk> chunks_;
	mutable std::vector<surface> surfs_;
#ifdef	HAVE_FRIBIDI
	bool is_rtl_;
	void bidi_cvt();
#endif
	void hash();
};

#ifdef	HAVE_FRIBIDI
void text_surface::bidi_cvt()
{
	char		*c_str = const_cast<char *>(str_.c_str());	// fribidi forgot const...
	int		len = str_.length();
	FriBidiChar	*bidi_logical = new FriBidiChar[len + 2];
	FriBidiChar	*bidi_visual = new FriBidiChar[len + 2];
	char		*utf8str = new char[4*len + 1];	//assume worst case here (all 4 Byte characters)
	FriBidiCharType	base_dir = FRIBIDI_TYPE_ON;
	int n;

	n = fribidi_utf8_to_unicode (c_str, len, bidi_logical);
	fribidi_log2vis(bidi_logical, n, &base_dir, bidi_visual, NULL, NULL, NULL);
	fribidi_unicode_to_utf8 (bidi_visual, n, utf8str);
	is_rtl_ = base_dir == FRIBIDI_TYPE_RTL;
	str_ = std::string(utf8str);
	delete[] bidi_logical;
	delete[] bidi_visual;
	delete[] utf8str;
}
#endif

text_surface::text_surface(std::string const &str, int size, SDL_Color color, int style)
  : font_size_(size), color_(color), style_(style), w_(-1), h_(-1), str_(str),
  initialized_(false)
{
#ifdef	HAVE_FRIBIDI
	bidi_cvt();
#endif
	hash();
}

text_surface::text_surface(int size, SDL_Color color, int style)
  : hash_(0), font_size_(size), color_(color), style_(style), w_(-1), h_(-1), initialized_(false)
{
}

void text_surface::set_text(std::string const &str)
{
	initialized_ = false;
	w_ = -1;
	h_ = -1;
	str_ = str;
#ifdef	HAVE_FRIBIDI
	bidi_cvt();
#endif
	hash();
}

void text_surface::hash()
{
	int h = 0;
	for(std::string::const_iterator it = str_.begin(), it_end = str_.end(); it != it_end; ++it)
		h = ((h << 9) | (h >> (sizeof(int) * 8 - 9))) ^ (*it);
	hash_ = h;
}

void text_surface::measure() const
{
	w_ = 0;
	h_ = 0;

	for(std::vector<text_chunk>::iterator itor = chunks_.begin();
			itor != chunks_.end(); ++itor) {

		TTF_Font* ttfont = get_font(font_id(itor->subset, font_size_));
		if(ttfont == NULL)
			continue;
		font_style_setter const style_setter(ttfont, style_);

		int w;
		int h;

		if(itor->ucs2_text.back() != 0) {
			itor->ucs2_text.push_back(0);
		}

		TTF_SizeUNICODE(ttfont, 
			static_cast<Uint16 const *>(&(itor->ucs2_text.front())), &w, &h);
		w_ += w;
		h_ = maximum<int>(h_, h);
	}
}

size_t text_surface::width() const
{
	if (w_ == -1) {
		if(chunks_.empty())
			chunks_ = split_text(str_);
		measure();
	}
	return w_;
}

size_t text_surface::height() const
{
	if (h_ == -1) {
		if(chunks_.empty())
			chunks_ = split_text(str_);
		measure();
	}
	return h_;
}

std::vector<surface> const &text_surface::get_surfaces() const
{
	if(initialized_)
		return surfs_;

	initialized_ = true;

	// Impose a maximal number of characters for a text line. Do now draw
	// any text longer that that, to prevent a SDL buffer overflow
	if(width() > max_text_line_width)
		return surfs_;

	for(std::vector<text_chunk>::const_iterator itor = chunks_.begin();
			itor != chunks_.end(); ++itor) {
		TTF_Font* ttfont = get_font(font_id(itor->subset, font_size_));
		if (ttfont == NULL)
			continue;
		font_style_setter const style_setter(ttfont, style_);

		surface s = surface(TTF_RenderUNICODE_Blended(ttfont, 
			static_cast<Uint16 const *>(&(itor->ucs2_text.front())), color_));
		if(!s.null())
			surfs_.push_back(s);
	}

	return surfs_;
}

class text_cache
{
public:
	static text_surface &find(text_surface const &t);
	static void resize(unsigned int size);
private:
	typedef std::list< text_surface > text_list;
	static text_list cache_;
	static unsigned int max_size_;
};

text_cache::text_list text_cache::cache_;
unsigned int text_cache::max_size_ = 50;

void text_cache::resize(unsigned int size)
{
	DBG_FT << "Text cache: resize from: " << max_size_ << " to: "
		<< size << " items in cache: " << cache_.size() << '\n';

	while(size < cache_.size()) {
		cache_.pop_back();
	}
	max_size_ = size;
}


text_surface &text_cache::find(text_surface const &t)
{
	static size_t lookup_ = 0, hit_ = 0;
	text_list::iterator it_bgn = cache_.begin(), it_end = cache_.end();
	text_list::iterator it = std::find(it_bgn, it_end, t);
	if (it != it_end) {
		cache_.splice(it_bgn, cache_, it);
		++hit_;
	} else {
		if (cache_.size() >= max_size_)
			cache_.pop_back();
		cache_.push_front(t);
	}
	if (++lookup_ % 1000 == 0) {
		DBG_FT << "Text cache: " << lookup_ << " lookups, " << (hit_ / 10) << "% hits\n";
		hit_ = 0;
	}
	return cache_.front();
}

}

static surface render_text(const std::string& text, int fontsize, const SDL_Color& colour, int style)
{
	const std::vector<std::string> lines = utils::split(text, '\n', utils::REMOVE_EMPTY);
	std::vector<std::vector<surface> > surfaces;
	surfaces.reserve(lines.size());
	size_t width = 0, height = 0;
	text_surface txt_surf(fontsize, colour, style);

	for(std::vector< std::string >::const_iterator ln = lines.begin(), ln_end = lines.end(); ln != ln_end; ++ln) {
		if (!ln->empty()) {
			txt_surf.set_text(*ln);
			const text_surface& cached_surf = text_cache::find(txt_surf);
			const std::vector<surface>&res = cached_surf.get_surfaces();

			if (!res.empty()) {
				surfaces.push_back(res);
				width = maximum<size_t>(cached_surf.width(), width);
				height += cached_surf.height();
			}
		}
	}

	if (surfaces.empty()) {
		return surface();
	} else if (surfaces.size() == 1 && surfaces.front().size() == 1) {
		surface surf = surfaces.front().front();
		SDL_SetAlpha(surf, SDL_SRCALPHA | SDL_RLEACCEL, SDL_ALPHA_OPAQUE);
		return surf;
	} else {

		surface res(create_compatible_surface(surfaces.front().front(),width,height));
		if (res.null())
			return res;

		size_t ypos = 0;
		for(std::vector< std::vector<surface> >::const_iterator i = surfaces.begin(),
		    i_end = surfaces.end(); i != i_end; ++i) {
			size_t xpos = 0;
			size_t height = 0;

			for(std::vector<surface>::const_iterator j = i->begin(),
					j_end = i->end(); j != j_end; ++j) {
				SDL_SetAlpha(*j, 0, 0); // direct blit without alpha blending
				SDL_Rect dstrect = {xpos, ypos, 0, 0};
				SDL_BlitSurface(*j, NULL, res, &dstrect);
				xpos += (*j)->w;
				height = maximum<size_t>((*j)->h, height);
			}
			ypos += height;
		}

		return res;
	}
}


surface get_rendered_text(const std::string& str, int size, const SDL_Color& colour, int style)
{
	return render_text(str, size, colour, style);
}

SDL_Rect draw_text_line(surface gui_surface, const SDL_Rect& area, int size,
		   const SDL_Color& colour, const std::string& text,
		   int x, int y, bool use_tooltips, int style)
{
	const std::string etext = make_text_ellipsis(text, size, area.w);

	if (gui_surface.null()) {
		text_surface const &u = text_cache::find(text_surface(text, size, colour, style));
		SDL_Rect res = {0, 0, u.width(), u.height()};
		return res;
	}

	surface surface(render_text(etext,size,colour,style));
	if(surface == NULL) {
		SDL_Rect res = {0,0,0,0};
		return res;
	}

	SDL_Rect dest;
	if(x!=-1) {
		dest.x = x;
#ifdef	HAVE_FRIBIDI
		// Oron -- Conditional, until all draw_text_line calls have fixed area parameter
		if(getenv("NO_RTL") == NULL) {
			bool is_rtl = text_cache::find(text_surface(text, size, colour, style)).is_rtl();
			if(is_rtl)
				dest.x = area.x + area.w - surface->w - (x - area.x);
		}
#endif
	} else
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

SDL_Rect draw_text_line(CVideo* gui, const SDL_Rect& area, int size,
                        const SDL_Color& colour, const std::string& text,
                        int x, int y, bool use_tooltips, int style)
{
	surface surface;

	if(gui == NULL) {
		surface = NULL;
	} else {
		surface = gui->getSurface();
	}

	return draw_text_line(surface, area, size, colour, text, x, y, use_tooltips, style);
}

int get_max_height(int size)
{
	// Only returns the maximal size of the first font
	TTF_Font* const font = get_font(font_id(0, size));
	if(font == NULL)
		return 0;
	return TTF_FontHeight(font);
}

int line_width(const std::string& line, int font_size, int style)
{
	return line_size(line,font_size,style).w;
}

SDL_Rect line_size(const std::string& line, int font_size, int style)
{
	line_size_cache_map& cache = line_size_cache[style][font_size];

	const line_size_cache_map::const_iterator i = cache.find(line);
	if(i != cache.end()) {
		return i->second;
	}

	SDL_Rect res;

	const SDL_Color col = { 0, 0, 0, 0 };
	text_surface s(line, font_size, col, style);

	res.w = s.width();
	res.h = s.height();
	res.x = res.y = 0;

	cache.insert(std::pair<std::string,SDL_Rect>(line,res));
	return res;
}

std::string make_text_ellipsis(const std::string &text, int font_size, int max_width, bool with_tags)
{
	static const std::string ellipsis = "...";

	if(line_width(with_tags ? text : del_tags(text), font_size) <= max_width)
		return text;
	if(line_width(ellipsis, font_size) > max_width)
		return "";

	std::string current_substring;

	utils::utf8_iterator itor(text);

	for(; itor != utils::utf8_iterator::end(text); ++itor) {
		std::string tmp = current_substring;
		tmp.append(itor.substr().first, itor.substr().second);
		tmp += ellipsis;

		if (line_width(with_tags ? tmp : del_tags(tmp), font_size) > max_width) {
			return current_substring + ellipsis;
		}

		current_substring.append(itor.substr().first, itor.substr().second);
	}

	return text; // Should not happen
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

	bool expired() const { return lifetime_ == 0; }


	void show(const bool value) { visible_ = value; }

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
		foreground_ = font::render_text(text_, font_size_, colour_, 0);

		if(foreground_ == NULL) {
			return NULL;
		}

		//if the surface has to be created onto some kind of background, then do that here
		if(bgalpha_ != 0) {
			surface tmp(create_compatible_surface(foreground_,foreground_->w+border_*2,foreground_->h+border_*2));
			if(tmp == NULL) {
				return NULL;
			}

			SDL_FillRect(tmp,NULL,SDL_MapRGBA(tmp->format,bgcolour_.r,bgcolour_.g,bgcolour_.b, bgalpha_));

			surf_.assign(tmp);
		} else {
			surface background = font::render_text(text_, font_size_, font::BLACK_COLOUR, 0);
			background = blur_alpha_surface(background,2);
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
			// we don't compress these surfaces since they will always change
			if (!surf_.null()) {
				surf_.assign(adjust_surface_alpha_add(surf_,alpha_change_,false));
			}
			if (!foreground_.null()) {
				foreground_.assign(adjust_surface_alpha_add(foreground_,alpha_change_,false));
			}
		}
	}
}

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

SDL_Rect get_floating_label_rect(int handle)
{
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

static bool add_font_to_fontlist(config* fonts_config, std::vector<font::subset_descriptor>& fontlist, const std::string& name)
	{
		config* font = fonts_config->find_child("font", "name", name);
		if(font == NULL)
			return false;

		fontlist.push_back(font::subset_descriptor());
		fontlist.back().name = name;
		std::vector<std::string> ranges = utils::split((*font)["codepoints"]);

		for(std::vector<std::string>::const_iterator itor = ranges.begin();
				itor != ranges.end(); ++itor) {

			std::vector<std::string> r = utils::split(*itor, '-');
			if(r.size() == 1) {
				size_t r1 = lexical_cast_default<size_t>(r[0], 0);
				fontlist.back().present_codepoints.push_back(std::pair<size_t, size_t>(r1, r1));
			} else if(r.size() == 2) {
				size_t r1 = lexical_cast_default<size_t>(r[0], 0);
				size_t r2 = lexical_cast_default<size_t>(r[1], 0);

				fontlist.back().present_codepoints.push_back(std::pair<size_t, size_t>(r1, r2));
			}
		}

		return true;
	}

namespace font {

bool load_font_config()
{
	//read font config separately, so we do not have to re-read the whole
	//config when changing languages
	config cfg;
	try {
		scoped_istream stream = preprocess_file("data/hardwired/fonts.cfg");
		read(cfg, *stream);
	} catch(config::error &e) {
		ERR_FT << "could not read fonts.cfg:\n"
		       << e.message << '\n';
		return false;
	}

	config* fonts_config = cfg.child("fonts");
	if(fonts_config == NULL)
		return false;

	std::set<std::string> known_fonts;
	const config::child_list fonts = fonts_config->get_children("font");
	for (config::child_list::const_iterator child = fonts.begin(); child != fonts.end(); ++child) {
		known_fonts.insert((**child)["name"]);
	}

	const std::vector<std::string> font_order = utils::split((*fonts_config)["order"]);
	std::vector<font::subset_descriptor> fontlist;
	std::vector<std::string>::const_iterator font;
	for(font = font_order.begin(); font != font_order.end(); ++font) {
		add_font_to_fontlist(fonts_config, fontlist, *font);
		known_fonts.erase(*font);
	}
	std::set<std::string>::const_iterator kfont;
	for(kfont = known_fonts.begin(); kfont != known_fonts.end(); ++kfont) {
		add_font_to_fontlist(fonts_config, fontlist, *kfont);
	}

	if(fontlist.empty())
		return false;

	font::set_font_list(fontlist);
	return true;
}

void cache_mode(CACHE mode)
{
	if(mode == CACHE_LOBBY) {
		text_cache::resize(1000);
	} else {
		text_cache::resize(50);
	}
}


}
