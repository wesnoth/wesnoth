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
	//FIXME MdW remove obsolete part
#if 0	
	const std::string& letter = cfg["char"];

	if(letter == "") {
		letter_ = 0;
	} else {
		letter_ = letter[0];
	}
	
	//FIXME MdW what do the next 4 lines do?? guess nothing, since seems to be overwritten
	mvt_type_.resize(1);
	mvt_type_[0] = letter_;
	def_type_.resize(1);
	def_type_[0] = letter_;
	const std::string& alias = cfg["aliasof"];
	if(!alias.empty()) {
		mvt_type_ = alias;
		def_type_ = alias;
	}
	const std::string& mvt_alias = cfg["mvt_alias"];
	if(!mvt_alias.empty()) {
		mvt_type_ = mvt_alias;
	}

	const std::string& def_alias = cfg["def_alias"];
	if(!def_alias.empty()) {
		def_type_ = def_alias;
	}
	union_type_ = mvt_type_ +def_type_;
	union_type_.erase(std::remove(union_type_.begin(),union_type_.end(),'-'),union_type_.end());
	union_type_.erase(std::remove(union_type_.begin(),union_type_.end(),'+'),union_type_.end());
	std::sort(union_type_.begin(),union_type_.end());
	union_type_.erase(std::unique(union_type_.begin(),union_type_.end()),union_type_.end());
#endif	

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

	union_type2_.erase(std::remove(union_type2_.begin(),union_type2_.end(),terrain_translation::MINUS),union_type2_.end());
	union_type2_.erase(std::remove(union_type2_.begin(),union_type2_.end(),terrain_translation::PLUS),union_type2_.end());
	std::sort(union_type2_.begin(),union_type2_.end());
	union_type2_.erase(std::unique(union_type2_.begin(),union_type2_.end()),union_type2_.end());


#if 0	
	number_ = terrain_translation().get_letter(cfg["char"]);
	
	def_type2_ = terrain_translation().get_list(cfg["def_alias"]);
	mvt_type2_ = terrain_translation().get_list(cfg["mvt_alias"]);

	// if the move or defense alias isn't defined use the aliasof
	if(def_type2_.empty() || mvt_type2_.empty()){
		union_type2_ = terrain_translation().get_list(cfg["aliasof"]);
		if(def_type2_.empty()){
			def_type2_ = union_type2_;
		}
		if(mvt_type2_.empty()){
			mvt_type2_= union_type2_;
		}
		union_type2_.clear();
	}

	//FIXME MdW test the union
	union_type2_ = mvt_type2_; //+ def_type2_;
	union_type2_.insert( union_type2_.end(), def_type2_.begin(), def_type2_.end() );

	union_type2_.erase(std::remove(union_type2_.begin(),union_type2_.end(),terrain_translation::MINUS),union_type2_.end());
	union_type2_.erase(std::remove(union_type2_.begin(),union_type2_.end(),terrain_translation::PLUS),union_type2_.end());
	union_type2_.erase(std::unique(union_type2_.begin(),union_type2_.end()),union_type2_.end());
#endif /////
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

//FIXME MdW obsolete
#if 0
terrain_type::TERRAIN_LETTER terrain_type::letter() const
{
	return letter_;
}
#endif

terrain_translation::TERRAIN_NUMBER terrain_type::number() const
{
	return number_;
}

bool terrain_type::is_nonnull() const
{
	return (number_ != 0) && (number_ != terrain_translation::VOID_TERRAIN );
}

//FIXME MdW obsolete
#if 0
const std::string& terrain_type::mvt_type() const
{
	return mvt_type_;
}

const std::string& terrain_type::def_type() const
{
	return def_type_;
}

const std::string& terrain_type::union_type() const
{
	return union_type_;
}
#endif

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
#if 0
terrain_translation::TERRAIN_NUMBER terrain_type::load_terrain_char_(const config& cfg) const
{

	const std::string& letter = cfg["char"];
	//FIXME MdW is an empty letter allowed???
	if(letter == ""){
		return translator_.letter_to_number(0);
	} else {
		return translator_.letter_to_number(letter[0]);
	}

}

std::vector<terrain_translation::TERRAIN_NUMBER> terrain_type::load_terrain_alias_(const config& cfg, std::string alias) const
{
	const std::string& def_alias = cfg[alias];
	std::vector<terrain_translation::TERRAIN_NUMBER> result; 

	//NOTE MdW the + and - sign are special but they will converted to a proper sign
	std::string::const_iterator i = def_alias.begin();
	for( ; i != def_alias.end(); ++i) {
		result.push_back(translator_.letter_to_number(*i));
	}

	return result;
}
#endif
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
