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
#include "game_config.hpp"
#include "gamestatus.hpp"
#include "replay.hpp"
#include "unit.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>

namespace {
	const std::string ModificationTypes[] = { "object", "trait" };
	const int NumModificationTypes = sizeof(ModificationTypes)/
	                                 sizeof(*ModificationTypes);
}

bool compare_unit_values::operator()(const unit& a, const unit& b) const
{
	const int lvla = a.type().level();
	const int lvlb = b.type().level();

	const std::string& namea = a.type().name();
	const std::string& nameb = b.type().name();

	const int xpa = a.max_experience() - a.experience();
	const int xpb = b.max_experience() - b.experience();

	return lvla > lvlb || lvla == lvlb && namea < nameb ||
	                      lvla == lvlb && namea == nameb && xpa < xpb;
}

//constructor for reading a unit
unit::unit(game_data& data, config& cfg) : state_(STATE_NORMAL),
                                           moves_(0), facingLeft_(true),
                                           recruit_(false),
                                           guardian_(false), loyal_(false)
{
	read(data,cfg);
}

//constructor for creating a new unit
unit::unit(const unit_type* t, int side, bool use_traits) :
               type_(t), state_(STATE_NORMAL),
			   hitpoints_(t->hitpoints()),
			   maxHitpoints_(t->hitpoints()),
               backupMaxHitpoints_(t->hitpoints()), experience_(0),
			   maxExperience_(t->experience_needed()),
			   backupMaxExperience_(t->experience_needed()),
               side_(side), moves_(0), facingLeft_(side != 1),
			   maxMovement_(t->movement()),
			   backupMaxMovement_(t->movement()),
			   recruit_(false), attacks_(t->attacks()),
			   backupAttacks_(t->attacks()),
               guardian_(false), loyal_(false)
{
	//calculate the unit's traits
	std::vector<config*> traits = t->possible_traits();
	const size_t num_traits = 2;
	if(use_traits && traits.size() >= num_traits) {
		std::set<int> chosen_traits;
		for(size_t i = 0; i != num_traits; ++i) {
			int num = recorder.get_random()%(traits.size()-i);
			while(chosen_traits.count(num)) {
				++num;
			}

			chosen_traits.insert(num);

			add_modification("trait",*traits[num]);

		}

		//build the traits description, making sure the traits are always
		//in the same order.
		for(std::set<int>::const_iterator itor = chosen_traits.begin();
		    itor != chosen_traits.end(); ++itor) {
			traitsDescription_ += traits[*itor]->values["name"];
			traitsDescription_ += ",";
		}

		//get rid of the trailing comma
		if(!traitsDescription_.empty())
			traitsDescription_.resize(traitsDescription_.size()-1);
	}
}

//constructor for advancing a unit from a lower level
unit::unit(const unit_type* t, const unit& u) :
               type_(t), state_(STATE_NORMAL),
			   hitpoints_(t->hitpoints()),
			   maxHitpoints_(t->hitpoints()),
			   backupMaxHitpoints_(t->hitpoints()),
			   experience_(0),
			   maxExperience_(t->experience_needed()),
			   backupMaxExperience_(t->experience_needed()),
               side_(u.side()), moves_(u.moves_), facingLeft_(u.facingLeft_),
			   maxMovement_(t->movement()),
			   backupMaxMovement_(t->movement()),
			   description_(u.description_), recruit_(u.recruit_),
			   role_(u.role_), statusFlags_(u.statusFlags_),
			   attacks_(t->attacks()), backupAttacks_(t->attacks()),
			   modifications_(u.modifications_),
			   traitsDescription_(u.traitsDescription_),
               guardian_(false), loyal_(false)
{
	//apply modifications etc, refresh the unit
	new_level();
}

const unit_type& unit::type() const
{
	return *type_;
}

std::string unit::name() const
{
	if(description_.empty() == false)
		return description_;
	else
		return type().language_name();
}

const std::string& unit::description() const
{
	return description_;
}

int unit::side() const
{
	return side_;
}

double unit::alpha() const
{
	return type().alpha();
}

void unit::make_recruiter()
{
	recruit_ = true;
}

bool unit::can_recruit() const
{
	return recruit_;
}

int unit::total_movement() const
{
	return maxMovement_;
}

int unit::movement_left() const
{
	return moves_ < 0 ? 0 : moves_;
}

bool unit::can_attack() const
{
	return moves_ != -1;
}

void unit::set_movement(int moves)
{
	if(moves_ != -1)
		moves_ = moves;
}

void unit::set_attacked()
{
	moves_ = -1;
}

void unit::new_turn()
{
	moves_ = total_movement();
	if(type().has_ability("ambush"))
		set_flag("ambush");
}

void unit::end_turn()
{
	remove_flag("slowed");
}

void unit::new_level()
{
	//revert stats to the beginning of the level
	attacks_ = backupAttacks_;
	maxHitpoints_ = backupMaxHitpoints_;
	maxMovement_ = backupMaxMovement_;
	maxExperience_ = backupMaxExperience_;

	//reapply all permanent modifications
	apply_modifications();

	heal_all();
	statusFlags_.clear();
}

int unit::hitpoints() const
{
	return hitpoints_;
}

int unit::max_hitpoints() const
{
	return maxHitpoints_;
}

int unit::experience() const
{
	return experience_;
}

int unit::max_experience() const
{
	return maxExperience_;
}

bool unit::get_experience(int xp)
{
	experience_ += xp;
	if(experience_ > max_experience())
		experience_ = max_experience();
	return advances();
}

bool unit::advances() const
{
	return experience_ >= max_experience() && !type().advances_to().empty();
}

bool unit::gets_hit(int damage)
{
	hitpoints_ -= damage;
	if(hitpoints_ > max_hitpoints() && damage > 0)
		hitpoints_ = max_hitpoints();
	return hitpoints_ <= 0;
}

void unit::heal()
{
	heal(game_config::heal_amount);
}

void unit::heal(int amount)
{
	if(hitpoints_ < max_hitpoints()) {
		hitpoints_ += amount;
		if(hitpoints_ > max_hitpoints())
			hitpoints_ = max_hitpoints();
	}
}

void unit::heal_all()
{
	hitpoints_ = max_hitpoints();
}

bool unit::invisible(gamemap::TERRAIN terrain) const
{
	static const std::string forest_invisible("ambush");
	if(terrain == gamemap::FOREST && has_flag(forest_invisible)) {
		return true;
	}

	return false;
}

bool unit::matches_filter(config& cfg) const
{
	const std::string& description = cfg.values["description"];
	const std::string& type = cfg.values["type"];
	const std::string& ability = cfg.values["ability"];
	const std::string& side = cfg.values["side"];
	const std::string& weapon = cfg.values["has_weapon"];
	const std::string& role = cfg.values["role"];

	if(description.empty() == false && description != this->description()) {
		return false;
	}

	const std::string& this_type = this->type().name();

	//the type could be a comma-seperated list of types
	if(type.empty() == false && type != this_type) {

		//we only do the full CSV search if we find a comma in there,
		//and if the subsequence is found within the main sequence. This
		//is because doing the full CSV split is expensive
		if(std::find(type.begin(),type.end(),',') != type.end() &&
		   std::search(type.begin(),type.end(),this_type.begin(),this_type.end()) !=
		                                                    type.end()) {
			const std::vector<std::string>& vals = config::split(type);

			if(std::find(vals.begin(),vals.end(),this_type) == vals.end()) {
				return false;
			}
		} else {
			return false;
		}
	}

	if(ability.empty() == false && this->type().has_ability(ability) == false)
		return false;

	if(side.empty() == false && this->side() != atoi(side.c_str()))
		return false;

	if(weapon.empty() == false) {
		bool has_weapon = false;
		const std::vector<attack_type>& attacks = this->type().attacks();
		for(std::vector<attack_type>::const_iterator i = attacks.begin();
		    i != attacks.end(); ++i) {
			if(i->name() == weapon) {
				has_weapon = true;
			}
		}

		if(!has_weapon)
			return false;
	}

	if(role.empty() == false && role_ != role)
		return false;

	return true;
}

void unit::set_flag(const std::string& flag)
{
	statusFlags_.insert(flag);
}

void unit::remove_flag(const std::string& flag)
{
	statusFlags_.erase(flag);
}

bool unit::has_flag(const std::string& flag) const
{
	return statusFlags_.count(flag) != 0;
}

void unit::read(game_data& data, config& cfg)
{
	std::map<std::string,unit_type>::iterator i = data.unit_types.find(
					                                     cfg.values["type"]);
	if(i != data.unit_types.end())
		type_ = &i->second;
	else
		throw gamestatus::load_game_failed("Unit not found: '"
		                                   + cfg.values["type"] + "'");

	attacks_ = type_->attacks();
	backupAttacks_ = attacks_;
	maxHitpoints_ = type_->hitpoints();
	backupMaxHitpoints_ = type_->hitpoints();
	maxMovement_ = type_->movement();
	backupMaxMovement_ = type_->movement();
	maxExperience_ = type_->experience_needed();
	backupMaxExperience_ = type_->experience_needed();

	const std::string& hitpoints = cfg.values["hitpoints"];
	if(hitpoints.size() == 0)
		hitpoints_ = type().hitpoints();
	else
		hitpoints_ = atoi(hitpoints.c_str());

	const std::string& experience = cfg.values["experience"];
	if(experience.size() == 0)
		experience_ = 0;
	else
		experience_ = atoi(experience.c_str());


	side_ = atoi(cfg.values["side"].c_str());
	description_ = cfg.values["description"];
	traitsDescription_ = cfg.values["traits_description"];
	const std::map<std::string,std::string>::const_iterator recruit_itor =
			                                 cfg.values.find("canrecruit");
	if(recruit_itor != cfg.values.end() && recruit_itor->second == "1") {
		recruit_ = true;
	}

	const std::vector<config*>& mods = cfg.children["modifications"];
	if(!mods.empty()) {
		modifications_ = *mods.front();
		apply_modifications();
	}

	const std::string& facing = cfg.values["facing"];
	if(facing == "reverse")
		facingLeft_ = false;
	else
		facingLeft_ = true;

	const std::string& ai_special = cfg.values["ai_special"];
	if(ai_special == "guardian") {
		guardian_ = true;
	}
}

void unit::write(config& cfg) const
{
	cfg.values["type"] = type_->name();

	std::stringstream hp;
	hp << hitpoints_;
	cfg.values["hitpoints"] = hp.str();

	std::stringstream xp;
	xp << experience_;
	cfg.values["experience"] = xp.str();

	std::stringstream sd;
	sd << side_;
	cfg.values["side"] = sd.str();

	cfg.values["description"] = description_;

	cfg.values["traits_description"] = traitsDescription_;

	if(can_recruit())
		cfg.values["canrecruit"] = "1";

	cfg.children["modifications"].push_back(new config(modifications_));

	cfg.values["facing"] = facingLeft_ ? "normal" : "reverse";
}

void unit::assign_role(const std::string& role)
{
	role_ = role;
}

const std::vector<attack_type>& unit::attacks() const
{
	return attacks_;
}

int unit::movement_cost(const gamemap& map, gamemap::TERRAIN terrain) const
{
	const int res = type_->movement_type().movement_cost(map,terrain);

	static const std::string slowed_string("slowed");
	if(has_flag(slowed_string)) {
		return res*2;
	}

	return res;
}

double unit::defense_modifier(const gamemap& map,
                              gamemap::TERRAIN terrain) const
{
	return type_->movement_type().defense_modifier(map,terrain);
}

int unit::damage_against(const attack_type& attack) const
{
	return type_->movement_type().damage_against(attack);
}

const std::string& unit::image() const
{
	switch(state_) {
		case STATE_NORMAL: return type_->image();
		case STATE_DEFENDING_LONG:
		           return type_->image_defensive(attack_type::LONG_RANGE);
		case STATE_DEFENDING_SHORT:
		           return type_->image_defensive(attack_type::SHORT_RANGE);
		case STATE_ATTACKING: {
			if(attackType_ == NULL)
				return type_->image();

			const std::string* const img =
			          attackType_->get_frame(attackingMilliseconds_);

			if(img == NULL)
				return type_->image_fighting(attackType_->range());
			else
				return *img;
		}
		default: return type_->image();
	}
}

void unit::set_defending(bool newval, attack_type::RANGE range)
{
	state_ = newval ? (range == attack_type::LONG_RANGE ? STATE_DEFENDING_LONG :
	                   STATE_DEFENDING_SHORT): STATE_NORMAL;
}

void unit::set_attacking(bool newval, const attack_type* type, int ms)
{
	state_ = newval ? STATE_ATTACKING : STATE_NORMAL;
	attackType_ = type;
	attackingMilliseconds_ = ms;
}

bool unit::facing_left() const
{
	return facingLeft_;
}

void unit::set_facing_left(bool newval)
{
	facingLeft_ = newval;
}

const std::string& unit::traits_description() const
{
	return traitsDescription_;
}

int unit::value() const
{
	return type().cost();
}

bool unit::is_guardian() const
{
	return guardian_;
}

const gamemap::location& unit::get_goto() const
{
	return goto_;
}

void unit::set_goto(const gamemap::location& new_goto)
{
	goto_ = new_goto;
}

void unit::add_modification(const std::string& type, config& mod, bool no_add)
{
	const std::string& span = mod.values["duration"];

	if(no_add == false && (span.empty() || span == "forever"))
		modifications_.children[type].push_back(new config(mod));

	const std::vector<config*>& effects = mod.children["effect"];
	for(std::vector<config*>::const_iterator i = effects.begin();
	    i != effects.end(); ++i) {

		const std::string& apply_to = (*i)->values["apply_to"];

		if(apply_to == "new_attack") {
			attacks_.push_back(attack_type(**i));
		} else if(apply_to == "attack") {
			for(std::vector<attack_type>::iterator a = attacks_.begin();
			    a != attacks_.end(); ++a) {
				a->apply_modification(**i);
			}
		} else if(apply_to == "hitpoints") {
			const std::string& increase_hp = (*i)->values["increase"];
			const std::string& heal_full = (*i)->values["heal_full"];
			const std::string& increase_total = (*i)->values["increase_total"];
			const std::string& mult_total = (*i)->values["multiply_total"];

			//if the hitpoints are allowed to end up greater than max hitpoints
			const std::string& violate_max = (*i)->values["violate_maximum"];

			if(increase_total.empty() == false) {
				const int increase = atoi(increase_total.c_str());
				maxHitpoints_ += increase;
			}

			if(mult_total.empty() == false) {
				const double factor = atoi(mult_total.c_str());
				maxHitpoints_ = int(double(maxHitpoints_)*factor);
			}

			if(maxHitpoints_ < 1)
				maxHitpoints_ = 1;

			if(heal_full.empty() == false && heal_full != "no") {
				heal_all();
			}

			if(increase_hp.empty() == false) {
				const int increase = atoi(increase_hp.c_str());
				hitpoints_ += increase;
			}

			if(hitpoints_ > maxHitpoints_ && violate_max.empty())
				hitpoints_ = maxHitpoints_;

			if(hitpoints_ < 1)
				hitpoints_ = 1;
		} else if(apply_to == "movement") {
			const std::string& increase = (*i)->values["increase"];
			const std::string& mult = (*i)->values["multiply"];
			const std::string& set_to = (*i)->values["set"];

			if(increase.empty() == false) {
				maxMovement_ += atoi(increase.c_str());
				if(maxMovement_ < 1)
					maxMovement_ = 1;
			}

			if(mult.empty() == false) {
				maxMovement_ = int(double(maxMovement_)*atof(mult.c_str()));
			}

			if(set_to.empty() == false) {
				maxMovement_ = atoi(set_to.c_str());
			}

			if(moves_ > maxMovement_)
				moves_ = maxMovement_;
		} else if(apply_to == "max_experience") {
			const std::string& increase = (*i)->values["increase"];
			const std::string& multiply = (*i)->values["multiply"];
			if(increase.empty() == false) {
				maxExperience_ += atoi(increase.c_str());
			}

			if(multiply.empty() == false) {
				maxExperience_ = int(double(maxExperience_)*
				                     atof(multiply.c_str()));
			}

			if(maxExperience_ < 1) {
				maxExperience_ = 1;
			}
		} else if(apply_to == "loyal") {
			loyal_ = true;
		}
	}
}

void unit::apply_modifications()
{
	for(int i = 0; i != NumModificationTypes; ++i) {
		const std::string& mod = ModificationTypes[i];
		std::vector<config*>& mods = modifications_.children[mod];
		for(std::vector<config*>::iterator j = mods.begin();
		    j != mods.end(); ++j) {
			add_modification(ModificationTypes[i],**j,true);
		}
	}
}

int unit::upkeep() const
{
	//special units with descriptions don't have any upkeep,
	//as they are major units, not hired units
	if(description_.empty() == false)
		return 0;

	//loyal units always have an upkeep of 1 gold. Other units have an
	//upkeep equal to their level
	return loyal_ ? 1 : type().level();
}

int team_upkeep(const unit_map& units, int side)
{
	int res = 0;
	for(unit_map::const_iterator i = units.begin(); i != units.end(); ++i) {
		if(i->second.side() == side) {
			res += i->second.upkeep();
		}
	}

	return res;
}
