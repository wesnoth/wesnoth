/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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
 * Map-generator, with standalone testprogram.
 */

#include "generators/default_map_generator_job.hpp"
#include "formula/string_utils.hpp"
#include "game_config_manager.hpp"
#include "gettext.hpp"
#include "language.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "generators/map_generator.hpp" // mapgen_exception
#include "pathfind/pathfind.hpp"
#include "pathutils.hpp"
#include "utils/name_generator_factory.hpp"
#include "seed_rng.hpp"
#include "wml_exception.hpp"

#include <SDL.h>

static lg::log_domain log_mapgen("mapgen");
#define ERR_NG LOG_STREAM(err, log_mapgen)
#define LOG_NG LOG_STREAM(info, log_mapgen)

typedef std::vector<std::vector<int> > height_map;
typedef t_translation::ter_map terrain_map;

namespace {
	/**
	 * Calculates the cost of building a road over terrain. For use in the
	 * a_star_search algorithm.
	 */
	struct road_path_calculator : pathfind::cost_calculator
	{
		road_path_calculator(const terrain_map& terrain, const config& cfg, int seed)
			: calls(0)
			, map_(terrain)
			, cfg_(cfg)
			, windiness_(std::max<int>(1, cfg["road_windiness"].to_int())) // Find out how windey roads should be.
			, seed_(seed)
			, cache_()
		{
		}

		virtual double cost(const map_location& loc, const double so_far) const;

		mutable int calls;
	private:
		const terrain_map& map_;
		const config& cfg_;
		int windiness_;
		int seed_;
		mutable std::map<t_translation::terrain_code, double> cache_;
	};

	double road_path_calculator::cost(const map_location& loc, const double /*so_far*/) const
	{
		++calls;
		if(loc.x < 0 || loc.y < 0 || loc.x >= map_.w || loc.y >= map_.h) {

			return (pathfind::cost_calculator::getNoPathValue());
		}

		// We multiply the cost by a random amount,
		// depending upon how 'windy' the road should be.
		// If windiness is 1, that will mean that the cost is always genuine,
		// and so the road always takes the shortest path.
		// If windiness is greater than 1, we sometimes over-report costs
		// for some segments, to make the road wind a little.

		double windiness = 1.0;

		if(windiness_ > 1) {
			// modified pseudo_random taken from builder.cpp
			unsigned int a = (loc.x + 92872973) ^ 918273;
			unsigned int b = (loc.y + 1672517) ^ 128123;
			unsigned int c = a*b + a + b + seed_;
			unsigned int random = c*c;
			// this is just "big random number modulo windiness_"
			// but avoid the "modulo by a low number (like 2)"
			// because it can increase arithmetic patterns
			int noise = random % (windiness_ * 137) / 137;
			windiness += noise;
		}

		const t_translation::terrain_code c = map_[loc.x][loc.y];
		const std::map<t_translation::terrain_code, double>::const_iterator itor = cache_.find(c);
		if(itor != cache_.end()) {
			return itor->second*windiness;
		}

		static std::string terrain;
		terrain = t_translation::write_terrain_code(c);
		double res = getNoPathValue();
		if(const config &child = cfg_.find_child("road_cost", "terrain", terrain)) {
			res = child["cost"].to_double();
		}

		cache_.emplace(c, res);
		return windiness*res;
	}


	struct is_valid_terrain
	{
		is_valid_terrain(const t_translation::ter_map& map, const t_translation::ter_list& terrain_list);
		bool operator()(int x, int y) const;
	private:
		t_translation::ter_map map_;
		const t_translation::ter_list& terrain_;
	};

	is_valid_terrain::is_valid_terrain(const t_translation::ter_map& map, const t_translation::ter_list& terrain_list)
		: map_(map), terrain_(terrain_list)
	{
	}

	bool is_valid_terrain::operator()(int x, int y) const
	{
		if(x < 0 || x >= map_.w || y < 0 || y >= map_.h) {

			return false;
		}

		return std::find(terrain_.begin(),terrain_.end(),map_[x][y]) != terrain_.end();
	}


	/* the configuration file should contain a number of [height] tags:
	 *   [height]
	 *     height=n
	 *     terrain=x
	 *   [/height]
	 * These should be in descending order of n.
	 * They are checked sequentially, and if height is greater than n for that tile,
	 * then the tile is set to terrain type x.
	 */
	class terrain_height_mapper
	{
	public:
		explicit terrain_height_mapper(const config& cfg);

		bool convert_terrain(const int height) const;
		t_translation::terrain_code convert_to() const;

	private:
		int terrain_height;
		t_translation::terrain_code to;
	};

	terrain_height_mapper::terrain_height_mapper(const config& cfg) :
		terrain_height(cfg["height"]),
		to(t_translation::GRASS_LAND)
	{
		const std::string& terrain = cfg["terrain"];
		if(terrain != "") {
			to = t_translation::read_terrain_code(terrain);
		}
	}

	bool terrain_height_mapper::convert_terrain(const int height) const
	{
		return height >= terrain_height;
	}

	t_translation::terrain_code terrain_height_mapper::convert_to() const
	{
		return to;
	}


	class terrain_converter
	{
	public:
		explicit terrain_converter(const config& cfg);

		bool convert_terrain(const t_translation::terrain_code & terrain, const int height, const int temperature) const;
		t_translation::terrain_code convert_to() const;

	private:
		int min_temp, max_temp, min_height, max_height;
		t_translation::ter_list from;
		t_translation::terrain_code to;
	};

	terrain_converter::terrain_converter(const config& cfg)
		: min_temp(cfg["min_temperature"].to_int(-100000))
		, max_temp(cfg["max_temperature"].to_int(100000))
		, min_height(cfg["min_height"].to_int(-100000))
		, max_height(cfg["max_height"].to_int(100000))
		, from(t_translation::read_list(cfg["from"]))
		, to(t_translation::NONE_TERRAIN)
	{
		const std::string& to_str = cfg["to"];
		if(to_str != "") {
			to = t_translation::read_terrain_code(to_str);
		}
	}

	bool terrain_converter::convert_terrain(const t_translation::terrain_code & terrain,
			const int height, const int temperature) const
	{
		return std::find(from.begin(),from.end(),terrain) != from.end() && height >= min_height && height <= max_height && temperature >= min_temp && temperature <= max_temp && to != t_translation::NONE_TERRAIN;
	}

	t_translation::terrain_code terrain_converter::convert_to() const
	{
		return to;
	}

} // end anon namespace


default_map_generator_job::default_map_generator_job()
	: rng_(seed_rng::next_seed())
	, game_config_(game_config_manager::get()->game_config())
{
}

default_map_generator_job::default_map_generator_job(uint32_t seed)
	: rng_(seed)
	, game_config_(game_config_manager::get()->game_config())
{
}

/**
 * Generate a height-map.
 *
 * Basically we generate a lot of hills, each hill being centered at a certain
 * point, with a certain radius - being a half sphere.  Hills are combined
 * additively to form a bumpy surface.  The size of each hill varies randomly
 * from 1-hill_size.  We generate 'iterations' hills in total.  The range of
 * heights is normalized to 0-1000.  'island_size' controls whether or not the
 * map should tend toward an island shape, and if so, how large the island
 * should be.  Hills with centers that are more than 'island_size' away from
 * the center of the map will be inverted (i.e. be valleys).  'island_size' as
 * 0 indicates no island.
 */
height_map default_map_generator_job::generate_height_map(size_t width, size_t height, size_t iterations, size_t hill_size, size_t island_size, size_t island_off_center)
{
	height_map res(width, std::vector<int>(height,0));

	size_t center_x = width/2;
	size_t center_y = height/2;

	LOG_NG << "off-centering...\n";

	if(island_off_center != 0) {
		switch(rng_()%4) {
		case 0:
			center_x += island_off_center;
			break;
		case 1:
			center_y += island_off_center;
			break;
		case 2:
			if(center_x < island_off_center) {
				center_x = 0;
			} else {
				center_x -= island_off_center;
			}
			break;
		case 3:
			if(center_y < island_off_center) {
				center_y = 0;
			} else {
				center_y -= island_off_center;
			}
			break;
		}
	}

	for(size_t i = 0; i != iterations; ++i) {

		// (x1,y1) is the location of the hill,
		// and 'radius' is the radius of the hill.
		// We iterate over all points, (x2,y2).
		// The formula for the amount the height is increased by is:
		// radius - sqrt((x2-x1)^2 + (y2-y1)^2) with negative values ignored.
		//
		// Rather than iterate over every single point, we can reduce the points
		// to a rectangle that contains all the positive values for this formula --
		// the rectangle is given by min_x,max_x,min_y,max_y.

		// Is this a negative hill? (i.e. a valley)
		bool is_valley = false;

		int x1 = island_size > 0 ? center_x - island_size + (rng_()%(island_size*2)) : int(rng_()%width);
		int y1 = island_size > 0 ? center_y - island_size + (rng_()%(island_size*2)) : int(rng_()%height);

		// We have to check whether this is actually a valley
		if(island_size != 0) {
			const size_t diffx = std::abs(x1 - int(center_x));
			const size_t diffy = std::abs(y1 - int(center_y));
			const size_t dist = size_t(std::sqrt(double(diffx*diffx + diffy*diffy)));
			is_valley = dist > island_size;
		}

		const int radius = rng_()%hill_size + 1;

		const int min_x = x1 - radius > 0 ? x1 - radius : 0;
		const int max_x = x1 + radius < static_cast<long>(res.size()) ? x1 + radius : res.size();
		const int min_y = y1 - radius > 0 ? y1 - radius : 0;
		const int max_y = y1 + radius < static_cast<long>(res.front().size()) ? y1 + radius : res.front().size();

		for(int x2 = min_x; x2 < max_x; ++x2) {
			for(int y2 = min_y; y2 < max_y; ++y2) {
				const int xdiff = (x2-x1);
				const int ydiff = (y2-y1);

				const int hill_height = radius - int(std::sqrt(double(xdiff*xdiff + ydiff*ydiff)));

				if(hill_height > 0) {
					if(is_valley) {
						if(hill_height > res[x2][y2]) {
							res[x2][y2] = 0;
						} else {
							res[x2][y2] -= hill_height;
						}
					} else {
						res[x2][y2] += hill_height;
					}
				}
			}
		}
	}

	// Find the highest and lowest points on the map for normalization:
	int heighest = 0, lowest = 100000, x;
	for(x = 0; size_t(x) != res.size(); ++x) {
		for(int y = 0; size_t(y) != res[x].size(); ++y) {
			if(res[x][y] > heighest) {
				heighest = res[x][y];
			}

			if(res[x][y] < lowest) {
				lowest = res[x][y];
			}
		}
	}

	// Normalize the heights to the range 0-1000:
	heighest -= lowest;
	for(x = 0; size_t(x) != res.size(); ++x) {
		for(int y = 0; size_t(y) != res[x].size(); ++y) {
			res[x][y] -= lowest;
			res[x][y] *= 1000;
			if(heighest != 0) {
				res[x][y] /= heighest;
			}
		}
	}

	return res;
}

/**
 * Generate a lake.
 *
 * It will create water at (x,y), and then have 'lake_fall_off' % chance to
 * make another water tile in each of the directions n,s,e,w.  In each of the
 * directions it does make another water tile, it will have 'lake_fall_off'/2 %
 * chance to make another water tile in each of the directions. This will
 * continue recursively.
 */
bool default_map_generator_job::generate_lake(terrain_map& terrain, int x, int y, int lake_fall_off, std::set<map_location>& locs_touched)
{
	if(x < 0 || y < 0 || x >= terrain.w || y >= terrain.h || lake_fall_off < 0) {
		return false;
	}
	//we checked for this eariler.
	unsigned int ulake_fall_off = lake_fall_off;
	terrain[x][y] = t_translation::SHALLOW_WATER;
	locs_touched.insert(map_location(x,y));

	if((rng_()%100) < ulake_fall_off) {
		generate_lake(terrain,x+1,y,lake_fall_off/2,locs_touched);
	}

	if((rng_()%100) < ulake_fall_off) {
		generate_lake(terrain,x-1,y,lake_fall_off/2,locs_touched);
	}

	if((rng_()%100) < ulake_fall_off) {
		generate_lake(terrain,x,y+1,lake_fall_off/2,locs_touched);
	}

	if((rng_()%100) < ulake_fall_off) {
		generate_lake(terrain,x,y-1,lake_fall_off/2,locs_touched);
	}

	return true;
}

/**
 * River generation.
 *
 * Rivers have a source, and then keep on flowing until they meet another body
 * of water, which they flow into, or until they reach the edge of the map.
 * Rivers will always flow downhill, except that they can flow a maximum of
 * 'river_uphill' uphill.  This is to represent the water eroding the higher
 * ground lower.
 *
 * Every possible path for a river will be attempted, in random order, and the
 * first river path that can be found that makes the river flow into another
 * body of water or off the map will be used.
 *
 * If no path can be found, then the river's generation will be aborted, and
 * false will be returned.  true is returned if the river is generated
 * successfully.
 */

bool default_map_generator_job::generate_river_internal(const height_map& heights,
	terrain_map& terrain, int x, int y, std::vector<map_location>& river,
	std::set<map_location>& seen_locations, int river_uphill)
{
	const bool on_map = x >= 0 && y >= 0 &&
		x < static_cast<long>(heights.size()) &&
		y < static_cast<long>(heights.back().size());

	if(on_map && !river.empty() && heights[x][y] >
			heights[river.back().x][river.back().y] + river_uphill) {

		return false;
	}

	// If we're at the end of the river
	if(!on_map || terrain[x][y] == t_translation::SHALLOW_WATER ||
			terrain[x][y] == t_translation::DEEP_WATER) {

		LOG_NG << "generating river...\n";

		// Generate the river
		for(auto i : river) {
			terrain[i.x][i.y] = t_translation::SHALLOW_WATER;
		}

		LOG_NG << "done generating river\n";

		return true;
	}

	map_location current_loc(x,y);
	map_location adj[6];
	get_adjacent_tiles(current_loc,adj);
	std::shuffle(std::begin(adj), std::end(adj), rng_);

	// Mark that we have attempted from this map_location
	seen_locations.insert(current_loc);
	river.push_back(current_loc);
	for(const map_location& loc : adj) {
		if(seen_locations.count(loc) == 0) {
			const bool res = generate_river_internal(heights,terrain,loc.x,loc.y,river,seen_locations,river_uphill);
			if(res) {
				return true;
			}

		}
	}

	river.pop_back();

	return false;
}

std::vector<map_location> default_map_generator_job::generate_river(const height_map& heights, terrain_map& terrain, int x, int y, int river_uphill)
{
	std::vector<map_location> river;
	std::set<map_location> seen_locations;
	const bool res = generate_river_internal(heights,terrain,x,y,river,seen_locations,river_uphill);
	if(!res) {
		river.clear();
	}

	return river;
}

/**
 * Returns a random tile at one of the borders of a map that is of the given
 * dimensions.
 */
map_location default_map_generator_job::random_point_at_side(size_t width, size_t height)
{
	const int side = rng_()%4;
	if(side < 2) {
		const int x = rng_()%width;
		const int y = side == 0 ? 0 : height-1;
		return map_location(x,y);
	} else {
		const int y = rng_()%height;
		const int x = side == 2 ? 0 : width-1;
		return map_location(x,y);
	}
}

/** Function which, given the map will output it in a valid format. */
static std::string output_map(const terrain_map& terrain,
		t_translation::starting_positions& starting_positions)
{
	// Remember that we only want the middle 1/9th of the map.
	// All other segments of the map are there only to give
	// the important middle part some context.
	// We also have a border so also adjust for that.
	const size_t begin_x = terrain.w / 3 - gamemap::default_border ;
	const size_t end_x = terrain.w * 2 / 3 + gamemap::default_border;
	const size_t begin_y = terrain.h / 3 - gamemap::default_border;
	const size_t end_y = terrain.h * 2 / 3 + gamemap::default_border;

	terrain_map map(end_x - begin_x, end_y - begin_y);
	for(size_t y = begin_y; y != end_y; ++y) {
		for(size_t x = begin_x; x != end_x; ++x) {
			map[x - begin_x][y - begin_y] = terrain[x][y];
		}
	}

	// Since the map has been resized,
	// the starting locations also need to be fixed
	for (auto it = starting_positions.left.begin(); it != starting_positions.left.end(); ++it) {
		starting_positions.left.modify_data(it, [=](t_translation::coordinate&  pos) { pos.x -= begin_x; pos.y -= begin_y; });
	}
	return t_translation::write_game_map(map, starting_positions);
}

static int rank_castle_location(int x, int y, const is_valid_terrain& valid_terrain, int min_x, int max_x, int min_y, int max_y, size_t min_distance, const std::vector<map_location>& other_castles, int highest_ranking)
{
	const map_location loc(x,y);

	size_t avg_distance = 0, lowest_distance = 1000;

	for(std::vector<map_location>::const_iterator c = other_castles.begin(); c != other_castles.end(); ++c) {
		const size_t distance = distance_between(loc,*c);
		if(distance < 6) {
			return 0;
		}

		if(distance < lowest_distance) {
			lowest_distance = distance;
		}

		if(distance < min_distance) {
			avg_distance = 0;
			return -1;
		}

		avg_distance += distance;
	}

	if(!other_castles.empty()) {
		avg_distance /= other_castles.size();
	}

	for(int i = x-1; i <= x+1; ++i) {
		for(int j = y-1; j <= y+1; ++j) {
			if(!valid_terrain(i,j)) {
				return 0;
			}
		}
	}

	const int x_from_border = std::min<int>(x - min_x,max_x - x);
	const int y_from_border = std::min<int>(y - min_y,max_y - y);

	const int border_ranking = min_distance - std::min<int>(x_from_border,y_from_border) + min_distance - x_from_border - y_from_border;

	int current_ranking = border_ranking*2 + avg_distance*10 + lowest_distance*10;
	static const int num_nearby_locations = 11*11;

	const int max_possible_ranking = current_ranking + num_nearby_locations;

	if(max_possible_ranking < highest_ranking) {
		return current_ranking;
	}

	int surrounding_ranking = 0;

	for(int xpos = x-5; xpos <= x+5; ++xpos) {
		for(int ypos = y-5; ypos <= y+5; ++ypos) {
			if(valid_terrain(xpos,ypos)) {
				++surrounding_ranking;
			}
		}
	}

	return surrounding_ranking + current_ranking;
}

typedef std::map<t_translation::terrain_code, t_translation::ter_list> tcode_list_cache;

static map_location place_village(const t_translation::ter_map& map,
	const size_t x, const size_t y, const size_t radius, const config& cfg,
	tcode_list_cache &adj_liked_cache)
{
	const map_location loc(x,y);
	std::set<map_location> locs;
	get_tiles_radius(loc,radius,locs);
	map_location best_loc;
	int best_rating = 0;
	for(auto i : locs) {
		if(i.x < 0 || i.y < 0 || i.x >= map.w ||
				i.y >= map.h) {

			continue;
		}

		const t_translation::terrain_code t = map[i.x][i.y];
		const std::string str = t_translation::write_terrain_code(t);
		if(const config &child = cfg.find_child("village", "terrain", str)) {
			tcode_list_cache::iterator l = adj_liked_cache.find(t);
			t_translation::ter_list *adjacent_liked;
			if(l != adj_liked_cache.end()) {
				adjacent_liked = &(l->second);
			} else {
				adj_liked_cache[t] = t_translation::read_list(child["adjacent_liked"]);
				adjacent_liked = &(adj_liked_cache[t]);
			}

			int rating = child["rating"];
			map_location adj[6];
			get_adjacent_tiles(map_location(i.x,i.y),adj);
			for(size_t n = 0; n != 6; ++n) {
				if(adj[n].x < 0 || adj[n].y < 0 ||
						adj[n].x >= map.w ||
						adj[n].y >= map.h) {

					continue;
				}

				const t_translation::terrain_code t2 = map[adj[n].x][adj[n].y];
				rating += std::count(adjacent_liked->begin(),adjacent_liked->end(),t2);
			}

			if(rating > best_rating) {
				best_loc = map_location(i.x,i.y);
				best_rating = rating;
			}
		}
	}

	return best_loc;
}

// "flood fill" a tile name to adjacent tiles of certain terrain
static void flood_name(const map_location& start, const std::string& name, std::map<map_location,std::string>& tile_names,
	const t_translation::ter_match& tile_types, const terrain_map& terrain,
	unsigned width, unsigned height,
	size_t label_count, std::map<map_location,std::string>* labels, const std::string& full_name) {
	map_location adj[6];
	get_adjacent_tiles(start,adj);
	size_t n;
	//if adjacent tiles are tiles and unnamed, name them
	for(n = 0; n < 6; n++) {
		//we do not care for tiles outside the middle part
		//cast to unsigned to skip x < 0 || y < 0 as well.
		if(unsigned(adj[n].x) >= width / 3 || unsigned(adj[n].y) >= height / 3) {
			continue;
		}

		const t_translation::terrain_code terr = terrain[adj[n].x + (width / 3)][adj[n].y + (height / 3)];
		const map_location loc(adj[n].x, adj[n].y);
		if((t_translation::terrain_matches(terr, tile_types)) && (tile_names.find(loc) == tile_names.end())) {
			tile_names.emplace(loc, name);
			//labeling decision: this is result of trial and error on what looks best in game
			if(label_count % 6 == 0) { //ensure that labels do not occur more often than every 6 recursions
				labels->emplace(loc, full_name);
				label_count++; //ensure that no adjacent tiles get labeled
			}
			flood_name(adj[n], name, tile_names, tile_types, terrain, width, height, label_count++, labels, full_name);
		}
	}
}

std::string default_map_generator_job::default_generate_map(generator_data data, std::map<map_location,std::string>* labels, const config& cfg)
{
	log_scope("map generation");

	// Odd widths are nasty
	VALIDATE(is_even(data.width), _("Random maps with an odd width aren't supported."));

	// Try to find configuration for castles
	const config& castle_config = cfg.child("castle");
	if(!castle_config) {
		LOG_NG << "Could not find castle configuration\n";
		return std::string();
	}

	int ticks = SDL_GetTicks();

	// We want to generate a map that is 9 times bigger than the actual size desired.
	// Only the middle part of the map will be used, but the rest is so that the map we
	// end up using can have a context (e.g. rivers flowing from out of the map into the map,
	// same for roads, etc.)
	data.width  *= 3;
	data.height *= 3;

	config naming = game_config_.child("naming");

	if(cfg.has_child("naming")) {
		naming.append_attributes(cfg.child("naming"));
	}

	// If the [naming] child is empty, we cannot provide good names.
	std::map<map_location,std::string>* misc_labels = naming.empty() ? nullptr : labels;

	std::shared_ptr<name_generator>
		base_name_generator, river_name_generator, lake_name_generator,
		road_name_generator, bridge_name_generator, mountain_name_generator,
		forest_name_generator, swamp_name_generator;

	if(misc_labels != nullptr) {
		name_generator_factory base_generator_factory{ naming, {"male", "base", "bridge", "road", "river", "forest", "lake", "mountain", "swamp"} };

		naming.get_old_attribute("base_names", "male_names", "[naming]male_names= is deprecated, use base_names= instead");
		//Due to the attribute detection feature of the factory we also support male_name_generator= but keep it undocumented.

		base_name_generator = base_generator_factory.get_name_generator( (naming.has_attribute("base_names") || naming.has_attribute("base_name_generator")) ? "base" : "male" );
		river_name_generator    = base_generator_factory.get_name_generator("river");
		lake_name_generator     = base_generator_factory.get_name_generator("lake");
		road_name_generator     = base_generator_factory.get_name_generator("road");
		bridge_name_generator   = base_generator_factory.get_name_generator("bridge");
		mountain_name_generator = base_generator_factory.get_name_generator("mountain");
		forest_name_generator   = base_generator_factory.get_name_generator("forest");
		swamp_name_generator    = base_generator_factory.get_name_generator("swamp");
	}

	// Generate the height of everything.
	const height_map heights = generate_height_map(data.width, data.height, data.iterations, data.hill_size, data.island_size, data.island_off_center);

	LOG_NG << "Done generating height map. " << (SDL_GetTicks() - ticks) << " ticks elapsed" << "\n";
	ticks = SDL_GetTicks();

	// Find out what the 'flatland' on this map is, i.e. grassland.
	std::string flatland = cfg["default_flatland"];
	if(flatland.empty()) {
		flatland = t_translation::write_terrain_code(t_translation::GRASS_LAND);
	}

	const t_translation::terrain_code grassland = t_translation::read_terrain_code(flatland);

	std::vector<terrain_height_mapper> height_conversion;
	for(const config& h : cfg.child_range("height")) {
		height_conversion.push_back(terrain_height_mapper(h));
	}

	terrain_map terrain(data.width, data.height, grassland);
	for(size_t x = 0; x != heights.size(); ++x) {
		for(size_t y = 0; y != heights[x].size(); ++y) {
			for(auto i : height_conversion) {
				if(i.convert_terrain(heights[x][y])) {
					terrain[x][y] = i.convert_to();
					break;
				}
			}
		}
	}

	t_translation::starting_positions starting_positions;
	LOG_NG << output_map(terrain, starting_positions);
	LOG_NG << "Placed landforms. " << (SDL_GetTicks() - ticks) << " ticks elapsed" << "\n";
	ticks = SDL_GetTicks();

	/* Now that we have our basic set of flatland/hills/mountains/water,
	 * we can place lakes and rivers on the map.
	 * All rivers are sourced at a lake.
	 * Lakes must be in high land - at least 'min_lake_height'.
	 * (Note that terrain below a certain altitude may be made into bodies of water
	 *  in the code above - i.e. 'sea', but these are not considered 'lakes',
	 *  because they are not sources of rivers).
	 *
	 * We attempt to place 'max_lakes' lakes.
	 * Each lake will be placed at a random location, if that random location meets theminimum
	 * terrain requirements for a lake. We will also attempt to source a river from each lake.
	 */
	std::set<map_location> lake_locs;

	std::map<map_location, std::string> river_names, lake_names, road_names, bridge_names, mountain_names, forest_names, swamp_names;

	const size_t nlakes = data.max_lakes > 0 ? (rng_()%data.max_lakes) : 0;
	for(size_t lake = 0; lake != nlakes; ++lake) {
		for(int tries = 0; tries != 100; ++tries) {
			const int x = rng_()%data.width;
			const int y = rng_()%data.height;

			if(heights[x][y] <= cfg["min_lake_height"].to_int()) {
				continue;
			}

			std::vector<map_location> river = generate_river(heights, terrain, x, y, cfg["river_frequency"]);

			if(!river.empty() && misc_labels != nullptr) {
				const std::string base_name = base_name_generator->generate();
				const std::string& name = river_name_generator->generate({{"base",  base_name}});
				LOG_NG << "Named river '" << name << "'\n";

				size_t name_frequency = 20;
				for(std::vector<map_location>::const_iterator r = river.begin(); r != river.end(); ++r) {
					const map_location loc(r->x-data.width/3,r->y-data.height/3);

					if(((r - river.begin())%name_frequency) == name_frequency/2) {
						misc_labels->emplace(loc, name);
					}

					river_names.emplace(loc, base_name);
				}
			}

			LOG_NG << "Generating lake...\n";

			std::set<map_location> locs;
			if(generate_lake(terrain, x, y, cfg["lake_size"], locs) && misc_labels != nullptr) {
				bool touches_other_lake = false;

				std::string base_name = base_name_generator->generate();
				const std::string& name = lake_name_generator->generate({{"base",  base_name}});

				// Only generate a name if the lake hasn't touched any other lakes,
				// so that we don't end up with one big lake with multiple names.
				for(auto i : locs) {
					if(lake_locs.count(i) != 0) {
						touches_other_lake = true;

						// Reassign the name of this lake to be the same as the other lake
						const map_location loc(i.x-data.width/3,i.y-data.height/3);
						const std::map<map_location,std::string>::const_iterator other_name = lake_names.find(loc);
						if(other_name != lake_names.end()) {
							base_name = other_name->second;
						}
					}

					lake_locs.insert(i);
				}

				if(!touches_other_lake) {
					const map_location loc(x-data.width/3,y-data.height/3);
					misc_labels->erase(loc);
					misc_labels->emplace(loc, name);
				}

				for(auto i : locs) {
					const map_location loc(i.x-data.width/3,i.y-data.height/3);
					lake_names.emplace(loc, base_name);
				}
			}

			break;
		}
	}

	LOG_NG << "Generated rivers. " << (SDL_GetTicks() - ticks) << " ticks elapsed" << "\n";
	ticks = SDL_GetTicks();

	const size_t default_dimensions = 40*40*9;

	/*
	 * Convert grassland terrain to other types of flat terrain.
	 *
	 * We generate a 'temperature map' which uses the height generation
	 * algorithm to generate the temperature levels all over the map.  Then we
	 * can use a combination of height and terrain to divide terrain up into
	 * more interesting types than the default.
	 */
	const height_map temperature_map = generate_height_map(data.width,data.height,
		cfg["temperature_iterations"].to_int() * data.width * data.height / default_dimensions,
		cfg["temperature_size"], 0, 0);

	LOG_NG << "Generated temperature map. " << (SDL_GetTicks() - ticks) << " ticks elapsed" << "\n";
	ticks = SDL_GetTicks();

	std::vector<terrain_converter> converters;
	for(const config& cv : cfg.child_range("convert")) {
		converters.push_back(terrain_converter(cv));
	}

	LOG_NG << "Created terrain converters. " << (SDL_GetTicks() - ticks) << " ticks elapsed" << "\n";
	ticks = SDL_GetTicks();

	// Iterate over every flatland tile, and determine what type of flatland it is, based on our [convert] tags.
	for(int x = 0; x != data.width; ++x) {
		for(int y = 0; y != data.height; ++y) {
			for(auto i : converters) {
				if(i.convert_terrain(terrain[x][y],heights[x][y],temperature_map[x][y])) {
					terrain[x][y] = i.convert_to();
					break;
				}
			}
		}
	}

	LOG_NG << "Placing castles...\n";

	/*
	 * Castle configuration tag contains a 'valid_terrain' attribute which is a
	 * list of terrains that the castle may appear on.
	 */
	const t_translation::ter_list list =
		t_translation::read_list(castle_config["valid_terrain"]);

	const is_valid_terrain terrain_tester(terrain, list);

	/*
	 * Attempt to place castles at random.
	 *
	 * After they are placed, we run a sanity check to make sure no two castles
	 * are closer than 'min_distance' hexes apart, and that they appear on a
	 * terrain listed in 'valid_terrain'.
	 *
	 * If not, we attempt to place them again.
	 */
	std::vector<map_location> castles;
	std::set<map_location> failed_locs;

	for(int player = 0; player != data.nplayers; ++player) {
		LOG_NG << "placing castle for " << player << "\n";
		lg::scope_logger inner_scope_logging_object__(lg::general(), "placing castle");
		const int min_x = data.width/3 + 3;
		const int min_y = data.height/3 + 3;
		const int max_x = (data.width/3)*2 - 4;
		const int max_y = (data.height/3)*2 - 4;
		int min_distance = castle_config["min_distance"];

		map_location best_loc;
		int best_ranking = 0;
		for(int x = min_x; x != max_x; ++x) {
			for(int y = min_y; y != max_y; ++y) {
				const map_location loc(x,y);
				if(failed_locs.count(loc)) {
					continue;
				}

				const int ranking = rank_castle_location(x, y, terrain_tester, min_x, max_x, min_y, max_y, min_distance, castles, best_ranking);
				if(ranking <= 0) {
					failed_locs.insert(loc);
				}

				if(ranking > best_ranking) {
					best_ranking = ranking;
					best_loc = loc;
				}
			}
		}

		if(best_ranking == 0) {
			ERR_NG << "No castle location found, aborting." << std::endl;
			const std::string error = _("No valid castle location found. Too many or too few mountain hexes? (please check the 'max hill size' parameter)");
			throw mapgen_exception(error);
		}

		assert(std::find(castles.begin(), castles.end(), best_loc) == castles.end());
		castles.push_back(best_loc);

		// Make sure the location can't get a second castle.
		failed_locs.insert(best_loc);
	}

	LOG_NG << "Placing roads...\n";
	ticks = SDL_GetTicks();

	// Place roads.
	// We select two tiles at random locations on the borders of the map
	// and try to build roads between them.
	int nroads = cfg["roads"];
	if(data.link_castles) {
		nroads += castles.size()*castles.size();
	}

	std::set<map_location> bridges;

	road_path_calculator calc(terrain, cfg, rng_());
	for(int road = 0; road != nroads; ++road) {
		lg::scope_logger another_inner_scope_logging_object__(lg::general(), "creating road");

		/*
		 * We want the locations to be on the portion of the map we're actually
		 * going to use, since roads on other parts of the map won't have any
		 * influence, and doing it like this will be quicker.
		 */
		map_location src = random_point_at_side(data.width/3 + 2,data.height/3 + 2);
		map_location dst = random_point_at_side(data.width/3 + 2,data.height/3 + 2);

		src.x += data.width/3 - 1;
		src.y += data.height/3 - 1;
		dst.x += data.width/3 - 1;
		dst.y += data.height/3 - 1;

		if(data.link_castles && road < int(castles.size() * castles.size())) {
			const size_t src_castle = road/castles.size();
			const size_t dst_castle = road%castles.size();
			if(src_castle >= dst_castle) {
				continue;
			}

			src = castles[src_castle];
			dst = castles[dst_castle];
		} else if(src.x == dst.x || src.y == dst.y) {
			// If the road isn't very interesting (on the same border), don't draw it.
			continue;
		}

		if(calc.cost(src, 0.0) >= 1000.0 || calc.cost(dst, 0.0) >= 1000.0) {
			continue;
		}

		// Search a path out for the road
		pathfind::plain_route rt = pathfind::a_star_search(src, dst, 10000.0, calc, data.width, data.height);

		const std::string& road_base_name = misc_labels != nullptr
			? base_name_generator->generate()
			: "";
		const std::string& road_name = misc_labels != nullptr
			? road_name_generator->generate({{"base", road_base_name}})
			: "";
		const int name_frequency = 20;
		int name_count = 0;

		bool on_bridge = false;

		// Draw the road.
		// If the search failed, rt.steps will simply be empty.
		for(std::vector<map_location>::const_iterator step = rt.steps.begin();
				step != rt.steps.end(); ++step) {

			const int x = step->x;
			const int y = step->y;

			if(x < 0 || y < 0 || x >= static_cast<long>(data.width) || y >= static_cast<long>(data.height)) {
				continue;
			}

			// Find the configuration which tells us what to convert this tile to, to make it into a road.
			const config& child = cfg.find_child("road_cost", "terrain", t_translation::write_terrain_code(terrain[x][y]));
			if(child.empty()){
				continue;
			}

			/* Convert to bridge means that we want to convert depending on the direction of the road.
			 * Typically it will be in a format like convert_to_bridge = \,|,/
			 * '|' will be used if the road is going north-south
			 * '/' will be used if the road is going south west-north east
			 * '\' will be used if the road is going south east-north west
			 * The terrain will be left unchanged otherwise (if there is no clear direction).
			 */
			const std::string& convert_to_bridge = child["convert_to_bridge"];
			if(!convert_to_bridge.empty()) {
				if(step == rt.steps.begin() || step+1 == rt.steps.end()) {
					continue;
				}

				const map_location& last = *(step-1);
				const map_location& next = *(step+1);

				map_location adj[6];
				get_adjacent_tiles(*step,adj);

				int direction = -1;

				// If we are going north-south
				if((last == adj[0] && next == adj[3]) || (last == adj[3] && next == adj[0])) {
					direction = 0;
				}

				// If we are going south west-north east
				else if((last == adj[1] && next == adj[4]) || (last == adj[4] && next == adj[1])) {
					direction = 1;
				}

				// If we are going south east-north west
				else if((last == adj[2] && next == adj[5]) || (last == adj[5] && next == adj[2])) {
					direction = 2;
				}

				if(misc_labels != nullptr && !on_bridge) {
					on_bridge = true;
					std::string bridge_base_name = base_name_generator->generate();
					const std::string& name = bridge_name_generator->generate({{"base",  bridge_base_name}});
					const map_location loc(x - data.width / 3, y-data.height/3);
					misc_labels->emplace(loc, name);
					bridge_names.emplace(loc, bridge_base_name); //add to use for village naming
					bridges.insert(loc);
				}

				if(direction != -1) {
					const std::vector<std::string> items = utils::split(convert_to_bridge);
					if(size_t(direction) < items.size() && !items[direction].empty()) {
						terrain[x][y] = t_translation::read_terrain_code(items[direction]);
					}

					continue;
				}
			} else {
				on_bridge = false;
			}

			// Just a plain terrain substitution for a road
			const std::string& convert_to = child["convert_to"];
			if(!convert_to.empty()) {
				const t_translation::terrain_code letter = t_translation::read_terrain_code(convert_to);
				if(misc_labels != nullptr && terrain[x][y] != letter && name_count++ == name_frequency && !on_bridge) {
					misc_labels->emplace(map_location(x - data.width / 3, y - data.height / 3), road_name);
					name_count = 0;
				}

				terrain[x][y] = letter;
				if(misc_labels != nullptr) {
					const map_location loc(x - data.width / 3, y - data.height / 3); //add to use for village naming
					if(road_base_name != "")
						road_names.emplace(loc, road_base_name);
				}
			}
		}
	}

	// Now that road drawing is done, we can plonk down the castles.
	for(std::vector<map_location>::const_iterator c = castles.begin(); c != castles.end(); ++c) {
		if(!c->valid()) {
			continue;
		}

		const int x = c->x;
		const int y = c->y;
		const int player = c - castles.begin() + 1;
		const t_translation::coordinate coord(x, y);
		starting_positions.insert(t_translation::starting_positions::value_type(std::to_string(player), coord));
		terrain[x][y] = t_translation::HUMAN_KEEP;

		const int castle_array[13][2] = {
			{-1, 0}, {-1, -1}, {0, -1}, {1, -1}, {1, 0}, {0, 1}, {-1, 1},
			{-2, 1}, {-2, 0}, {-2, -1}, {-1, -2}, {0, -2}, {1, -2}
		};

		for(int i = 0; i < data.castle_size - 1; i++) {
			terrain[x+ castle_array[i][0]][y+ castle_array[i][1]] = t_translation::HUMAN_CASTLE;
		}

		// Remove all labels under the castle tiles
		if(labels != nullptr) {
			labels->erase(map_location(x-data.width/3,y-data.height/3));
			for(int i = 0; i < data.castle_size - 1; i++) {
				labels->erase(map_location(x+ castle_array[i][0]-data.width/3, y+ castle_array[i][1]-data.height/3));
			}
		}
	}

	LOG_NG << "Placed roads and castles. " << (SDL_GetTicks() - ticks) << " ticks elapsed" << "\n";
	ticks = SDL_GetTicks();

	/* Random naming for landforms: mountains, forests, swamps, hills
	 * we name these now that everything else is placed (as e.g., placing
	 * roads could split a forest)
	 */
	if(misc_labels != nullptr) {
		for(int x = data.width / 3; x < (data.width / 3)*2; x++) {
			for(int y = data.height / 3; y < (data.height / 3) * 2;y++) {
				//check the terrain of the tile
				const map_location loc(x - data.width / 3, y - data.height / 3);
				const t_translation::terrain_code terr = terrain[x][y];
				std::string name, base_name;
				std::set<std::string> used_names;

				if(t_translation::terrain_matches(terr, t_translation::ALL_MOUNTAINS)) {
					//name every 15th mountain
					if((rng_() % 15) == 0) {
						for(size_t ntry = 0; ntry != 30 && (ntry == 0 || used_names.count(name) > 0); ++ntry) {
							base_name = base_name_generator->generate();
							name = mountain_name_generator->generate({{"base",  base_name}});
						}
						misc_labels->emplace(loc, name);
						mountain_names.emplace(loc, base_name);
					}
				} else if(t_translation::terrain_matches(terr, t_translation::ALL_FORESTS)) {
					// If the forest tile is not named yet, name it
					const std::map<map_location, std::string>::const_iterator forest_name = forest_names.find(loc);
					if(forest_name == forest_names.end()) {
						for(size_t ntry = 0; ntry != 30 && (ntry == 0 || used_names.count(name) > 0); ++ntry) {
							base_name = base_name_generator->generate();
							name = forest_name_generator->generate({{"base",  base_name}});
						}
						forest_names.emplace(loc, base_name);
						// name all connected forest tiles accordingly
						flood_name(loc, base_name, forest_names, t_translation::ALL_FORESTS, terrain, data.width, data.height, 0, misc_labels, name);
					}
				} else if(t_translation::terrain_matches(terr, t_translation::ALL_SWAMPS)) {
					// If the swamp tile is not named yet, name it
					const std::map<map_location, std::string>::const_iterator swamp_name = swamp_names.find(loc);
					if(swamp_name == swamp_names.end()) {
						for(size_t ntry = 0; ntry != 30 && (ntry == 0 || used_names.count(name) > 0); ++ntry) {
							base_name = base_name_generator->generate();
							name = swamp_name_generator->generate({{"base",  base_name}});
						}
						swamp_names.emplace(loc, base_name);
						// name all connected swamp tiles accordingly
						flood_name(loc, base_name, swamp_names, t_translation::ALL_SWAMPS, terrain, data.width, data.height, 0, misc_labels, name);
					}
				}
			}
		}
	}

	LOG_NG << "Placing villages...\n";
	ticks = SDL_GetTicks();

	/*
	 * Place villages in a 'grid', to make placing fair, but with villages
	 * displaced from their position according to terrain and randomness, to
	 * add some variety.
	 */
	std::set<map_location> villages;

	if(data.nvillages > 0) {

		// First we work out the size of the x and y distance between villages
		const size_t tiles_per_village = ((data.width*data.height)/9)/data.nvillages;
		size_t village_x = 1, village_y = 1;

		// Alternate between incrementing the x and y value.
		// When they are high enough to equal or exceed the tiles_per_village,
		// then we have them to the value we want them at.
		while(village_x*village_y < tiles_per_village) {
			if(village_x < village_y) {
				++village_x;
			} else {
				++village_y;
			}
		}

		std::set<std::string> used_names;
		tcode_list_cache adj_liked_cache;

		config village_naming = game_config_.child("village_naming");

		if(cfg.has_child("village_naming")) {
			village_naming.append_attributes(cfg.child("village_naming"));
		}

		// If the [village_naming] child is empty, we cannot provide good names.
		std::map<map_location,std::string>* village_labels = village_naming.empty() ? nullptr : labels;

		for(int vx = 0; vx < data.width; vx += village_x) {
			LOG_NG << "village at " << vx << "\n";

			for(int vy = rng_()%village_y; vy < data.height; vy += village_y) {
				const size_t add = rng_()%3;
				const size_t x = (vx + add) - 1;
				const size_t y = (vy + add) - 1;

				const map_location res = place_village(terrain, x, y, 2, cfg, adj_liked_cache);

				if(res.x  < static_cast<long>(data.width     ) / 3 ||
				   res.x >= static_cast<long>(data.width  * 2) / 3 ||
				   res.y  < static_cast<long>(data.height    ) / 3 ||
				   res.y >= static_cast<long>(data.height * 2) / 3) {
					continue;
				}

				const std::string str = t_translation::write_terrain_code(terrain[res.x][res.y]);

				const std::string& convert_to = cfg.find_child("village", "terrain", str)["convert_to"].str();
				if(convert_to.empty()) {
					continue;
				}

				terrain[res.x][res.y] = t_translation::read_terrain_code(convert_to);

				villages.insert(res);

				if(village_labels == nullptr) {
					continue;
				}

				name_generator_factory village_name_generator_factory{ village_naming,
					{"base", "male", "village", "lake", "river", "bridge", "grassland", "forest", "hill", "mountain", "mountain_anon", "road", "swamp"} };

				village_naming.get_old_attribute("base_names", "male_names", "[village_naming]male_names= is deprecated, use base_names= instead");
				//Due to the attribute detection feature of the factory we also support male_name_generator= but keep it undocumented.

				base_name_generator = village_name_generator_factory.get_name_generator(
					(village_naming.has_attribute("base_names") || village_naming.has_attribute("base_name_generator")) ? "base" : "male" );

				const map_location loc(res.x-data.width/3,res.y-data.height/3);

				map_location adj[6];
				get_adjacent_tiles(loc,adj);

				std::string name_type = "village";
				const t_translation::ter_list
					field	 = t_translation::ter_list(1, t_translation::GRASS_LAND),
					forest   = t_translation::ter_list(1, t_translation::FOREST),
					mountain = t_translation::ter_list(1, t_translation::MOUNTAIN),
					hill	 = t_translation::ter_list(1, t_translation::HILL);

				size_t field_count = 0, forest_count = 0, mountain_count = 0, hill_count = 0;

				std::map<std::string,std::string> symbols;

				size_t n;
				for(n = 0; n != 6; ++n) {
					const std::map<map_location,std::string>::const_iterator road_name = road_names.find(adj[n]);
					if(road_name != road_names.end()) {
						symbols["road"] = road_name->second;
						name_type = "road";
						break;
					}

					const std::map<map_location,std::string>::const_iterator river_name = river_names.find(adj[n]);
					if(river_name != river_names.end()) {
						symbols["river"] = river_name->second;
						name_type = "river";

						const std::map<map_location,std::string>::const_iterator bridge_name = bridge_names.find(adj[n]);
						if(bridge_name != bridge_names.end()) {
							//we should always end up here, since if there is an adjacent bridge, there has to be an adjacent river too
							symbols["bridge"] = bridge_name->second;
							name_type = "river_bridge";
						}

						break;
					}

					const std::map<map_location,std::string>::const_iterator forest_name = forest_names.find(adj[n]);
					if(forest_name != forest_names.end()) {
						symbols["forest"] = forest_name->second;
						name_type = "forest";
						break;
					}

					const std::map<map_location,std::string>::const_iterator lake_name = lake_names.find(adj[n]);
					if(lake_name != lake_names.end()) {
						symbols["lake"] = lake_name->second;
						name_type = "lake";
						break;
					}

					const std::map<map_location,std::string>::const_iterator mountain_name = mountain_names.find(adj[n]);
					if(mountain_name != mountain_names.end()) {
						symbols["mountain"] = mountain_name->second;
						name_type = "mountain";
						break;
					}

					const std::map<map_location,std::string>::const_iterator swamp_name = swamp_names.find(adj[n]);
					if(swamp_name != swamp_names.end()) {
						symbols["swamp"] = swamp_name->second;
						name_type = "swamp";
						break;
					}

					const t_translation::terrain_code terr = terrain[adj[n].x+data.width/3][adj[n].y+data.height/3];

					if(std::count(field.begin(),field.end(),terr) > 0) {
						++field_count;
					} else if(std::count(forest.begin(),forest.end(),terr) > 0) {
						++forest_count;
					} else if(std::count(hill.begin(),hill.end(),terr) > 0) {
						++hill_count;
					} else if(std::count(mountain.begin(),mountain.end(),terr) > 0) {
						++mountain_count;
					}
				}

				if(n == 6) {
					if(field_count == 6) {
						name_type = "grassland";
					} else if(forest_count >= 2) {
						name_type = "forest";
					} else if(mountain_count >= 1) {
						name_type = "mountain_anon";
					} else if(hill_count >= 2) {
						name_type = "hill";
					}
				}

				std::string name;

				symbols["base"] = base_name_generator->generate();
				std::shared_ptr<name_generator> village_name_generator = village_name_generator_factory.get_name_generator(name_type);

				for(size_t ntry = 0; ntry != 30 && (ntry == 0 || used_names.count(name) > 0); ++ntry) {
					name = village_name_generator->generate(symbols);
				}

				used_names.insert(name);
				village_labels->emplace(loc, name);
			}
		}
	}

	LOG_NG << "Placed villages. " << (SDL_GetTicks() - ticks) << " ticks elapsed" << "\n";

	return output_map(terrain, starting_positions);
}
