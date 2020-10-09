/*
   Copyright (C) 2016 - 2018 by Chris Beck<render787@gmail.com>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "font/marked-up_text.hpp"
#include "game_config.hpp"
#include "log.hpp"
#include "preferences/general.hpp"
#include "tooltips.hpp"

#include "sdl/rect.hpp"
#include "sdl/surface.hpp"
#include "serialization/unicode.hpp"

#include <SDL2/SDL_ttf.h>

#include <map>
#include <string>
#include <vector>

static lg::log_domain log_font("font");
#define DBG_FT LOG_STREAM(debug, log_font)
#define LOG_FT LOG_STREAM(info, log_font)
#define WRN_FT LOG_STREAM(warn, log_font)
#define ERR_FT LOG_STREAM(err, log_font)

namespace font
{
namespace
{
// Record stored in the font table.
// If the record for font_id (FOO, Bold + Underline) is a record (BAR, Bold),
// it means that BAR is a Bold-styled version of FOO which we shipped with the
// game, and now SDL_TTF should be used to style BAR as underline for the final results.
struct ttf_record
{
	std::shared_ptr<TTF_Font> font;
	int style;
};
static std::map<font_id, ttf_record> font_table;

// The indices in these vectors correspond to the font_id.subset values in font_table.
static std::vector<std::string> font_names;
static std::vector<std::string> bold_names;
static std::vector<std::string> italic_names;

struct family_record
{
	std::shared_ptr<const TTF_Font> font;
	subset_id subset;
	std::string name;
};
/**
 * Used for implementing find_font_containing, the elements are in the same order as the arguments
 * to set_font_list(). The fonts here are a subset of those in font_table, because
 * find_font_containing doesn't need size-specific instances of a font.
 *
 * In most locales, the subset_ids will match the indices into this vector.  This is only a
 * coincidence, and it won't be true (at the time of writing) in Chinese.
 *
 * \todo Are all variants of a font guaranteed to have exactly the same glyphs? For example, might
 * an italic variant only contain the glyphs which are major improvements on an automatic skew of
 * the non-italic version?
 */
std::vector<family_record> family_table;

const auto no_font_found = family_record{nullptr, -1, ""};
/**
 * Given a unicode code point, returns the first (using the order passed to set_font_list) font
 * that includes that code point. Returns no_font_found if none of the known fonts contain this value.
 */
const family_record& find_font_containing(int ch)
{
	for(const auto& i : family_table) {
		if(TTF_GlyphIsProvided(i.font.get(), ch)) {
			return i;
		}
	}
	LOG_FT << "Glyph " << ch << " not provided by any font\n";
	return no_font_found;
}

// cache sizes of small text
typedef std::map<std::string, SDL_Rect> line_size_cache_map;

// map of styles -> sizes -> cache
static std::map<int, std::map<int, line_size_cache_map>> line_size_cache;

/**
 * Destructor for using std::unique_ptr or std::shared_ptr as an RAII holder for a TTF_Font.
 */
struct font_deleter
{
	void operator()(TTF_Font* font)
	{
		if(font != nullptr)
			TTF_CloseFont(font);
	}
};

std::shared_ptr<TTF_Font> open_font(const std::string& fname, int size)
{
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

	filesystem::rwops_ptr rwops = filesystem::make_read_RWops(name);
	std::unique_ptr<TTF_Font, font_deleter> font;
	font.reset(TTF_OpenFontRW(rwops.release(), true, size)); // SDL takes ownership of rwops
	if(font == nullptr) {
		ERR_FT << "Failed opening font: '" << fname << "'\n";
		ERR_FT << "TTF_OpenFont: " << TTF_GetError() << std::endl;
		return nullptr;
	}

	DBG_FT << "Opened a font: " << fname << ", in size " << size << std::endl;

	return font;
}

} // anonymous namespace

// Gets an appropriately configured TTF Font, for this font size and style.
// Loads fonts if necessary. For styled fonts, we search for a ``shipped''
// version of the font which is prestyled. If this fails we find the closest
// thing which we did ship, and store a record of this, which allows to
// rapidly correct the remaining styling using SDL_TTF.
//
// Uses the font table for caching.
std::shared_ptr<TTF_Font> sdl_ttf::get_font(font_id id)
{
	const auto it = font_table.find(id);
	if(it != font_table.end() && it->second.font != nullptr) {
		return it->second.font;
	}

	// There's no record, so we need to try to find a solution for this font
	// and make a record of it. If the indices are out of bounds don't bother though.
	if(id.subset < 0 || std::size_t(id.subset) >= font_names.size()) {
		return nullptr;
	}

	// Favor to use the shipped Italic font over bold if both are present and are needed.
	if((id.style & TTF_STYLE_ITALIC) && italic_names[id.subset].size()) {
		if(auto font = open_font(italic_names[id.subset], id.size)) {
			ttf_record rec{font, TTF_STYLE_ITALIC};
			// The next line adds bold if needed
			TTF_SetFontStyle(font.get(), id.style ^ TTF_STYLE_ITALIC);
			font_table.emplace(id, rec);
			return font;
		}
	}

	// Now see if the shipped Bold font is useful and available.
	if((id.style & TTF_STYLE_BOLD) && bold_names[id.subset].size()) {
		if(auto font = open_font(bold_names[id.subset], id.size)) {
			ttf_record rec{font, TTF_STYLE_BOLD};
			// The next line adds italic if needed
			TTF_SetFontStyle(font.get(), id.style ^ TTF_STYLE_BOLD);
			font_table.emplace(id, rec);
			return font;
		}
	}

	// Try just to use the basic version of the font then.
	if(font_names[id.subset].size()) {
		if(auto font = open_font(font_names[id.subset], id.size)) {
			ttf_record rec{font, TTF_STYLE_NORMAL};
			TTF_SetFontStyle(font.get(), id.style);
			font_table.emplace(id, rec);
			return font;
		}
	}

	// Failed to find a font.
	ttf_record rec{nullptr, TTF_STYLE_NORMAL};
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
	std::vector<std::vector<surface>> surfaces;
	surfaces.reserve(lines.size());
	std::size_t width = 0, height = 0;

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
			width = std::max<std::size_t>(cached_surf.width(), width);
			height += cached_surf.height();
		}
	}

	if (surfaces.empty()) {
		return surface();
	} else if (surfaces.size() == 1 && surfaces.front().size() == 1) {
		surface surf = surfaces.front().front();
		return surf;
	} else {
		surface res(width,height);
		if (!res)
			return res;

		std::size_t ypos = 0;
		for(std::vector< std::vector<surface>>::iterator i = surfaces.begin(),
		    i_end = surfaces.end(); i != i_end; ++i) {
			std::size_t xpos = 0;
			height = 0;

			for(std::vector<surface>::iterator j = i->begin(),
					j_end = i->end(); j != j_end; ++j) {
				SDL_Rect dstrect = sdl::create_rect(xpos, ypos, 0, 0);
				blit_surface(*j, nullptr, res, &dstrect);
				xpos += (*j)->w;
				height = std::max<std::size_t>((*j)->h, height);
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
	if (!gui_surface) {
		const text_surface& u = text_cache::find(text_surface(text, size, color, style));
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
	const auto font = sdl_ttf::get_font(font_id(0, size));
	if(font == nullptr)
		return 0;
	return TTF_FontHeight(font.get());
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

	try {
		utf8::iterator itor(text);
		for(; itor != utf8::iterator::end(text); ++itor) {
			std::string tmp = current_substring;
			tmp.append(itor.substr().first, itor.substr().second);

			if (line_width(tmp + ellipsis, font_size, style) > max_width) {
				return current_substring + ellipsis;
			}

			current_substring.append(itor.substr().first, itor.substr().second);
		}
	}
	catch(utf8::invalid_utf8_exception&) {
		WRN_FT << "Invalid UTF-8 string: \"" << text << "\"" << std::endl;
		return "";
	}

	return text; // Should not happen
}

/***
 * Initialize and destruction
 */

sdl_ttf::sdl_ttf()
{
	const int res = TTF_Init();
	if(res == -1) {
		ERR_FT << "Could not initialize SDL_TTF" << std::endl;
		throw font::error("SDL_TTF could not initialize, TTF_INIT returned: " + std::to_string(res));
	} else {
		LOG_FT << "Initialized true type fonts\n";
	}
}

static void clear_fonts()
{
	// Ensure that the shared_ptr<TTF_Font>s' destructors run before TTF_Quit().
	font_table.clear();
	family_table.clear();

	font_names.clear();
	bold_names.clear();
	italic_names.clear();

	line_size_cache.clear();
}

sdl_ttf::~sdl_ttf()
{
	clear_fonts();
	TTF_Quit();
}

// sets the font list to be used.
void sdl_ttf::set_font_list(const std::vector<subset_descriptor>& fontlist)
{
	// Wesnoth's startup sequence usually loads the same set of fonts twice.
	// See if we can use the already-loaded fonts.
	if(!font_names.empty()) {
		std::vector<family_record> reordered_family_table;
		bool found_all_fonts = true;
		for(const auto& f : fontlist) {
			// Ignore fonts if the font file doesn't exist - this matches the behavior of when we
			// can't reuse the already-loaded fonts.
			if(!check_font_file(f.name))
				continue;
			const auto& old_record = std::find_if(
				family_table.cbegin(), family_table.cend(), [&f](family_record x) { return f.name == x.name; });
			if(old_record == family_table.cend()) {
				found_all_fonts = false;
				break;
			}
			reordered_family_table.emplace_back(*old_record);
		}
		if(found_all_fonts) {
			std::swap(family_table, reordered_family_table);
			DBG_FT << "Reordered the font list, the order is now:\n";
			for(const auto& x : family_table) {
				DBG_FT << "[" << x.subset << "]:\t\tbase:\t'" << x.name << "'\n";
			}
			return;
		}
	}

	// The existing fonts weren't sufficient, or this is the first time that this function has been
	// called. Load all the fonts from scratch.
	clear_fonts();

	// To access TTF_GlyphIsProvided, we need to create instances of each font. Choose a size that
	// the GUI will want to use.
	const auto default_size = preferences::font_scaled(font::SIZE_NORMAL);

	for(const auto& f : fontlist) {
		if(!check_font_file(f.name))
			continue;
		// Insert fonts only if the font file exists
		const subset_id subset = font_names.size();
		font_names.push_back(f.name);

		if(f.bold_name && check_font_file(*f.bold_name)) {
			bold_names.push_back(*f.bold_name);
		} else {
			bold_names.emplace_back();
		}

		if(f.italic_name && check_font_file(*f.italic_name)) {
			italic_names.push_back(*f.italic_name);
		} else {
			italic_names.emplace_back();
		}

		auto font = sdl_ttf::get_font(font_id{subset, default_size});
		family_table.push_back(family_record{std::move(font), subset, f.name});
	}

	assert(font_names.size() == bold_names.size());
	assert(font_names.size() == italic_names.size());

	DBG_FT << "Set the font list. The styled font families are:\n";

	for(std::size_t i = 0; i < font_names.size(); ++i) {
		DBG_FT << "[" << i << "]:\t\tbase:\t'" << font_names[i] << "'\tbold:\t'" << bold_names[i] << "'\titalic:\t'"
			   << italic_names[i] << "'\n";
	}
}

/**
 * Splits the UTF-8 text into text_chunks using the same font.
 *
 * This uses a greedy-match - once we've found the start of a chunk,
 * include as many characters as we can in the same chunk.
 *
 * If we've got a fallback font that contains all characters, and a
 * preferred font that will only contains some of them, this means that
 * we minimize the number of times that we switch from one font to the
 * other - once we've had to use the fallback, keep using it.
 *
 * This also means that combining characters such as U+308 or U+FE00 are
 * kept with the character that they should be modifying.
 */
std::vector<text_chunk> sdl_ttf::split_text(const std::string& utf8_text)
{
	std::vector<text_chunk> chunks;

	if(utf8_text.empty())
		return chunks;

	try {
		const auto end = utf8::iterator::end(utf8_text);
		auto chunk_start = utf8::iterator(utf8_text);
		while(chunk_start != end) {
			auto& family = find_font_containing(*chunk_start);
			if(family.subset >= 0) {
				auto ch = chunk_start;
				auto last_in_chunk = chunk_start;
				while(ch != end && TTF_GlyphIsProvided(family.font.get(), *ch)) {
					last_in_chunk = ch;
					++ch;
				}
				chunks.emplace_back(
					family.subset, std::string{chunk_start.substr().first, last_in_chunk.substr().second});
				chunk_start = ch;
			} else {
				++chunk_start;
			}
		}
	} catch(utf8::invalid_utf8_exception&) {
		WRN_FT << "Invalid UTF-8 string: \"" << utf8_text << "\"" << std::endl;
	}
	return chunks;
}

} // end namespace font
