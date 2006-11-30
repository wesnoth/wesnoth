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

//NOTE due to backwards compability some items are done in a
// not so nice way. This will be corrected in a later version
// after either 1.4 or 2.0. These items are marked with
// FIXME: remove
// Also the the next definition is used for the compatible
// mode. Disabling this define should make wesnoth run in
// non compatible mode. Using defines is not the most 
// beautiful way to do things but this way both versions of
// the code can be made. The callers should be fixed after
// the undefing
#define TERRAIN_TRANSLATION_COMPATIBLE

#include <SDL_types.h> //used for Uint32 definition
#include <string>
#include <vector>
#include <map>

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
	extern const TERRAIN_NUMBER TB_STAR;
	extern const TERRAIN_NUMBER NOT;
//	extern const TERRAIN_NUMBER EOL;
	extern const TERRAIN_NUMBER TB_DOT;
	extern const TERRAIN_NUMBER COMMA;
	extern const TERRAIN_NUMBER NONE_TERRAIN;

    //exception thrown if there's an error with the terrain
	//FIXME MdW we throw nobody catches...
	struct error {
		error(const std::string& msg) : message(msg) {}
		std::string message;
	};

		
#ifdef TERRAIN_TRANSLATION_COMPATIBLE
	//the terrain format lets the terrain functions know what to expect
	// TFORMAT_LETTER the string is a terrain letter (single char)
	// TFORMAT_STRING the string is a terrain string (multiple chars)
	// TFORMAT_AUTO   uses map_format_ to determine the type
	enum { TFORMAT_LETTER = 1, TFORMAT_STRING = 2, TFORMAT_AUTO = 3 };
	
#endif
	
	struct coordinate {
		size_t x; 
		size_t y;
	};

	/** Reads a single terrain from a string
	 * FIXME: remove tformat
	 *
	 * @param letter	The string which should contain 1 letter
	 * @param tformat	The format to read
	 *
	 * @return			A single terrain letter
	 */
	TERRAIN_NUMBER read_letter(const std::string& letter, const int tformat);
	
	/** Writes a single letter to a string
	 * The writers only support the new format
	 *
	 * @param letter	The letter to convert to a string
	 *
	 * @return			A string containing the letter
	 */
	std::string write_letter(const TERRAIN_NUMBER& letter);
	
	/** Reads a list of terrain from a string, when reading the 
	 * old format the comma separator is optional the new format
	 * only reads with a separator and ignores
	 * FIXME: remove separated and tformat
	 *
	 * @param list		A string with one or more terrain letters
	 * @param separated	The old terrain format is optional separated by a comma
	 *  				the new format is always separated by a comma and
	 *  				ignores this parameter. Possible values:
	 *						0 = no
	 *						1 = yes
	 * @param format	The format to read.
	 *
	 * @returns			A vector which contains the string
	 */
//	std::vector<TERRAIN_NUMBER> read_list(const std::string& list, const int separated = 0, const int tformat);
	std::vector<TERRAIN_NUMBER> read_list(const std::string& list, const int separated, const int tformat);

	/** Writes a list of terrains to a string, only writes the new format.
	 *
	 * @param list		A vector with one or more terrain letters
	 *
	 * @returns			A string with the terrain numbers, comma separated and 
	 * 					a space behind the comma's. Not padded.
	 */
	std::string write_list(const std::vector<TERRAIN_NUMBER>& list);

	/** Reads a gamemap string into a vector
	 *
	 * @param map		A string containing the gamemap, the following rules 
	 * 					are stated for a gamemap:
	 * 					* The map is square
	 * 					* The map can be prefixed with one or more empty lines,
	 * 					  these lines are ignored
	 * 					* The map can be postfixed with one or more empty lines,
	 * 					  these lines are ignored
	 * 					* Every end of line can be followed by one or more empty
	 * 					  lines, these lines are ignored. NOTE it's deapriciated
	 * 					  to use this feature.
	 * 					* Terrain strings are separated by comma's or an end of line
	 * 					  symbol, for the last terrain string in the row. For 
	 * 					  readability it's allowed to pad strings with either spaces
	 * 					  or tab, however the tab is deapriciated.
	 * 					* A terrain string contains either a terrain or a terrain and
	 * 					  starting loction. The followin format is used
	 * 					  [S ]T
	 * 					  S = starting location a positive non-zero number
	 * 					  T = terrain string 2 - 4 characters
	 * @param starting_positions This parameter will be filled with the starting
	 * 					locations found. Starting locations can only occur once 
	 * 					if multiple definitions occur of the same position only
	 * 					the last is stored. The returned value is a map:
	 * 					* first		the starting locations
	 * 					* second	a coordinate structure where the location was found
	 *
	 * @returns			A 2D vector with the terrains found the vector data is stored
	 * 					like result[x][y] where x the column number is and y the row number.
	 */
	std::vector<std::vector<TERRAIN_NUMBER> > read_game_map(const std::string& map, 
			std::map<int, coordinate>& starting_positions);

	/** Write a gamemap in to a vector string
	 *
	 * @param map		A terrain vector, as returned from read_game_map
	 * @param starting_positions A starting positions map, as returned from read_game_map
	 *
	 * @returns			A terrain string which can be read with read_game_map.
	 * 					For readability the map is padded to groups of 7 chars
	 * 					followed by a comma and space
	 */
	std::string write_game_map(const std::vector<std::vector<TERRAIN_NUMBER> >& map, 
			 std::map<int, coordinate> starting_positions);

	//read a string and convert it to a map
	//upon error is throws an incorrect_format_exception
	std::vector<std::vector<TERRAIN_NUMBER> > read_builder_map(const std::string& map); 

	// fixme maybe we should assume 
	TERRAIN_NUMBER builder_get_number(TERRAIN_NUMBER terrain);

	/** Tests whether a certain terrain matches another terrain
	 *
	 */
	bool terrain_matches(const TERRAIN_NUMBER src, const TERRAIN_NUMBER dest);
	 
	/** Tests whether a certain terrain matches a list of terrains 
	 */
	bool terrain_matches(const TERRAIN_NUMBER src, const std::vector<TERRAIN_NUMBER>& dest);
	
/***************************************************************************************/
	
#ifdef TERRAIN_TRANSLATION_COMPATIBLE 
	// The terrain letter is an old letter and will be converted with get_letter
	void add_translation(const std::string& letter, const TERRAIN_NUMBER number);
#endif	

};
#endif
