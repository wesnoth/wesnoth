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

class gamemap
{
public:

	typedef char TERRAIN;

	enum { VOID_TERRAIN = ' ', CASTLE = 'C', TOWER = 't', FOREST = 'f' };

	//the name of the terrain is the terrain itself, the underlying terrain
	//is the name of the terrain for game-logic purposes. I.e. if the terrain
	//is simply an alias, the underlying terrain name is the name of the
	//terrain that it's aliased to
	const std::string& terrain_name(TERRAIN terrain) const;
	const std::string& underlying_terrain_name(TERRAIN terrain) const;
	TERRAIN underlying_terrain(TERRAIN terrain) const;

	struct incorrect_format_exception {
		incorrect_format_exception(const char* msg) : msg_(msg) {}
		const char* const msg_;
	};

	struct location {
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

	gamemap(config& cfg,
	        const std::string& data); //throw(incorrect_format_exception)

	std::string write() const;

	int x() const;
	int y() const;

	const std::vector<TERRAIN>& operator[](int index) const;

	const location& starting_position(int n) const;
	int num_starting_positions() const;
	bool is_starting_position(const location& loc) const;

	bool on_board(const location& loc) const
	{
		return loc.valid() && loc.x < x() && loc.y < y();
	}

	bool empty() const
	{
		return x() == 0 || y() == 0;
	}

	const std::vector<location>& towers() const { return towers_; }

	const terrain_type& get_terrain_info(TERRAIN terrain) const;

	const std::vector<TERRAIN>& get_terrain_precedence() const;

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
