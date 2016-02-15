/* vim:set encoding=utf-8: */
/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "global.hpp"

#include "config.hpp"
#include "filesystem.hpp"
#include "font.hpp"
#include "game_config.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "text.hpp"
#include "tooltips.hpp"
#include "video.hpp"
#include "sdl/alpha.hpp"
#include "sdl/rect.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#include "preferences.hpp"

#include <boost/foreach.hpp>
#include <boost/optional.hpp>

#include <list>
#include <set>
#include <stack>
#include <sstream>

#include <cairo-features.h>

#ifdef CAIRO_HAS_WIN32_FONT
#include <windows.h>
#undef CAIRO_HAS_FT_FONT
#endif

#ifdef CAIRO_HAS_FT_FONT
#include <fontconfig/fontconfig.h>
#endif

#if !defined(CAIRO_HAS_FT_FONT) && !defined(CAIRO_HAS_WIN32_FONT)
// Is there soemthing like #warning which just gives awarnign insteads of an error?
#error unable to find font loading tools.
#endif

static lg::log_domain log_font("font");
#define DBG_FT LOG_STREAM(debug, log_font)
#define LOG_FT LOG_STREAM(info, log_font)
#define WRN_FT LOG_STREAM(warn, log_font)
#define ERR_FT LOG_STREAM(err, log_font)

#ifdef	HAVE_FRIBIDI
#include <fribidi.h>
#endif

// Signed int. Negative values mean "no subset".
typedef int subset_id;

// Used as a key in the font table, which caches the get_font results.
struct font_id
{
	font_id(subset_id subset, int size) : subset(subset), size(size), style(TTF_STYLE_NORMAL) {}
	font_id(subset_id subset, int size, int style) : subset(subset), size(size), style(style) {}
	bool operator==(const font_id& o) const
	{
		return subset == o.subset && size == o.size && style == o.style;
	}
	bool operator<(const font_id& o) const
	{
		return subset < o.subset || (subset == o.subset && size < o.size) || (subset == o.subset && size == o.size && style < o.style);
	}

	subset_id subset;
	int size;
	int style;
};

// Record stored in the font table.
// If the record for font_id (FOO, Bold + Underline) is a record (BAR, Bold),
// it means that BAR is a Bold-styled version of FOO which we shipped with the
// game, and now SDL_TTF should be used to style BAR as underline for the final results.
struct ttf_record
{
	TTF_Font* font;
	int style;
};

typedef std::map<font_id, ttf_record> tfont_table;

static tfont_table font_table;
static std::vector<std::string> font_names;
static std::vector<std::string> bold_names;
static std::vector<std::string> italic_names;

struct text_chunk
{
	text_chunk(subset_id subset) :
		subset(subset),
		text()
	{
	}

	bool operator==(text_chunk const & t) const { return subset == t.subset && text == t.text; }
	bool operator!=(text_chunk const & t) const { return !operator==(t); }

	subset_id subset;
	std::string text;
};

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
		cbmap.insert(std::make_pair(first, block_t(last, id)));
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

//Splits the UTF-8 text into text_chunks using the same font.
static std::vector<text_chunk> split_text(std::string const & utf8_text) {
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

typedef std::map<std::pair<std::string, int>, TTF_Font*> topen_font_cache;
topen_font_cache open_fonts;

static TTF_Font* open_font_impl(const std::string & , int);

// A wrapper which caches the results of open_font_impl.
// Note that clear_fonts() is responsible to clean up all of these font pointers,
// so to avoid memory leaks fonts should only be opened from this function.
static TTF_Font* open_font(const std::string& fname, int size)
{
	const std::pair<std::string, int> key = std::make_pair(fname, size);
	const topen_font_cache::iterator it = open_fonts.find(key);
	if (it != open_fonts.end()) {
		return it->second;
	}

	TTF_Font* result = open_font_impl(fname, size);
	open_fonts.insert(std::make_pair(key, result));
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
					return NULL;
				}
			}
		}

	} else {
		name = "fonts/" + fname;
		if(!filesystem::file_exists(name)) {
			if(!filesystem::file_exists(fname)) {
				ERR_FT << "Failed opening font: '" << name << "': No such file or directory" << std::endl;
				return NULL;
			}
			name = fname;
		}
	}

	SDL_RWops *rwops = filesystem::load_RWops(name);
	TTF_Font* font = TTF_OpenFontRW(rwops, true, size); // SDL takes ownership of rwops
	if(font == NULL) {
		ERR_FT << "Failed opening font: '" <<  fname << "'\n";
		ERR_FT << "TTF_OpenFont: " << TTF_GetError() << std::endl;
		return NULL;
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
static TTF_Font* get_font(font_id id)
{
	const std::map<font_id, ttf_record>::iterator it = font_table.find(id);
	if(it != font_table.end()) {
		if (it->second.font != NULL) {
			// If we found a valid record, use SDL_TTF to add in the difference
			// between its intrinsic style and the desired style.
			TTF_SetFontStyle(it->second.font, it->second.style ^ id.style);
		}
		return it->second.font;
	}

	// There's no record, so we need to try to find a solution for this font
	// and make a record of it. If the indices are out of bounds don't bother though.
	if(id.subset < 0 || size_t(id.subset) >= font_names.size()) {
		return NULL;
	}

	// Favor to use the shipped Italic font over bold if both are present and are needed.
	if ((id.style & TTF_STYLE_ITALIC) && italic_names[id.subset].size()) {
		if (TTF_Font* font = open_font(italic_names[id.subset], id.size)) {
			ttf_record rec = {font, TTF_STYLE_ITALIC};
			font_table.insert(std::make_pair(id, rec));
			return get_font(id);
		}
	}

	// Now see if the shipped Bold font is useful and available.
	if ((id.style & TTF_STYLE_BOLD) && bold_names[id.subset].size()) {
		if (TTF_Font* font = open_font(bold_names[id.subset], id.size)) {
			ttf_record rec = {font, TTF_STYLE_BOLD};
			font_table.insert(std::make_pair(id, rec));
			return get_font(id);
		}
	}

	// Try just to use the basic version of the font then.
	if (font_names[id.subset].size()) {
		if(TTF_Font* font = open_font(font_names[id.subset], id.size)) {
			ttf_record rec = {font, TTF_STYLE_NORMAL};
			font_table.insert(std::make_pair(id, rec));
			return get_font(id);
		}
	}

	// Failed to find a font.
	ttf_record rec = {NULL, TTF_STYLE_NORMAL};
	font_table.insert(std::make_pair(id, rec));
	return NULL;
}

static void clear_fonts()
{
	for(topen_font_cache::iterator i = open_fonts.begin(); i != open_fonts.end(); ++i) {
		TTF_CloseFont(i->second);
	}
	open_fonts.clear();

	font_table.clear();

	font_names.clear();
	bold_names.clear();
	italic_names.clear();

	char_blocks.cbmap.clear();
	line_size_cache.clear();
}

namespace font {

manager::manager()
{
	const int res = TTF_Init();
	if(res == -1) {
		ERR_FT << "Could not initialize true type fonts" << std::endl;
		throw error();
	} else {
		LOG_FT << "Initialized true type fonts\n";
	}

	init();
}

manager::~manager()
{
	deinit();

	clear_fonts();
	TTF_Quit();
}

void manager::update_font_path() const
{
	deinit();
	init();
}

void manager::init() const
{
#ifdef CAIRO_HAS_FT_FONT
	if (!FcConfigAppFontAddDir(FcConfigGetCurrent(),
		reinterpret_cast<const FcChar8 *>((game_config::path + "/fonts").c_str())))
	{
		ERR_FT << "Could not load the true type fonts" << std::endl;
		throw error();
	}

	if(!FcConfigParseAndLoad(FcConfigGetCurrent(),
							 reinterpret_cast<const FcChar8*>((game_config::path + "/fonts/fonts.conf").c_str()),
							 FcFalse))
	{
		ERR_FT << "Could not load local font configuration\n";
	}
	else
	{
		LOG_FT << "Local font configuration loaded\n";
	}
#endif

#if CAIRO_HAS_WIN32_FONT
	BOOST_FOREACH(const std::string& path, filesystem::get_binary_paths("fonts")) {
		std::vector<std::string> files;
		if(filesystem::is_directory(path)) {
			filesystem::get_files_in_dir(path, &files, NULL, filesystem::ENTIRE_FILE_PATH);
		}
		BOOST_FOREACH(const std::string& file, files) {
			if(file.substr(file.length() - 4) == ".ttf" || file.substr(file.length() - 4) == ".ttc")
			{
				const std::wstring wfile = unicode_cast<std::wstring>(file);
				AddFontResourceExW(wfile.c_str(), FR_PRIVATE, NULL);
			}
		}
	}
#endif
}

void manager::deinit() const
{
#ifdef CAIRO_HAS_FT_FONT
	FcConfigAppFontClear(FcConfigGetCurrent());
#endif

#if CAIRO_HAS_WIN32_FONT
	BOOST_FOREACH(const std::string& path, filesystem::get_binary_paths("fonts")) {
		std::vector<std::string> files;
		if(filesystem::is_directory(path))
			filesystem::get_files_in_dir(path, &files, NULL, filesystem::ENTIRE_FILE_PATH);
		BOOST_FOREACH(const std::string& file, files) {
			if(file.substr(file.length() - 4) == ".ttf" || file.substr(file.length() - 4) == ".ttc")
			{
				const std::wstring wfile = unicode_cast<std::wstring>(file);
				RemoveFontResourceExW(wfile.c_str(), FR_PRIVATE, NULL);
			}
		}
	}
#endif
}

//structure used to describe a font, and the subset of the Unicode character
//set it covers.
struct subset_descriptor
{
	subset_descriptor() :
		name(),
		present_codepoints()
	{
	}

	subset_descriptor(const config &);

	std::string name;
	boost::optional<std::string> bold_name; //If we are using another font for styled characters in this font, rather than SDL TTF method
	boost::optional<std::string> italic_name;

	typedef std::pair<int, int> range;
	std::vector<range> present_codepoints;
};

font::subset_descriptor::subset_descriptor(const config & font)
	: name(font["name"].str())
	, bold_name()
	, italic_name()
	, present_codepoints()
{
	if (font.has_attribute("bold_name")) {
		bold_name = font["bold_name"].str();
	}

	if (font.has_attribute("italic_name")) {
		italic_name = font["italic_name"].str();
	}

	std::vector<std::string> ranges = utils::split(font["codepoints"]);

	BOOST_FOREACH(const std::string & i, ranges) {
		std::vector<std::string> r = utils::split(i, '-');
		if(r.size() == 1) {
			size_t r1 = lexical_cast_default<size_t>(r[0], 0);
			present_codepoints.push_back(std::pair<size_t, size_t>(r1, r1));
		} else if(r.size() == 2) {
			size_t r1 = lexical_cast_default<size_t>(r[0], 0);
			size_t r2 = lexical_cast_default<size_t>(r[1], 0);

			present_codepoints.push_back(std::pair<size_t, size_t>(r1, r2));
		}
	}
}

static bool check_font_file(std::string name) {
	if(game_config::path.empty() == false) {
		if(!filesystem::file_exists(game_config::path + "/fonts/" + name)) {
			if(!filesystem::file_exists("fonts/" + name)) {
				if(!filesystem::file_exists(name)) {
				WRN_FT << "Failed opening font file '" << name << "': No such file or directory" << std::endl;
				return false;
				}
			}
		}
	} else {
		if(!filesystem::file_exists("fonts/" + name)) {
			if(!filesystem::file_exists(name)) {
				WRN_FT << "Failed opening font file '" << name << "': No such file or directory" << std::endl;
				return false;
			}
		}
	}
	return true;
}

//sets the font list to be used.
static void set_font_list(const std::vector<subset_descriptor>& fontlist)
{
	clear_fonts();

	std::vector<subset_descriptor>::const_iterator itor;
	for(itor = fontlist.begin(); itor != fontlist.end(); ++itor) {
		if (!check_font_file(itor->name)) continue;
		// Insert fonts only if the font file exists
		const subset_id subset = font_names.size();
		font_names.push_back(itor->name);

		if (itor->bold_name && check_font_file(*itor->bold_name)) {
			bold_names.push_back(*itor->bold_name);
		} else {
			bold_names.push_back("");
		}

		if (itor->italic_name && check_font_file(*itor->italic_name)) {
			italic_names.push_back(*itor->italic_name);
		} else {
			italic_names.push_back("");
		}

		BOOST_FOREACH(const subset_descriptor::range &cp_range, itor->present_codepoints) {
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

const SDL_Color NORMAL_COLOR = {0xDD,0xDD,0xDD,0},
                GRAY_COLOR   = {0x77,0x77,0x77,0},
                LOBBY_COLOR  = {0xBB,0xBB,0xBB,0},
                GOOD_COLOR   = {0x00,0xFF,0x00,0},
                BAD_COLOR    = {0xFF,0x00,0x00,0},
                BLACK_COLOR  = {0x00,0x00,0x00,0},
                YELLOW_COLOR = {0xFF,0xFF,0x00,0},
                BUTTON_COLOR = {0xBC,0xB0,0x88,0},
                PETRIFIED_COLOR = {0xA0,0xA0,0xA0,0},
                TITLE_COLOR  = {0xBC,0xB0,0x88,0},
				LABEL_COLOR  = {0x6B,0x8C,0xFF,0},
				BIGMAP_COLOR = {0xFF,0xFF,0xFF,0};
const SDL_Color DISABLED_COLOR = inverse(PETRIFIED_COLOR);

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
	FriBidiStrIndex	len = str_.length();
	FriBidiChar	*bidi_logical = new FriBidiChar[len + 2];
	FriBidiChar	*bidi_visual = new FriBidiChar[len + 2];
	char		*utf8str = new char[4*len + 1];	//assume worst case here (all 4 Byte characters)
	FriBidiCharType	base_dir = FRIBIDI_TYPE_ON;
	FriBidiStrIndex n;


	n = fribidi_charset_to_unicode(FRIBIDI_CHAR_SET_UTF8, c_str, len, bidi_logical);
	fribidi_log2vis(bidi_logical, n, &base_dir, bidi_visual, NULL, NULL, NULL);

	fribidi_unicode_to_charset(FRIBIDI_CHAR_SET_UTF8, bidi_visual, n, utf8str);
	is_rtl_ = base_dir == FRIBIDI_TYPE_RTL;
	str_ = std::string(utf8str);
	delete[] bidi_logical;
	delete[] bidi_visual;
	delete[] utf8str;
}
#endif

text_surface::text_surface(std::string const &str, int size,
		SDL_Color color, int style) :
	hash_(0),
	font_size_(size),
	color_(color),
	style_(style),
	w_(-1),
	h_(-1),
	str_(str),
	initialized_(false),
	chunks_(),
	surfs_()
#ifdef	HAVE_FRIBIDI
	,is_rtl_(false)
#endif
{
#ifdef	HAVE_FRIBIDI
	bidi_cvt();
#endif
	hash();
}

text_surface::text_surface(int size, SDL_Color color, int style) :
	hash_(0),
	font_size_(size),
	color_(color),
	style_(style),
	w_(-1),
	h_(-1),
	str_(),
	initialized_(false),
	chunks_(),
	surfs_()
#ifdef	HAVE_FRIBIDI
	,is_rtl_(false)
#endif
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

	BOOST_FOREACH(text_chunk const &chunk, chunks_)
	{
		TTF_Font* ttfont = get_font(font_id(chunk.subset, font_size_, style_));
		if(ttfont == NULL) {
			continue;
		}

		int w, h;
		TTF_SizeUTF8(ttfont, chunk.text.c_str(), &w, &h);
		w_ += w;
		h_ = std::max<int>(h_, h);
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

	BOOST_FOREACH(text_chunk const &chunk, chunks_)
	{
		TTF_Font* ttfont = get_font(font_id(chunk.subset, font_size_, style_));

		surface s = surface(TTF_RenderUTF8_Blended(ttfont, chunk.text.c_str(), color_));
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

static surface render_text(const std::string& text, int fontsize, const SDL_Color& color, int style, bool use_markup)
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
			parse_markup(ln->begin(), ln->end(), &sz, NULL, &text_style) : ln->begin();
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
				SDL_Rect dstrect = sdl::create_rect(xpos, ypos, 0, 0);
				sdl_blit(*j, NULL, res, &dstrect);
				xpos += (*j)->w;
				height = std::max<size_t>((*j)->h, height);
			}
			ypos += height;
		}

		return res;
	}
}


surface get_rendered_text(const std::string& str, int size, const SDL_Color& color, int style)
{
	// TODO maybe later also to parse markup here, but a lot to check
	return render_text(str, size, color, style, false);
}

SDL_Rect draw_text_line(surface& gui_surface, const SDL_Rect& area, int size,
		   const SDL_Color& color, const std::string& text,
		   int x, int y, bool use_tooltips, int style)
{
	size = preferences::font_scaled(size);
	if (gui_surface.null()) {
		text_surface const &u = text_cache::find(text_surface(text, size, color, style));
		return sdl::create_rect(0, 0, u.width(), u.height());
	}

	if(area.w == 0) {  // no place to draw
		return sdl::create_rect(0, 0, 0, 0);
	}

	const std::string etext = make_text_ellipsis(text, size, area.w);

	// for the main current use, we already parsed markup
	surface surface(render_text(etext,size,color,style,false));
	if(surface == NULL) {
		return sdl::create_rect(0, 0, 0, 0);
	}

	SDL_Rect dest;
	if(x!=-1) {
		dest.x = x;
#ifdef	HAVE_FRIBIDI
		// Oron -- Conditional, until all draw_text_line calls have fixed area parameter
		if(getenv("NO_RTL") == NULL) {
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

	if(gui_surface != NULL) {
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

std::string make_text_ellipsis(const std::string &text, int font_size,
	int max_width, int style)
{
	static const std::string ellipsis = "...";

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

}


static bool add_font_to_fontlist(const config &fonts_config,
	std::vector<font::subset_descriptor>& fontlist, const std::string& name)
{
	const config &font = fonts_config.find_child("font", "name", name);
	if (!font) {
		return false;
	}
	//DBG_FT << "Adding a font record: " << font.debug() << std::endl;

	fontlist.push_back(font::subset_descriptor(font));

	return true;
}

namespace font {

namespace {
	t_string family_order_sans;
	t_string family_order_mono;
} // namespace

bool load_font_config()
{
	//read font config separately, so we do not have to re-read the whole
	//config when changing languages
	config cfg;
	try {
		const std::string& cfg_path = filesystem::get_wml_location("hardwired/fonts.cfg");
		if(cfg_path.empty()) {
			ERR_FT << "could not resolve path to fonts.cfg, file not found\n";
			return false;
		}

		filesystem::scoped_istream stream = preprocess_file(cfg_path);
		read(cfg, *stream);
	} catch(config::error &e) {
		ERR_FT << "could not read fonts.cfg:\n"
		       << e.message << '\n';
		return false;
	}

	const config &fonts_config = cfg.child("fonts");
	if (!fonts_config)
		return false;

	std::set<std::string> known_fonts;
	BOOST_FOREACH(const config &font, fonts_config.child_range("font")) {
		known_fonts.insert(font["name"]);
		if (font.has_attribute("bold_name")) {
			known_fonts.insert(font["bold_name"]);
		}
		if (font.has_attribute("italic_name")) {
			known_fonts.insert(font["italic_name"]);
		}
	}

	family_order_sans = fonts_config["family_order"];
	family_order_mono = fonts_config["family_order_monospace"];

	if(family_order_mono.empty()) {
		ERR_FT << "No monospace font family order defined, falling back to sans serif order\n";
		family_order_mono = family_order_sans;
	}

	const std::vector<std::string> font_order = utils::split(fonts_config["order"]);
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

const t_string& get_font_families(family_class fclass)
{
	switch(fclass) {
	case FONT_MONOSPACE:
		return family_order_mono;
	default:
		return family_order_sans;
	}
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
