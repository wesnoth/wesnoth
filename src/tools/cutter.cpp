/*
   Copyright (C) 2004 - 2016 by Philippe Plantier <ayin@anathas.org>
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
 * Standalone-Utility for images / tiles
 */

#include "game_config.hpp"
#include "exploder_composer.hpp"

#include <SDL_image.h>

#include <iostream>

namespace {

	void print_usage(std::string name)
	{
		std::cerr << "usage: " << name << " [source] [dest_directory]\n";
	}
}

int main(int argc, char* argv[])
{
	std::string src;
	std::string dest_dir;
	cutter cut;

	// Parse arguments that shouldn't require a display device
	int arg;
	for(arg = 1; arg != argc; ++arg) {
		const std::string val(argv[arg]);
		if(val.empty()) {
			continue;
		}

		if(val == "--help" || val == "-h") {
			print_usage(argv[0]);
			return 0;
		} else if(val == "--verbose" || val == "-v") {
			cut.set_verbose(true);
		} else if(val == "--directory" || val == "-d" ) {
			game_config::path = argv[++arg];
		} else {
			if(src.empty()) {
				src = val;
			} else if(dest_dir.empty()) {
				dest_dir = val;
			} else {
				print_usage(argv[0]);
				return 1;
			}
		}
	}

	if(src.empty() || dest_dir.empty()) {
		print_usage(argv[0]);
		return 1;
	}

	try {
		const config conf = cut.load_config(src);
		cut.load_masks(conf);

		const surface src_surface(make_neutral_surface(IMG_Load(src.c_str())));
		if(src_surface == NULL)
			throw exploder_failure("Unable to load the source image " + src);

		const cutter::surface_map surfaces = cut.cut_surface(src_surface, conf);

		for(cutter::surface_map::const_iterator itor = surfaces.begin();
				itor != surfaces.end(); ++itor) {
			const cutter::mask &mask = itor->second.mask;

			surface surf = create_compatible_surface(
					  itor->second.image
					, mask.cut.w
					, mask.cut.h);

			masked_overwrite_surface(surf, itor->second.image, mask.image,
					mask.cut.x - mask.shift.x, mask.cut.y - mask.shift.y);

			save_image(surf, dest_dir + "/" + mask.name + ".png");
		}

	} catch(exploder_failure& err) {
		std::cerr << "Failed: " << err.message << "\n";
		return 1;
	}

	return 0;
}


