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
#include "unit_abilities.hpp"
#include "util.hpp"
#include "wassert.hpp"
#include "serialization/string_utils.hpp"
#include "halo.hpp"
#include "display.hpp"
#include "gamestatus.hpp"
#include "actions.hpp"
#include "game_events.hpp"
#include "sound.hpp"

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

	return lvla > lvlb || (lvla == lvlb && xpa < xpb);
}

void sort_units(std::vector< unit > &units)
{
	std::sort(units.begin(), units.end(), compare_unit_values);
}







// Copy constructor
unit::unit(const unit& o):
		cfg_(o.cfg_),
		movement_b_(o.movement_b_),
		defense_b_(o.defense_b_),
		resistance_b_(o.resistance_b_),

		advances_to_(o.advances_to_),
		id_(o.id_),
		name_(o.name_),
		description_(o.description_),
		custom_unit_description_(o.custom_unit_description_),
		underlying_description_(o.underlying_description_),
		language_name_(o.language_name_),
		race_(o.race_),
		undead_variation_(o.undead_variation_),
		variation_(o.variation_),

		flag_rgb_(o.flag_rgb_),
		hit_points_(o.hit_points_),
		max_hit_points_(o.max_hit_points_),
		max_hit_points_b_(o.max_hit_points_b_),
		experience_(o.experience_),
		max_experience_(o.max_experience_),
		max_experience_b_(o.max_experience_b_),
		level_(o.level_),


		alpha_(o.alpha_),
		side_(o.side_),

		movement_(o.movement_),
		max_movement_(o.max_movement_),
		max_movement_b_(o.max_movement_b_),
		attacks_left_(o.attacks_left_),
		max_attacks_(o.max_attacks_),
		movement_costs_(o.movement_costs_),
		recruits_(o.recruits_),

		states_(o.states_),
		variables_(o.variables_),

		overlays_(o.overlays_),

		role_(o.role_),
		attacks_(o.attacks_),
		attacks_b_(o.attacks_b_),

		traits_description_(o.traits_description_),

		modification_descriptions_(o.modification_descriptions_),
		defensive_animations_(o.defensive_animations_),

		teleport_animations_(o.teleport_animations_),

		extra_animations_(o.extra_animations_),

		death_animations_(o.death_animations_),

		movement_animations_(o.movement_animations_),

		standing_animations_(o.standing_animations_),

		leading_animations_(o.leading_animations_),

		healing_animations_(o.healing_animations_),
		goto_(o.goto_),
		interrupted_move_(o.interrupted_move_),
		anim_(o.anim_),
		offset_(o.offset_),
		emit_zoc_(o.emit_zoc_),
		frame_begin_time(o.frame_begin_time),
		unit_halo_(o.unit_halo_),
		unit_anim_halo_(o.unit_anim_halo_),
		unit_value_(o.unit_value_),
		getsHit_(o.getsHit_),
		refreshing_(o.refreshing_),
		hidden_(o.hidden_),
		draw_bars_(o.draw_bars_),
		unrenamable_(o.unrenamable_),
		flying_(o.flying_),
		hold_position_(o.hold_position_),
		end_turn_(o.end_turn_),
		resting_(o.resting_),
		facing_(o.facing_),
		state_(o.state_),
		gender_(o.gender_),
		alignment_(o.alignment_),

		modifications_(o.modifications_),
		gamedata_(o.gamedata_),
		units_(o.units_),
		map_(o.map_),
		gamestatus_(o.gamestatus_),
		teams_(o.teams_)
{
	anim_ = NULL;
	unit_halo_ = 0;
	unit_anim_halo_ = 0;
}

// Initilizes a unit from a config
unit::unit(const game_data* gamedata, unit_map* unitmap, const gamemap* map,
     const gamestatus* game_status, const std::vector<team>* teams,const config& cfg) :
	movement_(0), anim_(NULL),unit_halo_(0),unit_anim_halo_(0),draw_bars_(false), flying_(false),
	 hold_position_(false), resting_(false), facing_(gamemap::location::NORTH_EAST),
	 state_(STATE_STANDING), gamedata_(gamedata), units_(unitmap), map_(map),
	 gamestatus_(game_status),teams_(teams)
{
	read(cfg);
	getsHit_=0;
	end_turn_ = false;
	refreshing_  = false;
	hidden_ = false;
	offset_ = 0;
	if(race_->not_living()) {
		set_state("not_living","yes");
	}
}

unit::unit(const game_data& gamedata,const config& cfg) : movement_(0),
			anim_(NULL), unit_halo_(0), unit_anim_halo_(0), draw_bars_(false), flying_(false),
			hold_position_(false), resting_(false), facing_(gamemap::location::NORTH_EAST),
			state_(STATE_STANDING), gamedata_(&gamedata), 
			units_(NULL),map_(NULL), gamestatus_(NULL)
{
	read(cfg);
	getsHit_=0;
	end_turn_ = false;
	refreshing_  = false;
	hidden_ = false;
	offset_ = 0;
	if(race_->not_living()) {
		set_state("not_living","yes");
	}
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

// Initilizes a unit from a unit type
unit::unit(const game_data* gamedata, unit_map* unitmap, const gamemap* map,
           const gamestatus* game_status, const std::vector<team>* teams, const unit_type* t,
					 int side, bool use_traits, bool dummy_unit, unit_race::GENDER gender) :
	draw_bars_(false), resting_(false), facing_(gamemap::location::NORTH_EAST), state_(STATE_STANDING),
	gender_(dummy_unit ? gender : generate_gender(*t,use_traits)),
        gamedata_(gamedata), units_(unitmap), map_(map), gamestatus_(game_status), teams_(teams)
{
	goto_ = gamemap::location();
	side_ = side;
	movement_ = 0;
	attacks_left_ = 0;
	experience_ = 0;
	cfg_["upkeep"]="full";
	advance_to(&t->get_gender_unit_type(gender_));
	if(dummy_unit == false) validate_side(side_);
	if(use_traits) {
		//units that don't have traits generated are just generic
		//units, so they shouldn't get a description either.
		custom_unit_description_ = generate_description();
		generate_traits();
		underlying_description_ = description_;
	}else{
		underlying_description_ = id();
		if (race_->name() == "undead") {
			generate_traits();
		}
	}
	unrenamable_ = false;
	anim_ = NULL;
	getsHit_=0;
	end_turn_ = false;
	hold_position_ = false;
	offset_ = 0;
	unit_halo_ = 0;
	unit_anim_halo_ = 0;
}
unit::unit(const unit_type* t, int side, bool use_traits, bool dummy_unit, unit_race::GENDER gender) :
	draw_bars_(false), facing_(gamemap::location::NORTH_EAST), state_(STATE_STANDING),
	gender_(dummy_unit ? gender : generate_gender(*t,use_traits)), gamedata_(NULL), units_(NULL),
	map_(NULL), gamestatus_(NULL), teams_(NULL)
{
	goto_ = gamemap::location();
	side_ = side;
	movement_ = 0;
	attacks_left_ = 0;
	experience_ = 0;
	cfg_["upkeep"]="full";
	advance_to(&t->get_gender_unit_type(gender_));
	if(dummy_unit == false) validate_side(side_);
	if(use_traits) {
		//units that don't have traits generated are just generic
		//units, so they shouldn't get a description either.
		custom_unit_description_ = generate_description();
		generate_traits();
		underlying_description_ = description_;
	}else{
		underlying_description_ = id();
		if (race_->name() == "undead") {
			generate_traits();
		}
	}
	unrenamable_ = false;
	anim_ = NULL;
	getsHit_=0;
	end_turn_ = false;
	hold_position_ = false;
	offset_ = 0;
}

unit::~unit()
{
	if(unit_halo_) {
		halo::remove(unit_halo_);
	}
	if(unit_anim_halo_) {
		halo::remove(unit_anim_halo_);
	}
	if(anim_) {
		delete anim_;
	}
}



unit& unit::operator=(const unit& u)
{
	// use copy constructor to make sure we are coherant
	if (this != &u) {
		this->~unit();
		new (this) unit(u) ;
	}
	return *this ;
}



void unit::set_game_context(const game_data* gamedata, unit_map* unitmap, const gamemap* map, const gamestatus* game_status, const std::vector<team>* teams)
{
	gamedata_ = gamedata;
	units_ = unitmap;
	map_ = map;
	gamestatus_ = game_status;
	teams_ = teams;
}
void unit::write_checksum(std::string& str) const
{
	config unit_config;
	write(unit_config);
	unit_config["controller"] = "";
	// since the ai messes up the 'moves' attribute, ignore that for the checksum
	unit_config["moves"] = "";
	str = unit_config.hash();
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
	t = &t->get_gender_unit_type(gender_).get_variation(variation_);
	reset_modifications();
	// remove old animations
	cfg_.clear_children("defend");
	cfg_.clear_children("teleport_anim");
	cfg_.clear_children("extra_anim");
	cfg_.clear_children("death");
	cfg_.clear_children("movement_anim");
	cfg_.clear_children("leading_anim");
	cfg_.clear_children("healing_anim");
	cfg_.clear_children("standing_anim");
	cfg_.clear_children("attack");
	cfg_.clear_children("abilities");
	// clear cache of movement costs
	movement_costs_.clear();

	if(t->movement_type().get_parent()) {
		cfg_ = cfg_.merge_with(t->movement_type().get_parent()->get_cfg());
	}
	//if unit has specific profile, remember it and have it after advaces
	bool specific_profile = false;
	std::string profile;
	if (!id().empty() && gamedata_ != NULL)
	{
		specific_profile = (cfg_["profile"] != type().cfg_["profile"]);
		if (specific_profile)
		{
			profile = cfg_["profile"];
		}
	}
	cfg_ = cfg_.merge_with(t->cfg_);
	if (specific_profile)
	{
	cfg_["profile"] = profile; 
	}
	cfg_.clear_children("male");
	cfg_.clear_children("female");

	advances_to_ = t->advances_to();

	race_ = t->race_;
	language_name_ = t->language_name();
	cfg_["unit_description"] = t->unit_description();
	undead_variation_ = t->undead_variation();
	max_experience_ = t->experience_needed(false);
	level_ = t->level();
	alignment_ = t->alignment();
	alpha_ = t->alpha();
	hit_points_ = t->hitpoints();
	max_hit_points_ = t->hitpoints();
	max_movement_ = t->movement();
	emit_zoc_ = t->level();
	attacks_ = t->attacks();
	unit_value_ = t->cost();
	flying_ = t->movement_type().is_flying();

	max_attacks_ = lexical_cast_default<int>(t->cfg_["attacks"],1);
	defensive_animations_ = t->defensive_animations_;
	teleport_animations_ = t->teleport_animations_;
	extra_animations_ = t->extra_animations_;
	death_animations_ = t->death_animations_;
	movement_animations_ = t->movement_animations_;
	standing_animations_ = t->standing_animations_;
	leading_animations_ = t->leading_animations_;
	healing_animations_ = t->healing_animations_;
	flag_rgb_ = t->flag_rgb();

	backup_state();
	//apply modifications etc, refresh the unit
	apply_modifications();
	if(id()!=t->id() || cfg_["gender"] != cfg_["gender_id"]) {
		heal_all();
		id_ = t->id();
		cfg_["id"] = id_;
		cfg_["gender_id"] = cfg_["gender"];
	}
	game_events::add_events(cfg_.get_children("event"),id_);
	cfg_.clear_children("event");

	set_state("poisoned","");
	set_state("slowed","");
	set_state("stoned","");
	if(race_->not_living()) {
		set_state("not_living","yes");
	}
	end_turn_ = false;
	refreshing_  = false;
	hidden_ = false;
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
const unit_type& unit::type() const
{
	wassert(gamedata_ != NULL);
	std::map<std::string,unit_type>::const_iterator i = gamedata_->unit_types.find(id());
	wassert(i != gamedata_->unit_types.end());
	return i->second;
}
// the actual name of the unit
const std::string& unit::name() const
{
	if(description_.empty() == false)
		return description_;
	else
		return language_name();
}
void unit::rename(const std::string& name)
{
	if (!unrenamable_) {
		custom_unit_description_ = name;
	}
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
	if(cfg_["profile"] != "" && cfg_["profile"] != "unit_image") {
		return cfg_["profile"];
	}
	return absolute_image();
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
	return maximum<int>(1,(max_experience_*unit_type::experience_accelerator::get_acceleration() + 50) / 100);
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
	return game_config::tc_info(flag_rgb_);
}
std::vector<Uint32> unit::team_rgb_range() const
{
  std::vector<Uint32> temp;
  temp.push_back(team::get_side_rgb(side()));
  temp.push_back(team::get_side_rgb_max(side()));
  temp.push_back(team::get_side_rgb_min(side()));
  return(temp);
}
const std::string& unit::team_color() const
{
	return flag_rgb_;	
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
	return utils::string_bool(cfg_["canrecruit"]);
}
bool unit::incapacitated() const
{
	return utils::string_bool(get_state("stoned"),false);
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
	return end_turn_;
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
	if(movement_ == total_movement()) {
		movement_ = 0;
		set_state("not_moved","yes");
	} else if(movement_ >= 0) {
		movement_ = 0;
	}
}
void unit::new_turn()
{
	end_turn_ = false;
	movement_ = total_movement();
	attacks_left_ = max_attacks_;
	if(has_ability_type("hides")) {
		set_state("hides","yes");
	}
	if(incapacitated()) {
		set_attacks(0);
	}
	if (hold_position_) {
		end_turn_ = true;
	}
}
void unit::end_turn()
{
	set_state("slowed","");
	if((movement_ != total_movement()) && !utils::string_bool(get_state("not_moved"))) {
		resting_ = false;
	}
	set_state("not_moved","");
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
void unit::heal(int amount)
{
	int max_hp = max_hitpoints();
	if (hit_points_ < max_hp) {
		hit_points_ += amount;
		if (hit_points_ > max_hp) {
			hit_points_ = max_hp;
		}
	}
	if(hit_points_<1) {
		hit_points_ = 1;
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
	if(incapacitated()) {
		return false;
	}
	return emit_zoc_;
}

bool unit::has_ability_by_id(const std::string& ability) const
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

bool unit::matches_filter(const config& cfg,const gamemap::location& loc,bool use_flat_tod) const
{
	const std::string& description = cfg["description"];
	const std::string& speaker = cfg["speaker"];
	const std::string& type = cfg["type"];
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

	//the type could be a comma separated list of types
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

	if(ability.empty() == false && has_ability_by_id(ability) == false) {
		if(std::find(ability.begin(),ability.end(),',') != ability.end()) {
			const std::vector<std::string>& vals = utils::split(ability);
			bool has_ability = false;
			for(std::vector<std::string>::const_iterator this_ability = vals.begin(); this_ability != vals.end(); ++this_ability) {
				if(has_ability_by_id(*this_ability)) {
					has_ability = true;
					break;
				}
			}
			if(!has_ability) {
				return false;
			}
		} else {
			return false;
		}
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
	if(cfg["id"]=="" && cfg["type"]=="") {
		throw game::load_game_failed("Attempt to de-serialize an empty unit");
	}
	cfg_ = cfg;
	side_ = lexical_cast_default<int>(cfg["side"]);
	if(side_ <= 0) {
		side_ = 1;
	}

	validate_side(side_);
	bool id_set = cfg["id"] != "";

	/* prevent un-initialized variables */
	max_hit_points_=1;
	hit_points_=1;
	max_movement_=0;
	max_experience_=0;
	/* */

	const std::string& gender = cfg["gender"];
	if(gender == "female") {
		gender_ = unit_race::FEMALE;
	} else {
		gender_ = unit_race::MALE;
	}

	variation_ = cfg["variation"];

	wassert(gamedata_ != NULL);
	description_ = cfg["description"];
	custom_unit_description_ = cfg["user_description"];
	std::string custom_unit_desc = cfg["unit_description"];

	underlying_description_ = cfg["description"];
	if(description_.empty()) {
		description_ = underlying_description_;
	}

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

	advances_to_ = utils::split(cfg["advances_to"]);
	if(advances_to_.size() == 1 && advances_to_.front() == "") {
		advances_to_.clear();
	}

	name_ = cfg["name"];
	language_name_ = cfg["language_name"];
	undead_variation_ = cfg["undead_variation"];

	flag_rgb_ = cfg["flag_rgb"];
	alpha_ = lexical_cast_default<fixed_t>(cfg["alpha"]);

	level_ = lexical_cast_default<int>(cfg["level"]);
	unit_value_ = lexical_cast_default<int>(cfg["value"]);

	facing_ = gamemap::location::parse_direction(cfg["facing"]);
	if(facing_ == gamemap::location::NDIRECTIONS) facing_ = gamemap::location::NORTH_EAST;

	recruits_ = utils::split(cfg["recruits"]);
	if(recruits_.size() == 1 && recruits_.front() == "") {
		recruits_.clear();
	}
	attacks_left_ = lexical_cast_default<int>(cfg["attacks_left"]);
	const config* mods = cfg.child("modifications");
	if(mods) {
		modifications_ = *mods;
		cfg_.remove_child("modifications",0);
	}

	bool type_set = false;
	id_ = "";
	wassert(gamedata_ != NULL);
	if(cfg["type"] != "" && cfg["type"] != cfg["id"] || cfg["gender"] != cfg["gender_id"]) {
		std::map<std::string,unit_type>::const_iterator i = gamedata_->unit_types.find(cfg["type"]);
		if(i != gamedata_->unit_types.end()) {
			advance_to(&i->second.get_gender_unit_type(gender_));
			type_set = true;
		} else {
			LOG_STREAM(err, engine) << "unit of type " << cfg["type"] << " not found!\n";
			throw game::game_error("Unknown unit type '" + cfg["type"] + "'");
		}
		attacks_left_ = max_attacks_;
		if(cfg["moves"]=="") {
			movement_ = max_movement_;
		}
		if(movement_ < 0) {
			attacks_left_ = 0;
			movement_ = 0;
		}
	}
	if(cfg_["id"]=="") {
		id_ = cfg_["type"];
	} else {
		id_ = cfg_["id"];
	}
	if(!type_set || cfg["race"] != "") {
		const race_map::const_iterator race_it = gamedata_->races.find(cfg["race"]);
		if(race_it != gamedata_->races.end()) {
			race_ = &race_it->second;
		} else {
			static const unit_race dummy_race;
			race_ = &dummy_race;
		}
	}
	variation_ = cfg["variation"];
	if(!type_set || cfg["max_attacks"] != "") {
		max_attacks_ = minimum<int>(1,lexical_cast_default<int>(cfg["max_attacks"]));
	}
	if(!type_set || cfg["zoc"] != "") {
		emit_zoc_ = lexical_cast_default<int>(cfg["zoc"]);
	}
	if(cfg["max_hitpoints"] != "") {
		max_hit_points_ = lexical_cast_default<int>(cfg["max_hitpoints"]);
	}
	if(cfg["max_moves"] != "") {
		max_movement_ = lexical_cast_default<int>(cfg["max_moves"]);
	}
	if(cfg["max_experience"] != "") {
		max_experience_ = lexical_cast_default<int>(cfg["max_experience"]);
	}
	if(cfg["flying"] != "") {
		flying_ = utils::string_bool(cfg["flying"]);
	}
	if(custom_unit_desc != "") {
		cfg_["unit_description"] = custom_unit_desc;
	}
	
	if(cfg["profile"] != "" && !id_set) {
		cfg_["profile"] = cfg["profile"];
	}
	
	std::map<std::string,unit_type>::const_iterator uti = gamedata_->unit_types.find(cfg["type"]);
	const unit_type* ut = NULL;
	if(uti != gamedata_->unit_types.end()) {
		ut = &uti->second.get_gender_unit_type(gender_).get_variation(variation_);
	}
	if(!type_set) {
		if(ut) {
			if(cfg_["unit_description"] == "") {
				cfg_["unit_description"] = ut->unit_description();
			}
			config t_atks;
			config u_atks;
			config::const_child_itors range;
			for(range = ut->cfg_.child_range("attack");
			    range.first != range.second; ++range.first) {
				t_atks.add_child("attack",**range.first);
			}
			for(range = cfg.child_range("attack");
			    range.first != range.second; ++range.first) {
				u_atks.add_child("attack",**range.first);
			}
			u_atks = t_atks.merge_with(u_atks);
			for(range = u_atks.child_range("attack");
			    range.first != range.second; ++range.first) {
				attacks_.push_back(attack_type(**range.first,id(),image_fighting((**range.first)["range"] == "ranged" ? attack_type::LONG_RANGE : attack_type::SHORT_RANGE)));
			}
			std::vector<attack_type>::iterator at;
			for(at = attacks_.begin(); at != attacks_.end(); ++at) {
				at->get_cfg().clear_children("animation");
			}
			for(at = attacks_b_.begin(); at != attacks_b_.end(); ++at) {
				at->get_cfg().clear_children("animation");
			}
		} else {
			for(config::const_child_itors range = cfg.child_range("attack");
			    range.first != range.second; ++range.first) {
				attacks_.push_back(attack_type(**range.first,id(),image_fighting((**range.first)["range"] == "ranged" ? attack_type::LONG_RANGE : attack_type::SHORT_RANGE)));
			}
		}
	} else {
		std::vector<attack_type>::iterator at;
		for(at = attacks_.begin(); at != attacks_.end(); ++at) {
			at->get_cfg().clear_children("animation");
		}
		for(at = attacks_b_.begin(); at != attacks_b_.end(); ++at) {
			at->get_cfg().clear_children("animation");
		}
	}
	cfg_.clear_children("attack");
	const config* status_flags = cfg.child("status");
	if(status_flags) {
		for(string_map::const_iterator st = status_flags->values.begin(); st != status_flags->values.end(); ++st) {
			// backwards compatibility
			if(st->first == "stone") {
				states_["stoned"] = st->second;
			} else {
				states_[st->first] = st->second;
			}
		}
		cfg_.remove_child("status",0);
	}
	if(cfg["ai_special"] == "guardian") {
		set_state("guardian","yes");
	}
	if(!type_set) {
		backup_state();
		apply_modifications();
	}
	if(utils::string_bool(cfg["random_traits"]) ||
	    race_->name() == "undead") {
		generate_traits();
		cfg_["random_traits"] = "";
	}
	if(cfg["hitpoints"] != "") {
		hit_points_ = lexical_cast_default<int>(cfg["hitpoints"]);
	} else {
		hit_points_ = max_hit_points_;
	}
	goto_.x = lexical_cast_default<int>(cfg["goto_x"]) - 1;
	goto_.y = lexical_cast_default<int>(cfg["goto_y"]) - 1;
	if(cfg["moves"] != "") {
		movement_ = lexical_cast_default<int>(cfg["moves"]);
		if(movement_ < 0) {
			attacks_left_ = 0;
			movement_ = 0;
		}
	} else {
		movement_ = max_movement_;
	}
	experience_ = lexical_cast_default<int>(cfg["experience"]);
	resting_ = utils::string_bool(cfg["resting"]);
	unrenamable_ = utils::string_bool(cfg["unrenamable"]);
	if(cfg["alignment"]=="lawful") {
		alignment_ = unit_type::LAWFUL;
	} else if(cfg["alignment"]=="neutral") {
		alignment_ = unit_type::NEUTRAL;
	} else if(cfg["alignment"]=="chaotic") {
		alignment_ = unit_type::CHAOTIC;
	} else if(cfg["type"]=="") {
		alignment_ = unit_type::NEUTRAL;
	}
	if(utils::string_bool(cfg["generate_description"])) {
		custom_unit_description_ = generate_description();
		cfg_["generate_description"] = "";
	}
	
	if(!type_set) {
		if(ut) {
			defensive_animations_ = ut->defensive_animations_;
			teleport_animations_ = ut->teleport_animations_;
			extra_animations_ = ut->extra_animations_;
			death_animations_ = ut->death_animations_;
			movement_animations_ = ut->movement_animations_;
			standing_animations_ = ut->standing_animations_;
			leading_animations_ = ut->leading_animations_;
			healing_animations_ = ut->healing_animations_;
			// remove animations from private cfg, since they're not needed there now
			cfg_.clear_children("defend");
			cfg_.clear_children("teleport_anim");
			cfg_.clear_children("extra_anim");
			cfg_.clear_children("death");
			cfg_.clear_children("movement_anim");
			cfg_.clear_children("healing_anim");
			cfg_.clear_children("leading_anim");
			cfg_.clear_children("standing_anim");
		} else {
			const config::child_list& defends = cfg_.get_children("defend");
			const config::child_list& teleports = cfg_.get_children("teleport_anim");
			const config::child_list& extra_anims = cfg_.get_children("extra_anim");
			const config::child_list& deaths = cfg_.get_children("death");
			const config::child_list& movement_anims = cfg_.get_children("movement_anim");
			const config::child_list& standing_anims = cfg_.get_children("standing_anim");
			const config::child_list& leading_anims = cfg_.get_children("leading_anim");
			const config::child_list& healing_anims = cfg_.get_children("healing_anim");
			for(config::child_list::const_iterator d = defends.begin(); d != defends.end(); ++d) {
				defensive_animations_.push_back(defensive_animation(**d));
			}
			config::child_list::const_iterator t;
			for(t = teleports.begin(); t != teleports.end(); ++t) {
				teleport_animations_.push_back(unit_animation(**t));
			}
			for(t = extra_anims.begin(); t != extra_anims.end(); ++t) {
				extra_animations_.insert(std::pair<std::string,unit_animation>((**t)["flag"],unit_animation(**t)));
			}
			for(config::child_list::const_iterator death = deaths.begin(); death != deaths.end(); ++death) {
				death_animations_.push_back(death_animation(**death));
			}
			for(config::child_list::const_iterator movement_anim = movement_anims.begin(); movement_anim != movement_anims.end(); ++movement_anim) {
				movement_animations_.push_back(movement_animation(**movement_anim));
			}
			for(config::child_list::const_iterator standing_anim = standing_anims.begin(); standing_anim != standing_anims.end(); ++standing_anim) {
				standing_animations_.push_back(standing_animation(**standing_anim));
			}
			for(config::child_list::const_iterator leading_anim = leading_anims.begin(); leading_anim != leading_anims.end(); ++leading_anim) {
				leading_animations_.push_back(leading_animation(**leading_anim));
			}
			for(config::child_list::const_iterator healing_anim = healing_anims.begin(); healing_anim != healing_anims.end(); ++healing_anim) {
				healing_animations_.push_back(healing_animation(**healing_anim));
			}
			if(defensive_animations_.empty()) {
				defensive_animations_.push_back(defensive_animation(absolute_image()));
				// always have a defensive animation
			}
			if(teleport_animations_.empty()) {
				teleport_animations_.push_back(unit_animation(absolute_image(),-20,20));
				// always have a teleport animation
			}
			if(death_animations_.empty()) {
				death_animations_.push_back(death_animation(absolute_image()));
				// always have a death animation
			}
			if(movement_animations_.empty()) {
				movement_animations_.push_back(movement_animation(absolute_image()));
				// always have a movement animation
			}
			if(standing_animations_.empty()) {
				standing_animations_.push_back(standing_animation(absolute_image()));
				// always have a standing animation
			}
			if(leading_animations_.empty()) {
				leading_animations_.push_back(leading_animation(absolute_image()));
				// always have a leading animation
			}
			if(healing_animations_.empty()) {
				healing_animations_.push_back(healing_animation(image_healing(),image_halo_healing()));
				// always have a healing animation
			}
		}
	} else {
		// remove animations from private cfg, since they're not needed there now
		cfg_.clear_children("defend");
		cfg_.clear_children("teleport_anim");
		cfg_.clear_children("extra_anim");
		cfg_.clear_children("death");
		cfg_.clear_children("movement_anim");
		cfg_.clear_children("healing_anim");
		cfg_.clear_children("leading_anim");
		cfg_.clear_children("standing_anim");
	}
	game_events::add_events(cfg_.get_children("event"),id_);
	cfg_.clear_children("event");
}
void unit::write(config& cfg) const
{
	// if a location has been saved in the config, keep it
	std::string x = cfg["x"];
	std::string y = cfg["y"];
	cfg.append(cfg_);
	cfg.clear_children("movement_costs");
	cfg.clear_children("defense");
	cfg.clear_children("resistance");
	cfg.add_child("movement_costs",movement_b_);
	cfg.add_child("defense",defense_b_);
	cfg.add_child("resistance",resistance_b_);
	cfg["x"] = x;
	cfg["y"] = y;
	cfg["id"] = id();
	cfg["type"] = id();
	std::map<std::string,unit_type>::const_iterator uti = gamedata_->unit_types.find(id());
	const unit_type* ut = NULL;
	if(uti != gamedata_->unit_types.end()) {
		ut = &uti->second.get_gender_unit_type(gender_).get_variation(variation_);
	}
	if(ut && cfg["unit_description"] == ut->unit_description()) {
		cfg["unit_description"] = "";
	}

	std::stringstream hp;
	hp << hit_points_;
	cfg["hitpoints"] = hp.str();
	std::stringstream hpm;
	hpm << max_hit_points_b_;
	cfg["max_hitpoints"] = hpm.str();

	std::stringstream xp;
	xp << experience_;
	cfg["experience"] = xp.str();
	std::stringstream xpm;
	xpm << max_experience_b_;
	cfg["max_experience"] = xpm.str();

	std::stringstream sd;
	sd << side_;
	cfg["side"] = sd.str();

	cfg["gender"] = gender_ == unit_race::MALE ? "male" : "female";
	cfg["gender_id"] = gender_ == unit_race::MALE ? "male" : "female";

	cfg["variation"] = variation_;

	cfg["role"] = role_;
	cfg["flying"] = flying_ ? "yes" : "no";

	config status_flags;
	for(std::map<std::string,std::string>::const_iterator st = states_.begin(); st != states_.end(); ++st) {
		status_flags[st->first] = st->second;
	}

	cfg.clear_children("variables");
	cfg.add_child("variables",variables_);
	cfg.clear_children("status");
	cfg.add_child("status",status_flags);

	cfg["overlays"] = utils::join(overlays_);

	cfg["user_description"] = custom_unit_description_;
	cfg["description"] = underlying_description_;

	if(can_recruit())
		cfg["canrecruit"] = "1";

	cfg["facing"] = gamemap::location::write_direction(facing_);

	cfg["goto_x"] = lexical_cast_default<std::string>(goto_.x+1);
	cfg["goto_y"] = lexical_cast_default<std::string>(goto_.y+1);

	cfg["moves"] = lexical_cast_default<std::string>(movement_);
	cfg["max_moves"] = lexical_cast_default<std::string>(max_movement_b_);

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
	cfg["flag_rgb"] = flag_rgb_;
	cfg["unrenamable"] = unrenamable_ ? "yes" : "no";
	cfg["alpha"] = lexical_cast_default<std::string>(alpha_);

	cfg["recuits"] = utils::join(recruits_);
	cfg["attacks_left"] = lexical_cast_default<std::string>(attacks_left_);
	cfg["max_attacks"] = lexical_cast_default<std::string>(max_attacks_);
	cfg["zoc"] = lexical_cast_default<std::string>(emit_zoc_);
	cfg.clear_children("attack");
	for(std::vector<attack_type>::const_iterator i = attacks_b_.begin(); i != attacks_b_.end(); ++i) {
		cfg.add_child("attack",i->get_cfg());
	}
	cfg["value"] = lexical_cast_default<std::string>(unit_value_);
	cfg["cost"] = lexical_cast_default<std::string>(unit_value_);
	cfg.clear_children("modifications");
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

const surface unit::still_image() const
{
	image::locator  loc;
	if(flag_rgb().size()){
		loc = image::locator(absolute_image(),team_rgb_range(),flag_rgb());
	}else{
		loc = image::locator(absolute_image());
	}
	surface unit_image(image::get_image(loc,image::UNSCALED));
	return unit_image;
}
void unit::set_standing(const display &disp,const gamemap::location& loc, bool with_bars)
{
	state_ = STATE_STANDING;
	draw_bars_ = with_bars;
	offset_=0;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	anim_ = new standing_animation(stand_animation(disp.get_map().underlying_union_terrain(loc),facing_));
	anim_->start_animation(anim_->get_first_frame_time(),unit_animation::INFINITE_CYCLES,disp.turbo()?5:1);
	frame_begin_time = anim_->get_first_frame_time() -1;
	anim_->update_current_frame();
}
void unit::set_defending(const display &disp,const gamemap::location& loc, int damage,const attack_type* attack)
{
	state_ =  STATE_DEFENDING;
	draw_bars_ = false;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	fighting_animation::hit_type hit_type;
	if(damage >= hitpoints()) {
		hit_type = fighting_animation::KILL;
	} else if(damage > 0) {
		hit_type = fighting_animation::HIT;
	}else {
		hit_type = fighting_animation::MISS;
	}
	anim_ =  new defensive_animation(defend_animation(disp.get_map().underlying_union_terrain(loc),hit_type,attack));

	// add a blink on damage effect
	int anim_time = anim_->get_last_frame_time();
	const std::string my_image = anim_->get_last_frame().image;
	if(damage) {
		anim_->add_frame(anim_time,unit_frame(my_image,"",anim_time,anim_time+30,display::rgb(255,0,0),0.5));
		anim_time += 50;
		anim_->add_frame(anim_time,unit_frame(my_image,"",anim_time,anim_time+30,display::rgb(255,0,0),0.0));
		anim_time += 50;
	}
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	frame_begin_time = anim_->get_first_frame_time() -1;
	anim_->update_current_frame();
}

void unit::set_extra_anim(const display &disp,const gamemap::location& loc, std::string flag)
{
	state_ =  STATE_EXTRA;
	draw_bars_ = false;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	if(!extra_animation(disp.get_map().underlying_union_terrain(loc),flag)) {
		set_standing(disp,loc);
		return;
	}
	anim_ =  new unit_animation(*(extra_animation(disp.get_map().underlying_union_terrain(loc),flag)));
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	frame_begin_time = anim_->get_first_frame_time() -1;
	anim_->update_current_frame();
}

const unit_animation & unit::set_attacking(const display &disp,const gamemap::location& /*loc*/,bool hit,const attack_type& type)
{
	state_ =  STATE_ATTACKING;
	draw_bars_ = false;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	const attack_type::attack_animation &attack_anim = type.animation(hit,facing_) ;
	anim_ =  new unit_animation(attack_anim.animation);
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	frame_begin_time = anim_->get_first_frame_time() -1;
	anim_->update_current_frame();
	return attack_anim.missile_animation;
}
void unit::set_leading(const display &disp,const gamemap::location& loc)
{
	state_ = STATE_LEADING;
	draw_bars_ = true;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	anim_ = new leading_animation(lead_animation(disp.get_map().underlying_union_terrain(loc),facing_));
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	frame_begin_time = anim_->get_first_frame_time() -1;
	anim_->update_current_frame();
}
void unit::set_leveling_in(const display &disp,const gamemap::location& /*loc*/)
{
	state_ = STATE_LEVELIN;
	draw_bars_ = false;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	std::string my_image;
	my_image = absolute_image();
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
	frame_begin_time = anim_->get_first_frame_time() -1;
	anim_->update_current_frame();
}
void unit::set_leveling_out(const display &disp,const gamemap::location& /*loc*/)
{
	state_ = STATE_LEVELOUT;
	draw_bars_ = false;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	std::string my_image;
	my_image = absolute_image();
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
	frame_begin_time = anim_->get_first_frame_time() -1;
	anim_->update_current_frame();
}
void unit::set_recruited(const display &disp,const gamemap::location& /*loc*/)
{
	state_ = STATE_RECRUITED;
	draw_bars_ = false;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	anim_ = new unit_animation();
	// add a fade in effect
	double blend_ratio =0;
	int anim_time =0;
	while(blend_ratio < 1) {
		anim_->add_frame(anim_time,unit_frame(absolute_image(),"",anim_time,anim_time+10,0,0,ftofxp(blend_ratio)));
		blend_ratio +=0.015;
		anim_time += 10;
	}
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	frame_begin_time = anim_->get_first_frame_time() -1;
	anim_->update_current_frame();
}
void unit::set_healed(const display &disp,const gamemap::location& /*loc*/, int healing)
{
	state_ = STATE_HEALED;
	draw_bars_ = true;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	anim_ = new unit_animation(absolute_image());
	// add a blink on heal effect
	int anim_time = anim_->get_last_frame_time();
	int heal_left = healing;
	const std::string my_image = anim_->get_last_frame().image;
	while(anim_time < 1000 && heal_left > 0 ) {
		anim_->add_frame(anim_time,unit_frame(my_image,"",anim_time,anim_time+30,display::rgb(255,255,255),0.5));
		anim_time += 30;
		heal_left --;
		anim_->add_frame(anim_time,unit_frame(my_image,"",anim_time,anim_time+30,display::rgb(255,255,255),0.0));
		anim_time += 30;
		heal_left --;
	}
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	frame_begin_time = anim_->get_first_frame_time() -1;
	anim_->update_current_frame();
}
void unit::set_poisoned(const display &disp,const gamemap::location& /*loc*/, int damage)
{
	state_ = STATE_POISONED;
	draw_bars_ = true;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	anim_ = new unit_animation(absolute_image());
	// add a blink on damage effect
	int anim_time = anim_->get_last_frame_time();
	int damage_left = damage;
	const std::string my_image = anim_->get_last_frame().image;
	while(anim_time < 1000 && damage_left > 0 ) {
		anim_->add_frame(anim_time,unit_frame(my_image,"",anim_time,anim_time+30,display::rgb(0,255,0),0.5));
		anim_time += 30;
		damage_left --;
		anim_->add_frame(anim_time,unit_frame(my_image,"",anim_time,anim_time+30,display::rgb(0,255,0),0.0));
		anim_time += 30;
		damage_left --;
	}
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	frame_begin_time = anim_->get_first_frame_time() -1;
	anim_->update_current_frame();
}

void unit::set_teleporting(const display &disp,const gamemap::location& /*loc*/)
{
	state_ = STATE_TELEPORT;
	draw_bars_ = false;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	anim_ =  new unit_animation(teleport_animation());
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	frame_begin_time = anim_->get_first_frame_time() -1;
	anim_->update_current_frame();
}

void unit::set_dying(const display &disp,const gamemap::location& loc,const attack_type* attack)
{
	state_ = STATE_DYING;
	draw_bars_ = false;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	anim_ =  new death_animation(die_animation(disp.get_map().underlying_union_terrain(loc),fighting_animation::KILL,attack));
	double blend_ratio = 1.0;
	std::string tmp_image = anim_->get_last_frame().image;
	int anim_time =anim_->get_last_frame_time();
	while(blend_ratio > 0.0) {
		anim_->add_frame(anim_time,unit_frame(tmp_image,"",anim_time,anim_time+10,0,0,ftofxp(blend_ratio)));
		blend_ratio -=0.015;
		anim_time += 10;
	}
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	frame_begin_time = anim_->get_first_frame_time() -1;
	anim_->update_current_frame();
}
void unit::set_healing(const display &disp,const gamemap::location& loc)
{
	state_ = STATE_HEALING;
	draw_bars_ = true;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	anim_ = new healing_animation(heal_animation(disp.get_map().underlying_union_terrain(loc),facing_));
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	frame_begin_time = anim_->get_first_frame_time() -1;
	anim_->update_current_frame();
}

void unit::set_walking(const display &disp,const gamemap::location& loc)
{
	movement_animation* const anim = dynamic_cast<movement_animation*>(anim_);
	if(state_ == STATE_WALKING && anim != NULL && anim->matches(disp.get_map().underlying_union_terrain(loc),facing_) >=0) {
		return; // finish current animation, don't start a new one
	}
	state_ = STATE_WALKING;
	draw_bars_ = false;
	if(anim_) {
		delete anim_;
		anim_ = NULL;
	}
	anim_ = new movement_animation(move_animation(disp.get_map().underlying_union_terrain(loc),facing_));
	anim_->start_animation(anim_->get_first_frame_time(),1,disp.turbo()?5:1);
	frame_begin_time = anim_->get_first_frame_time() -1;
}




void unit::restart_animation(const display& disp,int start_time) {
	if(!anim_) return;
	anim_->start_animation(start_time,1,disp.turbo()?5:1);
	frame_begin_time = start_time -1;
}

void unit::redraw_unit(display& disp,gamemap::location hex)
{
	const gamemap & map = disp.get_map();
	if(hidden_ || 
	disp.fogged(hex.x,hex.y) ||
	(invisible(hex,disp.get_units(),disp.get_teams()) &&
			disp.get_teams()[disp.viewing_team()].is_enemy(side())) ){
		if(unit_halo_) halo::remove(unit_halo_);
		unit_halo_ = 0;
		if(unit_anim_halo_) halo::remove(unit_anim_halo_);
		unit_anim_halo_ = 0;
		return;
	}
	if(refreshing_) return;
	refreshing_ = true;
	const gamemap::location dst= hex.get_direction(facing());
	const double xsrc = disp.get_location_x(hex);
	const double ysrc = disp.get_location_y(hex);
	const double xdst = disp.get_location_x(dst);
	const double ydst = disp.get_location_y(dst);

	const int x = int(offset_*xdst + (1.0-offset_)*xsrc);
	const int y = int(offset_*ydst + (1.0-offset_)*ysrc);

	if(!anim_) set_standing(disp,hex);
	const gamemap::TERRAIN terrain = map.get_terrain(hex);
	const double submerge = is_flying() ? 0.0 : map.get_terrain_info(terrain).unit_submerge() * disp.zoom();
	const int height_adjust = is_flying() ? 0 : int(map.get_terrain_info(terrain).unit_height_adjust() * disp.zoom());

	std::string image_name;
	unit_frame current_frame;
	if(anim_->animation_finished()) current_frame = anim_->get_last_frame();
	else if(anim_->get_first_frame_time() > anim_->get_animation_time()) current_frame = anim_->get_first_frame();
	else current_frame = anim_->get_current_frame();

	image_name = current_frame.image;
	if(frame_begin_time != current_frame.begin_time) {
		frame_begin_time = current_frame.begin_time;
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
			unit_anim_halo_ = halo::add(x+d-static_cast<int>(current_frame.halo_x*disp.zoom()),
					y+d+static_cast<int>(current_frame.halo_y*disp.zoom()),
					current_frame.halo[sub_halo].first,
					halo::HREVERSE);
		} else {
			const int d = disp.hex_size() / 2;
			unit_anim_halo_ = halo::add(x+d+static_cast<int>(current_frame.halo_x*disp.zoom()),
					y+d+static_cast<int>(current_frame.halo_y*disp.zoom()),
					current_frame.halo[sub_halo].first);
		}
	}
	if(image_name.empty()) {
		image_name = absolute_image();
	}
	image::locator  loc;
	if(flag_rgb().size()){
		loc = image::locator(image_name,team_rgb_range(),flag_rgb());
	}else{
		loc = image::locator(image_name);
	}
	surface image(image::get_image(loc,utils::string_bool(get_state("stoned"))?image::GREYED : image::UNSCALED));
	if(image ==NULL) {
		image = still_image();
	}
	if(facing_ == gamemap::location::NORTH_WEST || facing_ == gamemap::location::SOUTH_WEST) {
		image.assign(image::reverse_image(image));
	}

	Uint32 blend_with = current_frame.blend_with;
	double blend_ratio = current_frame.blend_ratio;
	if(blend_ratio == 0) { blend_with = disp.rgb(0,0,0); }
	fixed_t highlight_ratio = minimum<fixed_t>(alpha(),current_frame.highlight_ratio);
	if(invisible(hex,disp.get_units(),disp.get_teams()) &&
			highlight_ratio > ftofxp(0.5)) {
		highlight_ratio = ftofxp(0.5);
	}
	if(hex == disp.selected_hex() && highlight_ratio == ftofxp(1.0)) {
		highlight_ratio = ftofxp(1.5);
	}

	if (utils::string_bool(get_state("poisoned")) && blend_ratio == 0){
		blend_with = disp.rgb(0,255,0);
		blend_ratio = 0.25;
	}




	surface ellipse_front(NULL);
	surface ellipse_back(NULL);
	if(preferences::show_side_colours() && draw_bars_) {
		const char* const selected = disp.selected_hex() == hex ? "selected-" : "";
		std::vector<Uint32> temp_rgb;
		//ellipse not pure red=255!
		for(int i=255;i>100;i--){
			temp_rgb.push_back((Uint32)(i<<16));
		}
		//selected ellipse not pure red at all!
		char buf[100];
		std::string ellipse=image_ellipse();
		if(ellipse.empty()){
			ellipse="misc/ellipse";
		}
		snprintf(buf,sizeof(buf),"%s-%stop.png",ellipse.c_str(),selected);
		ellipse_back.assign(image::get_image(image::locator(buf,team_rgb_range(),temp_rgb)));
		snprintf(buf,sizeof(buf),"%s-%sbottom.png",ellipse.c_str(),selected);
		ellipse_front.assign(image::get_image(image::locator(buf,team_rgb_range(),temp_rgb)));
	}

	
	
	int tmp_x = x;
	int tmp_y = y;
	
	if (!image.null())
	{
		tmp_x += (disp.hex_size() - image.get()->w)/2;
		tmp_y += (disp.hex_size() - image.get()->h)/2;
	}

	disp.draw_unit(tmp_x, tmp_y -height_adjust, image, false, highlight_ratio,
			blend_with, blend_ratio, submerge,ellipse_back,ellipse_front);
	if(!unit_halo_ && !image_halo().empty()) {
		unit_halo_ = halo::add(0,0,image_halo());
	}
	if(unit_halo_) {
		const int d = disp.hex_size() / 2;
		halo::set_location(unit_halo_, x+ d, y -height_adjust+ d);
	}

	if(draw_bars_) {
		const std::string* movement_file = NULL;
		const std::string* energy_file = &game_config::energy_image;
		const fixed_t bar_alpha = highlight_ratio < ftofxp(1.0) && blend_with == 0 ? highlight_ratio : (hex == disp.mouseover_hex() ? ftofxp(1.0): ftofxp(0.7));

		if(size_t(side()) != disp.viewing_team()+1) {
			if(disp.team_valid() &&
			   disp.get_teams()[disp.viewing_team()].is_enemy(side())) {
				movement_file = &game_config::enemy_ball_image;
			} else {
				movement_file = &game_config::ally_ball_image;
			}
		} else {
			movement_file = &game_config::moved_ball_image;
			if(disp.playing_team() == disp.viewing_team() && !user_end_turn()) {
				if (movement_left() == total_movement()) {
					movement_file = &game_config::unmoved_ball_image;
				} else if(unit_can_move(hex,disp.get_units(),map,disp.get_teams())) {
					movement_file = &game_config::partmoved_ball_image;
				}
			}
		}
		disp.draw_bar(*movement_file,x,y-height_adjust,0,0,hp_color(),bar_alpha);

		double unit_energy = 0.0;
		if(max_hitpoints() > 0) {
			unit_energy = double(hitpoints())/double(max_hitpoints());
		}
		disp.draw_bar(*energy_file,x-static_cast<int>(5*disp.zoom()),y-height_adjust,(max_hitpoints()*2)/3,unit_energy,hp_color(),bar_alpha);

		if(experience() > 0 && can_advance()) {
			const double filled = double(experience())/double(max_experience());
			const int level = maximum<int>(level_,1);

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

gamemap::location::DIRECTION unit::facing() const
{
	return facing_;
}
void unit::set_facing(gamemap::location::DIRECTION dir)
{
	wassert(dir != gamemap::location::NDIRECTIONS);
	facing_ = dir;
}

std::set<gamemap::location> unit::overlaps(const display &disp, const gamemap::location &loc) const
{
	std::set<gamemap::location> over;

	const gamemap & map = disp.get_map();
	int height_adjust = map.get_terrain_info(map.get_terrain(loc)).unit_height_adjust();
	if (height_adjust > 0) {
		over.insert(loc.get_direction(gamemap::location::NORTH));
		over.insert(loc.get_direction(gamemap::location::NORTH_WEST));
		over.insert(loc.get_direction(gamemap::location::NORTH_EAST));
	} else if (height_adjust < 0) {
		over.insert(loc.get_direction(gamemap::location::SOUTH));
		over.insert(loc.get_direction(gamemap::location::SOUTH_WEST));
		over.insert(loc.get_direction(gamemap::location::SOUTH_EAST));
	}

	switch (state()) {
		case STATE_STANDING:
			// Standing units don't overlap anything.
			break;
		default:
			gamemap::location arr[6];
			get_adjacent_tiles(loc, arr);
			for (unsigned int i = 0; i < 6; i++) {
				over.insert(arr[i]);
			}
	}
	if(offset_ > 0) {
		gamemap::location adj_loc = loc.get_direction(facing_);
		over.insert(adj_loc);
		gamemap::location arr[6];
		get_adjacent_tiles(adj_loc, arr);
		for (unsigned int i = 0; i < 6; i++) {
			over.insert(arr[i]);
		}
	} else if(offset_ < 0) {
		gamemap::location opp_loc = loc.get_direction(loc.get_opposite_dir(facing_));
		over.insert(opp_loc);
		gamemap::location arr[6];
		get_adjacent_tiles(opp_loc, arr);
		for (unsigned int i = 0; i < 6; i++) {
			over.insert(arr[i]);
		}
	} else {
	// we stay in our hex, do nothing
	}

	return over;
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
	if(cfg_["upkeep"] == "full") {
		return level();
	}
	if(cfg_["upkeep"] == "loyal") {
		return 0;
	}
	return lexical_cast_default<int>(cfg_["upkeep"]);
}

bool unit::is_flying() const
{
	return flying_;
}

int unit::movement_cost_internal(gamemap::TERRAIN terrain, int recurse_count) const
{
	const int impassable = 10000000;

	const std::map<gamemap::TERRAIN,int>::const_iterator i = movement_costs_.find(terrain);
	if(i != movement_costs_.end()) {
		return i->second;
	}

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
			const int value = movement_cost_internal(*i,recurse_count+1);
			if(value < ret_value && !revert) {
				ret_value = value;
			} else if(value > ret_value && revert) {
				ret_value = value;
			}
		}

		movement_costs_.insert(std::pair<gamemap::TERRAIN,int>(terrain,ret_value));

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

	movement_costs_.insert(std::pair<gamemap::TERRAIN,int>(terrain,res));
	return res;
}

int unit::movement_cost(gamemap::TERRAIN terrain) const
{
	const int res = movement_cost_internal(terrain, 0);
	if(utils::string_bool(get_state("slowed"))) {
		return res*2;
	}
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
	const std::string& underlying = map_->underlying_def_terrain(terrain);
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

bool unit::resistance_filter_matches(const config& cfg,bool attacker,const attack_type& damage_type) const
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
			res = 100 - lexical_cast_default<int>(val);
		}
	}

	unit_ability_list resistance_abilities = get_abilities("resistance",loc);
	for(std::vector<std::pair<config*,gamemap::location> >::iterator i = resistance_abilities.cfgs.begin(); i != resistance_abilities.cfgs.end();) {
		if(!resistance_filter_matches(*i->first,attacker,damage_type)) {
			i = resistance_abilities.cfgs.erase(i);
		} else {
			++i;
		}
	}
	if(!resistance_abilities.empty()) {
		unit_abilities::effect resist_effect(resistance_abilities,res,false);

		res = minimum<int>(resist_effect.get_composite_value(),resistance_abilities.highest("max_value").first);
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
	cfg_.clear_children("movement_costs");
	cfg_.clear_children("defense");
	cfg_.clear_children("resistance");
	cfg_.add_child("movement_costs",movement_b_);
	cfg_.add_child("defense",defense_b_);
	cfg_.add_child("resistance",resistance_b_);
}
void unit::backup_state()
{
	max_hit_points_b_ = max_hit_points_;
	max_experience_b_ = max_experience_;
	max_movement_b_ = max_movement_;
	attacks_b_ = attacks_;
	if(cfg_.child("movement_costs")) {
		movement_b_ = *cfg_.child("movement_costs");
	} else {
		movement_b_ = config();
	}
	if(cfg_.child("defense")) {
		defense_b_ = *cfg_.child("defense");
	} else {
		defense_b_ = config();
	}
	if(cfg_.child("resistance")) {
		resistance_b_ = *cfg_.child("resistance");
	} else {
		resistance_b_ = config();
	}
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
		if (!utils::string_bool((**i)["strict_amla"]) || advances_to_.empty()) {
			if(modification_count("advance",(**i)["id"]) < lexical_cast_default<size_t>((**i)["max_times"],1)) {
			   std::vector<std::string> temp = utils::split((**i)["require_amla"]);
			   if(temp.size()){
			      std::sort(temp.begin(),temp.end());
			      std::vector<std::string> uniq;
			      std::unique_copy(temp.begin(),temp.end(),std::back_inserter(uniq));
			      bool requirements_done=true;
			      for(std::vector<std::string>::const_iterator ii = uniq.begin(); ii != uniq.end(); ii++){
					int required_num = std::count(temp.begin(),temp.end(),*ii);
					int mod_num = modification_count("advance",*ii);
					if(required_num>mod_num){
						requirements_done=false;
					}
			      }
			     if(requirements_done){
					res.push_back(*i);
			      }
		          }else{
				res.push_back(*i);
		          }
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

namespace {
/* Helper function for add_modifications */
void mod_mdr_merge(config& dst, const config& mod, bool delta)
{
	string_map::const_iterator iter = mod.values.begin();
	string_map::const_iterator end = mod.values.end();
	for (; iter != end; iter++) {
		dst[iter->first] =
				lexical_cast_default<std::string>(
				(delta == true)*lexical_cast_default<int>(dst[iter->first])
				+ lexical_cast_default<int>(iter->second)
				);
	}
}
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
			advance_to(&var->second.get_variation(variation_));
		} else if(apply_to == "new_attack") {
			attacks_.push_back(attack_type(**i.first,id(),image_fighting((**i.first)["range"]=="ranged" ? attack_type::LONG_RANGE : attack_type::SHORT_RANGE)));
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

			if(heal_full.empty() == false && utils::string_bool(heal_full,true)) {
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
			cfg_["upkeep"] = "loyal";
		} else if(apply_to == "status") {
			const std::string& add = (**i.first)["add"];
			const std::string& remove = (**i.first)["remove"];

			if(add.empty() == false) {
				set_state(add,"yes");
			}

			if(remove.empty() == false) {
				set_state(remove,"");
			}
		} else if (apply_to == "movement_costs") {
			config *mv = cfg_.child("movement_costs");
			config *ap = (**i.first).child("movement_costs");
			const std::string& replace = (**i.first)["replace"];
			if(!mv) {
				mv = &cfg_.add_child("movement_costs");
			}
			if (ap) {
				mod_mdr_merge(*mv, *ap, !utils::string_bool(replace));
			}
			movement_costs_.clear();
		} else if (apply_to == "defense") {
			config *mv = cfg_.child("defense");
			config *ap = (**i.first).child("defense");
			const std::string& replace = (**i.first)["replace"];
			if(!mv) {
				mv = &cfg_.add_child("defense");
			}
			if (ap) {
				mod_mdr_merge(*mv, *ap, !utils::string_bool(replace));
			}
		} else if (apply_to == "resistance") {
			config *mv = cfg_.child("resistance");
			config *ap = (**i.first).child("resistance");
			const std::string& replace = (**i.first)["replace"];
			if(!mv) {
				mv = &cfg_.add_child("resistance");
			}
			if (ap) {
				mod_mdr_merge(*mv, *ap, !utils::string_bool(replace));
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
const std::string& unit::image_ellipse() const
{
	return cfg_["ellipse"];
}
const std::string& unit::image_halo() const
{
	return cfg_["halo"];
}
const std::string& unit::image_profile() const
{
	const std::string& val = cfg_["profile"];
	if(val.size() == 0)
		return absolute_image();
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
		return absolute_image();
	}
}


const std::string& unit::image_healing() const
{
	const std::string& val = cfg_["image_healing"];
	if(val.empty()) {
		return absolute_image();
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

const defensive_animation& unit::defend_animation(const std::string &terrain,
		fighting_animation::hit_type hits, const attack_type* attack) const
{
	//select one of the matching animations at random
	std::vector<const defensive_animation*> options;
	int max_val = -1;
	for(std::vector<defensive_animation>::const_iterator i = defensive_animations_.begin(); i != defensive_animations_.end(); ++i) {
		int matching = i->matches(terrain,facing_,hits,attack);
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
const unit_animation* unit::extra_animation(const std::string &terrain,const std::string &flag) const
{
	//select one of the matching animations at random
	std::vector<const unit_animation*> options;
	int max_val = -1;
	std::multimap<std::string,unit_animation>::const_iterator i;
	for(i = extra_animations_.lower_bound(flag); i != extra_animations_.upper_bound(flag); ++i) {
		int matching = i->second.matches(terrain,facing_);
		if(matching == max_val) {
			options.push_back(&i->second);
		} else if(matching > max_val) {
			max_val = matching;
			options.erase(options.begin(),options.end());
			options.push_back(&i->second);
		}
	}
	if(extra_animations_.lower_bound(flag) == extra_animations_.end()) { return NULL;}

	return options[rand()%options.size()];
}
const death_animation& unit::die_animation(const std::string &terrain,
		fighting_animation::hit_type hits,const attack_type* attack) const
{
	//select one of the matching animations at random
	std::vector<const death_animation*> options;
	int max_val = -1;
	for(std::vector<death_animation>::const_iterator i = death_animations_.begin(); i != death_animations_.end(); ++i) {
		int matching = i->matches(terrain,facing_,hits,attack);
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

const standing_animation& unit::stand_animation(const std::string terrain,gamemap::location::DIRECTION dir) const
{
	//select one of the matching animations at random
	std::vector<const standing_animation*> options;
	int max_val = -1;
	for(std::vector<standing_animation>::const_iterator i = standing_animations_.begin(); i != standing_animations_.end(); ++i) {
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

const leading_animation& unit::lead_animation(const std::string terrain,gamemap::location::DIRECTION dir) const
{
	//select one of the matching animations at random
	std::vector<const leading_animation*> options;
	int max_val = -1;
	for(std::vector<leading_animation>::const_iterator i = leading_animations_.begin(); i != leading_animations_.end(); ++i) {
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


const healing_animation& unit::heal_animation(const std::string terrain,gamemap::location::DIRECTION dir) const
{
	//select one of the matching animations at random
	std::vector<const healing_animation*> options;
	int max_val = -1;
	for(std::vector<healing_animation>::const_iterator i = healing_animations_.begin(); i != healing_animations_.end(); ++i) {
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


void unit::apply_modifications()
{
	log_scope("apply mods");
	reset_modifications();
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

bool unit::invisible(const gamemap::location& loc,
		const unit_map& units,const std::vector<team>& teams) const
{
	bool is_inv = false;

	static const std::string hides("hides");
	if (utils::string_bool(get_state(hides))) {
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
		const gamemap& map,
		const std::vector<team>& teams, const team& current_team)
{
	unit_map::iterator u = units.find(loc);
	if(map.on_board(loc)){
		if(u != units.end()){
			if(current_team.fogged(loc.x, loc.y)){
				return units.end();
			}
			if(current_team.is_enemy(u->second.side()) &&
					u->second.invisible(loc,units,teams)) {
				return units.end();
			}
		}
	}
	return u;
}

unit_map::const_iterator find_visible_unit(const unit_map& units,
		const gamemap::location loc,
		const gamemap& map,
		const std::vector<team>& teams, const team& current_team)
{
	unit_map::const_iterator u = units.find(loc);
	if(map.on_board(loc)){
		if(u != units.end()){
			if(current_team.fogged(loc.x, loc.y)){
				return units.end();
			}
			if(current_team.is_enemy(u->second.side()) &&
					u->second.invisible(loc,units,teams)) {
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
	wassert(false); // deprecated code
	return team_name(side, units);
}

temporary_unit_placer::temporary_unit_placer(unit_map& m, const gamemap::location& loc, const unit& u)
	: m_(m), loc_(loc), temp_(m.extract(loc))
{
	m.add(new std::pair<gamemap::location,unit>(loc,u));
}

temporary_unit_placer::~temporary_unit_placer()
{
	m_.erase(loc_);
	if(temp_) {
		m_.add(temp_);
	}
}
