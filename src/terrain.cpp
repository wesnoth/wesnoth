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

#include "global.hpp"
#include "tstring.hpp"
#include "config.hpp"
#include "log.hpp"
#include "util.hpp"
#include "terrain.hpp"
#include "serialization/string_utils.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>


terrain_type::terrain_type() : symbol_image_("void"),
			       number_(terrain_translation::VOID_TERRAIN),
			       mvt_type2_(terrain_translation::VOID_TERRAIN),
			       def_type2_(terrain_translation::VOID_TERRAIN),
			       union_type2_(terrain_translation::VOID_TERRAIN),
                               height_adjust_(0), submerge_(0.0),
                               heals_(false), village_(false), castle_(false), keep_(false) 
{}

terrain_type::terrain_type(const config& cfg)
{
	symbol_image_ = cfg["symbol_image"];

	name_ = cfg["name"];
	id_ = cfg["id"];

	number_ = terrain_translation().get_letter(cfg["char"]);

	mvt_type2_.push_back(number_);
	def_type2_.push_back(number_);
	const std::vector<terrain_translation::TERRAIN_NUMBER>& alias = 
		terrain_translation().get_list(cfg["aliasof"]);
	if(!alias.empty()) {
		mvt_type2_ = alias;
		def_type2_ = alias;
	}
	const std::vector<terrain_translation::TERRAIN_NUMBER>& mvt_alias = 
		terrain_translation().get_list(cfg["mvt_alias"]);
	if(!mvt_alias.empty()) {
		mvt_type2_ = mvt_alias;
	}

	const std::vector<terrain_translation::TERRAIN_NUMBER>& def_alias = 
		terrain_translation().get_list(cfg["def_alias"]);
	if(!def_alias.empty()) {
		def_type2_ = def_alias;
	}
	union_type2_ = mvt_type2_;
	union_type2_.insert( union_type2_.end(), def_type2_.begin(), def_type2_.end() );

	// remove + and -
	union_type2_.erase(std::remove(union_type2_.begin(),union_type2_.end(),terrain_translation::MINUS),union_type2_.end());
	union_type2_.erase(std::remove(union_type2_.begin(),union_type2_.end(),terrain_translation::PLUS),union_type2_.end());
	// remove doubles
	std::sort(union_type2_.begin(),union_type2_.end());
	union_type2_.erase(std::unique(union_type2_.begin(),union_type2_.end()),union_type2_.end());

	height_adjust_ = atoi(cfg["unit_height_adjust"].c_str());
	submerge_ = atof(cfg["submerge"].c_str());
	light_modification_ = atoi(cfg["light"].c_str());

	if (cfg["heals"] == "true") {
		LOG_STREAM(err, config) << "terrain " << id() << " uses heals=true which is deprecated (use number)\n";
		heals_ = 8;
	} else {
		heals_ = lexical_cast_default<int>(cfg["heals"], 0);
	}
	village_ = utils::string_bool(cfg["gives_income"]);
	castle_ = utils::string_bool(cfg["recruit_onto"]);
	keep_ = utils::string_bool(cfg["recruit_from"]);
}

const std::string& terrain_type::symbol_image() const
{
	return symbol_image_;
}

const t_string& terrain_type::name() const
{
	return name_;
}

const std::string& terrain_type::id() const
{
	return id_;
}

terrain_translation::TERRAIN_NUMBER terrain_type::number() const
{
	return number_;
}

bool terrain_type::is_nonnull() const
{
	return (number_ != 0) && (number_ != terrain_translation::VOID_TERRAIN );
}

const std::vector<terrain_translation::TERRAIN_NUMBER>& terrain_type::mvt_type2() const
{
	return mvt_type2_;
}

const std::vector<terrain_translation::TERRAIN_NUMBER>& terrain_type::def_type2() const
{
	return def_type2_;
}

const std::vector<terrain_translation::TERRAIN_NUMBER>& terrain_type::union_type2() const
{
	return union_type2_;
}

int terrain_type::light_modification() const
{
	return light_modification_;
}

int terrain_type::unit_height_adjust() const
{
	return height_adjust_;
}

double terrain_type::unit_submerge() const
{
	return submerge_;
}

int terrain_type::gives_healing() const
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

void create_terrain_maps(const std::vector<config*>& cfgs,
                         std::vector<terrain_translation::TERRAIN_NUMBER>& terrain_list,
                         std::map<terrain_translation::TERRAIN_NUMBER,terrain_type>& letter_to_terrain,
                         std::map<std::string,terrain_type>& str_to_terrain)
{
	for(std::vector<config*>::const_iterator i = cfgs.begin();
	    i != cfgs.end(); ++i) {
		terrain_type terrain(**i); 
		terrain_list.push_back(terrain.number()); 
		letter_to_terrain.insert(std::pair<terrain_translation::TERRAIN_NUMBER,terrain_type>(
		                              terrain.number(),terrain));
		str_to_terrain.insert(std::pair<std::string,terrain_type>(
		                              terrain.id(),terrain));
	}
}
