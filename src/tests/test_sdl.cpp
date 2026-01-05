/*
	Copyright (C) 2023 - 2025
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

#include "sdl/rect.hpp"
#include "sdl/surface.hpp"
#include "sdl/utils.hpp"

#include <algorithm>
#include <array>
#include <boost/test/unit_test.hpp>

namespace
{
constexpr uint32_t alpha    = 0x00'FF'FF'FF;
constexpr uint32_t red 		= 0xFF'FF'00'00;
constexpr uint32_t green 	= 0xFF'00'FF'00;
constexpr uint32_t blue 	= 0xFF'00'00'FF;
constexpr uint32_t yellow 	= 0xFF'FF'FF'00;
constexpr uint32_t white 	= 0xFF'FF'FF'FF;
constexpr uint32_t black 	= 0xFF'00'00'00;

constexpr std::array img_4x4 {
    red,    white,  green,  black,
    black,  black,  black,  black,
    blue,   white,  yellow, black,
    black,  black,  black,  black,
};

constexpr std::array img_4x4_to_2x2_result {
    red,    green,
    blue,   yellow,
};

constexpr std::array img_4x4_to_3x2_result {
    red,    white, 	green,
    blue,   white, 	yellow
};

constexpr std::array img_4x4_with_alpha {
    alpha, alpha, alpha, alpha,
    alpha, black, alpha, alpha,
    alpha, alpha, black, black,
    alpha, black, alpha, alpha,
};

constexpr std::array img_4x4_no_alpha {
    black, black, black, black,
    black, black, black, black,
    black, black, black, black,
    black, black, black, black,
};

template<std::size_t w, std::size_t h>
surface array_to_surface(const std::array<uint32_t, w * h>& arr)
{
	surface surf{w, h};

	{
		surface_lock surf_lock{surf};
		auto pixels = surf_lock.pixel_span();
		for(std::size_t i = 0; i < w * h; ++i) {
			pixels[i] = arr[i];
		}
	}

	return surf;
}

} // namespace

BOOST_AUTO_TEST_SUITE(sdl)

BOOST_AUTO_TEST_CASE(test_scale_sharp_nullptr)
{
	surface result = scale_surface_sharp(nullptr, 2, 2);
	BOOST_CHECK_EQUAL(result, nullptr);
}

BOOST_AUTO_TEST_CASE(test_scale_sharp_zero)
{
	surface src = array_to_surface<4, 4>(img_4x4);
	surface result = scale_surface_sharp(src, 0, 0);
	BOOST_CHECK_EQUAL(result->w, 0);
	BOOST_CHECK_EQUAL(result->h, 0);
}

BOOST_AUTO_TEST_CASE(test_scale_sharp_round)
{
	surface src = array_to_surface<4, 4>(img_4x4);
	surface result = scale_surface_sharp(src, 2, 2);
	const_surface_lock lock{result};
	auto result_pixels = lock.pixel_span();
	BOOST_CHECK_EQUAL_COLLECTIONS(
		result_pixels.begin(), result_pixels.end(), img_4x4_to_2x2_result.begin(), img_4x4_to_2x2_result.end());
}

BOOST_AUTO_TEST_CASE(test_scale_sharp_fractional)
{
	surface src = array_to_surface<4, 4>(img_4x4);
	surface result = scale_surface_sharp(src, 3, 2);
	const_surface_lock lock{result};
	auto result_pixels = lock.pixel_span();
	BOOST_CHECK_EQUAL_COLLECTIONS(
		result_pixels.begin(), result_pixels.end(), img_4x4_to_3x2_result.begin(), img_4x4_to_3x2_result.end());
}

BOOST_AUTO_TEST_CASE(test_transparent_clip)
{
	surface src = array_to_surface<4, 4>(img_4x4_with_alpha);
	rect result = get_non_transparent_portion(src);
	auto opaque = rect{1, 1, 3, 3};
	BOOST_CHECK_EQUAL(result, opaque);
}

BOOST_AUTO_TEST_CASE(test_transparent_clip_no_alpha)
{
	surface src = array_to_surface<4, 4>(img_4x4_no_alpha);
	rect result = get_non_transparent_portion(src);
	auto opaque = rect{0, 0, src->w, src->h};
	BOOST_CHECK_EQUAL(result, opaque);
}

BOOST_AUTO_TEST_SUITE_END()
