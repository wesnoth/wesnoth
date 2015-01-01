/*
   Copyright (C) 2004 - 2015 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "../game_config.hpp"
#include "exploder_composer.hpp"

#include <iostream>

namespace {

	void print_usage(std::string name)
	{
		std::cerr << "usage: " << name << " [source] [destination]\n";
	}
}

int main(int argc, char* argv[])
{
	std::string src;
	std::string dest;
	composer comp;

	//parse arguments that shouldn't require a display device
	int arg;
	for(arg = 1; arg != argc; ++arg) {
		const std::string val(argv[arg]);
		if(val.empty()) {
			continue;
		}

		if(val == "--help" || val == "-h") {
			print_usage(argv[0]);
			return 0;
		} else if(val == "--interactive" || val == "-i") {
			comp.set_interactive(true);
		} else if(val == "--verbose" || val == "-v") {
			comp.set_verbose(true);
		} else if(val == "--directory" || val == "-d" ) {
			game_config::path = argv[++arg];
		} else {
			if(src.empty()) {
				src = val;
			} else if(dest.empty()) {
				dest = val;
			} else {
				print_usage(argv[0]);
				return 1;
			}
		}
	}

	if(src.empty() || dest.empty()) {
		print_usage(argv[0]);
		return 1;
	}

	try {
		surface image = comp.compose(src, dest);
		save_image(image, dest);
	} catch(exploder_failure& err) {
		std::cerr << "Failed: " << err.message << "\n";
		return 1;
	}

	return 0;
}



