/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2.
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
#include "gettext.hpp"
#include "wassert.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>


terrain_type::terrain_type() : symbol_image_("void"),
			       number_(t_translation::VOID_TERRAIN),
			       mvt_type_(1, t_translation::VOID_TERRAIN),
			       def_type_(1, t_translation::VOID_TERRAIN),
			       union_type_(1, t_translation::VOID_TERRAIN),
                   height_adjust_(0), submerge_(0.0), light_modification_(0),
                   heals_(0), village_(false), castle_(false), keep_(false) 
{}

terrain_type::terrain_type(const config& cfg)
{
	symbol_image_ = cfg["symbol_image"];

	name_ = cfg["name"];
	id_ = cfg["id"];
	number_ = t_translation::read_letter(cfg["string"]);
	wassert(number_ != t_translation::NONE_TERRAIN);

	mvt_type_.push_back(number_);
	def_type_.push_back(number_);
	const t_translation::t_list& alias = t_translation::read_list(cfg["aliasof"]);
	if(!alias.empty()) {
		mvt_type_ = alias;
		def_type_ = alias;
	}

	const t_translation::t_list& mvt_alias = t_translation::read_list(cfg["mvt_alias"]);
	if(!mvt_alias.empty()) {
		mvt_type_ = mvt_alias;
	}

	const t_translation::t_list& def_alias = t_translation::read_list(cfg["def_alias"]);
	if(!def_alias.empty()) {
		def_type_ = def_alias;
	}
	union_type_ = mvt_type_;
	union_type_.insert( union_type_.end(), def_type_.begin(), def_type_.end() );

	// remove + and -
	union_type_.erase(std::remove(union_type_.begin(), union_type_.end(), 
				t_translation::MINUS), union_type_.end());

	union_type_.erase(std::remove(union_type_.begin(), union_type_.end(),
				t_translation::PLUS), union_type_.end());

	// remove doubles
	std::sort(union_type_.begin(),union_type_.end());
	union_type_.erase(std::unique(union_type_.begin(), union_type_.end()), union_type_.end());

	height_adjust_ = atoi(cfg["unit_height_adjust"].c_str());
	submerge_ = atof(cfg["submerge"].c_str());
#ifdef USE_TINY_GUI
	height_adjust_ /= 2;
#endif
	light_modification_ = atoi(cfg["light"].c_str());

	heals_ = lexical_cast_default<int>(cfg["heals"], 0);

	village_ = utils::string_bool(cfg["gives_income"]);
	castle_ = utils::string_bool(cfg["recruit_onto"]);
	keep_ = utils::string_bool(cfg["recruit_from"]);

	//mouse over message are only shown on villages
	if(village_) {
		income_description_ = cfg["income_description"];
		if(income_description_ == "") {
			income_description_ = _("Village");
		}

		income_description_ally_ = cfg["income_description_ally"];
		if(income_description_ally_ == "") {
			income_description_ally_ = _("Allied village");
		}

		income_description_enemy_ = cfg["income_description_enemy"];
		if(income_description_enemy_ == "") {
			income_description_enemy_ = _("Enemy village");
		}

		income_description_own_ = cfg["income_description_own"];
		if(income_description_own_ == "") {
			income_description_own_ = _("Owned village");
		}
	}

	editor_group_ = cfg["editor_group"]; 
}

void create_terrain_maps(const std::vector<config*>& cfgs,
                         t_translation::t_list& terrain_list,
                         std::map<t_translation::t_letter, terrain_type>& letter_to_terrain)
{
	for(std::vector<config*>::const_iterator i = cfgs.begin();
	    i != cfgs.end(); ++i) {
		terrain_type terrain(**i); 
		terrain_list.push_back(terrain.number()); 
		letter_to_terrain.insert(std::pair<t_translation::t_letter, terrain_type>(
		                              terrain.number(),terrain));
	}
}
