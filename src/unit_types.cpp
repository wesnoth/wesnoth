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

#include "game_config.hpp"
#include "gettext.hpp"
#include "loadscreen.hpp"
#include "log.hpp"
#include "unit_types.hpp"
#include "util.hpp"
#include "wassert.hpp"
#include "serialization/string_utils.hpp"
#include "color_range.hpp"
#include "display.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>


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
		LOG_STREAM(err, config) << "the animation for " << cfg["name"] << "in unit " << id << " is directly in the attack, please use [animation]\n" ;
	}
	if(animation_.empty()) {
		animation_.push_back(attack_animation(cfg));
	}
	if(animation_.empty()) {
		animation_.push_back(attack_animation(-200,unit_frame(image_fighting,300)));
	}
	assert(!animation_.empty());

	id_ = cfg["name"];
	description_ = cfg["description"];
	if (description_.empty())
		description_ = egettext(id_.c_str());

	type_ = cfg["type"];
	icon_ = cfg["icon"];
	if(icon_.empty())
		icon_ = "attacks/" + id_ + ".png";

	range_ = cfg["range"].value();
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

const attack_animation& attack_type::animation(const display& disp, const gamemap::location& loc,const unit* my_unit,
		const fighting_animation::hit_type hit,const attack_type*secondary_attack,int swing_num,int damage) const
{
	//select one of the matching animations at random
	std::vector<const attack_animation*>  options;
	int max_val = -1;
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

	assert(!options.empty());
	return *options[rand()%options.size()];
}

bool attack_type::matches_filter(const config& cfg,bool self) const
{
	const std::vector<std::string>& filter_range = utils::split(cfg["range"]);
	const std::vector<std::string> filter_name = utils::split(cfg["name"]);
	const std::vector<std::string> filter_type = utils::split(cfg["type"]);
	const std::string filter_special = cfg["special"];

	if(filter_range.empty() == false && std::find(filter_range.begin(),filter_range.end(),range()) == filter_range.end())
			return false;

	if(filter_name.empty() == false && std::find(filter_name.begin(),filter_name.end(),name()) == filter_name.end())
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

	const t_string& set_name = cfg["set_name"];
	const std::string& set_type = cfg["set_type"];
	const std::string& del_specials = cfg["remove_specials"];
	const config* set_specials = cfg.child("set_specials");
	const std::string& increase_damage = cfg["increase_damage"];
	const std::string& increase_attacks = cfg["increase_attacks"];
	const std::string& set_attack_weight = cfg["attack_weight"];
	const std::string& set_defense_weight = cfg["defense_weight"];

	std::stringstream desc;

	if(set_name.empty() == false) {
		description_ = set_name;
		id_ = set_name;
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

	// v1.2 backward compatibility (should be removed in a few v1.3.x releases
	const std::string& set_special = cfg["set_special"];
	if(set_special.empty() == false) {
		lg::wml_error << "[effect] uses set_special=" << set_special <<", which is now deprecated. Use [set_specials] instead. Support will be removed in version 1.3.3\n";
		cfg_.clear_children("specials");
		config new_specials;
		if(set_special == "berserk") {
			config& sp = new_specials.add_child("berserk");
			sp["name"] = t_string("berserk","wesnoth");
			sp["description"] = t_string("Berserk:\nWhether used offensively or defensively, this attack presses the engagement until one of the combatants is slain, or 30 rounds of attacks have occurred.","wesnoth");
			sp["value"] = "30";
			sp["id"] = "berserk";
		} else if(set_special == "backstab") {
			config& sp = new_specials.add_child("damage");
			sp["name"] = t_string("backstab","wesnoth");
			sp["description"] = t_string("Backstab:\nThis attack deals double damage if there is an enemy of the target on the opposite side of the target, and that unit is not incapacitated (e.g. turned to stone).","wesnoth");
			sp["backstab"] = "yes";
			sp["multiply"] = "2";
			sp["active_on"] = "offense";
			sp["id"] = "backstab";
		} else if(set_special == "plague") {
			config& sp = new_specials.add_child("plague");
			sp["name"] = t_string("plague","wesnoth");
			sp["description"] = t_string("Plague:\nWhen a unit is killed by a Plague attack, that unit is replaced with a unit identical to and on the same side as the unit with the Plague attack. (This doesn't work on Undead or units in villages.)","wesnoth");
		} else if(set_special.substr(0,7) == "plague(") {
			config& sp = new_specials.add_child("plague");
			sp["name"] = t_string("plague","wesnoth") + set_special.substr(6);
			sp["description"] = t_string("Plague:\nWhen a unit is killed by a Plague attack, that unit is replaced with a unit of the specified type on the same side as the unit with the Plague attack. (This doesn't work on Undead or units in villages.)","wesnoth");
			sp["type"] = set_special.substr(7,set_special.size()-8);
		} else if(set_special == "swarm") {
			config& sp = new_specials.add_child("attacks");
			sp["name"] = t_string("swarm","wesnoth");
			sp["description"] = t_string("Swarm:\nThe number of strikes of this attack decreases when the unit is wounded. The number of strikes is proportional to the % of HP/maximum HP the unit has. For example a unit with 3/4 of its maximum HP will get 3/4 of the number of strikes.","wesnoth");
			sp["id"] = "swarm";
		} else if(set_special == "slow") {
			config& sp = new_specials.add_child("slow");
			sp["name"] = t_string("slows","wesnoth");
			sp["description"] = t_string("Slow:\nThis attack slows the target until it ends a turn. Slow halves the damage caused by attacks and slowed units move at half the normal speed (rounded up).","wesnoth");
		} else if(set_special == "stone") {
			config& sp = new_specials.add_child("stones");
			sp["name"] = t_string("stones","wesnoth");
			sp["description"] = t_string("Stone:\nThis attack turns the target to stone. Units that have been turned to stone may not move or attack.","wesnoth");
		} else if(set_special == "marksman") {
			config& sp = new_specials.add_child("chance_to_hit");
			sp["name"] = t_string("marksman","wesnoth");
			sp["description"] = t_string("Marksman:\nWhen used offensively, this attack always has at least a 60% chance to hit.","wesnoth");
			sp["value"] = "60";
			sp["cumulative"] = "yes";
			sp["active_on"] = "offense";
			sp["id"] = "marksman";
		} else if(set_special == "magical") {
			config& sp = new_specials.add_child("chance_to_hit");
			sp["name"] = t_string("magical","wesnoth");
			sp["description"] = t_string("Magical:\nThis attack always has a 70% chance to hit.","wesnoth");
			sp["value"] = "70";
			sp["cumulative"] = "no";
			sp["id"] = "magical";
		} else if(set_special == "charge") {
			config& sp = new_specials.add_child("damage");
			sp["name"] = t_string("charge","wesnoth");
			sp["description"] = t_string("Charge:\nThis attack deals double damage to the target. It also causes this unit to take double damage from the target's counterattack.","wesnoth");
			sp["multiply"] = "2";
			sp["active_on"] = "offense";
			sp["apply_to"] = "both";
			sp["id"] = "charge";
		} else if(set_special == "drain") {
			config& sp = new_specials.add_child("drains");
			sp["name"] = t_string("drains","wesnoth");
			sp["description"] = t_string("Drain:\nThis unit drains health from living units, healing itself for half the amount of damage it deals (rounded down).","wesnoth");
		} else if(set_special == "firststrike") {
			config& sp = new_specials.add_child("firststrike");
			sp["name"] = t_string("firststrike","wesnoth");
			sp["description"] = t_string("Firststrike:\nThis unit always strikes first with this attack, even if they are defending.","wesnoth");
		} else if(set_special == "poison") {
			config& sp = new_specials.add_child("poison");
			sp["name"] = t_string("poison","wesnoth");
			sp["description"] = t_string("Poison:\nThis attack poisons the target. Poisoned units lose 8 HP every turn until they are cured or are reduced to 1 HP.","wesnoth");
		}
		cfg_.add_child("specials",new_specials);
	} // end of backward compatibility block

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

	//if this is an alias, then select the best of all underlying terrains
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

	//if this is an alias, then select the best of all underlying terrains
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
      hide_help_(o.hide_help_), advances_to_(o.advances_to_),
      experience_needed_(o.experience_needed_), alignment_(o.alignment_),
      movementType_(o.movementType_), possibleTraits_(o.possibleTraits_),
      genders_(o.genders_), defensive_animations_(o.defensive_animations_),
      teleport_animations_(o.teleport_animations_), extra_animations_(o.extra_animations_),
      death_animations_(o.death_animations_), movement_animations_(o.movement_animations_),
      standing_animations_(o.standing_animations_),leading_animations_(o.leading_animations_),
      healing_animations_(o.healing_animations_), victory_animations_(o.victory_animations_),
      recruit_animations_(o.recruit_animations_), idle_animations_(o.idle_animations_),
      levelin_animations_(o.levelin_animations_), levelout_animations_(o.levelout_animations_),
      healed_animations_(o.healed_animations_), poison_animations_(o.poison_animations_),
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
	: cfg_(cfg), alpha_(ftofxp(1.0)), movementType_(cfg), possibleTraits_(traits)
{
	const config::child_list& variations = cfg.get_children("variation");
	for(config::child_list::const_iterator var = variations.begin(); var != variations.end(); ++var) {
		const config& var_cfg = **var;
		if(var_cfg["inherit"] == "yes") {
			config nvar_cfg = cfg;
			nvar_cfg = nvar_cfg.merge_with(var_cfg);
			nvar_cfg.clear_children("variation");
			variations_.insert(std::pair<std::string,unit_type*>(nvar_cfg["variation_name"],new unit_type(nvar_cfg,mv_types,races,traits)));
		} else {
			variations_.insert(std::pair<std::string,unit_type*>((**var)["variation_name"],new unit_type(**var,mv_types,races,traits)));
		}
	}

	gender_types_[0] = NULL;
	gender_types_[1] = NULL;

	const config* const male_cfg = cfg.child("male");
	if(male_cfg != NULL) {
		config m_cfg(cfg);
		if((*male_cfg)["inherit"]=="no") {
			m_cfg = *male_cfg;
		} else {
			m_cfg = m_cfg.merge_with(*male_cfg);
		}
		m_cfg.clear_children("male");
		m_cfg.clear_children("female");
		gender_types_[unit_race::MALE] = new unit_type(m_cfg,mv_types,races,traits);
	}

	const config* const female_cfg = cfg.child("female");
	if(female_cfg != NULL) {
		config f_cfg(cfg);
		if((*female_cfg)["inherit"]=="no") {
			f_cfg = *female_cfg;
		} else {
			f_cfg = f_cfg.merge_with(*female_cfg);
		}
		f_cfg.clear_children("male");
		f_cfg.clear_children("female");
		gender_types_[unit_race::FEMALE] = new unit_type(f_cfg,mv_types,races,traits);
	}

	const std::vector<std::string> genders = utils::split(cfg["gender"]);
	for(std::vector<std::string>::const_iterator i = genders.begin();
	    i != genders.end(); ++i) {
		if(*i == "male") {
			genders_.push_back(unit_race::MALE);
		} else if(*i == "female") {
			genders_.push_back(unit_race::FEMALE);
		}
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
				for(config::const_child_iterator i=traits.begin(); i != traits.end(); ++i)
				{
					if(alignment_ != NEUTRAL || ((**i)["id"]) != "fearless")
						possibleTraits_.push_back(*i);
				}
			}
		}
	} else {
		static const unit_race dummy_race;
		race_ = &dummy_race;
	}

	//insert any traits that are just for this unit type
	const config::child_list& unit_traits = cfg.get_children("trait");
	possibleTraits_.insert(possibleTraits_.end(),unit_traits.begin(),unit_traits.end());

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

	config expanded_cfg = unit_animation::prepare_animation(cfg,"defend");
	const config::child_list& defends = expanded_cfg.get_children("defend");
	for(config::child_list::const_iterator d = defends.begin(); d != defends.end(); ++d) {
		defensive_animations_.push_back(defensive_animation(**d));
	}
	if(defensive_animations_.empty()) {
		defensive_animations_.push_back(defensive_animation(-150,unit_frame(image(),300)));
		// always have a defensive animation
	}



	expanded_cfg = unit_animation::prepare_animation(cfg,"teleport_anim");
	const config::child_list& teleports = expanded_cfg.get_children("teleport_anim");
	for(config::child_list::const_iterator t = teleports.begin(); t != teleports.end(); ++t) {
		teleport_animations_.push_back(unit_animation(**t));
	}
	if(teleport_animations_.empty()) {
		teleport_animations_.push_back(unit_animation(-20,unit_frame(image(),40)));
		// always have a defensive animation
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
		// always have a defensive animation
	}

	expanded_cfg = unit_animation::prepare_animation(cfg,"movement_anim");
	const config::child_list& movement_anims = expanded_cfg.get_children("movement_anim");
	for(config::child_list::const_iterator movement_anim = movement_anims.begin(); movement_anim != movement_anims.end(); ++movement_anim) {
		movement_animations_.push_back(movement_animation(**movement_anim));
	}
	if(movement_animations_.empty()) {
		movement_animations_.push_back(movement_animation(0,unit_frame(image(),150)));
		// always have a movement animation
	}

	expanded_cfg = unit_animation::prepare_animation(cfg,"standing_anim");
	const config::child_list& standing_anims = expanded_cfg.get_children("standing_anim");
	for(config::child_list::const_iterator standing_anim = standing_anims.begin(); standing_anim != standing_anims.end(); ++standing_anim) {
		standing_animations_.push_back(standing_animation(**standing_anim));
	}
	if(standing_animations_.empty()) {
		standing_animations_.push_back(standing_animation(0,unit_frame(image(),0)));
		// always have a standing animation
	}

	expanded_cfg = unit_animation::prepare_animation(cfg,"leading_anim");
	const config::child_list& leading_anims = expanded_cfg.get_children("leading_anim");
	for(config::child_list::const_iterator leading_anim = leading_anims.begin(); leading_anim != leading_anims.end(); ++leading_anim) {
		leading_animations_.push_back(leading_animation(**leading_anim));
	}
	if(leading_animations_.empty()) {
		leading_animations_.push_back(leading_animation(0,unit_frame(image(),150)));
		// always have a leading animation
	}

	expanded_cfg = unit_animation::prepare_animation(cfg,"healing_anim");
	const config::child_list& healing_anims = expanded_cfg.get_children("healing_anim");
	for(config::child_list::const_iterator healing_anim = healing_anims.begin(); healing_anim != healing_anims.end(); ++healing_anim) {
		healing_animations_.push_back(healing_animation(**healing_anim));
	}
	if(healing_animations_.empty()) {
		healing_animations_.push_back(healing_animation(0,unit_frame(image::locator(cfg["image_healing"]),1,"1.0","",0,"",cfg["image_halo_healing"])));
		// always have a healing animation
	}

	expanded_cfg = unit_animation::prepare_animation(cfg,"recruit_anim");
	const config::child_list& recruit_anims = expanded_cfg.get_children("recruit_anim");
	for(config::child_list::const_iterator recruit_anim = recruit_anims.begin(); recruit_anim != recruit_anims.end(); ++recruit_anim) {
		recruit_animations_.push_back(recruit_animation(**recruit_anim));
	}
	if(recruit_animations_.empty()) {
		recruit_animations_.push_back(recruit_animation(0,unit_frame(image(),600,"0~1:600")));
		// always have a recruit animation
	}
	expanded_cfg = unit_animation::prepare_animation(cfg,"idle_anim");
	const config::child_list& idle_anims = expanded_cfg.get_children("idle_anim");
	for(config::child_list::const_iterator idle_anim = idle_anims.begin(); idle_anim != idle_anims.end(); ++idle_anim) {
		idle_animations_.push_back(idle_animation(**idle_anim));
	}
	if(idle_animations_.empty()) {
		idle_animations_.push_back(idle_animation(0,unit_frame(image(),1)));
		// always have a idle animation
	}
	expanded_cfg = unit_animation::prepare_animation(cfg,"victory_anim");
	const config::child_list& victory_anims = expanded_cfg.get_children("victory_anim");
	for(config::child_list::const_iterator victory_anim = victory_anims.begin(); victory_anim != victory_anims.end(); ++victory_anim) {
		victory_animations_.push_back(victory_animation(**victory_anim));
	}
	if(victory_animations_.empty()) {
		victory_animations_.push_back(victory_animation(0,unit_frame(image(),1)));
		// always have a victory animation
	}
	expanded_cfg = unit_animation::prepare_animation(cfg,"levelin_anim");
	const config::child_list& levelin_anims = expanded_cfg.get_children("levelin_anim");
	for(config::child_list::const_iterator levelin_anim = levelin_anims.begin(); levelin_anim != levelin_anims.end(); ++levelin_anim) {
		levelin_animations_.push_back(levelin_animation(**levelin_anim));
	}
	if(levelin_animations_.empty()) {
		levelin_animations_.push_back(levelin_animation(0,unit_frame(image(),600,"1.0","",display::rgb(255,255,255),"1~0:600")));
		// always have a levelin animation
	}
	expanded_cfg = unit_animation::prepare_animation(cfg,"levelout_anim");
	const config::child_list& levelout_anims = expanded_cfg.get_children("levelout_anim");
	for(config::child_list::const_iterator levelout_anim = levelout_anims.begin(); levelout_anim != levelout_anims.end(); ++levelout_anim) {
		levelout_animations_.push_back(levelout_animation(**levelout_anim));
	}
	if(levelout_animations_.empty()) {
		levelout_animations_.push_back(levelout_animation(0,unit_frame(image(),600,"1.0","",display::rgb(255,255,255),"0~1:600")));
		// always have a levelout animation
	}
	expanded_cfg = unit_animation::prepare_animation(cfg,"healed_anim");
	const config::child_list& healed_anims = expanded_cfg.get_children("healed_anim");
	for(config::child_list::const_iterator healed_anim = healed_anims.begin(); healed_anim != healed_anims.end(); ++healed_anim) {
		healed_animations_.push_back(healed_animation(**healed_anim));
	}
	if(healed_animations_.empty()) {
		healed_animations_.push_back(healed_animation(0,unit_frame(image(),240,"1.0","",display::rgb(255,255,255),"0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30,0.5:30")));
		// always have a healed animation
	}
	expanded_cfg = unit_animation::prepare_animation(cfg,"poison_anim");
	const config::child_list& poison_anims = expanded_cfg.get_children("poison_anim");
	for(config::child_list::const_iterator poison_anim = poison_anims.begin(); poison_anim != poison_anims.end(); ++poison_anim) {
		poison_animations_.push_back(poison_animation(**poison_anim));
	}
	if(poison_animations_.empty()) {
		poison_animations_.push_back(poison_animation(0,unit_frame(image(),240,"1.0","",display::rgb(0,255,0),"0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30,0.5:30")));
		// always have a poison animation
	}
	flag_rgb_ = cfg["flag_rgb"];
	game_config::add_color_info(cfg);
	// deprecation messages, only seen when unit is parsed for the first time

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
		id_ = cfg_["id"];

		if(id_.empty()) {
			// this code is only for compatibility with old unit defs and savefiles
			id_ = cfg_["name"];
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


void unit_type::add_advancement(const unit_type &to_unit,int xp)
{
	const std::string &to_id =  to_unit.cfg_["id"];
	const std::string &from_id =  cfg_["id"];

	// add extra advancement path to this unit type
	lg::info(lg::config) << "adding advancement from " << from_id << " to " << to_id << "\n";
	advances_to_.push_back(to_id);
	if(xp>0 && experience_needed_>xp) experience_needed_=xp;

	// add advancements to gendered subtypes, if supported by to_unit
	for(int gender=0; gender<=1; ++gender) {
		if(gender_types_[gender] == NULL) continue;
		if(to_unit.gender_types_[gender] == NULL) {
			lg::warn(lg::config) << to_id << " does not support gender " << gender << "\n";
			continue;
		}
		lg::info(lg::config) << "gendered advancement " << gender << ": ";
		gender_types_[gender]->add_advancement(*(to_unit.gender_types_[gender]),xp);
	}

	// add advancements to variation subtypes
	// since these are still a rare and special-purpose feature,
	// we assume that the unit designer knows what they're doing,
	// and don't block advancements that would remove a variation
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

	for(config::const_child_itors i = cfg.child_range("movetype");
	    i.first != i.second; ++i.first) {
		const unit_movement_type move_type(**i.first);
		movement_types.insert(
				std::pair<std::string,unit_movement_type>(move_type.name(),
						                                  move_type));
		increment_set_config_progress();
	}

	for(config::const_child_itors r = cfg.child_range("race");
	    r.first != r.second; ++r.first) {
		const unit_race race(**r.first);
		races.insert(std::pair<std::string,unit_race>(race.name(),race));
		increment_set_config_progress();
	}

	for(config::const_child_itors j = cfg.child_range("unit");
	    j.first != j.second; ++j.first) {
		const unit_type u_type(**j.first,movement_types,races,unit_traits);
		unit_types.insert(std::pair<std::string,unit_type>(u_type.id(),u_type));
		increment_set_config_progress();
	}

        // fix up advance_from references
        for(config::const_child_itors k = cfg.child_range("unit");
            k.first != k.second; ++k.first)
          for(config::const_child_itors af = (*k.first)->child_range("advancefrom");
            af.first != af.second; ++af.first) {
                const std::string &to = (**k.first)["id"];
                const std::string &from = (**af.first)["unit"];
                const int xp = lexical_cast_default<int>((**af.first)["experience"],0);

                unit_type_map::iterator from_unit = unit_types.find(from);
                unit_type_map::iterator to_unit = unit_types.find(to);
                if(from_unit==unit_types.end()) {
                  lg::warn(lg::config) << "unknown unit " << from << " in advancefrom\n";
                        continue;
                }
                wassert(to_unit!=unit_types.end());

                from_unit->second.add_advancement(to_unit->second,xp);
		increment_set_config_progress();
        }

}

void game_data::clear()
{
	movement_types.clear();
	unit_types.clear();
	races.clear();
}
