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

namespace terrain_translation {


	
#define SET_TERRAIN_CONSTANT(x,y) \
	const TERRAIN_NUMBER x = (y)
SET_TERRAIN_CONSTANT(VOID_TERRAIN, ' ');
SET_TERRAIN_CONSTANT(FOGGED, '~');
SET_TERRAIN_CONSTANT(KEEP, 'K');

SET_TERRAIN_CONSTANT(CASTLE, 'C');
SET_TERRAIN_CONSTANT(SHALLOW_WATER, 'c');
SET_TERRAIN_CONSTANT(DEEP_WATER, 's');
SET_TERRAIN_CONSTANT(GRASS_LAND, 'g');
SET_TERRAIN_CONSTANT(FOREST, 'f');
SET_TERRAIN_CONSTANT(MOUNTAIN, 'm');
SET_TERRAIN_CONSTANT(HILL, 'h');

SET_TERRAIN_CONSTANT(CAVE_WALL, 'W');
SET_TERRAIN_CONSTANT(CAVE, 'u');
SET_TERRAIN_CONSTANT(UNDERGROUND_VILLAGE, 'D');
SET_TERRAIN_CONSTANT(DWARVEN_CASTLE, 'o');

SET_TERRAIN_CONSTANT(PLUS, '+');
SET_TERRAIN_CONSTANT(MINUS, '-');
SET_TERRAIN_CONSTANT(STAR, '*');
SET_TERRAIN_CONSTANT(NOT, '!');
SET_TERRAIN_CONSTANT(EOL, 7);
SET_TERRAIN_CONSTANT(DOT, '.');
SET_TERRAIN_CONSTANT(COMMA, ',');
SET_TERRAIN_CONSTANT(NONE_TERRAIN, 0); // undefined terrain

/*
const terrain_translation::TERRAIN_NUMBER terrain_translation::VOID_TERRAIN = ' ' ;
const terrain_translation::TERRAIN_NUMBER terrain_translation::FOGGED = '~' ;
const terrain_translation::TERRAIN_NUMBER terrain_translation::KEEP = 'K' ;

const terrain_translation::TERRAIN_NUMBER terrain_translation::CASTLE = 'C' ;
const terrain_translation::TERRAIN_NUMBER terrain_translation::SHALLOW_WATER = 'c' ;
const terrain_translation::TERRAIN_NUMBER terrain_translation::DEEP_WATER = 's' ;
const terrain_translation::TERRAIN_NUMBER terrain_translation::GRASS_LAND = 'g' ;

const terrain_translation::TERRAIN_NUMBER terrain_translation::CAVE_WALL = 'W' ;
const terrain_translation::TERRAIN_NUMBER terrain_translation::CAVE = 'u' ;
const terrain_translation::TERRAIN_NUMBER terrain_translation::UNDERGROUND_VILLAGE = 'D' ;
const terrain_translation::TERRAIN_NUMBER terrain_translation::DWARVEN_CASTLE = 'o' ;

const terrain_translation::TERRAIN_NUMBER terrain_translation::PLUS = '+' ;
const terrain_translation::TERRAIN_NUMBER terrain_translation::MINUS = '-' ;
const terrain_translation::TERRAIN_NUMBER terrain_translation::STAR = '*' ;
const terrain_translation::TERRAIN_NUMBER terrain_translation::NOT = '!' ;
const terrain_translation::TERRAIN_NUMBER terrain_translation::EOL = 7 ;
const terrain_translation::TERRAIN_NUMBER terrain_translation::DOT = '.' ;
const terrain_translation::TERRAIN_NUMBER terrain_translation::COMMA = ',' ;
*/
/***************************************************************************************/
// forward declaration of internal functions

	// This function can convert EOL's and converts them to EOL 
	// which doesn't need to be and EOL char
	// this will convert UNIX, Mac and Windows end of line types
	// this due to the fact they all have a different idea of EOL
	// Note this also eats all blank lines so the sequence "\n\n\n" will become just 1 EOL
	std::vector<TERRAIN_NUMBER> string_to_vector_(const std::string& map_data, const bool convert_eol, const int separated);
	
	std::string vector_to_string_(const std::vector<TERRAIN_NUMBER>& map_data, const int separated);

#ifdef TERRAIN_TRANSLATION_COMPATIBLE 

	// When the terrain is loaded it sends all letter, string combinations
	// to add_translation. This way the translation table is build.
	// This way it's possible to read old maps and convert them to
	// the proper internal format
	static std::map<TERRAIN_LETTER, TERRAIN_NUMBER> lookup_table_;
	
	// This value contains the map format used, when reading the main
	// map this format should be set, don't know how we're going to do
	// it but we will. This format determines whether the WML
	// map and letter are read old or new format.
	// formats
	// 0 = unknown
	// 1 = old single letter format
	// 2 = new multi letter format
	static int map_format_ = 1; //FIXME MdW this should be initialized to 0

	//old low level converters
	TERRAIN_LETTER number_to_letter_(const TERRAIN_NUMBER terrain);
	TERRAIN_NUMBER letter_to_number_(const TERRAIN_LETTER terrain); 

	// reads old terrain graphics map (will be obsoleted before release)
	std::vector<std::vector<TERRAIN_NUMBER> > read_map_old(const std::string& map);

	// reads old maps
	std::vector<std::vector<TERRAIN_NUMBER> > read_game_map_old(const std::string& map, 
			std::map<int, coordinate>& starting_positions); 
#endif

	// the low level convertors, these function are the ones which
	// now about the internal format. All other functions are unaware
	// of the internal format

	//converts a terrain string to a number
	// terrain 			the terrain with an optional number
	// start_position 	returns the start_position, the caller should set it on -1
	// 					and it's only changed it there is a starting position found
	TERRAIN_NUMBER string_to_number_(const std::string terrain);
	TERRAIN_NUMBER string_to_number_(std::string terrain, int& start_position);

	//converts a terrain number to a string
	// terrain				the terrain number to convert
	// starting_position	the starting position, if smaller than 0 it's ignored else it's written
	// returns				the converted string, if no starting position given it's padded to 4 chars
	// 						else padded to 7 chars
	std::string number_to_string_(TERRAIN_NUMBER terrain, const int start_position = -1);

/***************************************************************************************/	

//STATUS - testing	
TERRAIN_NUMBER read_letter(const std::string& letter, const int tformat)
{
	wassert(! letter.empty());
#ifdef TERRAIN_TRANSLATION_COMPATIBLE 
	if(tformat == TFORMAT_STRING ||
			(tformat == TFORMAT_AUTO && map_format_ == 2)) {
		return string_to_number_(letter);
	} else if(tformat == TFORMAT_LETTER ||
			(tformat == TFORMAT_AUTO && map_format_ == 1)) {
		return letter_to_number_(letter[0]);
	} else {
		wassert(false); //unknown case
	}
#else
		return string_to_number_(letter);
#endif
}

//STATUS - testing old code needs to be removed before testing
std::string write_letter(const TERRAIN_NUMBER& letter)
{
	
	// cheap hack reserve space to 1 char and put it in the string
	std::string res = "a";
	res[0] = number_to_letter_(letter);
	return res;


	//FIXME MdW this should be the new way but since the new 
	//multi-letters are not there yet, it can't be used
	//hence below the old routine
	return number_to_string_(letter);
	
}

//STATUS - under investigation
std::vector<TERRAIN_NUMBER> read_list(const std::string& list, const int separated, const int tformat)
{
	return string_to_vector_(list, false, separated);
}

//STATUS - under investigation
std::string write_list(const std::vector<TERRAIN_NUMBER>& list, const int separated)
{
	return vector_to_string_(list, separated);
}

//STATUS - none
std::vector<std::vector<TERRAIN_NUMBER> > read_builder_map(const std::string& map)
{

}

//STATUS - ready only compile tested
std::vector<std::vector<TERRAIN_NUMBER> > read_game_map(const std::string& map, 
	std::map<int, coordinate>& starting_positions)
{
#ifdef TERRAIN_TRANSLATION_COMPATIBLE 

	// process the data, polls for the format
	if(map.find(',') == std::string::npos) {
		//old format
		map_format_ = 1;
		return read_game_map_old(map, starting_positions);
	}

	// at this point we're the new format
	map_format_ = 2;
#endif

	size_t offset = 0;
	size_t x = 0, y = 0, width = 0;
	std::vector<std::vector<TERRAIN_NUMBER> > result;

	// skip the leading newlines
	while(offset < map.length() && utils::isnewline(map[offset])) {
		++offset;
	}
	
	// did we get an empty map?
	if((offset + 1) >= map.length()) {
		WRN_G << "Empty map found\n";
		return result;
	}
		
	while(offset < map.length()) {

		// get a terrain chunk
		const std::string separators = ",\n\r";
		const int pos_separator = map.find_first_of(separators, offset);
		const std::string terrain = map.substr(offset, pos_separator - offset - 1);

		// process the chunk
		int starting_position = -1; 
		const TERRAIN_NUMBER tile = string_to_number_(terrain, starting_position);

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

		//FIXME MdW remove the debug code
		std::cerr << "offset = " << offset << " terrain = " << terrain << " tile = " 
			<< tile << " x = " << x << " y = " << y 
			<< " starting postion = " << starting_position << "\n";

		//evaluate the separator
		if(utils::isnewline(map[pos_separator])) {
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
			while(offset < map.length() && utils::isnewline(map[offset])) {
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

	//FIXME MdW remove debug code
	// if at the end of a line the user didn't add a trailing
	// return, this is no error so fix it. 
	if(x == width) {
		++y;
	}
	int height = y;
	std::cerr << "map read width = " << width << " height = " << height << " data =\n";

	for(x = 0; x < width; ++x) {
		for(y = 0; y < height; ++y) {
			std::cerr << result[x][y] << ",";
		}
		std::cerr << "\n";
	}
	
	return result;
}


/***************************************************************************************/	
//internal

//STATUS - under investigation
std::string vector_to_string_(const std::vector<TERRAIN_NUMBER>& map_data, const int separated)
{
	std::string result; 

	std::vector<TERRAIN_NUMBER>::const_iterator itor = map_data.begin();
	for( ; itor != map_data.end(); ++itor) {
		if(*itor == EOL){
			result.push_back('\n');
		} else {
			result.push_back(number_to_letter_(*itor));
		}
		// all get a separtor if requested
		if(separated == 1){
			 result.push_back(',');
		}
	}
	
	//remove the last separator
	if(! result.empty() &&  separated == 1){
		result.erase(result.end() - 1);
	}
	
	return result;
}

//STATUS - under investigation
std::vector<TERRAIN_NUMBER> string_to_vector_(const std::string& data, const bool convert_eol, const int separated)
{
	bool last_eol = false;
	std::vector<TERRAIN_NUMBER> result = std::vector<TERRAIN_NUMBER>(); 

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

//STATUS - testing
TERRAIN_NUMBER string_to_number_(const std::string terrain) {
	int dummy = -1;
	return string_to_number_(terrain, dummy);
}

//STATUS - testing
TERRAIN_NUMBER string_to_number_(std::string terrain, int& start_position)
{
	TERRAIN_NUMBER result = 0;

	//strip the spaces around us
	const std::string& whitespace = " \t";
	terrain.erase(0, terrain.find_first_not_of(whitespace));
	terrain.erase(terrain.find_last_not_of(whitespace));

	// split if we have 1 space inside
	const size_t offset = terrain.find(' ', 0);
	if(offset != std::string::npos) {
		start_position = lexical_cast<int>(terrain.substr(0, offset));
		terrain.erase(0, offset + 1);
	}

	//the conversion to int puts the first char in the 
	//highest part of the number, this will make the 
	//wildcard matching later on a bit easier.
	//
	//FIXME MdW there should be tested how slow this
	//method is, not sure it's the fastest possible
	//solution.
	size_t c = terrain.length() - 1;
	int shift = 24;
	while(c != 0) {
		result = result + (terrain[c] << shift);
		shift -= 8;
		--c;
		}
	return result;
}

//STATUS - testing
std::string number_to_string_(TERRAIN_NUMBER terrain, const bool start_of_line, const int start_position)
{
	std::string result = "";
	char letter;
	int i;

	for(i = 0; i < 3; ++i) {
		//this will result in an overflow and the high bits will
		//drop off
		letter = terrain;	

		//filter the null chars
		if(letter != 0) {
			result.insert(0, 1, letter);
		} else {
			result.insert(0, 1, ' '); // the padding 
		}
		//move the next number in the lower part of the number
		terrain = (terrain >> 8);
	}

	//insert the start position
	if(start_position > 0) {
		std::string start(str_cast(start_position));
//		start = utils::itoa(start_position);
		if(start.size() == 2) {
			// pad to 7 chars
			result += ' ';
		}
	}
	
	return result;
}

//STATUS - ready needs modification once the new system is really used
std::string write_game_map(const std::vector<std::vector<TERRAIN_NUMBER> >& map,
	 std::map<int, coordinate> starting_positions)
{

	//FIXME MdW commented lines should be enabled, the others disabled
	std::stringstream str;
	for(size_t y = 0; y < map[0].size(); ++y) {
		for(size_t x = 0; x < map.size(); ++x) {

			// If the current location is a starting position it needs to
			// be added to the terrain. After it's found it can't be found
			// again so the location is removed from the map.
			std::map<int, coordinate>::iterator itor = starting_positions.begin();
			TERRAIN_LETTER starting_position = NONE_TERRAIN; //disable
//			int starting_position = -1;
			for(; itor != starting_positions.end(); ++itor) {
				if(itor->second.x == x && itor->second.y == y) {
					starting_position = itor->first + '0'; //disable
//					starting_position = itor->first;
					starting_positions.erase(itor);
					break;
				}
			}

			// add the separator
//			if(x != 0) {
//				std << ", ";
//			}
//			str << number_to_string_(map[x][y], starting_position);
//
			if(starting_position != NONE_TERRAIN) { //disable entire if statement
				str << starting_position;
			} else {
				str << write_letter(map[x][y]);
			}
		}

		str << "\n";
	}

	return str.str();

}
/***************************************************************************************/	
// These will probably become obsolete

int list_to_int(const std::vector<TERRAIN_NUMBER> number)
{	
	std::string data = "";
	std::vector<TERRAIN_NUMBER>::const_iterator itor = number.begin();

	for(; itor != number.end(); ++itor){
		data += number_to_letter_(*itor);
	}

	if(data.find_first_of("0123456789") != std::string::npos) {
		return atoi(data.c_str());
	} else {
		return -1;
	}
}

std::vector<TERRAIN_NUMBER> read_map(const std::string& map)
{
	return string_to_vector_(map, true, 0);
}

#ifdef TERRAIN_TRANSLATION_COMPATIBLE 

TERRAIN_LETTER number_to_letter_(const TERRAIN_NUMBER terrain)
{
	TERRAIN_NUMBER tmp = (terrain);
	return (TERRAIN_LETTER)(tmp);
}

TERRAIN_NUMBER letter_to_number_(const TERRAIN_LETTER terrain)
{
	TERRAIN_NUMBER result = (TERRAIN_NUMBER) terrain;
	result = (result);
	return result;
}

std::string write_map(const std::vector<TERRAIN_NUMBER>& map)
{
	return vector_to_string_(map, 0);
}

// The terrain letter is an old letter and will be converted with get_letter
void add_translation(const std::string& letter, const TERRAIN_NUMBER number)
{
	// if the table is empty we need to load some
	// hard-coded values for the custom terrains
	// FIXME: remove this after Wesnoth 1.4 or 2.0
	if(lookup_table_.empty()) {
		lookup_table_.insert(std::pair<TERRAIN_LETTER, TERRAIN_NUMBER>(' ', VOID_TERRAIN));	
		lookup_table_.insert(std::pair<TERRAIN_LETTER, TERRAIN_NUMBER>('~', FOGGED));	
		//UMC
		lookup_table_.insert(std::pair<TERRAIN_LETTER, TERRAIN_NUMBER>('^', string_to_number_("_za")));	
		lookup_table_.insert(std::pair<TERRAIN_LETTER, TERRAIN_NUMBER>('@', string_to_number_("_zb")));	
		lookup_table_.insert(std::pair<TERRAIN_LETTER, TERRAIN_NUMBER>('x', string_to_number_("_zx")));	
		lookup_table_.insert(std::pair<TERRAIN_LETTER, TERRAIN_NUMBER>('y', string_to_number_("_zy")));	
		lookup_table_.insert(std::pair<TERRAIN_LETTER, TERRAIN_NUMBER>('z', string_to_number_("_zz")));	
	}

	TERRAIN_LETTER first = read_letter(letter, TFORMAT_LETTER);
	std::map<TERRAIN_LETTER, TERRAIN_NUMBER>::iterator index = lookup_table_.find(first);
	
	if(index == lookup_table_.end()) {
		// add new item
		lookup_table_.insert(std::pair<TERRAIN_LETTER, TERRAIN_NUMBER>(first, number));
	} else {
		// replace existing item
		index->second = number;
	}
}

//FIXME MdW this code is obsolete as soon as the terrain map is converted
//so no need to clean it at all
std::vector<std::vector<TERRAIN_NUMBER> > read_map_old(const std::string& map)
{

	int width = 0, height = 0;
	int offset = 0;

	// skip the leading newlines
	while(offset < map.length() && utils::isnewline(map[offset])) {
		++offset;
	}
	
	int x = 0, y = 0;
	std::vector<std::vector<TERRAIN_NUMBER> > result;

	while(offset < map.length()) {
		TERRAIN_LETTER terrain;
		TERRAIN_NUMBER tile;

		// handle newlines
		if(utils::isnewline(map[offset])) {
			// the first line we set the with the other lines we check the width
			if(y == 0 ) { 
				width = x; 
			} 
			// prepare next itertration 
			++y;
			x = 0;
			
			++offset;
			while(offset < map.length() && utils::isnewline(map[offset])) {
				++offset;
			}

			// stop if at end of file
			if((offset + 1) == map.length()) {
				break;
			}
			
		} else {
			++offset;
		}

		// get a terrain chunk
		terrain = map[offset];

		// process the chunk
		tile = letter_to_number_(terrain);

		// add the resulting terrain number
		if(result.size() <= x) {
			result.resize(x + 1);
		}

		//width
		if(result[x].size() <= y) {
				result[x].resize(y + 1);
		}
		
		// add	
		result[x][y] = tile;
		//set next value
		++x; 
	}

	height = y; 
	return result;

}

//FIXME MdW this code is ready and tested only the map format needs to be documentated
//some debug code needs to be removed later on
std::vector<std::vector<TERRAIN_NUMBER> > read_game_map_old(const std::string& map, 
		/*const int mformat,*/ std::map<int, coordinate>& starting_positions) 
{
	size_t offset = 0;
	size_t x = 0, y = 0, width = 0;
	std::vector<std::vector<TERRAIN_NUMBER> > result;
	
	// skip the leading newlines
	while(offset < map.length() && utils::isnewline(map[offset])) {
		++offset;
	}

	// did we get an empty map?
	if((offset + 1) >= map.length()) {
		WRN_G << "Empty map found\n";
		return result;
	}
	
	while(offset < map.length()) {

		// handle newlines
		if(utils::isnewline(map[offset])) {
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
			while(offset < map.length() && utils::isnewline(map[offset])) {
				++offset;
			}
			
			// stop if at end of file
			if((offset + 1) >= map.length()) {
				break;
			}
		}
		
		// get a terrain chunk
		TERRAIN_LETTER terrain = map[offset];

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

		//FIXME MdW remove the debug code
		std::cerr << "offset = " << offset << " terrain = " << terrain << " tile = " 
			<< letter_to_number_(terrain) << " x = " << x << " y = " << y 
			<< " starting postion = " << starting_position << "\n";

		//set next value
		++x; 
		++offset;
	}

	if(x != 0 && x != width) {
		ERR_G << "Map not a rectangle error occured at the end\n"; 
		throw error("Map not a rectangle.");
	}

	//FIXME MdW remove debug code
	// if at the end of a line the user didn't add a trailing
	// return, this is no error so fix it. 
	if(x == width) {
		++y;
	}
	int height = y;
	std::cerr << "map read width = " << width << " height = " << height << " data =\n";

	for(x = 0; x < width; ++x) {
		for(y = 0; y < height; ++y) {
			std::cerr << result[x][y] << ",";
		}
		std::cerr << "\n";
	}
	
	return result;
}

#endif	

}
