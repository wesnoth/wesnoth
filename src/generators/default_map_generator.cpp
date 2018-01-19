/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "generators/default_map_generator.hpp"

#include "gui/dialogs/editor/generator_settings.hpp"
#include "generators/default_map_generator_job.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "seed_rng.hpp"

static lg::log_domain log_engine("engine");
#define DBG_NG LOG_STREAM(debug, log_engine)

namespace {
	const int max_island = 10;
	const int max_coastal = 5;
}

generator_data::generator_data(const config &cfg)
	: width(std::max(0, cfg["map_width"].to_int(40)))
	, height(std::max(0, cfg["map_height"].to_int(40)))
	, default_width(width)
	, default_height(height)
	, nplayers(std::max(0, cfg["players"].to_int(2)))
	, nvillages(std::max(0, cfg["villages"].to_int(25)))
	, iterations(std::max(0, cfg["iterations"].to_int(1000)))
	, hill_size(std::max(0, cfg["hill_size"].to_int(10)))
	, castle_size(std::max(0, cfg["castle_size"].to_int(9)))
	, island_size(std::max(0, cfg["island_size"].to_int(0)))
	, island_off_center(0)
	, max_lakes(std::max(0, cfg["max_lakes"].to_int(20)))
	, link_castles(true)
	, show_labels(true)
{
}

default_map_generator::default_map_generator(const config& cfg)
	: cfg_(cfg)
	, data_(cfg)
{
}

bool default_map_generator::allow_user_config() const { return true; }

void default_map_generator::user_config()
{
	gui2::dialogs::generator_settings::execute(data_);
}

std::string default_map_generator::name() const { return "default"; }

std::string default_map_generator::config_name() const
{
	if (const config &c = cfg_.child("scenario"))
		return c["name"];

	return std::string();
}

std::string default_map_generator::create_map(boost::optional<uint32_t> randomseed)
{
	return generate_map(nullptr, randomseed);
}

std::string default_map_generator::generate_map(std::map<map_location,std::string>* labels, boost::optional<uint32_t> randomseed)
{
	uint32_t seed;
	if(const uint32_t* pseed = randomseed.get_ptr()) {
		seed = *pseed;
	} else {
		seed = seed_rng::next_seed();
	}

	/* We construct a copy of the generator data and modify it as needed. This ensures every time
	 * this function is called the generator job gets a fresh set of settings, and that the internal
	 * copy of the settings are never touched except by the settings dialog.
	 *
	 * The original data is still used for conditional checks and calculations, but any modifications
	 * should be done on this object.
	 */
	generator_data job_data = data_;

	// Suppress labels?
	if(!data_.show_labels) {
		labels = nullptr;
	}

	// The random generator thinks odd widths are nasty, so make them even
	if(is_odd(data_.width)) {
		++job_data.width;
	}

	job_data.iterations = (data_.iterations * data_.width * data_.height)/(data_.default_width * data_.default_height);
	job_data.island_size = 0;
	job_data.nvillages = (data_.nvillages * data_.width * data_.height) / 1000;
	job_data.island_off_center = 0;

	if(data_.island_size >= max_coastal) {
		// Islands look good with much fewer iterations than normal, and fewer lakes
		job_data.iterations /= 10;
		job_data.max_lakes /= 9;

		// The radius of the island should be up to half the width of the map
		const int island_radius = 50 + ((max_island - data_.island_size) * 50)/(max_island - max_coastal);
		job_data.island_size = (island_radius * (data_.width/2))/100;
	} else if(data_.island_size > 0) {
		// The radius of the island should be up to twice the width of the map
		const int island_radius = 40 + ((max_coastal - data_.island_size) * 40)/max_coastal;
		job_data.island_size = (island_radius * data_.width * 2)/100;
		job_data.island_off_center = std::min(data_.width, data_.height);
		DBG_NG << "calculated coastal params...\n";
	}

	// A map generator can fail so try a few times to get a map before aborting.
	std::string map;

	// Keep a copy of labels as it can be written to by the map generator func
	std::map<map_location,std::string> labels_copy;
	std::map<map_location,std::string>* labels_ptr = labels ? &labels_copy : nullptr;

	// Iinitilize the job outside the loop so that we really get a different result everytime we run the loop.
	default_map_generator_job job(seed);

	int tries = 10;
	std::string error_message;
	do {
		// Reset the labels.
		if(labels) {
			labels_copy = *labels;
		}

		try {
			map = job.default_generate_map(job_data, labels_ptr, cfg_);
			error_message = "";
		} catch(mapgen_exception& exc) {
			error_message = exc.message;
		}

		--tries;
	} while(tries && map.empty());

	if(labels) {
		labels->swap(labels_copy);
	}

	if(!error_message.empty()) {
		throw mapgen_exception(error_message);
	}

	return map;
}

config default_map_generator::create_scenario(boost::optional<uint32_t> randomseed)
{
	DBG_NG << "creating scenario...\n";

	config res = cfg_.child_or_empty("scenario");

	DBG_NG << "got scenario data...\n";

	std::map<map_location,std::string> labels;
	DBG_NG << "generating map...\n";

	try{
		res["map_data"] = generate_map(&labels, randomseed);
	}
	catch (mapgen_exception& exc){
		res["map_data"] = "";
		res["error_message"] = exc.message;
	}
	DBG_NG << "done generating map..\n";

	for(std::map<map_location,std::string>::const_iterator i =
			labels.begin(); i != labels.end(); ++i) {

		if(i->first.x >= 0 && i->first.y >= 0 &&
				i->first.x < static_cast<long>(data_.width) &&
				i->first.y < static_cast<long>(data_.height)) {

			config& label = res.add_child("label");
			label["text"] = i->second;
			label["category"] = "villages";
			i->first.write(label);
		}
	}

	return res;
}
