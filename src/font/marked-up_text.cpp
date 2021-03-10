/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "gettext.hpp"
#include "font/marked-up_text.hpp"
#include "font/standard_colors.hpp"
#include "lexical_cast.hpp"
#include "sdl/surface.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#include "video.hpp"
#include "wml_exception.hpp"
#include "preferences/general.hpp"

namespace font {

bool is_cjk_char(const char32_t ch)
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

} // end namespace font
