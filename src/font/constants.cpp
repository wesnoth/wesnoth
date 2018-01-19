/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "font/constants.hpp"

namespace font {

const int SIZE_NORMAL = 14;

const int
	SIZE_TINY = 10 * SIZE_NORMAL / 14,
	SIZE_SMALL = 12 * SIZE_NORMAL / 14,

	SIZE_15 = 15 * SIZE_NORMAL / 14,
	SIZE_PLUS = 16 * SIZE_NORMAL / 14,
	SIZE_LARGE = 18 * SIZE_NORMAL / 14,
	SIZE_TITLE = 20 * SIZE_NORMAL / 14,
	SIZE_XLARGE = 24 * SIZE_NORMAL / 14
;

const size_t max_text_line_width = 4096;

const std::string
	ellipsis = "...",

	unicode_minus = "-",
	unicode_en_dash = "–", // unicode u2013
	unicode_em_dash = "—", // unicode u2014
	unicode_figure_dash = "‒", // unicode u2012
	unicode_multiplication_sign = "×",
	unicode_bullet = "•", // unicode u2022

	weapon_numbers_sep = "×",
	weapon_details_sep = "–";

} // end namespace font
