/*
   Copyright (C) 2004 - 2014 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "exploder_composer.hpp"
#include "SDL_image.h"

#include <iostream>

composer::composer() : interactive_(false), verbose_(false)
{
}


surface composer::compose(const std::string &src, const std::string &dest)
{
	cutter cut;
	cut.set_verbose(verbose_);

	const config src_conf = cut.load_config(src);
	const config dest_conf = cut.load_config(dest);

	if(verbose_) {
		std::cerr << "Loading masks...\n";
	}
	cut.load_masks(src_conf);
	cut.load_masks(dest_conf);

	if(verbose_) {
		std::cerr << "Loading images...\n";
	}
	const surface src_surface(make_neutral_surface(IMG_Load(src.c_str())));
	if(src_surface == NULL)
		throw exploder_failure("Unable to load the source image " + src);

	const surface dest_surface(make_neutral_surface(IMG_Load(dest.c_str())));
	if(dest_surface == NULL)
		throw exploder_failure("Unable to load the destination image " + dest);

	if(verbose_) {
		std::cerr << "Cutting images...\n";
	}
	const cutter::surface_map src_surfaces = cut.cut_surface(src_surface, src_conf);
	const cutter::surface_map dest_surfaces = cut.cut_surface(dest_surface, dest_conf);

	for(cutter::surface_map::const_iterator itor = dest_surfaces.begin();
			itor != dest_surfaces.end(); ++itor) {

		const std::string& name = itor->second.name;

		if(src_surfaces.find(name) == src_surfaces.end())
			continue;

		const cutter::positioned_surface& src_ps = src_surfaces.find(name)->second;
		const cutter::positioned_surface& dest_ps = itor->second;

		if(!image_empty(dest_ps.image)) {
			if(interactive_) {
				//TODO: make "interactive" mode work
			} else {
				std::cerr << "Warning: element " << name << " not empty on destination image\n";
			}
		}
		if(verbose_) {
			std::cerr << "Inserting image " << name
				<< " on position (" << dest_ps.pos.x
				<< ", " << dest_ps.pos.y << ")\n";
		}
		masked_overwrite_surface(dest_surface, src_ps.image,
				src_ps.mask.image,
				dest_ps.pos.x, dest_ps.pos.y);
	}

	return dest_surface;
}

void composer::set_interactive(bool value)
{
	interactive_ = value;
}

void composer::set_verbose(bool value)
{
	verbose_ = value;
}

