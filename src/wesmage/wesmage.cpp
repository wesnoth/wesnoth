/*
   Copyright (C) 2012 - 2016 by Mark de Wever <koraq@xs4all.nl>
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
 * Tool to test the image conversion functions.
 */

#include "tools/exploder_utils.hpp"
#include "wesmage/exit.hpp"
#include "wesmage/filter.hpp"
#include "wesmage/options.hpp"

#include <boost/foreach.hpp>

#include <SDL_image.h>

#include <ctime>
#include <iostream>

static clock_t
get_begin_time()
{
	clock_t begin = std::clock();
	clock_t end = std::clock();

	while(begin == (end = std::clock())) {
		/* DO NOTHING */
	}
	std::cout << "Clock resolution "
			<< end - begin
			<< " ticks, using " << CLOCKS_PER_SEC << " ticks/second.\n"
			<< "This give resolution of about "
			<<static_cast<double>(end - begin) / CLOCKS_PER_SEC
			<< " seconds.\n";

	/* IO might be slow so wait until the next value. */
	begin = std::clock();
	while(begin == (end = std::clock())) {
		/* DO NOTHING */
	}

	return begin;
}

int
main(int argc, char* argv[])
{
	try {
		const toptions& options = toptions::parse(argc, argv);

		surface surf(make_neutral_surface(
				IMG_Load(options.input_filename.c_str())));

		if(!surf) {
			std::cerr << "Error: Failed to load input file »"
					<< options.input_filename
					<< "«.\n";

			return EXIT_FAILURE;
		}

		std::vector<surface> surfaces;
		if(options.count != 1) {
			for(int i = 1; i < options.count; ++i) {
				// make_neutral_surface make a deep-copy of the image.
				surfaces.push_back(make_neutral_surface(surf));
			}
		}
		surfaces.push_back(surf);

		const clock_t begin = options.time ? get_begin_time() : 0;

		for(int i = 0; i < options.count; ++i) {
			BOOST_FOREACH(const std::string& filter, options.filters) {
				filter_apply(surfaces[i], filter);
			}
		}

		if(options.time) {
			const clock_t end = std::clock();
			std::cout << "Applying the filters took "
					<<  end - begin
					<< " ticks, "
					<< static_cast<double>(end - begin) / CLOCKS_PER_SEC
					<< " seconds.\n";
		}

		if(!options.output_filename.empty()) {
			save_image(surfaces[0], options.output_filename);
		}

	} catch(const texit& exit) {
		return exit.status;
	} catch(exploder_failure& err) {
		std::cerr << "Error: Failed with error »" << err.message << "«.\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
