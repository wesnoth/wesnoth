/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Support for simple markup in text (fonts, colors, images).
 * E.g. "@Victory" will be shown in green.
 */

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "font/sdl_ttf.hpp"
#include "gettext.hpp"
#include "font/marked-up_text.hpp"
#include "font/standard_colors.hpp"
#include "sdl/surface.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#include "video.hpp"
#include "wml_exception.hpp"
#include "preferences.hpp"

namespace font {

// NOTE: if you add more markup characters below, you'll need to update
// the list in campaign_server.cpp (illegal_markup_chars) to blacklist
// them for add-on names and titles.

const char LARGE_TEXT='*', SMALL_TEXT='`',
		   BOLD_TEXT='~',  NORMAL_TEXT='{',
		   NULL_MARKUP='^',
		   BLACK_TEXT='}', GRAY_TEXT='|',
           GOOD_TEXT='@',  BAD_TEXT='#',
           GREEN_TEXT='@', RED_TEXT='#',
           COLOR_TEXT='<', IMAGE='&';

std::string::const_iterator parse_markup(std::string::const_iterator i1,
												std::string::const_iterator i2,
												int* font_size,
												color_t* color, int* style)
{
	while(i1 != i2) {
		switch(*i1) {
		case '\\':
			// This must either be a quoted special character or a
			// quoted backslash - either way, remove leading backslash
			break;
		case BAD_TEXT:
			if (color) *color = BAD_COLOR;
			break;
		case GOOD_TEXT:
			if (color) *color = GOOD_COLOR;
			break;
		case NORMAL_TEXT:
			if (color) *color = NORMAL_COLOR;
			break;
		case BLACK_TEXT:
			if (color) *color = BLACK_COLOR;
			break;
		case GRAY_TEXT:
			if (color) *color = GRAY_COLOR;
			break;
		case LARGE_TEXT:
			if (font_size) *font_size += 2;
			break;
		case SMALL_TEXT:
			if (font_size) *font_size -= 2;
			break;
		case BOLD_TEXT:
			if (style) *style |= TTF_STYLE_BOLD;
			break;
		case NULL_MARKUP:
			return i1+1;
		case COLOR_TEXT:
			{
				std::string::const_iterator start = i1;
				// Very primitive parsing for rgb value
				// should look like <213,14,151>
				++i1;
				Uint8 red=0, green=0, blue=0, temp=0;
				while (i1 != i2 && *i1 >= '0' && *i1<='9') {
					temp*=10;
					temp += lexical_cast<int, char>(*i1);
					++i1;
				}
				red=temp;
				temp=0;
				if (i1 != i2 && ',' == (*i1)) {
					++i1;
					while(i1 != i2 && *i1 >= '0' && *i1<='9'){
						temp*=10;
						temp += lexical_cast<int, char>(*i1);
						++i1;
					}
					green=temp;
					temp=0;
				}
				if (i1 != i2 && ',' == (*i1)) {
					++i1;
					while(i1 != i2 && *i1 >= '0' && *i1<='9'){
						temp*=10;
						temp += lexical_cast<int, char>(*i1);
						++i1;
					}
				}
				blue=temp;
				if (i1 != i2 && '>' == (*i1)) {
					color_t temp_color = {red, green, blue, 0};
					if (color) *color = temp_color;
				} else {
					// stop parsing and do not consume any chars
					return start;
				}
				if (i1 == i2) return i1;
				break;
			}
		default:
			return i1;
		}
		++i1;
	}
	return i1;
}

std::string del_tags(const std::string& text){
	std::vector<std::string> lines = utils::split(text, '\n', 0);
	std::vector<std::string>::iterator line;
	for(line = lines.begin(); line != lines.end(); ++line) {
		std::string::const_iterator i1 = line->begin(),
			i2 = line->end();
		*line = std::string(parse_markup(i1,i2,nullptr,nullptr,nullptr),i2);
	}
	return utils::join(lines, "\n");
}

SDL_Rect text_area(const std::string& text, int size, int style)
{
	const SDL_Rect area {0,0,10000,10000};
	return draw_text(nullptr, area, size, font::NORMAL_COLOR, text, 0, 0, false, style);
}

SDL_Rect draw_text(surface& dst, const SDL_Rect& area, int size,
                   const color_t& color, const std::string& txt,
                   int x, int y, bool use_tooltips, int style)
{
	// Make sure there's always at least a space,
	// so we can ensure that we can return a rectangle for height
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
		color_t col = color;
		int sz = size;
		int text_style = style;

		i1 = parse_markup(i1,i2,&sz,&col,&text_style);

		if(i1 != i2) {
			std::string new_string = utils::unescape(std::string(i1, i2));

			const SDL_Rect rect = draw_text_line(dst, area, sz, col, new_string, x, y, use_tooltips, text_style);
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

SDL_Rect draw_text(CVideo* gui, const SDL_Rect& area, int size,
                   const color_t& color, const std::string& txt,
                   int x, int y, bool use_tooltips, int style)
{
	surface null_surf = surface(nullptr);

	return draw_text(gui != nullptr ? gui->getSurface() : null_surf, area, size, color, txt, x, y, use_tooltips, style);
}

bool is_format_char(char c)
{
	switch(c) {
	case LARGE_TEXT:
	case SMALL_TEXT:
	case GOOD_TEXT:
	case BAD_TEXT:
	case NORMAL_TEXT:
	case BLACK_TEXT:
	case GRAY_TEXT:
	case BOLD_TEXT:
	case NULL_MARKUP:
		return true;
	default:
		return false;
	}
}

bool is_cjk_char(const ucs4::char_t ch)
{
	/**
	 * You can check these range at http://unicode.org/charts/
	 * see the "East Asian Scripts" part.
	 * Notice that not all characters in that part is still in use today, so don't list them all here.
	 * Below are characters that I guess may be used in wesnoth translations.
	 */

	//FIXME add range from Japanese-specific and Korean-specific section if you know the characters are used today.

	if (ch < 0x2e80) return false; // shortcut for common non-CJK

	return
		//Han Ideographs: all except Supplement
		(ch >= 0x4e00 && ch < 0x9fcf) ||
		(ch >= 0x3400 && ch < 0x4dbf) ||
		(ch >= 0x20000 && ch < 0x2a6df) ||
		(ch >= 0xf900 && ch < 0xfaff) ||
		(ch >= 0x3190 && ch < 0x319f) ||

		//Radicals: all except Ideographic Description
		(ch >= 0x2e80 && ch < 0x2eff) ||
		(ch >= 0x2f00 && ch < 0x2fdf) ||
		(ch >= 0x31c0 && ch < 0x31ef) ||

		//Chinese-specific: Bopomofo and Bopomofo Extended
		(ch >= 0x3104 && ch < 0x312e) ||
		(ch >= 0x31a0 && ch < 0x31bb) ||

		//Yi-specific: Yi Radicals, Yi Syllables
		(ch >= 0xa490 && ch < 0xa4c7) ||
		(ch >= 0xa000 && ch < 0xa48d) ||

		//Japanese-specific: Hiragana, Katakana, Kana Supplement
		(ch >= 0x3040 && ch <= 0x309f) ||
		(ch >= 0x30a0 && ch <= 0x30ff) ||
		(ch >= 0x1b000 && ch <= 0x1b001) ||

		//Ainu-specific: Katakana Phonetic Extensions
		(ch >= 0x31f0 && ch <= 0x31ff) ||

		//Korean-specific: Hangul Syllables, Hangul Jamo, Hangul Jamo Extended-A, Hangul Jamo Extended-B
		(ch >= 0xac00 && ch < 0xd7af) ||
		(ch >= 0x1100 && ch <= 0x11ff) ||
		(ch >= 0xa960 && ch <= 0xa97c) ||
		(ch >= 0xd7b0 && ch <= 0xd7fb) ||

		//CJK Symbols and Punctuation
		(ch >= 0x3000 && ch < 0x303f) ||

		//Halfwidth and Fullwidth Forms
		(ch >= 0xff00 && ch < 0xffef);
}
static void cut_word(std::string& line, std::string& word, int font_size, int style, int max_width)
{
	std::string tmp = line;
	utf8::iterator tc(word);
	bool first = true;
	font_size = preferences::font_scaled(font_size);

	for(;tc != utf8::iterator::end(word); ++tc) {
		tmp.append(tc.substr().first, tc.substr().second);
		SDL_Rect tsize = line_size(tmp, font_size, style);
		if(tsize.w > max_width) {
			const std::string& w = word;
			if(line.empty() && first) {
				line += std::string(w.begin(), tc.substr().second);
				word = std::string(tc.substr().second, w.end());
			} else {
				line += std::string(w.begin(), tc.substr().first);
				word = std::string(tc.substr().first, w.end());
			}
			break;
		}
		first = false;
	}
}

namespace {

/*
 * According to Kinsoku-Shori, Japanese rules about line-breaking:
 *
 * * the following characters cannot begin a line (so we will never break before them):
 * 、。，．）〕］｝〉》」』】’”ゝゞヽヾ々？！：；ぁぃぅぇぉゃゅょゎァィゥェォャュョヮっヵッヶ・…ー
 *
 * * the following characters cannot end a line (so we will never break after them):
 * （〔［｛〈《「『【‘“
 *
 * Unicode range that concerns word wrap for Chinese:
 *   全角ASCII、全角中英文标点 (Fullwidth Character for ASCII, English punctuations and part of Chinese punctuations)
 *   http://www.unicode.org/charts/PDF/UFF00.pdf
 *   CJK 标点符号 (CJK punctuations)
 *   http://www.unicode.org/charts/PDF/U3000.pdf
 */
inline bool no_break_after(const ucs4::char_t ch)
{
	return
		/**
		 * don't break after these Japanese characters
		 */
		ch == 0x2018 || ch == 0x201c || ch == 0x3008 || ch == 0x300a || ch == 0x300c ||
		ch == 0x300e || ch == 0x3010 || ch == 0x3014 || ch == 0xff08 || ch == 0xff3b ||
		ch == 0xff5b ||

		/**
		 * FIXME don't break after these Korean characters
		 */

		/**
		 * don't break after these Chinese characters
		 * contains left side of different kinds of brackets and quotes
		 */
		ch == 0x3016 || ch == 0x301a || ch == 0x301d;
}

inline bool no_break_before(const ucs4::char_t ch)
{
	return
		/**
		 * don't break before these Japanese characters
		 */
		ch == 0x2019 || ch == 0x201d || ch == 0x2026 || ch == 0x3001 || ch == 0x3002 ||
		ch == 0x3005 || ch == 0x3009 || ch == 0x300b || ch == 0x300d || ch == 0x300f ||
		ch == 0x3011 || ch == 0x3015 || ch == 0x3041 || ch == 0x3043 || ch == 0x3045 ||
		ch == 0x3047 || ch == 0x3049 || ch == 0x3063 || ch == 0x3083 || ch == 0x3085 ||
		ch == 0x3087 || ch == 0x308e || ch == 0x309d || ch == 0x309e || ch == 0x30a1 ||
		ch == 0x30a3 || ch == 0x30a5 || ch == 0x30a7 || ch == 0x30a9 || ch == 0x30c3 ||
		ch == 0x30e3 || ch == 0x30e5 || ch == 0x30e7 || ch == 0x30ee || ch == 0x30f5 ||
		ch == 0x30f6 || ch == 0x30fb || ch == 0x30fc || ch == 0x30fd || ch == 0x30fe ||
		ch == 0xff01 || ch == 0xff09 || ch == 0xff0c || ch == 0xff0e || ch == 0xff1a ||
		ch == 0xff1b || ch == 0xff1f || ch == 0xff3d || ch == 0xff5d ||

		// Small katakana used in Ainu:
		ch == 0x31f0 || ch == 0x31f1 || ch == 0x31f2 || ch == 0x31f3 || ch == 0x31f4 ||
		ch == 0x31f5 || ch == 0x31f6 || ch == 0x31f7 || ch == 0x31f8 || ch == 0x31f9 ||
		ch == 0x31fa || ch == 0x31fb || ch == 0x31fc || ch == 0x31fd || ch == 0x31fe ||
		ch == 0x31ff ||

		/**
		 * FIXME don't break before these Korean characters
		 */

		/**
		 * don't break before these Chinese characters
		 * contains
		 *   many Chinese punctuations that should not start a line
		 *   and right side of different kinds of brackets, quotes
		 */
		ch == 0x301c || ch == 0xff0d || ch == 0xff64 || ch == 0xff65 || ch == 0x3017 ||
		ch == 0x301b || ch == 0x301e;
}

inline bool break_before(const ucs4::char_t ch)
{
	if(no_break_before(ch))
		return false;

	return is_cjk_char(ch);
}

inline bool break_after(const ucs4::char_t ch)
{
	if(no_break_after(ch))
		return false;

	return is_cjk_char(ch);
}

} // end of anon namespace

std::string word_wrap_text(const std::string& unwrapped_text, int font_size,
	int max_width, int max_height, int max_lines, bool partial_line)
{
	VALIDATE(max_width > 0, _("The maximum text width is less than 1."));

	utf8::iterator ch(unwrapped_text);
	std::string current_word;
	std::string current_line;
	size_t line_width = 0;
	size_t current_height = 0;
	bool line_break = false;
	bool first = true;
	bool start_of_line = true;
	std::string wrapped_text;
	std::string format_string;
	color_t color;
	int font_sz = font_size;
	int style = TTF_STYLE_NORMAL;
	utf8::iterator end = utf8::iterator::end(unwrapped_text);

	while(1) {
		if(start_of_line) {
			line_width = 0;
			format_string.clear();
			while(ch != end && *ch < static_cast<ucs4::char_t>(0x100)
					&& is_format_char(*ch) && !ch.next_is_end()) {

				format_string.append(ch.substr().first, ch.substr().second);
				++ch;
			}
			// We need to parse the special format characters
			// to give the proper font_size and style to line_size()
			font_sz = font_size;
			style = TTF_STYLE_NORMAL;
			parse_markup(format_string.begin(),format_string.end(),&font_sz,&color,&style);
			current_line.clear();
			start_of_line = false;
		}

		// If there is no current word, get one
		if(current_word.empty() && ch == end) {
			break;
		} else if(current_word.empty()) {
			if(*ch == ' ' || *ch == '\n') {
				current_word = *ch;
				++ch;
			} else {
				ucs4::char_t previous = 0;
				for(;ch != utf8::iterator::end(unwrapped_text) &&
						*ch != ' ' && *ch != '\n'; ++ch) {

					if(!current_word.empty() &&
							break_before(*ch) &&
							!no_break_after(previous))
						break;

					if(!current_word.empty() &&
							break_after(previous) &&
							!no_break_before(*ch))
						break;

					current_word.append(ch.substr().first, ch.substr().second);

					previous = *ch;
				}
			}
		}

		if(current_word == "\n") {
			line_break = true;
			current_word.clear();
			start_of_line = true;
		} else {

			const size_t word_width = line_size(current_word, preferences::font_scaled(font_sz), style).w;

			line_width += word_width;

			if(static_cast<long>(line_width) > max_width) {
				if (!partial_line && static_cast<long>(word_width) > max_width) {
					cut_word(current_line,
						current_word, font_sz, style, max_width);
				}
				if(current_word == " ")
					current_word = "";
				line_break = true;
			} else {
				current_line += current_word;
				current_word = "";
			}
		}

		if(line_break || (current_word.empty() && ch == end)) {
			SDL_Rect size = line_size(current_line, preferences::font_scaled(font_sz), style);
			if(max_height > 0 && current_height + size.h >= size_t(max_height)) {
				return wrapped_text;
			}

			if(!first) {
				wrapped_text += '\n';
			}

			wrapped_text += format_string + current_line;
			current_line.clear();
			line_width = 0;
			current_height += size.h;
			line_break = false;
			first = false;

			if(--max_lines == 0) {
				return wrapped_text;
			}
		}
	}
	return wrapped_text;
}

} // end namespace font
