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
#include "gamestatus.hpp"
#include "actions.hpp"
#include "sound.hpp"

//DEBUG
#include <iostream>
#include "serialization/parser.hpp"

#include <ctime>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>

#define LOG_UT LOG_STREAM(info, engine)

namespace {
	const std::string ModificationTypes[] = { "object", "trait", "advance" };
	const size_t NumModificationTypes = sizeof(ModificationTypes)/
	                                    sizeof(*ModificationTypes);
}

static bool compare_unit_values(unit const &a, unit const &b)
{
	const int lvla = a.type().level();
	const int lvlb = b.type().level();

	const std::string& namea = a.type().id();
	const std::string& nameb = b.type().id();

	const int xpa = a.max_experience() - a.experience();
	const int xpb = b.max_experience() - b.experience();

	return lvla > lvlb || lvla == lvlb && namea < nameb ||
	                      lvla == lvlb && namea == nameb && xpa < xpb;
}

void sort_units(std::vector< unit > &units)
{
	std::sort(units.begin(), units.end(), compare_unit_values);
}

//constructor for reading a unit
unit::unit(const game_data& data, const config& cfg) :
	state_(STATE_STANDING),
	moves_(0), user_end_turn_(false), facing_(gamemap::location::NORTH_EAST),
	resting_(false), hold_position_(false), recruit_(false),
	guardian_(false), upkeep_(UPKEEP_FREE),anim_(NULL),user_image_(""),
	unit_halo_(0),unit_anim_halo_(0),refreshing_(false),hidden_(false)
{
	read(data,cfg);
				
	
}

unit_race::GENDER unit::generate_gender(const unit_type& type, bool gen)
{
	const std::vector<unit_race::GENDER>& genders = type.genders();
	if(genders.empty() == false) {
		return gen ? genders[get_random()%genders.size()] : genders.front();
	} else {
		return unit_race::MALE;
	}
}
//constructor for creating a new unit
unit::unit(const unit_type* t, int side, bool use_traits, bool dummy_unit, unit_race::GENDER gender) :
               gender_(dummy_unit ? gender : generate_gender(*t,use_traits)),
	       type_(&t->get_gender_unit_type(gender_)), state_(STATE_STANDING),
	       hitpoints_(type_->hitpoints()),
	       maxHitpoints_(type_->hitpoints()),
	       backupMaxHitpoints_(type_->hitpoints()), experience_(0),
	       maxExperience_(type_->experience_needed()),
	       backupMaxExperience_(type_->experience_needed()),
	       side_(side), moves_(0),
	       user_end_turn_(false), facing_(gamemap::location::NORTH_EAST),
	       maxMovement_(type_->movement()),
	       backupMaxMovement_(type_->movement()),
	       resting_(false), hold_position_(false), recruit_(false),
	       attacks_(type_->attacks()),
	       backupAttacks_(type_->attacks()),
               guardian_(false), upkeep_(UPKEEP_FULL_PRICE),
               unrenamable_(false),anim_(NULL),user_image_(""),unit_halo_(0),
	       unit_anim_halo_(0),refreshing_(false),hidden_(false)
{
	//dummy units used by the 'move_unit_fake' command don't need to have a side.
	if(dummy_unit == false) validate_side(side_);

	if(use_traits) {
		//units that don't have traits generated are just generic
		//units, so they shouldn't get a description either.
		description_ = type_->generate_description();
		generate_traits();
		underlying_description_ = description_;
	}else{
	  underlying_description_ = type_->id();
	}
}

//constructor for advancing a unit from a lower level
unit::unit(const unit_type* t, const unit& u) :
	gender_(u.gender_), variation_(u.variation_),
	type_(&t->get_gender_unit_type(u.gender_).get_variation(u.variation_)),
	state_(STATE_STANDING),
	hitpoints_(u.hitpoints()),
	maxHitpoints_(type_->hitpoints()),
	backupMaxHitpoints_(type_->hitpoints()),
	experience_(0),
	maxExperience_(type_->experience_needed()),
	backupMaxExperience_(type_->experience_needed()),
	side_(u.side()), moves_(u.moves_),
	user_end_turn_(false), facing_(u.facing_),
	maxMovement_(type_->movement()),
	backupMaxMovement_(type_->movement()),
	resting_(u.resting_), hold_position_(u.hold_position_),
	underlying_description_(u.underlying_description_),
	description_(u.description_),profile_(u.profile_),
	 recruit_(u.recruit_),
	role_(u.role_), statusFlags_(u.statusFlags_),
	overlays_(u.overlays_), variables_(u.variables_),
	attacks_(type_->attacks()), backupAttacks_(type_->attacks()),
	modifications_(u.modifications_),
	traitsDescription_(u.traitsDescription_),
	guardian_(false), upkeep_(u.upkeep_),
	unrenamable_(u.unrenamable_),anim_(NULL),user_image_(u.user_image_),unit_halo_(0),
	unit_anim_halo_(0),refreshing_(false),hidden_(false)
{
	validate_side(side_);

	//apply modifications etc, refresh the unit
	apply_modifications();
	if(u.type().id()!=t->id()){
	  heal_all();
	}
	statusFlags_.clear();
}

unit::~unit()
{
	if(unit_halo_) {
		halo::remove(unit_halo_);
	}
	if(unit_anim_halo_) {
		halo::remove(unit_anim_halo_);
	}
	if(anim_) delete anim_;
}
void unit::generate_traits()
{
	if(!traitsDescription_.empty())
		return;

	//calculate the unit's traits
	std::vector<config*> candidate_traits = type().possible_traits();
	std::vector<config*> traits;

	const size_t num_traits = type().num_traits();
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
const std::string& unit::profile() const
{
	return profile_;
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
	if (!unrenamable_) {
		description_ = new_description;
	}
}

unsigned int unit::side() const
{
	return side_;
}

Uint32 unit::team_rgb() const
{
  return(team::get_side_rgb(side()));
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

bool unit::unrenamable() const
{
	return unrenamable_;
}

fixed_t unit::alpha() const
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
	return moves_ != ATTACKED;
}

void unit::set_movement(int moves)
{
	hold_position_ = false;
	user_end_turn_ = false;
	moves_ = moves;
}

//This does not mark the unit's turn as being done even if value is true. To do that, either call void unit_hold_position(), or set_user_end_turn(value).
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
	user_end_turn_ = value;
}

bool unit::user_end_turn() const
{
	return user_end_turn_;
}

void unit::set_attacked()
{
	moves_ = ATTACKED;
	set_hold_position(false);
}

void unit::unit_hold_position()
{
	hold_position_ = true;
	user_end_turn_ = true;
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
	user_end_turn_ = false;
	moves_ = total_movement();
	if(type().has_ability("hides"))
		set_flag("hides");
	if(stone())
		set_attacked();
	if (hold_position_) {
		user_end_turn_ = true;
	}
}

bool unit::move_interrupted() const {
	return movement_left() > 0 && interrupted_move_.x >= 0 && interrupted_move_.y >= 0;
}
const gamemap::location& unit::get_interrupted_move() const {
	return interrupted_move_;
}
void unit::set_interrupted_move(const gamemap::location& interrupted_move) {
	interrupted_move_ = interrupted_move;
}

void unit::end_turn()
{
	remove_flag("slowed");
	if((moves_ != total_movement()) && (moves_ != NOT_MOVED)){
		resting_ = false;
	}
	//clear interrupted move
	set_interrupted_move(gamemap::location());
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

SDL_Colour unit::hp_color() const{
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

int unit::experience() const
{
	return experience_;
}

SDL_Colour unit::xp_color() const{
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
  if(type().advances_to().size()){
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

int unit::max_experience() const
{
	return maxExperience_;
}

bool unit::get_experience(int xp)
{
	experience_ += xp;
	return advances();
}

bool unit::advances() const
{
	return experience_ >= max_experience() && can_advance();
}

bool unit::gets_hit(int damage)
{
	hitpoints_ -= damage;
	return hitpoints_ <= 0;
}

void unit::heal()
{
	heal(game_config::heal_amount);
}

void unit::heal(int amount)
{
	int max_hp = max_hitpoints();
	if (hitpoints_ < max_hp) {
		hitpoints_ += amount;
		if (hitpoints_ > max_hp)
			hitpoints_ = max_hp;
	}
}

void unit::heal_all()
{
	hitpoints_ = max_hitpoints();
}


bool unit::invisible(const std::string& terrain, int lawful_bonus,
		const gamemap::location& loc,
		const unit_map& units,const std::vector<team>& teams) const
{
	bool is_inv = false;

	static const std::string hides("hides");
	if (has_flag(hides)) {
		is_inv = this->type().hides_filter().matches_filter(terrain, lawful_bonus);
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

bool unit::slowed() const
{
	static const std::string slowed_str("slowed");
	return has_flag(slowed_str);
}

bool unit::stone() const
{
	static const std::string stone_str("stone");
	return has_flag(stone_str);
}

bool unit::incapacitated() const
{
	return stone();
}

bool unit::healable() const
{
	return !incapacitated();
}

bool unit::has_moved() const
{
	return this->movement_left() != this->total_movement();
}

bool unit::has_goto() const
{
        return this->get_goto().valid();
}

bool unit::emits_zoc() const
{
	return type().has_zoc() && stone() == false;
}

bool unit::matches_filter(const config& cfg) const
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
	if(profile.empty() == false && profile != profile_) {
		return false;
	}

	//allow 'speaker' as an alternative to description, since people use it so often
	if(speaker.empty() == false && speaker != this->underlying_description()) {
		return false;
	}

	const std::string& this_type = this->type().id();

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

	if(ability.empty() == false && this->type().has_ability(ability) == false) {
		return false;
	}

	if(race.empty() == false && this->type().race() != race) {
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

	if(level.empty() == false && this->type().level() != lexical_cast_default<int>(level,-1)) {
		return false;
	}

	//if there are [not] tags below this tag, it means that the filter
	//should not match if what is in the [not] tag does match
	const config::child_list& negatives = cfg.get_children("not");
	for(config::child_list::const_iterator not_it = negatives.begin(); not_it != negatives.end(); ++not_it) {
		if(matches_filter(**not_it)) {
			return false;
		}
	}

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

void unit::read(const game_data& data, const config& cfg)
{
	std::map<std::string,unit_type>::const_iterator i = data.unit_types.find(cfg["type"]);
	if(i != data.unit_types.end()) {
		type_ = &i->second;
	} else {
		//DEBUG
		::write(std::cerr, cfg);
		throw game::load_game_failed("Unit not found: '" + cfg["type"] + "'");
	}

	wassert(type_ != NULL);

	const std::string& gender = cfg["gender"];
	if(gender == "male") {
		gender_ = unit_race::MALE;
	} else if(gender == "female") {
		gender_ = unit_race::FEMALE;
	} else {
		const std::vector<unit_race::GENDER>& genders = type_->genders();
		if(genders.empty() == false) {
			gender_ = genders.front();
		} else {
			gender_ = unit_race::MALE;
		}
	}

	type_ = &type_->get_gender_unit_type(gender_);

	variation_ = cfg["variation"];
	type_ = &type_->get_variation(variation_);

	reset_modifications();

	side_ = atoi(cfg["side"].c_str());
	if(side_ <= 0) {
		side_ = 1;
	}

	validate_side(side_);
	
	traitsDescription_ = cfg["traits_description"];
	if(cfg["random_traits"] == "yes") {
		generate_traits();
	}

	description_ = cfg["user_description"];
	if(cfg["generate_description"] == "yes") {
		description_ = type_->generate_description();
	}
	user_image_ = cfg["image"];

	underlying_description_ = cfg["description"];
	if(description_.empty()) {
		description_ = underlying_description_;
	}

	custom_unit_description_ = cfg["unit_description"];

	const string_map::const_iterator recruit_itor = cfg.values.find("canrecruit");
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

	 facing_ = gamemap::location::parse_direction(cfg["facing"]);
	 if(facing_ == gamemap::location::NDIRECTIONS) facing_ = gamemap::location::NORTH_EAST;

	const std::string& ai_special = cfg["ai_special"];
	if(ai_special == "guardian") {
		guardian_ = true;
	}

	role_ = cfg["role"];

	statusFlags_.clear();
	const config* const status_flags = cfg.child("status");
	if(status_flags != NULL) {
		for(string_map::const_iterator i = status_flags->values.begin(); i != status_flags->values.end(); ++i) {
			if(i->second == "on") {
				statusFlags_.insert(i->first);
			}
		}
	}

	overlays_ = utils::split(cfg["overlays"]);
	if(overlays_.size() == 1 && overlays_.front() == "") {
		overlays_.clear();
	}

	const config* const variables = cfg.child("variables");
	if(variables != NULL) {
		variables_ = *variables;
	} else {
		variables_.clear();
	}

	// Set this to some sane value: add_modification clamps it when
	// altering maxHitpoints_, and if we don't set it valgrind
	// complains.  We actually set it below once maxHitpoints_ is stable.
	hitpoints_ = 1;
	const config* const modifications = cfg.child("modifications");
	if(modifications != NULL) {
		modifications_ = *modifications;
		apply_modifications();
	}
	
	const std::string& hitpoints = cfg["hitpoints"];
	hitpoints_ = lexical_cast_default<int>(hitpoints,maxHitpoints_);
	if(hitpoints_ > maxHitpoints_) {
		hitpoints_ = maxHitpoints_;
	}
	
	goto_.x = lexical_cast_default<int>(cfg["goto_x"]) - 1;
	goto_.y = lexical_cast_default<int>(cfg["goto_y"]) - 1;

	const std::string& moves = cfg["moves"];
	if(moves.empty())
		moves_ = total_movement();
	else
		moves_ = atoi(moves.c_str());

	const std::string& experience = cfg["experience"];
	if(experience.size() == 0)
		experience_ = 0;
	else
		experience_ = atoi(experience.c_str());

	resting_ = (cfg["resting"] == "yes");
	unrenamable_ = (cfg["unrenamable"] == "yes");
	
	profile_ = cfg["profile"];
}

void unit::write(config& cfg) const
{
	cfg["type"] = type_->id();

	std::stringstream hp;
	hp << hitpoints_;
	cfg["hitpoints"] = hp.str();

	std::stringstream xp;
	xp << experience_;
	cfg["experience"] = xp.str();

	std::stringstream sd;
	sd << side_;
	cfg["side"] = sd.str();

	cfg["gender"] = gender_ == unit_race::MALE ? "male" : "female";

	cfg["variation"] = variation_;

	cfg["role"] = role_;

	config status_flags;
	for(std::set<std::string>::const_iterator st = statusFlags_.begin(); st != statusFlags_.end(); ++st) {
		status_flags[*st] = "on";
	}

	cfg.add_child("variables",variables_);
	cfg.add_child("status",status_flags);

	cfg["overlays"] = utils::join(overlays_);

	cfg["user_description"] = description_;
	cfg["description"] = underlying_description_;
	cfg["unit_description"] = custom_unit_description_;
	cfg["image"] = user_image_;

	cfg["traits_description"] = traitsDescription_;

	if(can_recruit())
		cfg["canrecruit"] = "1";

	cfg.add_child("modifications",modifications_);

	cfg["facing"] = gamemap::location::write_direction(facing_);

	switch(upkeep_) {
	case UPKEEP_FULL_PRICE: cfg["upkeep"] = "full"; break;
	case UPKEEP_LOYAL: cfg["upkeep"] = "loyal"; break;
	case UPKEEP_FREE: cfg["upkeep"] = "free"; break;
	}

	if(guardian_) {
		cfg["ai_special"] = "guardian";
	}

	char buf[50];
	snprintf(buf,sizeof(buf),"%d",goto_.x+1);
	cfg["goto_x"] = buf;
	snprintf(buf,sizeof(buf),"%d",goto_.y+1);
	cfg["goto_y"] = buf;

	snprintf(buf,sizeof(buf),"%d",moves_);
	cfg["moves"] = buf;

	cfg["resting"] = resting_ ? "yes" : "no";
	cfg["unrenamable"] = unrenamable_ ? "yes" : "no";
	
	cfg["profile"] = profile_;
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

	if(slowed()) {
		return res*2;
	}

	return res;
}

int unit::defense_modifier(const gamemap& map, gamemap::TERRAIN terrain) const
{
	return type_->movement_type().defense_modifier(map,terrain);
}

int unit::damage_against(const attack_type& attack) const
{
	return type_->movement_type().damage_against(attack);
}


const surface unit::still_image() const
{
	image::locator  loc;
	if(type().flag_rgb().size()){
		loc = image::locator(absolute_image(),team_rgb_range(),type().flag_rgb());
	}else{
		loc = image::locator(absolute_image());
	}
	surface unit_image(image::get_image(loc,image::UNSCALED));
	return unit_image;
}

void unit::set_standing(const display &disp)
{
	state_ = STATE_STANDING;
	offset_=0;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	if(!user_image_.empty()) {
		anim_ = new unit_animation(user_image_);
	} else {
		anim_ = new unit_animation(type_->image());
	}
	anim_->start_animation(anim_->get_first_frame_time(),unit_animation::INFINITE_CYCLES,disp.turbo()?5:1);
	anim_->update_current_frame();
}

void unit::set_defending(const display &disp, int damage, std::string range)
{
	state_ =  STATE_DEFENDING;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	anim_ =  new defensive_animation(type_->defend_animation(damage > 0?true:false,range));
	
	// add a blink on damage effect
	int anim_time = anim_->get_last_frame_time();
	int damage_left = damage;
	const std::string my_image = anim_->get_last_frame().image;
	while(anim_time < 1000 && damage_left > 0 ) {
		anim_->add_frame(anim_time,unit_frame(my_image,"",anim_time,anim_time+30,display::rgb(255,255,255),1.0));
		anim_time += 30;
		damage_left --;
		anim_->add_frame(anim_time,unit_frame(my_image,"",anim_time,anim_time+30,display::rgb(255,255,255),0.0));
		anim_time += 30;
		damage_left --;
	}
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	anim_->update_current_frame();
}

void unit::set_extra_anim(const display &disp, std::string flag)
{
	state_ =  STATE_EXTRA;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	if(!type_->extra_animation(flag)) {
		set_standing(disp);
		return;
	}
	anim_ =  new unit_animation(*(type_->extra_animation(flag)));
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	anim_->update_current_frame();
}


const unit_animation & unit::set_attacking(const display &disp,bool hit,const attack_type& type)
{
	state_ =  STATE_ATTACKING;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	const attack_type::attack_animation &attack_anim = type.animation(hit,facing_) ;
	anim_ =  new unit_animation(attack_anim.animation);
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	anim_->update_current_frame();
	return attack_anim.missile_animation;
}

void unit::set_leading(const display &disp)
{
	state_ = STATE_LEADING;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	anim_ = new unit_animation(type().image_leading());
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	anim_->update_current_frame();
}

void unit::set_leveling_in(const display &disp)
{
	state_ = STATE_LEVELIN;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	std::string my_image;
	if(! user_image_.empty()) {
		my_image = user_image_;
	} else {
		my_image = type_->image();
	}
	anim_ = new unit_animation();
	// add a fade in effect
	double blend_ratio =1;
	int anim_time =0;
	while(blend_ratio > 0) {
		anim_->add_frame(anim_time,unit_frame(my_image,"",anim_time,anim_time+10,display::rgb(255,255,255),blend_ratio));
		blend_ratio -=0.015;
		anim_time += 10;
	}
	
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	anim_->update_current_frame();
}

void unit::set_leveling_out(const display &disp)
{
	state_ = STATE_LEVELOUT;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	std::string my_image;
	if(! user_image_.empty()) {
		my_image = user_image_;
	} else {
		my_image = type_->image();
	}
	anim_ = new unit_animation();
	// add a fade out effect
	double blend_ratio =0;
	int anim_time =0;
	while(blend_ratio < 1) {
		anim_->add_frame(anim_time,unit_frame(my_image,"",anim_time,anim_time+10,display::rgb(255,255,255),blend_ratio));
		blend_ratio +=0.015;
		anim_time += 10;
	}
	
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	anim_->update_current_frame();
}

void unit::set_recruited(const display &disp)
{
	state_ = STATE_RECRUITED;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	anim_ = new unit_animation();
	// add a fade in effect
	double blend_ratio =0;
	int anim_time =0;
	while(blend_ratio < 1) {
		anim_->add_frame(anim_time,unit_frame(type_->image(),"",anim_time,anim_time+10,0,0,ftofxp(blend_ratio)));
		blend_ratio +=0.015;
		anim_time += 10;
	}
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	anim_->update_current_frame();
}

void unit::set_healed(const display &disp, int healing)
{
	state_ = STATE_HEALED;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	anim_ = new unit_animation(type_->image());
	// add a blink on heal effect
	int anim_time = anim_->get_last_frame_time();
	int heal_left = healing;
	const std::string my_image = anim_->get_last_frame().image;
	while(anim_time < 1000 && heal_left > 0 ) {
		anim_->add_frame(anim_time,unit_frame(my_image,"",anim_time,anim_time+30,display::rgb(255,255,255),1.0));
		anim_time += 30;
		heal_left --;
		anim_->add_frame(anim_time,unit_frame(my_image,"",anim_time,anim_time+30,display::rgb(255,255,255),0.0));
		anim_time += 30;
		heal_left --;
	}
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	anim_->update_current_frame();
}

void unit::set_poisoned(const display &disp, int damage)
{
	state_ = STATE_POISONED;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	anim_ = new unit_animation(type_->image());
	// add a blink on damage effect
	int anim_time = anim_->get_last_frame_time();
	int damage_left = damage;
	const std::string my_image = anim_->get_last_frame().image;
	while(anim_time < 1000 && damage_left > 0 ) {
		anim_->add_frame(anim_time,unit_frame(my_image,"",anim_time,anim_time+30,display::rgb(0,255,0),1.0));
		anim_time += 30;
		damage_left --;
		anim_->add_frame(anim_time,unit_frame(my_image,"",anim_time,anim_time+30,display::rgb(0,255,0),0.0));
		anim_time += 30;
		damage_left --;
	}
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	anim_->update_current_frame();
}

void unit::set_teleporting(const display &disp)
{
	state_ = STATE_TELEPORT;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	anim_ =  new unit_animation(type_->teleport_animation());
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	anim_->update_current_frame();
}

void unit::set_dying(const display &disp, const attack_type* attack)
{
	state_ = STATE_DYING;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	anim_ =  new death_animation(type_->die_animation(attack));
	double blend_ratio = 1.0;
	std::string tmp_image = anim_->get_last_frame().image;
	int anim_time =anim_->get_last_frame_time();
	while(blend_ratio > 0.0) {
		anim_->add_frame(anim_time,unit_frame(tmp_image,"",anim_time,anim_time+10,0,0,ftofxp(blend_ratio)));
		blend_ratio -=0.015;
		anim_time += 10;
	}
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	anim_->update_current_frame();
}
void unit::set_healing(const display &disp)
{
	state_ = STATE_HEALING;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	int duration =0;
	const std::vector<std::pair<std::string,int> > halos = unit_frame::prepare_halo(type().image_halo_healing(),0,0);
	std::vector<std::pair<std::string,int> >::const_iterator cur_halo;
	for(cur_halo = halos.begin() ; cur_halo != halos.end() ; cur_halo++) {
		duration += cur_halo->second;
	}
	duration = maximum<int>(200,duration);
	anim_ = new unit_animation(type().image_healing(),0,duration,"",type().image_halo_healing(),0);
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	anim_->update_current_frame();
}

void unit::set_walking(const display &disp, const std::string terrain)
{
	movement_animation* const anim = dynamic_cast<movement_animation*>(anim_);
	if(state_ == STATE_WALKING && anim != NULL && anim->matches(terrain,facing_) >=0) {
		return; // finish current animation, don't start a new one
	}
	state_ = STATE_WALKING;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	anim_ = new movement_animation(type_->move_animation(terrain,facing_));
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	anim_->update_current_frame();
}



gamemap::location::DIRECTION unit::facing() const
{
	return facing_;
}

void unit::set_facing(gamemap::location::DIRECTION dir)
{
	wassert(dir != gamemap::location::NDIRECTIONS);
	facing_ = dir;
}

const t_string& unit::traits_description() const
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

bool unit::can_advance() const
{
	return type().can_advance() || get_modification_advances().empty() == false;
}

std::map<std::string,std::string> unit::advancement_icons() const
{
  std::map<std::string,std::string> temp;
  std::string image;
  if(can_advance()){
    if(type().can_advance()){
      std::stringstream tooltip;
      image=game_config::level_image;
      std::vector<std::string> adv=type().advances_to();
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

  const config::child_list& advances = type().modification_advancements();
  for(config::child_list::const_iterator i = advances.begin(); i != advances.end(); ++i) {
    icon.first=(**i)["icon"];
    icon.second=(**i)["description"];

    for(unsigned int j=0;j<(modification_count("advance",(**i)["id"]));j++) {

      temp.push_back(icon);
    }
  }
  return(temp);
}

config::child_list unit::get_modification_advances() const
{
	config::child_list res;
	const config::child_list& advances = type().modification_advancements();
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

void unit::add_modification(const std::string& type,
                            const config& mod, bool no_add)
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
			if(std::find(types.begin(),types.end(),this->type().id()) == types.end()) {
				continue;
			}
		}

		t_string description;

		const std::string& apply_to = (**i.first)["apply_to"];

		//apply variations -- only apply if we are adding this
		//for the first time.
		if(apply_to == "variation" && no_add == false) {
			variation_ = (**i.first)["name"];
			type_ = &type_->get_variation(variation_);
			reset_modifications();
			apply_modifications();
		} else if(apply_to == "new_attack") {
			attacks_.push_back(attack_type(**i.first,this->type()));
		} else if(apply_to == "attack") {

			bool first_attack = true;

			for(std::vector<attack_type>::iterator a = attacks_.begin();
			    a != attacks_.end(); ++a) {
				std::string desc;
				const bool affected = a->apply_modification(**i.first,&desc);
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
			LOG_UT << "applying hitpoint mod..." << hitpoints_ << "/" << maxHitpoints_ << "\n";
			const std::string& increase_hp = (**i.first)["increase"];
			const std::string& heal_full = (**i.first)["heal_full"];
			const std::string& increase_total = (**i.first)["increase_total"];
			const std::string& set_hp = (**i.first)["set"];
			const std::string& set_total = (**i.first)["set_total"];

			//if the hitpoints are allowed to end up greater than max hitpoints
			const std::string& violate_max = (**i.first)["violate_maximum"];
			
			if(set_hp.empty() == false) {
				if(set_hp[set_hp.size()-1] == '%') {
					hitpoints_ = lexical_cast_default<int>(set_hp)*maxHitpoints_/100;
				} else {
					hitpoints_ = lexical_cast_default<int>(set_hp);
				}
			}
			if(set_total.empty() == false) {
				if(set_total[set_total.size()-1] == '%') {
					maxHitpoints_ = lexical_cast_default<int>(set_total)*maxHitpoints_/100;
				} else {
					maxHitpoints_ = lexical_cast_default<int>(set_total);
				}
			}
			
			if(increase_total.empty() == false) {
				description += (increase_total[0] != '-' ? "+" : "") + increase_total +
					" " + t_string(N_("HP"), "wesnoth");

				//a percentage on the end means increase by that many percent
				maxHitpoints_ = utils::apply_modifier(maxHitpoints_, increase_total);
			}

			if(maxHitpoints_ < 1)
				maxHitpoints_ = 1;

			if(heal_full.empty() == false && heal_full != "no") {
				heal_all();
			}

			if(increase_hp.empty() == false) {
				hitpoints_ = utils::apply_modifier(hitpoints_, increase_hp);
			}

			LOG_UT << "modded to " << hitpoints_ << "/" << maxHitpoints_ << "\n";
			if(hitpoints_ > maxHitpoints_ && violate_max.empty()) {
				LOG_UT << "resetting hp to max\n";
				hitpoints_ = maxHitpoints_;
			}

			if(hitpoints_ < 1)
				hitpoints_ = 1;
		} else if(apply_to == "movement") {
			const std::string& increase = (**i.first)["increase"];
			const std::string& set_to = (**i.first)["set"];

			if(increase.empty() == false) {
				description += (increase[0] != '-' ? "+" : "") + increase +
					" " + t_string(N_("Moves"), "wesnoth");

				maxMovement_ = utils::apply_modifier(maxMovement_, increase, 1);
			}

			if(set_to.empty() == false) {
				maxMovement_ = atoi(set_to.c_str());
			}

			if(moves_ > maxMovement_)
				moves_ = maxMovement_;
		} else if(apply_to == "max_experience") {
			const std::string& increase = (**i.first)["increase"];

			if(increase.empty() == false) {
				description += (increase[0] != '-' ? "+" : "") +
					increase + " " +
					t_string(N_("XP to advance"), "wesnoth");

				maxExperience_ = utils::apply_modifier(maxExperience_, increase);
			}

			if(maxExperience_ < 1) {
				maxExperience_ = 1;
			}
		} else if(apply_to == "loyal") {
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

		if(!description.empty())
			effects_description.push_back(description);
	}

	t_string& description = modificationDescriptions_[type];

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

void unit::reset_modifications()
{
	attacks_ = type_->attacks();
	backupAttacks_ = attacks_;
	maxHitpoints_ = type_->hitpoints();
	backupMaxHitpoints_ = type_->hitpoints();
	maxMovement_ = type_->movement();
	backupMaxMovement_ = type_->movement();
	maxExperience_ = type_->experience_needed();
	backupMaxExperience_ = type_->experience_needed();
}

void unit::apply_modifications()
{
	log_scope("apply mods");
	modificationDescriptions_.clear();

	for(size_t i = 0; i != NumModificationTypes; ++i) {
		const std::string& mod = ModificationTypes[i];
		const config::child_list& mods = modifications_.get_children(mod);
		for(config::child_list::const_iterator j = mods.begin(); j != mods.end(); ++j) {
			log_scope("add mod");
			add_modification(ModificationTypes[i],**j,true);
		}
	}

	traitsDescription_ = "";

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
			traitsDescription_ += *(k++);
			if (k == k_end) break;
			traitsDescription_ += ", ";
		}
	}
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

const t_string& unit::modification_description(const std::string& type) const
{
	const string_map::const_iterator i = modificationDescriptions_.find(type);
	if(i == modificationDescriptions_.end()) {
		static const t_string empty_string;
		return empty_string;
	} else {
		return i->second;
	}
}

int unit::upkeep() const
{
	switch(upkeep_) {
	case UPKEEP_FREE: return 0;
	case UPKEEP_LOYAL: return 0;
	case UPKEEP_FULL_PRICE: return type().level();
	default: wassert(false); return 0;
	}
}

bool unit::is_flying() const
{
	return type().movement_type().is_flying();
}

void unit::restart_animation(const display& disp,int start_time) {
	if(!anim_) return;
	anim_->start_animation(start_time,1,disp.turbo()?5:1);
}

void unit::refresh_unit(display& disp,gamemap::location hex,bool with_status)
{
	const gamemap & map = disp.get_map();
	if(hidden_) { 
		if(unit_halo_) halo::remove(unit_halo_);
		unit_halo_ = 0;
		if(unit_anim_halo_) halo::remove(unit_anim_halo_);
		unit_anim_halo_ = 0;
		return;
	}
	if(refreshing_) return;
	if(disp.fogged(hex.x,hex.y)) { return;}
	if(invisible(map.underlying_union_terrain(map[hex.x][hex.y]),
				disp.get_game_status().get_time_of_day().lawful_bonus,hex,
				disp.get_units(),disp.get_teams()) &&
			disp.get_teams()[disp.playing_team()].is_enemy(side())) {
		return;
	}
	refreshing_ = true;
	const gamemap::location dst= hex.get_direction(facing());
	const double xsrc = disp.get_location_x(hex);
	const double ysrc = disp.get_location_y(hex);
	const double xdst = disp.get_location_x(dst);
	const double ydst = disp.get_location_y(dst);

	const int x = int(offset_*xdst + (1.0-offset_)*xsrc);
	const int y = int(offset_*ydst + (1.0-offset_)*ysrc);

	if(!anim_) set_standing(disp);
	const gamemap::TERRAIN terrain = map.get_terrain(hex);
	const double submerge = is_flying() ? 0.0 : map.get_terrain_info(terrain).unit_submerge();
	const int height_adjust = is_flying() ? 0 : int(map.get_terrain_info(terrain).unit_height_adjust() * disp.zoom());

	std::string image_name;
	unit_frame current_frame;
	if(anim_->animation_finished()) current_frame = anim_->get_last_frame();
	else if(anim_->get_first_frame_time() > anim_->get_animation_time()) current_frame = anim_->get_first_frame();
	else current_frame = anim_->get_current_frame();

	image_name = current_frame.image;
	if(anim_->frame_changed() ) {
		if(!current_frame.sound.empty()) {
			sound::play_sound(current_frame.sound);
		}

	}
	if(unit_anim_halo_) halo::remove(unit_anim_halo_);
	unit_anim_halo_ = 0;
	if(!current_frame.halo.empty()) {
		int time = current_frame.begin_time;
		unsigned int sub_halo = 0;
		while(time < anim_->get_animation_time()&& sub_halo < current_frame.halo.size()) {
			time += current_frame.halo[sub_halo].second;
			sub_halo++;

		}
		if(sub_halo >= current_frame.halo.size()) sub_halo = current_frame.halo.size() -1;


		if(facing_ == gamemap::location::NORTH_WEST || facing_ == gamemap::location::SOUTH_WEST) {
			const int d = disp.hex_size() / 2;
			unit_anim_halo_ = halo::add(x+d-current_frame.halo_x,
					y+d+current_frame.halo_y,
					current_frame.halo[sub_halo].first);
		} else {
			const int d = disp.hex_size() / 2;
			unit_anim_halo_ = halo::add(x+d+current_frame.halo_x,
					y+d+current_frame.halo_y,
					current_frame.halo[sub_halo].first,
					halo::REVERSE);
		}
	}
	if(image_name.empty()) {
		image_name = type_->image();
	}
	image::locator  loc;
	if(type().flag_rgb().size()){
		loc = image::locator(image_name,team_rgb_range(),type().flag_rgb());
	}else{
		loc = image::locator(image_name);
	}
	surface image(image::get_image(loc,stone()?image::GREYED : image::UNSCALED));
	if(image ==NULL) {
		image = still_image();
	}
	if(facing_ == gamemap::location::NORTH_WEST || facing_ == gamemap::location::SOUTH_WEST) {
		image.assign(image::reverse_image(image));
	}

	Uint32 blend_with = current_frame.blend_with;
	double blend_ratio = current_frame.blend_ratio;
	fixed_t highlight_ratio = minimum<fixed_t>(type().alpha(),current_frame.highlight_ratio);
	if(invisible(map.underlying_union_terrain(map[hex.x][hex.y]),
				disp.get_game_status().get_time_of_day().lawful_bonus,hex,
				disp.get_units(),disp.get_teams()) &&
			highlight_ratio > ftofxp(0.5)) {
		highlight_ratio = ftofxp(0.5);
	}
	if(hex == disp.selected_hex() && highlight_ratio == ftofxp(1.0)) {
		highlight_ratio = ftofxp(1.5);
	}

	if (poisoned() && blend_ratio == 0){
		blend_with = disp.rgb(0,255,0);
		blend_ratio = 0.25;
	}

	

	
	surface ellipse_front(NULL);
	surface ellipse_back(NULL);
	if(preferences::show_side_colours() && with_status) {
		const char* const selected = disp.selected_hex() == hex ? "selected-" : "";
		std::vector<Uint32> temp_rgb;
		//ellipse not pure red=255!
		for(int i=255;i>100;i--){
			temp_rgb.push_back((Uint32)(i<<16));
		}
		//selected ellipse not pure red at all!
		char buf[100];
		std::string ellipse=type().image_ellipse();
		if(ellipse.empty()){
			ellipse="misc/ellipse";
		}	
		snprintf(buf,sizeof(buf),"%s-%stop.png",ellipse.c_str(),selected);
		ellipse_back.assign(image::get_image(image::locator(buf,team_rgb_range(),temp_rgb)));
		snprintf(buf,sizeof(buf),"%s-%sbottom.png",ellipse.c_str(),selected);
		ellipse_front.assign(image::get_image(image::locator(buf,team_rgb_range(),temp_rgb)));
	}

	gamemap::location adjacent[6];
	get_adjacent_tiles(hex, adjacent);
	disp.draw_tile(hex.x, hex.y);
	if(state_ != STATE_STANDING) {
		for(int tile = 0; tile != 6; ++tile) {
			disp.draw_tile(adjacent[tile].x, adjacent[tile].y);
		}
	}
	disp.draw_unit(x, y -height_adjust, image, false, highlight_ratio, 
			blend_with, blend_ratio, submerge,ellipse_back,ellipse_front);
	if(!unit_halo_ && !type().image_halo().empty()) {
		unit_halo_ = halo::add(0,0,type().image_halo());
	}
	if(unit_halo_) {
		const int d = disp.hex_size() / 2;
		halo::set_location(unit_halo_, x+ d, y -height_adjust+ d);
	}

	if(with_status) {
		const std::string* movement_file = NULL;
		const std::string* energy_file = &game_config::energy_image;
		const fixed_t bar_alpha = highlight_ratio < ftofxp(1.0) && blend_with == 0 ? highlight_ratio : (hex == disp.mouseover_hex() ? ftofxp(1.0): ftofxp(0.7));

		if(size_t(side()) != disp.playing_team()+1) {
			if(disp.team_valid() &&
			   disp.get_teams()[disp.playing_team()].is_enemy(side())) {
				movement_file = &game_config::enemy_ball_image;
			} else {
				movement_file = &game_config::ally_ball_image;
			}
		} else {
			if(disp.playing_team() == disp.playing_team() && movement_left() == total_movement() && !user_end_turn()) {
				movement_file = &game_config::unmoved_ball_image;
			} else if(disp.playing_team() == disp.playing_team() && unit_can_move(hex,disp.get_units(),map,disp.get_teams()) && !user_end_turn()) {
				movement_file = &game_config::partmoved_ball_image;
			} else {
				movement_file = &game_config::moved_ball_image;
			}
		}
		disp.draw_bar(*movement_file,x,y-height_adjust,0,0,hp_color(),bar_alpha);

		double unit_energy = 0.0;
		if(max_hitpoints() > 0) {
			unit_energy = double(hitpoints())/double(max_hitpoints());
		}
		disp.draw_bar(*energy_file,x-5,y-height_adjust,(max_hitpoints()*2)/3,unit_energy,hp_color(),bar_alpha);

		if(experience() > 0 && can_advance()) {
			const double filled = double(experience())/double(max_experience());
			const int level = maximum<int>(type().level(),1);

			SDL_Color colour=xp_color();
			disp.draw_bar(*energy_file,x,y-height_adjust,max_experience()/(level*2),filled,colour,bar_alpha);
		}

		if (can_recruit()) {
			surface crown(image::get_image("misc/leader-crown.png",image::SCALED,image::NO_ADJUST_COLOUR));
			if(!crown.null()) {
				//if(bar_alpha != ftofxp(1.0)) {
				//	crown = adjust_surface_alpha(crown, bar_alpha);
				//}

				SDL_Rect r = {0, 0, crown->w, crown->h};
				disp.video().blit_surface(x,y-height_adjust,crown,&r);
			}
		}

	}
	for(std::vector<std::string>::const_iterator ov = overlays().begin(); ov != overlays().end(); ++ov) {
		const surface img(image::get_image(*ov));
		if(img != NULL) {
			disp.draw_unit(x,y-height_adjust,img);
		}
	}
	refreshing_ = false;
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
