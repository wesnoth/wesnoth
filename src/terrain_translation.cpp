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

namespace terrain_translation {

//FIXME MdW remove the shift
// This shift is a dummy, since I've only terrains in the 0 - 127 range
// things work too often too good :(. With the shift I shift the terrains
// in different ranges. This is only for debugging.
// Tested and working shifts 0, 3 - 8, 10, 15 - 17, 20
// 24 aborts with out of memory :(
// memory usage statistics starting the tutorial
// NOTE, samples just from one run so not really reliable
// but the problem becomes clear. When using high terrain numbers the
// memory usage becomes unacceptable
//            VIRT   RES  SHR
// shoft 0    53220  40m  18m
// shift 5    53336  40m  18m
// shift 10   53632  40m  18m
// shift 15   66772  53m  18m
// shift 16   77960  64m  18m 
// shift 17   100m   88m  18m
// shift 20   436m   302m 18m
// After the fix tested 24, 25 both work and the memory consumption of 25 is normal

#define SHIFT 25
#define SET_TERRAIN_CONSTANT(x,y) \
	const TERRAIN_NUMBER x = (y << SHIFT)
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

	TERRAIN_LETTER number_to_letter_(const TERRAIN_NUMBER terrain);
	TERRAIN_NUMBER letter_to_number_(const TERRAIN_LETTER terrain); 

/***************************************************************************************/	

TERRAIN_NUMBER read_letter(const std::string& letter)
{
	wassert(! letter.empty());
	return letter_to_number_(letter[0]);
}

std::string write_letter(const TERRAIN_NUMBER& letter)
{
	// cheap hack reserve space to 1 char and put it in the string
	std::string res = "a";
	res[0] = number_to_letter_(letter);
	return res;
}

std::vector<TERRAIN_NUMBER> read_list(const std::string& list, const int separated)
{
	return string_to_vector_(list, false, separated);
}

std::string write_list(const std::vector<TERRAIN_NUMBER>& list, const int separated)
{
	return vector_to_string_(list, separated);
}

std::vector<TERRAIN_NUMBER> read_map(const std::string& map)
{
	return string_to_vector_(map, true, 0);
}

std::string write_map(const std::vector<TERRAIN_NUMBER>& map)
{
	return vector_to_string_(map, 0);
}


/***************************************************************************************/	
//internal

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

TERRAIN_LETTER number_to_letter_(const TERRAIN_NUMBER terrain)
{
	TERRAIN_NUMBER tmp = (terrain >> SHIFT);
	return (TERRAIN_LETTER)(tmp);
}

TERRAIN_NUMBER letter_to_number_(const TERRAIN_LETTER terrain)
{
	TERRAIN_NUMBER result = (TERRAIN_NUMBER) terrain;
	result = (result << SHIFT);
	return result;
}

/***************************************************************************************/	
// These will probably become obsolete

int letter_to_start_location(const TERRAIN_NUMBER number)
{
	const TERRAIN_LETTER letter = number_to_letter_(number);
	if(letter >= '0' && letter <= '9'){
		return letter - '0';
	} else {
		return -1;
	}
}

TERRAIN_NUMBER get_start_location(int player)
{
	return read_letter(std::string(1, '1' + player));
}

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

}
