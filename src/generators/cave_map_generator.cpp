/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
 * Map-generator for caves.
 */

#include "global.hpp"

#include "cave_map_generator.hpp"
#include "log.hpp"
#include "map.hpp"
#include "pathfind/pathfind.hpp"
#include "serialization/string_utils.hpp"
#include "util.hpp"
#include "seed_rng.hpp"
#include <boost/foreach.hpp>

static lg::log_domain log_engine("engine");
#define LOG_NG LOG_STREAM(info, log_engine)

cave_map_generator::cave_map_generator(const config &cfg) :
	wall_(t_translation::CAVE_WALL),
	clear_(t_translation::CAVE),
	village_(t_translation::UNDERGROUND_VILLAGE),
	castle_(t_translation::DWARVEN_CASTLE),
	keep_(t_translation::DWARVEN_KEEP),
	cfg_(cfg ? cfg : config()),
	width_(50),
	height_(50),
	village_density_(0),
	flipx_chance_(cfg_["flipx_chance"]),
	flipy_chance_(cfg_["flipy_chance"])
{
	width_ = cfg_["map_width"];
	height_ = cfg_["map_height"];

	village_density_ = cfg_["village_density"];
}

std::string cave_map_generator::config_name() const
{
	return "";
}

size_t cave_map_generator::cave_map_generator_job::translate_x(size_t x) const
{
	if(flipx_) {
		x = params.width_ - x - 1;
	}

	return x;
}

size_t cave_map_generator::cave_map_generator_job::translate_y(size_t y) const
{
	if(flipy_) {
		y = params.height_ - y - 1;
	}

	return y;
}

std::string cave_map_generator::create_map(boost::optional<boost::uint32_t> randomseed)
{
	const config res = create_scenario(randomseed);
	return res["map_data"];
}

config cave_map_generator::create_scenario(boost::optional<boost::uint32_t> randomseed)
{
	cave_map_generator_job job(*this, randomseed);
	return job.res_;
}

cave_map_generator::cave_map_generator_job::cave_map_generator_job(const cave_map_generator& pparams, boost::optional<boost::uint32_t> randomseed)
	: params(pparams)
	, flipx_(false)
	, flipy_(false)
	, map_(t_translation::t_map(params.width_ + 2 * gamemap::default_border,
	       t_translation::t_list(params.height_ + 2 * gamemap::default_border, params.wall_)))
	, starting_positions_()
	, chamber_ids_()
	, chambers_()
	, passages_()
	, res_(params.cfg_.child_or_empty("settings"))
	, rng_() //initialises with rand()
{
	uint32_t seed = randomseed.get_ptr() ? *randomseed.get_ptr() : seed_rng::next_seed();
	rng_.seed(seed);
	std::cerr << "creating random cave with seed:" << seed;
	flipx_ = int(rng_() % 100) < params.flipx_chance_;
	flipy_ = int(rng_() % 100) < params.flipy_chance_;

	LOG_NG << "creating scenario....\n";
	generate_chambers();

	LOG_NG << "placing chambers...\n";
	for(std::vector<chamber>::const_iterator c = chambers_.begin(); c != chambers_.end(); ++c) {
		place_chamber(*c);
	}

	LOG_NG << "placing passages...\n";

	for(std::vector<passage>::const_iterator p = passages_.begin(); p != passages_.end(); ++p) {
		place_passage(*p);
	}
	LOG_NG << "outputting map....\n";

	res_["map_data"] = t_translation::write_game_map(map_, starting_positions_);
}

void cave_map_generator::cave_map_generator_job::build_chamber(map_location loc, std::set<map_location>& locs, size_t size, size_t jagged)
{
	if(size == 0 || locs.count(loc) != 0 || !params.on_board(loc))
		return;

	locs.insert(loc);

	map_location adj[6];
	get_adjacent_tiles(loc,adj);
	for(size_t n = 0; n != 6; ++n) {
		if(int(rng_() % 100) < (100l - static_cast<long>(jagged))) {
			build_chamber(adj[n],locs,size-1,jagged);
		}
	}
}

void cave_map_generator::cave_map_generator_job::generate_chambers()
{
	BOOST_FOREACH(const config &ch, params.cfg_.child_range("chamber"))
	{
		// If there is only a chance of the chamber appearing, deal with that here.
		if (ch.has_attribute("chance") && int(rng_() % 100) < ch["chance"].to_int()) {
			continue;
		}

		const std::string &xpos = ch["x"];
		const std::string &ypos = ch["y"];

		size_t min_xpos = 0, min_ypos = 0, max_xpos = params.width_, max_ypos = params.height_;

		if (!xpos.empty()) {
			const std::vector<std::string>& items = utils::split(xpos, '-');
			if(items.empty() == false) {
				min_xpos = atoi(items.front().c_str()) - 1;
				max_xpos = atoi(items.back().c_str());
			}
		}

		if (!ypos.empty()) {
			const std::vector<std::string>& items = utils::split(ypos, '-');
			if(items.empty() == false) {
				min_ypos = atoi(items.front().c_str()) - 1;
				max_ypos = atoi(items.back().c_str());
			}
		}
		const size_t x = translate_x(min_xpos + (rng_()%(max_xpos-min_xpos)));
		const size_t y = translate_y(min_ypos + (rng_()%(max_ypos-min_ypos)));

		int chamber_size = ch["size"].to_int(3);
		int jagged_edges = ch["jagged"];

		chamber new_chamber;
		new_chamber.center = map_location(x,y);
		build_chamber(new_chamber.center,new_chamber.locs,chamber_size,jagged_edges);

		const config &items = ch.child("items");
		new_chamber.items = items ? &items : NULL;

		const std::string &id = ch["id"];
		if (!id.empty()) {
			chamber_ids_[id] = chambers_.size();
		}

		chambers_.push_back(new_chamber);

		BOOST_FOREACH(const config &p, ch.child_range("passage"))
		{
			const std::string &dst = p["destination"];

			// Find the destination of this passage
			const std::map<std::string,size_t>::const_iterator itor = chamber_ids_.find(dst);
			if(itor == chamber_ids_.end())
				continue;

			assert(itor->second < chambers_.size());

			passages_.push_back(passage(new_chamber.center, chambers_[itor->second].center, p));
		}
	}
}

void cave_map_generator::cave_map_generator_job::place_chamber(const chamber& c)
{
	for(std::set<map_location>::const_iterator i = c.locs.begin(); i != c.locs.end(); ++i) {
		set_terrain(*i,params.clear_);
	}

	if (c.items == NULL || c.locs.empty()) return;

	size_t index = 0;
	BOOST_FOREACH(const config::any_child &it, c.items->all_children_range())
	{
		config cfg = it.cfg;
		config &filter = cfg.child("filter");
		config* object_filter = NULL;
		if (config &object = cfg.child("object")) {
			if (config &of = object.child("filter"))
				object_filter = &of;
		}

		if (!it.cfg["same_location_as_previous"].to_bool()) {
			index = rng_()%c.locs.size();
		}
		std::string loc_var = it.cfg["store_location_as"];

		std::set<map_location>::const_iterator loc = c.locs.begin();
		std::advance(loc,index);

		cfg["x"] = loc->x + 1;
		cfg["y"] = loc->y + 1;

		if (filter) {
			filter["x"] = loc->x + 1;
			filter["y"] = loc->y + 1;
		}

		if (object_filter) {
			(*object_filter)["x"] = loc->x + 1;
			(*object_filter)["y"] = loc->y + 1;
		}

		// If this is a side, place a castle for the side
		if (it.key == "side" && !it.cfg["no_castle"].to_bool()) {
			place_castle(it.cfg["side"].to_int(-1), *loc);
		}

		res_.add_child(it.key, cfg);

		if(!loc_var.empty()) {
			config &temp = res_.add_child("event");
			temp["name"] = "prestart";
			config &xcfg = temp.add_child("set_variable");
			xcfg["name"] = loc_var + "_x";
			xcfg["value"] = loc->x + 1;
			config &ycfg = temp.add_child("set_variable");
			ycfg["name"] = loc_var + "_y";
			ycfg["value"] = loc->y + 1;
		}
	}
}

struct passage_path_calculator : pathfind::cost_calculator
{
	passage_path_calculator(const t_translation::t_map& mapdata,
	                        const t_translation::t_terrain & wall,
	                        double laziness, size_t windiness,
							boost::random::mt19937& rng) :
		map_(mapdata), wall_(wall), laziness_(laziness), windiness_(windiness), rng_(rng)
	{}

	virtual double cost(const map_location& loc, const double so_far) const;
private:
	const t_translation::t_map& map_;
	t_translation::t_terrain wall_;
	double laziness_;
	size_t windiness_;
	boost::random::mt19937& rng_;
};

double passage_path_calculator::cost(const map_location& loc, const double) const
{
	double res = 1.0;
	if (map_[loc.x + gamemap::default_border][loc.y + gamemap::default_border] == wall_) {
		res = laziness_;
	}

	if(windiness_ > 1) {
		res *= double(rng_()%windiness_);
	}

	return res;
}

void cave_map_generator::cave_map_generator_job::place_passage(const passage& p)
{
	const std::string& chance = p.cfg["chance"];
	if(chance != "" && int(rng_()%100) < atoi(chance.c_str())) {
		return;
	}


	int windiness = p.cfg["windiness"];
	double laziness = std::max<double>(1.0, p.cfg["laziness"].to_double());

	passage_path_calculator calc(map_, params.wall_, laziness, windiness, rng_);

	pathfind::plain_route rt = a_star_search(p.src, p.dst, 10000.0, &calc, params.width_, params.height_);

	int width = std::max<int>(1, p.cfg["width"].to_int());
	int jagged = p.cfg["jagged"];

	for(std::vector<map_location>::const_iterator i = rt.steps.begin(); i != rt.steps.end(); ++i) {
		std::set<map_location> locs;
		build_chamber(*i,locs,width,jagged);
		for(std::set<map_location>::const_iterator j = locs.begin(); j != locs.end(); ++j) {
			set_terrain(*j, params.clear_);
		}
	}
}

void cave_map_generator::cave_map_generator_job::set_terrain(map_location loc, const t_translation::t_terrain & t)
{
	if (params.on_board(loc)) {
		t_translation::t_terrain& c = map_[loc.x + gamemap::default_border][loc.y + gamemap::default_border];

		if(c == params.clear_ || c == params.wall_ || c == params.village_) {
			// Change this terrain.
			if ( t == params.clear_  &&  int(rng_() % 1000) < params.village_density_ )
				// Override with a village.
				c = params.village_;
			else
				c = t;
		}
	}
}

void cave_map_generator::cave_map_generator_job::place_castle(int starting_position, const map_location &loc)
{
	if (starting_position != -1) {
		set_terrain(loc, params.keep_);

		t_translation::coordinate coord(
				  loc.x + gamemap::default_border
				, loc.y + gamemap::default_border);
		starting_positions_[starting_position] = coord;
	}

	map_location adj[6];
	get_adjacent_tiles(loc,adj);
	for(size_t n = 0; n != 6; ++n) {
		set_terrain(adj[n], params.castle_);
	}
}

