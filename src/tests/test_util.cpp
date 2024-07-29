/*
	Copyright (C) 2007 - 2024
	by Karol Nowak <grywacz@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-test"

#include "utils/math.hpp"

#include <cstdint>

static_assert(bit_width<uint8_t>() == 8);
static_assert(bit_width<uint16_t>() == 16);
static_assert(bit_width<uint32_t>() == 32);
static_assert(bit_width<uint64_t>() == 64);

static_assert(bit_width(static_cast<uint8_t>(0)) == 8);
static_assert(bit_width(static_cast<uint16_t>(0)) == 16);
static_assert(bit_width(static_cast<uint32_t>(0)) == 32);
static_assert(bit_width(static_cast<uint64_t>(0)) == 64);

static_assert(count_leading_zeros(static_cast<uint8_t>(1)) == 7);
static_assert(count_leading_zeros(static_cast<uint16_t>(1)) == 15);
static_assert(count_leading_zeros(static_cast<uint32_t>(1)) == 31);
static_assert(count_leading_zeros(static_cast<uint64_t>(1)) == 63);
static_assert(count_leading_zeros(static_cast<uint8_t>(0xFF)) == 0);
static_assert(count_leading_zeros(static_cast<unsigned int>(0)) == bit_width<unsigned int>());
static_assert(count_leading_zeros(static_cast<unsigned long int>(0)) == bit_width<unsigned long int>());
static_assert(count_leading_zeros(static_cast<unsigned long long int>(0)) == bit_width<unsigned long long int>());
static_assert(count_leading_zeros(static_cast<uint16_t>(12345)) == 2); // 12345 == 0x3039
static_assert(count_leading_zeros(static_cast<int16_t>(12345)) == 2); // 12345 == 0x3039
static_assert(count_leading_zeros(uint8_t{0xff}) == 0);
static_assert(count_leading_zeros('\0') == bit_width<char>());
static_assert(count_leading_zeros('\b') == bit_width<char>() - 4);
static_assert(count_leading_zeros('\033') == bit_width<char>() - 5);
static_assert(count_leading_zeros(' ') == bit_width<char>() - 6);

static_assert(count_leading_ones(0) == 0);
static_assert(count_leading_ones(1) == 0);
static_assert(count_leading_ones(0u) == 0);
static_assert(count_leading_ones(1u) == 0);
static_assert(count_leading_ones(static_cast<uint8_t>(0xFF)) == 8);
static_assert(count_leading_ones(static_cast<uint16_t>(0xFFFF)) == 16);
static_assert(count_leading_ones(0xFFFFFFFFU) == 32);
static_assert(count_leading_ones(0xFFFFFFFFFFFFFFFFULL) == 64);
static_assert(count_leading_ones(static_cast<uint8_t>(0xF8)) == 5);
static_assert(count_leading_ones(static_cast<uint16_t>(54321)) == 2);
