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

#include "global.hpp"
#include "tstring.hpp"
#include "config.hpp"
#include "log.hpp"
#include "util.hpp"
#include "terrain_translation.hpp"
#include "serialization/string_utils.hpp"
#include "wassert.hpp"

//FIXME MdW for some strange reason only shift 8 and 16 seem to 
//work. Even 0 which doesn't do anything fails. It seems there
//a bug lurking. With the "wrong" values the terrain transition
//fails. With the 8 and 16 at least 1 transition fails, to be 
//seen in the tutorial at 9,1. This also fails with shift 0.
//The shift was orignal intended to make sure the first byte
//of the terrain is empty so callers who would access a byte
//would find a 0 byte.
#define SHIFT 8
#define SET_TERRAIN_CONSTANT(x,y) \
	const terrain_translation::TERRAIN_NUMBER terrain_translation::x = (y)

//	const terrain_translation::TERRAIN_NUMBER terrain_translation::x = (y << SHIFT)

SET_TERRAIN_CONSTANT(VOID_TERRAIN, ' ');
SET_TERRAIN_CONSTANT(FOGGED, '~');
SET_TERRAIN_CONSTANT(KEEP, 'K');

SET_TERRAIN_CONSTANT(CASTLE, 'C');
SET_TERRAIN_CONSTANT(SHALLOW_WATER, 'c');
SET_TERRAIN_CONSTANT(DEEP_WATER, 's');
SET_TERRAIN_CONSTANT(GRASS_LAND, 'g');

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

terrain_translation::terrain_translation() {}
terrain_translation::~terrain_translation(){}

terrain_translation::TERRAIN_LETTER terrain_translation::number_to_letter_(const terrain_translation::TERRAIN_NUMBER terrain) const
{
	return terrain; //FIXME MdW debug

	TERRAIN_NUMBER tmp = (terrain >> SHIFT);
	return (TERRAIN_LETTER)(tmp);
}

terrain_translation::TERRAIN_NUMBER terrain_translation::letter_to_number_(const terrain_translation::TERRAIN_LETTER terrain) const
{
	return terrain; //FIXME MdW debug

	TERRAIN_NUMBER result = (TERRAIN_NUMBER) terrain;
	result = (result << SHIFT);
	return result;
}

int terrain_translation::list_to_int(const std::vector<terrain_translation::TERRAIN_NUMBER> number)const
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

int terrain_translation::letter_to_start_location(const terrain_translation::TERRAIN_NUMBER number) const
{
	const TERRAIN_LETTER letter = number_to_letter_(number);
	if(letter >= '0' && letter <= '9'){
		return letter - '0';
	} else {
		return -1;
	}
}
 
std::vector<std::vector<terrain_translation::TERRAIN_NUMBER> > terrain_translation::get_splitted_list(const std::string& list) const
{

	//for now use the standard splitter
	const std::vector<std::string> data = utils::split(list);
	std::vector<std::vector<terrain_translation::TERRAIN_NUMBER> > res;
	std::vector<terrain_translation::TERRAIN_NUMBER> inner_res;
	std::vector<std::string>::const_iterator iter = data.begin();

	// convert the string version to the TERRAIN_NUMBER version
	for(; iter != data.end(); ++iter){
		std::string::const_iterator inner_iter = iter->begin();
		for(; inner_iter != iter->begin(); ++inner_iter){
			inner_res.push_back(letter_to_number_(*inner_iter));
		}
		res.push_back(inner_res);
		inner_res.clear();
	}

	return res;
}

terrain_translation::TERRAIN_NUMBER terrain_translation::get_letter(const std::string& letter) const
{
	wassert(! letter.empty());
	return letter_to_number_(letter[0]);
}

std::vector<terrain_translation::TERRAIN_NUMBER> terrain_translation::get_list(const std::string& list, const int separated) const
{
	return string_to_vector_(list, false, separated);
}

std::vector<terrain_translation::TERRAIN_NUMBER> terrain_translation::get_map(const std::string& map) const
{
	return string_to_vector_(map, true, 0);
}

std::string terrain_translation::set_map(const std::vector<terrain_translation::TERRAIN_NUMBER>& map) const
{
	return vector_to_string_(map, 0);
}
	
std::string terrain_translation::set_letter(const terrain_translation::TERRAIN_NUMBER& letter) const
{
	// cheap hack reserve space to 1 char and put it in the string
	std::string res = "a";
	res[0] = number_to_letter_(letter);
	return res;
}

std::string terrain_translation::vector_to_string_(const std::vector<terrain_translation::TERRAIN_NUMBER>& map_data, const int separated) const
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

std::vector<terrain_translation::TERRAIN_NUMBER> terrain_translation::string_to_vector_(const std::string& data, const bool convert_eol, const int separated) const
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

std::string terrain_translation::set_list(const std::vector<terrain_translation::TERRAIN_NUMBER>& list, const int separated) const
{
	return vector_to_string_(list, separated);
}
