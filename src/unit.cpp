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
#include "game_errors.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "pathfind.hpp"
#include "preferences.hpp"
#include "random.hpp"
#include "unit.hpp"
#include "unit_types.hpp"
#include "util.hpp"
#include "wassert.hpp"
#include "serialization/string_utils.hpp"
#include "halo.hpp"
#include "display.hpp"

#include <ctime>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iterator>

#define LOG_UT LOG_STREAM(info, engine)

namespace {
	const std::string ModificationTypes[] = { "object", "trait", "advance" };
	const size_t NumModificationTypes = sizeof(ModificationTypes)/
	                                    sizeof(*ModificationTypes);
}

static bool compare_unit_values(unit const &a, unit const &b)
{
	const int lvla = a.level();
	const int lvlb = b.level();

	const int xpa = a.max_experience() - a.experience();
	const int xpb = b.max_experience() - b.experience();

	return lvla > lvlb || lvla == lvlb || lvla == lvlb && xpa < xpb;
}

void sort_units(std::vector< unit > &units)
{
	std::sort(units.begin(), units.end(), compare_unit_values);
}







// Copy constructor
unit::unit(const unit& u)
{
	cfg_ = u.cfg_;
	
	advances_to_ = u.advances_to_;
	id_ = u.id_;
	race_ = u.race_;
	name_ = u.name_;
	description_ = u.description_;
	custom_unit_description_ = u.custom_unit_description_;
	underlying_description_ = u.underlying_description_;
	language_name_ = u.language_name_;
	undead_variation_ = u.undead_variation_;
	variation_ = u.variation_;
	
	hit_points_ = u.hit_points_;
	max_hit_points_ = u.max_hit_points_;
	max_hit_points_b_ = u.max_hit_points_b_;
	experience_ = u.experience_;
	max_experience_ = u.max_experience_;
	max_experience_b_ = u.max_experience_b_;
	level_ = u.level_;
	alignment_ = u.alignment_;
	flag_rgb_ = u.flag_rgb_;
	
	unrenamable_ = u.unrenamable_;
	side_ = u.side_;
	gender_ = u.gender_;
	
	alpha_ = u.alpha_;
	
	recruits_ = u.recruits_;
	
	movement_ = u.movement_;
	max_movement_ = u.max_movement_;
	max_movement_b_ = u.max_movement_b_;
	hold_position_ = u.hold_position_;
	end_turn_ = u.end_turn_;
	resting_ = u.resting_;
	attacks_left_ = u.attacks_left_;
	max_attacks_ = u.max_attacks_;
	
	states_ = u.states_;
	variables_ = u.variables_;
	emit_zoc_ = u.emit_zoc_;
	state_ = u.state_;
	
	overlays_ = u.overlays_;
	
	role_ = u.role_;
	attacks_ = u.attacks_;
	attacks_b_ = u.attacks_b_;
	facing_dir_ = u.facing_dir_;
	
	traits_description_ = u.traits_description_;
	unit_value_ = u.unit_value_;
	goto_ = u.goto_;
	interrupted_move_ = u.interrupted_move_;
	upkeep_ = u.upkeep_;
	flying_ = u.flying_;
	
	modification_descriptions_ = u.modification_descriptions_;
	defensive_animations_ = u.defensive_animations_;
	
	teleport_animations_ = u.teleport_animations_;
	
	extra_animations_ = u.extra_animations_;
	
	death_animations_ = u.death_animations_;
	
	movement_animations_ = u.movement_animations_;
	anim_ = NULL;
	unit_halo_ = u.unit_halo_;
	unit_anim_halo_ = u.unit_anim_halo_;
	attackType_ = NULL;
	attackingMilliseconds_=0;
	getsHit_=0;
	
	modifications_ = u.modifications_;
	
	
	gamedata_ = u.gamedata_;
	units_ = u.units_;
	map_ = u.map_;
	gamestatus_ = u.gamestatus_;
	teams_ = u.teams_;
}

// Initilizes a unit from a config
unit::unit(const game_data* gamedata, unit_map* unitmap, const gamemap* map, 
     const gamestatus* game_status, const std::vector<team>* teams,
	 const config& cfg) : gamedata_(gamedata),units_(unitmap),map_(map),
	                      gamestatus_(game_status),teams_(teams)
{
	facing_dir_ = LEFT;
	read(cfg);
	anim_ = NULL;
	attackType_ = NULL;
	attackingMilliseconds_=0;
	getsHit_=0;
	end_turn_ = false;
	backup_state();
}

unit::unit(const game_data& gamedata,const config& cfg) : gamedata_(&gamedata),units_(NULL),map_(NULL),
	                      gamestatus_(NULL)
{
	facing_dir_ = LEFT;
	read(cfg);
	anim_ = NULL;
	attackType_ = NULL;
	attackingMilliseconds_=0;
	getsHit_=0;
	end_turn_ = false;
	backup_state();
}

// Initilizes a unit from a unit type
unit::unit(const game_data* gamedata, unit_map* unitmap, const gamemap* map, 
           const gamestatus* game_status, const std::vector<team>* teams, const unit_type* t, 
					 int side, bool use_traits, bool dummy_unit, unit_race::GENDER gender) : 
           gamedata_(gamedata),units_(unitmap),map_(map),gamestatus_(game_status),teams_(teams)
{
	side_ = side;
	facing_dir_ = LEFT;
	movement_ = 0;
	advance_to(t);
	if(dummy_unit == false) validate_side(side_);
	if(use_traits) {
		//units that don't have traits generated are just generic
		//units, so they shouldn't get a description either.
		description_ = generate_description();
		generate_traits();
		underlying_description_ = description_;
	}else{
	  underlying_description_ = id();
	}
	anim_ = NULL;
	attackType_ = NULL;
	attackingMilliseconds_=0;
	getsHit_=0;
	end_turn_ = false;
	attacks_left_ = max_attacks_;
}
unit::unit(const unit_type* t,
					 int side, bool use_traits, bool dummy_unit, unit_race::GENDER gender) : 
           gamedata_(NULL),units_(NULL),map_(NULL),gamestatus_(NULL),teams_(NULL)
{
	side_ = side;
	facing_dir_ = LEFT;
	movement_ = 0;
	advance_to(t);
	if(dummy_unit == false) validate_side(side_);
	if(use_traits) {
		//units that don't have traits generated are just generic
		//units, so they shouldn't get a description either.
		description_ = generate_description();
		generate_traits();
		underlying_description_ = description_;
	}else{
	  underlying_description_ = id();
	}
	attacks_left_ = max_attacks_;
}

unit::~unit()
{
	if(unit_halo_) {
		halo::remove(unit_halo_);
	}
}
void unit::set_game_context(const game_data* gamedata, unit_map* unitmap, const gamemap* map, const gamestatus* game_status, const std::vector<team>* teams)
{
	gamedata_ = gamedata;
	units_ = unitmap;
	map_ = map;
	gamestatus_ = game_status;
	teams_ = teams;
}


std::string unit::generate_description() const
{
	return race_->generate_name(cfg_["gender"] == "female" ? unit_race::FEMALE : unit_race::MALE);
}

void unit::generate_traits()
{
	if(!traits_description_.empty())
		return;
	
	wassert(gamedata_ != NULL);
	const game_data::unit_type_map::const_iterator type = gamedata_->unit_types.find(id());
	//calculate the unit's traits
	std::vector<config*> candidate_traits = type->second.possible_traits();
	std::vector<config*> traits;

	const size_t num_traits = type->second.num_traits();
	for(size_t n = 0; n != num_traits && candidate_traits.empty() == false; ++n) {
		const int num = get_random()%candidate_traits.size();
		traits.push_back(candidate_traits[num]);
		candidate_traits.erase(candidate_traits.begin()+num);
	}

	for(std::vector<config*>::const_iterator j = traits.begin(); j != traits.end(); ++j) {
		modifications_.add_child("trait",**j);
	}

	apply_modifications();
}

// Advances this unit to another type
void unit::advance_to(const unit_type* t)
{
	
	cfg_ = cfg_.merge_with(t->cfg_);
	
	advances_to_ = t->advances_to();
	
	race_ = t->race_;
	language_name_ = t->language_name();
	cfg_["unit_description"] = t->unit_description();
	undead_variation_ = t->undead_variation();
	experience_ = 0;
//	experience_ = t->experience_needed();
	max_experience_ = t->experience_needed();
	level_ = t->level();
	alignment_ = t->alignment();
	alpha_ = t->alpha();
	hit_points_ = t->hitpoints();
	max_hit_points_ = t->hitpoints();
//	movement_ = t->movement();
	max_movement_ = t->movement();
	emit_zoc_ = t->level();
	attacks_ = t->attacks();
	unit_value_ = t->cost();
	upkeep_ = t->level();
	flying_ = t->movement_type().is_flying();
//	movement_costs_ = t->movement_type().movement_costs();
//	defense_mods_ = t->movement_type().defense_mods();
	max_attacks_ = lexical_cast_default<int>(t->cfg_["attacks"],1);
	defensive_animations_ = t->defensive_animations_;
	teleport_animations_ = t->teleport_animations_;
	extra_animations_ = t->extra_animations_;
	death_animations_ = t->death_animations_;
	movement_animations_ = t->movement_animations_;
	flag_rgb_ = t->flag_rgb();
	
	backup_state();
	//apply modifications etc, refresh the unit
	reset_modifications();
	apply_modifications();
	if(id()!=t->id()) {
	  heal_all();
	}
	id_ = t->id();
	
	set_state("poisoned","");
	set_state("slowed","");
	set_state("stoned","");
	end_turn_ = false;
}
const std::vector<std::string> unit::advances_to() const
{
	return advances_to_;
}

// the current type id
const std::string& unit::id() const
{
	return id_;
}
// the actual name of the unit
const std::string& unit::name() const
{
	return name_;
}
void unit::rename(const std::string& name)
{
	name_ = name;
}
// the unit type name
const std::string& unit::description() const
{
	if(custom_unit_description_ != "") {
		return custom_unit_description_;
	} else {
		return description_;
	}
}
const std::string& unit::underlying_description() const
{
	return underlying_description_;
}
const t_string& unit::language_name() const
{
	return language_name_;
}
// the unit's profile
const std::string& unit::profile() const
{
	return cfg_["profile"];
}
//information about the unit -- a detailed description of it
const std::string& unit::unit_description() const
{
	return cfg_["unit_description"];
}
const std::string& unit::undead_variation() const
{
	return undead_variation_;
}
int unit::hitpoints() const
{
	return hit_points_;
}
int unit::max_hitpoints() const
{
	return max_hit_points_;
}
int unit::experience() const
{
	return experience_;
}
int unit::max_experience() const
{
	return max_experience_;
}
int unit::level() const
{
	return level_;
}
// adds 'xp' points to the units experience; returns true if advancement should occur
bool unit::get_experience(int xp)
{
	experience_ += xp;
	return advances();
}

bool unit::advances() const
{
	return experience_ >= max_experience() && can_advance();
}
SDL_Colour unit::hp_color() const
{
  double unit_energy = 0.0;
  SDL_Color energy_colour = {0,0,0,0};

  if(max_hitpoints() > 0) {
    unit_energy = double(hitpoints())/double(max_hitpoints());
  }

  if(1.0 == unit_energy){
    energy_colour.r = 33;
    energy_colour.g = 225;
    energy_colour.b = 0;
  } else if(unit_energy > 1.0) {
    energy_colour.r = 100;
    energy_colour.g = 255;
    energy_colour.b = 100;
  } else if(unit_energy >= 0.75) {
    energy_colour.r = 170;
    energy_colour.g = 255;
    energy_colour.b = 0;
  } else if(unit_energy >= 0.5) {
    energy_colour.r = 255;
    energy_colour.g = 155;
    energy_colour.b = 0;
  } else if(unit_energy >= 0.25) {
    energy_colour.r = 255;
    energy_colour.g = 175;
    energy_colour.b = 0;
  } else {
    energy_colour.r = 255;
    energy_colour.g = 0;
    energy_colour.b = 0;
  }
  return energy_colour;
}
SDL_Colour unit::xp_color() const
{
  const SDL_Color near_advance_colour = {255,255,255,0};
  const SDL_Color mid_advance_colour = {150,255,255,0};
  const SDL_Color far_advance_colour = {0,205,205,0};
  const SDL_Color normal_colour = {0,160,225,0};
  const SDL_Color near_amla_colour = {225,0,255,0};
  const SDL_Color mid_amla_colour = {169,30,255,0};
  const SDL_Color far_amla_colour = {139,0,237,0};
  const SDL_Color amla_colour = {100,0,150,0};
  const bool near_advance = max_experience() - experience() <= game_config::kill_experience;
  const bool mid_advance = max_experience() - experience() <= game_config::kill_experience*2;
  const bool far_advance = max_experience() - experience() <= game_config::kill_experience*3;

  SDL_Color colour=normal_colour;
  if(advances_to().size()){
    if(near_advance){
      colour=near_advance_colour;
    } else if(mid_advance){
      colour=mid_advance_colour;
    } else if(far_advance){
      colour=far_advance_colour;
    }
  } else if (get_modification_advances().size()){
    if(near_advance){
      colour=near_amla_colour;
    } else if(mid_advance){
      colour=mid_amla_colour;
    } else if(far_advance){
      colour=far_amla_colour;
    } else {
      colour=amla_colour;
    }
  }
  return(colour);
}
bool unit::unrenamable() const /** < Set to true for some scenario-specific units which should not be renamed */
{
	return unrenamable_;
}

unsigned int unit::side() const
{
	return side_;
}
Uint32 unit::team_rgb() const
{
	return(team::get_side_rgb(side()));
}
const std::vector<Uint32>& unit::flag_rgb() const
{
	return flag_rgb_;
}
std::vector<Uint32> unit::team_rgb_range() const
{
  std::vector<Uint32> temp;
  temp.push_back(team::get_side_rgb(side()));
  temp.push_back(team::get_side_rgb_max(side()));
  temp.push_back(team::get_side_rgb_min(side()));
  return(temp);
}
unit_race::GENDER unit::gender() const
{
	return gender_;
}
void unit::set_side(unsigned int new_side)
{
	side_ = new_side;
}
fixed_t unit::alpha() const
{
	return alpha_;
}

bool unit::can_recruit() const
{
	return !recruits_.empty() || cfg_["canrecruit"]=="1";
}
const std::vector<std::string>& unit::recruits() const
{
	return recruits_;
}
int unit::total_movement() const
{
	return max_movement_;
}
int unit::movement_left() const
{
	return movement_;
}
void unit::set_hold_position(bool value)
{
	hold_position_ = value;
}
bool unit::hold_position() const
{
	return hold_position_;
}
void unit::set_user_end_turn(bool value)
{
	end_turn_ = value;
}
bool unit::user_end_turn() const
{
//	return end_turn_;
	return false;
}
int unit::attacks_left() const
{
	return attacks_left_;
}
void unit::set_movement(int moves)
{
	hold_position_ = false;
	end_turn_ = false;
	movement_ = maximum<int>(0,minimum<int>(moves,max_movement_));
}
void unit::set_attacks(int left)
{
	attacks_left_ = maximum<int>(0,minimum<int>(left,max_attacks_));
}
void unit::unit_hold_position()
{
	hold_position_ = true;
	end_turn_ = true;
}
void unit::end_unit_turn()
{
//	wassert("not done" == "done");
//	if(movement_ == total_movement()) {
//		movement_ = NOT_MOVED;
//	} else if(movement_ >= 0) {
//		movement_ = MOVED;
//	}
}
void unit::new_turn(const gamemap::location& loc)
{
	end_turn_ = false;
	movement_ = total_movement();
	attacks_left_ = max_attacks_;
	if(get_ability_bool("hides",loc)) {
		set_state("hides","true");
	} else {
		set_state("hides","");
	}
	if(get_state("stoned")=="true") {
		set_attacks(0);
	}
	if (hold_position_) {
		end_turn_ = true;
	}
}
void unit::end_turn()
{
	set_state("slowed","");
	if((movement_ != total_movement())) {
		resting_ = false;
	}
	//clear interrupted move
	set_interrupted_move(gamemap::location());
}
void unit::new_level()
{
	role_ = "";

	//set the goto command to be going to no-where
	goto_ = gamemap::location();

	remove_temporary_modifications();

	//reapply all permanent modifications
	apply_modifications();

	heal_all();
	set_state("slowed","");
	set_state("poisoned","");
	set_state("stoned","");
}
void unit::remove_temporary_modifications()
{
	for(unsigned int i = 0; i != NumModificationTypes; ++i) {
		const std::string& mod = ModificationTypes[i];
		const config::child_list& mods = modifications_.get_children(mod);
		for(size_t j = 0; j != mods.size(); ++j) {
			if((*mods[j])["duration"] != "forever" && (*mods[j])["duration"] != "") {
				modifications_.remove_child(mod,j);
				--j;
			}
		}
	}
}

bool unit::take_hit(int damage)
{
	hit_points_ -= damage;
	return hit_points_ <= 0;
}
void unit::heal()
{
	heal(game_config::heal_amount);
}
void unit::heal(int amount)
{
	int max_hp = max_hitpoints();
	if (hit_points_ < max_hp) {
		hit_points_ += amount;
		if (hit_points_ > max_hp) {
			hit_points_ = max_hp;
		}
	}
}
void unit::heal_all()
{
	hit_points_ = max_hitpoints();
}
bool unit::resting() const
{
	return resting_;
}
void unit::set_resting(bool rest)
{
	resting_ = rest;
}

const std::string unit::get_state(const std::string& state) const
{
	std::map<std::string,std::string>::const_iterator i = states_.find(state);
	if(i != states_.end()) {
		return i->second;
	}
	return "";
}
void unit::set_state(const std::string& state, const std::string& value)
{
	if(value == "") {
		std::map<std::string,std::string>::iterator i = states_.find(state);
		if(i != states_.end()) {
			states_.erase(i);
		}
	} else {
		states_[state] = value;
	}
}

bool unit::has_moved() const
{
	return movement_left() != total_movement();
}
bool unit::has_goto() const
{
	return get_goto().valid();
}
int unit::emits_zoc() const
{
	return emit_zoc_;
}
bool unit::matches_filter(const config& cfg,const gamemap::location& loc,bool use_flat_tod) const
{
	const std::string& description = cfg["description"];
	const std::string& speaker = cfg["speaker"];
	const std::string& type = cfg["type"];
	const std::string& profile = cfg["profile"];
	const std::string& ability = cfg["ability"];
	const std::string& side = cfg["side"];
	const std::string& weapon = cfg["has_weapon"];
	const std::string& role = cfg["role"];
	const std::string& race = cfg["race"];
	const std::string& gender = cfg["gender"];
	const std::string& canrecruit = cfg["canrecruit"];
	const std::string& level = cfg["level"];

	if(description.empty() == false && description != this->underlying_description()) {
		return false;
	}
	if(profile.empty() == false && profile != cfg_["profile"]) {
		return false;
	}

	//allow 'speaker' as an alternative to description, since people use it so often
	if(speaker.empty() == false && speaker != this->underlying_description()) {
		return false;
	}
	
	const config* filter_location = cfg.child("filter_location");
	if(filter_location) {
		wassert(map_ != NULL);
		wassert(gamestatus_ != NULL);
		wassert(units_ != NULL);
		bool res = map_->terrain_matches_filter(loc,*filter_location,*gamestatus_,*units_,use_flat_tod);
		if(res == false) {
			return false;
		}
	}
	
	const std::string& this_type = id();

	//the type could be a comma-seperated list of types
	if(type.empty() == false && type != this_type) {

		//we only do the full CSV search if we find a comma in there,
		//and if the subsequence is found within the main sequence. This
		//is because doing the full CSV split is expensive
		if(std::find(type.begin(),type.end(),',') != type.end() &&
		   std::search(type.begin(),type.end(),this_type.begin(),
			           this_type.end()) != type.end()) {
			const std::vector<std::string>& vals = utils::split(type);

			if(std::find(vals.begin(),vals.end(),this_type) == vals.end()) {
				return false;
			}
		} else {
			return false;
		}
	}

	if(ability.empty() == false && get_ability_bool(ability,loc) == false) {
		return false;
	}

	if(race.empty() == false && race_->name() != race) {
		return false;
	}

	if(gender.empty() == false) {
		const unit_race::GENDER gender_type = gender == "female" ? unit_race::FEMALE : unit_race::MALE;
		if(gender_type != this->gender()) {
			return false;
		}
	}

	if(side.empty() == false && this->side() != (unsigned)atoi(side.c_str()))
	  {
		if(std::find(side.begin(),side.end(),',') != side.end()) {
			const std::vector<std::string>& vals = utils::split(side);

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
			if(i->id() == weapon) {
				has_weapon = true;
			}
		}

		if(!has_weapon)
			return false;
	}

	if(role.empty() == false && role_ != role) {
		return false;
	}

	if (canrecruit.empty() == false && (canrecruit == "1") != can_recruit())
		return false;

	if(level.empty() == false && level_ != lexical_cast_default<int>(level,-1)) {
		return false;
	}

	//if there are [not] tags below this tag, it means that the filter
	//should not match if what is in the [not] tag does match
	const config::child_list& negatives = cfg.get_children("not");
	for(config::child_list::const_iterator not_it = negatives.begin(); not_it != negatives.end(); ++not_it) {
		if(matches_filter(**not_it,loc)) {
			return false;
		}
	}

	return true;
}
void unit::add_overlay(const std::string& overlay)
{
	overlays_.push_back(overlay);
}
void unit::remove_overlay(const std::string& overlay)
{
	overlays_.erase(std::remove(overlays_.begin(),overlays_.end(),overlay),overlays_.end());
}
const std::vector<std::string>& unit::overlays() const
{
	return overlays_;
}
/**
* Initializes this unit from a cfg object.
*
* \param cfg  Configuration object from which to read the unit
*/
void unit::read(const config& cfg)
{
	cfg_ = cfg;
	wassert(gamedata_ != NULL);
	std::map<std::string,unit_type>::const_iterator i = gamedata_->unit_types.find(cfg["type"]);
	if(cfg["id"]=="") {
		id_ = cfg["type"];
	} else {
		id_ = cfg["id"];
	}
	side_ = lexical_cast_default<int>(cfg["side"]);
	if(side_ <= 0) {
		side_ = 1;
	}

	validate_side(side_);
	
	const std::string& gender = cfg["gender"];
	if(gender == "male") {
		gender_ = unit_race::MALE;
	} else if(gender == "female") {
		gender_ = unit_race::FEMALE;
	}

	variation_ = cfg["variation"];
	
	wassert(gamedata_ != NULL);
	const race_map::const_iterator race_it = gamedata_->races.find(cfg["race"]);
	if(race_it != gamedata_->races.end()) {
		race_ = &race_it->second;
	} else {
		static const unit_race dummy_race;
		race_ = &dummy_race;
	}
	traits_description_ = cfg["traits_description"];
	if(cfg["random_traits"] == "yes") {
		generate_traits();
	}

	description_ = cfg["user_description"];
	if(cfg["generate_description"] == "yes") {
		description_ = generate_description();
	}
	underlying_description_ = cfg["description"];
	if(description_.empty()) {
		description_ = underlying_description_;
	}

	custom_unit_description_ = cfg["unit_description"];
	
	role_ = cfg["role"];
	overlays_ = utils::split(cfg["overlays"]);
	if(overlays_.size() == 1 && overlays_.front() == "") {
		overlays_.clear();
	}
	const config* const variables = cfg.child("variables");
	if(variables != NULL) {
		variables_ = *variables;
		cfg_.remove_child("variables",0);
	} else {
		variables_.clear();
	}
	hit_points_ = lexical_cast_default<int>("hitpoints");
	max_hit_points_ = lexical_cast_default<int>("max_hitpoints");
	goto_.x = lexical_cast_default<int>(cfg["goto_x"]) - 1;
	goto_.y = lexical_cast_default<int>(cfg["goto_y"]) - 1;
	movement_ = lexical_cast_default<int>("moves");
	max_movement_ = lexical_cast_default<int>("max_moves");
	experience_ = lexical_cast_default<int>("experience");
	max_experience_ = lexical_cast_default<int>("max_experience");
	resting_ = (cfg["resting"] == "yes");
	unrenamable_ = (cfg["unrenamable"] == "yes");
	
	advances_to_ = utils::split(cfg["advances_to"]);
	if(advances_to_.size() == 1 && advances_to_.front() == "") {
		advances_to_.clear();
	}
	
	name_ = cfg["name"];
	language_name_ = cfg["language_name"];
	undead_variation_ = cfg["undead_variation"];
	variation_ = cfg["variation"];
	
	if(cfg["alignment"]=="lawful") {
		alignment_ == unit_type::LAWFUL;
	} else if(cfg["alignment"]=="neutral") {
		alignment_ == unit_type::NEUTRAL;
	} else if(cfg["alignment"]=="chaotic") {
		alignment_ == unit_type::CHAOTIC;
	} else {
		alignment_ == unit_type::NEUTRAL;
	}
	flag_rgb_ = string2rgb(cfg["flag_rgb"]);
	alpha_ = lexical_cast_default<fixed_t>(cfg["alpha"]);
	
	level_ = lexical_cast_default<int>(cfg["level"]);
	upkeep_ = lexical_cast_default<int>(cfg["upkeep"],level_);
	
	std::string f = cfg["facing"];
	if(f=="north") {
		facing_dir_ = NORTH;
	} else if(f=="north_east") {
		facing_dir_ = NORTH_EAST;
	} else if(f=="south_east") {
		facing_dir_ = SOUTH_EAST;
	} else if(f=="south") {
		facing_dir_ = SOUTH;
	} else if(f=="south_west") {
		facing_dir_ = SOUTH_WEST;
	} else if(f=="north_west") {
		facing_dir_ = NORTH_WEST;
	} else if(f=="left") {
		facing_dir_ = LEFT;
	} else if(f=="right") {
		facing_dir_ = RIGHT;
	}
	recruits_ = utils::split(cfg["recruits"]);
	if(recruits_.size() == 1 && recruits_.front() == "") {
		recruits_.clear();
	}
	attacks_left_ = lexical_cast_default<int>(cfg["attacks_left"]);
	max_attacks_ = lexical_cast_default<int>(cfg["max_attacks"]);
	emit_zoc_ = lexical_cast_default<int>(cfg["zoc"]);
	unit_value_ = lexical_cast_default<int>(cfg["value"]);
	const config* mod_desc = cfg.child("modifications_description");
	if(mod_desc) {
		for(string_map::const_iterator k = mod_desc->values.begin(); k != mod_desc->values.end(); ++k) {
			modification_descriptions_[k->first] = k->second;
		}
		cfg_.remove_child("modifications_description",0);
	}
	const config* mods = cfg.child("modifications");
	if(mods) {
		modifications_ = *mods;
		cfg_.remove_child("modifications",0);
	}
	
	if(cfg["type"] != "") {
		advance_to(&i->second);
		max_attacks_ = 1;
	}
	
}
void unit::write(config& cfg) const
{
	cfg = cfg_;
	cfg["type"] = "";
	cfg["id"] = id();

	std::stringstream hp;
	hp << hit_points_;
	cfg["hitpoints"] = hp.str();
	std::stringstream hpm;
	hpm << max_hit_points_;
	cfg["max_hitpoints"] = hpm.str();

	std::stringstream xp;
	xp << experience_;
	cfg["experience"] = xp.str();
	std::stringstream xpm;
	xpm << max_experience_;
	cfg["max_experience"] = xpm.str();

	std::stringstream sd;
	sd << side_;
	cfg["side"] = sd.str();

	cfg["gender"] = gender_ == unit_race::MALE ? "male" : "female";

	cfg["variation"] = variation_;

	cfg["role"] = role_;

	config status_flags;
	for(std::map<std::string,std::string>::const_iterator st = states_.begin(); st != states_.end(); ++st) {
		status_flags[st->first] = st->second;
	}

	cfg.add_child("variables",variables_);
	cfg.add_child("states",status_flags);

	cfg["overlays"] = utils::join(overlays_);

	cfg["user_description"] = description_;
	cfg["description"] = underlying_description_;

	cfg["traits_description"] = traits_description_;

	if(can_recruit())
		cfg["canrecruit"] = "1";

	cfg.add_child("modifications",modifications_);

	static const std::string dirnames[8] = {"north","north_east","south_east","south","south_west","north_west","left","right"};
	cfg["facing"] = dirnames[facing_dir_];

	std::stringstream upk;
	upk << upkeep_;
	cfg["upkeep"] = upk.str();

	char buf[50];
	snprintf(buf,sizeof(buf),"%d",goto_.x+1);
	cfg["goto_x"] = buf;
	snprintf(buf,sizeof(buf),"%d",goto_.y+1);
	cfg["goto_y"] = buf;

	snprintf(buf,sizeof(buf),"%d",movement_);
	cfg["moves"] = buf;
	snprintf(buf,sizeof(buf),"%d",max_movement_);
	cfg["max_moves"] = buf;

	cfg["resting"] = resting_ ? "yes" : "no";
	
	cfg["advances_to"] = utils::join(advances_to_);
	
	cfg["race"] = race_->name();
	cfg["name"] = name_;
	cfg["language_name"] = language_name_;
	cfg["undead_variation"] = undead_variation_;
	cfg["variation"] = variation_;
	cfg["level"] = lexical_cast_default<std::string>(level_);
	switch(alignment_) {
		case unit_type::LAWFUL:
			cfg["alignment"] = "lawful";
			break;
		case unit_type::NEUTRAL:
			cfg["alignment"] = "neutral";
			break;
		case unit_type::CHAOTIC:
			cfg["alignment"] = "chaotic";
			break;
		default:
			cfg["alignment"] = "neutral";
	}
	std::stringstream flg_rgb;
	for(std::vector<Uint32>::const_iterator j = flag_rgb_.begin(); j != flag_rgb_.end(); ++j) {
		flg_rgb << *j;
		if(j+1 != flag_rgb_.end()) {
			flg_rgb << ",";
		}
	}
	cfg["flag_rgb"] = flg_rgb.str();
	cfg["unrenamable"] = unrenamable_ ? "yes" : "no";
	cfg["gender"] = gender_ == unit_race::FEMALE ? "female" : "male";
	cfg["alpha"] = lexical_cast_default<std::string>(alpha_);
	
	cfg["recuits"] = utils::join(recruits_);
	cfg["attacks_left"] = lexical_cast_default<std::string>(attacks_left_);
	cfg["max_attacks"] = lexical_cast_default<std::string>(max_attacks_);
	cfg["zoc"] = lexical_cast_default<std::string>(emit_zoc_);
	for(std::vector<attack_type>::const_iterator i = attacks_.begin(); i != attacks_.end(); ++i) {
		cfg.add_child("attack",i->get_cfg());
	}
	cfg["value"] = lexical_cast_default<std::string>(unit_value_);
	cfg["cost"] = lexical_cast_default<std::string>(unit_value_);
	config mod_desc;
	for(string_map::const_iterator k = modification_descriptions_.begin(); k != modification_descriptions_.end(); ++k) {
		mod_desc[k->first] = k->second;
	}
	cfg.add_child("modifications_description",mod_desc);
	cfg.add_child("modifications",modifications_);
	
	
}

void unit::assign_role(const std::string& role)
{
	role_ = role;
}
const std::vector<attack_type>& unit::attacks() const
{
	return attacks_;
}
std::vector<attack_type>& unit::attacks()
{
	return attacks_;
}

int unit::damage_from(const attack_type& attack,bool attacker,const gamemap::location& loc) const
{
	return resistance_against(attack,attacker,loc);
}

const std::string& unit::image() const
{
	switch(state_) {
		case STATE_STANDING:
			return absolute_image();
		case STATE_WALKING: 
		case STATE_DEFENDING: {
			const std::string& res = anim_->get_current_frame().image;
			if(res != "") {
				return res;
			} else { 
				if(anim_->animation_finished()) 
					return anim_->get_last_frame().image;
				else
					return anim_->get_first_frame().image;
			}
		}
		case STATE_ATTACKING: {
			if(attackType_ == NULL)
				return absolute_image();

			const std::string& img = attackType_->animation(true).first->get_current_frame().image;
			if (img.empty())
				return image_fighting(attackType_->range_type());
			else
				return img;
		}
		case STATE_HEALING:
			return image_healing();
		case STATE_LEADING:
			return image_leading();

		default: return absolute_image();
	}
}
const image::locator unit::image_loc() const
{
  if(flag_rgb().size()){
    return(image::locator(image(),team_rgb_range(),flag_rgb()));
  }else{
    return(image::locator(image()));
  }
}
void unit::refresh_unit(display& disp,const int& x,const int& y,const double& submerge)
{
		gamemap::location hex = disp.hex_clicked_on(x+disp.hex_size()/2,y+disp.hex_size()/2);
		gamemap::location adjacent[6];
		get_adjacent_tiles(hex, adjacent);
		
		surface image(image::get_image(image_loc()));
		if (!facing_left()) {
			image.assign(image::reverse_image(image));
		}
		disp.draw_tile(hex.x, hex.y);
		for(int tile = 0; tile != 6; ++tile) {
			disp.draw_tile(adjacent[tile].x, adjacent[tile].y);
		}
		disp.draw_unit(x, y, image, false, ftofxp(1.0), 0, 0.0, submerge);
		if(!unit_halo_ && !image_halo().empty()) {
			unit_halo_ = halo::add(0,0,image_halo());
		}
		if(unit_halo_) {
			int d = disp.hex_size() / 2;
			halo::set_location(unit_halo_, x+ d, y+ d);
		}

		disp.update_display();
		events::pump();
}

void unit::set_standing()
{
	state_ = STATE_STANDING;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
}
void unit::set_defending(bool hits, std::string range, int start_frame, int acceleration)
{
	update_frame();
	state_ =  STATE_DEFENDING;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	anim_ =  new defensive_animation(defend_animation(hits,range));
	anim_->start_animation(start_frame,1,acceleration);
	anim_->update_current_frame();
}
void unit::update_frame()
{
	if (!anim_) return;
	anim_->update_current_frame();
	if( anim_->animation_finished()) {
		delete anim_;
		anim_ = NULL;	
		set_standing();
	}
}
void unit::set_attacking( const attack_type* type, int ms)
{
	state_ =  STATE_ATTACKING;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	attackType_ = type;
	attackingMilliseconds_ = ms;
}

void unit::set_leading()
{
	state_ = STATE_LEADING;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
}
void unit::set_healing()
{
	state_ = STATE_HEALING;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
}
void unit::set_walking(const std::string terrain,gamemap::location::DIRECTION dir,int acceleration)
{
	update_frame();
	movement_animation* const anim = dynamic_cast<movement_animation*>(anim_);
	if(state_ == STATE_WALKING && anim != NULL && anim->matches(terrain,dir) >=0) {
		return; // finish current animation, don't start a new one
	}
	state_ = STATE_WALKING;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	anim_ = new movement_animation(move_animation(terrain,dir));
	anim_->start_animation(anim_->get_first_frame_time(),1,acceleration);
	anim_->update_current_frame();
}

bool unit::facing_left() const
{
	return facing_dir_ == LEFT;
}
void unit::set_facing(FACING newval)
{
	facing_dir_ = newval;
}

const t_string& unit::traits_description() const
{
	return traits_description_;
}

int unit::value() const
{
	return unit_value_;
}
int unit::cost() const
{
	return unit_value_;
}

const gamemap::location& unit::get_goto() const
{
	return goto_;
}
void unit::set_goto(const gamemap::location& new_goto)
{
	goto_ = new_goto;
}

int unit::upkeep() const
{
	return upkeep_;
}

bool unit::is_flying() const
{
	return flying_;
}
int unit::movement_cost(gamemap::TERRAIN terrain, int recurse_count) const
{
	const int impassable = 10000000;
	
//	const std::map<gamemap::TERRAIN,int>::const_iterator i = movement_costs_.find(terrain);
//	if(i != movement_costs_.end()) {
//		return i->second;
//	}
	
	wassert(map_ != NULL);
	//if this is an alias, then select the best of all underlying terrains
	const std::string& underlying = map_->underlying_mvt_terrain(terrain);
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
			const int value = movement_cost(*i,recurse_count+1);
			if(value < ret_value && !revert) {
				ret_value = value;
			} else if(value > ret_value && revert) {
				ret_value = value;
			}
		}

//		movement_costs_.insert(std::pair<gamemap::TERRAIN,int>(terrain,ret_value));

		return ret_value;
	}

	const config* movement_costs = cfg_.child("movement_costs");

	int res = -1;

	if(movement_costs != NULL) {
		if(underlying.size() != 1) {
			LOG_STREAM(err, config) << "terrain '" << terrain << "' has " << underlying.size() << " underlying names - 0 expected\n";
			return impassable;
		}

		const std::string& id = map_->get_terrain_info(underlying[0]).id();

		const std::string& val = (*movement_costs)[id];

		if(val != "") {
			res = atoi(val.c_str());
		}
	}

	if(res <= 0) {
		res = impassable;
	}

//	movement_costs_.insert(std::pair<gamemap::TERRAIN,int>(terrain,res));

	return res;
}

int unit::defense_modifier(gamemap::TERRAIN terrain, int recurse_count) const
{
//	const std::map<gamemap::TERRAIN,int>::const_iterator i = defense_mods_.find(terrain);
//	if(i != defense_mods_.end()) {
//		return i->second;
//	}

	wassert(map_ != NULL);
	//if this is an alias, then select the best of all underlying terrains
	const std::string& underlying = map_->underlying_mvt_terrain(terrain);
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
			const int value = defense_modifier(*i,recurse_count+1);
			if(value < ret_value && !revert) {
				ret_value = value;
			} else if(value > ret_value && revert) {
				ret_value = value;
			}
			if(value < ret_value) {
				ret_value = value;
			}
		}

//		defense_mods_.insert(std::pair<gamemap::TERRAIN,int>(terrain,ret_value));

		return ret_value;
	}

	int res = -1;

	const config* const defense = cfg_.child("defense");

	if(defense != NULL) {
		if(underlying.size() != 1) {
			LOG_STREAM(err, config) << "terrain '" << terrain << "' has " << underlying.size() << " underlying names - 0 expected\n";
			return 100;
		}

		const std::string& id = map_->get_terrain_info(underlying[0]).id();
		const std::string& val = (*defense)[id];

		if(val != "") {
			res = atoi(val.c_str());
		}
	}

	if(res <= 0) {
		res = 50;
	}

//	defense_mods_.insert(std::pair<gamemap::TERRAIN,int>(terrain,res));

	return res;
}

bool unit::resistance_filter_matches(const config& cfg,const gamemap::location& loc,bool attacker,const attack_type& damage_type) const
{
	if(!(cfg["active_on"]=="" || (attacker && cfg["active_on"]=="offense") || (!attacker && cfg["active_on"]=="defense"))) {
		return false;
	}
	const std::string& apply_to = cfg["apply_to"];
	const std::string& damage_name = damage_type.type();
	if(!apply_to.empty()) {
		if(damage_name != apply_to) {
			if(std::find(apply_to.begin(),apply_to.end(),',') != apply_to.end() &&
				std::search(apply_to.begin(),apply_to.end(),
				damage_name.begin(),damage_name.end()) != apply_to.end()) {
				const std::vector<std::string>& vals = utils::split(apply_to);
				if(std::find(vals.begin(),vals.end(),damage_name) == vals.end()) {
					return false;
				}
			} else {
				return false;
			}
		}
	}
	int res = 100;
	const config* const resistance = cfg_.child("resistance");
	if(resistance != NULL) {
		const std::string& val = (*resistance)[damage_name];
		if(val != "") {
			res = lexical_cast_default<int>(val);
		}
	}
	res = 100 - res;
	const config* const apply_filter = cfg.child("filter_apply");
	if(apply_filter) {
		if((*apply_filter)["type"] == "value") {
			if((*apply_filter)["equals"] != "" && lexical_cast_default<int>((*apply_filter)["equals"]) != res) {
				return false;
			}
			if((*apply_filter)["not_equals"] != "" && lexical_cast_default<int>((*apply_filter)["not_equals"]) == res) {
				return false;
			}
			if((*apply_filter)["less_than"] != "" && lexical_cast_default<int>((*apply_filter)["less_than"]) >= res) {
				return false;
			}
			if((*apply_filter)["greater_than"] != "" && lexical_cast_default<int>((*apply_filter)["greater_than"]) <= res) {
				return false;
			}
			if((*apply_filter)["greater_than_equal_to"] != "" && lexical_cast_default<int>((*apply_filter)["greater_than_equal_to"]) < res) {
				return false;
			}
			if((*apply_filter)["less_than_equal_to"] != "" && lexical_cast_default<int>((*apply_filter)["less_than_equal_to"]) > res) {
				return false;
			}
		}
	}
	return true;
}


int unit::resistance_against(const attack_type& damage_type,bool attacker,const gamemap::location& loc) const
{
	int res = 100;
	
	const std::string& damage_name = damage_type.type();
	const config* const resistance = cfg_.child("resistance");
	if(resistance != NULL) {
		const std::string& val = (*resistance)[damage_name];
		if(val != "") {
			res = lexical_cast_default<int>(val);
		}
	}
	res = 100 - res;
	
	int set_to = 0;
	bool set_to_set = false;
	int add_cum = 0;
	int add_ncum = 0;
	int mul_cum = 1;
	int mul_ncum = 1;
	int max_value = -100000;
	int min_value = 100000;
	bool max_set = false;
	bool min_set = false;
	
	unit_ability_list resistance_abilities = get_abilities("resistance",loc);
	for(std::vector<std::pair<config*,gamemap::location> >::iterator i = resistance_abilities.cfgs.begin(); i != resistance_abilities.cfgs.end(); ++i) {
		if(resistance_filter_matches(*i->first,loc,attacker,damage_type)) {
			if((*i->first)["cumulative"] == "yes") {
				if((*i->first)["value"]!="") {
					if(set_to_set) {
						set_to = maximum<int>(set_to,lexical_cast_default<int>((*i->first)["value"]));
					} else {
						set_to = lexical_cast_default<int>((*i->first)["value"]);
						set_to_set = true;
					}
				}
				add_cum += lexical_cast_default<int>((*i->first)["add"]);
				mul_cum *= lexical_cast_default<int>((*i->first)["multiply"],1);
			} else {
				if((*i->first)["value"]!="") {
					if(set_to_set) {
						set_to = maximum<int>(set_to,lexical_cast_default<int>((*i->first)["value"]));
					} else {
						set_to = lexical_cast_default<int>((*i->first)["value"]);
						set_to_set = true;
					}
				}
				add_ncum = maximum<int>(add_ncum,lexical_cast_default<int>((*i->first)["add"]));
				mul_ncum = maximum<int>(mul_ncum,lexical_cast_default<int>((*i->first)["multiply"],1));
			}
			if((*i->first)["min_value"]!="") {
				if(min_set) {
					min_value = minimum<int>(min_value,lexical_cast_default<int>((*i->first)["min_value"]));
				} else {
					min_value = lexical_cast_default<int>((*i->first)["min_value"]);
					min_set = true;
				}
			}
			if((*i->first)["max_value"]!="") {
				if(max_set) {
					max_value = maximum<int>(max_value,lexical_cast_default<int>((*i->first)["max_value"]));
				} else {
					max_value = lexical_cast_default<int>((*i->first)["max_value"]);
					max_set = true;
				}
			}
		}
	}
	if(set_to_set) {
		res = set_to;
	}
	res *= maximum<int>(mul_cum,mul_ncum);
	res += maximum<int>(add_cum,add_ncum);
	
	if(min_set) {
		res = maximum<int>(min_value,res);
	}
	if(max_set) {
		res = minimum<int>(max_value,res);
	}

	return 100 - res;
}
#if 0
std::map<gamemap::TERRAIN,int> unit::movement_type() const
{
	return movement_costs_;
}
#endif

bool unit::can_advance() const
{
	return advances_to_.empty()==false || get_modification_advances().empty() == false;
}

std::map<std::string,std::string> unit::advancement_icons() const
{
  std::map<std::string,std::string> temp;
  std::string image;
  if(can_advance()){
    if(advances_to_.empty()==false){
      std::stringstream tooltip;
      image=game_config::level_image;
      std::vector<std::string> adv=advances_to();
      for(std::vector<std::string>::const_iterator i=adv.begin();i!=adv.end();i++){
	if((*i).size()){
	  tooltip<<(*i).c_str()<<"\n";
	}
      }
      temp[image]=tooltip.str();
    }
    const config::child_list mods=get_modification_advances();
    for(config::child_list::const_iterator i = mods.begin(); i != mods.end(); ++i) {
      image=(**i)["image"];
      if(image.size()){
	std::stringstream tooltip;
	tooltip<<temp[image];
	std::string tt=(**i)["description"];
	if(tt.size()){
	  tooltip<<tt<<"\n";
	}
	temp[image]=tooltip.str();
      }
    }
  }
  return(temp);
}
std::vector<std::pair<std::string,std::string> > unit::amla_icons() const
{
  std::vector<std::pair<std::string,std::string> > temp;
  std::pair<std::string,std::string> icon; //<image,tooltip>

  const config::child_list& advances = get_modification_advances();
  for(config::child_list::const_iterator i = advances.begin(); i != advances.end(); ++i) {
    icon.first=(**i)["icon"];
    icon.second=(**i)["description"];

    for(unsigned int j=0;j<(modification_count("advance",(**i)["id"]));j++) {

      temp.push_back(icon);
    }
  }
  return(temp);
}

void unit::reset_modifications()
{
	max_hit_points_ = max_hit_points_b_;
	max_experience_ = max_experience_b_;
	max_movement_ = max_movement_b_;
	attacks_ = attacks_b_;
}
void unit::backup_state()
{
	max_hit_points_b_ = max_hit_points_;
	max_experience_b_ = max_experience_;
	max_movement_b_ = max_movement_;
	attacks_b_ = attacks_;
}

const config::child_list& unit::modification_advancements() const
{
	return cfg_.get_children("advancement");
}

config::child_list unit::get_modification_advances() const
{
	config::child_list res;
	const config::child_list& advances = modification_advancements();
	for(config::child_list::const_iterator i = advances.begin(); i != advances.end(); ++i) {
		if(modification_count("advance",(**i)["id"]) < lexical_cast_default<size_t>((**i)["max_times"],1)) {
		  std::vector<std::string> temp = utils::split((**i)["require_amla"]);
		  if(temp.size()){
			std::sort(temp.begin(),temp.end());
		    std::vector<std::string> uniq;
			std::unique_copy(temp.begin(),temp.end(),std::back_inserter(uniq));
		    for(std::vector<std::string>::const_iterator ii = uniq.begin(); ii != uniq.end(); ii++){
			  int required_num = std::count(temp.begin(),temp.end(),*ii);
		      int mod_num = modification_count("advance",*ii);
		      if(required_num<=mod_num){
			res.push_back(*i);		  
		      }
		    }
		  }else{
		    res.push_back(*i);
		  }
		}
	}

	return res;
}

size_t unit::modification_count(const std::string& type, const std::string& id) const
{
	size_t res = 0;
	const config::child_list& items = modifications_.get_children(type);
	for(config::child_list::const_iterator i = items.begin(); i != items.end(); ++i) {
		if((**i)["id"] == id) {
			++res;
		}
	}

	return res;
}

void unit::add_modification(const std::string& type, const config& mod,
                    bool no_add)
{
	if(no_add == false) {
		modifications_.add_child(type,mod);
	}

	std::vector<t_string> effects_description;

	for(config::const_child_itors i = mod.child_range("effect");
	    i.first != i.second; ++i.first) {

		//see if the effect only applies to certain unit types
		const std::string& type_filter = (**i.first)["unit_type"];
		if(type_filter.empty() == false) {
			const std::vector<std::string>& types = utils::split(type_filter);
			if(std::find(types.begin(),types.end(),id()) == types.end()) {
				continue;
			}
		}

		t_string description;

		const std::string& apply_to = (**i.first)["apply_to"];

		//apply variations -- only apply if we are adding this
		//for the first time.
		if(apply_to == "variation" && no_add == false) {
			variation_ = (**i.first)["name"];
			wassert(gamedata_ != NULL);
			const game_data::unit_type_map::const_iterator var = gamedata_->unit_types.find(id());
			advance_to(&var->second);
//			type_ = &type_->get_variation(variation_);
//			reset_modifications();
//			apply_modifications();
//			wassert("not done" == "done");
		} else if(apply_to == "new_attack") {
			attacks_.push_back(attack_type(**i.first,id(),image_fighting((**i.first)["range"]=="ranged" ? attack_type::LONG_RANGE : attack_type::SHORT_RANGE)));
		} else if(apply_to == "attack") {

			bool first_attack = true;

			for(std::vector<attack_type>::iterator a = attacks_.begin();
			    a != attacks_.end(); ++a) {
				std::string desc;
				const bool affected = a->apply_modification(**i.first,&desc,0);
				if(affected && desc != "") {
					if(first_attack) {
						first_attack = false;
					} else {
						description += t_string(N_("; "), "wesnoth");
					}

					description += t_string(a->name(), "wesnoth") + " " + desc;
				}
			}
		} else if(apply_to == "hitpoints") {
			LOG_UT << "applying hitpoint mod..." << hit_points_ << "/" << max_hit_points_ << "\n";
			const std::string& increase_hp = (**i.first)["increase"];
			const std::string& heal_full = (**i.first)["heal_full"];
			const std::string& increase_total = (**i.first)["increase_total"];
			const std::string& set_hp = (**i.first)["set"];
			const std::string& set_total = (**i.first)["set_total"];

			//if the hitpoints are allowed to end up greater than max hitpoints
			const std::string& violate_max = (**i.first)["violate_maximum"];
			
			if(set_hp.empty() == false) {
				if(set_hp[set_hp.size()-1] == '%') {
					hit_points_ = lexical_cast_default<int>(set_hp)*max_hit_points_/100;
				} else {
					hit_points_ = lexical_cast_default<int>(set_hp);
				}
			}
			if(set_total.empty() == false) {
				if(set_total[set_total.size()-1] == '%') {
					max_hit_points_ = lexical_cast_default<int>(set_total)*max_hit_points_/100;
				} else {
					max_hit_points_ = lexical_cast_default<int>(set_total);
				}
			}
			
			if(increase_total.empty() == false) {
				description += (increase_total[0] != '-' ? "+" : "") + increase_total +
					" " + t_string(N_("HP"), "wesnoth");

				//a percentage on the end means increase by that many percent
				max_hit_points_ = utils::apply_modifier(max_hit_points_, increase_total);
			}

			if(max_hit_points_ < 1)
				max_hit_points_ = 1;

			if(heal_full.empty() == false && heal_full != "no") {
				heal_all();
			}

			if(increase_hp.empty() == false) {
				hit_points_ = utils::apply_modifier(hit_points_, increase_hp);
			}

			LOG_UT << "modded to " << hit_points_ << "/" << max_hit_points_ << "\n";
			if(hit_points_ > max_hit_points_ && violate_max.empty()) {
				LOG_UT << "resetting hp to max\n";
				hit_points_ = max_hit_points_;
			}

			if(hit_points_ < 1)
				hit_points_ = 1;
		} else if(apply_to == "movement") {
			const std::string& increase = (**i.first)["increase"];
			const std::string& set_to = (**i.first)["set"];

			if(increase.empty() == false) {
				description += (increase[0] != '-' ? "+" : "") + increase +
					" " + t_string(N_("Moves"), "wesnoth");

				max_movement_ = utils::apply_modifier(max_movement_, increase, 1);
			}

			if(set_to.empty() == false) {
				max_movement_ = atoi(set_to.c_str());
			}

			if(movement_ > max_movement_)
				movement_ = max_movement_;
		} else if(apply_to == "max_experience") {
			const std::string& increase = (**i.first)["increase"];

			if(increase.empty() == false) {
				description += (increase[0] != '-' ? "+" : "") +
					increase + " " +
					t_string(N_("XP to advance"), "wesnoth");

				max_experience_ = utils::apply_modifier(max_experience_, increase);
			}

			if(max_experience_ < 1) {
				max_experience_ = 1;
			}
		} else if(apply_to == "loyal") {
			if(upkeep_ > 0)
				upkeep_ = 0;
		} else if(apply_to == "status") {
			const std::string& add = (**i.first)["add"];
			const std::string& remove = (**i.first)["remove"];

			if(add.empty() == false) {
				set_state(add,"true");
			}

			if(remove.empty() == false) {
				set_state(remove,"");
			}
		}

		if(!description.empty())
			effects_description.push_back(description);
	}

	t_string& description = modification_descriptions_[type];

	// Punctuation should be translatable: not all languages use latin punctuation.
	// (however, there maybe is a better way to do it)
	description += mod["name"];
	if (!mod["description"].empty())
		description += t_string(N_(": "), "wesnoth") + mod["description"];
	description += " ";

	if(effects_description.empty() == false) {
		description += t_string(N_("("), "wesnoth");
		for(std::vector<t_string>::const_iterator i = effects_description.begin();
				i != effects_description.end(); ++i) {
			description += *i;
			if(i+1 != effects_description.end())
				description += t_string(N_("; "), "wesnoth");
		}
		description += t_string(N_(")"), "wesnoth");
	}

	description += "\n";
}

const t_string& unit::modification_description(const std::string& type) const
{
	const string_map::const_iterator i = modification_descriptions_.find(type);
	if(i == modification_descriptions_.end()) {
		static const t_string empty_string;
		return empty_string;
	} else {
		return i->second;
	}
}

bool unit::move_interrupted() const
{
	return movement_left() > 0 && interrupted_move_.x >= 0 && interrupted_move_.y >= 0;
}
const gamemap::location& unit::get_interrupted_move() const
{
	return interrupted_move_;
}
void unit::set_interrupted_move(const gamemap::location& interrupted_move)
{
	interrupted_move_ = interrupted_move;
}

unit::STATE unit::state() const
{
	return state_;
}

const std::string& unit::absolute_image() const
{
	return cfg_["image"];
}
const std::string& unit::image_halo() const
{
	return cfg_["halo"];
}
const std::string& unit::image_profile() const
{
	const std::string& val = cfg_["profile"];
	if(val.size() == 0)
		return image();
	else
		return val;
}
const std::string& unit::image_fighting(attack_type::RANGE range) const
{
	static const std::string short_range("image_short");
	static const std::string long_range("image_long");

	const std::string& str = range == attack_type::LONG_RANGE ?
	                                  long_range : short_range;
	const std::string& val = cfg_[str];

	if(!val.empty()) {
		return val;
	} else {
		return image();
	}
}
const std::string& unit::image_leading() const
{
	const std::string& val = cfg_["image_leading"];
	if(val.empty()) {
		return image();
	} else {
		return val;
	}
}
const std::string& unit::image_healing() const
{
	const std::string& val = cfg_["image_healing"];
	if(val.empty()) {
		return image();
	} else {
		return val;
	}
}
const std::string& unit::image_halo_healing() const
{
	return cfg_["image_halo_healing"];
}
const std::string& unit::get_hit_sound() const
{
	return cfg_["get_hit_sound"];
}
const std::string& unit::die_sound() const
{
	return cfg_["die_sound"];
}

const std::string& unit::usage() const
{
	return cfg_["usage"];
}
unit_type::ALIGNMENT unit::alignment() const
{
	return alignment_;
}
const std::string& unit::race() const
{
	return race_->name();
}

const defensive_animation& unit::defend_animation(bool hits, std::string range) const
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

	wassert(!options.empty());
	return *options[rand()%options.size()];
}
const unit_animation& unit::teleport_animation() const
{
	return teleport_animations_[rand() % teleport_animations_.size()];
}
const unit_animation* unit::extra_animation(std::string flag) const
{
	if (extra_animations_.count(flag) == 0) return NULL;
	std::multimap<std::string,unit_animation>::const_iterator index = extra_animations_.lower_bound(flag);
	int i = (rand()% extra_animations_.count(flag));
	for(int j = 0 ; j < i ; j++) {
		index++;
	}
	return &index->second;
}
const death_animation& unit::die_animation(const attack_type* attack) const
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

	wassert(!options.empty());
	return *options[rand()%options.size()];
}
const movement_animation& unit::move_animation(const std::string terrain,gamemap::location::DIRECTION dir) const
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

	wassert(!options.empty());
	return *options[rand()%options.size()];
}


/*
 *
 * [abilities]
 * ...
 * 
 * [heals]
 * 	value=4
 * 	max_value=8
 * 	cumulative=no
 * 	affect_allies=yes
 * 	name= _ "heals"
// * 	name_inactive=null
 * 	description=  _ "Heals:
Allows the unit to heal adjacent friendly units at the beginning of each turn.

A unit cared for by a healer may heal up to 4 HP per turn.
A poisoned unit cannot be cured of its poison by a healer, and must seek the care of a village or a unit that can cure."
// * 	description_inactive=null
 * 	icon="misc/..."
// * 	icon_inactive=null
 * 	[adjacent_description]
 * 		name= _ "heals"
// * 		name_inactive=null
 * 		description=  _ "Heals:
Allows the unit to heal adjacent friendly units at the beginning of each turn.

A unit cared for by a healer may heal up to 4 HP per turn.
A poisoned unit cannot be cured of its poison by a healer, and must seek the care of a village or a unit that can cure."
// * 		description_inactive=null
 * 		icon="misc/..."
// * 		icon_inactive=null
 * 	[/adjacent_description]
 * 	
 * 	affect_self=yes
 * 	[filter] // SUF
 * 		...
 * 	[/filter]
 * 	[filter_location]
 * 		terrain=f
 * 		tod=lawful
 * 	[/filter_location]
 * 	[filter_self] // SUF
 * 		...
 * 	[/filter_self]
 * 	[filter_adjacent] // SUF
 * 		adjacent=n,ne,nw
 * 		...
 * 	[/filter_adjacent]
 * 	[filter_adjacent_location]
 * 		adjacent=n,ne,nw
 * 		...
 * 	[/filter_adjacent]
 * 	[affect_adjacent]
 * 		adjacent=n,ne,nw
 * 		[filter] // SUF
 * 			...
 * 		[/filter]
 * 	[/affect_adjacent]
 * 	[affect_adjacent]
 * 		adjacent=s,se,sw
 * 		[filter] // SUF
 * 			...
 * 		[/filter]
 * 	[/affect_adjacent]
 * 	
 * [/heals]
 * 
 * ...
 * [/abilities]
 *
 */


bool unit::get_ability_bool(const std::string& ability, const gamemap::location& loc) const
{
	const config* abilities = cfg_.child("abilities");
	if(abilities) {
		const config::child_list& list = abilities->get_children(ability);
		for(config::child_list::const_iterator i = list.begin(); i != list.end(); ++i) {
			if(ability_active(ability,**i,loc) && ability_affects_self(ability,**i,loc)) {
				return true;
			}
		}
	}
	
	wassert(units_ != NULL);
	wassert(teams_ != NULL);
	gamemap::location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	for(int i = 0; i != 6; ++i) {
		const unit_map::const_iterator it = units_->find(adjacent[i]);
		if(it != units_->end() && 
					it->second.get_state("stoned") != "true") {
			const config* adj_abilities = it->second.cfg_.child("abilities");
			if(adj_abilities) {
				const config::child_list& list = adj_abilities->get_children(ability);
				for(config::child_list::const_iterator j = list.begin(); j != list.end(); ++j) {
					if(((**j)["affect_allies"]=="yes" && !(*teams_)[side()-1].is_enemy(it->second.side())) 
						|| ((**j)["affect_enemies"]=="yes" && (*teams_)[side()-1].is_enemy(it->second.side())) &&
							ability_active(ability,**j,loc) && ability_affects_adjacent(ability,**j,i,loc)) {
						return true;
					}
				}
			}
		}
	}

	
	return false;
}
unit_ability_list unit::get_abilities(const std::string& ability, const gamemap::location& loc) const
{
	unit_ability_list res;
	
	const config* abilities = cfg_.child("abilities");
	if(abilities) {
		const config::child_list& list = abilities->get_children(ability);
		for(config::child_list::const_iterator i = list.begin(); i != list.end(); ++i) {
			if(ability_active(ability,**i,loc) && ability_affects_self(ability,**i,loc)) {
				res.cfgs.push_back(std::pair<config*,gamemap::location>(*i,loc));
			}
		}
	}
	
	wassert(units_ != NULL);
	gamemap::location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	for(int i = 0; i != 6; ++i) {
		const unit_map::const_iterator it = units_->find(adjacent[i]);
		if(it != units_->end() && 
		it->second.get_state("stoned") != "true") {
			const config* adj_abilities = it->second.cfg_.child("abilities");
			if(adj_abilities) {
				const config::child_list& list = adj_abilities->get_children(ability);
				for(config::child_list::const_iterator j = list.begin(); j != list.end(); ++j) {
					if(((**j)["affect_allies"]=="yes" && !(*teams_)[side()-1].is_enemy(it->second.side())) 
						|| ((**j)["affect_enemies"]=="yes" && (*teams_)[side()-1].is_enemy(it->second.side())) &&
						ability_active(ability,**j,loc) && ability_affects_adjacent(ability,**j,i,loc)) {
						res.cfgs.push_back(std::pair<config*,gamemap::location>(*j,loc));
					}
				}
			}
		}
	}

	
	return res;
}

std::vector<std::string> unit::unit_ability_tooltips() const
{
	std::vector<std::string> res;
	
	const config* abilities = cfg_.child("abilities");
	if(abilities) {
		const config::child_map& list_map = abilities->all_children();
		for(config::child_map::const_iterator i = list_map.begin(); i != list_map.end(); ++i) {
			for(config::child_list::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
				res.push_back((**j)["name"].str());
				res.push_back((**j)["description"].str());
			}
		}
	}
	return res;
}

std::vector<std::string> unit::ability_tooltips(const gamemap::location& loc) const
{
	std::vector<std::string> res;
	
	const config* abilities = cfg_.child("abilities");
	if(abilities) {
		const config::child_map& list_map = abilities->all_children();
		for(config::child_map::const_iterator i = list_map.begin(); i != list_map.end(); ++i) {
			for(config::child_list::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
				if(ability_active(i->first,**j,loc)) {
					if((**j)["name"] != "") {
						res.push_back((**j)["name"].str());
						res.push_back((**j)["description"].str());
					}
				} else {
					if((**j)["name_inactive"] != "") {
						res.push_back((**j)["name_inactive"].str());
						res.push_back((**j)["description_inactive"].str());
					}
				}
			}
		}
	}
	
	wassert(units_ != NULL);
	gamemap::location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	for(int i = 0; i != 6; ++i) {
		const unit_map::const_iterator it = units_->find(adjacent[i]);
		if(it != units_->end() && 
		it->second.get_state("stoned") != "true") {
			const config* adj_abilities = it->second.cfg_.child("abilities");
			if(adj_abilities) {
				const config::child_map& list_map = adj_abilities->all_children();
				for(config::child_map::const_iterator k = list_map.begin(); k != list_map.end(); ++k) {
					for(config::child_list::const_iterator j = k->second.begin(); j != k->second.end(); ++j) {
					if(((**j)["affect_allies"]=="yes" && !(*teams_)[side()-1].is_enemy(it->second.side())) 
						|| ((**j)["affect_enemies"]=="yes" && (*teams_)[side()-1].is_enemy(it->second.side()))) {
							const config* adj_desc = (*j)->child("adjacent_description");
							if(ability_affects_adjacent(k->first,**j,i,loc)) {
								if(adj_desc) {
									if(ability_active(k->first,**j,loc)) {
										if((**j)["name"] != "") {
											res.push_back((**j)["name"].str());
											res.push_back((**j)["description"].str());
										}
									} else {
										if((**j)["name_inactive"] != "") {
											res.push_back((**j)["name_inactive"].str());
											res.push_back((**j)["description_inactive"].str());
										}
									}
								} else {
									if(ability_active(k->first,**j,loc)) {
										if((*adj_desc)["name"] != "") {
											res.push_back((*adj_desc)["name"].str());
											res.push_back((*adj_desc)["description"].str());
										}
									} else {
										if((*adj_desc)["name_inactive"] != "") {
											res.push_back((*adj_desc)["name_inactive"].str());
											res.push_back((*adj_desc)["description_inacive"].str());
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	
	return res;
}


bool unit::ability_active(const std::string& ability,const config& cfg,const gamemap::location& loc) const
{
	if(cfg.child("filter_self") != NULL) {
		if(!matches_filter(*cfg.child("filter_self"),loc,ability=="illuminates")) {
			return false;
		}
	}
	wassert(units_ != NULL);
	gamemap::location adjacent[6];
	get_adjacent_tiles(loc,adjacent);
	int index=-1;
	const config::child_list& adj_filt = cfg.get_children("filter_adjacent");
	for(config::child_list::const_iterator i = adj_filt.begin(); i != adj_filt.end(); ++i) {
		std::vector<std::string> dirs = utils::split((**i)["adjacent"]);
		if(dirs.size()==1 && dirs.front()=="") {	
		} else {
			for(std::vector<std::string>::const_iterator j = dirs.begin(); j != dirs.end(); ++j) {
				if(*j=="n") {
					index=0;
				} else if(*j=="ne") {
					index=1;
				} else if(*j=="se") {
					index=2;
				} else if(*j=="s") {
					index=3;
				} else if(*j=="sw") {
					index=4;
				} else if(*j=="nw") {
					index=5;
				} else {
					index=-1;
				}
				if(index != -1) {
					units_map::const_iterator unit = units_->find(adjacent[index]);
					if(unit == units_->end()) {
						return false;
					}
					if(!unit->second.matches_filter(**i,unit->first,ability=="illuminates")) {
						return false;
					}
				}
			}
		}
	}
	index=-1;
	const config::child_list& adj_filt_loc = cfg.get_children("filter_adjacent_location");
	for(config::child_list::const_iterator i = adj_filt_loc.begin(); i != adj_filt_loc.end(); ++i) {
		std::vector<std::string> dirs = utils::split((**i)["adjacent"]);
		if(dirs.size()==1 && dirs.front()=="") {	
		} else {
			for(std::vector<std::string>::const_iterator j = dirs.begin(); j != dirs.end(); ++j) {
				if(*j=="n") {
					index=0;
				} else if(*j=="ne") {
					index=1;
				} else if(*j=="se") {
					index=2;
				} else if(*j=="s") {
					index=3;
				} else if(*j=="sw") {
					index=4;
				} else if(*j=="nw") {
					index=5;
				} else {
					index=-1;
				}
				if(index != -1) {
					wassert(map_ != NULL);
					wassert(gamestatus_ != NULL);
					if(!map_->terrain_matches_filter(adjacent[index],**i,*gamestatus_,*units_,ability=="illuminates")) {
						return false;
					}
				}
			}
		}
	}
	return true;
}
bool unit::ability_affects_adjacent(const std::string& ability,const config& cfg,int dir,const gamemap::location& loc) const
{
//	wassert("not done" == "done");
	wassert(dir >=0 && dir <= 5);
	static const std::string adjacent_names[6] = {"n","ne","se","s","sw","nw"};
	const config::child_list& affect_adj = cfg.get_children("affect_adjacent");
	bool passed = false;
	for(config::child_list::const_iterator i = affect_adj.begin(); i != affect_adj.end(); ++i) {
		std::vector<std::string> dirs = utils::split((**i)["adjacent"]);
		if(std::find(dirs.begin(),dirs.end(),adjacent_names[dir]) != dirs.end()) {
			if((*i)->child("filter") != NULL) {
				if(matches_filter(*(*i)->child("filter"),loc,ability=="illuminates")) {
					passed = true;
				} else {
					return false;
				}
			} else {
				passed = true;
			}
		}
	}
	return passed;
}
bool unit::ability_affects_self(const std::string& ability,const config& cfg,const gamemap::location& loc) const
{
	if(cfg.child("filter")==NULL) {
		if(cfg["affect_self"] == "yes") {
			return true;
		} else {
			return false;
		}
	}
	if(cfg["affect_self"] == "yes") {
		return matches_filter(*cfg.child("filter"),loc,ability=="illuminates");
	} else {
		return false;
	}
}


void unit::apply_modifications()
{
	log_scope("apply mods");
	modification_descriptions_.clear();

	for(size_t i = 0; i != NumModificationTypes; ++i) {
		const std::string& mod = ModificationTypes[i];
		const config::child_list& mods = modifications_.get_children(mod);
		for(config::child_list::const_iterator j = mods.begin(); j != mods.end(); ++j) {
			log_scope("add mod");
			add_modification(ModificationTypes[i],**j,true);
		}
	}

	traits_description_ = "";

	std::vector< t_string > traits;
	config::child_list const &mods = modifications_.get_children("trait");
	for(config::child_list::const_iterator j = mods.begin(), j_end = mods.end(); j != j_end; ++j) {
		t_string const &name = (**j)["name"];
		if (!name.empty())
			traits.push_back(name);
	}

	std::vector< t_string >::iterator k = traits.begin(), k_end = traits.end();
	if (k != k_end) {
		// we want to make sure the traits always have a consistent ordering
	        // try out not sorting, since quick,resilient can give
		// different HP to resilient,quick so rather preserve order
		// std::sort(k, k_end);
		for(;;) {
			traits_description_ += *(k++);
			if (k == k_end) break;
			traits_description_ += ", ";
		}
	}
}














bool unit_ability_list::empty() const
{
	return cfgs.empty();
}

std::pair<int,gamemap::location> unit_ability_list::highest(const std::string& key, int def) const
{
	gamemap::location best_loc;
	int abs_max = def;
	int flat = def;
	int stack = 0;
	for(std::vector<std::pair<config*,gamemap::location> >::const_iterator i = cfgs.begin(); i != cfgs.end(); ++i) {
		if((*i->first)["cumulative"]=="yes") {
			stack += lexical_cast_default<int>((*i->first)[key]);
			if(lexical_cast_default<int>((*i->first)[key]) > abs_max) {
				abs_max = lexical_cast_default<int>((*i->first)[key]);
				best_loc = i->second;
			}
		} else {
			flat = maximum<int>(flat,lexical_cast_default<int>((*i->first)[key]));
			if(lexical_cast_default<int>((*i->first)[key]) > abs_max) {
				abs_max = lexical_cast_default<int>((*i->first)[key]);
				best_loc = i->second;
			}
		}
	}
	return std::pair<int,gamemap::location>(flat + stack,best_loc);
}
std::pair<int,gamemap::location> unit_ability_list::lowest(const std::string& key, int def) const
{
	gamemap::location best_loc;
	int abs_max = def;
	int flat = def;
	int stack = 0;
	for(std::vector<std::pair<config*,gamemap::location> >::const_iterator i = cfgs.begin(); i != cfgs.end(); ++i) {
		if((*i->first)["cumulative"]=="yes") {
			stack += lexical_cast_default<int>((*i->first)[key]);
			if(lexical_cast_default<int>((*i->first)[key]) < abs_max) {
				abs_max = lexical_cast_default<int>((*i->first)[key]);
				best_loc = i->second;
			}
		} else {
			flat = minimum<int>(flat,lexical_cast_default<int>((*i->first)[key]));
			if(lexical_cast_default<int>((*i->first)[key]) < abs_max) {
				abs_max = lexical_cast_default<int>((*i->first)[key]);
				best_loc = i->second;
			}
		}
	}
	return std::pair<int,gamemap::location>(flat + stack,best_loc);
}
























/* 
 * 
 * [special]
 * [swarm]
 * 	name= _ "swarm"
 * 	name_inactive= _ ""
 * 	description= _ ""
 * 	description_inactive= _ ""
 * 	cumulative=no
 * 	apply_to=self  #self,opponent,defender,attacker
 * 	#active_on=defend  .. offense
 * 	
 * 	attacks_max=4
 * 	attacks_min=2
 * 	
 * 	[filter_self] // SUF
 * 		...
 * 	[/filter_self]
 * 	[filter_opponent] // SUF
 * 	[filter_attacker] // SUF
 * 	[filter_defender] // SUF
 * 	[filter_adjacent] // SAUF
 * 	[filter_adjacent_location] // SAUF + locs
 * [/swarm]
 * [/special]
 * 
 */






bool attack_type::get_special_bool(const std::string& special,bool force) const
{
	const config* specials = cfg_.child("specials");
	if(specials) {
		const config::child_list& list = specials->get_children(special);
		for(config::child_list::const_iterator i = list.begin(); i != list.end(); ++i) {
			if(force || special_active(**i,true)) {
				return true;
			}
		}
	}
	if(!force && other_attack_ != NULL) {
		specials = other_attack_->cfg_.child("specials");
		if(specials) {
			const config::child_list& list = specials->get_children(special);
			for(config::child_list::const_iterator i = list.begin(); i != list.end(); ++i) {
				if(other_attack_->special_active(**i,false)) {
					return true;
				}
			}
		}
	}
	return false;
}
weapon_special_list attack_type::get_specials(const std::string& special) const
{
	weapon_special_list res;
	const config* specials = cfg_.child("specials");
	if(specials) {
		const config::child_list& list = specials->get_children(special);
		for(config::child_list::const_iterator i = list.begin(); i != list.end(); ++i) {
			if(special_active(**i,true)) {
				res.cfgs.push_back(*i);
			}
		}
	}
	if(other_attack_ != NULL) {
		specials = other_attack_->cfg_.child("specials");
		if(specials) {
			const config::child_list& list = specials->get_children(special);
			for(config::child_list::const_iterator i = list.begin(); i != list.end(); ++i) {
				if(other_attack_->special_active(**i,false)) {
					res.cfgs.push_back(*i);
				}
			}
		}
	}
	return res;
}
std::vector<std::string> attack_type::special_tooltips() const
{
	std::vector<std::string> res;
	const config* specials = cfg_.child("specials");
	if(specials) {
		const config::child_map& list_map = specials->all_children();
		for(config::child_map::const_iterator i = list_map.begin(); i != list_map.end(); ++i) {
			for(config::child_list::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
				if(special_active(**j,true)) {
					if((**j)["name"] != "") {
						res.push_back((**j)["name"]);
						res.push_back((**j)["description"]);
					}
				} else {
					if((**j)["name_inactive"] != "") {
						res.push_back((**j)["name_inactive"]);
						res.push_back((**j)["description_inactive"]);
					}
				}
			}
		}
	}
	return res;
}
std::string attack_type::weapon_specials(bool force) const
{
	std::string res;
	const config* specials = cfg_.child("specials");
	if(specials) {
		const config::child_map& list_map = specials->all_children();
		for(config::child_map::const_iterator i = list_map.begin(); i != list_map.end(); ++i) {
			for(config::child_list::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
				if(force || special_active(**j,true)) {
					if((**j)["name"] != "") {
						res += (**j)["name"];
						res += ",";
					}
				} else {
					if((**j)["name_inactive"] != "") {
						res += (**j)["name_inactive"];
						res += ",";
					}
				}
			}
		}
	}
	
	return res.substr(0,res.size()-1);
}



bool attack_type::special_active(const config& cfg,bool self) const
{
	unit_map::const_iterator att = unitmap_->find(aloc_);
	unit_map::const_iterator def = unitmap_->find(dloc_);
	wassert(unitmap_ != NULL);
	
	if(self) {
		if(!special_affects_self(cfg)) {
			return false;
		}
	} else {
		if(!special_affects_opponent(cfg)) {
			return false;
		}
	}
	
	if(attacker_) {
		if(cfg["active_on"] != "" && cfg["active_on"] != "attacker") {
			return false;
		}
		if(cfg.child("filter_self") != NULL) {
			if(att == unitmap_->end() || !att->second.matches_filter(*cfg.child("filter_self"),aloc_)) {
				return false;
			}
			if(cfg.child("filter_self")->child("filter_weapon") != NULL) {
				if(!matches_filter(*cfg.child("filter_self")->child("filter_weapon"),0,true)) {
					return false;
				}
			}
		}
		if(cfg.child("filter_opponent") != NULL) {
			if(def == unitmap_->end() || !def->second.matches_filter(*cfg.child("filter_opponent"),dloc_)) {
				return false;
			}
			
			if(cfg.child("filter_opponent")->child("filter_weapon") != NULL) {
				if(!other_attack_ || other_attack_->matches_filter(*cfg.child("filter_opponent")->child("filter_weapon"),0,true)) {
					return false;
				}
			}
		}
	} else {
		if(cfg["active_on"] != "" && cfg["active_on"] != "defender") {
			return false;
		}
		if(cfg.child("filter_self") != NULL) {
			if(def == unitmap_->end() || !def->second.matches_filter(*cfg.child("filter_self"),dloc_)) {
				return false;
			}
			if(cfg.child("filter_self")->child("filter_weapon") != NULL) {
				if(!matches_filter(*cfg.child("filter_self")->child("filter_weapon"),0,true)) {
					return false;
				}
			}
		}
		if(cfg.child("filter_opponent") != NULL) {
			if(att == unitmap_->end() || !att->second.matches_filter(*cfg.child("filter_opponent"),aloc_)) {
				return false;
			}
			if(cfg.child("filter_opponent")->child("filter_weapon") != NULL) {
				if(!other_attack_ || !other_attack_->matches_filter(*cfg.child("filter_opponent")->child("filter_weapon"),0,true)) {
					return false;
				}
			}
		}
	}
	if(cfg.child("filter_attacker") != NULL) {
		if(att == unitmap_->end() || !att->second.matches_filter(*cfg.child("filter_attacker"),aloc_)) {
			return false;
		}
		if(attacker_) {
			if(cfg.child("filter_attacker")->child("filter_weapon") != NULL) {
				if(!matches_filter(*cfg.child("filter_attacker")->child("filter_weapon"),0,true)) {
					return false;
				}
			}
		} else {
			if(cfg.child("filter_attacker")->child("filter_weapon") != NULL) {
				if(!other_attack_ || !other_attack_->matches_filter(*cfg.child("filter_attacker")->child("filter_weapon"),0,true)) {
					return false;
				}
			}
		}
	}
	if(cfg.child("filter_defender") != NULL) {
		if(def == unitmap_->end() || !def->second.matches_filter(*cfg.child("filter_defender"),dloc_)) {
			return false;
		}
		if(!attacker_) {
			if(cfg.child("filter_defender")->child("filter_weapon") != NULL) {
				if(!matches_filter(*cfg.child("filter_defender")->child("filter_weapon"),0,true)) {
					return false;
				}
			}
		} else {
			if(cfg.child("filter_defender")->child("filter_weapon") != NULL) {
				if(!other_attack_ || !other_attack_->matches_filter(*cfg.child("filter_defender")->child("filter_weapon"),0,true)) {
					return false;
				}
			}
		}
	}
	gamemap::location adjacent[6];
	if(attacker_) {
		get_adjacent_tiles(aloc_,adjacent);
	} else {
		get_adjacent_tiles(dloc_,adjacent);
	}
	int index=-1;
	const config::child_list& adj_filt = cfg.get_children("filter_adjacent");
	for(config::child_list::const_iterator i = adj_filt.begin(); i != adj_filt.end(); ++i) {
		std::vector<std::string> dirs = utils::split((**i)["adjacent"]);
		if(dirs.size()==1 && dirs.front()=="") {	
		} else {
			for(std::vector<std::string>::const_iterator j = dirs.begin(); j != dirs.end(); ++j) {
				if(*j=="n") {
					index=0;
				} else if(*j=="ne") {
					index=1;
				} else if(*j=="se") {
					index=2;
				} else if(*j=="s") {
					index=3;
				} else if(*j=="sw") {
					index=4;
				} else if(*j=="nw") {
					index=5;
				} else {
					index=-1;
				}
				if(index != -1) {
					units_map::const_iterator unit = unitmap_->find(adjacent[index]);
					if(unit == unitmap_->end()) {
						return false;
					}
					if(!unit->second.matches_filter(**i,unit->first)) {
						return false;
					}
				}
			}
		}
	}
	index=-1;
	const config::child_list& adj_filt_loc = cfg.get_children("filter_adjacent_location");
	for(config::child_list::const_iterator i = adj_filt_loc.begin(); i != adj_filt_loc.end(); ++i) {
		std::vector<std::string> dirs = utils::split((**i)["adjacent"]);
		if(dirs.size()==1 && dirs.front()=="") {	
		} else {
			for(std::vector<std::string>::const_iterator j = dirs.begin(); j != dirs.end(); ++j) {
				if(*j=="n") {
					index=0;
				} else if(*j=="ne") {
					index=1;
				} else if(*j=="se") {
					index=2;
				} else if(*j=="s") {
					index=3;
				} else if(*j=="sw") {
					index=4;
				} else if(*j=="nw") {
					index=5;
				} else {
					index=-1;
				}
				if(index != -1) {
					wassert(map_ != NULL);
					wassert(game_status_ != NULL);
					if(!map_->terrain_matches_filter(adjacent[index],**i,*game_status_,*unitmap_)) {
						return false;
					}
				}
			}
		}
	}
	return true;
}
bool attack_type::special_affects_opponent(const config& cfg) const
{
	if(cfg["apply_to"]=="opponent") {
		return true;
	}
	if(cfg["apply_to"]!="") {
		if(attacker_ && cfg["apply_to"] == "defender") {
			return true;
		} 
		if(!attacker_ && cfg["apply_to"] == "attacker") {
			return true;
		}
	}
	return false;
}
bool attack_type::special_affects_self(const config& cfg) const
{
	if(cfg["apply_to"]=="self" || cfg["apply_to"]=="") {
		return true;
	}
	if(attacker_ && cfg["apply_to"] == "attacker") {
		return true;
	} 
	if(!attacker_ && cfg["apply_to"] == "defender") {
		return true;
	}
	return false;
}
void attack_type::set_specials_context(const gamemap::location& aloc,const gamemap::location& dloc,
                              const game_data* gamedata, unit_map* unitmap, 
							  const gamemap* map, const gamestatus* game_status, 
							  const std::vector<team>* teams, bool attacker,attack_type* other_attack)
{
	aloc_ = aloc;
	dloc_ = dloc;
	gamedata_ = gamedata;
	unitmap_ = unitmap;
	map_ = map;
	game_status_ = game_status;
	teams_ = teams;
	attacker_ = attacker;
	other_attack_ = other_attack;
}

void attack_type::set_specials_context(const gamemap::location& loc,const unit& un)
{
	aloc_ = loc;
	dloc_ = gamemap::location();
	gamedata_ = un.gamedata_;
	unitmap_ = un.units_;
	map_ = un.map_;
	game_status_ = un.gamestatus_;
	teams_ = un.teams_;
	attacker_ = true;
	other_attack_ = NULL;
}


weapon_special_list& weapon_special_list::operator +(const weapon_special_list& w)
{
	std::copy(w.cfgs.begin(),w.cfgs.end(),std::insert_iterator<config::child_list>(cfgs,cfgs.end()));
	return *this;
}

bool weapon_special_list::empty() const
{
	return cfgs.empty();
}

int weapon_special_list::highest(const std::string& key, int def) const
{
	int flat = def;
	int stack = 0;
	for(config::child_list::const_iterator i = cfgs.begin(); i != cfgs.end(); ++i) {
		if((**i)["cumulative"]=="yes") {
			stack += lexical_cast_default<int>((**i)[key]);
		} else {
			flat = maximum<int>(flat,lexical_cast_default<int>((**i)[key]));
		}
	}
	return flat + stack;
}
int weapon_special_list::lowest(const std::string& key, int def) const
{
	int flat = def;
	int stack = 0;
	for(config::child_list::const_iterator i = cfgs.begin(); i != cfgs.end(); ++i) {
		if((**i)["cumulative"]=="yes") {
			stack += lexical_cast_default<int>((**i)[key]);
		} else {
			flat = minimum<int>(flat,lexical_cast_default<int>((**i)[key]));
		}
	}
	return flat + stack;
}



































































bool unit::invisible(const std::string& terrain, int lawful_bonus,
		const gamemap::location& loc,
		const unit_map& units,const std::vector<team>& teams) const
{
	bool is_inv = false;

	static const std::string hides("hides");
	if (get_state(hides)=="true") {
		is_inv = this->get_ability_bool("hides",loc);
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





void unit_temporary_state::reset()
{
	healing_flat_ = 0;
	healing_cumulative_ = 0;
}
void unit_temporary_state::take_healing(int value, bool cumulative)
{
	if(cumulative) {
		healing_cumulative_ += value;
	} else {
		healing_flat_ = maximum<int>(value,healing_flat_);
	}
}
int unit_temporary_state::total_healing()
{
	return healing_flat_ + healing_cumulative_;
}















int team_units(const unit_map& units, unsigned int side)
{
	int res = 0;
	for(unit_map::const_iterator i = units.begin(); i != units.end(); ++i) {
		if(i->second.side() == side) {
			++res;
		}
	}

	return res;
}

int team_upkeep(const unit_map& units, unsigned int side)
{
	int res = 0;
	for(unit_map::const_iterator i = units.begin(); i != units.end(); ++i) {
		if(i->second.side() == side) {
			res += i->second.upkeep();
		}
	}

	return res;
}

unit_map::const_iterator team_leader(unsigned int side, const unit_map& units)
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
			if(current_team.fogged(loc.x, loc.y)){
				return units.end();
			}
			if(current_team.is_enemy(u->second.side()) &&
					u->second.invisible(
						map.underlying_union_terrain(map[loc.x][loc.y]),lawful_bonus,
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
			if(current_team.fogged(loc.x, loc.y)){
				return units.end();
			}
			if(current_team.is_enemy(u->second.side()) &&
					u->second.invisible(
						map.underlying_union_terrain(map[loc.x][loc.y]),lawful_bonus,
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
	res.villages = tm.villages().size();
	res.expenses = maximum<int>(0,res.upkeep - res.villages);
	res.net_income = tm.income() - res.expenses;
	res.gold = tm.gold();
	return res;
}

std::string get_team_name(unsigned int side, const unit_map& units)
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
