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
#include "wassert.hpp" // FIXME MdW only used for the terrain initalization

#include <algorithm>
#include <cstdlib>
#include <iostream>


terrain_type::terrain_type() : symbol_image_("void"),
			       number_(terrain_translation::VOID_TERRAIN),
			       mvt_type_(1, terrain_translation::VOID_TERRAIN),
			       def_type_(1, terrain_translation::VOID_TERRAIN),
			       union_type_(1, terrain_translation::VOID_TERRAIN),
                               height_adjust_(0), submerge_(0.0),
                               heals_(false), village_(false), castle_(false), keep_(false) 
{}

terrain_type::terrain_type(const config& cfg)
{
	symbol_image_ = cfg["symbol_image"];

	name_ = cfg["name"];
	id_ = cfg["id"];

#if 0	
	number_ = terrain_translation::read_letter(cfg["char"]); // FIXME MdW tag should read old format
#endif	
	
#ifdef TERRAIN_TRANSLATION_COMPATIBLE
	// load the old char and the new string part
	std::string terrain_char = cfg["char"];
	std::string terrain_string = cfg["string"];

	//this hack makes sure the string is defined, ugly but works
	//FIXME MdW this temp hack should be removed
//	if(terrain_string == "") {
//		terrain_string = "_ _" + terrain_char;
//	}
	wassert(terrain_string != "");
	
	number_ = terrain_translation::read_letter(terrain_string, terrain_translation::TFORMAT_STRING);
	//if both a char and a string are defined load it in the translation 
	//table. This to maintain backwards compability
	if(terrain_char != "") {
		terrain_translation::add_translation(terrain_char, number_);
	}
#else
	number_ = terrain_translation::read_letter(terrain_string, TFORMAT_STRING);
#endif


	mvt_type_.push_back(number_);
	def_type_.push_back(number_);
	const std::vector<terrain_translation::TERRAIN_NUMBER>& alias = 
		terrain_translation::read_list(cfg["aliasof"]); //FIXME MdW the aliasses should also be converted to new format
	if(!alias.empty()) {
		mvt_type_ = alias;
		def_type_ = alias;
	}
	const std::vector<terrain_translation::TERRAIN_NUMBER>& mvt_alias = 
		terrain_translation::read_list(cfg["mvt_alias"]);
	if(!mvt_alias.empty()) {
		mvt_type_ = mvt_alias;
	}

	const std::vector<terrain_translation::TERRAIN_NUMBER>& def_alias = 
		terrain_translation::read_list(cfg["def_alias"]);
	if(!def_alias.empty()) {
		def_type_ = def_alias;
	}
	union_type_ = mvt_type_;
	union_type_.insert( union_type_.end(), def_type_.begin(), def_type_.end() );

	// remove + and -
	union_type_.erase(std::remove(union_type_.begin(), union_type_.end(), 
				terrain_translation::MINUS), union_type_.end());

	union_type_.erase(std::remove(union_type_.begin(), union_type_.end(),
				terrain_translation::PLUS), union_type_.end());
	// remove doubles
	std::sort(union_type_.begin(),union_type_.end());
	union_type_.erase(std::unique(union_type_.begin(), union_type_.end()), union_type_.end());

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

const std::vector<terrain_translation::TERRAIN_NUMBER>& terrain_type::mvt_type() const
{
	return mvt_type_;
}

const std::vector<terrain_translation::TERRAIN_NUMBER>& terrain_type::def_type() const
{
	return def_type_;
}

const std::vector<terrain_translation::TERRAIN_NUMBER>& terrain_type::union_type() const
{
	return union_type_;
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
