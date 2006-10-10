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
//include "variable.hpp"
#include "serialization/string_utils.hpp"
#include "wassert.hpp"

#define SHIFT 0

const terrain_translation::TERRAIN_NUMBER terrain_translation::VOID_TERRAIN = ' ' << SHIFT;
const terrain_translation::TERRAIN_NUMBER terrain_translation::FOGGED = '~' << SHIFT;
const terrain_translation::TERRAIN_NUMBER terrain_translation::KEEP = 'K' << SHIFT;

const terrain_translation::TERRAIN_NUMBER terrain_translation::CASTLE = 'C' << SHIFT;
const terrain_translation::TERRAIN_NUMBER terrain_translation::SHALLOW_WATER = 'c' << SHIFT;
const terrain_translation::TERRAIN_NUMBER terrain_translation::DEEP_WATER = 's' << SHIFT;
const terrain_translation::TERRAIN_NUMBER terrain_translation::GRASS_LAND = 'g' << SHIFT;

const terrain_translation::TERRAIN_NUMBER terrain_translation::CAVE_WALL = 'W' << SHIFT;
const terrain_translation::TERRAIN_NUMBER terrain_translation::CAVE = 'u' << SHIFT;
const terrain_translation::TERRAIN_NUMBER terrain_translation::UNDERGROUND_VILLAGE = 'D' << SHIFT;
const terrain_translation::TERRAIN_NUMBER terrain_translation::DWARVEN_CASTLE = 'o' << SHIFT;

const terrain_translation::TERRAIN_NUMBER terrain_translation::PLUS = '+' << SHIFT;
const terrain_translation::TERRAIN_NUMBER terrain_translation::MINUS = '-' << SHIFT;
const terrain_translation::TERRAIN_NUMBER terrain_translation::STAR = '*' << SHIFT;
const terrain_translation::TERRAIN_NUMBER terrain_translation::NOT = '!' << SHIFT;
const terrain_translation::TERRAIN_NUMBER terrain_translation::EOL = 7 << SHIFT; // char 7 is the bell so no EOL
const terrain_translation::TERRAIN_NUMBER terrain_translation::DOT = '.' << SHIFT;
const terrain_translation::TERRAIN_NUMBER terrain_translation::COMMA = ',' << SHIFT;

terrain_translation::terrain_translation() {}
terrain_translation::~terrain_translation(){}

terrain_translation::TERRAIN_LETTER terrain_translation::letter_to_number_(const terrain_translation::TERRAIN_NUMBER terrain) const
{
	TERRAIN_NUMBER tmp = terrain >> SHIFT;
	return (TERRAIN_LETTER)(tmp);
}

terrain_translation::TERRAIN_NUMBER terrain_translation::number_to_letter_(const terrain_translation::TERRAIN_LETTER terrain) const
{
	TERRAIN_NUMBER result = (TERRAIN_NUMBER) terrain;
	result = result << SHIFT;
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
 
std::vector<std::vector<terrain_translation::TERRAIN_NUMBER> > terrain_translation::get_terrain_vector_splitted(const config& cfg, const std::string tag) const
{
	//for now use the standard splitter
	const std::vector<std::string> data = utils::split(cfg[tag]);
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
/*
std::vector<terrain_translation::TERRAIN_NUMBER> terrain_translation::string_to_vector(const std::string map_data) const
{
	const std::string& data = map_data;
	return terrain_translation::string_to_vector_(data, false);

}
*/



//NEW will stay
terrain_translation::TERRAIN_NUMBER terrain_translation::get_letter(const std::string& letter) const
{
	wassert(! letter.empty());
	return letter_to_number_(letter[0]);
}

//NEW will stay
std::vector<terrain_translation::TERRAIN_NUMBER> terrain_translation::get_list(const std::string& list, const int separated) const
{
	return string_to_vector_(list, false, separated);
}

//NEW will stay
std::vector<terrain_translation::TERRAIN_NUMBER> terrain_translation::get_map(const std::string& map) const
{
	//FIXME MdW remove assigment, only used as debug aid
	std::vector<terrain_translation::TERRAIN_NUMBER> result = string_to_vector_(map, true, 0);
	return result;
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

//USED private
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

//USED private
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
#if 0		
		if(convert_eol){
			if(*itor == '\n' || *itor == '\r'){
				// end of line marker found
				if(last_eol == false){
					// last wasn't eol then add us
					result.push_back(EOL);
				}
				//else we're ignored
			}else{
				last_eol = false;
				result.push_back(letter_to_number(*itor));
			}
		} else {
			// no EOL conversion just pushback
			result.push_back(letter_to_number(*itor));
		}
	}
#endif
	//note 2 exit path for debugging aids
	if(result.empty()) {
		return result;
	} else {
		return result;
	}
		

}

std::string terrain_translation::set_list(const std::vector<terrain_translation::TERRAIN_NUMBER>& list, const int separated) const
{
	return vector_to_string_(list, separated);
}
