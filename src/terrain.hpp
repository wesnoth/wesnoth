/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef TERRAIN_H_INCLUDED
#define TERRAIN_H_INCLUDED

class config;
#include "sdl_utils.hpp"

#include <map>
#include <string>
#include <vector>

class terrain_type
{
public:
	terrain_type();
	terrain_type(const config& cfg);

	const std::string& symbol_image() const;
	const std::string& name() const;
	const std::string& id() const;

	//the character representing this terrain
	char letter() const;

	//the underlying type of the terrain
	const std::string& type() const;

	pixel_data get_rgb() const;

	bool is_light() const;
	bool is_alias() const;

	int unit_height_adjust() const;
	double unit_submerge() const;

	bool gives_healing() const;
	bool is_village() const;
	bool is_castle() const;
	bool is_keep() const;

private:
	std::string symbol_image_;
	std::string id_;
	std::string name_;

	//the 'letter' is the letter that represents this
	//terrain type. The 'type' is a list of the 'underlying types'
	//of the terrain. This may simply be the same as the letter.
	char letter_;
	std::string type_;

	pixel_data colour_;

	int height_adjust_;

	double submerge_;

	bool is_light_;

	bool heals_, village_, castle_, keep_;
};

void create_terrain_maps(const std::vector<config*>& cfgs,
                         std::vector<char>& terrain_precedence,
                         std::map<char,terrain_type>& letter_to_terrain,
						 std::map<std::string,terrain_type>& str_to_terrain);

#endif
