/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef MAP_H_INCLUDED
#define MAP_H_INCLUDED

class config;

#include "terrain.hpp"

#include <map>
#include <string>
#include <vector>

//class which encapsulates the map of the game. Although the game is hexagonal,
//the map is stored as a grid. Each type of terrain is represented by a letter.
class gamemap
{
public:

	typedef char TERRAIN;
	enum { NB_TERRAIN = 256, ADD_INDEX_TERRAIN = 128 };

	//some types of terrain which must be known, and can't just be loaded
	//in dynamically because they're special. It's asserted that there will
	//be corresponding entries for these types of terrain in the terrain
	//configuration file.
	enum { FOGGED = '~', VOID_TERRAIN = ' ', KEEP = 'K', CASTLE = 'C', VILLAGE = 't', FOREST = 'f' };

	//the name of the terrain is the terrain itself, the underlying terrain
	//is the name of the terrain for game-logic purposes. I.e. if the terrain
	//is simply an alias, the underlying terrain name is the name of the
	//terrain that it's aliased to
	const std::string& underlying_terrain(TERRAIN terrain) const;

	//exception thrown if the map file is not in the correct format.
	struct incorrect_format_exception {
		incorrect_format_exception(const char* msg) : msg_(msg) {}
		const char* const msg_;
	};

	//structure which represents a location on the map.
	struct location {
		//any valid direction which can be moved in in our hexagonal world.
		enum DIRECTION { NORTH, NORTH_EAST, SOUTH_EAST, SOUTH,
		                 SOUTH_WEST, NORTH_WEST, NDIRECTIONS };

		static DIRECTION parse_direction(const std::string& str);

		location() : x(-1), y(-1) {}
		location(int x, int y) : x(x), y(y) {}
		explicit location(const config& cfg);

		void write(config& cfg) const;

		inline bool valid() const { return x >= 0 && y >= 0; }
		
		inline bool valid(const int parWidth, const int parHeight) const
		{ 
			return ((x >= 0) && (y >= 0) && (x < parWidth) && (y < parHeight));
		}

		int x, y;

		// Inlining those for performance reasons
		bool operator<(const location& a) const { return x < a.x || x == a.x && y < a.y; }
		bool operator==(const location& a) const { return x == a.x && y == a.y; }
		bool operator!=(const location& a) const { return !operator==(a); }

		// Adds an absolute location to a "delta" location
		location operator-() const;
		location operator+(const location &a) const;
		location &operator+=(const location &a);
		location operator-(const location &a) const;
		location &operator-=(const location &a);

		location get_direction(DIRECTION d) const;

		static location null_location;
	};

	const std::string& underlying_terrain(const location& loc) const
	{ return underlying_terrain(get_terrain(loc)); }

	bool is_village(TERRAIN terrain) const;
	bool gives_healing(TERRAIN terrain) const;
	bool is_castle(TERRAIN terrain) const;
	bool is_keep(TERRAIN terrain) const;

	bool is_village(const location& loc) const;
	bool gives_healing(const location& loc) const;
	bool is_castle(const location& loc) const;
	bool is_keep(const location& loc) const;

	//loads a map, with the given terrain configuration.
	//data should be a series of lines, with each character representing
	//one hex on the map. Starting locations are represented by numbers,
	//and will be of type keep.
	gamemap(const config& terrain_cfg, const std::string& data); //throw(incorrect_format_exception)
	void read(const std::string& data);

	std::string write() const;

	//overlays another map onto this one at the given position.
	void overlay(const gamemap& m, const config& rules, int x=0, int y=0);

	//dimensions of the map.
	int x() const;
	int y() const;

	//allows lookup of terrain at a particular location.
	const std::vector<TERRAIN>& operator[](int index) const;

	//looks up terrain at a particular location. Hexes off the map
	//may be looked up, and their 'emulated' terrain will also be returned.
	//this allows proper drawing of the edges of the map
	TERRAIN get_terrain(const location& loc) const;

	//functions to manipulate starting positions of the different sides.
	const location& starting_position(int side) const;
	int is_starting_position(const location& loc) const;
	int num_valid_starting_positions() const;

	void set_starting_position(int side, const location& loc);

	//function which, given a location, will tell if that location is
	//on the map. Should be called before indexing using []
	bool on_board(const location& loc) const
	{
		return loc.valid() && loc.x < x() && loc.y < y();
	}

	//function to tell if the map is of 0 size.
	bool empty() const
	{
		return x() == 0 || y() == 0;
	}

	//function to return a list of the locations of villages on the map
	const std::vector<location>& villages() const { return villages_; }

	//function to get the corresponding terrain_type information object
	//for a given type of terrain
	const terrain_type& get_terrain_info(TERRAIN terrain) const;

	//shortcut to get_terrain_info(get_terrain(loc))
	const terrain_type& get_terrain_info(const location &loc) const;
	
	//gets the list of terrains
	const std::vector<TERRAIN>& get_terrain_list() const;

	//clobbers over the terrain at location 'loc', with the given terrain
	void set_terrain(const location& loc, TERRAIN ter);

	//function which returns a list of the frequencies of different terrain
	//types on the map, with terrain nearer the center getting weighted higher
	const std::map<TERRAIN,size_t>& get_weighted_terrain_frequencies() const;
	//remove the cached border terrain at loc. Needed by the editor
	//to make tiles at the border update correctly when drawing
	//other tiles.
	void remove_from_border_cache(const location &loc);
private:
	int num_starting_positions() const;

	std::vector<TERRAIN> terrainList_;
	std::map<TERRAIN,terrain_type> letterToTerrain_;
	std::map<std::string,terrain_type> terrain_;

	std::vector<std::vector<TERRAIN> > tiles_;
	std::vector<location> villages_;

	enum { STARTING_POSITIONS = 10 };
	location startingPositions_[STARTING_POSITIONS];

	mutable std::map<location,TERRAIN> borderCache_;
	mutable std::map<TERRAIN,size_t> terrainFrequencyCache_;
};

//a utility function which parses ranges of locations
//into a vector of locations
std::vector<gamemap::location> parse_location_range(const std::string& xvals, const std::string& yvals);


#endif
