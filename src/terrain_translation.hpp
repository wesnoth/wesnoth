/* $Id$ */
/*
   Copyright (C) 2006 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef TERRAIN_TRANSLATION_H_INCLUDED
#define TERRAIN_TRANSLATION_H_INCLUDED

#include <SDL_types.h> //used for Uint32 definition
#include <string>
#include <vector>

#include "variable.hpp"

namespace terrain_translation {

	//The new definition of terrain
	typedef char TERRAIN_LETTER;
	typedef Uint32 TERRAIN_NUMBER;

	//some types of terrain which must be known, and can't just be loaded
	//in dynamically because they're special. It's asserted that there will
	//be corresponding entries for these types of terrain in the terrain
	//configuration file.
	extern const TERRAIN_NUMBER VOID_TERRAIN;
	extern const TERRAIN_NUMBER FOGGED;
	extern const TERRAIN_NUMBER KEEP;

	extern const TERRAIN_NUMBER CASTLE;
	extern const TERRAIN_NUMBER SHALLOW_WATER;
	extern const TERRAIN_NUMBER DEEP_WATER;
	extern const TERRAIN_NUMBER GRASS_LAND;
	extern const TERRAIN_NUMBER FOREST;
	extern const TERRAIN_NUMBER MOUNTAIN;
	extern const TERRAIN_NUMBER HILL;

	extern const TERRAIN_NUMBER CAVE_WALL;
	extern const TERRAIN_NUMBER CAVE;
	extern const TERRAIN_NUMBER UNDERGROUND_VILLAGE;
	extern const TERRAIN_NUMBER DWARVEN_CASTLE;

	extern const TERRAIN_NUMBER PLUS;
	extern const TERRAIN_NUMBER MINUS;
	extern const TERRAIN_NUMBER STAR;
	extern const TERRAIN_NUMBER NOT;
	extern const TERRAIN_NUMBER EOL;
	extern const TERRAIN_NUMBER DOT;
	extern const TERRAIN_NUMBER COMMA;
	extern const TERRAIN_NUMBER NONE_TERRAIN;

	//converts a string to a TERRAIN_NUMBER it expects the input to be a string of 1 item
	//to convert
	TERRAIN_NUMBER read_letter(const std::string& letter);
	//converts a letter to a string
	std::string write_letter(const TERRAIN_NUMBER& letter);
	
	//converts a string to a vector of TERRAIN_NUMBER it expects the input to be a continues string of items
	//to convert
	//separated, is the list separated by a,
	// 0 = no
	// 1 = yes
	// 2 = auto, might be required for the future, make this value the default
	//     This conversion is not implanted, since it's unkown whether it's required
	std::vector<TERRAIN_NUMBER> read_list(const std::string& list, const int separated=0);

	//converts a list to a string
	std::string write_list(const std::vector<TERRAIN_NUMBER>& list, const int separated=0);

	//converts a string to a vector of TERRAIN_NUMBER it expects the input to be a map and  converts it accordingly
	std::vector<TERRAIN_NUMBER> read_map(const std::string& map);

	//converts an map to a string
	std::string write_map(const std::vector<TERRAIN_NUMBER>& map);

/***************************************************************************************/
// These will probably become obsolete
	//gets the internal number for a start location
	//FIXME MdW this function needs to be modified later on
	TERRAIN_NUMBER get_start_location(int player);
	
	int letter_to_start_location(const TERRAIN_NUMBER number);

	//expects a vector of TERRAIN_NUMBER and converts it to s number -1 upon failure
	int list_to_int(const std::vector<TERRAIN_NUMBER> number);

};

#endif
