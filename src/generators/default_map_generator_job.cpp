/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
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

#include "global.hpp"

#include "gettext.hpp"
#include "language.hpp"
#include "log.hpp"
#include "map.hpp"
#include "map_generator.hpp"// mapgen_esxeption
#include "default_map_generator_job.hpp"
#include "pathfind/pathfind.hpp"
#include "pathutils.hpp"
#include "race.hpp"
#include "util.hpp"
#include "wml_exception.hpp"
#include "formula_string_utils.hpp"
#include "SDL.h"

#include <boost/foreach.hpp>
#include "seed_rng.hpp"
static lg::log_domain log_mapgen("mapgen");
#define ERR_NG LOG_STREAM(err, log_mapgen)
#define LOG_NG LOG_STREAM(info, log_mapgen)

default_map_generator_job::default_map_generator_job()
	: rng_(seed_rng::next_seed())
{

}

default_map_generator_job::default_map_generator_job(uint32_t seed)
	: rng_(seed)
{

}


typedef std::vector<std::vector<int> > height_map;
typedef t_translation::t_map terrain_map;
typedef map_location location;

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
height_map default_map_generator_job::generate_height_map(size_t width, size_t height,
                               size_t iterations, size_t hill_size,
							   size_t island_size, size_t island_off_center)
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
			if(center_x < island_off_center)
				center_x = 0;
			else
				center_x -= island_off_center;
			break;
		case 3:
			if(center_y < island_off_center)
				center_y = 0;
			else
				center_y -= island_off_center;
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

		int x1 = island_size > 0 ? center_x - island_size + (rng_()%(island_size*2)) :
			                                 int(rng_()%width);
		int y1 = island_size > 0 ? center_y - island_size + (rng_()%(island_size*2)) :
			                                 int(rng_()%height);

		// We have to check whether this is actually a valley
		if(island_size != 0) {
			const size_t diffx = abs(x1 - int(center_x));
			const size_t diffy = abs(y1 - int(center_y));
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

				const int height = radius - int(std::sqrt(double(xdiff*xdiff + ydiff*ydiff)));

				if(height > 0) {
					if(is_valley) {
						if(height > res[x2][y2]) {
							res[x2][y2] = 0;
						} else {
							res[x2][y2] -= height;
						}
					} else {
						res[x2][y2] += height;
					}
				}
			}
		}
	}

	// Find the highest and lowest points on the map for normalization:
	int heighest = 0, lowest = 100000, x;
	for(x = 0; size_t(x) != res.size(); ++x) {
		for(int y = 0; size_t(y) != res[x].size(); ++y) {
			if(res[x][y] > heighest)
				heighest = res[x][y];

			if(res[x][y] < lowest)
				lowest = res[x][y];
		}
	}

	// Normalize the heights to the range 0-1000:
	heighest -= lowest;
	for(x = 0; size_t(x) != res.size(); ++x) {
		for(int y = 0; size_t(y) != res[x].size(); ++y) {
			res[x][y] -= lowest;
			res[x][y] *= 1000;
			if(heighest != 0)
				res[x][y] /= heighest;
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
bool default_map_generator_job::generate_lake(terrain_map& terrain, int x, int y, int lake_fall_off, std::set<location>& locs_touched)
{
	if(x < 0 || y < 0 || size_t(x) >= terrain.size() || size_t(y) >= terrain.front().size() || lake_fall_off < 0) {
		return false;
	}
	//we checked for this eariler.
	unsigned int ulake_fall_off = lake_fall_off;
	terrain[x][y] = t_translation::SHALLOW_WATER;
	locs_touched.insert(location(x,y));

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
static bool generate_river_internal(const height_map& heights,
	terrain_map& terrain, int x, int y, std::vector<location>& river,
	std::set<location>& seen_locations, int river_uphill)
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
		for(std::vector<location>::const_iterator i = river.begin();
		    i != river.end(); ++i) {
			terrain[i->x][i->y] = t_translation::SHALLOW_WATER;
		}

		LOG_NG << "done generating river\n";

		return true;
	}

	location current_loc(x,y);
	location adj[6];
	get_adjacent_tiles(current_loc,adj);
	static int items[6] = {0,1,2,3,4,5};
	std::random_shuffle(items,items+4);

	// Mark that we have attempted from this location
	seen_locations.insert(current_loc);
	river.push_back(current_loc);
	for(int a = 0; a != 6; ++a) {
		const location& loc = adj[items[a]];
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

static std::vector<location> generate_river(const height_map& heights, terrain_map& terrain, int x, int y, int river_uphill)
{
	std::vector<location> river;
	std::set<location> seen_locations;
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
		return location(x,y);
	} else {
		const int y = rng_()%height;
		const int x = side == 2 ? 0 : width-1;
		return location(x,y);
	}
}

/** Function which, given the map will output it in a valid format. */
static std::string output_map(const terrain_map& terrain,
		std::map<int, t_translation::coordinate> starting_positions)
{
	// Remember that we only want the middle 1/9th of the map.
	// All other segments of the map are there only to give
	// the important middle part some context.
	// We also have a border so also adjust for that.
	const size_t begin_x = terrain.size() / 3 - gamemap::default_border ;
	const size_t end_x = terrain.size() * 2 / 3 + gamemap::default_border;
	const size_t begin_y = terrain.front().size() / 3 - gamemap::default_border;
	const size_t end_y = terrain.front().size() * 2 / 3 + gamemap::default_border;

	terrain_map map;
	map.resize(end_x - begin_x);
	for(size_t y = begin_y; y != end_y; ++y) {
		for(size_t x = begin_x; x != end_x; ++x) {
			if((y - begin_y) == 0){
				map[x - begin_x].resize(end_y - begin_y);
			}
			map[x - begin_x][y - begin_y] = terrain[x][y];
		}
	}

	// Since the map has been resized,
	// the starting locations also need to be fixed
	std::map<int, t_translation::coordinate>::iterator itor = starting_positions.begin();
	for(; itor != starting_positions.end(); ++itor) {
		itor->second.x -= begin_x;
		itor->second.y -= begin_y;
	}

	return gamemap::default_map_header + t_translation::write_game_map(map, starting_positions);
}

namespace {

/**
 * Calculates the cost of building a road over terrain. For use in the
 * a_star_search algorithm.
 */
struct road_path_calculator : pathfind::cost_calculator
{
	road_path_calculator(const terrain_map& terrain, const config& cfg, int seed) :
    	calls(0),
		map_(terrain),
		cfg_(cfg),
		// Find out how windy roads should be.
		windiness_(std::max<int>(1, cfg["road_windiness"].to_int())),
		seed_(seed),
		cache_()
	{
	}

	virtual double cost(const location& loc, const double so_far) const;

	mutable int calls;
private:
	const terrain_map& map_;
	const config& cfg_;
	int windiness_;
	int seed_;
	mutable std::map<t_translation::t_terrain, double> cache_;
};

double road_path_calculator::cost(const location& loc,
	const double /*so_far*/) const
{
	++calls;
	if (loc.x < 0 || loc.y < 0 || loc.x >= static_cast<long>(map_.size()) ||
			loc.y >= static_cast<long>(map_.front().size())) {

		return (pathfind::cost_calculator::getNoPathValue());
	}

	// We multiply the cost by a random amount,
	// depending upon how 'windy' the road should be.
	// If windiness is 1, that will mean that the cost is always genuine,
	// and so the road always takes the shortest path.
	// If windiness is greater than 1, we sometimes over-report costs
	// for some segments, to make the road wind a little.

	double windiness = 1.0;

	if (windiness_ > 1) {
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

	const t_translation::t_terrain c = map_[loc.x][loc.y];
	const std::map<t_translation::t_terrain, double>::const_iterator itor = cache_.find(c);
	if(itor != cache_.end()) {
		return itor->second*windiness;
	}

	static std::string terrain;
	terrain = t_translation::write_terrain_code(c);
	double res = getNoPathValue();
	if (const config &child = cfg_.find_child("road_cost", "terrain", terrain)) {
		res = child["cost"].to_double();
	}

	cache_.insert(std::pair<t_translation::t_terrain, double>(c,res));
	return windiness*res;
}

struct is_valid_terrain
{
	is_valid_terrain(const t_translation::t_map& map,
			const t_translation::t_list& terrain_list);
	bool operator()(int x, int y) const;
private:
	t_translation::t_map map_;
	const t_translation::t_list& terrain_;
};

is_valid_terrain::is_valid_terrain(const t_translation::t_map& map,
		const t_translation::t_list& terrain_list)
: map_(map), terrain_(terrain_list)
{}

bool is_valid_terrain::operator()(int x, int y) const
{
	if(x < 0 || x >= static_cast<long>(map_.size()) ||
			y < 0 || y >= static_cast<long>(map_[x].size())) {

		return false;
	}

	return std::find(terrain_.begin(),terrain_.end(),map_[x][y]) != terrain_.end();
}

}

static int rank_castle_location(int x, int y, const is_valid_terrain& valid_terrain, int min_x, int max_x, int min_y, int max_y,
						 size_t min_distance, const std::vector<map_location>& other_castles, int highest_ranking)
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

	if(other_castles.empty() == false) {
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

	const int border_ranking = min_distance - std::min<int>(x_from_border,y_from_border) +
	                           min_distance - x_from_border - y_from_border;

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

typedef std::map<t_translation::t_terrain, t_translation::t_list> tcode_list_cache;

static map_location place_village(const t_translation::t_map& map,
	const size_t x, const size_t y, const size_t radius, const config& cfg,
	tcode_list_cache &adj_liked_cache)
{
	const map_location loc(x,y);
	std::set<map_location> locs;
	get_tiles_radius(loc,radius,locs);
	map_location best_loc;
	int best_rating = 0;
	for(std::set<map_location>::const_iterator i = locs.begin();
			i != locs.end(); ++i) {

		if(i->x < 0 || i->y < 0 || i->x >= static_cast<long>(map.size()) ||
				i->y >= static_cast<long>(map[i->x].size())) {

			continue;
		}

		const t_translation::t_terrain t = map[i->x][i->y];
		const std::string str = t_translation::write_terrain_code(t);
		if (const config &child = cfg.find_child("village", "terrain", str)) {
			tcode_list_cache::iterator l = adj_liked_cache.find(t);
			t_translation::t_list *adjacent_liked;
			if (l != adj_liked_cache.end()) {
				adjacent_liked = &(l->second);
			} else {
				adj_liked_cache[t] = t_translation::read_list(child["adjacent_liked"]);
				adjacent_liked = &(adj_liked_cache[t]);
			}

			int rating = child["rating"];
			map_location adj[6];
			get_adjacent_tiles(map_location(i->x,i->y),adj);
			for(size_t n = 0; n != 6; ++n) {
				if(adj[n].x < 0 || adj[n].y < 0 ||
						adj[n].x >= static_cast<long>(map.size()) ||
						adj[n].y >= static_cast<long>(map[adj[n].x].size())) {

					continue;
				}

				const t_translation::t_terrain t2 = map[adj[n].x][adj[n].y];
				rating += std::count(adjacent_liked->begin(),adjacent_liked->end(),t2);
			}

			if(rating > best_rating) {
				best_loc = map_location(i->x,i->y);
				best_rating = rating;
			}
		}
	}

	return best_loc;
}

std::string default_map_generator_job::generate_name(const unit_race& name_generator, const std::string& id,
		std::string* base_name, utils::string_map* additional_symbols)
{
	const std::vector<std::string>& options = utils::split(string_table[id].str());
	if(options.empty() == false) {
		const size_t choice = rng_()%options.size();
		LOG_NG << "calling name generator...\n";
		const std::string& name = name_generator.generate_name(unit_race::MALE);
		LOG_NG << "name generator returned '" << name << "'\n";
		if(base_name != NULL) {
			*base_name = name;
		}

		LOG_NG << "assigned base name..\n";
		utils::string_map  table;
		if(additional_symbols == NULL) {
			additional_symbols = &table;
		}

		LOG_NG << "got additional symbols\n";

		(*additional_symbols)["name"] = name;
		LOG_NG << "interpolation variables into '" << options[choice] << "'\n";
		return utils::interpolate_variables_into_string(options[choice], additional_symbols);
	}

	return "";
}

// "flood fill" a tile name to adjacent tiles of certain terrain
static void flood_name(const map_location& start, const std::string& name, std::map<map_location,std::string>& tile_names,
	const t_translation::t_match& tile_types, const terrain_map& terrain,
	unsigned width, unsigned height,
	size_t label_count, std::map<map_location,std::string>* labels, const std::string& full_name) {
	map_location adj[6];
	get_adjacent_tiles(start,adj);
	size_t n;
	//if adjacent tiles are tiles and unnamed, name them
	for (n = 0; n < 6; n++) {
		//we do not care for tiles outside the middle part
		//cast to unsigned to skip x < 0 || y < 0 as well.
		if (unsigned(adj[n].x) >= width / 3 || unsigned(adj[n].y) >= height / 3) {
			continue;
		}

		const t_translation::t_terrain terr = terrain[adj[n].x + (width / 3)][adj[n].y + (height / 3)];
		const location loc(adj[n].x, adj[n].y);
		if((t_translation::terrain_matches(terr, tile_types)) && (tile_names.find(loc) == tile_names.end())) {
			tile_names.insert(std::pair<location, std::string>(loc, name));
			//labeling decision: this is result of trial and error on what looks best in game
			if (label_count % 6 == 0) { //ensure that labels do not occur more often than every 6 recursions
				labels->insert(std::pair<map_location, std::string>(loc, full_name));
				label_count++; //ensure that no adjacent tiles get labeled
			}
			flood_name(adj[n], name, tile_names, tile_types, terrain, width, height, label_count++, labels, full_name);
		}
	}
}

namespace {

// the configuration file should contain a number of [height] tags:
//   [height]
//     height=n
//     terrain=x
//   [/height]
// These should be in descending order of n.
// They are checked sequentially, and if height is greater than n for that tile,
// then the tile is set to terrain type x.
class terrain_height_mapper
{
public:
	explicit terrain_height_mapper(const config& cfg);

	bool convert_terrain(const int height) const;
	t_translation::t_terrain convert_to() const;

private:
	int terrain_height;
	t_translation::t_terrain to;
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

t_translation::t_terrain terrain_height_mapper::convert_to() const
{
	return to;
}

class terrain_converter
{
public:
	explicit terrain_converter(const config& cfg);

	bool convert_terrain(const t_translation::t_terrain & terrain, const int height, const int temperature) const;
	t_translation::t_terrain convert_to() const;

private:
	int min_temp, max_temp, min_height, max_height;
	t_translation::t_list from;
	t_translation::t_terrain to;
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

bool terrain_converter::convert_terrain(const t_translation::t_terrain & terrain,
		const int height, const int temperature) const
{
	return std::find(from.begin(),from.end(),terrain) != from.end() && height >= min_height && height <= max_height &&
	       temperature >= min_temp && temperature <= max_temp && to != t_translation::NONE_TERRAIN;
}

t_translation::t_terrain terrain_converter::convert_to() const
{
	return to;
}

} // end anon namespace

std::string default_map_generator_job::default_generate_map(size_t width, size_t height, size_t island_size, size_t island_off_center,
                                 size_t iterations, size_t hill_size,
						         size_t max_lakes, size_t nvillages, size_t castle_size, size_t nplayers, bool roads_between_castles,
								 std::map<map_location,std::string>* labels, const config& cfg)
{
	log_scope("map generation");

	// Odd widths are nasty
	VALIDATE(is_even(width), _("Random maps with an odd width aren't supported."));

	int ticks = SDL_GetTicks();

	// Find out what the 'flatland' on this map is, i.e. grassland.
	std::string flatland = cfg["default_flatland"];
	if(flatland == "") {
		flatland = t_translation::write_terrain_code(t_translation::GRASS_LAND);
	}

	const t_translation::t_terrain grassland = t_translation::read_terrain_code(flatland);

	// We want to generate a map that is 9 times bigger
	// than the actual size desired.
	// Only the middle part of the map will be used,
	// but the rest is so that the map we end up using
	// can have a context (e.g. rivers flowing from
	// out of the map into the map, same for roads, etc.)
	width *= 3;
	height *= 3;

	LOG_NG << "generating height map...\n";
	// Generate the height of everything.
	const height_map heights = generate_height_map(width,height,iterations,hill_size,island_size,island_off_center);
	LOG_NG << "done generating height map...\n";
	LOG_NG << (SDL_GetTicks() - ticks) << "\n"; ticks = SDL_GetTicks();

	config naming = cfg.child_or_empty("naming");
	// If the [naming] child is empty, we cannot provide good names.
	std::map<map_location,std::string>* misc_labels = naming.empty() ? NULL : labels;
	// HACK: dummy names to satisfy unit_race requirements
	naming["id"] = "village_naming";
	naming["plural_name"] = "villages";

	// Make a dummy race for generating names
	const unit_race name_generator(naming);

	std::vector<terrain_height_mapper> height_conversion;

	BOOST_FOREACH(const config &h, cfg.child_range("height")) {
		height_conversion.push_back(terrain_height_mapper(h));
	}

	terrain_map terrain(width, t_translation::t_list(height, grassland));
	size_t x, y;
	for(x = 0; x != heights.size(); ++x) {
		for(y = 0; y != heights[x].size(); ++y) {
			for(std::vector<terrain_height_mapper>::const_iterator i = height_conversion.begin();
			    i != height_conversion.end(); ++i) {
				if(i->convert_terrain(heights[x][y])) {
					terrain[x][y] = i->convert_to();
					break;
				}
			}
		}
	}

	std::map<int, t_translation::coordinate> starting_positions;
	LOG_NG << output_map(terrain, starting_positions);
	LOG_NG << "placed land forms\n";
	LOG_NG << (SDL_GetTicks() - ticks) << "\n"; ticks = SDL_GetTicks();

	// Now that we have our basic set of flatland/hills/mountains/water,
	// we can place lakes and rivers on the map.
	// All rivers are sourced at a lake.
	// Lakes must be in high land - at least 'min_lake_height'.
	// (Note that terrain below a certain altitude may be made
	// into bodies of water in the code above - i.e. 'sea',
	// but these are not considered 'lakes', because
	// they are not sources of rivers).
	//
	// We attempt to place 'max_lakes' lakes.
	// Each lake will be placed at a random location,
	// if that random location meets the minimum terrain requirements for a lake.
	// We will also attempt to source a river from each lake.
	std::set<location> lake_locs;

	std::map<location, std::string> river_names, lake_names, road_names, bridge_names, mountain_names, forest_names, swamp_names;

	const size_t nlakes = max_lakes > 0 ? (rng_()%max_lakes) : 0;
	for(size_t lake = 0; lake != nlakes; ++lake) {
		for(int tries = 0; tries != 100; ++tries) {
			const int x = rng_()%width;
			const int y = rng_()%height;
			if (heights[x][y] > cfg["min_lake_height"].to_int()) {
				std::vector<location> river = generate_river(heights,
					terrain, x, y, cfg["river_frequency"]);

				if(river.empty() == false && misc_labels != NULL) {
					std::string base_name;
					LOG_NG << "generating name for river...\n";
					const std::string& name = generate_name(name_generator,"river_name",&base_name);
					LOG_NG << "named river '" << name << "'\n";
					size_t name_frequency = 20;
					for(std::vector<location>::const_iterator r = river.begin(); r != river.end(); ++r) {

						const map_location loc(r->x-width/3,r->y-height/3);

						if(((r - river.begin())%name_frequency) == name_frequency/2) {
							misc_labels->insert(std::pair<map_location,std::string>(loc,name));
						}

						river_names.insert(std::pair<location,std::string>(loc,base_name));
					}

					LOG_NG << "put down river name...\n";
				}

				LOG_NG << "generating lake...\n";
				std::set<location> locs;
				bool res = generate_lake(terrain, x, y, cfg["lake_size"], locs);
				if(res && misc_labels != NULL) {
					bool touches_other_lake = false;

					std::string base_name;
					const std::string& name = generate_name(name_generator,"lake_name",&base_name);

					std::set<location>::const_iterator i;

					// Only generate a name if the lake hasn't touched any other lakes,
					// so that we don't end up with one big lake with multiple names.
					for(i = locs.begin(); i != locs.end(); ++i) {
						if(lake_locs.count(*i) != 0) {
							touches_other_lake = true;

							// Reassign the name of this lake to be the same as the other lake
							const location loc(i->x-width/3,i->y-height/3);
							const std::map<location,std::string>::const_iterator other_name = lake_names.find(loc);
							if(other_name != lake_names.end()) {
								base_name = other_name->second;
							}
						}

						lake_locs.insert(*i);
					}

					if(!touches_other_lake) {
						const map_location loc(x-width/3,y-height/3);
						misc_labels->erase(loc);
						misc_labels->insert(std::pair<map_location,std::string>(loc,name));
					}

					for(i = locs.begin(); i != locs.end(); ++i) {
						const location loc(i->x-width/3,i->y-height/3);
						lake_names.insert(std::pair<location, std::string>(loc, base_name));
					}
				}

				break;
			}
		}
	}

	LOG_NG << "done generating rivers...\n";
	LOG_NG << (SDL_GetTicks() - ticks) << "\n"; ticks = SDL_GetTicks();

	const size_t default_dimensions = 40*40*9;

	/*
	 * Convert grassland terrain to other types of flat terrain.
	 *
	 * We generate a 'temperature map' which uses the height generation
	 * algorithm to generate the temperature levels all over the map.  Then we
	 * can use a combination of height and terrain to divide terrain up into
	 * more interesting types than the default.
	 */
	const height_map temperature_map = generate_height_map(width,height,
		cfg["temperature_iterations"].to_int() * width * height / default_dimensions,
		cfg["temperature_size"], 0, 0);

	LOG_NG << "generated temperature map...\n";
	LOG_NG << (SDL_GetTicks() - ticks) << "\n"; ticks = SDL_GetTicks();

	std::vector<terrain_converter> converters;
	BOOST_FOREACH(const config &cv, cfg.child_range("convert")) {
		converters.push_back(terrain_converter(cv));
	}

	LOG_NG << "created terrain converters\n";
	LOG_NG << (SDL_GetTicks() - ticks) << "\n"; ticks = SDL_GetTicks();


	// Iterate over every flatland tile, and determine
	// what type of flatland it is, based on our [convert] tags.
	for(x = 0; x != width; ++x) {
		for(y = 0; y != height; ++y) {
			for(std::vector<terrain_converter>::const_iterator i = converters.begin(); i != converters.end(); ++i) {
				if(i->convert_terrain(terrain[x][y],heights[x][y],temperature_map[x][y])) {
					terrain[x][y] = i->convert_to();
					break;
				}
			}
		}
	}

	LOG_NG << "placing villages...\n";
	LOG_NG << (SDL_GetTicks() - ticks) << "\n"; ticks = SDL_GetTicks();

	/*
	 * Place villages in a 'grid', to make placing fair, but with villages
	 * displaced from their position according to terrain and randomness, to
	 * add some variety.
	 */
	std::set<location> villages;

	LOG_NG << "placing castles...\n";

	/** Try to find configuration for castles. */
	const config &castle_config = cfg.child("castle");
	if (!castle_config) {
		LOG_NG << "Could not find castle configuration\n";
		return std::string();
	}

	/*
	 * Castle configuration tag contains a 'valid_terrain' attribute which is a
	 * list of terrains that the castle may appear on.
	 */
	const t_translation::t_list list =
		t_translation::read_list(castle_config["valid_terrain"]);

	const is_valid_terrain terrain_tester(terrain, list);

	/*
	 * Attempt to place castles at random.
	 *
	 * Once we have placed castles, we run a sanity check to make sure that the
	 * castles are well-placed.  If the castles are not well-placed, we try
	 * again.  Definition of 'well-placed' is if no two castles are closer than
	 * 'min_distance' hexes from each other, and the castles appear on a
	 * terrain listed in 'valid_terrain'.
	 */
	std::vector<location> castles;
	std::set<location> failed_locs;

	for(size_t player = 0; player != nplayers; ++player) {
		LOG_NG << "placing castle for " << player << "\n";
		log_scope("placing castle");
		const int min_x = width/3 + 3;
		const int min_y = height/3 + 3;
		const int max_x = (width/3)*2 - 4;
		const int max_y = (height/3)*2 - 4;
		int min_distance = castle_config["min_distance"];

		location best_loc;
		int best_ranking = 0;
		for(int x = min_x; x != max_x; ++x) {
			for(int y = min_y; y != max_y; ++y) {
				const location loc(x,y);
				if(failed_locs.count(loc)) {
					continue;
				}

				const int ranking = rank_castle_location(x,y,terrain_tester,min_x,max_x,min_y,max_y,min_distance,castles,best_ranking);
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
			std::string error = _("No valid castle location found. Too many or too few mountain hexes? (please check the 'max hill size' parameter)");
			throw mapgen_exception(error);
		}
		assert(std::find(castles.begin(), castles.end(), best_loc) == castles.end());
		castles.push_back(best_loc);
		// Make sure the location can't get a second castle.
		failed_locs.insert(best_loc);
	}

	LOG_NG << "placing roads...\n";
	LOG_NG << (SDL_GetTicks() - ticks) << "\n"; ticks = SDL_GetTicks();

	// Place roads.
	// We select two tiles at random locations on the borders
	// of the map, and try to build roads between them.
	int nroads = cfg["roads"];
	if(roads_between_castles) {
		nroads += castles.size()*castles.size();
	}

	std::set<location> bridges;

	road_path_calculator calc(terrain, cfg, rng_());
	for (int road = 0; road != nroads; ++road) {
		log_scope("creating road");

		/*
		 * We want the locations to be on the portion of the map we're actually
		 * going to use, since roads on other parts of the map won't have any
		 * influence, and doing it like this will be quicker.
		 */
		location src = random_point_at_side(width/3 + 2,height/3 + 2);
		location dst = random_point_at_side(width/3 + 2,height/3 + 2);

		src.x += width/3 - 1;
		src.y += height/3 - 1;
		dst.x += width/3 - 1;
		dst.y += height/3 - 1;

		if (roads_between_castles && road < int(castles.size() * castles.size())) {
			const size_t src_castle = road/castles.size();
			const size_t dst_castle = road%castles.size();
			if(src_castle >= dst_castle) {
				continue;
			}

			src = castles[src_castle];
			dst = castles[dst_castle];
		}

		// If the road isn't very interesting (on the same border), don't draw it.
		else if(src.x == dst.x || src.y == dst.y) {
			continue;
		}

		if (calc.cost(src, 0.0) >= 1000.0 || calc.cost(dst, 0.0) >= 1000.0) {
			continue;
		}

		// Search a path out for the road
		pathfind::plain_route rt = pathfind::a_star_search(src, dst, 10000.0, &calc, width, height);

		std::string road_base_name;
		const std::string& name = generate_name(name_generator, "road_name", &road_base_name);
		const int name_frequency = 20;
		int name_count = 0;

		bool on_bridge = false;

		// Draw the road.
		// If the search failed, rt.steps will simply be empty.
		for(std::vector<location>::const_iterator step = rt.steps.begin();
				step != rt.steps.end(); ++step) {

			const int x = step->x;
			const int y = step->y;

			if(x < 0 || y < 0 || x >= static_cast<long>(width) ||
					y >= static_cast<long>(height)) {

				continue;
			}

			// Find the configuration which tells us
			// what to convert this tile to, to make it into a road.
			if (const config &child = cfg.find_child("road_cost", "terrain",
					t_translation::write_terrain_code(terrain[x][y])))
			{
				// Convert to bridge means that we want to convert
				// depending upon the direction the road is going.
				// Typically it will be in a format like,
				// convert_to_bridge=\,|,/
				// '|' will be used if the road is going north-south
				// '/' will be used if the road is going south west-north east
				// '\' will be used if the road is going south east-north west
				// The terrain will be left unchanged otherwise
				// (if there is no clear direction).
				const std::string &convert_to_bridge = child["convert_to_bridge"];
				if(convert_to_bridge.empty() == false) {
					if(step == rt.steps.begin() || step+1 == rt.steps.end())
						continue;

					const location& last = *(step-1);
					const location& next = *(step+1);

					location adj[6];
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

					if(misc_labels != NULL && on_bridge == false) {
						on_bridge = true;
						std::string bridge_base_name;
						const std::string& name = generate_name(name_generator, "bridge_name", &bridge_base_name);
						const location loc(x - width / 3, y-height/3);
						misc_labels->insert(std::pair<map_location,std::string>(loc,name));
						bridge_names.insert(std::pair<location,std::string>(loc, bridge_base_name)); //add to use for village naming
						bridges.insert(loc);
					}

					if(direction != -1) {
						const std::vector<std::string> items = utils::split(convert_to_bridge);
						if(size_t(direction) < items.size() && items[direction].empty() == false) {
							terrain[x][y] = t_translation::read_terrain_code(items[direction]);
						}

						continue;
					}
				} else {
					on_bridge = false;
				}

				// Just a plain terrain substitution for a road
				const std::string &convert_to = child["convert_to"];
				if(convert_to.empty() == false) {
					const t_translation::t_terrain letter =
						t_translation::read_terrain_code(convert_to);
					if(misc_labels != NULL && terrain[x][y] != letter && name_count++ == name_frequency && on_bridge == false) {
						misc_labels->insert(std::pair<map_location,std::string>(map_location(x-width/3,y-height/3),name));
						name_count = 0;
					}

					terrain[x][y] = letter;
					const location loc(x - width / 3, y - height / 3); //add to use for village naming
					road_names.insert(std::pair<location,std::string>(loc, road_base_name));
				}
			}
		}

		LOG_NG << "looked at " << calc.calls << " locations\n";
	}


	// Now that road drawing is done, we can plonk down the castles.
	for(std::vector<location>::const_iterator c = castles.begin(); c != castles.end(); ++c) {
		if(c->valid() == false) {
			continue;
		}

		const int x = c->x;
		const int y = c->y;
		const int player = c - castles.begin() + 1;
		const struct t_translation::coordinate coord(x, y);
		starting_positions.insert(std::pair<int, t_translation::coordinate>(player, coord));
		terrain[x][y] = t_translation::HUMAN_KEEP;

		const int castles[13][2] = {
		  {-1, 0}, {-1, -1}, {0, -1}, {1, -1}, {1, 0}, {0, 1}, {-1, 1},
		  {-2, 1}, {-2, 0}, {-2, -1}, {-1, -2}, {0, -2}, {1, -2}
		};

		for (size_t i = 0; i < castle_size - 1; i++) {
		  terrain[x+castles[i][0]][y+castles[i][1]] = t_translation::HUMAN_CASTLE;
		}

		// Remove all labels under the castle tiles
		if(labels != NULL) {
		  labels->erase(location(x-width/3,y-height/3));
		  for (size_t i = 0; i < castle_size - 1; i++) {
		    labels->erase(location(x+castles[i][0]-width/3,
					   y+castles[i][1]-height/3));
		  }

		}

	}

	LOG_NG << "placed castles\n";
	LOG_NG << (SDL_GetTicks() - ticks) << "\n"; ticks = SDL_GetTicks();

	/*Random naming for landforms: mountains, forests, swamps, hills
	 *we name these now that everything else is placed (as e.g., placing
	 * roads could split a forest)
	 */
	if ( misc_labels != NULL ) {
		for (x = width / 3; x < (width / 3)*2; x++) {
			for (y = height / 3; y < (height / 3) * 2;y++) {
				//check the terrain of the tile
				const location loc(x - width / 3, y - height / 3);
				const t_translation::t_terrain terr = terrain[x][y];
				std::string name, base_name;
				std::set<std::string> used_names;
				if (t_translation::terrain_matches(terr, t_translation::ALL_MOUNTAINS)) {
					//name every 15th mountain
					if ((rng_()%15) == 0) {
						for(size_t ntry = 0; ntry != 30 && (ntry == 0 || used_names.count(name) > 0); ++ntry) {
							name = generate_name(name_generator, "mountain_name", &base_name);
						}
						misc_labels->insert(std::pair<map_location, std::string>(loc, name));
						mountain_names.insert(std::pair<location, std::string>(loc, base_name));
					}
				}
				else if (t_translation::terrain_matches(terr, t_translation::ALL_FORESTS)) {
					//if the forest tile is not named yet, name it
					const std::map<location, std::string>::const_iterator forest_name = forest_names.find(loc);
					if(forest_name == forest_names.end()) {
						for(size_t ntry = 0; ntry != 30 && (ntry == 0 || used_names.count(name) > 0); ++ntry) {
							name = generate_name(name_generator, "forest_name", &base_name);
						}
						forest_names.insert(std::pair<location, std::string>(loc, base_name));
						// name all connected forest tiles accordingly
						flood_name(loc, base_name, forest_names, t_translation::ALL_FORESTS, terrain, width, height, 0, misc_labels, name);
					}
				}
				else if (t_translation::terrain_matches(terr, t_translation::ALL_SWAMPS)) {
					//if the swamp tile is not named yet, name it
					const std::map<location, std::string>::const_iterator swamp_name = swamp_names.find(loc);
					if(swamp_name == swamp_names.end()) {
						for(size_t ntry = 0; ntry != 30 && (ntry == 0 || used_names.count(name) > 0); ++ntry) {
							name = generate_name(name_generator, "swamp_name", &base_name);
						}
						swamp_names.insert(std::pair<location, std::string>(loc, base_name));
						// name all connected swamp tiles accordingly
						flood_name(loc, base_name, swamp_names, t_translation::ALL_SWAMPS, terrain, width, height, 0, misc_labels, name);
					}
				}
			}//for (y)
		}//for (x)
	}//if (misc_labels)

	if (nvillages > 0)
	{
		config naming_cfg = cfg.child_or_empty("village_naming");
		// If the [village_naming] child is empty, we cannot provide good names.
		std::map<map_location,std::string>* village_labels = naming_cfg.empty() ? NULL : labels;
		// HACK: dummy names to satisfy unit_race requirements
		naming_cfg["id"] = "village_naming";
		naming_cfg["plural_name"] = "villages";

		const unit_race village_names_generator(naming_cfg);

		// First we work out the size of the x and y distance between villages
		const size_t tiles_per_village = ((width*height)/9)/nvillages;
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

		for(size_t vx = 0; vx < width; vx += village_x) {
			LOG_NG << "village at " << vx << "\n";
			for(size_t vy = rng_()%village_y; vy < height; vy += village_y) {

				const size_t add_x = rng_()%3;
				const size_t add_y = rng_()%3;
				const size_t x = (vx + add_x) - 1;
				const size_t y = (vy + add_y) - 1;

				const map_location res = place_village(terrain,x,y,2,cfg,adj_liked_cache);

				if(res.x >= static_cast<long>(width) / 3 &&
						res.x  < static_cast<long>(width * 2) / 3 &&
						res.y >= static_cast<long>(height) / 3 &&
						res.y  < static_cast<long>(height * 2) / 3) {

					const std::string str =
						t_translation::write_terrain_code(terrain[res.x][res.y]);
					if (const config &child = cfg.find_child("village", "terrain", str))
					{
						const std::string &convert_to = child["convert_to"];
						if(convert_to != "") {
							terrain[res.x][res.y] =
								t_translation::read_terrain_code(convert_to);

							villages.insert(res);

							if ( village_labels != NULL ) {
								const map_location loc(res.x-width/3,res.y-height/3);

								map_location adj[6];
								get_adjacent_tiles(loc,adj);

								std::string name_type = "village_name";
								const t_translation::t_list
									field    = t_translation::t_list(1, t_translation::GRASS_LAND),
									forest   = t_translation::t_list(1, t_translation::FOREST),
									mountain = t_translation::t_list(1, t_translation::MOUNTAIN),
									hill     = t_translation::t_list(1, t_translation::HILL);

								size_t field_count = 0, forest_count = 0, mountain_count = 0, hill_count = 0;

								utils::string_map symbols;

								size_t n;
								for(n = 0; n != 6; ++n) {
									const std::map<location,std::string>::const_iterator road_name = road_names.find(adj[n]);
									if(road_name != road_names.end()) {
										symbols["road"] = road_name->second;
										name_type = "village_name_road";
										break;
									}

									const std::map<location,std::string>::const_iterator river_name = river_names.find(adj[n]);
									if(river_name != river_names.end()) {
										symbols["river"] = river_name->second;
										name_type = "village_name_river";

										const std::map<location,std::string>::const_iterator bridge_name = bridge_names.find(adj[n]);
										if(bridge_name != bridge_names.end()) {
										//we should always end up here, since if there is an adjacent bridge, there has to be an adjacent river too
										symbols["bridge"] = bridge_name->second;
										name_type = "village_name_river_bridge";
										}

										break;
									}

									const std::map<location,std::string>::const_iterator forest_name = forest_names.find(adj[n]);
									if(forest_name != forest_names.end()) {
										symbols["forest"] = forest_name->second;
										name_type = "village_name_forest";
										break;
									}

									const std::map<location,std::string>::const_iterator lake_name = lake_names.find(adj[n]);
									if(lake_name != lake_names.end()) {
										symbols["lake"] = lake_name->second;
										name_type = "village_name_lake";
										break;
									}

									const std::map<location,std::string>::const_iterator mountain_name = mountain_names.find(adj[n]);
									if(mountain_name != mountain_names.end()) {
										symbols["mountain"] = mountain_name->second;
										name_type = "village_name_mountain";
										break;
									}

									const std::map<location,std::string>::const_iterator swamp_name = swamp_names.find(adj[n]);
									if(swamp_name != swamp_names.end()) {
										symbols["swamp"] = swamp_name->second;
										name_type = "village_name_swamp";
										break;
									}

									const t_translation::t_terrain terr =
										terrain[adj[n].x+width/3][adj[n].y+height/3];

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
										name_type = "village_name_grassland";
									} else if(forest_count >= 2) {
										name_type = "village_name_forest";
									} else if(mountain_count >= 1) {
										name_type = "village_name_mountain_anonymous";
									} else if(hill_count >= 2) {
										name_type = "village_name_hill";
									}
								}

								std::string name;
								for(size_t ntry = 0; ntry != 30 && (ntry == 0 || used_names.count(name) > 0); ++ntry) {
									name = generate_name(village_names_generator,name_type,NULL,&symbols);
								}

								used_names.insert(name);
								village_labels->insert(std::pair<map_location,std::string>(loc,name));
							}
						}
					}
				}
			}
		}
	}

	LOG_NG << "placed villages\n";
	LOG_NG << (SDL_GetTicks() - ticks) << "\n"; ticks = SDL_GetTicks();

	return output_map(terrain, starting_positions);
}
