/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef TERRAIN_H_INCLUDED
#define TERRAIN_H_INCLUDED

class config;
#include "tstring.hpp"
#include "terrain_translation.hpp"

#include <map>
#include <string>
#include <vector>

class terrain_type
{
public:

	terrain_type();
	terrain_type(const config& cfg);

	const std::string& symbol_image() const;
	const t_string& name() const;
	const std::string& id() const;

	//the character representing this terrain
	//this is the old type will be obsoleted
	//FIXME MdW maybe reinstall the letter later again after debugging everything
//	TERRAIN_LETTER letter() const;
//	
	//the number representing this terrain
	//this is the new type
	terrain_translation::TERRAIN_NUMBER number() const;

	//the underlying type of the terrain
	const std::vector<terrain_translation::TERRAIN_NUMBER>& mvt_type() const;
	const std::vector<terrain_translation::TERRAIN_NUMBER>& def_type() const;
	const std::vector<terrain_translation::TERRAIN_NUMBER>& union_type() const;

	bool is_nonnull() const;
	int light_modification() const;

	int unit_height_adjust() const;
	double unit_submerge() const;

	int gives_healing() const;
	bool is_village() const;
	bool is_castle() const;
	bool is_keep() const;

private:
	std::string symbol_image_;
	std::string id_;
	t_string name_;

	//the 'number' is the number that represents this
	//terrain type. The 'type' is a list of the 'underlying types'
	//of the terrain. This may simply be the same as the number.
	//This is the internal number used, WML still used characters
	terrain_translation::TERRAIN_NUMBER number_;
	std::vector<terrain_translation::TERRAIN_NUMBER> mvt_type_;
	std::vector<terrain_translation::TERRAIN_NUMBER> def_type_;
	std::vector<terrain_translation::TERRAIN_NUMBER> union_type_;
		
	int height_adjust_;

	double submerge_;

	int light_modification_, heals_;

	bool village_, castle_, keep_;

};


void create_terrain_maps(const std::vector<config*>& cfgs,
                         std::vector<terrain_translation::TERRAIN_NUMBER>& terrain_list,
                         std::map<terrain_translation::TERRAIN_NUMBER,terrain_type>& letter_to_terrain,
                         std::map<std::string,terrain_type>& str_to_terrain);
#endif
