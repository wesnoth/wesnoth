/*
   Copyright (C) 2012 - 2018 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//#define GETTEXT_DOMAIN "wesnoth-test"
//
//#include "tests/test_sdl_utils.hpp"
//
//#include "image.hpp"
//
//#include "utils/functional.hpp"
//#include <boost/test/auto_unit_test.hpp>
//
//#include <iomanip>

//static const std::string root = "data/test/test/image";

//static void
//compare_image(
//		  const surface& src
//		, const surface& dst
//		, const std::string message)
//{
//	BOOST_REQUIRE_MESSAGE(
//			  src->w == dst->w
//			, message
//				<< "source width »"
//				<< src->w
//				<< "« destination width »"
//				<< dst->w
//				<< "«.\n"
//			);
//
//	BOOST_REQUIRE_MESSAGE(
//			  src->h == dst->h
//			, message
//				<< "source height »"
//				<< src->w
//				<< "« destination height »"
//				<< dst->w
//				<< "«.\n"
//			);
//
//	const_surface_lock src_lock(src);
//	const_surface_lock dst_lock(dst);
//
//	const uint32_t* src_pixels = src_lock.pixels();
//	const uint32_t* dst_pixels = dst_lock.pixels();
//
//	const unsigned pixels = src->w * src->h;
//
//	unsigned matches = 0;
//
//	for(unsigned i = 0; i < pixels; ++i, ++src_pixels, ++dst_pixels) {
//		matches += (*src_pixels == *dst_pixels);
//	}
//
//	BOOST_CHECK_MESSAGE(
//			  pixels == matches
//			, message
//				<< "of the " << pixels
//				<< " pixels in the image " << matches
//				<< " match.\n"
//			);
//}

//static void
//test_blend(const surface& dst, const uint8_t amount, const uint32_t color)
//{
//	std::stringstream sstr;
//	sstr << std::hex << std::setfill('0')
//			<< "Blend image amount »"
//			<< std::setw(2) << static_cast<uint32_t>(amount)
//			<< "« color »"
//			<< std::setw(8) << color
//			<< "« : ";
//
//	const std::string filename =
//			blend_get_filename("data/test/test/image/blend/", amount, color);
//
//	BOOST_REQUIRE_EQUAL(image::exists(filename), true);
//
//	const surface& src = image::get_image(filename);
//	compare_image(src, dst, sstr.str());
//}

//BOOST_AUTO_TEST_CASE(test_blend_surface)
//{
//
//	BOOST_REQUIRE_EQUAL(image::exists(root + "/base.png"), true);
//
//	const surface base = image::get_image(root + "/base.png");
//
//	blend_image(base, std::bind(&test_blend, _1, _2, _3));
//}
