/*
   Copyright (C) 2006 - 2015 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#ifndef TERRAIN_TRANSLATION_H_INCLUDED
#define TERRAIN_TRANSLATION_H_INCLUDED

#include <SDL_types.h> //used for Uint32 definition
#include <vector>
#include <map>

#include "exceptions.hpp"

namespace t_translation {

    /**
     * Return the maximum allowed map size (in either dimension),
     * the maximum map area is, therefore, this value squared.
     */
    size_t max_map_size();

	typedef Uint32 t_layer;
	const t_layer WILDCARD = 0x2A000000;
	const t_layer NO_LAYER = 0xFFFFFFFF;

	// The definitions for a terrain
	/**
	 * A terrain string which is converted to a terrain is a string with 1 or 2 layers
	 * the layers are separated by a caret and each group consists of 2 to 4 characters
	 * if no second layer is defined it is stored as 0xFFFFFFFF, if the second layer
	 * is empty (needed for matching) the layer has the value 0.
	 */
	struct t_terrain {
		t_terrain(const std::string& b, const std::string& o);
		t_terrain(const std::string& b, t_layer o = NO_LAYER);
		t_terrain(t_layer b, t_layer o) : base(b), overlay(o) {}
		t_terrain() : base(0), overlay(NO_LAYER) {}

		t_layer base;
		t_layer overlay;
	};
	const t_terrain NONE_TERRAIN = t_terrain();

	inline bool operator<(const t_terrain& a, const t_terrain& b)
		{ return a.base < b.base ||  (a.base == b.base && a.overlay < b.overlay); }

	inline bool operator==(const t_terrain& a, const t_terrain& b)
		{ return a.base == b.base && a.overlay == b.overlay; }

	inline bool operator!=(const t_terrain& a, const t_terrain& b)
		{ return a.base != b.base || a.overlay != b.overlay; }

	inline t_terrain operator&(const t_terrain& a, const t_terrain& b)
		{ return t_terrain(a.base & b.base, a.overlay & b.overlay); }

	inline t_terrain operator|(const t_terrain& a, const t_terrain& b)
		{ return t_terrain(a.base | b.base, a.overlay | b.overlay); }

	// operator<< is defined later

	typedef std::vector<t_terrain> t_list;
	typedef std::vector<std::vector<t_terrain> > t_map;

	/**
	 * This structure can be used for matching terrain strings.
	 * It optimized for strings that need to be matched often,
	 * and caches the wildcard info required for matching.
	 */
	struct t_match{
		t_match();
		t_match(const std::string& str, const t_layer filler = NO_LAYER);
		t_match(const t_terrain& tcode);

		t_list terrain;
		t_list mask;
		t_list masked_terrain;
		bool has_wildcard;
		bool is_empty;
	};

	/**  Contains an x and y coordinate used for starting positions in maps. */
	struct coordinate {
		coordinate();
		coordinate(const size_t x_, const size_t y_);
		size_t x;
		size_t y;
	};

    // Exception thrown if there's an error with the terrain.
	// Note: atm most thrown result in a crash, but I like
	// an uncatched exception better than an assert.
	struct error : public game::error {
		error(const std::string& message) : game::error(message) {}
	};

	// Some types of terrain which must be known, and can't just
	// be loaded in dynamically because they're special.
	// It's asserted that there will be corresponding entries for
	// these types of terrain in the terrain configuration file.
	extern const t_terrain VOID_TERRAIN;
	extern const t_terrain FOGGED;

	// On the map the user can use this type to make odd shaped maps look good.
	extern const t_terrain OFF_MAP_USER;

	extern const t_terrain HUMAN_CASTLE;
	extern const t_terrain HUMAN_KEEP;
	extern const t_terrain SHALLOW_WATER;
	extern const t_terrain DEEP_WATER;
	extern const t_terrain GRASS_LAND;
	extern const t_terrain FOREST;
	extern const t_terrain MOUNTAIN;
	extern const t_terrain HILL;

	extern const t_terrain CAVE_WALL;
	extern const t_terrain CAVE;
	extern const t_terrain UNDERGROUND_VILLAGE;
	extern const t_terrain DWARVEN_CASTLE;
	extern const t_terrain DWARVEN_KEEP;

	extern const t_terrain PLUS;	// +
	extern const t_terrain MINUS;	// -
	extern const t_terrain NOT;		// !
	extern const t_terrain STAR;	// *
	extern const t_terrain BASE;	// references the base terrain in movement/defense aliases

	extern const t_match ALL_FORESTS;
	extern const t_match ALL_HILLS;
	extern const t_match ALL_MOUNTAINS; //excluding impassable mountains
	extern const t_match ALL_SWAMPS;

	/**
	 * Reads a single terrain from a string.
	 *
	 * @param str		The string which should contain 1 terrain code;
                                        the new format of a terrain code
	 *				is 2 to 4 characters in the set
	 *@verbatim
	 *				[a-Z][A-Z]/|\_
	 *@endverbatim
	 *				The underscore is intended for internal use.
	 *				Other letters and characters are not validated but
	 *				users of these letters can get nasty surprises.
	 *				The * is used as wildcard in some cases.
	 *				The terrain code can be two groups separated by a caret,
	 *				the first group is the base terrain,
	 *				the second the overlay terrain.
	 *
	 * @param filler	if there's no layer this value will be used as the second layer
	 *
	 * @return			A single terrain code
	 */
	t_terrain read_terrain_code(const std::string& str, const t_layer filler = NO_LAYER);

	/**
	 * Writes a single terrain code to a string.
	 * The writers only support the new format.
	 *
	 * @param tcode	The terrain code to convert to a string
	 *
	 * @return		A string containing the terrain code
	 */
	std::string write_terrain_code(const t_terrain& tcode);
	inline std::ostream &operator<<(std::ostream &s, const t_terrain &a)
		{ s << write_terrain_code(a); return s; }

	/**
	 * Reads a list of terrains from a string, when reading the
	 *
	 * @param str		A string with one or more terrain codes (see read_terrain_code)
	 * @param filler	If there's no layer, this value will be used as the second layer
	 *
	 * @returns		A vector which contains the terrain codes found in the string
	 */
	 t_list read_list(const std::string& str, const t_layer filler = NO_LAYER);

	/**
	 * Writes a list of terrains to a string, only writes the new format.
	 *
	 * @param list		A vector with one or more terrain codes
	 *
	 * @returns		A string with the terrain codes, comma separated
	 *			and a space behind the commas. Not padded.
	 */
	std::string write_list(const t_list& list);

	/**
	 * Reads a gamemap string into a 2D vector
	 *
	 * @param str		A string containing the gamemap, the following rules
	 *					are stated for a gamemap:
	 *					* The map is square
	 *					* The map can be prefixed with one or more empty lines,
	 *					  these lines are ignored
	 *					* The map can be postfixed with one or more empty lines,
	 *					  these lines are ignored
	 *					* Every end of line can be followed by one or more empty
	 *					  lines, these lines are ignored.
	 *        @deprecated NOTE it's deprecated to use this feature.
	 *					* Terrain strings are separated by comma's or an end of line
	 *					  symbol, for the last terrain string in the row. For
	 *					  readability it's allowed to pad strings with either spaces
	 *					  or tab, however the tab is deprecated.
	 *					* A terrain string contains either a terrain or a terrain and
	 *					  starting location. The following format is used
	 *					  [S ]T
	 *					  S = starting location a positive non-zero number
	 *					  T = terrain code (see read_terrain_code)
	 * @param starting_positions This parameter will be filled with the starting
	 *					locations found. Starting locations can only occur once
	 *					if multiple definitions occur of the same position only
	 *					the last is stored. The returned value is a map:
	 *					* first		the starting locations
	 *					* second	a coordinate structure where the location was found
	 *
	 * @returns			A 2D vector with the terrains found the vector data is stored
	 *					like result[x][y] where x the column number is and y the row number.
	 */
	t_map read_game_map(const std::string& str, std::map<int, coordinate>& starting_positions);

	/**
	 * Write a gamemap in to a vector string.
	 *
	 * @param map				 A terrain vector, as returned from read_game_map
	 * @param starting_positions A starting positions map, as returned from read_game_map
	 *
	 * @returns			A terrain string which can be read with read_game_map.
	 *					For readability the map is padded to groups of 12 chars,
	 *					followed by a comma and space.
	 */
	std::string write_game_map(const t_map& map, std::map<int, coordinate> starting_positions = std::map<int, coordinate>());

	/**
	 * Tests whether a specific terrain matches a list of expressions.
	 * The list can use wildcard matching with *.
	 * It also has an inversion function.
	 * When a ! is found the result of the match is inverted.
	 * The matching stops at the first match (regardless of the ! found)
	 * the data is match from start to end.
	 *
	 * Example:
	 * Ww, W*		does match and returns true
	 * Ww, {!, W*}	does match and returns false (due to the !)
	 * WW, Ww		doesn't match and return false
	 *
	 * Multilayer rules:
	 * If a terrain has multiple layers, each layer will be matched separately,
	 * returning true only if both layers match.
	 *
	 * Example:
	 * A*^*     matches Abcd but also Abcd^Abcd
	 * A*^      matches Abcd but *not* Abcd^Abcd
	 * A*^Abcd  does not match Abcd but matches Abcd^Abcd
	 *
	 * Note: If an expression doesn't specify a second layer (i.e. it contains
	 * no caret) the second layer will be filled in with a default value
	 * (See read_terrain_code and read_list).
	 *
	 * In the terrain building code, the second layer will default to the wildcard,
	 * so both A* and A*^* will match Abcd^Abcd
	 *
	 * @param src	the value to match (may not contain wildcards)
	 * @param dest	the list of expressions to match against
	 *
	 * @returns		the result of the match (depending on the !'s)
	 */
	bool terrain_matches(const t_terrain& src, const t_list& dest);

	/**
	 * Tests whether a specific terrain matches an expression,
	 * for matching rules see above.
	 *
	 * @param src	the value to match (may not contain wildcards)
	 * @param dest	the expression to match against
	 *
	 * @returns		the result of the match (depending on the !'s)
	 */
	bool terrain_matches(const t_terrain& src, const t_terrain& dest);

	/**
	 * Tests whether a certain terrain matches a list of expressions, for matching
	 * rules see above. The matching requires some bit mask which impose a
	 * certain overhead. This version uses a cache to cache the masks so if
	 * a list needs to be matched often this version is preferred.
	 *
	 * @param src	the value to match (may not contain wildcards)
	 * @param dest	the cached list of expressions to match against
	 *
	 * @returns		the result of the match (depending on the !'s)
	 */
	bool terrain_matches(const t_terrain& src, const t_match& dest);

	/**
	 * Tests whether a terrain code contains a wildcard
	 *
	 *  @param tcode	the terrain code to test for a wildcard
	 *
	 *  @returns		true if wildcard found,	else false
	 */
	bool has_wildcard(const t_terrain& tcode);

	/**
	 * Tests whether a terrain-code list contains at least
	 * one item with a wildcard
	 *
	 *  @param list		the list to test for a wildcard
	 *
	 *  @returns		true if a wildcard found, else false
	 */
	bool has_wildcard(const t_list& list);

	// These terrain letters are in the builder format,
	// and not usable in other parts of the engine
	const t_layer TB_STAR = '*' << 24;	// It can be assumed this is the equivalent of STAR
	const t_layer TB_DOT  = '.' << 24;

	/**
	 * Reads a builder map.
	 * A builder map differs a great deal from a normal map,
	 * hence the different functions.
	 *
	 * @param str		The map data, a terrain letter is either a * or a . or a number as
	 *					anchor. The star or dot are stored in the base part of the terrain
	 *					and the anchor in the overlay part. If more letters are allowed as
	 *					special case they will be stored in the base part.
	 *					Anchor 0 is no anchor.
	 *
	 * @returns			A 2D vector with the data found the vector data is stored
	 *					like result[y][x] where x the column number is and y the row number.
	 */
	t_map read_builder_map(const std::string& str);

} // end namespace t_translation

#endif
