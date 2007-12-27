/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "config.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "util.hpp"
#include "terrain.hpp"
#include "serialization/string_utils.hpp"
#include "tstring.hpp"
#include "wml_exception.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>


terrain_type::terrain_type() :
		minimap_image_("void"), 
		editor_image_("void"),
		id_(),
		name_(),
		number_(t_translation::VOID_TERRAIN),
		mvt_type_(1, t_translation::VOID_TERRAIN),
		def_type_(1, t_translation::VOID_TERRAIN),
		union_type_(1, t_translation::VOID_TERRAIN),
        height_adjust_(0), 
		submerge_(0.0), 
		light_modification_(0),
        heals_(0), 
		village_(false), 
		castle_(false), 
		keep_(false),
		income_description_(),
		income_description_ally_(),
		income_description_enemy_(),
		income_description_own_(),
		editor_group_()
{}

terrain_type::terrain_type(const config& cfg) :
		minimap_image_(cfg["symbol_image"]),
		editor_image_(cfg["editor_image"]),
		id_(cfg["id"]),
		name_(cfg["name"]),
		number_(t_translation::read_letter(cfg["string"])),
		mvt_type_(),
		def_type_(),
		union_type_(),
		height_adjust_(atoi(cfg["unit_height_adjust"].c_str())),
		submerge_(atof(cfg["submerge"].c_str())),
		light_modification_(atoi(cfg["light"].c_str())),
		heals_(lexical_cast_default<int>(cfg["heals"], 0)),
		village_(utils::string_bool(cfg["gives_income"])),
		castle_(utils::string_bool(cfg["recruit_onto"])),
		keep_(utils::string_bool(cfg["recruit_from"])),
		income_description_(),
		income_description_ally_(),
		income_description_enemy_(),
		income_description_own_(),
		editor_group_(cfg["editor_group"])
{
	VALIDATE(number_ != t_translation::NONE_TERRAIN, 
		missing_mandatory_wml_key("terrain", "string"));
	VALIDATE(!minimap_image_.empty(), 
		missing_mandatory_wml_key("terrain", "symbol_image", "string", 
		t_translation::write_letter(number_)));
	VALIDATE(!name_.empty(), 
		missing_mandatory_wml_key("terrain", "name", "string", 
		t_translation::write_letter(number_)));

	if(editor_image_.empty()) {
		editor_image_ = minimap_image_;
	}

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

#ifdef USE_TINY_GUI
	height_adjust_ /= 2;
#endif


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
