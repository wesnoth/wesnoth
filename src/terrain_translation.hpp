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
// not so nice way. This will be corrected in version 1.3.3
// These items are marked with "FIXME: remove"
// Also the the next definition is used for the compatible
// mode. Disabling this define should make wesnoth run in
// non compatible mode. Using defines is not the most 
// beautiful way to do things but this way both versions of
// the code can be made. The callers should be fixed after
// the undefing. The define is more a hint for me than
// to really use -- Mordante
#define TERRAIN_TRANSLATION_COMPATIBLE

#include <string>
#include <vector>
#include <map>

// MS VC at least up to version 8 doesn't include stdint.h which is 
// defined C99. Use this to fix that ommision, if other  compilers
// also miss this header add them to the ifdef
#ifdef _MSC_VER
#include "stdint/pstdint.h"
#define output_terrain(Num) ((Uint32)((Num)>>32)) << ((Uint32)(Num))
//comment this operator<< part out for now, since it doesn't work
/*ostream& operator << (ostream& os, const uint64_t num);*/
#else
#include <stdint.h>
#define output_terrain(Num) (Num)
#endif

namespace t_translation {

	//The definitions for a terrain
	typedef uint64_t t_letter;
	typedef std::vector<t_letter> t_list;
	typedef std::vector<std::vector<t_letter> > t_map;

	/**
	 * This structure can be used for matching terrain strings
	 * it optimized for strings that need to be matched often
	 * and caches the wildcard info required for matching
	 */
	struct t_match{
		t_match(){};
		t_match(const std::string& str);
		t_match(const t_letter letter);
		~t_match(){};

		t_list terrain;	
		t_list mask;
		t_list masked_terrain;
		bool has_wildcard;
        bool is_empty;
	};
	
	/** 
	 * Contains an x and c coordinate used for starting positions
	 * in maps
	 */
	struct coordinate {
		size_t x; 
		size_t y;
	};

    //exception thrown if there's an error with the terrain
	//Note atm most thrown result in a crash but I like an 
	//uncatched exception better than an assert
	struct error {
		error(const std::string& msg) : message(msg) {}
		std::string message;
	};

	//some types of terrain which must be known, and can't just be loaded
	//in dynamically because they're special. It's asserted that there will
	//be corresponding entries for these types of terrain in the terrain
	//configuration file.
	extern const t_letter VOID_TERRAIN;
	extern const t_letter FOGGED;

	extern const t_letter HUMAN_CASTLE;
	extern const t_letter HUMAN_KEEP;
	extern const t_letter SHALLOW_WATER;
	extern const t_letter DEEP_WATER;
	extern const t_letter GRASS_LAND;
	extern const t_letter FOREST;
	extern const t_letter MOUNTAIN;
	extern const t_letter HILL;

	extern const t_letter CAVE_WALL;
	extern const t_letter CAVE;
	extern const t_letter UNDERGROUND_VILLAGE;
	extern const t_letter DWARVEN_CASTLE;
	extern const t_letter DWARVEN_KEEP;

	extern const t_letter PLUS; 	// +
	extern const t_letter MINUS; 	// -
	extern const t_letter NOT;		// !
	extern const t_letter STAR; 	// *
	const t_letter NONE_TERRAIN = 0xFFFFFFFF;
		
#ifdef TERRAIN_TRANSLATION_COMPATIBLE
	//the terrain format lets the terrain functions know what to expect
	// this is part of the backwards compability layer.
	// T_FORMAT_LETTER the string is a terrain letter (single char)
	// T_FORMAT_STRING the string is a terrain string (multiple chars)
	// T_FORMAT_AUTO   uses map_format_ to determine the type
	enum { T_FORMAT_LETTER = 1, T_FORMAT_STRING = 2, T_FORMAT_AUTO = 3 };
	
	extern const t_letter OBSOLETE_KEEP;
#endif
	
	/** 
	 * Reads a single terrain from a string
	 * FIXME: remove t_format
	 *
	 * @param str		The string which should contain 1 letter the new format.
	 * 					This 'letter' can be one or 2 groups of letters separated
	 * 					by a caret.	A group letter is 2 to 4 characters in the set
	 * 					[a-Z][A-Z]/|\_ The underscore is intended for internal
	 * 					use. Other letters and characters are not validated but
	 * 					users of these letters can get nasty surprices. The * 
	 * 					is used as wildcard in some cases.
	 * 
	 * @param t_format	The format to read
	 *
	 * @return			A single terrain letter
	 */
	t_letter read_letter(const std::string& str, const int t_format);
	
	/** 
	 * Writes a single letter to a string.
	 * The writers only support the new format
	 *
	 * @param letter	The letter to convert to a string
	 *
	 * @return			A string containing the letter
	 */
	std::string write_letter(const t_letter letter);
	
	/** 
	 * Reads a list of terrain from a string, when reading the 
	 * old format the comma separator is optional the new format
	 * only reads with a separator and ignores
	 * FIXME: remove separated and t_format
	 *
	 * @param str		A string with one or more terrain letters (see read_letter)
	 * @param separated	The old terrain format is optional separated by a comma
	 *  				the new format is always separated by a comma and
	 *  				ignores this parameter. Possible values:
	 *						0 = no
	 *						1 = yes
	 * @param t_format	The format to read.
	 *
	 * @returns			A vector which contains the letters found in the string
	 */
	 t_list read_list(const std::string& str, const int separated, const int t_format);

	/** 
	 * Writes a list of terrains to a string, only writes the new format.
	 *
	 * @param list		A vector with one or more terrain letters
	 *
	 * @returns			A string with the terrain numbers, comma separated and 
	 * 					a space behind the comma's. Not padded.
	 */
	std::string write_list(const t_list& list);

	/** 
	 * Reads a gamemap string into a 2D vector
	 *
	 * @param str		A string containing the gamemap, the following rules 
	 * 					are stated for a gamemap:
	 * 					* The map is square
	 * 					* The map can be prefixed with one or more empty lines,
	 * 					  these lines are ignored
	 * 					* The map can be postfixed with one or more empty lines,
	 * 					  these lines are ignored
	 * 					* Every end of line can be followed by one or more empty
	 * 					  lines, these lines are ignored. NOTE it's deprecated
	 * 					  to use this feature.
	 * 					* Terrain strings are separated by comma's or an end of line
	 * 					  symbol, for the last terrain string in the row. For 
	 * 					  readability it's allowed to pad strings with either spaces
	 * 					  or tab, however the tab is deprecated.
	 * 					* A terrain string contains either a terrain or a terrain and
	 * 					  starting loction. The following format is used
	 * 					  [S ]T
	 * 					  S = starting location a positive non-zero number
	 * 					  T = terrain letter (see read_letter)
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
	t_map read_game_map(const std::string& str, std::map<int, coordinate>& starting_positions);

	/** 
	 * Write a gamemap in to a vector string
	 *
	 * @param map		A terrain vector, as returned from read_game_map
	 * @param starting_positions A starting positions map, as returned from read_game_map
	 *
	 * @returns			A terrain string which can be read with read_game_map.
	 * 					For readability the map is padded to groups of 12 chars
	 * 					followed by a comma and space
	 */
	std::string write_game_map(const t_map& map, std::map<int, coordinate> starting_positions = std::map<int, coordinate>());

	/** 
	 * Tests whether a certain terrain matches a list of terrains the terrains can 
	 * use wildcard matching with *. It also has an inversion function. When a ! 
	 * is found the result of the match is inverted. The matching stops at the 
	 * first match (regardless of the ! found) the data is match from start to end.
	 *
	 * Example: 
	 * W*, Ww 		does match and returns true
	 * W*, {!, Ww}	does match and returns false (due to the !)
	 * Ww, WW		doesn't match and return false
	 * 
	 * Layer based matching:
	 * A*			matches Abcd but also Abcd^Abcd
	 * A*^*			matches Abcd but also Abcd^Abcd
	 * A*^			matches Abcd but *not* Abcd^Abcd
	 * A*^Abcd		does not match Abcd but matches Abcd^Abcd
	 *
	 * ^*			invalid since it will only match terrains with a second layer
	 *
	 * @param src	the value to match (may also contain the wildcard)
	 * @param dest	the list of values to match against
	 *
	 * @returns		the result of the match (depending on the !'s)
	 */
	bool terrain_matches(const t_letter src, const t_list& dest);

	/** 
	 * Tests whether a certain terrain matches another terrain, for matching 
	 * rules see above.
	 *
	 * @param src	the value to match (may also contain the wildcard)
	 * @param dest 	the value to match against
	 *
	 * @returns		the result of the match (depending on the !'s)
	 */
	bool terrain_matches(const t_letter src, const t_letter dest);
	
	/** 
	 * Tests whether a certain terrain matches another terrain, for matching 
	 * rules see above. The matching requires some bit mask which impose a
	 * certain overhead. This version uses a cache to cache the masks so if
	 * a list needs to be matched often this version is preferred.
	 *
	 * @param src	the value to match (may also contain the wildcard)
	 * @param dest 	the value to match against
	 *
	 * @returns		the result of the match (depending on the !'s)
	 */
	bool terrain_matches(const t_letter src, const t_match& dest);

	/** 
	 * Tests wither a terrain contains a wildcard
	 *
	 *  @param		the letter to test for a wildcard
	 *
	 *  @returns	true if wildcard found else false
	 */
	bool has_wildcard(const t_letter letter);

	/** 
	 * Tests wither a terrain list contains at least
	 * one item with a wildcard
	 *
	 *  @param		the list to test for a wildcard
	 *
	 *  @returns	true if a wildcard found else false
	 */
	bool has_wildcard(const t_list& list);

	// these terrain letters are in the builder format, and 
	// not usable in other parts of the engine
	const t_letter TB_STAR = '*'; //it can be assumed this is the equivalent of STAR
	const t_letter TB_DOT = '.';
	
	/** 
	 * Reads a builder map, a builder map differs much from a normal map hence
	 * the different functions
	 *
	 * @param str		The map data, the exact rules are not stated yet since still 
	 * 					in development
	 *
	 * @returns			A 2D vector with the data found the vector data is stored
	 * 					like result[y][x] where x the column number is and y the row number.
	 */
	t_map read_builder_map(const std::string& str); 

	/** 
	 * Translates a terrain number to the map number, since
	 * there are some differences between the two
	 */
	t_letter cast_to_builder_number(const t_letter terrain); 
	
/***************************************************************************************/
	
#ifdef TERRAIN_TRANSLATION_COMPATIBLE 
	// The terrain letter is an old letter and will be converted with get_letter
	void add_translation(const std::string& letter, const t_letter number);

	std::string get_old_letter(const t_letter number);
#endif	

};
#endif
