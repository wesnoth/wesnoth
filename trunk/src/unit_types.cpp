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
#include "log.hpp"
#include "unit_types.hpp"
#include "util.hpp"
#include "wassert.hpp"
#include "serialization/string_utils.hpp"
#include "color_range.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>


attack_type::attack_type(const config& cfg,const unit_type& unit)
{
	if(cfg["range"] == "long" || cfg["range"] == "ranged") {
		range_type_ = LONG_RANGE;
	} else {
		range_type_ = SHORT_RANGE;
	}
	const config::child_list& animations = cfg.get_children("animation");
	for(config::child_list::const_iterator d = animations.begin(); d != animations.end(); ++d) {
		animation_.push_back(attack_animation(**d));
	}

	if(cfg.child("frame") || cfg.child("missile_frame") || cfg.child("sound")) {
		LOG_STREAM(err, config) << "the animation for " << cfg["name"] << "in unit " << unit.id() << " is directly in the attack, please use [animation]\n" ;
	}
	if(animation_.empty()) {
		animation_.push_back(attack_animation(cfg));
	}
	if(animation_.empty()) {
		animation_.push_back(attack_animation(unit.image_fighting(range_type_)));
	}
	assert(!animation_.empty());

	id_ = cfg["name"];
	description_ = cfg["description"];
	if (description_.empty())
		description_ = egettext(id_.c_str());

	type_ = cfg["type"];
	special_ = cfg["special"];
	backstab_ = special_ == "backstab";
	slow_ = special_ == "slow";
	icon_ = cfg["icon"];
	if(icon_.empty())
		icon_ = "attacks/" + id_ + ".png";

	range_ = cfg["range"];
	damage_ = atol(cfg["damage"].c_str());
	num_attacks_ = atol(cfg["number"].c_str());

	attack_weight_ = lexical_cast_default<double>(cfg["attack_weight"],1.0);
	defense_weight_ = lexical_cast_default<double>(cfg["defense_weight"],1.0);
}

const t_string& attack_type::name() const
{
	return description_;
}

const std::string& attack_type::id() const
{
	return id_;
}

const std::string& attack_type::type() const
{
	return type_;
}

const std::string& attack_type::special() const
{
	return special_;
}

const std::string& attack_type::icon() const
{
	return icon_;
}

attack_type::RANGE attack_type::range_type() const
{
	return range_type_;
}

const std::string& attack_type::range() const
{
	return range_;
}

int attack_type::damage() const
{
	return damage_;
}

int attack_type::num_attacks() const
{
	return num_attacks_;
}

int attack_type::num_swarm_attacks(int hp, int maxhp) const
{
  if(special() == "swarm"){
    return (num_attacks_ - (num_attacks_ * (maxhp-hp) / maxhp));
  }else{
    return (num_attacks_);
  }
}

double attack_type::attack_weight() const
{
	return attack_weight_;
}

double attack_type::defense_weight() const
{
	return defense_weight_;
}

bool attack_type::backstab() const
{
	return backstab_;
}

bool attack_type::slow() const
{
	return slow_;
}

const std::pair<const unit_animation*,const unit_animation*> attack_type::animation(bool hit,const gamemap::location::DIRECTION dir) const
{
	//select one of the matching animations at random
	std::vector<std::pair<const unit_animation*,const unit_animation*> > options;
	int max_val = -1;
	for(std::vector<attack_animation>::const_iterator i = animation_.begin(); i != animation_.end(); ++i) {
		int matching = i->matches(hit,dir);
		if(matching == max_val) {
			options.push_back(std::pair<const unit_animation*,const unit_animation*>(&i->animation,&i->missile_animation));
		} else if(matching > max_val) {
			max_val = matching;
			options.erase(options.begin(),options.end());
			options.push_back(std::pair<const unit_animation*,const unit_animation*>(&i->animation,&i->missile_animation));
		}
	}

	assert(!options.empty());
	return options[rand()%options.size()];
}
attack_type::attack_animation::attack_animation(const config& cfg):animation(cfg),missile_animation(cfg,"missile_frame"),hits(HIT_OR_MISS)
{
	const std::vector<std::string>& my_directions = utils::split(cfg["direction"]);
	for(std::vector<std::string>::const_iterator i = my_directions.begin(); i != my_directions.end(); ++i) {
		const gamemap::location::DIRECTION d = gamemap::location::parse_direction(*i);
		directions.push_back(d);
	}
	if(missile_animation.get_first_frame_time() == 0 && missile_animation.get_last_frame_time() == 0) {
		// create animation ourself
		missile_animation = unit_animation(game_config::missile_n_image,-100,0,game_config::missile_ne_image);
	}
	const std::string& hits_str = cfg["hits"];
	if(hits_str.empty() == false) {
		hits = (hits_str == "yes") ? HIT : MISS;
	}
	assert(missile_animation.get_first_frame_time() != 0 || missile_animation.get_last_frame_time() != 0);
}
int attack_type::attack_animation::matches(bool hit,gamemap::location::DIRECTION dir) const
{
	int result = 0;

	if(directions.empty()== false) {
		if (std::find(directions.begin(),directions.end(),dir)== directions.end()) {
			return -1;
		} else {
			result ++;
		}
	}
	if(hits != HIT_OR_MISS ) {
		if(hit && (hits == HIT)) {
			result++;
		} else if(!hit && (hits == MISS)) {
			result++;
		} else {
			return -1;
		}
	}

	return result;
}
bool attack_type::matches_filter(const config& cfg) const
{
	const std::string& filter_range = cfg["range"];
	const t_string& filter_name = cfg["name"];
	const std::string& filter_type = cfg["type"];
	const std::string& filter_special = cfg["special"];

	if(filter_range.empty() == false && filter_range != range())
			return false;

	if(filter_name.empty() == false && filter_name != name())
		return false;

	if(filter_type.empty() == false && filter_type != type())
		return false;

	if(filter_special.empty() == false && filter_special != special())
		return false;

	return true;
}

bool attack_type::apply_modification(const config& cfg, std::string* description)
{
	if(!matches_filter(cfg))
		return false;

	const t_string& set_name = cfg["set_name"];
	const std::string& set_type = cfg["set_type"];
	const std::string& set_special = cfg["set_special"];
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

	if(set_special.empty() == false) {
		special_ = set_special;
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

unit_movement_type::unit_movement_type(const config& cfg, const unit_movement_type* parent)
             : cfg_(cfg), parent_(parent)
{}

const t_string& unit_movement_type::name() const
{
	const t_string& res = cfg_["name"];
	if(res == "" && parent_ != NULL)
		return parent_->name();
	else
		return res;
}

int unit_movement_type::movement_cost(const gamemap& map,gamemap::TERRAIN terrain,int recurse_count) const
{
	const int impassable = 10000000;

	const std::map<gamemap::TERRAIN,int>::const_iterator i = moveCosts_.find(terrain);
	if(i != moveCosts_.end()) {
		return i->second;
	}

	//if this is an alias, then select the best of all underlying terrains
	const std::string& underlying = map.underlying_mvt_terrain(terrain);
	if(underlying.size() != 1 || underlying[0] != terrain) {
		bool revert = (underlying[0] == '-'?true:false);
		if(recurse_count >= 100) {
			return impassable;
		}

		int ret_value = revert?0:impassable;
		for(std::string::const_iterator i = underlying.begin(); i != underlying.end(); ++i) {
			if(*i == '+') {
				revert = false;
				continue;
			} else if(*i == '-') {
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

		moveCosts_.insert(std::pair<gamemap::TERRAIN,int>(terrain,ret_value));

		return ret_value;
	}

	const config* movement_costs = cfg_.child("movement_costs");

	int res = -1;

	if(movement_costs != NULL) {
		if(underlying.size() != 1) {
			LOG_STREAM(err, config) << "terrain '" << terrain << "' has " << underlying.size() << " underlying names - 0 expected\n";
			return impassable;
		}

		const std::string& id = map.get_terrain_info(underlying[0]).id();

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

	moveCosts_.insert(std::pair<gamemap::TERRAIN,int>(terrain,res));

	return res;
}

int unit_movement_type::defense_modifier(const gamemap& map,gamemap::TERRAIN terrain, int recurse_count) const
{
	const std::map<gamemap::TERRAIN,int>::const_iterator i = defenseMods_.find(terrain);
	if(i != defenseMods_.end()) {
		return i->second;
	}

	//if this is an alias, then select the best of all underlying terrains
	const std::string& underlying = map.underlying_mvt_terrain(terrain);
	if(underlying.size() != 1 || underlying[0] != terrain) {
		bool revert = (underlying[0] == '-'?true:false);
		if(recurse_count >= 100) {
			return 100;
		}

		int ret_value = revert?0:100;
		for(std::string::const_iterator i = underlying.begin(); i != underlying.end(); ++i) {
			if(*i == '+') {
				revert = false;
				continue;
			} else if(*i == '-') {
				revert = true;
				continue;
			}
			const int value = defense_modifier(map,*i,recurse_count+1);
			if(value < ret_value && !revert) {
				ret_value = value;
			} else if(value > ret_value && revert) {
				ret_value = value;
			}
			if(value < ret_value) {
				ret_value = value;
			}
		}

		defenseMods_.insert(std::pair<gamemap::TERRAIN,int>(terrain,ret_value));

		return ret_value;
	}

	int res = -1;

	const config* const defense = cfg_.child("defense");

	if(defense != NULL) {
		if(underlying.size() != 1) {
			LOG_STREAM(err, config) << "terrain '" << terrain << "' has " << underlying.size() << " underlying names - 0 expected\n";
			return 100;
		}

		const std::string& id = map.get_terrain_info(underlying[0]).id();
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

	defenseMods_.insert(std::pair<gamemap::TERRAIN,int>(terrain,res));

	return res;
}

int unit_movement_type::damage_against(const attack_type& attack) const
{
	return resistance_against(attack);
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

	return flies == "true";
}

void unit_movement_type::set_parent(const unit_movement_type* parent)
{
	parent_ = parent;
}

ability_filter::ability_filter()
{
	// we add a null string to prevent the filter to be empty
	terrain_filter_chaotic.push_back("");
	terrain_filter_neutral.push_back("");
	terrain_filter_lawful.push_back("");
}

bool ability_filter::matches_filter(const std::string& terrain, int lawful_bonus) const
{		
	const std::vector<std::string>* terrain_filter;
	if (lawful_bonus < 0) {
		terrain_filter = &terrain_filter_chaotic;
	} else if (lawful_bonus == 0) {
		terrain_filter = &terrain_filter_neutral;
	} else {
		terrain_filter = &terrain_filter_lawful;
	}		
		
	if (terrain_filter->empty()) {
		return true;
	} else {
		for (std::string::const_iterator i = terrain.begin(); i != terrain.end(); ++i) {
			std::string t(1,*i);
			if (std::find(terrain_filter->begin(),terrain_filter->end(),t) != terrain_filter->end())
				return true;
		}
		return false;
	}
}

void ability_filter::unfilter()
{
	terrain_filter_chaotic.clear();
	terrain_filter_neutral.clear();
	terrain_filter_lawful.clear();		
}

void ability_filter::add_terrain_filter(const std::string& terrains)
{
	std::vector<std::string> add_to_filter = utils::split(terrains);
	for (std::vector<std::string>::const_iterator t = add_to_filter.begin(); t != add_to_filter.end(); ++t) {
		terrain_filter_chaotic.push_back(*t);
		terrain_filter_neutral.push_back(*t);
		terrain_filter_lawful.push_back(*t);
	}
}

void ability_filter::add_tod_filter(const std::string& times)
{
	std::vector<std::string> add_to_filter = utils::split(times);
	for (std::vector<std::string>::const_iterator t = add_to_filter.begin(); t != add_to_filter.end(); ++t) {
		if (*t == "chaotic") {
			terrain_filter_chaotic.clear();
		} else if (*t == "neutral") {
			terrain_filter_neutral.clear();
		} else if (*t == "lawful") {
			terrain_filter_lawful.clear();		
		} else {
			LOG_STREAM(err, config) << "Unknown time of day type : " << *t << "\n";
		}
	}
}
	
void ability_filter::add_filters(const config* cfg)
{
	if (cfg) {
		std::string tods =(*cfg)["tod"];
		std::string terrains =(*cfg)["terrains"];
		if (tods == "" && terrains == "") {
			unfilter();
			return;
		} else if (tods == "") {
			add_terrain_filter(terrains);
			return;
		} else if (terrains == "") {
			add_tod_filter(tods);
			return;
		} else {
			std::vector<std::string> tod_slices = utils::split(tods);
			for (std::vector<std::string>::const_iterator td = tod_slices.begin(); td != tod_slices.end(); ++td) {
				std::vector<std::string>* terrain_filter;
				if (*td == "chaotic") {
					terrain_filter= &terrain_filter_chaotic;
				} else if (*td == "neutral") {
					terrain_filter= &terrain_filter_neutral;
				} else if (*td == "lawful") {
					terrain_filter= &terrain_filter_lawful;
				} else {
					LOG_STREAM(err, config) << "Unknown time of day type : " << *td << "\n";
					continue;
				}
				std::vector<std::string> terrain_slices = utils::split(terrains);
				for (std::vector<std::string>::const_iterator te = terrain_slices.begin(); te != terrain_slices.end(); ++te) {
					terrain_filter->push_back(*te);
				}
			}
		}
	} else {
		unfilter();
	}
}

unit_type::unit_type(const unit_type& o)
    : variations_(o.variations_), cfg_(o.cfg_), race_(o.race_),
      alpha_(o.alpha_), abilities_(o.abilities_),ability_tooltips_(o.ability_tooltips_),
      heals_filter_(o.heals_filter_), max_heals_(o.max_heals_), heals_(o.heals_), 
      regenerates_filter_(o.regenerates_filter_),regenerates_(o.regenerates_),
      regeneration_(o.regeneration_),
      leadership_filter_(o.leadership_filter_), leadership_(o.leadership_),
      leadership_percent_(o.leadership_percent_),
      illuminates_filter_(o.illuminates_filter_), illuminates_(o.illuminates_),
      skirmisher_filter_(o.skirmisher_filter_), skirmish_(o.skirmish_),
      teleports_filter_(o.teleports_filter_), teleport_(o.teleport_),
      steadfast_filter_(o.steadfast_filter_), steadfast_(o.steadfast_),
      steadfast_bonus_(o.steadfast_bonus_),steadfast_max_(o.steadfast_max_),
      hides_filter_(o.hides_filter_), hides_(o.hides_),
      advances_to_(o.advances_to_), experience_needed_(o.experience_needed_),
      alignment_(o.alignment_),
      movementType_(o.movementType_), possibleTraits_(o.possibleTraits_),
      genders_(o.genders_), defensive_animations_(o.defensive_animations_),
      teleport_animations_(o.teleport_animations_), extra_animations_(o.extra_animations_),
      death_animations_(o.death_animations_), movement_animations_(o.movement_animations_),
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
		variations_.insert(std::pair<std::string,unit_type*>((**var)["variation_name"],new unit_type(**var,mv_types,races,traits)));
	}

	gender_types_[0] = NULL;
	gender_types_[1] = NULL;

	const config* const male_cfg = cfg.child("male");
	if(male_cfg != NULL) {
		config m_cfg(cfg);
		m_cfg = m_cfg.merge_with(*male_cfg);
		m_cfg.clear_children("male");
		m_cfg.clear_children("female");
		gender_types_[unit_race::MALE] = new unit_type(m_cfg,mv_types,races,traits);
	}

	const config* const female_cfg = cfg.child("female");
	if(female_cfg != NULL) {
		config f_cfg(cfg);
		f_cfg = f_cfg.merge_with(*female_cfg);
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

	const race_map::const_iterator race_it = races.find(cfg["race"]);
	if(race_it != races.end()) {
		race_ = &race_it->second;
		if(race_ != NULL) {
			if(race_->uses_global_traits() == false) {
				possibleTraits_.clear();
			}
			
			if(cfg["ignore_race_traits"] == "yes") {
				possibleTraits_.clear();
			} else {
				const config::child_list& traits = race_->additional_traits();
				possibleTraits_.insert(possibleTraits_.end(),traits.begin(),traits.end());
			}
		}
	} else {
		static const unit_race dummy_race;
		race_ = &dummy_race;
	}

	//insert any traits that are just for this unit type
	const config::child_list& unit_traits = cfg.get_children("trait");
	possibleTraits_.insert(possibleTraits_.end(),unit_traits.begin(),unit_traits.end());

	heals_ = 0;
	max_heals_ = 0;
	regenerates_ = false;
	regeneration_ = 0;
	steadfast_ = false;
	steadfast_bonus_ = 0;
	steadfast_max_ = 0;
	skirmish_ = false;
	teleport_ = false;
	illuminates_ = 0;
	leadership_ = false;
	hides_ = false;

	/* handle deprecated ability=x,y,... */

	std::vector<std::string> deprecated_abilities = utils::split(cfg_["ability"]);

	//if the string was empty, split will give us one empty string in the list,
	//remove it.
	if(!deprecated_abilities.empty() && deprecated_abilities.back() == "") {
		deprecated_abilities.pop_back();
	}
	
	if(!deprecated_abilities.empty()) {
		LOG_STREAM(err, config) << "unit " << id() << " uses the ability=list tag, which is deprecated\n";
		if(std::find(deprecated_abilities.begin(),deprecated_abilities.end(),"heals") != deprecated_abilities.end()) {
			heals_ = game_config::healer_heals_per_turn;
			max_heals_ = game_config::heal_amount;
			heals_filter_.unfilter();
			abilities_.push_back("heals");
			ability_tooltips_.push_back("heals");
		}
		if(std::find(deprecated_abilities.begin(),deprecated_abilities.end(),"cures") != deprecated_abilities.end()) {
			heals_ = game_config::curer_heals_per_turn;
			max_heals_ = game_config::cure_amount;
			heals_filter_.unfilter();
			abilities_.push_back("cures");
			ability_tooltips_.push_back("cures");
		}
		if(std::find(deprecated_abilities.begin(),deprecated_abilities.end(),"regenerates") != deprecated_abilities.end()) {
			regenerates_ = true;
			regeneration_ = game_config::cure_amount;
			regenerates_filter_.unfilter();
			abilities_.push_back("regenerates");
			ability_tooltips_.push_back("regenerates");
		}
		if(std::find(deprecated_abilities.begin(),deprecated_abilities.end(),"steadfast") != deprecated_abilities.end()) {
			steadfast_ = true;
			steadfast_bonus_ = 100;
			steadfast_max_ = 50;
			steadfast_percent_ = true;
			steadfast_filter_.unfilter();
			abilities_.push_back("steadfast");
			ability_tooltips_.push_back("steadfast");
		}
		if(std::find(deprecated_abilities.begin(),deprecated_abilities.end(),"teleport") != deprecated_abilities.end()) {
			teleport_ = true;
			teleports_filter_.unfilter();
			abilities_.push_back("teleport");
			ability_tooltips_.push_back("teleport");
		}
		if(std::find(deprecated_abilities.begin(),deprecated_abilities.end(),"skirmisher") != deprecated_abilities.end()) {
			skirmish_ = true;
			skirmisher_filter_.unfilter();
			abilities_.push_back("skirmisher");
			ability_tooltips_.push_back("skirmisher");
		}
		if(std::find(deprecated_abilities.begin(),deprecated_abilities.end(),"leadership") != deprecated_abilities.end()) {
			leadership_ = true;
			leadership_percent_ = game_config::leadership_bonus;
			leadership_filter_.unfilter();
			abilities_.push_back("leadership");
			ability_tooltips_.push_back("leadership");
		}
		if(std::find(deprecated_abilities.begin(),deprecated_abilities.end(),"illuminates") != deprecated_abilities.end()) {
			illuminates_ = 1;
			illuminates_filter_.unfilter();
			abilities_.push_back("illuminates");
			ability_tooltips_.push_back("illuminates");
		}
		if(std::find(deprecated_abilities.begin(),deprecated_abilities.end(),"ambush") != deprecated_abilities.end()) {
			hides_ = true;
			hides_filter_.add_terrain_filter("f");
			abilities_.push_back("ambush");
			ability_tooltips_.push_back("ambush");
		}
		if(std::find(deprecated_abilities.begin(),deprecated_abilities.end(),"nightstalk") != deprecated_abilities.end()) {
			hides_ = true;
			hides_filter_.add_tod_filter("chaotic");
			abilities_.push_back("nightstalk");
			ability_tooltips_.push_back("nightstalk");
		}
	}

	const config* abil_cfg = cfg.child("abilities");
	if(abil_cfg) {
		const config::child_list& heal_abilities = abil_cfg->get_children("heals");
		if (!heal_abilities.empty()) {
			abilities_.push_back("heals");
			for(config::child_list::const_iterator ab = heal_abilities.begin(); ab != heal_abilities.end(); ++ab) {
				if((**ab)["description"] != "") {
					ability_tooltips_.push_back((**ab)["description"]);
				} else {
					ability_tooltips_.push_back("heals");
				}
				heals_ = maximum<int>(heals_, lexical_cast_default<int>((**ab)["amount"],game_config::healer_heals_per_turn));
				max_heals_ = maximum<int>(max_heals_, lexical_cast_default<int>((**ab)["max"],game_config::heal_amount));
				heals_filter_.add_filters((*ab)->child("filter"));
			}
		}
		const config::child_list& regenerate_abilities = abil_cfg->get_children("regenerates");
		if (!regenerate_abilities.empty()) {
			abilities_.push_back("regenerates");
			for(config::child_list::const_iterator ab = regenerate_abilities.begin(); ab != regenerate_abilities.end(); ++ab) {
				if((**ab)["description"] != "") {
					ability_tooltips_.push_back((**ab)["description"]);
				} else {
					ability_tooltips_.push_back("regenerates");
				}
				regenerates_ = true;
				regeneration_ = maximum<int>(regeneration_, lexical_cast_default<int>((**ab)["amount"],game_config::cure_amount));
				regenerates_filter_.add_filters((*ab)->child("filter"));
			}
		}
		const config::child_list& steadfast_abilities = abil_cfg->get_children("steadfast");
		if (!steadfast_abilities.empty()) {
			abilities_.push_back("steadfast");
			for(config::child_list::const_iterator ab = steadfast_abilities.begin(); ab != steadfast_abilities.end(); ++ab) {
				if((**ab)["description"] != "") {
					ability_tooltips_.push_back((**ab)["description"]);
				} else {
					ability_tooltips_.push_back("steadfast");
				}
				steadfast_ = true;
				steadfast_bonus_ = maximum<int>(steadfast_bonus_,lexical_cast_default<int>((**ab)["bonus"],100));
				steadfast_max_ = maximum<int>(steadfast_max_,lexical_cast_default<int>((**ab)["max"],50));
				std::string steadfast_ispercent = (**ab)["bonus"];
				if(steadfast_ispercent != "" && steadfast_ispercent[steadfast_ispercent.size()-1] == '%') {
					steadfast_percent_ = true;
				} else {
					steadfast_percent_ = false;
				}
				steadfast_filter_.add_filters((*ab)->child("filter"));
			}
		}
		const config::child_list& leadership_abilities = abil_cfg->get_children("leadership");
		if (!leadership_abilities.empty()) {
			abilities_.push_back("leadership");
			for(config::child_list::const_iterator ab = leadership_abilities.begin(); ab != leadership_abilities.end(); ++ab) {
				if((**ab)["description"] != "") {
					ability_tooltips_.push_back((**ab)["description"]);
				} else {
					ability_tooltips_.push_back("leadership");
				}
				leadership_ = true;
				leadership_percent_ = lexical_cast_default<int>((**ab)["perlevel_bonus"],game_config::leadership_bonus);
				leadership_filter_.add_filters((*ab)->child("filter"));
			}
		}
		const config::child_list& skirmisher_abilities = abil_cfg->get_children("skirmisher");
		if (!skirmisher_abilities.empty()) {
			abilities_.push_back("skirmisher");
			for(config::child_list::const_iterator ab = skirmisher_abilities.begin(); ab != skirmisher_abilities.end(); ++ab) {
				if((**ab)["description"] != "") {
					ability_tooltips_.push_back((**ab)["description"]);
				} else {
					ability_tooltips_.push_back("skirmisher");
				}
				skirmish_ = true;
				skirmisher_filter_.add_filters((*ab)->child("filter"));
			}
		}
		const config::child_list& illuminate_abilities = abil_cfg->get_children("illuminates");
		if (!illuminate_abilities.empty()) {
			abilities_.push_back("illuminates");
			for(config::child_list::const_iterator ab = illuminate_abilities.begin(); ab != illuminate_abilities.end(); ++ab) {
				if((**ab)["description"] != "") {
					ability_tooltips_.push_back((**ab)["description"]);
				} else {
					ability_tooltips_.push_back("illuminates");
				}
				illuminates_ += lexical_cast_default<int>((**ab)["level"],1);
				illuminates_filter_.add_filters((*ab)->child("filter"));
			}
		}
		const config::child_list& teleport_abilities = abil_cfg->get_children("teleport");
		if (!teleport_abilities.empty()) {
			abilities_.push_back("teleport");
			for(config::child_list::const_iterator ab = teleport_abilities.begin(); ab != teleport_abilities.end(); ++ab) {
				if((**ab)["description"] != "") {
					ability_tooltips_.push_back((**ab)["description"]);
				} else {
					ability_tooltips_.push_back("teleport");
				}
				teleport_ = true;
				teleports_filter_.add_filters((*ab)->child("filter"));
			}
		}
		const config::child_list& hide_abilities = abil_cfg->get_children("hides");
		if (!hide_abilities.empty()) {
			abilities_.push_back("hides");
			for(config::child_list::const_iterator ab = hide_abilities.begin(); ab != hide_abilities.end(); ++ab) {
				if((**ab)["description"] != "") {
					ability_tooltips_.push_back((**ab)["description"]);
				} else {
					ability_tooltips_.push_back("hides");
				}
				hides_ = true;
				hides_filter_.add_filters((*ab)->child("filter"));
			}
		}
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

	const config::child_list& defends = cfg_.get_children("defend");
	for(config::child_list::const_iterator d = defends.begin(); d != defends.end(); ++d) {
		defensive_animations_.push_back(defensive_animation(**d));
	}
	if(!cfg["image_defensive_short"].empty()) {
		LOG_STREAM(err, config) << "unit " << id() << " uses an image_defensive_short tag, which is deprecated\n";
		defensive_animations_.push_back(defensive_animation(cfg["image_defensive_short"],"melee"));

	}
	if(!cfg["image_defensive_long"].empty()) {
		LOG_STREAM(err, config) << "unit " << id() << " uses an image_defensive_long tag, which is deprecated\n";
		defensive_animations_.push_back(defensive_animation(cfg["image_defensive_long"],"ranged"));
	}
	if(!cfg["image_defensive"].empty()) {
		LOG_STREAM(err, config) << "unit " << id() << " uses an image_defensive tag, which is deprecated\n";
		defensive_animations_.push_back(defensive_animation(cfg["image_defensive"]));
	}
	if(defensive_animations_.empty()) {
		defensive_animations_.push_back(defensive_animation(image()));
		// always have a defensive animation
	}



	const config::child_list& teleports = cfg_.get_children("teleport_anim");
	for(config::child_list::const_iterator t = teleports.begin(); t != teleports.end(); ++t) {
		teleport_animations_.push_back(unit_animation(**t));
	}
	if(teleport_animations_.empty()) {
		teleport_animations_.push_back(unit_animation(image(),-20,20));
		// always have a defensive animation
	}
	const config::child_list& extra_anims = cfg_.get_children("extra_anim");
	{
		for(config::child_list::const_iterator t = extra_anims.begin(); t != extra_anims.end(); ++t) {
			extra_animations_.insert(std::pair<std::string,unit_animation>((**t)["flag"],unit_animation(**t)));
		}
	}

	const config::child_list& deaths = cfg_.get_children("death");
	for(config::child_list::const_iterator death = deaths.begin(); death != deaths.end(); ++death) {
		death_animations_.push_back(death_animation(**death));
	}
	if(death_animations_.empty()) {
		death_animations_.push_back(death_animation(image()));
		// always have a defensive animation
	}

	const config::child_list& movement_anims = cfg_.get_children("movement_anim");
	for(config::child_list::const_iterator movement_anim = movement_anims.begin(); movement_anim != movement_anims.end(); ++movement_anim) {
		movement_animations_.push_back(movement_animation(**movement_anim));
	}
	if(!cfg["image_moving"].empty()) {
		LOG_STREAM(err, config) << "unit " << id() << " uses an image_moving tag, which is deprecated\n";
		movement_animations_.push_back(movement_animation(cfg["image_moving"]));
	}
	if(movement_animations_.empty()) {
		movement_animations_.push_back(movement_animation(image()));
		// always have a movement animation
	}

	flag_rgb_ = string2rgb(cfg["flag_rgb"]);
	// deprecation messages, only seen when unit is parsed for the first time
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

int unit_type::num_traits() const {
  return (cfg_["num_traits"].size() ? atoi(cfg_["num_traits"].c_str()) : race_->num_traits());
}

std::string unit_type::generate_description() const
{
	return race_->generate_name(cfg_["gender"] == "female" ? unit_race::FEMALE : unit_race::MALE);
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

const t_string& unit_type::language_name() const
{
	return cfg_["name"];
}

#if 0
const std::string& unit_type::name() const
{
	return cfg_["id"];
}
#endif

const std::string& unit_type::image() const
{
	return cfg_["image"];
}

const std::string& unit_type::image_halo() const
{
	return cfg_["halo"];
}


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


const std::string& unit_type::image_leading() const
{
	const std::string& val = cfg_["image_leading"];
	if(val.empty()) {
		return image();
	} else {
		return val;
	}
}

const std::string& unit_type::image_healing() const
{
	const std::string& val = cfg_["image_healing"];
	if(val.empty()) {
		return image();
	} else {
		return val;
	}
}

const std::string& unit_type::image_halo_healing() const
{
	return cfg_["image_halo_healing"];
}

const std::string& unit_type::image_profile() const
{
	const std::string& val = cfg_["profile"];
	if(val.size() == 0)
		return image();
	else
		return val;
}

const std::string& unit_type::unit_description() const
{
	static const std::string default_val("No description available");

	const std::string& desc = cfg_["unit_description"];
	if(desc.empty())
		return default_val;
	else
		return desc;
}

const std::string& unit_type::get_hit_sound() const
{
	return cfg_["get_hit_sound"];
}

const std::string& unit_type::die_sound() const
{
	return cfg_["die_sound"];
}

int unit_type::hitpoints() const
{
	return atoi(cfg_["hitpoints"].c_str());
}

std::vector<attack_type> unit_type::attacks() const
{
	std::vector<attack_type> res;
	for(config::const_child_itors range = cfg_.child_range("attack");
	    range.first != range.second; ++range.first) {
		res.push_back(attack_type(**range.first,*this));
	}

	return res;
}

const unit_movement_type& unit_type::movement_type() const
{
	return movementType_;
}

int unit_type::cost() const
{
	return atoi(cfg_["cost"].c_str());
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

int unit_type::experience_needed() const
{
	int exp = (experience_needed_ * experience_modifier + 50) /100;
	if(exp < 1) exp = 1;
	return exp;
}

std::vector<std::string> unit_type::advances_to() const
{
    return advances_to_;
}

const config::child_list& unit_type::modification_advancements() const
{
	return cfg_.get_children("advancement");
}

const std::string& unit_type::undead_variation() const
{
        return cfg_["undead_variation"];
}



const std::string& unit_type::usage() const
{
	return cfg_["usage"];
}

int unit_type::level() const
{
	return atoi(cfg_["level"].c_str());
}

int unit_type::movement() const
{
	return atoi(cfg_["movement"].c_str());
}

unit_type::ALIGNMENT unit_type::alignment() const
{
	return alignment_;
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

fixed_t unit_type::alpha() const
{
	return alpha_;
}

const std::vector<std::string>& unit_type::abilities() const
{
	return abilities_;
}

const std::vector<std::string>& unit_type::ability_tooltips() const
{
	return ability_tooltips_;
}

int unit_type::max_unit_healing() const
{
	return max_heals_;
}

int unit_type::heals() const
{
	return heals_;
}

bool unit_type::regenerates() const
{
	return regenerates_;
}

int unit_type::regenerate_amount() const
{
	return regeneration_;
}


bool unit_type::is_leader() const
{
	return leadership_;
}

int unit_type::leadership(int led_level) const
{
	char key[24]; // level[x]
	snprintf(key,sizeof(key),"level_%d",led_level);
	const config* abilities=cfg_.child("abilities");
	const config* leadership_ability=abilities ? abilities->child("leadership") : NULL;
	if(leadership_ability) {
		if((*leadership_ability)[key] != "") {
			return lexical_cast_default<int>((*leadership_ability)[key]);
		}
	}
	return maximum<int>(0,leadership_percent_*(level()-led_level));
}

int unit_type::illuminates() const
{
	return illuminates_;
}

bool unit_type::is_skirmisher() const
{
	return skirmish_;
}

bool unit_type::teleports() const
{
	return teleport_;
}

bool unit_type::steadfast() const
{
	return steadfast_;
}

int unit_type::steadfast_bonus() const
{
	return steadfast_bonus_;
}
int unit_type::steadfast_max() const
{
	return steadfast_max_;
}
bool unit_type::steadfast_ispercent() const
{
	return steadfast_percent_;
}

bool unit_type::not_living() const
{
	return race_->not_living();
}

bool unit_type::can_advance() const
{
	return !advances_to_.empty();
}

bool unit_type::has_zoc() const
{
	return level() > 0;
}

bool unit_type::has_ability(const std::string& ability) const
{
	return std::find(abilities_.begin(),abilities_.end(),ability) != abilities_.end();
}

const std::vector<config*>& unit_type::possible_traits() const
{
	return possibleTraits_;
}

const std::vector<unit_race::GENDER>& unit_type::genders() const
{
	return genders_;
}

const std::string& unit_type::race() const
{
	if(race_ == NULL) {
		static const std::string empty_string;
		return empty_string;
	}

	return race_->name();
}

const ability_filter unit_type::heals_filter() const
{
	return heals_filter_;
}
const ability_filter unit_type::regenerates_filter() const
{
	return regenerates_filter_;
}
const ability_filter unit_type::leadership_filter() const
{
	return leadership_filter_;
}
const ability_filter unit_type::illuminates_filter() const
{
	return illuminates_filter_;
}
const ability_filter unit_type::skirmisher_filter() const
{
	return skirmisher_filter_;
}
const ability_filter unit_type::teleports_filter() const
{
	return teleports_filter_;
}
const ability_filter unit_type::steadfast_filter() const
{
	return steadfast_filter_;
}
const ability_filter unit_type::hides_filter() const
{
	return hides_filter_;
}

const defensive_animation& unit_type::defend_animation(bool hits, std::string range) const
{
	//select one of the matching animations at random
	std::vector<const defensive_animation*> options;
	int max_val = -1;
	for(std::vector<defensive_animation>::const_iterator i = defensive_animations_.begin(); i != defensive_animations_.end(); ++i) {
		int matching = i->matches(hits,range);
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

const unit_animation& unit_type::teleport_animation( ) const
{
	return teleport_animations_[rand() % teleport_animations_.size()];
}

const unit_animation* unit_type::extra_animation(std::string flag ) const
{
	if (extra_animations_.count(flag) == 0) return NULL;
	std::multimap<std::string,unit_animation>::const_iterator index = extra_animations_.lower_bound(flag);
	int i = (rand()% extra_animations_.count(flag));
	for(int j = 0 ; j < i ; j++) {
		index++; // damn iterators
	}
	return &index->second;
}

const death_animation& unit_type::die_animation(const attack_type* attack) const
{
	//select one of the matching animations at random
	std::vector<const death_animation*> options;
	int max_val = -1;
	for(std::vector<death_animation>::const_iterator i = death_animations_.begin(); i != death_animations_.end(); ++i) {
		int matching = i->matches(attack);
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
const movement_animation& unit_type::move_animation(const std::string terrain,gamemap::location::DIRECTION dir) const
{
	//select one of the matching animations at random
	std::vector<const movement_animation*> options;
	int max_val = -1;
	for(std::vector<movement_animation>::const_iterator i = movement_animations_.begin(); i != movement_animations_.end(); ++i) {
		int matching = i->matches(terrain,dir);
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

const std::vector<Uint32>& unit_type::flag_rgb() const
{
        return flag_rgb_;
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
	}

	for(config::const_child_itors r = cfg.child_range("race");
	    r.first != r.second; ++r.first) {
		const unit_race race(**r.first);
		races.insert(std::pair<std::string,unit_race>(race.name(),race));
	}

	for(config::const_child_itors j = cfg.child_range("unit");
	    j.first != j.second; ++j.first) {
		const unit_type u_type(**j.first,movement_types,races,unit_traits);
		unit_types.insert(std::pair<std::string,unit_type>(u_type.id(),u_type));
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
        }

}

void game_data::clear()
{
	movement_types.clear();
	unit_types.clear();
	races.clear();
}
