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

#include "config.hpp"
#include "sdl_utils.hpp"

#include <string>

class terrain_type
{
public:
	terrain_type();
	terrain_type(const config& cfg);

	const std::string& image(int x, int y) const;
	const std::string& default_image() const;
	const std::string& adjacent_image() const;
	const std::string& name() const;
	char letter() const;
	char type() const;

	pixel_data get_rgb() const;

	bool is_alias() const;

	int unit_height_adjust() const;
	double unit_submerge() const;

	//whether the terrain's overlay precedence is equal (rather than higher
	//than) the preceeding terrain
	bool equal_precedence() const;
private:
	std::vector<std::string> images_;
	std::string adjacent_image_;
	std::string name_;

	//the 'letter' is the letter that represents this
	//terrain type. The 'type' is the letter of the
	//terrain type which this is equivalent to, which
	//may be the same as 'letter'
	char type_, letter_;

	pixel_data colour_;

	int height_adjust_;

	double submerge_;

	bool equal_precedence_;
};

void create_terrain_maps(const std::vector<config*>& cfgs,
                         std::vector<char>& terrain_precedence,
                         std::map<char,terrain_type>& letter_to_terrain,
						 std::map<std::string,terrain_type>& str_to_terrain);

#endif
