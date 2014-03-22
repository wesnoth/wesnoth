/* vim:set encoding=utf-8: */
/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
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
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "serialization/string_utils.hpp"

#include <boost/foreach.hpp>

#include <list>
#include <set>
#include <stack>

#include <cairo-features.h>

#ifdef CAIRO_HAS_WIN32_FONT
#include <windows.h>
#undef CAIRO_HAS_FT_FONT
#endif

#ifdef CAIRO_HAS_FT_FONT
#include <fontconfig/fontconfig.h>
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

struct font_id
{
	font_id(subset_id subset, int size) : subset(subset), size(size) {}
	bool operator==(const font_id& o) const
	{
		return subset == o.subset && size == o.size;
	}
	bool operator<(const font_id& o) const
	{
		return subset < o.subset || (subset == o.subset && size < o.size);
	}

	subset_id subset;
	int size;
};

static std::map<font_id, TTF_Font*> font_table;
static std::vector<std::string> font_names;

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
	char_blocks.cbmap.clear();
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
		ERR_FT << "Could not load the true type fonts\n";
		throw error();
	}
#endif

#if CAIRO_HAS_WIN32_FONT
	BOOST_FOREACH(const std::string& path, get_binary_paths("fonts")) {
		std::vector<std::string> files;
		get_files_in_dir(path, &files, NULL, ENTIRE_FILE_PATH);
		BOOST_FOREACH(const std::string& file, files)
			if(file.substr(file.length() - 4) == ".ttf" || file.substr(file.length() - 4) == ".ttc")
				AddFontResourceA(file.c_str());
	}
#endif
}

void manager::deinit() const
{
#ifdef CAIRO_HAS_FT_FONT
	FcConfigAppFontClear(FcConfigGetCurrent());
#endif

#if CAIRO_HAS_WIN32_FONT
	BOOST_FOREACH(const std::string& path, get_binary_paths("fonts")) {
		std::vector<std::string> files;
		get_files_in_dir(path, &files, NULL, ENTIRE_FILE_PATH);
		BOOST_FOREACH(const std::string& file, files)
			if(file.substr(file.length() - 4) == ".ttf" || file.substr(file.length() - 4) == ".ttc")
				RemoveFontResourceA(file.c_str());
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

	std::string name;
	typedef std::pair<int, int> range;
	std::vector<range> present_codepoints;
};

//sets the font list to be used.
static void set_font_list(const std::vector<subset_descriptor>& fontlist)
{
	clear_fonts();

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

		BOOST_FOREACH(const subset_descriptor::range &cp_range, itor->present_codepoints) {
			char_blocks.insert(cp_range.first, cp_range.second, subset);
		}
	}
	char_blocks.compress();
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


#ifdef	OLD_FRIBIDI
	n = fribidi_utf8_to_unicode (c_str, len, bidi_logical);
#else
	n = fribidi_charset_to_unicode(FRIBIDI_CHAR_SET_UTF8, c_str, len, bidi_logical);
#endif
	fribidi_log2vis(bidi_logical, n, &base_dir, bidi_visual, NULL, NULL, NULL);
#ifdef	OLD_FRIBIDI
	fribidi_unicode_to_utf8 (bidi_visual, n, utf8str);
#else
	fribidi_unicode_to_charset(FRIBIDI_CHAR_SET_UTF8, bidi_visual, n, utf8str);
#endif
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
		TTF_Font* ttfont = get_font(font_id(chunk.subset, font_size_));
		if(ttfont == NULL)
			continue;
		font_style_setter const style_setter(ttfont, style_);

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
		TTF_Font* ttfont = get_font(font_id(chunk.subset, font_size_));
		if (ttfont == NULL)
			continue;
		font_style_setter const style_setter(ttfont, style_);

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
				SDL_Rect dstrect = create_rect(xpos, ypos, 0, 0);
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

SDL_Rect draw_text_line(surface gui_surface, const SDL_Rect& area, int size,
		   const SDL_Color& color, const std::string& text,
		   int x, int y, bool use_tooltips, int style)
{
	if (gui_surface.null()) {
		text_surface const &u = text_cache::find(text_surface(text, size, color, style));
		return create_rect(0, 0, u.width(), u.height());
	}

	if(area.w == 0) {  // no place to draw
		return create_rect(0, 0, 0, 0);
	}

	const std::string etext = make_text_ellipsis(text, size, area.w);

	// for the main current use, we already parsed markup
	surface surface(render_text(etext,size,color,style,false));
	if(surface == NULL) {
		return create_rect(0, 0, 0, 0);
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

namespace {

typedef std::map<int, font::floating_label> label_map;
label_map labels;
int label_id = 1;

std::stack<std::set<int> > label_contexts;
}


namespace font {

floating_label::floating_label(const std::string& text)
		: surf_(NULL), buf_(NULL), text_(text),
		font_size_(SIZE_NORMAL),
		color_(NORMAL_COLOR),	bgcolor_(), bgalpha_(0),
		xpos_(0), ypos_(0),
		xmove_(0), ymove_(0), lifetime_(-1),
		width_(-1), height_(-1),
		clip_rect_(screen_area()),
		alpha_change_(0), visible_(true), align_(CENTER_ALIGN),
		border_(0), scroll_(ANCHOR_LABEL_SCREEN), use_markup_(true)
{}

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
		font::ttext text;
		text.set_foreground_color((color_.r << 24) | (color_.g << 16) | (color_.b << 8) | 255);
		text.set_font_size(font_size_);
		text.set_maximum_width(width_ < 0 ? clip_rect_.w : width_);
		text.set_maximum_height(height_ < 0 ? clip_rect_.h : height_, true);

		//ignore last '\n'
		if(!text_.empty() && *(text_.rbegin()) == '\n'){
			text.set_text(std::string(text_.begin(), text_.end()-1), use_markup_);
		} else {
			text.set_text(text_, use_markup_);
		}

		surface foreground = text.render();

		if(foreground == NULL) {
			ERR_FT << "could not create floating label's text" << std::endl;
			return NULL;
		}

		// combine foreground text with its background
		if(bgalpha_ != 0) {
			// background is a dark tooltip box
			surface background = create_neutral_surface(foreground->w + border_*2, foreground->h + border_*2);

			if (background == NULL) {
				ERR_FT << "could not create tooltip box" << std::endl;
				surf_ = create_optimized_surface(foreground);
				return surf_;
			}

			Uint32 color = SDL_MapRGBA(foreground->format, bgcolor_.r,bgcolor_.g, bgcolor_.b, bgalpha_);
			sdl_fill_rect(background,NULL, color);

			// we make the text less transparent, because the blitting on the
			// dark background will darken the anti-aliased part.
			// This 1.13 value seems to restore the brightness of version 1.4
			// (where the text was blitted directly on screen)
			foreground = adjust_surface_alpha(foreground, ftofxp(1.13), false);

			SDL_Rect r = create_rect( border_, border_, 0, 0);
			SDL_SetAlpha(foreground,SDL_SRCALPHA,SDL_ALPHA_OPAQUE);
			blit_surface(foreground, NULL, background, &r);

			surf_ = create_optimized_surface(background);
			// RLE compression seems less efficient for big semi-transparent area
			// so, remove it for this case, but keep the optimized display format
			SDL_SetAlpha(surf_,SDL_SRCALPHA,SDL_ALPHA_OPAQUE);
		}
		else {
			// background is blurred shadow of the text
			surface background = create_neutral_surface
				(foreground->w + 4, foreground->h + 4);
			sdl_fill_rect(background, NULL, 0);
			SDL_Rect r = { 2, 2, 0, 0 };
			blit_surface(foreground, NULL, background, &r);
			background = shadow_image(background, false);

			if (background == NULL) {
				ERR_FT << "could not create floating label's shadow" << std::endl;
				surf_ = create_optimized_surface(foreground);
				return surf_;
			}
			SDL_SetAlpha(foreground,SDL_SRCALPHA,SDL_ALPHA_OPAQUE);
			blit_surface(foreground, NULL, background, &r);
			surf_ = create_optimized_surface(background);
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
		buf_.assign(create_compatible_surface(screen, surf_->w, surf_->h));
		if(buf_ == NULL) {
			return;
		}
	}

	if(screen == NULL) {
		return;
	}

	SDL_Rect rect = create_rect(xpos(surf_->w), ypos_, surf_->w, surf_->h);
	const clip_rect_setter clip_setter(screen, &clip_rect_);
	sdl_blit(screen,&rect,buf_,NULL);
	sdl_blit(surf_,NULL,screen,&rect);

	update_rect(rect);
}

void floating_label::undraw(surface screen)
{
	if(screen == NULL || buf_ == NULL) {
		return;
	}

	SDL_Rect rect = create_rect(xpos(surf_->w), ypos_, surf_->w, surf_->h);
	const clip_rect_setter clip_setter(screen, &clip_rect_);
	sdl_blit(buf_,NULL,screen,&rect);

	update_rect(rect);

	move(xmove_,ymove_);
	if(lifetime_ > 0) {
		--lifetime_;
		if(alpha_change_ != 0 && (xmove_ != 0.0 || ymove_ != 0.0) && surf_ != NULL) {
			// fade out moving floating labels
			// note that we don't optimize these surfaces since they will always change
			surf_.assign(adjust_surface_alpha_add(surf_,alpha_change_,false));
		}
	}
}

int add_floating_label(const floating_label& flabel)
{
	if(label_contexts.empty()) {
		return 0;
	}

	++label_id;
	labels.insert(std::pair<int, floating_label>(label_id, flabel));
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
		if(i->second.scroll() == ANCHOR_LABEL_MAP) {
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
			return create_rect(0, 0, surf->w, surf->h);
		}
	}

	return empty_rect;
}

floating_label_context::floating_label_context()
{
#if SDL_VERSION_ATLEAST(2, 0, 0)
	surface const screen = NULL;
#else
	surface const screen = SDL_GetVideoSurface();
#endif
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

#if SDL_VERSION_ATLEAST(2, 0, 0)
	surface const screen = NULL;
#else
	surface const screen = SDL_GetVideoSurface();
#endif
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
			++j;
		}
	}
}

}

static bool add_font_to_fontlist(config &fonts_config,
	std::vector<font::subset_descriptor>& fontlist, const std::string& name)
{
	config &font = fonts_config.find_child("font", "name", name);
	if (!font)
		return false;

		fontlist.push_back(font::subset_descriptor());
		fontlist.back().name = name;
		std::vector<std::string> ranges = utils::split(font["codepoints"]);

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

namespace {
	t_string family_order;
} // namespace

bool load_font_config()
{
	//read font config separately, so we do not have to re-read the whole
	//config when changing languages
	config cfg;
	try {
		scoped_istream stream = preprocess_file(get_wml_location("hardwired/fonts.cfg"));
		read(cfg, *stream);
	} catch(config::error &e) {
		ERR_FT << "could not read fonts.cfg:\n"
		       << e.message << '\n';
		return false;
	}

	config &fonts_config = cfg.child("fonts");
	if (!fonts_config)
		return false;

	std::set<std::string> known_fonts;
	BOOST_FOREACH(const config &font, fonts_config.child_range("font")) {
		known_fonts.insert(font["name"]);
	}

	family_order = fonts_config["family_order"];
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

const t_string& get_font_families()
{
	return family_order;
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
