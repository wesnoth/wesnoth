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

#include "exploder_cutter.hpp"
#include "filesystem.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "serialization/string_utils.hpp"
#include "SDL_image.h"

#include <boost/foreach.hpp>

#include <iostream>

cutter::cutter()
	: masks_()
	, verbose_(false)
{
}

const config cutter::load_config(const std::string &filename)
{
	const std::string conf_string = find_configuration(filename);

	config res;

	try {
		filesystem::scoped_istream stream = preprocess_file(conf_string);
		read(res, *stream);
	} catch(config::error& err) {
		throw exploder_failure("Unable to load the configuration for the file " + filename + ": "+ err.message);
	}

	return res;
}


void cutter::load_masks(const config& conf)
{
	BOOST_FOREACH(const config &m, conf.child_range("mask"))
	{
		const std::string name = m["name"];
		const std::string image = get_mask_dir() + "/" + std::string(m["image"]);

		if(verbose_) {
			std::cerr << "Adding mask " << name << "\n";
		}

		if(image.empty())
			throw exploder_failure("Missing image for mask " + name);

		const exploder_point shift(m["shift"]);
		const exploder_rect cut(m["cut"]);

		if(masks_.find(name) != masks_.end() && masks_[name].filename != image) {
			throw exploder_failure("Mask " + name +
					" correspond to two different files: " +
					name + " and " +
					masks_.find(name)->second.filename);
		}

		if(masks_.find(name) == masks_.end()) {
			mask& cur_mask = masks_[name];

			cur_mask.name = name;
			cur_mask.shift = shift;
			cur_mask.cut = cut;
			cur_mask.filename = image;
			surface tmp(IMG_Load(image.c_str()));
			if(tmp == NULL)
				throw exploder_failure("Unable to load mask image " + image);

			cur_mask.image = make_neutral_surface(tmp);
		}

		if(masks_[name].image == NULL)
			throw exploder_failure("Unable to load mask image " + image);
	}
}


cutter::surface_map cutter::cut_surface(surface surf, const config& conf)
{
	surface_map res;

	BOOST_FOREACH(const config &part, conf.child_range("part")) {
		add_sub_image(surf, res, &part);
	}

	return res;
}


std::string cutter::find_configuration(const std::string &file)
{
	//finds the file prefix.
	const std::string fname = filesystem::file_name(file);
	const std::string::size_type dotpos = fname.rfind('.');

	std::string basename;
	if(dotpos == std::string::npos) {
		basename = fname;
	} else {
		basename = fname.substr(0, dotpos);
	}

	return get_exploder_dir() + "/" + basename + ".cfg";
}


void cutter::add_sub_image(const surface &surf, surface_map &map, const config* config)
{
	const std::string name = (*config)["name"];
	if(name.empty())
		throw exploder_failure("Un-named sub-image");

	if(masks_.find(name) == masks_.end())
		throw exploder_failure("Unable to find mask corresponding to " + name);

	const cutter::mask& mask = masks_[name];

	std::vector<std::string> pos = utils::split((*config)["pos"]);
	if(pos.size() != 2)
		throw exploder_failure("Invalid position " + (*config)["pos"].str());

	int x = atoi(pos[0].c_str());
	int y = atoi(pos[1].c_str());

	const SDL_Rect cut = create_rect(x - mask.shift.x
			, y - mask.shift.y
			, mask.image->w
			, mask.image->h);

	typedef std::pair<std::string, positioned_surface> sme;

	positioned_surface ps;
	ps.image = ::cut_surface(surf, cut);
	if(ps.image == NULL)
		throw exploder_failure("Unable to cut surface!");
	ps.name = name;
	ps.mask = mask;
	ps.pos.x = x - mask.shift.x;
	ps.pos.y = y - mask.shift.y;
	map.insert(sme(name, ps));

	if(verbose_) {
		std::cerr << "Extracting sub-image " << name << ", position (" << x << ", " << y << ")\n";
	}
}

void cutter::set_verbose(bool value)
{
	verbose_ = value;
}

