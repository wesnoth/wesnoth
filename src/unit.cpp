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

#include "font.hpp"
#include "game_config.hpp"
#include "gamestatus.hpp"
#include "language.hpp"
#include "log.hpp"
#include "pathfind.hpp"
#include "replay.hpp"
#include "unit.hpp"
#include "util.hpp"

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
unit::unit(game_data& data, const config& cfg) : state_(STATE_NORMAL),
                                           moves_(0), facingLeft_(true),
                                           recruit_(false),
                                           guardian_(false), upkeep_(UPKEEP_FREE)
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
               guardian_(false), upkeep_(UPKEEP_FULL_PRICE)
{
	if(use_traits) {
		//units that don't have traits generated are just generic
		//units, so they shouldn't get a description either.
		description_ = t->generate_description();
		generate_traits();
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
			   underlying_description_(u.underlying_description_),
			   description_(u.description_), recruit_(u.recruit_),
			   role_(u.role_), statusFlags_(u.statusFlags_),
			   attacks_(t->attacks()), backupAttacks_(t->attacks()),
			   modifications_(u.modifications_),
			   traitsDescription_(u.traitsDescription_),
               guardian_(false), upkeep_(u.upkeep_)
{
	//apply modifications etc, refresh the unit
	apply_modifications();
	heal_all();
	statusFlags_.clear();

	//generate traits for the unit if it doesn't already have some
	//currently removed, because we are testing not giving advancing
	//traitless units new traits
//	generate_traits();
}

void unit::generate_traits()
{
	if(!traitsDescription_.empty())
		return;

	//calculate the unit's traits
	const std::vector<config*> traits = type().possible_traits();

	const size_t num_traits = type().num_traits();
	if(traits.size() >= num_traits) {
		std::set<int> chosen_traits;
		for(size_t i = 0; i != num_traits; ++i) {
			int num = get_random()%(traits.size()-i);
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
			const std::string& trait_name = (*traits[*itor])["name"];
			const std::string& lang_trait = string_table["trait_"+trait_name];
			if(lang_trait.empty() == false)
				traitsDescription_ += lang_trait;
			else
				traitsDescription_ += trait_name;

			traitsDescription_ += ",";
		}

		//get rid of the trailing comma
		if(!traitsDescription_.empty())
			traitsDescription_.resize(traitsDescription_.size()-1);
	}
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

const std::string& unit::underlying_description() const
{
	return underlying_description_;
}

const std::string& unit::unit_description() const
{
	if(custom_unit_description_ != "") {
		return custom_unit_description_;
	} else {
		return type().unit_description();
	}
}

void unit::rename(const std::string& new_description)
{
	description_ = new_description;
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
	moves_ = moves;
}

void unit::set_attacked()
{
	moves_ = ATTACKED; 
}

void unit::end_unit_turn()
{
	if(moves_ == total_movement()){
		moves_ = NOT_MOVED;
	} else if(moves_ >= 0) {
		moves_ = MOVED;
	}
}

void unit::new_turn()
{
	moves_ = total_movement();
	if(type().has_ability("ambush"))
		set_flag("ambush");
	if(type().has_ability("nightstalk"))
		set_flag("nightstalk");

	if(stone())
		set_attacked();
}

void unit::end_turn()
{
	remove_flag("slowed");
	if((moves_ != total_movement()) && (moves_ != NOT_MOVED)){
		resting_ = false;
	}
}

void unit::new_level()
{
	//revert stats to the beginning of the level
	attacks_ = backupAttacks_;
	maxHitpoints_ = backupMaxHitpoints_;
	maxMovement_ = backupMaxMovement_;
	maxExperience_ = backupMaxExperience_;

	role_ = "";

	//set the goto command to be going to no-where
	goto_ = gamemap::location();

	remove_temporary_modifications();

	//reapply all permanent modifications
	apply_modifications();

	heal_all();
	statusFlags_.clear();
}

void unit::set_resting(bool resting)
{
	resting_ = resting;	
}

bool unit::is_resting() const
{
	return resting_;
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

bool unit::invisible(const std::string& terrain, int lawful_bonus, 
		const gamemap::location loc, 
		const unit_map& units,const std::vector<team>& teams) const
{
	bool is_inv = false;

	static const std::string forest_invisible("ambush");
	if(std::count(terrain.begin(),terrain.end(),gamemap::FOREST) && has_flag(forest_invisible)) {
		is_inv = true;
	}
	static const std::string night_invisible("nightstalk");
	if((lawful_bonus < 0) && has_flag(night_invisible)) {
		is_inv = true;
	}

	if(is_inv){
		for(unit_map::const_iterator u = units.begin(); u != units.end(); ++u) {
			if(teams[side_-1].is_enemy(u->second.side())) {
				if(tiles_adjacent(loc,u->first)) 
					return false;
			}
		}
		return true;
	}

	return false;
}

bool unit::poisoned() const
{
	static const std::string poisoned_str("poisoned");
	return has_flag(poisoned_str);
}

bool unit::stone() const
{
	static const std::string stone_str("stone");
	return has_flag(stone_str);
}

bool unit::matches_filter(const config& cfg) const
{
	const std::string& description = cfg["description"];
	const std::string& speaker = cfg["speaker"];
	const std::string& type = cfg["type"];
	const std::string& ability = cfg["ability"];
	const std::string& side = cfg["side"];
	const std::string& weapon = cfg["has_weapon"];
	const std::string& role = cfg["role"];

	if(description.empty() == false && description != this->underlying_description()) {
		return false;
	}

	//allow 'speaker' as an alternative to description, since people use it so often
	if(speaker.empty() == false && speaker != this->underlying_description()) {
		return false;
	}

	const std::string& this_type = this->type().name();

	//the type could be a comma-seperated list of types
	if(type.empty() == false && type != this_type) {

		//we only do the full CSV search if we find a comma in there,
		//and if the subsequence is found within the main sequence. This
		//is because doing the full CSV split is expensive
		if(std::find(type.begin(),type.end(),',') != type.end() &&
		   std::search(type.begin(),type.end(),this_type.begin(),
			           this_type.end()) != type.end()) {
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
	  {
		if(std::find(side.begin(),side.end(),',') != side.end()) {
			const std::vector<std::string>& vals = config::split(side);

			std::ostringstream s;
			s << (this->side());
			if(std::find(vals.begin(),vals.end(),s.str()) == vals.end()) {
				return false;
			}
		} else {
			return false;
		}
	  }

	if(weapon.empty() == false) {
		bool has_weapon = false;
		const std::vector<attack_type>& attacks = this->attacks();
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

void unit::read(game_data& data, const config& cfg)
{
	std::map<std::string,unit_type>::iterator i = data.unit_types.find(cfg["type"]);
	if(i != data.unit_types.end())
		type_ = &i->second;
	else
		throw gamestatus::load_game_failed("Unit not found: '" + cfg["type"] + "'" + " : " + cfg.write() + "'\n");

	assert(type_ != NULL);

	attacks_ = type_->attacks();
	backupAttacks_ = attacks_;
	maxHitpoints_ = type_->hitpoints();
	backupMaxHitpoints_ = type_->hitpoints();
	maxMovement_ = type_->movement();
	backupMaxMovement_ = type_->movement();
	maxExperience_ = type_->experience_needed();
	backupMaxExperience_ = type_->experience_needed();

	side_ = atoi(cfg["side"].c_str());
	if(side_ <= 0)
		side_ = 1;

	description_ = cfg["user_description"];
	underlying_description_ = cfg["description"];
	if(description_ == "") {
		description_ = underlying_description_;
	}

	custom_unit_description_ = cfg["unit_description"];

	traitsDescription_ = cfg["traits_description"];
	const std::map<std::string,std::string>::const_iterator recruit_itor =
			                                 cfg.values.find("canrecruit");
	if(recruit_itor != cfg.values.end() && recruit_itor->second == "1") {
		recruit_ = true;
	}

	const std::string& upkeep = cfg["upkeep"];
	if(upkeep == "full") {
		upkeep_ = UPKEEP_FULL_PRICE;
	} else if(upkeep == "loyal") {
		upkeep_ = UPKEEP_LOYAL;
	} else if(upkeep == "free") {
		upkeep_ = UPKEEP_FREE;
	}

	const std::string& facing = cfg["facing"];
	if(facing == "reverse")
		facingLeft_ = false;
	else
		facingLeft_ = true;

	const std::string& ai_special = cfg["ai_special"];
	if(ai_special == "guardian") {
		guardian_ = true;
	}

	role_ = cfg["role"];

	statusFlags_.clear();
	const config* const status_flags = cfg.child("status");
	if(status_flags != NULL) {
		for(string_map::const_iterator i = status_flags->values.begin(); i != status_flags->values.end(); ++i) {
			statusFlags_.insert(i->first);
		}
	}

	const config* const modifications = cfg.child("modifications");
	if(modifications != NULL) {
		modifications_ = *modifications;
		apply_modifications();
	}

	goto_.x = atoi(cfg["goto_x"].c_str()) - 1;
	goto_.y = atoi(cfg["goto_y"].c_str()) - 1;

	const std::string& moves = cfg["moves"];
	if(moves.empty())
		moves_ = total_movement();
	else
		moves_ = atoi(moves.c_str());

	const std::string& hitpoints = cfg["hitpoints"];
	if(hitpoints.size() == 0)
		hitpoints_ = maxHitpoints_;
	else
		hitpoints_ = atoi(hitpoints.c_str());

	const std::string& experience = cfg["experience"];
	if(experience.size() == 0)
		experience_ = 0;
	else
		experience_ = atoi(experience.c_str());

	resting_ = (cfg["resting"] == "yes");
}

void unit::write(config& cfg) const
{
	cfg["type"] = type_->name();

	std::stringstream hp;
	hp << hitpoints_;
	cfg["hitpoints"] = hp.str();

	std::stringstream xp;
	xp << experience_;
	cfg["experience"] = xp.str();

	std::stringstream sd;
	sd << side_;
	cfg["side"] = sd.str();

	cfg["role"] = role_;

	config status_flags;
	for(std::set<std::string>::const_iterator st = statusFlags_.begin(); st != statusFlags_.end(); ++st) {
		status_flags[*st] = "on";
	}

	cfg.add_child("status",status_flags);

	cfg["user_description"] = description_;
	cfg["description"] = underlying_description_;
	cfg["unit_description"] = custom_unit_description_;

	cfg["traits_description"] = traitsDescription_;

	if(can_recruit())
		cfg["canrecruit"] = "1";

	cfg.add_child("modifications",modifications_);

	cfg["facing"] = facingLeft_ ? "normal" : "reverse";

	switch(upkeep_) {
	case UPKEEP_FULL_PRICE: cfg["upkeep"] = "full"; break;
	case UPKEEP_LOYAL: cfg["upkeep"] = "loyal"; break;
	case UPKEEP_FREE: cfg["upkeep"] = "free"; break;
	}

	if(guardian_) {
		cfg["ai_special"] = "guardian";
	}

	char buf[50];
	sprintf(buf,"%d",goto_.x+1);
	cfg["goto_x"] = buf;
	sprintf(buf,"%d",goto_.y+1);
	cfg["goto_y"] = buf;

	sprintf(buf,"%d",moves_);
	cfg["moves"] = buf;

	cfg["resting"] = resting_ ? "yes" : "no"; 
}

void unit::assign_role(const std::string& role)
{
	role_ = role;
}

const std::vector<attack_type>& unit::attacks() const
{
	return attacks_;
}

int unit::longest_range() const
{
	int res = 0;
	for(std::vector<attack_type>::const_iterator i = attacks_.begin(); i != attacks_.end(); ++i) {
		if(i->hexes() >= res)
			res = i->hexes();
	}

	return res;
}

std::vector<attack_type> unit::attacks_at_range(int range) const
{
	std::vector<attack_type> res;
	for(std::vector<attack_type>::const_iterator i = attacks_.begin(); i != attacks_.end(); ++i) {
		if(i->hexes() >= range)
			res.push_back(*i);
	}

	return res;
}

int unit::movement_cost(const gamemap& map, gamemap::TERRAIN terrain) const
{
//don't allow level 0 units to take villages - removed until AI
//is smart enough to deal with this.
//	if(type_->level() == 0 && map.is_village(terrain))
//		return 100;



	const int res = type_->movement_type().movement_cost(map,terrain);

	static const std::string slowed_string("slowed");
	if(has_flag(slowed_string)) {
		return res*2;
	}

	return res;
}

int unit::defense_modifier(const gamemap& map,
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

void unit::add_modification(const std::string& type,
                            const config& mod, bool no_add)
{
	if(no_add == false) {
		modifications_.add_child(type,mod);
	}

	std::vector<std::string> effects_description;

	for(config::const_child_itors i = mod.child_range("effect");
	    i.first != i.second; ++i.first) {

		std::stringstream description;

		const std::string& apply_to = (**i.first)["apply_to"];

		if(apply_to == "new_attack") {
			attacks_.push_back(attack_type(**i.first));
		} else if(apply_to == "attack") {
			for(std::vector<attack_type>::iterator a = attacks_.begin();
			    a != attacks_.end(); ++a) {
				a->apply_modification(**i.first);
			}
		} else if(apply_to == "hitpoints") {
			std::cerr << "applying hitpoint mod...." << hitpoints_ << "/" << maxHitpoints_ << "\n";
			const std::string& increase_hp = (**i.first)["increase"];
			const std::string& heal_full = (**i.first)["heal_full"];
			const std::string& increase_total = (**i.first)["increase_total"];

			//if the hitpoints are allowed to end up greater than max hitpoints
			const std::string& violate_max = (**i.first)["violate_maximum"];

			if(increase_total.empty() == false) {
				description << (increase_total[0] != '-' ? "+" : "") << increase_total << translate_string("hp");

				//a percentage on the end means increase by that many percent
				if(increase_total[increase_total.size()-1] == '%') {
					const std::string inc(increase_total.begin(),increase_total.end()-1);
					maxHitpoints_ += (maxHitpoints_*atoi(inc.c_str()))/100;
				} else {
					maxHitpoints_ += atoi(increase_total.c_str());
				}
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

			std::cerr << "modded to " << hitpoints_ << "/" << maxHitpoints_ << "\n";
			if(hitpoints_ > maxHitpoints_ && violate_max.empty()) {
				std::cerr << "resetting hp to max\n";
				hitpoints_ = maxHitpoints_;
			}

			if(hitpoints_ < 1)
				hitpoints_ = 1;
		} else if(apply_to == "movement") {
			const std::string& increase = (**i.first)["increase"];
			const std::string& set_to = (**i.first)["set"];

			if(increase.empty() == false) {
				description << (increase[0] != '-' ? "+" : "") << increase << translate_string("moves");

				if(increase[increase.size()-1] == '%') {
					const std::string inc(increase.begin(),increase.end()-1);
					maxMovement_ += (maxMovement_*atoi(inc.c_str()))/100;
				} else {
					maxMovement_ += atoi(increase.c_str());
				}

				if(maxMovement_ < 1)
					maxMovement_ = 1;
			}

			if(set_to.empty() == false) {
				maxMovement_ = atoi(set_to.c_str());
			}

			if(moves_ > maxMovement_)
				moves_ = maxMovement_;
		} else if(apply_to == "max_experience") {
			const std::string& increase = (**i.first)["increase"];

			if(increase.empty() == false) {
				description << (increase[0] != '-' ? "+" : "") << increase << translate_string("xp");
				if(increase[increase.size()-1] == '%') {
					const std::string inc(increase.begin(),increase.end()-1);
					maxExperience_ += (maxExperience_*atoi(inc.c_str()))/100;
				} else {
					maxExperience_ += atoi(increase.c_str());
				}
			}

			if(maxExperience_ < 1) {
				maxExperience_ = 1;
			}
		} else if(apply_to == "loyal") {
			description << string_table["loyal_description"];
			if(upkeep_ > UPKEEP_LOYAL)
				upkeep_ = UPKEEP_LOYAL;
		} else if(apply_to == "status") {
			const std::string& add = (**i.first)["add"];
			const std::string& remove = (**i.first)["remove"];

			if(add.empty() == false) {
				set_flag(add);
			}
			
			if(remove.empty() == false) {
				remove_flag(remove);
			}
		}

		const std::string desc = description.str();
		if(!desc.empty())
			effects_description.push_back(desc);
	}

	std::stringstream description;
	description << translate_string_default(mod["id"],mod["name"]) << ": ";
	if(mod["id"].empty() == false) {
		description << translate_string_default(mod["id"] + "_description",mod["description"]) << " ";
	}

	if(effects_description.empty() == false) {
		description << "(";
		for(std::vector<std::string>::const_iterator i = effects_description.begin(); i != effects_description.end(); ++i) {
			description << *i;
			if(i+1 != effects_description.end())
				description << "; ";
		}
		description << ")";
	}

	description << "\n";

	modificationDescriptions_[type] += description.str();
}

void unit::apply_modifications()
{
	log_scope("apply mods");
	modificationDescriptions_.clear();

	for(int i = 0; i != NumModificationTypes; ++i) {
		const std::string& mod = ModificationTypes[i];
		const config::child_list& mods = modifications_.get_children(mod);
		for(config::child_list::const_iterator j = mods.begin(); j != mods.end(); ++j) {
			log_scope("add mod");
			add_modification(ModificationTypes[i],**j,true);
		}
	}
}

void unit::remove_temporary_modifications()
{
	for(int i = 0; i != NumModificationTypes; ++i) {
		const std::string& mod = ModificationTypes[i];
		const config::child_list& mods = modifications_.get_children(mod);
		for(int j = 0; j != mods.size(); ++j) {
			if((*mods[j])["duration"] != "forever" && (*mods[j])["duration"] != "") {
				modifications_.remove_child(mod,j);
				--j;
			}
		}
	}
}

const std::string& unit::modification_description(const std::string& type) const
{
	const string_map::const_iterator i = modificationDescriptions_.find(type);
	if(i == modificationDescriptions_.end()) {
		static const std::string empty_string;
		return empty_string;
	} else {
		return i->second;
	}
}

int unit::upkeep() const
{
	switch(upkeep_) {
	case UPKEEP_FREE: return 0;
	case UPKEEP_LOYAL: return minimum<int>(1,type().level());
	case UPKEEP_FULL_PRICE: return type().level();
	default: assert(false); return 0;
	}
}

bool unit::is_flying() const
{
	return type().movement_type().is_flying();
}

int team_units(const unit_map& units, int side)
{
	int res = 0;
	for(unit_map::const_iterator i = units.begin(); i != units.end(); ++i) {
		if(i->second.side() == side) {
			++res;
		}
	}

	return res;
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

unit_map::const_iterator team_leader(int side, const unit_map& units)
{
	for(unit_map::const_iterator i = units.begin(); i != units.end(); ++i) {
		if(i->second.can_recruit() && i->second.side() == side) {
			return i;
		}
	}

	return units.end();
}

std::string team_name(int side, const unit_map& units)
{
	const unit_map::const_iterator i = team_leader(side,units);
	if(i != units.end())
		return i->second.description();
	else
		return "-";
}

unit_map::iterator find_visible_unit(unit_map& units,
		const gamemap::location loc, 
		const gamemap& map, int lawful_bonus, 
		const std::vector<team>& teams, const team& current_team)
{
	unit_map::iterator u = units.find(loc);
	if(map.on_board(loc)){
		if(u != units.end()){
			if(current_team.is_enemy(u->second.side()) &&
					u->second.invisible(
						map.underlying_terrain(map[loc.x][loc.y]),lawful_bonus,
						loc,units,teams)) {
				return units.end();
			}
		}
	}
	return u;
}

unit_map::const_iterator find_visible_unit(const unit_map& units,
		const gamemap::location loc, 
		const gamemap& map, int lawful_bonus, 
		const std::vector<team>& teams, const team& current_team)
{
	unit_map::const_iterator u = units.find(loc);
	if(map.on_board(loc)){
		if(u != units.end()){
			if(current_team.is_enemy(u->second.side()) &&
					u->second.invisible(
						map.underlying_terrain(map[loc.x][loc.y]),lawful_bonus,
						loc,units,teams)) {
				return units.end();
			}
		}
	}
	return u;
}

team_data calculate_team_data(const team& tm, int side, const unit_map& units)
{
	team_data res;
	res.units = team_units(units,side);
	res.upkeep = team_upkeep(units,side);
	res.villages = tm.towers().size();
	res.expenses = maximum<int>(0,res.upkeep - res.villages);
	res.net_income = tm.income() - res.expenses;
	res.gold = tm.gold();
	return res;
}

std::string get_team_name(int side, const unit_map& units)
{
	for(unit_map::const_iterator i = units.begin(); i != units.end(); ++i) {
		if(i->second.can_recruit() && i->second.side() == side) {
			return i->second.description();
		}
	}

	return "-";
}

temporary_unit_placer::temporary_unit_placer(unit_map& m, const gamemap::location& loc, const unit& u)
  : m_(m), loc_(loc), temp_(m.count(loc) == 1 ? m.find(loc)->second : u), use_temp_(m.count(loc) == 1)
{
	if(use_temp_) {
		m.erase(loc);
	}

	m.insert(std::pair<gamemap::location,unit>(loc,u));
}

temporary_unit_placer::~temporary_unit_placer()
{
	m_.erase(loc_);
	if(use_temp_) {
		m_.insert(std::pair<gamemap::location,unit>(loc_,temp_));
	}
}