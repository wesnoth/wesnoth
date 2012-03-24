/* $Id$ */
/*
   Copyright (C) 2012 by Mark de Wever <koraq@xs4all.nl>
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

#include "foreach.hpp"
#include "tools/exploder_utils.hpp"
#include "wesmage/exit.hpp"
#include "wesmage/filter.hpp"
#include "wesmage/options.hpp"

#include <SDL_image.h>

#include <iostream>

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

		BOOST_FOREACH(const std::string& filter, options.filters) {
			filter_apply(surf, filter);
		}

		if(!options.output_filename.empty()) {
			save_image(surf, options.output_filename);
		}

	} catch(const texit& exit) {
		return exit.status;
	} catch(exploder_failure& err) {
		std::cerr << "Error: Failed with error »" << err.message << "«.\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
