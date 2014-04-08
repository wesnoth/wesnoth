/*
   Copyright (C) 2012 - 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

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
 * Tool to create the test images for the unit tests.
 */

#include "filesystem.hpp"
#include "tools/exploder_utils.hpp"
#include "tests/test_sdl_utils.hpp"

#include <SDL_image.h>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <iostream>

static void
show_usage()
{
	std::cerr <<
"Usage:\n"
"test_images -h\n"
"test_images DIR\n"
"Helper program to create images for the unit tests.\n"
"\n"
"The DIR is the output directory, will be created if doesn't exist.\n"
"OPTIONS:\n"
"-h, --help              Show this help and terminate the program.\n"
;
}

static surface
create_image_base(const std::string& filename)
{
	surface result = SDL_CreateRGBSurface(
			  SDL_SWSURFACE
			, 1024
			, 1024
			, 32
			, 0xFF0000
			, 0xFF00
			, 0xFF
			, 0);

	surface_lock lock(result);

	Uint32 *pixels = lock.pixels();

	for(Uint32 r = 0; r < 256; r += 4) {
		for(Uint32 g = 0; g < 256; g += 2) {
			for(Uint32 b = 0; b < 256; b += 2, ++pixels) {

				*pixels = (0xFF << 24) | (r << 16) | (g << 8) | b;

			}
		}
	}

	save_image(result, filename);

	return result;
}

static void
create_image_blend_functor(
		  const surface& dst
		, const std::string root
		, const Uint8 amount
		, const Uint32 color)
{
	const std::string filename = blend_get_filename(root, amount, color);

	save_image(dst, filename);
}

static void
create_image_blend(const surface& src, const std::string& root)
{
	blend_image(
			  src
			, boost::bind(&create_image_blend_functor, _1, root, _2, _3));
}

typedef void (*tfunctor) (const surface&, const std::string&);
typedef std::pair<std::string, tfunctor> tcreator;

static const tcreator creators[] =
{
	std::make_pair("/blend/", &create_image_blend)
};

int
main(int argc, char* argv[])
{
	if(argc != 2) {
		show_usage();
		return EXIT_FAILURE;
	}

	const std::string root = argv[1];

	if(root == "-h" || root == "--help") {
		show_usage();
		return EXIT_SUCCESS;
	}

	if(!filesystem::is_directory(root)) {
		if(filesystem::file_exists(root)) {
			std::cerr << "";
			return EXIT_FAILURE;
		}
		if(!filesystem::make_directory(root)) {
			std::cerr << "";
			return EXIT_FAILURE;
		}
	}

	BOOST_FOREACH(const tcreator& creator, creators) {
		if(!filesystem::make_directory(root + creator.first)) {
			std::cerr << "";
			return EXIT_FAILURE;
		}
	}

	try {
		const surface base_image = create_image_base(root + "/base.png");

		BOOST_FOREACH(const tcreator& creator, creators) {
			creator.second(base_image, root + creator.first);
		}

	} catch(exploder_failure& err) {
		std::cerr << "Error: Failed with error »" << err.message << "«.\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

