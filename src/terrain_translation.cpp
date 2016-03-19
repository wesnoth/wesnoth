/*
   Copyright (C) 2006 - 2016 by Mark de Wever <koraq@xs4all.nl>
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
 *  @file
 *  Routines for terrain-conversion.
 */

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "global.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "terrain_translation.hpp"
#include "serialization/string_utils.hpp"
#include "util.hpp"
#include "wml_exception.hpp"


#define ERR_G LOG_STREAM(err, lg::general())
#define WRN_G LOG_STREAM(warn, lg::general())

namespace t_translation {

	size_t max_map_size() {
		return 1000; //TODO make this overridable by the user without having to rebuild
	}

/***************************************************************************************/
// forward declaration of internal functions

	// The low level convertors,
	// These function are the ones which know about the internal format.
	// All other functions are unaware of the internal format.

	/**
	 * Get the mask for a single layer.
	 *
	 * @param terrain   1 layer of a terrain, might have a wildcard.
	 *
	 * @return          Mask for that layer.
	 */
	static t_layer get_layer_mask_(t_layer terrain); //inlined

	/**
	 * Gets a mask for a terrain, this mask is used for wildcard matching.
	 *
	 * @param terrain   The terrain which might have a wildcard.
	 *
	 * @return          The mask for this terrain.
	 */
	static t_terrain get_mask_(const t_terrain& terrain);

	/**
	 * Converts a string to a layer.
	 *
	 * @param str       The terrain string to convert, but needs to be
	 *                  sanitized so no spaces and only the terrain to convert.
	 *
	 * @return          The converted layer.
	 */
	static t_layer string_to_layer_(const std::string& str);

	/**
	 * Converts a terrain string to a number.
	 * @param str               The terrain string with an optional number.
	 * @param start_position    Returns the start_position, the caller should
	 *                          set it on -1 and it's only changed it there is
	 *                          a starting position found.
	 * @param filler            If the terrain has only 1 layer then the filler
	 *                          will be used as the second layer.
	 *
	 * @return                  The terrain code found in the string if no
	 *                          valid terrain is found VOID will be returned.
	 */
	static t_terrain string_to_number_(std::string str, int& start_position, const t_layer filler);
	static t_terrain string_to_number_(const std::string& str, const t_layer filler = NO_LAYER);

	/**
	 * Converts a terrain number to a string
	 *
	 * @param terrain               The terrain number to convert.
	 * @param start_position        The starting position, if smaller than 0
	 *                              it's ignored else it's written.
	 *
	 * @return                      The converted string, if no starting
	 *                              position given it's padded to 4 chars else
	 *                              padded to 7 chars.
	 */
	static std::string number_to_string_(t_terrain terrain, const int start_position = -1);

	/**
	 * Converts a terrain string to a number for the builder.
	 * The translation rules differ from the normal conversion rules
	 *
	 * @param str   The terrain string.
	 *
	 * @return      Number for the builder map.
	 */
	static t_terrain string_to_builder_number_(std::string str);

/***************************************************************************************/

const t_terrain OFF_MAP_USER = string_to_number_("_off^_usr");

const t_terrain VOID_TERRAIN = string_to_number_("_s");
const t_terrain FOGGED = string_to_number_("_f");

const t_terrain HUMAN_CASTLE = string_to_number_("Ch");
const t_terrain HUMAN_KEEP = string_to_number_("Kh");
const t_terrain SHALLOW_WATER = string_to_number_("Ww");
const t_terrain DEEP_WATER = string_to_number_("Wo");
const t_terrain GRASS_LAND = string_to_number_("Gg");
const t_terrain FOREST = string_to_number_("Gg^Ff");
const t_terrain MOUNTAIN = string_to_number_("Mm");
const t_terrain HILL = string_to_number_("Hh");

const t_terrain CAVE_WALL = string_to_number_("Xu");
const t_terrain CAVE = string_to_number_("Uu");
const t_terrain UNDERGROUND_VILLAGE = string_to_number_("Uu^Vu");
const t_terrain DWARVEN_CASTLE = string_to_number_("Cud");
const t_terrain DWARVEN_KEEP = string_to_number_("Kud");

const t_terrain PLUS = string_to_number_("+");
const t_terrain MINUS = string_to_number_("-");
const t_terrain NOT = string_to_number_("!");
const t_terrain STAR = string_to_number_("*");
const t_terrain BASE = string_to_number_("_bas");

const t_match ALL_FORESTS("F*,*^F*");
const t_match ALL_HILLS("!,*^V*,!,H*");
const t_match ALL_MOUNTAINS("!,*^V*,!,M*"); //excluding impassable mountains
const t_match ALL_SWAMPS("!,*^V*,*^B*,!,S*"); //excluding swamp villages and bridges

/***************************************************************************************/

t_terrain::t_terrain(const std::string& b, t_layer o) :
	base(string_to_layer_(b)), overlay(o)
{}

t_terrain::t_terrain(const std::string& b, const std::string& o) :
	base(string_to_layer_(b)), overlay(string_to_layer_(o))
{}

t_match::t_match() :
	terrain(),
	mask(),
	masked_terrain(),
	has_wildcard(false),
	is_empty(true)
{}

t_match::t_match(const std::string& str, const t_layer filler) :
	terrain(t_translation::read_list(str, filler)),
	mask(),
	masked_terrain(),
	has_wildcard(t_translation::has_wildcard(terrain)),
	is_empty(terrain.empty())

{
	mask.resize(terrain.size());
	masked_terrain.resize(terrain.size());

	for(size_t i = 0; i < terrain.size(); i++) {
		mask[i] = t_translation::get_mask_(terrain[i]);
		masked_terrain[i] = mask[i] & terrain[i];
	}
}

t_match::t_match(const t_terrain& tcode):
	terrain(t_list(1, tcode)),
	mask(),
	masked_terrain(),
	has_wildcard(t_translation::has_wildcard(terrain)),
	is_empty(terrain.empty())
{
	mask.resize(terrain.size());
	masked_terrain.resize(terrain.size());

	for(size_t i = 0; i < terrain.size(); i++) {
		mask[i] = t_translation::get_mask_(terrain[i]);
		masked_terrain[i] = mask[i] & terrain[i];
	}
}

coordinate::coordinate()
	: x(0)
	, y(0)
{
}

coordinate::coordinate(const size_t x_, const size_t y_)
	: x(x_)
	, y(y_)
{
}

t_terrain read_terrain_code(const std::string& str, const t_layer filler)
{
	return string_to_number_(str, filler);
}

std::string write_terrain_code(const t_terrain& tcode)
{
	return number_to_string_(tcode);
}

t_list read_list(const std::string& str, const t_layer filler)
{
	// Handle an empty string
	t_list result;

	if(str.empty()) {
		return result;
	}

	size_t offset = 0;
	while(offset < str.length()) {

		// Get a terrain chunk
		const std::string separators = ",";
		const size_t pos_separator = str.find_first_of(separators, offset);
		const std::string terrain = str.substr(offset, pos_separator - offset);

		// Process the chunk
		const t_terrain tile = string_to_number_(terrain, filler);

		// Add the resulting terrain number
		result.push_back(tile);

		// Evaluate the separator
		if(pos_separator == std::string::npos) {
			offset =  str.length();
		} else {
			offset = pos_separator + 1;
		}
	}

	return result;
}

std::string write_list(const t_list& list)
{
	std::stringstream result;

	t_list::const_iterator itor = list.begin();
	for( ; itor != list.end(); ++itor) {
		if(itor == list.begin()) {
			result << number_to_string_(*itor);
		} else {
			result << ", " << number_to_string_(*itor);
		}
	}

	return result.str();
}

t_map read_game_map(const std::string& str,	std::map<int, coordinate>& starting_positions)
{
	t_map result;

	size_t offset = 0;
	size_t x = 0, y = 0, width = 0;

	// Skip the leading newlines
	while(offset < str.length() && utils::isnewline(str[offset])) {
		++offset;
	}

	// Did we get an empty map?
	if((offset + 1) >= str.length()) {
		return result;
	}

	while(offset < str.length()) {

		// Get a terrain chunk
		const std::string separators = ",\n\r";
		const size_t pos_separator = str.find_first_of(separators, offset);
		const std::string terrain = str.substr(offset, pos_separator - offset);

		// Process the chunk
		int starting_position = -1;
		// The gamemap never has a wildcard
		const t_terrain tile = string_to_number_(terrain, starting_position, NO_LAYER);

		// Add to the resulting starting position
		if(starting_position != -1) {
			if(starting_positions.find(starting_position) != starting_positions.end()) {
				// Redefine existion position
				WRN_G << "Starting position " << starting_position << " is redefined." << std::endl;
				starting_positions[starting_position].x = x;
				starting_positions[starting_position].y = y;
			} else {
				// Add new position
				const struct coordinate coord(x, y);
				starting_positions.insert(std::pair<int, coordinate>(starting_position, coord));
			}
		}

		// Make space for the new item
		// NOTE we increase the vector every loop for every x and y.
		// Profiling with an increase of y with 256 items didn't show
		// an significant speed increase.
		// So didn't rework the system to allocate larger vectors at once.
		if(result.size() <= x) {
			result.resize(x + 1);
		}
		if(result[x].size() <= y) {
			result[x].resize(y + 1);
		}

		// Add the resulting terrain number
		result[x][y] = tile;

		// Evaluate the separator
		if(pos_separator == std::string::npos || utils::isnewline(str[pos_separator])) {
			// the first line we set the with the other lines we check the width
			if(y == 0) {
				// x contains the offset in the map
				width = x + 1;
			} else {
				if((x + 1) != width ) {
					ERR_G << "Map not a rectangle error occurred at line offset " << y << " position offset " << x << std::endl;
					throw error("Map not a rectangle.");
				}
				if (y > max_map_size()) {
					ERR_G << "Map size exceeds limit (y > " << max_map_size() << ")" << std::endl;
					throw error("Map height limit exceeded.");
				}
			}

			// Prepare next iteration
			++y;
			x = 0;

			// Avoid in infinite loop if the last line ends without an EOL
			if(pos_separator == std::string::npos) {
				offset = str.length();

			} else {

				offset = pos_separator + 1;
				// Skip the following newlines
				while(offset < str.length() && utils::isnewline(str[offset])) {
					++offset;
				}
			}

		} else {
			++x;
			offset = pos_separator + 1;
			if (x > max_map_size()) {
				ERR_G << "Map size exceeds limit (x > " << max_map_size() << ")" << std::endl;
				throw error("Map width limit exceeded.");
			}
		}

	}

	if(x != 0 && (x + 1) != width) {
		ERR_G << "Map not a rectangle error occurred at the end" << std::endl;
		throw error("Map not a rectangle.");
	}

	return result;
}

std::string write_game_map(const t_map& map, std::map<int, coordinate> starting_positions)
{
	std::stringstream str;

	for(size_t y = 0; y < map[0].size(); ++y) {
		for(size_t x = 0; x < map.size(); ++x) {

			// If the current location is a starting position,
			// it needs to be added to the terrain.
			// After it's found it can't be found again,
			// so the location is removed from the map.
			std::map<int, coordinate>::iterator itor = starting_positions.begin();
			int starting_position = -1;
			for(; itor != starting_positions.end(); ++itor) {
				if(itor->second.x == x && itor->second.y == y) {
					starting_position = itor->first;
					starting_positions.erase(itor);
					break;
				}
			}

			// Add the separator
			if(x != 0) {
				str << ", ";
			}
			str << number_to_string_(map[x][y], starting_position);
		}

		if (y < map[0].size() -1)
			str << "\n";
	}

	return str.str();
}

bool terrain_matches(const t_terrain& src, const t_terrain& dest)
{
	return terrain_matches(src, t_list(1, dest));
}

bool terrain_matches(const t_terrain& src, const t_list& dest)
{
	// NOTE we impose some code duplication.
	// It could have been rewritten to get a match structure
	// and then call the version with the match structure.
	// IMO that's some extra overhead to this function
	// which is not required. Hence the two versions
	if(dest.empty()) {
		return false;
	}

#if 0
	std::cerr << std::hex << "src = " << src.base << "^" << src.overlay << "\t"
		<< src_mask.base << "^" << src_mask.overlay << "\t"
		<< masked_src.base << "^" << masked_src.overlay << "\t"
		<< src_has_wildcard << "\n";
#endif

	bool result = true;
	t_list::const_iterator itor = dest.begin();

	// Try to match the terrains if matched jump out of the loop.
	for(; itor != dest.end(); ++itor) {

		// Match wildcard
		if(*itor == STAR) {
			return result;
		}

		// Match inverse symbol
		if(*itor == NOT) {
			result = !result;
			continue;
		}

		// Full match
		if(src == *itor) {
			return result;
		}

		// Does the destination wildcard match
		const t_terrain dest_mask = get_mask_(*itor);
		const t_terrain masked_dest = (*itor & dest_mask);
		const bool dest_has_wildcard = has_wildcard(*itor);
#if 0
		std::cerr << std::hex << "dest= "
			<< itor->base << "^" << itor->overlay  << "\t"
			<< dest_mask.base << "^" << dest_mask.overlay << "\t"
			<< masked_dest.base << "^" << masked_dest.overlay << "\t"
			<< dest_has_wildcard << "\n";
#endif
		if(dest_has_wildcard &&
				(src.base & dest_mask.base) == masked_dest.base &&
				(src.overlay & dest_mask.overlay) == masked_dest.overlay) {
			return result;
		}

/* Test code */ /*
		if(src_has_wildcard && dest_has_wildcard && (
				(
					get_layer_mask_(itor->base) != NO_LAYER &&
					get_layer_mask_(src.overlay) != NO_LAYER &&
					(src.base & dest_mask.base) == masked_dest.base &&
					(itor->overlay & src_mask.overlay) == masked_src.overlay
				) || (
					get_layer_mask_(itor->overlay) != NO_LAYER &&
					get_layer_mask_(src.base) != NO_LAYER &&
					(src.overlay & dest_mask.overlay) == masked_dest.overlay &&
					(itor->base & src_mask.base) == masked_src.base
				))) {

			return result;
		}
*/
	}

	// No match, return the inverse of the result
	return !result;
}

// This routine is used for the terrain building,
// so it's one of the delays while loading a map.
// This routine is optimized a bit at the loss of readability.
bool terrain_matches(const t_terrain& src, const t_match& dest)
{
	if(dest.is_empty) {
		return false;
	}

	bool result = true;

	// Try to match the terrains if matched jump out of the loop.
	// We loop on the dest.terrain since the iterator is faster than operator[].
	// The i holds the value for operator[].
	// Since dest.mask and dest.masked_terrain need to be in sync,
	// they are less often looked up, so no iterator for them.
	size_t i = 0;
	t_list::const_iterator end = dest.terrain.end();
	for(t_list::const_iterator terrain_itor = dest.terrain.begin();
			terrain_itor != end;
			++i, ++terrain_itor) {

		// Match wildcard
		if(*terrain_itor == STAR) {
			return result;
		}

		// Match inverse symbol
		if(*terrain_itor == NOT) {
			result = !result;
			continue;
		}

		// Full match
		if(*terrain_itor == src) {
			return result;
		}

		// Does the destination wildcard match
		if(dest.has_wildcard &&
				(src.base & dest.mask[i].base) == dest.masked_terrain[i].base &&
				(src.overlay & dest.mask[i].overlay) == dest.masked_terrain[i].overlay) {
			return result;
		}

/* Test code */ /*
		if(src_has_wildcard && has_wildcard(*terrain_itor) && (
				(
					get_layer_mask_(terrain_itor->base) != NO_LAYER &&
					get_layer_mask_(src.overlay) != NO_LAYER &&
					(src.base & dest.mask[i].base) == dest.masked_terrain[i].base &&
					(terrain_itor->overlay & src_mask.overlay) == masked_src.overlay
				) || (
					get_layer_mask_(terrain_itor->overlay) != NO_LAYER &&
					get_layer_mask_(src.base) != NO_LAYER &&
					(src.overlay & dest.mask[i].overlay) == dest.masked_terrain[i].overlay &&
					(terrain_itor->base & src_mask.base) == masked_src.base
				))) {

			return result;
		}
*/
	}

	// No match, return the inverse of the result
	return !result;
}

bool has_wildcard(const t_terrain& tcode)
{
	if(tcode.overlay == NO_LAYER) {
		return get_layer_mask_(tcode.base) != NO_LAYER;
	} else {
		return get_layer_mask_(tcode.base) != NO_LAYER || get_layer_mask_(tcode.overlay) != NO_LAYER;
	}
}

bool has_wildcard(const t_list& list)
{
	if(list.empty()) {
		return false;
	}

	// Test all items for a wildcard
	t_list::const_iterator itor = list.begin();
	for(; itor != list.end(); ++itor) {
		if(has_wildcard(*itor)) {
			return true;
		}
	}

	// No wildcard found
	return false;
}

t_map read_builder_map(const std::string& str)
{
	size_t offset = 0;
	t_map result;

	// Skip the leading newlines
	while(offset < str.length() && utils::isnewline(str[offset])) {
		++offset;
	}

	// Did we get an empty map?
	if((offset + 1) >= str.length()) {
		return result;
	}

	size_t x = 0, y = 0;
	while(offset < str.length()) {

		// Get a terrain chunk
		const std::string separators = ",\n\r";
		const size_t pos_separator = str.find_first_of(separators, offset);

		std::string terrain = "";
		// Make sure we didn't hit an empty chunk
		// which is allowed
		if(pos_separator != offset) {
			terrain = str.substr(offset, pos_separator - offset);
		}

		// Process the chunk
		const t_terrain tile = string_to_builder_number_(terrain);

		// Make space for the new item
		if(result.size() <= y) {
			result.resize(y + 1);
		}
		if(result[y].size() <= x) {
			result[y].resize(x + 1);
		}

		// Add the resulting terrain number,
		result[y][x] = tile;

		// evaluate the separator
		if(pos_separator == std::string::npos) {
			// Probably not required to change the value,
			// but be sure the case should be handled at least.
			// I'm not sure how it is defined in the standard,
			// but here it's defined at max u32 which with +1 gives 0
			// and make a nice infinite loop.
			offset = str.length();
		} else if(utils::isnewline(str[pos_separator])) {
			// Prepare next iteration
			++y;
			x = 0;

			offset =  pos_separator + 1;
			// Skip the following newlines
			while(offset < str.length() && utils::isnewline(str[offset])) {
				++offset;
			}

		} else {
			++x;
			offset = pos_separator + 1;
		}

	}

	return result;
}

/***************************************************************************************/
// Internal

inline t_layer get_layer_mask_(t_layer terrain)
{
	// Test for the star 0x2A in every position
	// and return the appropriate mask
/*
 *	This is what the code intents to do, but in order to gain some more
 *	speed it's changed to the code below, which does the same but faster.
 *	This routine is used often in the builder and the speedup is noticeable. */
	if((terrain & 0xFF000000) == 0x2A000000) return 0x00000000;
	if((terrain & 0x00FF0000) == 0x002A0000) return 0xFF000000;
	if((terrain & 0x0000FF00) == 0x00002A00) return 0xFFFF0000;
	if((terrain & 0x000000FF) == 0x0000002A) return 0xFFFFFF00;

/*
	Uint8 *ptr = (Uint8 *) &terrain;

	if(ptr[3] == 0x2A) return 0x00000000;
	if(ptr[2] == 0x2A) return 0xFF000000;
	if(ptr[1] == 0x2A) return 0xFFFF0000;
	if(ptr[0] == 0x2A) return 0xFFFFFF00;
*/
	// no star found return the default
	return 0xFFFFFFFF;
}

static t_terrain get_mask_(const t_terrain& terrain)
{
	if(terrain.overlay == NO_LAYER) {
		return t_terrain(get_layer_mask_(terrain.base), 0xFFFFFFFF);
	} else {
		return t_terrain(get_layer_mask_(terrain.base), get_layer_mask_(terrain.overlay));
	}
}

static t_layer string_to_layer_(const std::string& str)
{
	if (str.empty())
		return NO_LAYER;

	t_layer result = 0;

	// Validate the string
	VALIDATE(str.size() <= 4, _("A terrain with a string with more "
		"than 4 characters has been found, the affected terrain is :") + str);

	// The conversion to int puts the first char
	// in the highest part of the number.
	// This will make the wildcard matching
	// later on a bit easier.
	for(size_t i = 0; i < 4; ++i) {
		const unsigned char c = (i < str.length()) ? str[i] : 0;

		// Clearing the lower area is a nop on i == 0
		// so no need for if statement
		result <<= 8;

		// Add the result
		result += c;
	}

	return result;
}

static t_terrain string_to_number_(const std::string& str, const t_layer filler) {
	int dummy = -1;
	return string_to_number_(str, dummy, filler);
}

static t_terrain string_to_number_(std::string str, int& start_position, const t_layer filler)
{
	t_terrain result;

	// Strip the spaces around us
	const std::string& whitespace = " \t";
	str.erase(0, str.find_first_not_of(whitespace));
	str.erase(str.find_last_not_of(whitespace) + 1);
	if(str.empty()) {
		return result;
	}

	// Split if we have 1 space inside
	size_t offset = str.find(' ', 0);
	if(offset != std::string::npos) {
		try {
			start_position = lexical_cast<int>(str.substr(0, offset));
		} catch(bad_lexical_cast&) {
			return VOID_TERRAIN;
		}
		str.erase(0, offset + 1);
	}

	offset = str.find('^', 0);
	if(offset !=  std::string::npos) {
		const std::string base_str(str, 0, offset);
		const std::string overlay_str(str, offset + 1, str.size());
		result = t_terrain(base_str, overlay_str);
	} else {
		result = t_terrain(str, filler);

		// Ugly hack
		if(filler == WILDCARD && (result.base == NOT.base ||
				result.base == STAR.base)) {

			result.overlay = NO_LAYER;
		}
	}

	return result;
}

static std::string number_to_string_(t_terrain terrain, const int start_position)
{
	std::string result = "";

	// Insert the start position
	if(start_position > 0) {
		result = str_cast(start_position) + " ";
	}

	/*
	 * The initialization of tcode is done to make gcc-4.7 happy. Otherwise it
	 * some uninitialized fields might be used. Its analysis are wrong, but
	 * Initialize to keep it happy.
	 */
	unsigned char tcode[9] = {0};
	// Insert the terrain tcode
	tcode[0] = ((terrain.base & 0xFF000000) >> 24);
	tcode[1] = ((terrain.base & 0x00FF0000) >> 16);
	tcode[2] = ((terrain.base & 0x0000FF00) >> 8);
	tcode[3] =  (terrain.base & 0x000000FF);

	if(terrain.overlay != NO_LAYER) {
		tcode[4] = '^'; //the layer separator
		tcode[5] = ((terrain.overlay & 0xFF000000) >> 24);
		tcode[6] = ((terrain.overlay & 0x00FF0000) >> 16);
		tcode[7] = ((terrain.overlay & 0x0000FF00) >> 8);
		tcode[8] =  (terrain.overlay & 0x000000FF);
	} else {
		// If no second layer, the second layer won't be written,
		// so no need to initialize that part of the array
		tcode[4] = 0;
	}

	for(int i = 0; i < 9; ++i) {
		if(tcode[i] != 0 && tcode[i] != 0xFF) {
			result += tcode[i];
		}
		if(i == 4 && tcode[i] == 0) {
			// no layer, stop
			break;
		}
	}

	return result;
}

static t_terrain string_to_builder_number_(std::string str)
{
	// Strip the spaces around us
	const std::string& whitespace = " \t";
	str.erase(0, str.find_first_not_of(whitespace));
	if(! str.empty()) {
		str.erase(str.find_last_not_of(whitespace) + 1);
	}

	// Empty string is allowed here, so handle it
	if(str.empty()) {
		return t_terrain();
	}

	const int number = lexical_cast_default(str, -1);
	if(number == -1) {
		// At this point we have a single char
		// which should be interpreted by the
		// map builder, so return this number
		return t_terrain(str[0] << 24, 0);
	} else {
		return t_terrain(0, number);
	}
}

} // end namespace t_translation

#if 0
// small helper rule to test the matching rules
// building rule
// make terrain_translation.o &&  g++ terrain_translation.o libwesnoth-core.a -lSDL -o terrain_translation
int main(int argc, char** argv)
{
	if(argc > 1) {

		if(std::string(argv[1]) == "match" && argc == 4) {
			t_translation::t_terrain src = t_translation::read_terrain_code(std::string(argv[2]));

			t_translation::t_list dest = t_translation::read_list(std::string(argv[3]));

			if(t_translation::terrain_matches(src, dest)) {
				std::cout << "Match\n" ;
			} else {
				std::cout << "No match\n";
			}
		}
	}
}

#endif

