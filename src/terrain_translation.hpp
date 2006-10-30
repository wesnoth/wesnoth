/* $Id: boilerplate-header.cpp 8092 2005-09-02 16:10:12Z ott $ */
/*
   Copyright (C) 2006 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/


//FIXME MdW is Makefile.am good modified
//do I need to change the $Id line??
//


#ifndef TERRAIN_TRANSLATION_H_INCLUDED
#define TERRAIN_TRANSLATION_H_INCLUDED

class config;

#include <string>
#include <vector>

#include "variable.hpp"


/* The base terrain translation class
 *
 * There should be 2 sub classes
 *   terrain_translation map 
 *   - adds load_terrain_map
 *   - _if_ we use the XLAT version it has to translation table
 *   terrain_translation_preferences
 *   - should override load_terrain_vector since it loads a , separated list
 *
 * It seems the builder can use the base class
 * The terrain_type will also use the base class
 */
class terrain_translation
{
public:

	//The new definition of terrain
	typedef char TERRAIN_LETTER; //maybe should be private
	typedef unsigned long TERRAIN_NUMBER;
	
	//some types of terrain which must be known, and can't just be loaded
	//in dynamically because they're special. It's asserted that there will
	//be corresponding entries for these types of terrain in the terrain
	//configuration file.
	static const TERRAIN_NUMBER VOID_TERRAIN;
	static const TERRAIN_NUMBER FOGGED;
	static const TERRAIN_NUMBER KEEP;
	// used in mapgen
	static const TERRAIN_NUMBER CASTLE;
	static const TERRAIN_NUMBER SHALLOW_WATER;
	static const TERRAIN_NUMBER DEEP_WATER;
	static const TERRAIN_NUMBER GRASS_LAND;
	static const TERRAIN_NUMBER FOREST;
	static const TERRAIN_NUMBER MOUNTAIN;
	static const TERRAIN_NUMBER HILL;
	// used in cavegen
	static const TERRAIN_NUMBER CAVE_WALL;
	static const TERRAIN_NUMBER CAVE;
	static const TERRAIN_NUMBER UNDERGROUND_VILLAGE;
	static const TERRAIN_NUMBER DWARVEN_CASTLE;
	// used for special purposes
	static const TERRAIN_NUMBER PLUS;
	static const TERRAIN_NUMBER MINUS;
	static const TERRAIN_NUMBER STAR;
	static const TERRAIN_NUMBER NOT;
	static const TERRAIN_NUMBER EOL; //end of line will be translated to a bell character
	static const TERRAIN_NUMBER DOT;
	static const TERRAIN_NUMBER COMMA;
	static const TERRAIN_NUMBER NONE_TERRAIN;

	terrain_translation();
	~terrain_translation();
//	load_translation(std::string translation);
	
	//converts a string to a TERRAIN_NUMBER it expects the input to be a string of 1 item
	//to convert
	TERRAIN_NUMBER get_letter(const std::string& letter) const;
	//gets the internal number for a start location
	TERRAIN_NUMBER get_start_location(int player) const;
	
	
	//converts a string to a vector of TERRAIN_NUMBER it expects the input to be a continues string of items
	//to convert
	//separated, is the list separated by a,
	// 0 = no
	// 1 = yes
	// 2 = auto, might be required for the future, make this value the default
	//     This conversion is not implanted, since it's unkown whether it's required
	std::vector<TERRAIN_NUMBER> get_list(const std::string& list, const int separated=0) const;
	
	//converts a string to a vector of TERRAIN_NUMBER it expects the input to be a map and  converts it accordingly
	std::vector<TERRAIN_NUMBER> get_map(const std::string& map) const;

	//used in unit animation
	std::vector<std::vector<TERRAIN_NUMBER> > get_splitted_list(const std::string& list) const;
	
	//expects a vector of TERRAIN_NUMBER and converts it to s number -1 upon failure
	int list_to_int(const std::vector<TERRAIN_NUMBER> number)const;

	//converts an map to a string
	std::string set_map(const std::vector<TERRAIN_NUMBER>& map) const;
	
	//converts a letter to a string
	std::string set_letter(const TERRAIN_NUMBER& letter) const;
	
	//converts a list to a string
	std::string set_list(const std::vector<TERRAIN_NUMBER>& list, const int separated=0) const;
	
	int letter_to_start_location(const TERRAIN_NUMBER number) const;

private:
	
	// This function can convert EOL's and converts them to EOL 
	// which doesn't need to be and EOL char
	// this will convert UNIX, Mac and Windows end of line types
	// this due to the fact they all have a different idea of EOL
	// Note this also eats all blank lines so the sequence "\n\n\n" will become just 1 EOL
	std::vector<TERRAIN_NUMBER> string_to_vector_(const std::string& map_data, const bool convert_eol, const int separated) const;
	
	std::string vector_to_string_(const std::vector<TERRAIN_NUMBER>& map_data, const int separated) const;

	TERRAIN_LETTER number_to_letter_(const TERRAIN_NUMBER terrain) const;
	TERRAIN_NUMBER letter_to_number_(const TERRAIN_LETTER terrain) const; 


};

#endif
