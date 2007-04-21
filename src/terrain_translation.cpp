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
	static t_list string_to_vector_(const std::string& str, const bool convert_eol, const int separated);

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
	static t_letter letter_to_number_(const TERRAIN_LETTER terrain); 

	// reads old maps
	static t_map read_game_map_old_(const std::string& map,std::map<int, coordinate>& starting_positions); 

	//this is used for error messages used in string_to_number_ 
	//so we can't use this function to convert us. So we do the conversion here 
	//manually not the best solution but good enough for a tempory solution
	const t_letter OBSOLETE_KEEP('_' << 24 | 'K' << 16, 0xFFFFFFFF);

#endif

	// the low level convertors, these function are the ones which
	// now about the internal format. All other functions are unaware
	// of the internal format
	
	/**
	 * This is the new convertor converts a single line
	 * and only acceptes the new terrain string format
	 *
	 * @param str		the terrain string in new format
	 * @param filler	if the terrain has only 1 layer then the filler will be used
	 * 					as the second layer
	 *
	 * @return 			the list of converted terrains
	 */
	static t_list string_to_vector_(const std::string& str, const t_layer filler);
	
	/**
	 * Get the mask for a single layer
	 *
	 * @param terrain 	1 layer of a terrain, might have a wildcard
	 *
	 * @return			mask for that layer
	 */
	static t_layer get_layer_mask_(t_layer terrain); //inlined
	
	/**
	 * Gets a mask for a terrain, this mask is used for wildcard matching
	 *
	 * @param terrain 	the terrain which might have a wildcard
	 *
	 * @return 			the mask for this terrain
	 */
	static t_letter get_mask_(const t_letter& terrain);

	/**
	 * converts a string to a layer
	 *
	 * @param str		the terrain string to convert, but needs to be sanitized
	 * 					so no spaces and only the terrain to convert
	 * 					
	 * @return			the converted layer
	 */
	static t_layer string_to_layer_(const std::string& str);
	
	/**
	 * converts a terrain string to a number
	 * @param str 				the terrain string with an optional number
	 * @param start_position	returns the start_position, the caller should set it on -1
	 * 					    	and it's only changed it there is a starting position found
	 * @param filler			if the terrain has only 1 layer then the filler will be used
	 * 							as the second layer
	 *
	 * @return					the letter found in the string
	 */ 					
	static t_letter string_to_number_(const std::string& str, const t_layer filler = NO_LAYER);
	static t_letter string_to_number_(std::string str, int& start_position, const t_layer filler);

	/**
	 * converts a terrain number to a string
	 * 
	 * @param terrain				the terrain number to convert
	 * @param starting_position		the starting position, if smaller than 0 it's
	 * 								ignored else it's written
	 * @param min_size				padds the results with spaces if required untill the 
	 * 								result has a length of min_size
	 * @return						the converted string, if no starting position 
	 * 								given it's padded to 4 chars else padded to 7 chars
	 */
	static std::string number_to_string_(t_letter terrain, const int start_position = -1);
	static std::string number_to_string_(t_letter terrain, const int start_position, const size_t min_size);

	/**
	 * converts a terrain string to a letter for the builder the translation 
	 * rules differ from the normal conversion rules
	 *
	 * @param str	the terrain string
	 *
	 * @return		number for the builder map
	 */
	static t_letter string_to_builder_number_(std::string str);

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

/***************************************************************************************/	

t_letter::t_letter(const std::string& b) : 
	base(string_to_layer_(b)), overlay(NO_LAYER)
{}

t_letter::t_letter(const std::string& b, const t_layer o) : 
	base(string_to_layer_(b)), overlay(o)
{}

t_letter::t_letter(const std::string& b, const std::string& o) : 
	base(string_to_layer_(b)), overlay(string_to_layer_(o)) 
{}

t_match::t_match() :
	has_wildcard(false), is_empty(true)
{}

t_match::t_match(const std::string& str, const t_layer filler):
	terrain(t_translation::read_list(str, -1, t_translation::T_FORMAT_STRING, filler)) 
{
	mask.resize(terrain.size());
	masked_terrain.resize(terrain.size());
	has_wildcard = t_translation::has_wildcard(terrain);
	is_empty = terrain.empty();

	for(size_t i = 0; i < terrain.size(); i++) {
		mask[i] = t_translation::get_mask_(terrain[i]);
		masked_terrain[i] = mask[i] & terrain[i];
	}
}

t_match::t_match(const t_letter& letter):
	terrain(t_list(1, letter)) 
{
	mask.resize(terrain.size());
	masked_terrain.resize(terrain.size());
	has_wildcard = t_translation::has_wildcard(terrain);
	is_empty = terrain.empty();

	for(size_t i = 0; i < terrain.size(); i++) {
		mask[i] = t_translation::get_mask_(terrain[i]);
		masked_terrain[i] = mask[i] & terrain[i];
	}
}

t_letter read_letter(const std::string& str, const int t_format, const t_layer filler)
{
#ifdef TERRAIN_TRANSLATION_COMPATIBLE 
	if(t_format == T_FORMAT_STRING ||
			(t_format == T_FORMAT_AUTO && map_format_ == 2)) {
		return string_to_number_(str, filler);
		
	} else if(t_format == T_FORMAT_LETTER ||
			(t_format == T_FORMAT_AUTO && map_format_ == 1)) {
		return letter_to_number_(str[0]);
		
	} else {
		throw error("Invalid case in read_letter");
	}
#else
		return string_to_number_(str, filler);
#endif
}

std::string write_letter(const t_letter& letter)
{
	return number_to_string_(letter);
}

t_list read_list(const std::string& str, const int separated, const int t_format, const t_layer filler)
{
#ifdef TERRAIN_TRANSLATION_COMPATIBLE 
	if(t_format == T_FORMAT_STRING ||
			(t_format == T_FORMAT_AUTO && map_format_ == 2)) {
		return string_to_vector_(str, filler);
		
	} else if(t_format == T_FORMAT_LETTER ||
			(t_format == T_FORMAT_AUTO && map_format_ == 1)) {
		return string_to_vector_(str, false, separated);
		
	} else {
		throw error("Invalid case in read_list");
	}
#else
		return string_to_vector_(str, filler);
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
	t_map result;

#ifdef TERRAIN_TRANSLATION_COMPATIBLE 
	// the test here is too avoid deprecated warning
	if(str.empty()) {
		return result;
	}

	// process the data, polls for the format. NOTE we test for a comma
	// so an empty map or a map with 1 letter is doomed to be the old
	// format. Shouldn't hurt
	if(str.find(',') == std::string::npos) {
		//old format
		lg::wml_error << "Using the single letter map format is deprecated, support will be removed in version 1.3.3\n";
		map_format_ = 1;
		return read_game_map_old_(str, starting_positions);
	}

	// at this point we're the new format, we also dissapear in the future so
	// inside the ifdef
	map_format_ = 2;
#endif
	
	size_t offset = 0;
	size_t x = 0, y = 0, width = 0;

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
		// the gamemap never has a wildcard
		const t_letter tile = string_to_number_(terrain, starting_position, NO_LAYER);

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

bool terrain_matches(const t_letter& src, const t_letter& dest)
{
	return terrain_matches(src, t_list(1, dest));
}

bool terrain_matches(const t_letter& src, const t_list& dest)
{
	//NOTE we impose some code duplication we could have been rewritten
	//to get a match structure and then call the version with the match
	//structure. IMO that's some extra overhead to this function which is
	//not required. Hence the two versions
	if(dest.empty()) {
		return false;
	}

	const t_letter src_mask = get_mask_(src);
	const t_letter masked_src = (src & src_mask);
	const bool src_has_wildcard = has_wildcard(src);
	
#if 0
	std::cerr << std::hex << "src = " << src.base << "^" << src.overlay << "\t" 
		<< src_mask.base << "^" << src_mask.overlay << "\t" 
		<< masked_src.base << "^" << masked_src.overlay << "\t" 
		<< src_has_wildcard << "\n";
#endif

	bool result = true;
	t_list::const_iterator itor = dest.begin();

	// try to match the terrains if matched jump out of the loop.
	for(; itor != dest.end(); ++itor) {

		// match wildcard 
		if(*itor == STAR) {
			return result;
		}

		// match inverse symbol
		if(*itor == NOT) {
			result = !result;
			continue;
		}

		// full match 
		if(src == *itor) {
			return result;
		}
		
		// does the source wildcard match
		if(src_has_wildcard && 
				(itor->base & src_mask.base) == masked_src.base &&
				(itor->overlay & src_mask.overlay) == masked_src.overlay) {
			return result;
		}
		
		// does the destination wildcard match
		const t_letter dest_mask = get_mask_(*itor);
		const t_letter masked_dest = (*itor & dest_mask);
		const bool dest_has_wildcard = has_wildcard(*itor);
#if 0
		std::cerr << std::hex << "dest= " 
			<< itor->base << "^" << itor->overlay  << "\t" 
			<< dest_mask.base << "^" << dest_mask.overlay << "\t" 
			<< masked_dest.base << "^" << masked_dest.overlay << "\n";
#endif
		if(dest_has_wildcard && 
				(src.base & dest_mask.base) == masked_dest.base &&
				(src.overlay & dest_mask.overlay) == masked_dest.overlay) {
			return result;
		}

		if(src_has_wildcard && src.overlay == 0 && itor->overlay == NO_LAYER &&
				 ((itor->base & src_mask.base) == masked_src.base )) {
			 return result;
		}

		if(dest_has_wildcard && itor->overlay == 0 && src.overlay == NO_LAYER &&
				((src.base & dest_mask.base) == masked_dest.base)) {
			return result;
		}

	}

	// no match, return the inverse of the result
	return !result;
}

// this routine is use for the terrain building so it's one of the delays
// while loading a map. This routine is optimized a bit at the loss of 
// readability.
bool terrain_matches(const t_letter& src, const t_match& dest)
{
	if(dest.is_empty) {
		return false;
	}

	const t_letter src_mask = get_mask_(src);
	const t_letter masked_src = (src & src_mask);
	const bool src_has_wildcard = has_wildcard(src);

	bool result = true;

	// try to match the terrains if matched jump out of the loop.
	// We loop on the dest.terrain since the iterator is faster than operator[]
	// the i holds the value for operator[] since dest.mask and dest.masked_terrain
	// need to be in sync, they are less often looked up so no iterator for them.
	size_t i = 0;
	t_list::const_iterator end = dest.terrain.end();
	for(t_list::const_iterator terrain_itor = dest.terrain.begin();
			terrain_itor != end; 
			++i, ++terrain_itor) {
		
		// match wildcard 
		if(*terrain_itor == STAR) {
			return result;
		}

		// match inverse symbol
		if(*terrain_itor == NOT) {
			result = !result;
			continue;
		}

		// full match 
		if(*terrain_itor == src) {
			return result;
		}
		
		// does the source wildcard match
		if(src_has_wildcard && 
				(terrain_itor->base & src_mask.base) == masked_src.base &&
				(terrain_itor->overlay & src_mask.overlay) == masked_src.overlay) {
			return result;
		}
		
		// does the destination wildcard match
		if(dest.has_wildcard && 
				(src.base & dest.mask[i].base) == dest.masked_terrain[i].base &&
				(src.overlay & dest.mask[i].overlay) == dest.masked_terrain[i].overlay) {
			return result;
		}
		
		// does the source have a wildcard and an empty overlay and the destination
		// no overlay, we need to check the part base for a match
		if(src_has_wildcard && src.overlay == 0 && terrain_itor->overlay == NO_LAYER &&
				 ((terrain_itor->base & src_mask.base) == masked_src.base )) {
			 return result;
		}

		// does the desination have a wildcard and an empty overlay and the source
		// no overlay, we need to check the part base for a match
		// NOTE the has_wildcard(*terrain_itor) is expensive so move the test to
		// later in the line
		if(terrain_itor->overlay == 0 && src.overlay == NO_LAYER && has_wildcard(*terrain_itor) &&
				((src.base & dest.mask[i].base) == dest.masked_terrain[i].base)) {
			return result;
		}
		
	}

	// no match, return the inverse of the result
	return !result;
}

bool has_wildcard(const t_letter& letter) 
{
	if(letter.overlay == NO_LAYER) {
		return get_layer_mask_(letter.base) != NO_LAYER;
	} else {
		return get_layer_mask_(letter.base) != NO_LAYER || get_layer_mask_(letter.overlay) != NO_LAYER;
	}
}

bool has_wildcard(const t_list& list)
{
	if(list.empty()) {
		return false;
	}
	
	// test all items for a wildcard 
	t_list::const_iterator itor = list.begin();
	for(; itor != list.end(); ++itor) {
		if(has_wildcard(*itor)) {
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

#ifdef TERRAIN_TRANSLATION_COMPATIBLE 
void add_translation(const std::string& str, const t_letter& number)
{
	lookup_table_[str[0]] = number;
}

std::string get_old_letter(const t_letter& number) 
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

static t_list string_to_vector_(const std::string& str, const bool convert_eol, const int separated)
{
	// only used here so define here
	const t_letter EOL(7, NO_LAYER);
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

static t_letter letter_to_number_(const TERRAIN_LETTER terrain)
{
	std::map<TERRAIN_LETTER, t_letter>::const_iterator itor = lookup_table_.find(terrain);

	if(itor == lookup_table_.end()) {
		ERR_G << "No translation found for old terrain letter " << terrain << "\n";
		throw error("No translation found for old terrain letter");
	}

	return itor->second;
}

static t_map read_game_map_old_(const std::string& str, std::map<int, coordinate>& starting_positions) 
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

static t_list string_to_vector_(const std::string& str, const t_layer filler)
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
		const t_letter tile = string_to_number_(terrain, filler);

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

inline t_layer get_layer_mask_(t_layer terrain)
{
	// test for the star 0x2A in every postion and return the
	// appropriate mask
/*	
 *	This is what the code intents to do, but in order to gain some more
 *	speed it's changed to the code below, which does the same but faster.
 *	This routine is used often in the builder and the speedup is noticable. */
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

static t_letter get_mask_(const t_letter& terrain)
{
	if(terrain.overlay == NO_LAYER) {
		return t_letter(get_layer_mask_(terrain.base), 0);
	} else {
		return t_letter(get_layer_mask_(terrain.base), get_layer_mask_(terrain.overlay));
	}
}

static t_layer string_to_layer_(const std::string& str)
{
	t_layer result = 0;

	//validate the string
	wassert(str.size() <= 4);
	
	//the conversion to int puts the first char in the 
	//highest part of the number, this will make the 
	//wildcard matching later on a bit easier.
	for(size_t i = 0; i < 4; ++i) {
		const unsigned char c = (i < str.length()) ? str[i] : 0;
		
		// clear the lower area is a nop on i == 0 so 
		// no need for if statement
		result <<= 8; 
		
		// add the result
		result += c;
	}

	return result;
}

static t_letter string_to_number_(const std::string& str, const t_layer filler) {
	int dummy = -1;
	return string_to_number_(str, dummy, filler);
}

static t_letter string_to_number_(std::string str, int& start_position, const t_layer filler)
{
	t_letter result;

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


    offset = str.find('^', 0);
    if(offset !=  std::string::npos) {
		const std::string base_str(str, 0, offset);
		const std::string overlay_str(str, offset + 1, str.size());
		result = t_letter(base_str, overlay_str);
	} else {
		result = t_letter(str, filler);

		//ugly hack 
		if(filler == WILDCARD && (result.base == NOT.base || result.base == STAR.base)) {
			result.overlay = NO_LAYER;
		}
	}	

#ifndef TERRAIN_TRANSLATION_COMPATIBLE 
	if(result == OBSOLETE_KEEP) {
		lg::wml_error << "Using _K for a keep is deprecated, support will be removed in version 1.3.5\n";
		result = HUMAN_KEEP;
	}
#endif
	
	return result;
}

static std::string number_to_string_(t_letter terrain, const int start_position)
{
	std::string result = "";

	//insert the start position
	if(start_position > 0) {
		result = str_cast(start_position) + " ";
	}
	
	//insert the terrain letter
	unsigned char letter[9];
	letter[0] = ((terrain.base & 0xFF000000) >> 24);
	letter[1] = ((terrain.base & 0x00FF0000) >> 16);
	letter[2] = ((terrain.base & 0x0000FF00) >> 8);
	letter[3] =  (terrain.base & 0x000000FF);

	if(terrain.overlay != NO_LAYER) {
		letter[4] = '^'; //the layer separator
		letter[5] = ((terrain.overlay & 0xFF000000) >> 24);
		letter[6] = ((terrain.overlay & 0x00FF0000) >> 16);
		letter[7] = ((terrain.overlay & 0x0000FF00) >> 8);
		letter[8] =  (terrain.overlay & 0x000000FF);
	} else {
		// if no second layer, the second layer won't be written
		// so no need to initialize that part of the array
		letter[4] = 0;
	}
		
	for(int i = 0; i < 9; ++i) {
		if(letter[i] != 0 && letter[i] != 0xFF) {
			result += letter[i];
		}
		if(i == 4 && letter[i] == 0) {
			// no layer, stop
			break;
		}
	}

	return result;
}

static std::string number_to_string_(t_letter terrain, const int start_position, const size_t min_size)
{
	std::string result = number_to_string_(terrain, start_position);
	if(result.size() < min_size) {
		result.resize(min_size, ' ');
	}

	return result;
}

static t_letter string_to_builder_number_(std::string str)
{
	//strip the spaces around us
	const std::string& whitespace = " \t";
	str.erase(0, str.find_first_not_of(whitespace));
	if(! str.empty()) {
		str.erase(str.find_last_not_of(whitespace) + 1);
	}

	// empty string is allowed here so handle it
	if(str.empty()) {
		return t_letter();
	}
	
	const int number = lexical_cast_default(str, -1);
	if(number == -1) {
		// at this point we have a single char
		// which should be interpreted by the map
		// builder, so return this number
		return t_letter(str[0] << 24, 0);
	} else {
		return t_letter(0, number);
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

