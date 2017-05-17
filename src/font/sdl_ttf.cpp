/*
   Copyright (C) 2016 - 2017 by Chris Beck<render787@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "font/sdl_ttf.hpp"

#include "font/error.hpp"
#include "font/font_config.hpp"
#include "font/font_id.hpp"
#include "font/text_cache.hpp"
#include "font/text_surface.hpp"

#include "filesystem.hpp"
#include "game_config.hpp"
#include "log.hpp"
#include "font/marked-up_text.hpp"
#include "preferences/general.hpp"
#include "tooltips.hpp"

#include "sdl/rect.hpp"
#include "sdl/surface.hpp"
#include "serialization/unicode.hpp"

#include <SDL_ttf.h>

#include <map>
#include <string>
#include <vector>

static lg::log_domain log_font("font");
#define DBG_FT LOG_STREAM(debug, log_font)
#define LOG_FT LOG_STREAM(info, log_font)
#define WRN_FT LOG_STREAM(warn, log_font)
#define ERR_FT LOG_STREAM(err, log_font)

namespace font {

/***
 * Caches used to speed up font rendering
 */

// Record stored in the font table.
// If the record for font_id (FOO, Bold + Underline) is a record (BAR, Bold),
// it means that BAR is a Bold-styled version of FOO which we shipped with the
// game, and now SDL_TTF should be used to style BAR as underline for the final results.
struct ttf_record
{
	TTF_Font* font;
	int style;
};

static std::map<font_id, ttf_record> font_table;
static std::vector<std::string> font_names;
static std::vector<std::string> bold_names;
static std::vector<std::string> italic_names;

struct char_block_map
{
	char_block_map()
		: cbmap()
	{
	}

	typedef std::pair<int, subset_id> block_t;
	typedef std::map<int, block_t> cbmap_t;
	cbmap_t cbmap;
	/** Associates not-associated parts of a range with a new font. */
	void insert(int first, int last, subset_id id)
	{
		if (first > last) return;
		cbmap_t::iterator i = cbmap.lower_bound(first);
		// At this point, either first <= i->first or i is past the end.
		if (i != cbmap.begin()) {
			cbmap_t::iterator j = i;
			--j;
			if (first <= j->second.first /* prev.last */) {
				insert(j->second.first + 1, last, id);
				return;
			}
		}
		if (i != cbmap.end()) {
			if (/* next.first */ i->first <= last) {
				insert(first, i->first - 1, id);
				return;
			}
		}
		cbmap.emplace(first, block_t(last, id));
	}
	/**
	 * Compresses map by merging consecutive ranges with the same font, even
	 * if there is some unassociated ranges in-between.
	 */
	void compress()
	{
		LOG_FT << "Font map size before compression: " << cbmap.size() << " ranges\n";
		cbmap_t::iterator i = cbmap.begin(), e = cbmap.end();
		while (i != e) {
			cbmap_t::iterator j = i;
			++j;
			if (j == e || i->second.second != j->second.second) {
				i = j;
				continue;
			}
			i->second.first = j->second.first;
			cbmap.erase(j);
		}
		LOG_FT << "Font map size after compression: " << cbmap.size() << " ranges\n";
	}
	subset_id get_id(int ch)
	{
		cbmap_t::iterator i = cbmap.upper_bound(ch);
		// At this point, either ch < i->first or i is past the end.
		if (i != cbmap.begin()) {
			--i;
			if (ch <= i->second.first /* prev.last */)
				return i->second.second;
		}
		return -1;
	}
};

static char_block_map char_blocks;

//cache sizes of small text
typedef std::map<std::string,SDL_Rect> line_size_cache_map;

//map of styles -> sizes -> cache
static std::map<int,std::map<int,line_size_cache_map> > line_size_cache;

typedef std::map<std::pair<std::string, int>, TTF_Font*> open_font_cache;
open_font_cache open_fonts;

static TTF_Font* open_font_impl(const std::string & , int);

// A wrapper which caches the results of open_font_impl.
// Note that clear_fonts() is responsible to clean up all of these font pointers,
// so to avoid memory leaks fonts should only be opened from this function.
static TTF_Font* open_font(const std::string& fname, int size)
{
	const std::pair<std::string, int> key = std::make_pair(fname, size);
	const open_font_cache::iterator it = open_fonts.find(key);
	if (it != open_fonts.end()) {
		return it->second;
	}

	TTF_Font* result = open_font_impl(fname, size);
	open_fonts.emplace(key, result);
	return result;
}

static TTF_Font* open_font_impl(const std::string & fname, int size) {
	std::string name;
	if(!game_config::path.empty()) {
		name = game_config::path + "/fonts/" + fname;
		if(!filesystem::file_exists(name)) {
			name = "fonts/" + fname;
			if(!filesystem::file_exists(name)) {
				name = fname;
				if(!filesystem::file_exists(name)) {
					ERR_FT << "Failed opening font: '" << name << "': No such file or directory" << std::endl;
					return nullptr;
				}
			}
		}

	} else {
		name = "fonts/" + fname;
		if(!filesystem::file_exists(name)) {
			if(!filesystem::file_exists(fname)) {
				ERR_FT << "Failed opening font: '" << name << "': No such file or directory" << std::endl;
				return nullptr;
			}
			name = fname;
		}
	}

	SDL_RWops *rwops = filesystem::load_RWops(name);
	TTF_Font* font = TTF_OpenFontRW(rwops, true, size); // SDL takes ownership of rwops
	if(font == nullptr) {
		ERR_FT << "Failed opening font: '" <<  fname << "'\n";
		ERR_FT << "TTF_OpenFont: " << TTF_GetError() << std::endl;
		return nullptr;
	}

	DBG_FT << "Opened a font: " << fname << std::endl;

	return font;
}

// Gets an appropriately configured TTF Font, for this font size and style.
// Loads fonts if necessary. For styled fonts, we search for a ``shipped''
// version of the font which is prestyled. If this fails we find the closest
// thing which we did ship, and store a record of this, which allows to
// rapidly correct the remaining styling using SDL_TTF.
//
// Uses the font table for caching.
TTF_Font* sdl_ttf::get_font(font_id id)
{
	const auto it = font_table.find(id);
	if(it != font_table.end()) {
		if (it->second.font != nullptr) {
			// If we found a valid record, use SDL_TTF to add in the difference
			// between its intrinsic style and the desired style.
			TTF_SetFontStyle(it->second.font, it->second.style ^ id.style);
		}
		return it->second.font;
	}

	// There's no record, so we need to try to find a solution for this font
	// and make a record of it. If the indices are out of bounds don't bother though.
	if(id.subset < 0 || size_t(id.subset) >= font_names.size()) {
		return nullptr;
	}

	// Favor to use the shipped Italic font over bold if both are present and are needed.
	if ((id.style & TTF_STYLE_ITALIC) && italic_names[id.subset].size()) {
		if (TTF_Font* font = open_font(italic_names[id.subset], id.size)) {
			ttf_record rec = {font, TTF_STYLE_ITALIC};
			font_table.emplace(id, rec);
			return sdl_ttf::get_font(id);
		}
	}

	// Now see if the shipped Bold font is useful and available.
	if ((id.style & TTF_STYLE_BOLD) && bold_names[id.subset].size()) {
		if (TTF_Font* font = open_font(bold_names[id.subset], id.size)) {
			ttf_record rec = {font, TTF_STYLE_BOLD};
			font_table.emplace(id, rec);
			return sdl_ttf::get_font(id);
		}
	}

	// Try just to use the basic version of the font then.
	if (font_names[id.subset].size()) {
		if(TTF_Font* font = open_font(font_names[id.subset], id.size)) {
			ttf_record rec = {font, TTF_STYLE_NORMAL};
			font_table.emplace(id, rec);
			return sdl_ttf::get_font(id);
		}
	}

	// Failed to find a font.
	ttf_record rec = {nullptr, TTF_STYLE_NORMAL};
	font_table.emplace(id, rec);
	return nullptr;
}


/***
 * Interface to SDL_TTF
 */

static surface render_text(const std::string& text, int fontsize, const color_t& color, int style, bool use_markup)
{
	// we keep blank lines and spaces (may be wanted for indentation)
	const std::vector<std::string> lines = utils::split(text, '\n', 0);
	std::vector<std::vector<surface> > surfaces;
	surfaces.reserve(lines.size());
	size_t width = 0, height = 0;

	for(std::vector< std::string >::const_iterator ln = lines.begin(), ln_end = lines.end(); ln != ln_end; ++ln) {

		int sz = fontsize;
		int text_style = style;

		std::string::const_iterator after_markup = use_markup ?
			parse_markup(ln->begin(), ln->end(), &sz, nullptr, &text_style) : ln->begin();
		text_surface txt_surf(sz, color, text_style);

		if (after_markup == ln->end() && (ln+1 != ln_end || lines.begin()+1 == ln_end)) {
			// we replace empty line by a space (to have a line height)
			// except for the last line if we have several
			txt_surf.set_text(" ");
		} else if (after_markup == ln->begin()) {
		 	// simple case, no markup to skip
			txt_surf.set_text(*ln);
		} else  {
			const std::string line(after_markup,ln->end());
			txt_surf.set_text(line);
		}

		const text_surface& cached_surf = text_cache::find(txt_surf);
		const std::vector<surface>&res = cached_surf.get_surfaces();

		if (!res.empty()) {
			surfaces.push_back(res);
			width = std::max<size_t>(cached_surf.width(), width);
			height += cached_surf.height();
		}
	}

	if (surfaces.empty()) {
		return surface();
	} else if (surfaces.size() == 1 && surfaces.front().size() == 1) {
		surface surf = surfaces.front().front();
		return surf;
	} else {
		surface res(create_compatible_surface(surfaces.front().front(),width,height));
		if (res.null())
			return res;

		size_t ypos = 0;
		for(std::vector< std::vector<surface> >::iterator i = surfaces.begin(),
		    i_end = surfaces.end(); i != i_end; ++i) {
			size_t xpos = 0;
			height = 0;

			for(std::vector<surface>::iterator j = i->begin(),
					j_end = i->end(); j != j_end; ++j) {
				SDL_Rect dstrect = sdl::create_rect(xpos, ypos, 0, 0);
				blit_surface(*j, nullptr, res, &dstrect);
				xpos += (*j)->w;
				height = std::max<size_t>((*j)->h, height);
			}
			ypos += height;
		}

		return res;
	}
}


surface get_rendered_text(const std::string& str, int size, const color_t& color, int style)
{
	// TODO maybe later also to parse markup here, but a lot to check
	return render_text(str, size, color, style, false);
}

SDL_Rect draw_text_line(surface& gui_surface, const SDL_Rect& area, int size,
		   const color_t& color, const std::string& text,
		   int x, int y, bool use_tooltips, int style)
{
	size = preferences::font_scaled(size);
	if (gui_surface.null()) {
		text_surface const &u = text_cache::find(text_surface(text, size, color, style));
		return sdl::create_rect(0, 0, u.width(), u.height());
	}

	if(area.w == 0) {  // no place to draw
		return {0, 0, 0, 0};
	}

	const std::string etext = make_text_ellipsis(text, size, area.w);

	// for the main current use, we already parsed markup
	surface surface(render_text(etext,size,color,style,false));
	if(surface == nullptr) {
		return {0, 0, 0, 0};
	}

	SDL_Rect dest;
	if(x!=-1) {
		dest.x = x;
#ifdef	HAVE_FRIBIDI
		// Oron -- Conditional, until all draw_text_line calls have fixed area parameter
		if(getenv("NO_RTL") == nullptr) {
			bool is_rtl = text_cache::find(text_surface(text, size, color, style)).is_rtl();
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

	if(gui_surface != nullptr) {
		SDL_Rect src = dest;
		src.x = 0;
		src.y = 0;
		sdl_blit(surface,&src,gui_surface,&dest);
	}

	if(use_tooltips) {
		tooltips::add_tooltip(dest,text);
	}

	return dest;
}

int get_max_height(int size)
{
	// Only returns the maximal size of the first font
	TTF_Font* const font = sdl_ttf::get_font(font_id(0, size));
	if(font == nullptr)
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

	const color_t col { 0, 0, 0, 0 };
	text_surface s(line, font_size, col, style);

	res.w = s.width();
	res.h = s.height();
	res.x = res.y = 0;

	cache.emplace(line,res);
	return res;
}

std::string make_text_ellipsis(const std::string &text, int font_size,
	int max_width, int style)
{
	if (line_width(text, font_size, style) <= max_width)
		return text;
	if(line_width(ellipsis, font_size, style) > max_width)
		return "";

	std::string current_substring;

	utf8::iterator itor(text);

	for(; itor != utf8::iterator::end(text); ++itor) {
		std::string tmp = current_substring;
		tmp.append(itor.substr().first, itor.substr().second);

		if (line_width(tmp + ellipsis, font_size, style) > max_width) {
			return current_substring + ellipsis;
		}

		current_substring.append(itor.substr().first, itor.substr().second);
	}

	return text; // Should not happen
}

void cache_mode(CACHE mode)
{
	if(mode == CACHE_LOBBY) {
		text_cache::resize(1000);
	} else {
		text_cache::resize(50);
	}
}

/***
 * Initialize and destruction
 */

sdl_ttf::sdl_ttf() {
	const int res = TTF_Init();
	if(res == -1) {
		ERR_FT << "Could not initialize SDL_TTF" << std::endl;
		throw font::error("SDL_TTF could not initialize, TTF_INIT returned: " + std::to_string(res));
	} else {
		LOG_FT << "Initialized true type fonts\n";
	}
}

static void clear_fonts() {
	for(const auto & i : open_fonts) {
		TTF_CloseFont(i.second);
	}

	open_fonts.clear();

	font_table.clear();

	font_names.clear();
	bold_names.clear();
	italic_names.clear();

	char_blocks.cbmap.clear();
	line_size_cache.clear();
}

sdl_ttf::~sdl_ttf() {
	clear_fonts();
	TTF_Quit();
}

//sets the font list to be used.
void sdl_ttf::set_font_list(const std::vector<subset_descriptor>& fontlist)
{
	clear_fonts();

	for(const auto & f : fontlist) {
		if (!check_font_file(f.name)) continue;
		// Insert fonts only if the font file exists
		const subset_id subset = font_names.size();
		font_names.push_back(f.name);

		if (f.bold_name && check_font_file(*f.bold_name)) {
			bold_names.push_back(*f.bold_name);
		} else {
			bold_names.emplace_back();
		}

		if (f.italic_name && check_font_file(*f.italic_name)) {
			italic_names.push_back(*f.italic_name);
		} else {
			italic_names.emplace_back();
		}

		for (const subset_descriptor::range &cp_range : f.present_codepoints) {
			char_blocks.insert(cp_range.first, cp_range.second, subset);
		}
	}
	char_blocks.compress();

	assert(font_names.size() == bold_names.size());
	assert(font_names.size() == italic_names.size());

	DBG_FT << "Set the font list. The styled font families are:\n";

	for (size_t i = 0; i < font_names.size(); ++i) {
		DBG_FT << "[" << i << "]:\t\tbase:\t'" << font_names[i] << "'\tbold:\t'" << bold_names[i] << "'\titalic:\t'" << italic_names[i] << "'\n";
	}
}

//Splits the UTF-8 text into text_chunks using the same font.
std::vector<text_chunk> sdl_ttf::split_text(const std::string& utf8_text) {
	text_chunk current_chunk(0);
	std::vector<text_chunk> chunks;

	if (utf8_text.empty())
		return chunks;

	try {
		utf8::iterator ch(utf8_text);
		int sub = char_blocks.get_id(*ch);
		if (sub >= 0) current_chunk.subset = sub;
		for(utf8::iterator end = utf8::iterator::end(utf8_text); ch != end; ++ch)
		{
			sub = char_blocks.get_id(*ch);
			if (sub >= 0 && sub != current_chunk.subset) {
				chunks.push_back(current_chunk);
				current_chunk.text.clear();
				current_chunk.subset = sub;
			}
			current_chunk.text.append(ch.substr().first, ch.substr().second);
		}
		if (!current_chunk.text.empty()) {
			chunks.push_back(current_chunk);
		}
	}
	catch(utf8::invalid_utf8_exception&) {
		WRN_FT << "Invalid UTF-8 string: \"" << utf8_text << "\"" << std::endl;
	}
	return chunks;
}

} // end namespace font
