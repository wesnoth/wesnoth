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

#include "global.hpp"
#include "log.hpp"
#include "terrain_translation.hpp"
#include "serialization/string_utils.hpp"
#include "wassert.hpp"

#include <iostream>

#define ERR_G  LOG_STREAM(err, general)
#define WRN_G  LOG_STREAM(warn, general)

namespace t_translation {

/***************************************************************************************/
// forward declaration of internal functions
	
#ifdef TERRAIN_TRANSLATION_COMPATIBLE 

	// the former terrain letter
	typedef char TERRAIN_LETTER;

	// This function can convert EOL's and converts them to EOL 
	// which doesn't need to be and EOL char
	// this will convert UNIX, Mac and Windows end of line types
	// this due to the fact they all have a different idea of EOL
	// Note this also eats all blank lines so the sequence "\n\n\n" will become just 1 EOL
	t_list string_to_vector_(const std::string& str, const bool convert_eol, const int separated);

	// When the terrain is loaded it sends all letter, string combinations
	// to add_translation. This way the translation table is build.
	// This way it's possible to read old maps and convert them to
	// the proper internal format
	static std::map<TERRAIN_LETTER, t_letter> lookup_table_;
	
	// This value contains the map format used, when reading the main
	// map this format should be set, don't know how we're going to do
	// it but we will. This format determines whether the WML
	// map and letter are read old or new format.
	// formats
	// 0 = unknown
	// 1 = old single letter format
	// 2 = new multi letter format
	static int map_format_ = 0;

	//old low level converters
	t_letter letter_to_number_(const TERRAIN_LETTER terrain); 

	// reads old maps
	t_map read_game_map_old_(const std::string& map,std::map<int, coordinate>& starting_positions); 

	//this is used for error messages used in string_to_number_ 
	//so we can't use this function to convert us. So we do the conversion here 
	//manually not the best solution but good enough for a tempory solution
	const t_letter OBSOLETE_KEEP = ((t_letter)'_' << 56) | ((t_letter)'K' << 48) | 0xFFFFFFFF;

#endif

	// the low level convertors, these function are the ones which
	// now about the internal format. All other functions are unaware
	// of the internal format
	
	/**
	 * This is the new convertor converts a single line
	 * and only acceptes the new terrain string format
	 *
	 * @param str	the terrain string in new format
	 *
	 * @return the list of converted terrains
	 */
	t_list string_to_vector_(const std::string& str);
	
	/**
	 * Gets a mask for a terrain, this mask is used for wildcard matching
	 *
	 * @param terrain 	the terrain which might have a wildcard
	 *
	 * @return the mask for this terrain
	 */
	t_letter get_mask_(const t_letter terrain);

	/** 
	 * if one of the 2 has a caret and we use a wildcard in the first part 
	 * we fail eg *^ != A without wildcards no problem occurs eg A^ == A
	 *
	 * This function fixes that problem but the match is expensive due to the
	 * overhead of an extra function
	 *
	 * @param src	the value to match (may also contain the wildcard)
	 * @param dest 	the value to match against
	 *
	 * @returns		the result of the match ! not allowed in this match
	 */
	bool match_ignore_layer_(const t_letter& src, const t_letter& dest);

	/**
	 * converts a terrain layer part to the partial number to convert a terrain
	 * with 2 layers it's possible to send the layer strings to this procedure
	 * and get the converted result back. The old result will always be send
	 * back with << 32.
	 * @param str				the terrain string without spaces between 2 - 4 characters
	 * @param result			result + old value << 32 is send back
	 */
	void convert_string_to_number_(const std::string& str, t_letter& result); 
		
	/**
	 * converts a terrain string to a number
	 * @param str 				the terrain string with an optional number
	 * 
	 * @param start_position	returns the start_position, the caller should set it on -1
	 * 					    	and it's only changed it there is a starting position found
	 *
	 * @return					the letter found in the string
	 */ 					
	t_letter string_to_number_(const std::string& str);
	t_letter string_to_number_(std::string str, int& start_position);

	/**
	 * converts a terrain number to a string
	 * 
	 * @param terrain				the terrain number to convert
	 * @param starting_position		the starting position, if smaller than 0 it's
	 * 								ignored else it's written
	 * @param min_size				padds the results with spaces if required untill the 
	 * 								result has a length of min_size
	 * @return						the converted string
	 */
	std::string number_to_string_(t_letter terrain, const int start_position = -1);
	std::string number_to_string_(t_letter terrain, const int start_position, const size_t min_size);

	/**
	 * converts a terrain string to a letter for the builder the translation 
	 * rules differ from the normal conversion rules
	 *
	 * @param str	the terrain string
	 *
	 * @return		number for the builder map
	 */
	t_letter string_to_builder_number_(std::string str);

/***************************************************************************************/	

const t_letter VOID_TERRAIN = string_to_number_("_s");
const t_letter FOGGED = string_to_number_("_f");

const t_letter HUMAN_CASTLE = string_to_number_("Ch");
const t_letter HUMAN_KEEP = string_to_number_("Kh");
const t_letter SHALLOW_WATER = string_to_number_("Ww");
const t_letter DEEP_WATER = string_to_number_("Wo");
const t_letter GRASS_LAND = string_to_number_("Gg");
const t_letter FOREST = string_to_number_("Ff");
const t_letter MOUNTAIN = string_to_number_("Mm");
const t_letter HILL = string_to_number_("Hh");

const t_letter CAVE_WALL = string_to_number_("Xu");
const t_letter CAVE = string_to_number_("Uu");
const t_letter UNDERGROUND_VILLAGE = string_to_number_("Vu");
const t_letter DWARVEN_CASTLE = string_to_number_("Cud");
const t_letter DWARVEN_KEEP = string_to_number_("Kud");

const t_letter PLUS = string_to_number_("+");
const t_letter MINUS = string_to_number_("-");
const t_letter NOT = string_to_number_("!");
const t_letter STAR = string_to_number_("*");

// the shift used for the builder (needs more comment)
const int BUILDER_SHIFT = 8;

// constant for no wildcard used at multiple places thus 
// defined here. The other masks are only used without
// other functions knowing their value, so they're not
// defined here
const t_letter WILDCARD_NONE = 0xFFFFFFFFFFFFFFFFi64;

/***************************************************************************************/	

t_match::t_match(const std::string& str):
	terrain(t_translation::read_list(str, -1, t_translation::T_FORMAT_STRING)) 
{
	mask.resize(terrain.size());
	masked_terrain.resize(terrain.size());
	has_wildcard = t_translation::has_wildcard(terrain);

	for(size_t i = 0; i < terrain.size(); i++) {
		mask[i] = t_translation::get_mask_(terrain[i]);
		masked_terrain[i] = mask[i] & terrain[i];
	}
}

t_match::t_match(const t_letter letter):
	terrain(t_list(1, letter)) 
{
	mask.resize(terrain.size());
	masked_terrain.resize(terrain.size());
	has_wildcard = t_translation::has_wildcard(terrain);

	for(size_t i = 0; i < terrain.size(); i++) {
		mask[i] = t_translation::get_mask_(terrain[i]);
		masked_terrain[i] = mask[i] & terrain[i];
	}
}

t_letter read_letter(const std::string& str, const int t_format)
{
#ifdef TERRAIN_TRANSLATION_COMPATIBLE 
	if(t_format == T_FORMAT_STRING ||
			(t_format == T_FORMAT_AUTO && map_format_ == 2)) {
		return string_to_number_(str);
		
	} else if(t_format == T_FORMAT_LETTER ||
			(t_format == T_FORMAT_AUTO && map_format_ == 1)) {
		return letter_to_number_(str[0]);
		
	} else {
		throw error("Invalid case in read_letter");
	}
#else
		return string_to_number_(str);
#endif
}

std::string write_letter(const t_letter letter)
{
	return number_to_string_(letter);
}

t_list read_list(const std::string& str, const int separated, const int t_format)
{
#ifdef TERRAIN_TRANSLATION_COMPATIBLE 
	if(t_format == T_FORMAT_STRING ||
			(t_format == T_FORMAT_AUTO && map_format_ == 2)) {
		return string_to_vector_(str);
		
	} else if(t_format == T_FORMAT_LETTER ||
			(t_format == T_FORMAT_AUTO && map_format_ == 1)) {
		return string_to_vector_(str, false, separated);
		
	} else {
		throw error("Invalid case in read_list");
	}
#else
		return string_to_vector_(str);
#endif
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
#ifdef TERRAIN_TRANSLATION_COMPATIBLE 

	// process the data, polls for the format. NOTE we test for a comma
	// so an empty map or a map with 1 letter is doomed to be the old
	// format. Shouldn't hurt
	if(str.find(',') == std::string::npos) {
		//old format
		ERR_G << "Using the single letter map format is deprecated\n";
		map_format_ = 1;
		return read_game_map_old_(str, starting_positions);
	}

	// at this point we're the new format, we also dissapear in the future so
	// inside the ifdef
	map_format_ = 2;
#endif
	
	size_t offset = 0;
	size_t x = 0, y = 0, width = 0;
	t_map result;

	// skip the leading newlines
	while(offset < str.length() && utils::isnewline(str[offset])) {
		++offset;
	}
	
	// did we get an empty map?
	if((offset + 1) >= str.length()) {
		return result;
	}
		
	while(offset < str.length()) {

		// get a terrain chunk
		const std::string separators = ",\n\r";
		const size_t pos_separator = str.find_first_of(separators, offset);
		const std::string terrain = str.substr(offset, pos_separator - offset);

		// process the chunk
		int starting_position = -1; 
		const t_letter tile = string_to_number_(terrain, starting_position);

		// add to the resulting starting position
		if(starting_position != -1) {
			if(starting_positions.find(starting_position) != starting_positions.end()) {
				// redefine existion position
				WRN_G << "Starting position " << starting_position << " is redefined.\n";
				starting_positions[starting_position].x = x;
				starting_positions[starting_position].y = y;
			} else {
				// add new position
				const struct coordinate coord = {x, y};
				starting_positions.insert(std::pair<int, coordinate>(starting_position, coord));
			}
		} 

		// make space for the new item 
		// NOTE we increase the vector every loop for every x and y profiling
		// with an increase of y with 256 items didn't show an significant speed
		// increase. So didn't rework the system to allocate larger vectors at 
		// once. 
		if(result.size() <= x) {
			result.resize(x + 1);
		}
		if(result[x].size() <= y) {
			result[x].resize(y + 1);
		}
		
		// add the resulting terrain number
		result[x][y] = tile;

		//evaluate the separator
		if(utils::isnewline(str[pos_separator]) || pos_separator == std::string::npos) {
			// the first line we set the with the other lines we check the width
			if(y == 0) { 
				// x contains the offset in the map
				width = x + 1;
			} else {
				if((x + 1) != width ) {
					ERR_G << "Map not a rectangle error occured at line offset " << y << " position offset " << x << "\n";
					throw error("Map not a rectangle.");
				}
			}

			// prepare next itertration 
			++y;
			x = 0;
			
			// avoid in infinite loop if the last line ends without an EOL
			if(pos_separator == std::string::npos) {
				offset = str.length();

			} else {
			
				offset = pos_separator + 1;
				//skip the following newlines 
				while(offset < str.length() && utils::isnewline(str[offset])) {
					++offset;
				}
			} 

		} else {
			++x;
			offset = pos_separator + 1;
		}

	}

	if(x != 0 && (x + 1) != width) {
		ERR_G << "Map not a rectangle error occured at the end\n"; 
		throw error("Map not a rectangle.");
	}
	
	return result;
}

std::string write_game_map(const t_map& map, std::map<int, coordinate> starting_positions)
{
	std::stringstream str;

	for(size_t y = 0; y < map[0].size(); ++y) {
		for(size_t x = 0; x < map.size(); ++x) {

			// If the current location is a starting position it needs to
			// be added to the terrain. After it's found it can't be found
			// again so the location is removed from the map.
			std::map<int, coordinate>::iterator itor = starting_positions.begin();
			int starting_position = -1;
			for(; itor != starting_positions.end(); ++itor) {
				if(itor->second.x == x && itor->second.y == y) {
					starting_position = itor->first;
					starting_positions.erase(itor);
					break;
				}
			}

			// add the separator
			if(x != 0) {
				str << ", ";
			}
			str << number_to_string_(map[x][y], starting_position, 12);
		}

		str << "\n";
	}

	return str.str();
}

bool terrain_matches(const t_letter src, const t_letter dest)
{
	return terrain_matches(src, t_list(1, dest));
}

bool terrain_matches(const t_letter src, const t_list& dest)
{
	//NOTE we impose some code duplication we could have been rewritten
	//to get a match structure and then call the version with the match
	//structure. IMO that's some extra overhead to this function which is
	//not required. Hence the two versions
	if(dest.empty()) {
		return false;
	}

	const t_letter star = STAR;
	const t_letter inverse = NOT;
	
	const t_letter src_mask = get_mask_(src);
	const t_letter masked_src = (src & src_mask);
	const bool src_has_wildcard = has_wildcard(src);

#if 0	
	std::cerr << std::hex << "src = " << src << "\t" 
		<< src_mask << "\t" << masked_src << "\t" << src_has_wildcard << "\n";
#endif
	
	bool result = true;
	t_list::const_iterator itor = dest.begin();

	// try to match the terrains if matched jump out of the loop.
	for(; itor != dest.end(); ++itor) {

		// match wildcard 
		if(*itor == star) {
			return result;
		}

		// match inverse symbol
		if(*itor == inverse) {
			result = !result;
			continue;
		}

		// full match 
		if(src == *itor) {
			return result;
		}
		
		// does the source wildcard match
		if(src_has_wildcard && (*itor & src_mask) == masked_src) {
			return result;
		}
		
		// does the destination wildcard match
		const t_letter dest_mask = get_mask_(*itor);
		const t_letter masked_dest = (*itor & dest_mask);
#if 0		
		std::cerr << std::hex << "dest= " 
			<< *itor << "\t" << dest_mask << "\t" << masked_dest << "\n";
#endif
		if((src & dest_mask) == masked_dest) {
			return result;
		}

		// if one of the 2 has a caret and we use a wildcard in the first part
		// we fail eg *^ != A without wildcards no problem occurs eg A^ == A
		if(src_has_wildcard || has_wildcard(*itor)) {
			if(match_ignore_layer_(src, *itor)) {
				return result;
			}
		}
	}

	// no match, return the inverse of the result
	return !result;
}

bool terrain_matches(const t_letter src, const t_match& dest)
{
	if(dest.terrain.empty()) {
		return false;
	}

	const t_letter star = STAR;
	const t_letter inverse = NOT;

	const t_letter src_mask = get_mask_(src);
	const t_letter masked_src = (src & src_mask);
	const bool src_has_wildcard = has_wildcard(src);

	bool result = true;

	// try to match the terrains if matched jump out of the loop.
	for(size_t i = 0; i < dest.terrain.size(); ++i) {

		// match wildcard 
		if(dest.terrain[i] == star) {
			return result;
		}

		// match inverse symbol
		if(dest.terrain[i] == inverse) {
			result = !result;
			continue;
		}

		// full match 
		if(dest.terrain[i] == src) {
			return result;
		}
		
		// does the source wildcard match
		if(src_has_wildcard && (dest.terrain[i] & src_mask) == masked_src) {
			return result;
		}
		
		// does the destination wildcard match
		if(dest.has_wildcard && (src & dest.mask[i]) == dest.masked_terrain[i]) {
			return result;
		}
		
		// if one of the 2 has a caret and we use a wildcard in the first part
		// we fail eg *^ != A without wildcards no problem occurs eg A^ == A
		if(src_has_wildcard || has_wildcard(dest.terrain[i])) {
			if(match_ignore_layer_(src, dest.terrain[i])) {
				return result;
			}
		}
	}

	// no match, return the inverse of the result
	return !result;
}

bool has_wildcard(const t_letter letter) 
{
	return has_wildcard(t_list(1, letter));
}

bool has_wildcard(const t_list& list)
{
	if(list.empty()) {
		return false;
	}
	
	// test all items for a wildcard 
	t_list::const_iterator itor = list.begin();
	for(; itor != list.end(); ++itor) {
		if(get_mask_(*itor) != WILDCARD_NONE) {
			return true;
		}
	}

	// no wildcard found
	return false;
}

t_map read_builder_map(const std::string& str)
{
	size_t offset = 0;
	t_map result;

	// skip the leading newlines
	while(offset < str.length() && utils::isnewline(str[offset])) {
		++offset;
	}
	
	// did we get an empty map?
	if((offset + 1) >= str.length()) {
		return result;
	}
		
	size_t x = 0, y = 0;
	while(offset < str.length()) {

		// get a terrain chunk
		const std::string separators = ",\n\r";
		const size_t pos_separator = str.find_first_of(separators, offset);

		std::string terrain = "";
		// make sure we didn't hit and empty chunk
		// which is allowed
		if(pos_separator != offset) {
			terrain = str.substr(offset, pos_separator - offset);
		}

		// process the chunk
		const t_letter tile = string_to_builder_number_(terrain);

		// make space for the new item
		if(result.size() <= y) {
			result.resize(y + 1);
		}
		if(result[y].size() <= x) {
			result[y].resize(x + 1);
		}
		
		// add the resulting terrain number,
		result[y][x] = tile;

		//evaluate the separator
		if(pos_separator == std::string::npos) {
			// probably not required to change the value but be sure
			// the case should be handled at least. I'm not sure how
			// it defined in the standard but here it's defined at 
			// max u32 which with +1 gives 0 and make a nice infinite 
			// loop.
			offset = str.length();	
		} else if(utils::isnewline(str[pos_separator])) {
			// prepare next itertration 
			++y;
			x = 0;
			
			offset =  pos_separator + 1;
			//skip the following newlines 
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

t_letter cast_to_builder_number(const t_letter terrain) 
{
		return (terrain >> BUILDER_SHIFT); 
}

#ifdef TERRAIN_TRANSLATION_COMPATIBLE 
void add_translation(const std::string& str, const t_letter number)
{
	lookup_table_[str[0]] = number;
}

std::string get_old_letter(const t_letter number) 
{
	std::map<TERRAIN_LETTER, t_letter>::iterator itor = lookup_table_.begin();

	for(; itor !=  lookup_table_.end(); ++itor) {
		if(itor->second == number) return std::string(1, itor->first);
	}

	return "";
	
}
#endif

/***************************************************************************************/	
//internal

#ifdef TERRAIN_TRANSLATION_COMPATIBLE 

t_list string_to_vector_(const std::string& str, const bool convert_eol, const int separated)
{
	// only used here so define here
	const t_letter EOL = 7;
	bool last_eol = false;
	t_list result;

	std::string::const_iterator itor = str.begin();
	for( ; itor != str.end(); ++itor) {
		
		if(separated == 1 && *itor == ',') {
			//ignore the character
			last_eol = false;
			
		} else if ((convert_eol) && (*itor == '\n' || *itor == '\r')) {
			// end of line marker found
			if(last_eol == false){
				// last wasn't eol then add us
				result.push_back(EOL);
			}
			last_eol = true;
			
		} else {
			// normal just add
			last_eol = false;
			result.push_back(letter_to_number_(*itor));
		}
	}

	return result;
}

t_letter letter_to_number_(const TERRAIN_LETTER terrain)
{
	std::map<TERRAIN_LETTER, t_letter>::const_iterator itor = lookup_table_.find(terrain);

	if(itor == lookup_table_.end()) {
		ERR_G << "No translation found for old terrain letter " << terrain << "\n";
		throw error("No translation found for old terrain letter");
	}

	return itor->second;
}

t_map read_game_map_old_(const std::string& str, std::map<int, coordinate>& starting_positions) 
{
	size_t offset = 0;
	size_t x = 0, y = 0, width = 0;
	t_map result;
	
	// skip the leading newlines
	while(offset < str.length() && utils::isnewline(str[offset])) {
		++offset;
	}

	// did we get an empty map?
	if((offset + 1) >= str.length()) {
		WRN_G << "Empty map found\n";
		return result;
	}
	
	while(offset < str.length()) {

		// handle newlines
		if(utils::isnewline(str[offset])) {
			// the first line we set the with the other lines we check the width
			if(y == 0 ) { 
				// note x has been increased to the new value at the end of 
				// the loop so width = x and not x + 1
				width = x;
			} else {
				if(x != width ) {
					ERR_G << "Map not a rectangle error occured at line " << y << " position " << x << "\n"; 
					throw error("Map not a rectangle.");
				}
			}

			// prepare next itertration 
			++y;
			x = 0;
			++offset;
			
			//skip the following newlines 
			while(offset < str.length() && utils::isnewline(str[offset])) {
				++offset;
			}
			
			// stop if at end of file
			if((offset + 1) >= str.length()) {
				break;
			}
		}
		
		// get a terrain chunk
		TERRAIN_LETTER terrain = str[offset];

		// process the chunk
		int starting_position = lexical_cast_default<int>(std::string(1, terrain), -1);
		
		// add to the resulting starting position
		if(starting_position != -1) {
			if(starting_positions.find(starting_position) != starting_positions.end()) {
				// redefine existion position
				WRN_G << "Starting position " << starting_position <<" redefined.\n";
				starting_positions[starting_position].x = x;
				starting_positions[starting_position].y = y;
			} else {
				// add new position
				struct coordinate coord = {x, y};
				starting_positions.insert(std::pair<int, coordinate>(starting_position, coord));
			}
			//the letter of the keep hardcoded, since this code is 
			//scheduled for removal the hardcoded letter is oke
			terrain = 'K';
		} 
		
		// make space for the new item
		if(result.size() <= x) {
			result.resize(x + 1);
		}
		if(result[x].size() <= y) {
			result[x].resize(y + 1);
		}
		
		// add the resulting terrain number,
		result[x][y] = letter_to_number_(terrain);

		//set next value
		++x; 
		++offset;
	}

	if(x != 0 && x != width) {
		ERR_G << "Map not a rectangle error occured at the end\n"; 
		throw error("Map not a rectangle.");
	}

	return result;
}

#endif

t_list string_to_vector_(const std::string& str)
{
	// handle an empty string
	t_list result;

	if(str.empty()) {
		return result;
	}
		
	size_t offset = 0;
	while(offset < str.length()) {

		// get a terrain chunk
		const std::string separators = ",";
		const size_t pos_separator = str.find_first_of(separators, offset);
		const std::string terrain = str.substr(offset, pos_separator - offset);

		// process the chunk
		const t_letter tile = string_to_number_(terrain);

		// add the resulting terrain number
		result.push_back(tile);

		//evaluate the separator
		if(pos_separator == std::string::npos) {
			offset =  str.length();
		} else {
			offset = pos_separator + 1;
		}
	}

	return result;
}

t_letter get_mask_(const t_letter terrain)
{
	// test for the star 0x2A in every postion and return the
	// appropriate mask
	uint32_t hi = terrain >> 32;
	uint32_t lo = terrain;

	if     ((hi & 0xFF000000) == 0x2A000000) hi = 0x00000000;
	else if((hi & 0x00FF0000) == 0x002A0000) hi = 0xFF000000;
	else if((hi & 0x0000FF00) == 0x00002A00) hi = 0xFFFF0000;
	else if((hi & 0x000000FF) == 0x0000002A) hi = 0xFFFFFF00;
	else                                     hi = 0xFFFFFFFF;
		
	if     (lo == 0xFFFFFFFF)                lo = 0x00000000; // no caret so match all layers
	else if((lo & 0xFF000000) == 0x2A000000) lo = 0x00000000;
	else if((lo & 0x00FF0000) == 0x002A0000) lo = 0xFF000000;
	else if((lo & 0x0000FF00) == 0x00002A00) lo = 0xFFFF0000;
	else if((lo & 0x000000FF) == 0x0000002A) lo = 0xFFFFFF00;
	else                                     lo = 0xFFFFFFFF;

	t_letter result = hi;
	return ((result << 32) | lo);
}

bool match_ignore_layer_(const t_letter& src, const t_letter& dest)
{
	// if one of the 2 has a caret and we use a wildcard in the first part
	// we fail eg *^ != A without wildcards no problem occurs eg A^ == A
	// our caller makes sure one of the 2 has a wildcard
	
	// mask out the part before the caret
	t_letter src_lo = src & 0x00000000FFFFFFFFi64;
	t_letter dest_lo = dest & 0x00000000FFFFFFFFi64;

	// if the part after the caret is 0 in one and all F in the 
	// other we have our special case
	if((src_lo == 0x00000000 && dest_lo == 0xFFFFFFFF) ||
			(dest_lo == 0x00000000 && src_lo == 0xFFFFFFFF)) {

		// force both to have a caret
		src_lo = src & 0xFFFFFFFF00000000i64;
		dest_lo = dest & 0xFFFFFFFF00000000i64;

		// match again
		return terrain_matches(src_lo, dest_lo);
	}

	return false;
}

t_letter string_to_number_(const std::string& str) {
	int dummy = -1;
	return string_to_number_(str, dummy);
}

void convert_string_to_number_(const std::string& str, t_letter& result) {

	//validate the string
	wassert(str.size() <= 4);

	//the conversion to int puts the first char in the 
	//highest part of the number, this will make the 
	//wildcard matching later on a bit easier.
	for(size_t i = 0; i < 4; ++i) {
		const unsigned char c = (i < str.length()) ? str[i] : 0;
		
		// clear the lower area also needed on i == 0 due
		// to a possible partial result
		result <<= 8; 
		
		// add the result
		result += c;
	}
}

t_letter string_to_number_(std::string str, int& start_position)
{
	t_letter result = NONE_TERRAIN;

	//strip the spaces around us
	const std::string& whitespace = " \t";
	str.erase(0, str.find_first_not_of(whitespace));
	str.erase(str.find_last_not_of(whitespace) + 1);
	if(str.empty()) {
		return result;
	}

	// split if we have 1 space inside
	size_t offset = str.find(' ', 0);
	if(offset != std::string::npos) {
		start_position = lexical_cast<int>(str.substr(0, offset));
		str.erase(0, offset + 1);
	}

	// split on the caret
	offset = str.find('^', 0);
	if(offset !=  std::string::npos) {
		convert_string_to_number_(std::string(str, 0, offset).c_str(), result);
		const std::string layer(str.c_str(), offset + 1);
		if(layer == "" ) {
			// set layer to empty
			result <<= 32;
		} else {
			convert_string_to_number_(layer, result);
		}
	} else {
		convert_string_to_number_(str, result);
		// set the layer to not existing
		result <<= 32;
		result |= 0xFFFFFFFF; 
	}

#ifndef TERRAIN_TRANSLATION_COMPATIBLE 
	if(result == OBSOLETE_KEEP) {
		ERR_G << "Using _K for a keep is deprecated, support will be removed shortly\n";
		result = HUMAN_KEEP;
	}
#endif
	
	return result;
}

std::string number_to_string_(t_letter terrain, const int start_position)
{
	std::string result = "";

	//insert the start position
	if(start_position > 0) {
		result = str_cast(start_position) + " ";
	}
	
	//insert the terrain letter
	unsigned char letter[8];
	letter[0] = ((terrain & 0xFF00000000000000i64) >> 56);
	letter[1] = ((terrain & 0x00FF000000000000i64) >> 48);
	letter[2] = ((terrain & 0x0000FF0000000000i64) >> 40);
	letter[3] = ((terrain & 0x000000FF00000000i64) >> 32);
	letter[4] = ((terrain & 0x00000000FF000000i64) >> 24);
	letter[5] = ((terrain & 0x0000000000FF0000i64) >> 16);
	letter[6] = ((terrain & 0x000000000000FF00i64) >> 8);
	letter[7] =  (terrain & 0x00000000000000FFi64);
		
	for(int i = 0; i < 8; ++i) {
		if(letter[i] != 0 && letter[i] != 0xFF) {
			if(i == 5) {
				result += '^';
			}
			result += letter[i];
		} else {
			// no letter, but can be more due to the 2 parts
			continue;
		}
	}

	return result;
}

std::string number_to_string_(t_letter terrain, const int start_position, const size_t min_size)
{
	std::string result = number_to_string_(terrain, start_position);
	if(result.size() < min_size) {
		result.resize(min_size, ' ');
	}

	return result;
}

t_letter string_to_builder_number_(std::string str)
{
	//strip the spaces around us
	const std::string& whitespace = " \t";
	str.erase(0, str.find_first_not_of(whitespace));
	if(! str.empty()) {
		str.erase(str.find_last_not_of(whitespace) + 1);
	}

	// empty string is allowed here so handle it
	if(str.empty()) {
		return NONE_TERRAIN;
	}
	
	const int number = lexical_cast_default(str, -1);
	if(number == -1) {
		// at this point we have a single char
		// which should be interpreted by the map
		// builder, so return this number
		return str[0];
	} else {
		wassert(number >= 0 && number < (2 << 24)); 
		return (number << BUILDER_SHIFT);
	}
}
	
} //namespace

#if 0
// small helper rule to test the matching rules
// building rule
// make terrain_translation.o &&  g++ terrain_translation.o libwesnoth-core.a -lSDL -o terrain_translation
int main(int argc, char** argv)
{
	if(argc > 1) {
	
		if(std::string(argv[1]) == "match" && argc == 4) {
			t_translation::t_letter src = 
				t_translation::read_letter(std::string(argv[2]), t_translation::T_FORMAT_STRING);
			
			t_translation::t_list dest = 
				t_translation::read_list(std::string(argv[3]), -1, t_translation::T_FORMAT_STRING);

			if(t_translation::terrain_matches(src, dest)) {
				std::cout << "Match\n" ;
			} else {
				std::cout << "No match\n";
			}
		}
	}
}

#endif

