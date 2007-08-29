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

//! @file unit_types.cpp 
//! Handle unit-type specific attributes, animations, advancement.

#include "global.hpp"

#include "game_config.hpp"
#include "gettext.hpp"
#include "loadscreen.hpp"
#include "log.hpp"
#include "unit_types.hpp"
#include "util.hpp"
#include "wassert.hpp"
#include "serialization/string_utils.hpp"
#include "color_range.hpp"
#include "game_display.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>


//! The Halbardier got renamed so this function fixes that, all 
//! functions which assign id_ in unit or unit_types should use 
//! this wrapper in order to keep compatibility with older versions.
//
// @param id	the id of the unit
//
// @returns		the new id of the unit.
// 				if id == Halbardier it returns Halberdier, 
// 				otherwise id unmodified
std::string unit_id_test(const std::string& id)
{
	if(id == "Halbardier") {
		lg::wml_error << "'Halbardier' has been renamed to 'Halberdier' "
			"support for the old name will be removed in version 1.3.9\n";
		return "Halberdier";
	}

	return id;
}

attack_type::attack_type(const config& cfg,const std::string& id, const std::string& image_fighting)
{
	cfg_ = cfg;
	if(cfg["range"] == "long" || cfg["range"] == "ranged") {
		range_type_ = LONG_RANGE;
	} else {
		range_type_ = SHORT_RANGE;
	}
	const config expanded_cfg = unit_animation::prepare_animation(cfg,"animation");
	const config::child_list& animations = expanded_cfg.get_children("animation");
	for(config::child_list::const_iterator d = animations.begin(); d != animations.end(); ++d) {
		animation_.push_back(attack_animation(**d));
	}

	if(cfg.child("frame") || cfg.child("missile_frame") || cfg.child("sound")) {
		LOG_STREAM(err, config) << "the animation for " << cfg["name"] << "in unit " << id 
								<< " is directly in the attack, please use [animation]\n" ;
	}
	if(animation_.empty()) {
		animation_.push_back(attack_animation(cfg));
	}
	if(animation_.empty()) {
		animation_.push_back(attack_animation(-200,unit_frame(image_fighting,300)));
	}
	assert(!animation_.empty());

	id_ = unit_id_test(cfg["name"]);
	description_ = cfg["description"];
	if (description_.empty())
		description_ = egettext(id_.c_str());

	type_ = cfg["type"];
	icon_ = cfg["icon"];
	if(icon_.empty()){
		if (id_ != "")
			icon_ = "attacks/" + id_ + ".png";
		else
			icon_ = "attacks/blank-attack.png";
	}

	range_ = cfg["range"].base_str();
	damage_ = atol(cfg["damage"].c_str());
	num_attacks_ = atol(cfg["number"].c_str());

	attack_weight_ = lexical_cast_default<double>(cfg["attack_weight"],1.0);
	defense_weight_ = lexical_cast_default<double>(cfg["defense_weight"],1.0);

	gamedata_=NULL;
	unitmap_=NULL;
	map_=NULL;
	game_status_=NULL;
	teams_=NULL;
	other_attack_=NULL;

}

const attack_animation* attack_type::animation(const game_display& disp, const gamemap::location& loc,const unit* my_unit,
		const fighting_animation::hit_type hit,const attack_type*secondary_attack,int swing_num,int damage) const
{
	// Select one of the matching animations at random
	std::vector<const attack_animation*>  options;
	int max_val = -3;
	for(std::vector<attack_animation>::const_iterator i = animation_.begin(); i != animation_.end(); ++i) {
		int matching = i->matches(disp,loc,my_unit,hit,this,secondary_attack,swing_num,damage);
		if(matching == max_val) {
			options.push_back(&*i);
		} else if(matching > max_val) {
			max_val = matching;
			options.erase(options.begin(),options.end());
			options.push_back(&*i);
		}
	}

	if(options.empty()) return NULL;
	return options[rand()%options.size()];
}

bool attack_type::matches_filter(const config& cfg,bool self) const
{
	const std::vector<std::string>& filter_range = utils::split(cfg["range"]);
	const std::vector<std::string> filter_name = utils::split(cfg["name"]);
	const std::vector<std::string> filter_type = utils::split(cfg["type"]);
	const std::string filter_special = cfg["special"];

	if(filter_range.empty() == false && std::find(filter_range.begin(),filter_range.end(),range()) == filter_range.end())
			return false;

	if(filter_name.empty() == false && std::find(filter_name.begin(),filter_name.end(),id()) == filter_name.end())
		return false;

	if(filter_type.empty() == false && std::find(filter_type.begin(),filter_type.end(),type()) == filter_type.end())
		return false;

	if(!self && filter_special.empty() == false && !get_special_bool(filter_special,true))
		return false;

	return true;
}

bool attack_type::apply_modification(const config& cfg,std::string* description)
{
	if(!matches_filter(cfg,0))
		return false;

	const std::string& set_name = cfg["set_name"];
	const t_string& set_desc = cfg["set_description"];
	const std::string& set_type = cfg["set_type"];
	const std::string& del_specials = cfg["remove_specials"];
	const config* set_specials = cfg.child("set_specials");
	const std::string& increase_damage = cfg["increase_damage"];
	const std::string& increase_attacks = cfg["increase_attacks"];
	const std::string& set_attack_weight = cfg["attack_weight"];
	const std::string& set_defense_weight = cfg["defense_weight"];

	std::stringstream desc;

	if(set_name.empty() == false) {
		id_ = unit_id_test(set_name);
	}

	if(set_desc.empty() == false) {
		description_ = set_desc;
	}

	if(set_type.empty() == false) {
		type_ = set_type;
	}

	if(del_specials.empty() == false) {
		const std::vector<std::string>& dsl = utils::split(del_specials);
		config* specials = cfg_.child("specials");
		if (specials != NULL) {
			config new_specials;
			for(config::all_children_iterator s = specials->ordered_begin(); s != specials->ordered_end(); ++s) {
				const std::pair<const std::string*,const config*>& vp = *s;
				std::vector<std::string>::const_iterator found_id =
					std::find(dsl.begin(),dsl.end(),vp.second->get_attribute("id"));
				if (found_id == dsl.end()) {
					new_specials.add_child(*vp.first,*vp.second);
				}
			}
			cfg_.clear_children("specials");
			cfg_.add_child("specials",new_specials);
		}
	}

	if (set_specials) {
		const std::string& mode = set_specials->get_attribute("mode");
		if ( mode != "append") {
			cfg_.clear_children("specials");
		}
		config* new_specials = cfg_.child("specials");
		if (new_specials == NULL) {
			cfg_.add_child("specials");
			new_specials = cfg_.child("specials");
		}
		for(config::all_children_iterator s = set_specials->ordered_begin(); s != set_specials->ordered_end(); ++s) {
			const std::pair<const std::string*,const config*>& value = *s;
			new_specials->add_child(*value.first,*value.second);
		}
	}

	if(increase_damage.empty() == false) {
		damage_ = utils::apply_modifier(damage_, increase_damage, 1);

		if(description != NULL) {
			desc << (increase_damage[0] == '-' ? "" : "+") << increase_damage << " " << _("damage");
		}
	}

	if(increase_attacks.empty() == false) {
		num_attacks_ = utils::apply_modifier(num_attacks_, increase_attacks, 1);

		if(description != NULL) {
			desc << (increase_attacks[0] == '-' ? "" : "+") << increase_attacks << " " << _("strikes");
		}
	}

	if(set_attack_weight.empty() == false) {
		attack_weight_ = lexical_cast_default<double>(set_attack_weight,1.0);
	}

	if(set_defense_weight.empty() == false) {
		defense_weight_ = lexical_cast_default<double>(set_defense_weight,1.0);
	}

	if(description != NULL) {
		*description = desc.str();
	}

	return true;
}

// Same as above, except only update the descriptions
bool attack_type::describe_modification(const config& cfg,std::string* description)
{
	if(!matches_filter(cfg,0))
		return false;

	const std::string& increase_damage = cfg["increase_damage"];
	const std::string& increase_attacks = cfg["increase_attacks"];

	std::stringstream desc;

	if(increase_damage.empty() == false) {
		if(description != NULL) {
			desc << (increase_damage[0] == '-' ? "" : "+") << increase_damage << " " << _("damage");
		}
	}

	if(increase_attacks.empty() == false) {
		if(description != NULL) {
			desc << (increase_attacks[0] == '-' ? "" : "+") << increase_attacks << " " << _("strikes");
		}
	}

	if(description != NULL) {
		*description = desc.str();
	}

	return true;
}
bool attack_type::has_special_by_id(const std::string& special) const
{
	const config* abil = cfg_.child("specials");
	if(abil) {
		for(config::child_map::const_iterator i = abil->all_children().begin(); i != abil->all_children().end(); ++i) {
			for(config::child_list::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
				if((**j)["id"] == special) {
					return true;
				}
			}
		}
	}
	return false;
}

const t_string& unit_movement_type::name() const
{
	const t_string& res = cfg_["name"];
	if(res == "" && parent_ != NULL)
		return parent_->name();
	else
		return res;
}

int unit_movement_type::movement_cost(const gamemap& map, 
		t_translation::t_letter terrain, int recurse_count) const
{
	const int impassable = 10000000;

	const std::map<t_translation::t_letter, int>::const_iterator i = 
		moveCosts_.find(terrain);

	if(i != moveCosts_.end()) {
		return i->second;
	}

	// If this is an alias, then select the best of all underlying terrains.
	const t_translation::t_list& underlying = map.underlying_mvt_terrain(terrain);
	if(underlying.size() != 1 || underlying.front() != terrain) {
		bool revert = (underlying.front() == t_translation::MINUS ? true : false);
		if(recurse_count >= 100) {
			return impassable;
		}

		int ret_value = revert?0:impassable;
		for(t_translation::t_list::const_iterator i = underlying.begin(); 
				i != underlying.end(); ++i) {

			if(*i == t_translation::PLUS) {
				revert = false;
				continue;
			} else if(*i == t_translation::MINUS) {
				revert = true;
				continue;
			}
			const int value = movement_cost(map,*i,recurse_count+1);
			if(value < ret_value && !revert) {
				ret_value = value;
			} else if(value > ret_value && revert) {
				ret_value = value;
			}
		}

		moveCosts_.insert(std::pair<t_translation::t_letter, int>(terrain,ret_value));

		return ret_value;
	}

	const config* movement_costs = cfg_.child("movement_costs");

	int res = -1;

	if(movement_costs != NULL) {
		if(underlying.size() != 1) {
			LOG_STREAM(err, config) << "terrain '" << terrain << "' has " 
				<< underlying.size() << " underlying names - 0 expected\n";

			return impassable;
		}

		const std::string& id = map.get_terrain_info(underlying.front()).id();

		const std::string& val = (*movement_costs)[id];

		if(val != "") {
			res = atoi(val.c_str());
		}
	}

	if(res <= 0 && parent_ != NULL) {
		res = parent_->movement_cost(map,terrain);
	}

	if(res <= 0) {
		res = impassable;
	}

	moveCosts_.insert(std::pair<t_translation::t_letter, int>(terrain,res));

	return res;
}

int unit_movement_type::defense_modifier(const gamemap& map, 
		t_translation::t_letter terrain, int recurse_count) const
{
	const std::map<t_translation::t_letter, int>::const_iterator i = 
		defenseMods_.find(terrain);

	if(i != defenseMods_.end()) {
		return i->second;
	}

	// If this is an alias, then select the best of all underlying terrains.
	const t_translation::t_list& underlying = 
		map.underlying_def_terrain(terrain);

	if(underlying.size() != 1 || underlying.front() != terrain) {
		bool revert = (underlying.front() == t_translation::MINUS ? true : false);
		if(recurse_count >= 100) {
			return 100;
		}

		int ret_value = revert?0:100;
		for(t_translation::t_list::const_iterator i = underlying.begin(); 
				i != underlying.end(); ++i) {

			if(*i == t_translation::PLUS) {
				revert = false;
				continue;
			} else if(*i == t_translation::MINUS) {
				revert = true;
				continue;
			}
			const int value = defense_modifier(map,*i,recurse_count+1);
			if(value < ret_value && !revert) {
				ret_value = value;
			} else if(value > ret_value && revert) {
				ret_value = value;
			}
		}

		defenseMods_.insert(std::pair<t_translation::t_letter, int>(terrain, ret_value));

		return ret_value;
	}

	int res = -1;

	const config* const defense = cfg_.child("defense");

	if(defense != NULL) {
		if(underlying.size() != 1) {
			LOG_STREAM(err, config) << "terrain '" << terrain << "' has " 
				<< underlying.size() << " underlying names - 0 expected\n";

			return 100;
		}

		const std::string& id = map.get_terrain_info(underlying.front()).id();
		const std::string& val = (*defense)[id];

		if(val != "") {
			res = atoi(val.c_str());
		}
	}

	if(res <= 0 && parent_ != NULL) {
		res = parent_->defense_modifier(map,terrain);
	}

	if(res <= 0) {
		res = 50;
	}

	defenseMods_.insert(std::pair<t_translation::t_letter, int>(terrain, res));

	return res;
}

int unit_movement_type::resistance_against(const attack_type& attack) const
{
	bool result_found = false;
	int res = 0;

	const config* const resistance = cfg_.child("resistance");
	if(resistance != NULL) {
		const std::string& val = (*resistance)[attack.type()];
		if(val != "") {
			res = atoi(val.c_str());
			result_found = true;
		}
	}

	if(!result_found && parent_ != NULL) {
		res = parent_->resistance_against(attack);
	}

	return res;
}

string_map unit_movement_type::damage_table() const
{
	string_map res;
	if(parent_ != NULL)
		res = parent_->damage_table();

	const config* const resistance = cfg_.child("resistance");
	if(resistance != NULL) {
		for(string_map::const_iterator i = resistance->values.begin(); i != resistance->values.end(); ++i) {
			res[i->first] = i->second;
		}
	}

	return res;
}

bool unit_movement_type::is_flying() const
{
	const std::string& flies = cfg_["flies"];
	if(flies == "" && parent_ != NULL)
		return parent_->is_flying();

	return utils::string_bool(flies);
}

unit_type::unit_type(const unit_type& o)
    : variations_(o.variations_), cfg_(o.cfg_), race_(o.race_),
      alpha_(o.alpha_), abilities_(o.abilities_),ability_tooltips_(o.ability_tooltips_),
      hide_help_(o.hide_help_), advances_to_(o.advances_to_), advances_from_(o.advances_from_),
      experience_needed_(o.experience_needed_), alignment_(o.alignment_),
      movementType_(o.movementType_), possibleTraits_(o.possibleTraits_),
      genders_(o.genders_), animations_(o.animations_),

      defensive_animations_(o.defensive_animations_),
      teleport_animations_(o.teleport_animations_), extra_animations_(o.extra_animations_),
      death_animations_(o.death_animations_), 
      victory_animations_(o.victory_animations_),
      flag_rgb_(o.flag_rgb_)
{
	gender_types_[0] = o.gender_types_[0] != NULL ? new unit_type(*o.gender_types_[0]) : NULL;
	gender_types_[1] = o.gender_types_[1] != NULL ? new unit_type(*o.gender_types_[1]) : NULL;

	for(variations_map::const_iterator i = o.variations_.begin(); i != o.variations_.end(); ++i) {
		variations_[i->first] = new unit_type(*i->second);
	}
}


unit_type::unit_type(const config& cfg, const movement_type_map& mv_types,
                     const race_map& races, const std::vector<config*>& traits)
	: cfg_(cfg), alpha_(ftofxp(1.0)), movementType_(cfg)
{
	config::const_child_iterator i;
	for(i=traits.begin(); i != traits.end(); ++i)
	{
		possibleTraits_.add_child("trait", **i);
	}
	const config::child_list& variations = cfg.get_children("variation");
	for(config::child_list::const_iterator var = variations.begin(); var != variations.end(); ++var) {
		const config& var_cfg = **var;
		if(var_cfg["inherit"] == "yes") {
			config nvar_cfg(cfg);
			nvar_cfg.merge_with(var_cfg);
			nvar_cfg.clear_children("variation");
			variations_.insert(std::pair<std::string,unit_type*>(nvar_cfg["variation_name"],
												 new unit_type(nvar_cfg,mv_types,races,traits)));
		} else {
			variations_.insert(std::pair<std::string,unit_type*>((**var)["variation_name"],
												 new unit_type(**var,mv_types,races,traits)));
		}
	}

	gender_types_[0] = NULL;
	gender_types_[1] = NULL;

	const config* const male_cfg = cfg.child("male");
	if(male_cfg != NULL) {
		config m_cfg;
		if((*male_cfg)["inherit"]=="no") {
			m_cfg = *male_cfg;
		} else {
			m_cfg = cfg;
			m_cfg.merge_with(*male_cfg);
		}
		m_cfg.clear_children("male");
		m_cfg.clear_children("female");
		gender_types_[unit_race::MALE] = new unit_type(m_cfg,mv_types,races,traits);
	}

	const config* const female_cfg = cfg.child("female");
	if(female_cfg != NULL) {
		config f_cfg;
		if((*female_cfg)["inherit"]=="no") {
			f_cfg = *female_cfg;
		} else {
			f_cfg = cfg;
			f_cfg.merge_with(*female_cfg);
		}
		f_cfg.clear_children("male");
		f_cfg.clear_children("female");
		gender_types_[unit_race::FEMALE] = new unit_type(f_cfg,mv_types,races,traits);
	}

	const std::vector<std::string> genders = utils::split(cfg["gender"]);
	for(std::vector<std::string>::const_iterator g = genders.begin(); g != genders.end(); ++g) {
		genders_.push_back(string_gender(*g));
	}
	if(genders_.empty()) {
		genders_.push_back(unit_race::MALE);
	}

	const std::string& align = cfg_["alignment"];
	if(align == "lawful")
		alignment_ = LAWFUL;
	else if(align == "chaotic")
		alignment_ = CHAOTIC;
	else if(align == "neutral")
		alignment_ = NEUTRAL;
	else {
		LOG_STREAM(err, config) << "Invalid alignment found for " << id() << ": '" << align << "'\n";
		alignment_ = NEUTRAL;
	}

	const race_map::const_iterator race_it = races.find(cfg["race"]);
	if(race_it != races.end()) {
		race_ = &race_it->second;
		if(race_ != NULL) {
			if(race_->uses_global_traits() == false) {
				possibleTraits_.clear();
			}
			if(utils::string_bool(cfg["ignore_race_traits"])) {
				possibleTraits_.clear();
			} else {
				const config::child_list& traits = race_->additional_traits();
				for(i=traits.begin(); i != traits.end(); ++i)
				{
					if(alignment_ != NEUTRAL || ((**i)["id"]) != "fearless")
						possibleTraits_.add_child("trait", **i);
				}
			}
		}
	} else {
		static const unit_race dummy_race;
		race_ = &dummy_race;
	}

	// Insert any traits that are just for this unit type
	const config::child_list& unit_traits = cfg.get_children("trait");
	for(i=unit_traits.begin(); i != unit_traits.end(); ++i)
	{
		possibleTraits_.add_child("trait", **i);
	}

	const config* abil_cfg = cfg.child("abilities");
	if(abil_cfg) {
		const config::child_map& abi = abil_cfg->all_children();
		for(config::child_map::const_iterator j = abi.begin(); j != abi.end(); ++j) {
			for(config::child_list::const_iterator k = j->second.begin(); k != j->second.end(); ++k) {
				if((**k)["name"] != "") {
					abilities_.push_back((**k)["name"]);
					ability_tooltips_.push_back((**k)["description"]);
				}
			}
		}
	}

	if(cfg_["zoc"] == "") {
		zoc_ = lexical_cast_default<int>(cfg_["level"]) > 0;
	} else {
		zoc_ = false;
		if(utils::string_bool(cfg_["zoc"])) {
			zoc_ = true;
		}
	}


	const std::string& alpha_blend = cfg_["alpha"];
	if(alpha_blend.empty() == false) {
		alpha_ = ftofxp(atof(alpha_blend.c_str()));
	}

	const std::string& move_type = cfg_["movement_type"];

	const movement_type_map::const_iterator it = mv_types.find(move_type);

	if(it != mv_types.end()) {
		movementType_.set_parent(&(it->second));
	}

	const std::string& advance_to_val = cfg_["advanceto"];
	if(advance_to_val != "null" && advance_to_val != "")
		advances_to_ = utils::split(advance_to_val);

	experience_needed_=lexical_cast_default<int>(cfg_["experience"],500);

	config expanded_cfg = unit_animation::prepare_animation(cfg,"animation");
	const config::child_list& animations = expanded_cfg.get_children("animation");
	for(config::child_list::const_iterator d = animations.begin(); d != animations.end(); ++d) {
		animations_.push_back(unit_animation(**d));
	}
	expanded_cfg = unit_animation::prepare_animation(cfg,"leading_anim");
	const config::child_list& leading_anims = expanded_cfg.get_children("leading_anim");
	for(config::child_list::const_iterator leading_anim = leading_anims.begin(); leading_anim != leading_anims.end(); ++leading_anim) {
		(**leading_anim)["apply_to"] ="leading";
		animations_.push_back(unit_animation(**leading_anim));
		//lg::wml_error<<"leading animations  are deprecate, support will be removed in 1.3.8 (in unit "<<id()<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=leading flag\n";
	}
	animations_.push_back(unit_animation(0,unit_frame(image(),150),"leading",unit_animation::DEFAULT_ANIM));
	expanded_cfg = unit_animation::prepare_animation(cfg,"recruit_anim");
	const config::child_list& recruit_anims = expanded_cfg.get_children("recruit_anim");
	for(config::child_list::const_iterator recruit_anim = recruit_anims.begin(); recruit_anim != recruit_anims.end(); ++recruit_anim) {
		(**recruit_anim)["apply_to"] ="recruited";
		animations_.push_back(unit_animation(**recruit_anim));
		//lg::wml_error<<"recruit animations  are deprecate, support will be removed in 1.3.8 (in unit "<<id()<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=recruited flag\n";
	}
	animations_.push_back(unit_animation(0,unit_frame(image(),600,"0~1:600"),"recruited",unit_animation::DEFAULT_ANIM));
	expanded_cfg = unit_animation::prepare_animation(cfg,"standing_anim");
	const config::child_list& standing_anims = expanded_cfg.get_children("standing_anim");
	for(config::child_list::const_iterator standing_anim = standing_anims.begin(); standing_anim != standing_anims.end(); ++standing_anim) {
		(**standing_anim)["apply_to"] ="standing";
		animations_.push_back(unit_animation(**standing_anim));
		//lg::wml_error<<"standing animations  are deprecate, support will be removed in 1.3.8 (in unit "<<id()<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=standing flag\n";
	}
	// Always have a standing animation
	animations_.push_back(unit_animation(0,unit_frame(image(),0),"standing",unit_animation::DEFAULT_ANIM));
	expanded_cfg = unit_animation::prepare_animation(cfg,"idle_anim");
	const config::child_list& idle_anims = expanded_cfg.get_children("idle_anim");
	for(config::child_list::const_iterator idle_anim = idle_anims.begin(); idle_anim != idle_anims.end(); ++idle_anim) {
		(**idle_anim)["apply_to"] ="idling";
		animations_.push_back(unit_animation(**idle_anim));
		//lg::wml_error<<"idling animations  are deprecate, support will be removed in 1.3.8 (in unit "<<id()<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=idling flag\n";
	}
	// Idle anims can be empty
	expanded_cfg = unit_animation::prepare_animation(cfg,"levelin_anim");
	const config::child_list& levelin_anims = expanded_cfg.get_children("levelin_anim");
	for(config::child_list::const_iterator levelin_anim = levelin_anims.begin(); levelin_anim != levelin_anims.end(); ++levelin_anim) {
		(**levelin_anim)["apply_to"] ="levelin";
		animations_.push_back(unit_animation(**levelin_anim));
		//lg::wml_error<<"levelin animations  are deprecate, support will be removed in 1.3.8 (in unit "<<id()<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=levelin flag\n";
	}
	animations_.push_back(unit_animation(0,unit_frame(image(),600,"1.0","",display::rgb(255,255,255),"1~0:600"),"levelin",unit_animation::DEFAULT_ANIM));
	// Always have a levelin animation
	expanded_cfg = unit_animation::prepare_animation(cfg,"levelout_anim");
	const config::child_list& levelout_anims = expanded_cfg.get_children("levelout_anim");
	for(config::child_list::const_iterator levelout_anim = levelout_anims.begin(); levelout_anim != levelout_anims.end(); ++levelout_anim) {
		(**levelout_anim)["apply_to"] ="levelout";
		animations_.push_back(unit_animation(**levelout_anim));
		//lg::wml_error<<"levelout animations  are deprecate, support will be removed in 1.3.8 (in unit "<<id()<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=levelout flag\n";
	}
	animations_.push_back(unit_animation(0,unit_frame(image(),600,"1.0","",display::rgb(255,255,255),"0~1:600"),"levelin",unit_animation::DEFAULT_ANIM));
	// Always have a levelout animation
	expanded_cfg = unit_animation::prepare_animation(cfg,"healing_anim");
	const config::child_list& healing_anims = expanded_cfg.get_children("healing_anim");
	for(config::child_list::const_iterator healing_anim = healing_anims.begin(); healing_anim != healing_anims.end(); ++healing_anim) {
		(**healing_anim)["apply_to"] ="healing";
		(**healing_anim)["value"]=(**healing_anim)["damage"];
		animations_.push_back(unit_animation(**healing_anim));
		//lg::wml_error<<"healing animations  are deprecate, support will be removed in 1.3.8 (in unit "<<id()<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=healing flag\n";
	}
	animations_.push_back(unit_animation(0,unit_frame(image(),500),"healing",unit_animation::DEFAULT_ANIM));
	// Always have a healing animation
	expanded_cfg = unit_animation::prepare_animation(cfg,"healed_anim");
	const config::child_list& healed_anims = expanded_cfg.get_children("healed_anim");
	for(config::child_list::const_iterator healed_anim = healed_anims.begin(); healed_anim != healed_anims.end(); ++healed_anim) {
		(**healed_anim)["apply_to"] ="healed";
		(**healed_anim)["value"]=(**healed_anim)["healing"];
		animations_.push_back(unit_animation(**healed_anim));
		//lg::wml_error<<"healed animations  are deprecate, support will be removed in 1.3.8 (in unit "<<id()<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=healed flag\n";
	}
	animations_.push_back(unit_animation(0,unit_frame(image(),240,"1.0","",display::rgb(255,255,255),"0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30"),"healed",unit_animation::DEFAULT_ANIM));
	// Always have a healed animation
	expanded_cfg = unit_animation::prepare_animation(cfg,"poison_anim");
	const config::child_list& poison_anims = expanded_cfg.get_children("poison_anim");
	for(config::child_list::const_iterator poison_anim = poison_anims.begin(); poison_anim != poison_anims.end(); ++poison_anim) {
		(**poison_anim)["apply_to"] ="poison";
		(**poison_anim)["value"]=(**poison_anim)["damage"];
		animations_.push_back(unit_animation(**poison_anim));
		//lg::wml_error<<"poison animations  are deprecate, support will be removed in 1.3.8 (in unit "<<id()<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=poison flag\n";
	}
	animations_.push_back(unit_animation(0,unit_frame(image(),240,"1.0","",display::rgb(0,255,0),"0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30,0.5:30"),"poison",unit_animation::DEFAULT_ANIM));
	// Always have a poison animation 
	expanded_cfg = unit_animation::prepare_animation(cfg,"movement_anim");
	const config::child_list& movement_anims = expanded_cfg.get_children("movement_anim");
	for(config::child_list::const_iterator movement_anim = movement_anims.begin(); movement_anim != movement_anims.end(); ++movement_anim) {
		(**movement_anim)["apply_to"] ="movement";
		animations_.push_back(unit_animation(**movement_anim));
		//lg::wml_error<<"movement animations  are deprecate, support will be removed in 1.3.8 (in unit "<<id()<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=movement flag\n";
	}
	animations_.push_back(unit_animation(0,unit_frame(image(),150),"movement",unit_animation::DEFAULT_ANIM));
	// Always have a movement animation



	expanded_cfg = unit_animation::prepare_animation(cfg,"defend");
	const config::child_list& defends = expanded_cfg.get_children("defend");
	for(config::child_list::const_iterator d2 = defends.begin(); d2 != defends.end(); ++d2) {
		defensive_animations_.push_back(defensive_animation(**d2));
	}
	if(defensive_animations_.empty()) {
		defensive_animations_.push_back(defensive_animation(-150,unit_frame(image(),300)));
		// Always have a defensive animation
	}

	expanded_cfg = unit_animation::prepare_animation(cfg,"teleport_anim");
	const config::child_list& teleports = expanded_cfg.get_children("teleport_anim");
	for(config::child_list::const_iterator t = teleports.begin(); t != teleports.end(); ++t) {
		teleport_animations_.push_back(unit_animation(**t));
	}
	if(teleport_animations_.empty()) {
		teleport_animations_.push_back(unit_animation(-20,unit_frame(image(),40)));
		// Always have a defensive animation
	}
	expanded_cfg = unit_animation::prepare_animation(cfg,"extra_anim");
	const config::child_list& extra_anims = expanded_cfg.get_children("extra_anim");
	{
		for(config::child_list::const_iterator t = extra_anims.begin(); t != extra_anims.end(); ++t) {
			extra_animations_.insert(std::pair<std::string,unit_animation>((**t)["flag"],unit_animation(**t)));
		}
	}

	expanded_cfg = unit_animation::prepare_animation(cfg,"death");
	const config::child_list& deaths = expanded_cfg.get_children("death");
	for(config::child_list::const_iterator death = deaths.begin(); death != deaths.end(); ++death) {
		death_animations_.push_back(death_animation(**death));
	}
	if(death_animations_.empty()) {
		death_animations_.push_back(death_animation(0,unit_frame(image(),10)));
		// Always have a defensive animation
	}



	expanded_cfg = unit_animation::prepare_animation(cfg,"victory_anim");
	const config::child_list& victory_anims = expanded_cfg.get_children("victory_anim");
	for(config::child_list::const_iterator victory_anim = victory_anims.begin(); victory_anim != victory_anims.end(); ++victory_anim) {
		victory_animations_.push_back(victory_animation(**victory_anim));
	}
	if(victory_animations_.empty()) {
		victory_animations_.push_back(victory_animation(0,unit_frame(image(),1)));
		// Always have a victory animation
	}
	flag_rgb_ = cfg["flag_rgb"];
	game_config::add_color_info(cfg);
	// Deprecation messages, only seen when unit is parsed for the first time.

	hide_help_= cfg_["hide_help"] == "true" ? true : false;
}

unit_type::~unit_type()
{
	delete gender_types_[unit_race::MALE];
	delete gender_types_[unit_race::FEMALE];

	for(variations_map::iterator i = variations_.begin(); i != variations_.end(); ++i) {
		delete i->second;
	}
}

const unit_type& unit_type::get_gender_unit_type(unit_race::GENDER gender) const
{
	const size_t i = gender;
	if(i < sizeof(gender_types_)/sizeof(*gender_types_) && gender_types_[i] != NULL) {
		return *gender_types_[i];
	}

	return *this;
}

const unit_type& unit_type::get_variation(const std::string& name) const
{
	const variations_map::const_iterator i = variations_.find(name);
	if(i != variations_.end()) {
		return *i->second;
	} else {
		return *this;
	}
}

const std::string& unit_type::id() const
{
	if(id_.empty()) {
		id_ = unit_id_test(cfg_["id"]);

		if(id_.empty()) {
			// This code is only for compatibility with old unit defs and savefiles.
			id_ = unit_id_test(cfg_["name"]);
		}

		//id_.erase(std::remove(id_.begin(),id_.end(),' '),id_.end());
	}

	return id_;
}


#if 0
const std::string& unit_type::name() const
{
	return cfg_["id"];
}
#endif


const std::string& unit_type::image_fighting(attack_type::RANGE range) const
{
	static const std::string short_range("image_short");
	static const std::string long_range("image_long");

	const std::string& str = range == attack_type::LONG_RANGE ?
	                                  long_range : short_range;
	const std::string& val = cfg_[str];

	if(!val.empty())
		return val;
	else
		return image();
}


const std::string& unit_type::image_profile() const
{
	const std::string& val = cfg_["profile"];
	if(val.size() == 0)
		return image();
	else
		return val;
}

const t_string& unit_type::unit_description() const
{
	static const t_string default_val("No description available");

	const t_string& desc = cfg_["unit_description"];
	if(desc.empty())
		return default_val;
	else
		return desc;
}


std::vector<attack_type> unit_type::attacks() const
{
	std::vector<attack_type> res;
	for(config::const_child_itors range = cfg_.child_range("attack");
	    range.first != range.second; ++range.first) {
		res.push_back(attack_type(**range.first,id(),image_fighting((**range.first)["range"] == "ranged" ? attack_type::LONG_RANGE : attack_type::SHORT_RANGE)));
	}

	return res;
}


namespace {
	int experience_modifier = 100;
}

unit_type::experience_accelerator::experience_accelerator(int modifier) : old_value_(experience_modifier)
{
	experience_modifier = modifier;
}

unit_type::experience_accelerator::~experience_accelerator()
{
	experience_modifier = old_value_;
}

int unit_type::experience_accelerator::get_acceleration() 
{
	return experience_modifier;
}

int unit_type::experience_needed(bool with_acceleration) const
{
	if(with_acceleration) {
		int exp = (experience_needed_ * experience_modifier + 50) /100;
		if(exp < 1) exp = 1;
		return exp;
	}
	return experience_needed_;
}



const char* unit_type::alignment_description(unit_type::ALIGNMENT align)
{
	static const char* aligns[] = { N_("lawful"), N_("neutral"), N_("chaotic") };
	return (gettext(aligns[align]));
}

const char* unit_type::alignment_id(unit_type::ALIGNMENT align)
{
	static const char* aligns[] = { "lawful", "neutral", "chaotic" };
	return (aligns[align]);
}

bool unit_type::has_ability(const std::string& ability) const
{
	const config* abil = cfg_.child("abilities");
	if(abil) {
		return (abil->get_children(ability).size() > 0);
	}
	return false;
}
bool unit_type::has_ability_by_id(const std::string& ability) const
{
	const config* abil = cfg_.child("abilities");
	if(abil) {
		for(config::child_map::const_iterator i = abil->all_children().begin(); i != abil->all_children().end(); ++i) {
			for(config::child_list::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
				if((**j)["id"] == ability) {
					return true;
				}
			}
		}
	}
	return false;
}


const std::string& unit_type::race() const
{
	if(race_ == NULL) {
		static const std::string empty_string;
		return empty_string;
	}

	return race_->name();
}

// Allow storing "advances from" info for convenience in Help.
void unit_type::add_advancesfrom(const unit_type &from_unit)
{
	const std::string &from_id = from_unit.cfg_["id"];
	advances_from_.push_back(from_id);
}


void unit_type::add_advancement(const unit_type &to_unit,int xp)
{
	const std::string &to_id =  to_unit.cfg_["id"];
	const std::string &from_id =  cfg_["id"];

	// Add extra advancement path to this unit type
	lg::info(lg::config) << "adding advancement from " << from_id << " to " << to_id << "\n";
	advances_to_.push_back(to_id);
	if(xp>0 && experience_needed_>xp) experience_needed_=xp;

	// Add advancements to gendered subtypes, if supported by to_unit
	for(int gender=0; gender<=1; ++gender) {
		if(gender_types_[gender] == NULL) continue;
		if(to_unit.gender_types_[gender] == NULL) {
			lg::warn(lg::config) << to_id << " does not support gender " << gender << "\n";
			continue;
		}
		lg::info(lg::config) << "gendered advancement " << gender << ": ";
		gender_types_[gender]->add_advancement(*(to_unit.gender_types_[gender]),xp);
	}

	// Add advancements to variation subtypes.
	// Since these are still a rare and special-purpose feature,
	// we assume that the unit designer knows what they're doing,
	// and don't block advancements that would remove a variation.
	for(variations_map::iterator v=variations_.begin();
	    v!=variations_.end(); ++v) {
		lg::info(lg::config) << "variation advancement: ";
		v->second->add_advancement(to_unit,xp);
	}
}


game_data::game_data()
{}

game_data::game_data(const config& cfg)
{
	set_config(cfg);
}

void game_data::set_config(const config& cfg)
{
	static const std::vector<config*> dummy_traits;

	const config::child_list& unit_traits = cfg.get_children("trait");

	config::const_child_itors i;
	for(i = cfg.child_range("movetype"); i.first != i.second; ++i.first)
	{
		const unit_movement_type move_type(**i.first);
		movement_types.insert(
			std::pair<std::string,unit_movement_type>(move_type.name(), move_type));
		increment_set_config_progress();
	}

	for(i = cfg.child_range("race"); i.first != i.second; ++i.first) 
	{
		const unit_race race(**i.first);
		races.insert(std::pair<std::string,unit_race>(race.name(),race));
		increment_set_config_progress();
	}

	unsigned base_unit_count = 0;
	for(i = cfg.child_range("unit"); i.first != i.second; ++i.first) 
	{
		if((**i.first).child("base_unit"))
		{
			++base_unit_count;
		}
		else 
		{
			// LOAD UNIT TYPES
			const unit_type u_type(**i.first,movement_types,races,unit_traits);
			unit_types.insert(std::pair<std::string,unit_type>(u_type.id(),u_type));
			increment_set_config_progress();
		} 
	}
	while(base_unit_count > 0)
	{
		unsigned new_count = base_unit_count;
		std::string skipped;
		for(i = cfg.child_range("unit"); i.first != i.second; ++i.first) 
		{
			const config *bu_cfg = (**i.first).child("base_unit");
			if(bu_cfg)
			{
				const std::string &based_from = (*bu_cfg)["id"];
				unit_type_map::iterator from_unit = unit_types.find(based_from);
				if(from_unit != unit_types.end())
				{
					// Derive a new unit type from an existing base unit id
					config merge_cfg = merged_units.add_child(based_from, from_unit->second.cfg_);
					merge_cfg.merge_with(**i.first);
					merge_cfg.clear_children("base_unit");
					const unit_type u_type(merge_cfg,movement_types,races,unit_traits);
					unit_types.insert(std::pair<std::string,unit_type>(u_type.id(),u_type));
					increment_set_config_progress();
					--new_count;
				}
				else if(skipped.empty())
				{
					skipped = based_from;
				}
				else
				{
					skipped += ',' + based_from;
				}
			}
		}
		// If we iterate through the whole list and no work was done, an error has occurred
		if(new_count >= base_unit_count)
		{
			lg::warn(lg::config) << "unknown unit(s) " << skipped
				<< " specified in [base_unit] id(s)\n";
			break;
		}
		else
		{
			base_unit_count = new_count;
		}
	}
	
	// Fix up advance_from references
	for(i = cfg.child_range("unit"); i.first != i.second; ++i.first)
	{
		config::const_child_itors af;
		for(af = (*i.first)->child_range("advancefrom"); af.first != af.second; ++af.first)
		{
			const std::string &to = (**i.first)["id"];
			const std::string &from = (**af.first)["unit"];
			const int xp = lexical_cast_default<int>((**af.first)["experience"],0);

			unit_type_map::iterator from_unit = unit_types.find(from);
			unit_type_map::iterator to_unit = unit_types.find(to);
			if(from_unit==unit_types.end())
			{
				lg::warn(lg::config) << "unknown unit " << from << " in advancefrom\n";
				continue;
			}
			wassert(to_unit!=unit_types.end());

			from_unit->second.add_advancement(to_unit->second,xp);
			increment_set_config_progress();
		}
	}

	// For all unit types, store what units they advance from
	unit_type_map::iterator from_unit;
	for(from_unit = unit_types.begin(); from_unit != unit_types.end(); ++from_unit)
	{
		std::vector<std::string> to_units_ids = from_unit->second.advances_to();
		std::vector<std::string>::iterator to_unit_id;
		for(to_unit_id = to_units_ids.begin(); to_unit_id != to_units_ids.end(); ++to_unit_id)
		{
			unit_type_map::iterator to_unit = unit_types.find(*to_unit_id);
			if(to_unit != unit_types.end())
			{
				to_unit->second.add_advancesfrom(from_unit->second);
			}
			else
			{
				lg::warn(lg::config) << "unknown unit " << *to_unit_id 
					<< " advanced to by unit " << from_unit->first << "\n";
			}
		}
	}
}

void game_data::clear()
{
	movement_types.clear();
	unit_types.clear();
	merged_units.clear();
	races.clear();
}

// This function is only meant to return the likely state of not_living
// for a new recruit of this type. It should not be used to check if
// a particular unit is living or not, use get_state("not_living") for that.

bool unit_type::not_living() const
{
		// If a unit hasn't been modified it starts out as living.
		bool not_living = false;

		// Look at all of the "musthave" traits to see if the not_living
		// status gets changed. In the unlikely event it gets changed
		// multiple times, we want to try to do it in the same order
		// that unit::apply_modifications does things.
		config::child_list const &mods = possible_traits();
		config::child_list::const_iterator j, j_end = mods.end();
		for(j = mods.begin(); j != j_end; ++j) {
			const string_map *vals = &((**j).values);
			string_map::const_iterator temp = vals->find("availability");
			if (temp == vals->end() || (*temp).second != "musthave") {
				continue;
			}
			config::const_child_itors i;
			for(i = (**j).child_range("effect"); i.first != i.second; ++i.first) {

				// See if the effect only applies to certain unit types
				// But don't worry about gender checks, since we don't
				// know what the gender of the hypothetical recruit is.
				vals = &((**i.first).values);
				temp = vals->find("unit_type");
				if(temp != vals->end()) {
					const std::vector<std::string>& types = utils::split((*temp).second);
					if(std::find(types.begin(),types.end(),id()) == types.end()) {
							continue;
					}
				}

				// We're only interested in status changes.
				temp = vals->find("apply_to");
				if (temp == vals->end() || (*temp).second != "status") {
					continue;
				}
				temp = vals->find("add");
				if (temp != vals->end() && (*temp).second == "not_living") {
					not_living = true;
				}
				temp = vals->find("remove");
				if (temp != vals->end() && (*temp).second == "not_living") {
					not_living = false;
				}
			}
		}

		return not_living;
}
