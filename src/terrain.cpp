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

#include "terrain.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>

terrain_type::terrain_type() : images_(1,"void"), letter_(' '), type_(" "),
                               height_adjust_(0), submerge_(0.0), equal_precedence_(false),
							   heals_(false), village_(false), castle_(false), keep_(false)
{}

terrain_type::terrain_type(const config& cfg)
{
	images_ = config::split(cfg["image"]);
	adjacent_image_ = cfg["adjacent_image"];
	if(adjacent_image_ == "" && images_.empty() == false)
		adjacent_image_ = images_.front();

	name_ = cfg["name"];
	const std::string& letter = cfg["char"];
	
	if(letter == "") {
		letter_ = 0;
	} else {
		letter_ = letter[0];
	}

	const std::string& alias = cfg["aliasof"];
	if(alias.empty()) {
		type_.resize(1);
		type_[0] = letter_;
	} else {
		type_ = alias;
	}

	colour_.read(cfg);

	height_adjust_ = atoi(cfg["unit_height_adjust"].c_str());
	submerge_ = atof(cfg["submerge"].c_str());

	equal_precedence_ = cfg["no_overlay"] == "true";
	is_light_ = cfg["light"] == "true";

	heals_ = cfg["heals"] == "true";
	village_ = cfg["gives_income"] == "true";
	castle_ = cfg["recruit_onto"] == "true";
	keep_ = cfg["recruit_from"] == "true";
}

const std::string& terrain_type::image(int x, int y) const
{
	assert(!images_.empty());

	return images_[(((x<<8)^3413402)+y^34984 + x*y)%images_.size()];
}

const std::string& terrain_type::default_image() const
{
	assert(!images_.empty());
	return images_.front();
}

const std::string& terrain_type::adjacent_image() const
{
	return adjacent_image_;
}

const std::string& terrain_type::name() const
{
	return name_;
}

char terrain_type::letter() const
{
	return letter_;
}

const std::string& terrain_type::type() const
{
	return type_;
}

pixel_data terrain_type::get_rgb() const
{
	return colour_;
}

bool terrain_type::is_light() const
{
	return is_light_;
}

bool terrain_type::is_alias() const
{
	return type_.size() != 1 || type_[0] != letter_;
}

int terrain_type::unit_height_adjust() const
{
	return height_adjust_;
}

double terrain_type::unit_submerge() const
{
	return submerge_;
}

bool terrain_type::equal_precedence() const
{
	return equal_precedence_;
}

bool terrain_type::gives_healing() const
{
	return heals_;
}

bool terrain_type::is_village() const
{
	return village_;
}

bool terrain_type::is_castle() const
{
	return castle_;
}

bool terrain_type::is_keep() const
{
	return keep_;
}

bool terrain_type::matches(const std::string &expression) const
{
	bool res = false;
	
	std::string types;
	bool negative = false;
	
	// If there is a wildcard in the string, it matches.
	if(expression.find('*') != std::string::npos)
		return true;
	
	if(expression[0] == '!') {
		types = expression.substr(1);
		negative = true;
	} else {
		types = expression;
	}

	if(types.find(letter_) != std::string::npos)
		res = true;
	
	if(negative)
		return !res;
	
	return res;
}

void create_terrain_maps(const std::vector<config*>& cfgs,
                         std::vector<char>& terrain_precedence,
                         std::map<char,terrain_type>& letter_to_terrain,
                         std::map<std::string,terrain_type>& str_to_terrain)
{
	for(std::vector<config*>::const_iterator i = cfgs.begin();
	    i != cfgs.end(); ++i) {
		terrain_type terrain(**i);
		terrain_precedence.push_back(terrain.letter());
		letter_to_terrain.insert(std::pair<char,terrain_type>(
		                              terrain.letter(),terrain));
		str_to_terrain.insert(std::pair<std::string,terrain_type>(
		                              terrain.name(),terrain));
	}
}
