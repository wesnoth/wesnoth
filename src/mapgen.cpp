#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <ctime>
#include <vector>

#include "cavegen.hpp"
#include "mapgen.hpp"
#include "mapgen_dialog.hpp"
#include "pathfind.hpp"
#include "util.hpp"

//function to generate a random map, from a string which describes
//the generator to use and its arguments
std::string random_generate_map(const std::string& parms)
{
	//the first token is the name of the generator, tokens after
	//that are arguments to the generator
	std::vector<std::string> parameters = config::split(parms,' ');
	map_generator* const generator = get_map_generator(parameters.front());
	if(generator == NULL) {
		std::cerr << "could not find map generator '" << parameters.front() << "'\n";
		return "";
	}

	parameters.erase(parameters.begin());
	return generator->create_map(parameters);
}

config random_generate_scenario(const std::string& parms)
{
	//the first token is the name of the generator, tokens after
	//that are arguments to the generator
	std::vector<std::string> parameters = config::split(parms,' ');
	map_generator* const generator = get_map_generator(parameters.front());
	if(generator == NULL) {
		std::cerr << "could not find map generator '" << parameters.front() << "'\n";
		return config();
	}

	parameters.erase(parameters.begin());
	return generator->create_scenario(parameters);
}

config map_generator::create_scenario(const std::vector<std::string>& args)
{
	config res;
	config& item = res.add_child("scenario");
	item.values["map_data"] = create_map(args);
	return res;
}

namespace {

typedef std::vector<std::vector<int> > height_map;
typedef std::vector<std::vector<char> > terrain_map;

//basically we generate alot of hills, each hill being centered at a certain point, with a certain radius - being a half sphere.
//Hills are combined additively to form a bumpy surface
//The size of each hill varies randomly from 1-hill_size.
//we generate 'iterations' hills in total.
//the range of heights is normalized to 0-1000
height_map generate_height_map(size_t width, size_t height,
                               size_t iterations, size_t hill_size)
{
	height_map res(width,std::vector<int>(height,0));

	for(size_t i = 0; i != iterations; ++i) {

		//(x1,y1) is the location of the hill, and 'radius' is the radius of the hill.
		//we iterate over all points, (x2,y2). The formula for the amount the height
		//is increased by is radius - sqrt((x2-x1)^2 + (y2-y1)^2) with negative values
		//ignored.
		//
		//rather than iterate over every single point, we can reduce the points to
		//a rectangle that contains all the positive values for this formula --
		//the rectangle is given by min_x,max_x,min_y,max_y

		const int x1 = int(rand()%width);
		const int y1 = int(rand()%height);
		const int radius = rand()%hill_size + 1;

		const int min_x = x1 - radius > 0 ? x1 - radius : 0;
		const int max_x = x1 + radius < res.size() ? x1 + radius : res.size();
		const int min_y = y1 - radius > 0 ? y1 - radius : 0;
		const int max_y = y1 + radius < res.front().size() ? y1 + radius : res.front().size();

		for(int x2 = min_x; size_t(x2) != max_x; ++x2) {
			for(int y2 = min_y; size_t(y2) != max_y; ++y2) {
				const int xdiff = (x2-x1);
				const int ydiff = (y2-y1);
				const int height = radius - int(sqrt(double(xdiff*xdiff + ydiff*ydiff)));
				if(height > 0) {
					res[x2][y2] += height;
				}
			}
		}
	}

	//find the heighest and lowest points on the map for normalization
	int heighest = 0, lowest = 100000, x;
	for(x = 0; size_t(x) != res.size(); ++x) {
		for(int y = 0; size_t(y) != res[x].size(); ++y) {
			if(res[x][y] > heighest)
				heighest = res[x][y];

			if(res[x][y] < lowest)
				lowest = res[x][y];
		}
	}

	//normalize the heights to the range 0-1000
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

//function to generate a lake. It will create water at (x,y), and then have
//'lake_fall_off' % chance to make another water tile in each of the directions n,s,e,w.
//In each of the directions it does make another water tile, it will have 'lake_fall_off'/2 %
//chance to make another water tile in each of the directions. This will continue recursively.
void generate_lake(terrain_map& terrain, int x, int y, int lake_fall_off)
{
	if(x < 0 || y < 0 || size_t(x) >= terrain.size() || size_t(y) >= terrain.front().size()) {
		return;
	}

	terrain[x][y] = 'c';

	if((rand()%100) < lake_fall_off) {
		generate_lake(terrain,x+1,y,lake_fall_off/2);
	}

	if((rand()%100) < lake_fall_off) {
		generate_lake(terrain,x-1,y,lake_fall_off/2);
	}

	if((rand()%100) < lake_fall_off) {
		generate_lake(terrain,x,y+1,lake_fall_off/2);
	}

	if((rand()%100) < lake_fall_off) {
		generate_lake(terrain,x,y-1,lake_fall_off/2);
	}
}

typedef gamemap::location location;

//river generation:
//rivers have a source, and then keep on flowing until they meet another body of water,
//which they flow into, or until they reach the edge of the map. Rivers will always flow
//downhill, except that they can flow a maximum of 'river_uphill' uphill - this is to
//represent the water eroding the higher ground lower.
//
//Every possible path for a river will be attempted, in random order, and the first river
//path that can be found that makes the river flow into another body of water or off the map
//will be used. If no path can be found, then the river's generation will be aborted, and
//false will be returned. true is returned if the river is generated successfully.
bool generate_river_internal(const height_map& heights, terrain_map& terrain, int x, int y, std::vector<location>& river, std::set<location>& seen_locations, int river_uphill)
{
	const bool on_map = x >= 0 && y >= 0 && x < heights.size() && y < heights.back().size();

	if(on_map && !river.empty() && heights[x][y] > heights[river.back().x][river.back().y] + river_uphill) {
		return false;
	}
	
	//if we're at the end of the river
	if(!on_map || terrain[x][y] == 'c' || terrain[x][y] == 's') {
		std::cerr << "generating river...\n";

		//generate the river
		for(std::vector<location>::const_iterator i = river.begin();
		    i != river.end(); ++i) {
			terrain[i->x][i->y] = 'c';
		}

		return true;
	}
	
	location current_loc(x,y);
	location adj[6];
	get_adjacent_tiles(current_loc,adj);
	static int items[6] = {0,1,2,3,4,5};
	std::random_shuffle(items,items+4);

	//mark that we have attempted from this location
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

void generate_river(const height_map& heights, terrain_map& terrain, int x, int y, int river_uphill)
{
	std::vector<location> river;
	std::set<location> seen_locations;
	generate_river_internal(heights,terrain,x,y,river,seen_locations,river_uphill);
}

//function to return a random tile at one of the borders of a map that is
//of the given dimensions.
location point_at_side(size_t width, size_t height)
{
	const int side = rand()%4;
	if(side < 2) {
		const int x = rand()%width;
		const int y = side == 0 ? 0 : height-1;
		return location(x,y);
	} else {
		const int y = rand()%height;
		const int x = side == 2 ? 0 : width-1;
		return location(x,y);
	}
}

//function which, given the map will output it in a valid format.
std::string output_map(const terrain_map& terrain)
{
	std::stringstream res;

	//remember that we only want the middle 1/9th of the map. All other
	//segments of the map are there only to give the important middle part
	//some context.
	const size_t begin_x = terrain.size()/3;
	const size_t end_x = begin_x*2;
	const size_t begin_y = terrain.front().size()/3;
	const size_t end_y = begin_y*2;

	for(size_t y = begin_y; y != end_y; ++y) {
		for(size_t x = begin_x; x != end_x; ++x) {
			res << terrain[x][y];
		}

		res << "\n";
	}

	return res.str();
}

//an object that will calculate the cost of building a road over terrain
//for use in the a_star_search algorithm.
struct road_path_calculator
{
	road_path_calculator(const terrain_map& terrain, const config& cfg)
		        : map_(terrain), cfg_(cfg),

				  //find out how windy roads should be.
				  windiness_(maximum<int>(1,atoi(cfg["road_windiness"].c_str()))) {}
	double cost(const location& loc, double so_far) const;
private:
	const terrain_map& map_;
	const config& cfg_;
	int windiness_;
	mutable std::map<char,double> cache_;
};

double road_path_calculator::cost(const location& loc, double so_far) const
{
	if(loc.x < 0 || loc.y < 0 || loc.x >= map_.size() || loc.y >= map_.front().size())
		return 100000.0;

	//we multiply the cost by a random amount depending upon how 'windy' the road should
	//be. If windiness is 1, that will mean that the cost is always genuine, and so
	//the road always takes the shortest path. If windiness is greater than 1, we sometimes
	//over-report costs for some segments, to make the road wind a little.
	const double windiness = (double(rand()%windiness_) + 1.0);

	const char c = map_[loc.x][loc.y];
	const std::map<char,double>::const_iterator itor = cache_.find(c);
	if(itor != cache_.end())
		return itor->second*windiness;

	static std::string terrain(1,'x');
	terrain[0] = c;

	const config* const child = cfg_.find_child("road_cost","terrain",terrain);
	double res = 100000.0;
	if(child != NULL) {
		res = double(atoi((*child)["cost"].c_str()));
	}

	cache_.insert(std::pair<char,double>(c,res));
	return windiness*res;
}

//a function that takes the locations of castles, villages, and the map border,
//and repositions castles to be better located.
//This function runs the castles through an attraction/repulsion system, where
// - castles repel each other (strongly)
// - villages attract castles (mildly)
// - map borders repel castles (moderately)
// the aim is to have castles nicely spread out
void place_castles(std::vector<gamemap::location>& castles, const std::set<gamemap::location>& village_locs,
				   int min_x, int min_y, int max_x, int max_y)
{
	std::vector<double> xvelocity, yvelocity;
	std::vector<gamemap::location>::iterator ci;
	for(ci = castles.begin(); ci != castles.end(); ++ci) {
		ci->x *= 1000;
		ci->y *= 1000;
		xvelocity.push_back(0.0);
		yvelocity.push_back(0.0);
	}

	std::vector<gamemap::location> villages;
	for(std::set<gamemap::location>::const_iterator v = village_locs.begin(); v != village_locs.end(); ++v) {
		villages.push_back(gamemap::location(v->x*1000,v->y*1000));
	}

	const double force_multiplier = 0.00001;

	const int niterations = 30;
	for(int i = 0; i != niterations; ++i) {

		//go through each castle, repelling
		for(ci = castles.begin(); ci != castles.end(); ++ci) {
			const size_t index = ci - castles.begin();
			for(std::vector<gamemap::location>::iterator i = castles.begin(); i != castles.end(); ++i) {
				if(i == ci)
					continue;

				if(*i == *ci)
					i->x += 1;

				const double xdist = double(abs(i->x - ci->x));
				const double ydist = double(abs(i->y - ci->y));
				const double dist = sqrt(xdist*xdist + ydist*ydist);
			
				const double force_size = 50000;
				if(dist < force_size) {
					const double power = force_multiplier * (force_size - dist) * (force_size - dist);
					const double xpower = power * xdist/(xdist+ydist) * (ci->x < i->x ? -1.0 : 1.0);
					const double ypower = power * ydist/(xdist+ydist) * (ci->y < i->y ? -1.0 : 1.0);
					xvelocity[index] += xpower;
					yvelocity[index] += ypower;
				}
			}

			//go through each village, attracting
			for(std::vector<gamemap::location>::const_iterator v = villages.begin(); v != villages.end(); ++v) {
				if(v->x < min_x*1000 || v->x > max_x*1000 || v->y < min_y*1000 || v->y > max_y*1000) {
					continue;
				}

				const double xdist = double(abs(v->x - ci->x));
				const double ydist = double(abs(v->y - ci->y));
				const double dist = sqrt(xdist*xdist + ydist*ydist);

				if(*ci == *v)
					ci->x += 1;
			
				const double force_size = 30000;
				if(dist < force_size) {
					const double power = force_multiplier * (force_size - dist) * (force_size - dist);
					const double xpower = power * xdist/(xdist+ydist) * (ci->x < v->x ? 1.0 : -1.0);
					const double ypower = power * ydist/(xdist+ydist) * (ci->y < v->y ? 1.0 : -1.0);
					xvelocity[index] += xpower;
					yvelocity[index] += ypower;
				}			
			}

			//repel from the borders
			const int border_force = 20000;
			if(ci->x < min_x*1000 + border_force) {
				const double force = (border_force - (ci->x - min_x*1000));
				const double power = force_multiplier * force * force;
				xvelocity[index] += power;
			}

			if(ci->x > max_x*1000 - border_force) {
				const double force = (border_force - (max_x*1000 - ci->x));
				const double power = force_multiplier * force * force;
				xvelocity[index] -= power;
			}

			if(ci->y < min_y*1000 + border_force) {
				const double force = (border_force - (ci->y - min_y*1000));
				const double power = force_multiplier * force * force;
				yvelocity[index] += power;
			}

			if(ci->y > max_y*1000 - border_force) {
				const double force = (border_force - (max_y*1000 - ci->y));
				const double power = force_multiplier * force * force;
				yvelocity[index] -= power;
			}

			const double friction = 0.8;
			xvelocity[index] *= friction;
			yvelocity[index] *= friction;

			ci->x += int(xvelocity[index]);
			ci->y += int(yvelocity[index]);

			if(ci->x > max_x*1000) {
				xvelocity[index] *= -1.0;
				ci->x = max_x*1000;
			}

			if(ci->x < min_x*1000) {
				xvelocity[index] *= -1.0;
				ci->x = min_x*1000;
			}

			if(ci->y > max_y*1000) {
				yvelocity[index] *= -1.0;
				ci->y = max_y*1000;
			}

			if(ci->y < min_y*1000) {
				yvelocity[index] *= -1.0;
				ci->y = min_y*1000;
			}
		}
	}

	for(ci = castles.begin(); ci != castles.end(); ++ci) {
		ci->x /= 1000;
		ci->y /= 1000;
		ci->x = minimum<int>(maximum<int>(ci->x,min_x),max_x);
		ci->y = minimum<int>(maximum<int>(ci->y,min_y),max_y);
	}
}

}

//function to generate the map.
std::string default_generate_map(size_t width, size_t height,
                                 size_t iterations, size_t hill_size,
						         size_t max_lakes, size_t nvillages, size_t nplayers,
						         const config& cfg)
{
	//find out what the 'flatland' on this map is. i.e. grassland.
	std::string flatland = cfg["default_flatland"];
	if(flatland == "") {
		flatland = "g";
	}

	const char grassland = flatland[0];

	//we want to generate a map that is 9 times bigger than the
	//actual size desired. Only the middle part of the map will be
	//used, but the rest is so that the map we end up using can
	//have a context (e.g. rivers flowing from out of the map into
	//the map, same for roads, etc etc)
	width *= 3;
	height *= 3;
	
	//generate the height of everything.
	const height_map heights = generate_height_map(width,height,iterations,hill_size);

	//the configuration file should contain a number of [height] tags:
	//[height]
	//height=n
	//terrain=x
	//[/height]
	//these should be in descending order of n. They are checked sequentially, and if
	//height is greater than n for that tile, then the tile is set to terrain type x.
	terrain_map terrain(width,std::vector<char>(height,grassland));
	size_t x, y;
	for(x = 0; x != heights.size(); ++x) {
		for(y = 0; y != heights[x].size(); ++y) {
			const int val = heights[x][y];
			const config::child_list& items = cfg.get_children("height");
			for(config::child_list::const_iterator i = items.begin(); i != items.end(); ++i) {
				const int height = atoi((**i)["height"].c_str());
				if(val >= height) {
					const std::string& c = (**i)["terrain"];
					terrain[x][y] = c.empty() ? 'g' : c[0];
					break;
				}
			}
		}
	}

	//now that we have our basic set of flatland/hills/mountains/water, we can place lakes
	//and rivers on the map. All rivers are sourced at a lake. Lakes must be in high land -
	//at least 'min_lake_height'. (Note that terrain below a certain altitude may be made
	//into bodies of water in the code above - i.e. 'sea', but these are not considered 'lakes' because
	//they are not sources of rivers).
	//
	//we attempt to place 'max_lakes' lakes. Each lake will be placed at a random location,
	//if that random location meets the minimum terrain requirements for a lake.
	//We will also attempt to source a river from each lake.
	const int nlakes = max_lakes > 0 ? (rand()%max_lakes) : 0;
	for(size_t lake = 0; lake != nlakes; ++lake) {
		for(int tries = 0; tries != 100; ++tries) {
			const int x = rand()%width;
			const int y = rand()%height;
			if(heights[x][y] > atoi(cfg["min_lake_height"].c_str())) {
				generate_river(heights,terrain,x,y,atoi(cfg["river_frequency"].c_str()));
				generate_lake(terrain,x,y,atoi(cfg["lake_size"].c_str()));
				break;
			}
		}
	}

	std::cerr << "done generating rivers...\n";

	//convert grassland terrain to other types of flat terrain.
	//we generate a 'temperature map' which uses the height generation algorithm to
	//generate the temperature levels all over the map. Then we can use a combination
	//of height and terrain to divide flatland up into more interesting types than the default
	const height_map temperature_map = generate_height_map(width,height,
	                                                       atoi(cfg["temperature_iterations"].c_str()),
														   atoi(cfg["temperature_size"].c_str()));

	std::cerr << "generated temperature map...\n";

	//iterate over every flatland tile, and determine what type of flatland it is,
	//based on our [flatland] tags.
	for(x = 0; x != width; ++x) {
		for(y = 0; y != height; ++y) {
			if(terrain[x][y] != grassland)
				continue;

			const int temperature = temperature_map[x][y];
			const int height = heights[x][y];

			//iterate over our list of [flatland] tags. Each tag specifies ranges
			//for temperature and height, and if that range is met, then the terrain
			//becomes the type specified. For instance, a tag to put snow in 
			//high areas with low temperature would look like:
			//[flatland]
			//max_temp=200
			//min_height=600
			//[/flatland]
			const config::child_list& items = cfg.get_children("flatland");
			for(config::child_list::const_iterator i = items.begin(); i != items.end(); ++i) {
				const std::string& min_temp = (**i)["min_temperature"];
				const std::string& max_temp = (**i)["max_temperature"];
				const std::string& min_height = (**i)["min_height"];
				const std::string& max_height = (**i)["max_height"];

				if(min_temp.empty() == false && atoi(min_temp.c_str()) > temperature)
					continue;

				if(max_temp.empty() == false && atoi(max_temp.c_str()) < temperature)
					continue;

				if(min_height.empty() == false && atoi(min_height.c_str()) > height)
					continue;

				if(max_height.empty() == false && atoi(max_height.c_str()) < height)
					continue;
				
				//we are in the right range, so set the terrain and go
				//on to the next tile.
				const std::string& set_to = (**i)["terrain"];
				if(set_to.empty() == false)
					terrain[x][y] = set_to[0];

				break;
			}
		}
	}

	std::cerr << "placing roads...\n";

	//place roads. We select two tiles at random locations on the borders of the map,
	//and try to build roads between them.
	const size_t nroads = atoi(cfg["roads"].c_str());
	for(size_t road = 0; road != nroads; ++road) {

		//we want the locations to be on the portion of the map we're actually going
		//to use, since roads on other parts of the map won't have any influence,
		//and doing it like this will be quicker.
		location src = point_at_side(width/3 + 2,height/3 + 2);
		location dst = point_at_side(width/3 + 2,height/3 + 2);
		src.x += width/3 - 1;
		src.y += height/3 - 1;
		dst.x += width/3 - 1;
		dst.y += height/3 - 1;

		//if the road isn't very interesting (on the same border), don't draw it
		if(src.x == dst.x || src.y == dst.y) {
			continue;
		}

		const road_path_calculator calc(terrain,cfg);
		if(calc.cost(src,0.0) >= 1000.0 || calc.cost(dst,0.0) >= 1000.0) {
			continue;
		}

		//search a path out for the road
		const paths::route rt = a_star_search(src,dst,1000.0,calc);

		//draw the road. If the search failed, rt.steps will simply be empty
		for(std::vector<location>::const_iterator step = rt.steps.begin(); step != rt.steps.end(); ++step) {
			const int x = step->x;
			const int y = step->y;

			if(x < 0 || y < 0 || x >= width || y >= height)
				continue;

			//find the configuration which tells us what to convert this tile to
			//to make it into a road.
			const std::string str(1,terrain[x][y]);
			const config* const child = cfg.find_child("road_cost","terrain",str);
			if(child != NULL) {
				//convert to bridge means that we want to convert depending
				//upon the direction the road is going.
				//typically it will be in a format like,
				//convert_to_bridge=|,/,\
				// '|' will be used if the road is going north-south
				// '/' will be used if the road is going south west-north east
				// '\' will be used if the road is going south east-north west
				//the terrain will be left unchanged otherwise (if there is no clear
				//direction)
				const std::string& convert_to_bridge = (*child)["convert_to_bridge"];
				if(convert_to_bridge.empty() == false) {
					if(step == rt.steps.begin() || step+1 == rt.steps.end())
						continue;

					const location& last = *(step-1);
					const location& next = *(step+1);

					location adj[6];
					get_adjacent_tiles(*step,adj);

					size_t direction = 0;

					//if we are going north-south
					if(last == adj[0] && next == adj[3] || last == adj[3] && next == adj[0]) {
						direction = 0;
					}
					
					//if we are going south west-north east
					else if(last == adj[1] && next == adj[4] || last == adj[4] && next == adj[1]) {
						direction = 1;
					}
					
					//if we are going south east-north west
					else if(last == adj[2] && next == adj[5] || last == adj[5] && next == adj[2]) {
						direction = 2;
					} else {
						continue;
					}

					const std::vector<std::string> items = config::split(convert_to_bridge);
					if(direction < items.size() && items[direction].empty() == false) {
						terrain[x][y] = items[direction][0];
					}

					continue;
				}

				//just a plain terrain substitution for a road
				const std::string& convert_to = (*child)["convert_to"];
				if(convert_to.empty() == false)
					terrain[x][y] = convert_to[0];
			}
		}
	}

	std::cerr << "placing villages...\n";
	std::cerr << "villages: ";
	std::cerr << nvillages;
	std::cerr << "...\n";
	//place villages. We attempt to place nvillages, all at random locations.
	//After a location is chosen, we can through [village] tags to see
	//what kind of village we should place on that type of terrain. If no
	//[village] tag is found for the terrain type (e.g. deep water) no village
	//will be placed.
	std::set<location> villages;
	for(size_t village = 0; village != nvillages; ++village) {
		const int x = rand()%width/3+width/3;
		const int y = rand()%height/3+height/3;
		const std::string str(1,terrain[x][y]);
		const config* const child = cfg.find_child("village","terrain",str);
		if(child != NULL) {
			const std::string& convert_to = (*child)["convert_to"];
			if(convert_to.empty() == false)
				terrain[x][y] = convert_to[0];
		}
	}

	std::cerr << "placing castles...\n";

	//try to find configuration for castles.
	const config* const castle_config = cfg.child("castle");
	if(castle_config == NULL) {
		std::cerr << "Could not find castle configuration\n";
		return "";
	}

	//castle configuration tag contains a 'valid_terrain' attribute which is a list of
	//terrains that the castle may appear on.
	const std::vector<std::string> castle_terrains = config::split((*castle_config)["valid_terrain"]);

	//attempt to place castles at random. Once we have placed castles, we run a sanity
	//check to make sure that the castles are well-placed. If the castles are not well-placed,
	//we try again. Definition of 'well-placed' is if no two castles are closer than
	//'min_distance' hexes from each other, and the castles appear on a terrain listed
	//in 'valid_terrain'.
	int ntries = 0;
	bool placing_bad = true;
	const size_t max_tries = 4;
	while(placing_bad && ntries++ < max_tries) {

		std::vector<location> castles;
		for(size_t player = 0; player != nplayers; ++player) {
			const int x = (rand()%(width/3)) + width/3;
			const int y = (rand()%(height/3)) + height/3;

			castles.push_back(location(x,y));
		}

		place_castles(castles,villages,width/3 + 2,height/3 + 2,(width/3)*2 - 3,(height/3)*2 - 3);

		//make sure all castles are placed on valid terrain. Check the castle tile
		//itself, and all surrounding tiles
		placing_bad = false;
		std::vector<location>::const_iterator c;
		for(c = castles.begin(); c != castles.end(); ++c) {
			const int x = c->x;
			const int y = c->y;
			const char terrains[] = {terrain[x][y],terrain[x+1][y],terrain[x-1][y],
				                     terrain[x][y+1],terrain[x+1][y+1],terrain[x-1][y+1],
									 terrain[x][y-1],terrain[x+1][y-1],terrain[x-1][y-1]};

			for(int n = 0; n != sizeof(terrains)/sizeof(*terrains); ++n) {
				const std::string t(1,terrains[n]);
				if(std::find(castle_terrains.begin(),castle_terrains.end(),t) == castle_terrains.end()) {
					placing_bad = true;
					break;
				}
			}

			if(placing_bad) {
				break;
			}
		}

		if(placing_bad && ntries < max_tries)
			continue;

		//make sure all castles are a minimum distance away from each other
		const int min_distance = atoi((*castle_config)["min_distance"].c_str());
		for(std::vector<location>::const_iterator c1 = castles.begin(); c1 != castles.end(); ++c1) {
			for(std::vector<location>::const_iterator c2 = c1+1; c2 != castles.end(); ++c2) {
				if(distance_between(*c1,*c2) < min_distance) {
					placing_bad = true;
					break;
				}
			}
		}

		if(placing_bad && ntries < max_tries)
			continue;

		//plonk down the castles.
		for(c = castles.begin(); c != castles.end(); ++c) {
			const int x = c->x;
			const int y = c->y;
			const int player = c - castles.begin();
			terrain[x][y] = '1' + player;
			terrain[x-1][y] = 'C';
			terrain[x+1][y] = 'C';
			terrain[x][y-1] = 'C';
			terrain[x][y+1] = 'C';
			terrain[x-1][y-1] = 'C';
			terrain[x-1][y+1] = 'C';
			terrain[x+1][y-1] = 'C';
			terrain[x+1][y+1] = 'C';
		}

	}

	return output_map(terrain);
}

namespace {

typedef std::map<std::string,map_generator*> generator_map;
generator_map generators;

}

map_generator::manager::manager(const config& game_config)
{
	map_generator* gen = new default_map_generator(game_config);
	assert(generators.count(gen->name()) == 0);
	generators[gen->name()] = gen;

	gen = new cave_map_generator(game_config);
	generators[gen->name()] = gen;
}

map_generator::manager::~manager()
{
	for(generator_map::iterator i = generators.begin(); i != generators.end(); ++i) {
		delete i->second;
	}

	generators.clear();
}

map_generator* get_map_generator(const std::string& name)
{
	const generator_map::iterator i = generators.find(name);
	if(i != generators.end())
		return i->second;
	else
		return NULL;
}

#ifdef TEST_MAPGEN

int main(int argc, char** argv)
{
	int x = 50, y = 50, iterations = 50, hill_size = 50, lakes=3,
	    nvillages = 25, nplayers = 2;
	if(argc >= 2) {
		x = atoi(argv[1]);
	}

	if(argc >= 3) {
		y = atoi(argv[2]);
	}

	if(argc >= 4) {
		iterations = atoi(argv[3]);
	}

	if(argc >= 5) {
		hill_size = atoi(argv[4]);
	}

	if(argc >= 6) {
		lakes = atoi(argv[5]);
	}

	if(argc >= 7) {
		nvillages = atoi(argv[6]);
	}

	if(argc >= 8) {
		nplayers = atoi(argv[7]);
	}

	srand(time(NULL));
	std::cout << generate_map(x,y,iterations,hill_size,lakes,nvillages,nplayers) << "\n";
}

#endif
