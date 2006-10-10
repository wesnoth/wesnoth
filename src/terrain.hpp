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
#if 0	
	TERRAIN_LETTER letter() const;

	//the underlying type of the terrain
	const std::string& mvt_type() const;
	const std::string& def_type() const;
	const std::string& union_type() const;
#endif
	//the number representing this terrain
	//this is the new type
	terrain_translation::TERRAIN_NUMBER number() const;

	//FIXME MdW rename the ones below to the name of the ones above and kill the ones above
	//the underlying type of the terrain
	const std::vector<terrain_translation::TERRAIN_NUMBER>& mvt_type2() const;
	const std::vector<terrain_translation::TERRAIN_NUMBER>& def_type2() const;
	const std::vector<terrain_translation::TERRAIN_NUMBER>& union_type2() const;

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

	//the 'letter' is the letter that represents this
	//terrain type. The 'type' is a list of the 'underlying types'
	//of the terrain. This may simply be the same as the letter.
#if 0	
	TERRAIN_LETTER letter_;
	//These strings are no longer bound to contain chars
	//and only kept for backwards compability
	std::string mvt_type_;
	std::string def_type_;
	std::string union_type_;
#endif

	//the 'number' is the new way
	//NOTE the aliases stay strings for now
	//will become space separated list of numbers in the future
	terrain_translation::TERRAIN_NUMBER number_;
	//FIXME MdW rename these as soon as the originals are killed
	std::vector<terrain_translation::TERRAIN_NUMBER> mvt_type2_; 
	std::vector<terrain_translation::TERRAIN_NUMBER> def_type2_; 
	std::vector<terrain_translation::TERRAIN_NUMBER> union_type2_; 
		
	int height_adjust_;

	double submerge_;

	int light_modification_, heals_;

	bool village_, castle_, keep_;

	//FIXME MdW remove
	// loads the terrain number from the config file
	// we're the only one who should know what's in the
	// file. If the terrain system changes we should be
	// changed, the rest of the world not the old name 
	// is used since it's the known key
//	terrain_translation::TERRAIN_NUMBER load_terrain_char_(const config& cfg) const;
//	std::vector<terrain_translation::TERRAIN_NUMBER> load_terrain_alias_(const config& cfg, std::string alias) const;
};


void create_terrain_maps(const std::vector<config*>& cfgs,
                         std::vector<terrain_translation::TERRAIN_NUMBER>& terrain_list,
                         std::map<terrain_translation::TERRAIN_NUMBER,terrain_type>& letter_to_terrain,
                         std::map<std::string,terrain_type>& str_to_terrain);
#endif
