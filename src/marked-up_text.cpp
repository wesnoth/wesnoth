/* $Id$ */
/*
   Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "font.hpp"
#include "global.hpp"
#include "marked-up_text.hpp"
#include "team.hpp"
#include "video.hpp"
#include "wassert.hpp"

namespace font {

const char LARGE_TEXT='*', SMALL_TEXT='`', GOOD_TEXT='@', BAD_TEXT='#',
           NORMAL_TEXT='{', BLACK_TEXT='}', BOLD_TEXT='~', IMAGE='&', NULL_MARKUP='^';

namespace {

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
		// semi ANSI colour escape sequences at the start of the line for now only
		case '\033':
			if(i2 - i1 >= 4) {
				++i1;
				if(*i1 == '[') {
					++i1;
					if(*i1 == '3') {
						++i1;
						if(*i1 >= '0' && *i1 <= '9' && *(i1 + 1) == 'm')
						{
							if(*i1 != '0')
								*colour = team::get_side_colour(lexical_cast<int, char>(*i1));
							++i1;
						}
					}
				}
			}
			break;
		default:
			return i1;
		}

		++i1;
	}

	return i1;
}

}


SDL_Rect text_area(const std::string& text, int size, int style)
{
	const SDL_Rect area = {0,0,10000,10000};
	return draw_text(NULL, area, size, font::NORMAL_COLOUR, text, 0, 0, false, style);
}

SDL_Rect draw_text(CVideo* gui, const SDL_Rect& area, int size,
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

bool is_format_char(char c)
{
	//side coloring
	if(c > 0 && c <= 10) {
		return true;
	}

	switch(c) {
	case LARGE_TEXT:
	case SMALL_TEXT:
	case GOOD_TEXT:
	case BAD_TEXT:
	case NORMAL_TEXT:
	case BLACK_TEXT:
	case BOLD_TEXT:
	case NULL_MARKUP:
		return true;
	default:
		return false;
	}
}

namespace {

void cut_word(std::string& line, std::string& word, int size, int max_width)
{
	std::string tmp = line;
	utils::utf8_iterator tc(word);
	bool first = true;

	for(;tc != utils::utf8_iterator::end(word); ++tc) {
		tmp.append(tc.substr().first, tc.substr().second);
		SDL_Rect tsize = line_size(tmp, size);
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


/*
 * According to Kinsoku-Shori, Japanese rules about line-breaking:
 *
 * * the following characters cannot begin a line (so we will never break before them):
 * 、。，．）〕］｝〉》」』】’”ゝゞヽヾ々？！：；ぁぃぅぇぉゃゅょゎァィゥェォャュョヮっヵッヶ・…ー
 *
 * * the following characters cannot end a line (so we will never break after them):
 * （〔［｛〈《「『【‘“
 */
inline bool no_break_after(wchar_t ch)
{
	return
		ch == 0x2018 || ch == 0x201c || ch == 0x3008 || ch == 0x300a || ch == 0x300c ||
		ch == 0x300e || ch == 0x3010 || ch == 0x3014 || ch == 0xff08 || ch == 0xff3b ||
		ch == 0xff5b;

}

inline bool no_break_before(wchar_t ch)
{
	return
		ch == 0x2019 || ch == 0x201d || ch == 0x2026 || ch == 0x3001 || ch == 0x3002 ||
		ch == 0x3005 || ch == 0x3009 || ch == 0x300b || ch == 0x300d || ch == 0x300f ||
		ch == 0x3011 || ch == 0x3015 || ch == 0x3041 || ch == 0x3043 || ch == 0x3045 ||
		ch == 0x3047 || ch == 0x3049 || ch == 0x3063 || ch == 0x3083 || ch == 0x3085 ||
		ch == 0x3087 || ch == 0x308e || ch == 0x309d || ch == 0x309e || ch == 0x30a1 ||
		ch == 0x30a3 || ch == 0x30a5 || ch == 0x30a7 || ch == 0x30a9 || ch == 0x30c3 ||
		ch == 0x30e3 || ch == 0x30e5 || ch == 0x30e7 || ch == 0x30ee || ch == 0x30f5 ||
		ch == 0x30f6 || ch == 0x30fb || ch == 0x30fc || ch == 0x30fd || ch == 0x30fe ||
		ch == 0xff01 || ch == 0xff09 || ch == 0xff0c || ch == 0xff0e || ch == 0xff1a ||
		ch == 0xff1b || ch == 0xff1f || ch == 0xff3d || ch == 0xff5d;
}

inline bool break_before(wchar_t ch)
{
	if(no_break_before(ch))
		return false;

	return ch == ' ' ||
		// CKJ characters
		(ch >= 0x3000 && ch < 0xa000) ||
		(ch >= 0xf900 && ch < 0xfb00) ||
		(ch >= 0xff00 && ch < 0xfff0);
}

inline bool break_after(wchar_t ch)
{
	if(no_break_after(ch))
		return false;

	return ch == ' ' ||
		// CKJ characters
		(ch >= 0x3000 && ch < 0xa000) ||
		(ch >= 0xf900 && ch < 0xfb00) ||
		(ch >= 0xff00 && ch < 0xfff0);
}
}

std::string word_wrap_text(const std::string& unwrapped_text, int font_size, int max_width, int max_height, int max_lines)
{
	wassert(max_width > 0);

	utils::utf8_iterator ch(unwrapped_text);
	std::string current_word;
	std::string current_line;
	size_t line_width = 0;
	size_t current_height = 0;
	bool line_break = false;
	bool first = true;
	bool start_of_line = true;
	std::string wrapped_text;
	std::string format_string;
	utils::utf8_iterator end = utils::utf8_iterator::end(unwrapped_text);

	while(1) {
		if(start_of_line) {
			line_width = 0;
			format_string = "";
			while(ch != end && *ch < 0x100U && is_format_char(*ch)) {
				format_string.append(ch.substr().first, ch.substr().second);
				++ch;
			}
			current_line = format_string;
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
				wchar_t previous = 0;
				for(;ch != utils::utf8_iterator::end(unwrapped_text) &&
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
			current_word = "";
			start_of_line = true;
		} else {

			const std::string word = format_string + current_word;

			const size_t word_width = line_size(word,font_size).w;

			line_width += word_width;

			if(line_width > max_width) {
				if(word_width > max_width) {
					cut_word(current_line, current_word, font_size, max_width);
				}
				if(current_word == " ")
					current_word = "";
				line_break = true;
			} else {
				current_line += current_word;
				current_word = "";
			}
		}

		if(line_break || current_word.empty() && ch == end) {
			SDL_Rect size = line_size(current_line, font_size);
			if(max_height > 0 && current_height + size.h >= size_t(max_height)) {
				return wrapped_text;
			}

			if(!first) {
				wrapped_text += '\n';
			}

			wrapped_text += current_line;
			current_line = format_string;
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

SDL_Rect draw_wrapped_text(CVideo* gui, const SDL_Rect& area, int font_size,
		     const SDL_Color& colour, const std::string& text,
		     int x, int y, int max_width)
{
	std::string wrapped_text = word_wrap_text(text, font_size, max_width);
	return font::draw_text(gui, area, font_size, colour, wrapped_text, x, y, false);
}


size_t text_to_lines(std::string& message, size_t max_length)
{
	std::string starting_markup;
	bool at_start = true;

	size_t cur_line = 0, longest_line = 0;
	for(std::string::iterator i = message.begin(); i != message.end(); ++i) {
		if(at_start) {
			if(font::is_format_char(*i)) {
				push_back(starting_markup,*i);
			} else {
				at_start = false;
			}
		}

		if(*i == '\n') {
			at_start = true;
			starting_markup = "";
		}

		if(*i == ' ' && cur_line > max_length) {
			*i = '\n';
			const size_t index = i - message.begin();
			message.insert(index+1,starting_markup);
			i = message.begin() + index + starting_markup.size();

			if(cur_line > longest_line)
				longest_line = cur_line;

			cur_line = 0;
		}

		if(*i == '\n' || i+1 == message.end()) {
			if(cur_line > longest_line)
				longest_line = cur_line;

			cur_line = 0;

		} else {
			++cur_line;
		}
	}

	return longest_line;
}



}
