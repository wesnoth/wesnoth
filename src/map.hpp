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

#include "config.hpp"
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

	//some types of terrain which must be known, and can't just be loaded
	//in dynamically because they're special. It's asserted that there will
	//be corresponding entries for these types of terrain in the terrain
	//configuration file.
	enum { VOID_TERRAIN = ' ', CASTLE = 'C', TOWER = 't', FOREST = 'f' };

	//the name of the terrain is the terrain itself, the underlying terrain
	//is the name of the terrain for game-logic purposes. I.e. if the terrain
	//is simply an alias, the underlying terrain name is the name of the
	//terrain that it's aliased to
	const std::string& terrain_name(TERRAIN terrain) const;
	const std::string& underlying_terrain_name(TERRAIN terrain) const;
	TERRAIN underlying_terrain(TERRAIN terrain) const;

	//exception thrown if the map file is not in the correct format.
	struct incorrect_format_exception {
		incorrect_format_exception(const char* msg) : msg_(msg) {}
		const char* const msg_;
	};

	//structure which represents a location on the map.
	struct location {
		//any valid direction which can be moved in in our hexagonal world.
		enum DIRECTION { NORTH, NORTH_EAST, SOUTH_EAST, SOUTH,
		                 SOUTH_WEST, NORTH_WEST };

		location() : x(-1), y(-1) {}
		location(int x, int y) : x(x), y(y) {}
		location(config& cfg);

		bool valid() const { return x >= 0 && y >= 0; }

		int x, y;

		bool operator<(const location& a) const;
		bool operator==(const location& a) const;
		bool operator!=(const location& a) const;

		location get_direction(DIRECTION d) const;

		static location null_location;
	};

	//loads a map, with the given terrain configuration.
	//data should be a series of lines, with each character representing
	//one hex on the map. Starting locations are represented by numbers,
	//and will be of type keep.
	gamemap(config& terrain_cfg,
	        const std::string& data); //throw(incorrect_format_exception)

	std::string write() const;

	//dimensions of the map.
	int x() const;
	int y() const;

	//allows lookup of terrain at a particular location.
	const std::vector<TERRAIN>& operator[](int index) const;

	//functions to manipulate starting positions of the different sides.
	const location& starting_position(int side) const;
	int num_starting_positions() const;
	bool is_starting_position(const location& loc) const;

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
	const std::vector<location>& towers() const { return towers_; }

	//function to get the corresponding terrain_type information object
	//for a given type of terrain
	const terrain_type& get_terrain_info(TERRAIN terrain) const;

	//gets the list of which terrain types should display on top of
	//other terrain types. Has no effect on gameplay, only display.
	const std::vector<TERRAIN>& get_terrain_precedence() const;

	//clobbers over the terrain at location 'loc', with the given terrain
	void set_terrain(const location& loc, TERRAIN ter);
private:
	std::vector<TERRAIN> terrainPrecedence_;
	std::map<TERRAIN,terrain_type> letterToTerrain_;
	std::map<std::string,terrain_type> terrain_;

	std::vector<std::vector<TERRAIN> > tiles_;
	std::vector<location> towers_;
	location startingPositions_[10];
};

#endif
