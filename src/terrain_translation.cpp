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
#include "tstring.hpp"
#include "config.hpp"
#include "log.hpp"
#include "util.hpp"
#include "terrain_translation.hpp"
#include "serialization/string_utils.hpp"
#include "wassert.hpp"

#include <iostream>

#define ERR_G  LOG_STREAM(err, general)
#define WRN_G  LOG_STREAM(warn, general)

namespace t_translation {

/***************************************************************************************/
// forward declaration of internal functions
	
	// This is the new convertor converts a single line
	// and only acceptes the new terrain string format
	t_list string_to_vector_(const std::string& data);
	
	t_letter get_mask_(t_letter terrain);

#ifdef TERRAIN_TRANSLATION_COMPATIBLE 

	typedef char TERRAIN_LETTER;

	// This function can convert EOL's and converts them to EOL 
	// which doesn't need to be and EOL char
	// this will convert UNIX, Mac and Windows end of line types
	// this due to the fact they all have a different idea of EOL
	// Note this also eats all blank lines so the sequence "\n\n\n" will become just 1 EOL
	t_list string_to_vector_(const std::string& map_data, const bool convert_eol, const int separated);

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
	t_map read_game_map_old(const std::string& map,std::map<int, coordinate>& starting_positions); 

#endif

	// the low level convertors, these function are the ones which
	// now about the internal format. All other functions are unaware
	// of the internal format

	//converts a terrain string to a number
	// terrain 			the terrain with an optional number
	// start_position 	returns the start_position, the caller should set it on -1
	// 					and it's only changed it there is a starting position found
	t_letter string_to_number_(const std::string terrain);
	t_letter string_to_number_(std::string terrain, int& start_position);

	//converts a terrain number to a string
	// terrain				the terrain number to convert
	// starting_position	the starting position, if smaller than 0 it's ignored else it's written
	// returns				the converted string, if no starting position given it's padded to 4 chars
	// 						else padded to 7 chars
	std::string number_to_string_(t_letter terrain, const int start_position = -1);

	t_letter string_to_builder_number_(std::string terrain);

/***************************************************************************************/	

#define SET_TERRAIN_CONSTANT(x,y) \
	const t_letter x = (y)

#define SET_TERRAIN_CONSTANT_NEW(x, y) \
	const t_letter x = string_to_number_(y)

SET_TERRAIN_CONSTANT(VOID_TERRAIN, ' '); // to be translated
SET_TERRAIN_CONSTANT(FOGGED, '~'); // to be translated

SET_TERRAIN_CONSTANT_NEW(CASTLE, "Ch");
SET_TERRAIN_CONSTANT_NEW(SHALLOW_WATER, "Ww");
SET_TERRAIN_CONSTANT(DEEP_WATER, 's'); //to be translated
SET_TERRAIN_CONSTANT_NEW(GRASS_LAND, "Gg");
SET_TERRAIN_CONSTANT_NEW(FOREST, "Ff");
SET_TERRAIN_CONSTANT(MOUNTAIN, 'm'); //to be translated
SET_TERRAIN_CONSTANT(HILL, 'h'); //to be translated

SET_TERRAIN_CONSTANT(CAVE_WALL, 'W'); //to be translated
SET_TERRAIN_CONSTANT(CAVE, 'u'); //to be translated
SET_TERRAIN_CONSTANT(UNDERGROUND_VILLAGE, 'D'); //to be translated
SET_TERRAIN_CONSTANT(DWARVEN_CASTLE, 'o'); //to be translated

SET_TERRAIN_CONSTANT_NEW(PLUS, "+");
SET_TERRAIN_CONSTANT_NEW(MINUS, "-");
//if only used in terrain match code it can move here since 
//we will contain terrain match code
SET_TERRAIN_CONSTANT_NEW(NOT, "!");
SET_TERRAIN_CONSTANT(COMMA, ','); //to be translated?

	// the shift used for the builder (needs more comment)
	const int BUILDER_SHIFT = 8;
	
/***************************************************************************************/	

t_letter read_letter(const std::string& str, const int t_format)
{
	wassert(! str.empty());
#ifdef TERRAIN_TRANSLATION_COMPATIBLE 
	if(t_format == T_FORMAT_STRING ||
			(t_format == T_FORMAT_AUTO && map_format_ == 2)) {
		return string_to_number_(str);
		
	} else if(t_format == T_FORMAT_LETTER ||
			(t_format == T_FORMAT_AUTO && map_format_ == 1)) {
		return letter_to_number_(str[0]);
		
	} else {
		wassert(false); //unknown case
	}
#else
		return string_to_number_(str);
#endif
}

std::string write_letter(const t_letter& letter)
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
		wassert(false); //unknown case
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

t_map read_builder_map(const std::string& str)
{

	size_t offset = 0;
	t_map result(1);//FIXME MdW find a way to initialize and empty vector
	result.clear();

	// skip the leading newlines
	while(offset < str.length() && utils::isnewline(str[offset])) {
		++offset;
	}
	
	// did we get an empty map?
	if((offset + 1) >= str.length()) {
		WRN_G << "Empty map found\n";
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
			//skip the following newlines FIXME MdW this should be documented "aa<CR><CR>bb<CR><CR><CR>" is now valid
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
		wassert(number >= 0 && number < 2^24); 
		return (number << BUILDER_SHIFT);
	}
}
	
t_map read_game_map(const std::string& str,	std::map<int, coordinate>& starting_positions)
{
#ifdef TERRAIN_TRANSLATION_COMPATIBLE 

	// process the data, polls for the format
	if(str.find(',') == std::string::npos) {
		//old format
		map_format_ = 1;
		return read_game_map_old(str, starting_positions);
	}

	// at this point we're the new format
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
		WRN_G << "Empty map found\n";
		return result;
	}
		
	while(offset < str.length()) {

		// get a terrain chunk
		const std::string separators = ",\n\r";
		const int pos_separator = str.find_first_of(separators, offset);
		const std::string terrain = str.substr(offset, pos_separator - offset);

		// process the chunk
		int starting_position = -1; 
		const t_letter tile = string_to_number_(terrain, starting_position);

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
		} 

		// make space for the new item
		if(result.size() <= x) {
			result.resize(x + 1);
		}
		if(result[x].size() <= y) {
			result[x].resize(y + 1);
		}
		
		// add the resulting terrain number,
		result[x][y] = tile;

		//evaluate the separator
		if(utils::isnewline(str[pos_separator])) {
			// the first line we set the with the other lines we check the width
			if(y == 0 ) { 
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
			
			offset =  pos_separator + 1;
			//skip the following newlines FIXME this should be documented "aa<CR><CR>bb<CR><CR><CR>" is now valid
			while(offset < str.length() && utils::isnewline(str[offset])) {
				++offset;
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


/***************************************************************************************/	
//internal

t_list string_to_vector_(const std::string& str)
{
	// handle an empty string
	t_list result(1); //FIXME MdW find a way to initialize and empty vector
	result.clear();
	if(str.empty()) {
		WRN_G << "Empty list found\n"; //or info??
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

t_letter string_to_number_(const std::string str) {
	int dummy = -1;
	return string_to_number_(str, dummy);
}

t_letter string_to_number_(std::string str, int& start_position)
{
	t_letter result = NONE_TERRAIN;

	//strip the spaces around us
	const std::string& whitespace = " \t";
	str.erase(0, str.find_first_not_of(whitespace));
	str.erase(str.find_last_not_of(whitespace) + 1);

	// split if we have 1 space inside
	const size_t offset = str.find(' ', 0);
	if(offset != std::string::npos) {
		start_position = lexical_cast<int>(str.substr(0, offset));
		str.erase(0, offset + 1);
	}

	//the conversion to int puts the first char in the 
	//highest part of the number, this will make the 
	//wildcard matching later on a bit easier.
	//
	//FIXME MdW there should be tested how slow this
	//method is, not sure it's the fastest possible
	//solution.
	for(size_t i = 0; i < 4; ++i) {
		unsigned char c;
		if(i < str.length()) {
			c = str[i];
		} else {
			c = 0;
		}
		
		// clear the lower area is a nop on i == 0 so 
		// no need for if statement
		result = (result << 8);
		
		// add the result
		result += c;
	}
	
	return result;
}

std::string number_to_string_(t_letter terrain, const int start_position)
{
	std::string result = std::string();

	//insert the start position
	if(start_position > 0) {
		result = str_cast(start_position) + " ";
	}
	
	//insert the terrain letter
	unsigned char letter[4];
	letter[0] = ((terrain & 0xFF000000) >> 24);
	letter[1] = ((terrain & 0x00FF0000) >> 16);
	letter[2] = ((terrain & 0x0000FF00) >> 8);
	letter[3] = (terrain & 0x000000FF);
		
	for(int i = 0; i < 4; ++i) {
		if(letter[i] != 0) {
			result.push_back(letter[i]);
		} else {
			// no letter, means no more letters at all
			// so leave the loop
			break;
		}
	}

	// make sure the string gets the proper size
	result.resize(7, ' ');
	
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
			str << number_to_string_(map[x][y], starting_position);
		}

		str << "\n";
	}

	return str.str();
}

//FIXME MdW, there are some optimizations which can be done here
//but look at that later
bool terrain_matches(const t_letter src, const t_letter dest)
{
	const t_list list(1, dest);
	return terrain_matches(src, list);

}
	 
t_letter get_mask_(t_letter terrain)
{
	//FIXME MdW add documentation
	const t_letter result_mask[5] = 
		{0x00000000, 0xFF000000, 0xFFFF0000, 0xFFFFFF00, 0xFFFFFFFF};

	// mask to find the '*' is hexcode 2A
	const t_letter wildcard_mask[4] = 
		{0x2A000000, 0x002A0000, 0x00002A00,0x0000002A};

	// match the first position of the * and
	// return the appropriate result mask
	for(int i = 0; i < 4; ++i) {
		if((terrain & wildcard_mask[i]) == wildcard_mask[i]) {
			return result_mask[i];
		}
	}

	// no match return default mask
	return result_mask[4];
}

//FIXME MdW, there are some optimizations which can be done here
//but look at that later
// FIXME MdW remove debug code
bool terrain_matches(const t_letter src, const t_list& dest)
{

//	std::cerr << "Terrain matching src = " << write_letter(src) << " dest = " << write_list(dest) << "\n";
	if(dest.empty()) {
//		std::cerr << ">> Empty result, no match\n";
		return false;
	}

	const t_letter star = string_to_number_("*");
	const t_letter inverse = string_to_number_("!");
	
	const t_letter src_mask = get_mask_(src);
	const t_letter masked_src = (src & src_mask);
	
	bool result = true;
	t_list::const_iterator itor = dest.begin();

	// try to match the terrains if matched jump out of the loop.
	for(; itor != dest.end(); ++itor) {
//		std::cerr << ">> Testing dest " << write_letter(*itor) << "\n";

		// match wildcard 
		if(*itor == star) {
//			std::cerr << ">>>> wildcard match\n";
			return result;
		}

		// match inverse symbol
		if(*itor == inverse) {
//			std::cerr << ">>>> inverse symbol matched\n";
			result = !result;
			continue;
		}

		// full match 
		if(src == *itor) {
//			std::cerr << ">>>> Full match\n";
			return result;
		}
		
		// test on wildcards
		const t_letter dest_mask = get_mask_(*itor);
		const t_letter masked_dest = (*itor & dest_mask);

		// does the source wildcard match
		if((*itor & src_mask) == masked_src) {
//			std::cerr << ">>>> Source wildcard matched\n";
			return result;
		}
		
		// does the destination wildcard match
		if((src & dest_mask) == masked_dest) {
//			std::cerr << ">>>> Destination wildcard matched\n";
			return result;
		}
	}

	// no match, return the inverse of the result
//	std::cerr << ">> No match found\n";
	return !result;
}

/***************************************************************************************/	

#ifdef TERRAIN_TRANSLATION_COMPATIBLE 

t_list string_to_vector_(const std::string& data, const bool convert_eol, const int separated)
{
	// only used here so define here
	SET_TERRAIN_CONSTANT(EOL, 7);
	bool last_eol = false;
	t_list result = t_list(); 


	std::string::const_iterator itor = data.begin();
	for( ; itor != data.end(); ++itor) {
		
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

t_letter cast_to_builder_number(t_letter terrain) 
{
		return (terrain >> BUILDER_SHIFT); 
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

// The terrain letter is an old letter and will be converted with get_letter
void add_translation(const std::string& str, const t_letter number)
{
	// if the table is empty we need to load some
	// hard-coded values for the custom terrains
	// FIXME: remove this after Wesnoth 1.4 or 2.0
	if(lookup_table_.empty()) {
		lookup_table_.insert(std::pair<TERRAIN_LETTER, t_letter>(' ', VOID_TERRAIN));	
		lookup_table_.insert(std::pair<TERRAIN_LETTER, t_letter>('~', FOGGED));	
		//UMC
		lookup_table_.insert(std::pair<TERRAIN_LETTER, t_letter>('^', string_to_number_("_za")));	
		lookup_table_.insert(std::pair<TERRAIN_LETTER, t_letter>('@', string_to_number_("_zb")));	
		lookup_table_.insert(std::pair<TERRAIN_LETTER, t_letter>('x', string_to_number_("_zx")));	
		lookup_table_.insert(std::pair<TERRAIN_LETTER, t_letter>('y', string_to_number_("_zy")));	
		lookup_table_.insert(std::pair<TERRAIN_LETTER, t_letter>('z', string_to_number_("_zz")));	
	}

	// we translate the terrain manual since the helper
	// functions use the translation table and give
	// chicken and egg problem
	TERRAIN_LETTER terrain = str[0];  //FIXME MdW rename terrain to letter 
	std::map<TERRAIN_LETTER, t_letter>::iterator index = lookup_table_.find(terrain);
	
	if(index == lookup_table_.end()) {
		// add new item
		lookup_table_.insert(std::pair<TERRAIN_LETTER, t_letter>(terrain, number));
	} else {
		// replace existing item
		index->second = number;
	}
}

//FIXME MdW this code is ready and tested only the map format needs to be documentated
//some debug code needs to be removed later on
t_map read_game_map_old(const std::string& str, std::map<int, coordinate>& starting_positions) 
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
			
			//skip the following newlines FIXME MdW this should be documented "aa<CR><CR>bb<CR><CR><CR>" is now valid
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
			//sceduled for removal the hardcoded letter is oke
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

