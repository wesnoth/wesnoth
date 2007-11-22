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

//! @file unit.cpp
//! Routines to manage units.

#include "global.hpp"

#include "game_config.hpp"
#include "game_errors.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "pathfind.hpp"
#include "random.hpp"
#include "unit.hpp"
#include "unit_types.hpp"
#include "unit_abilities.hpp"
#include "util.hpp"
#include "wassert.hpp"
#include "serialization/string_utils.hpp"
#include "halo.hpp"
#include "game_display.hpp"
#include "gamestatus.hpp"
#include "actions.hpp"
#include "game_events.hpp"
#include "sound.hpp"
#include "sdl_utils.hpp"
#include "terrain_filter.hpp"
#include "variable.hpp"

#include <climits>
#include <ctime>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iterator>

#define LOG_UT LOG_STREAM(info, engine)

namespace {
	const std::string ModificationTypes[] = { "advance", "trait", "object" };
	const size_t NumModificationTypes = sizeof(ModificationTypes)/
										sizeof(*ModificationTypes);

	//! Pointers to units which have data in their internal caches.
	// The destructor of an unit removes itself from the cache,
	// so the pointers are always valid.
	static std::vector<const unit *> units_with_cache;
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
           abilities_b_(o.abilities_b_),

           advances_to_(o.advances_to_),
           id_(o.id_),
           race_(o.race_),
           name_(o.name_),
           description_(o.description_),
           custom_unit_description_(o.custom_unit_description_),
           underlying_description_(o.underlying_description_),
           language_name_(o.language_name_),
           undead_variation_(o.undead_variation_),
           variation_(o.variation_),

           hit_points_(o.hit_points_),
           max_hit_points_(o.max_hit_points_),
           max_hit_points_b_(o.max_hit_points_b_),
           experience_(o.experience_),
           max_experience_(o.max_experience_),
           max_experience_b_(o.max_experience_b_),
           level_(o.level_),
           alignment_(o.alignment_),
           flag_rgb_(o.flag_rgb_),
           image_mods_(o.image_mods_),

           unrenamable_(o.unrenamable_),
           side_(o.side_),
           gender_(o.gender_),

           alpha_(o.alpha_),

           recruits_(o.recruits_),

           movement_(o.movement_),
           max_movement_(o.max_movement_),
           max_movement_b_(o.max_movement_b_),
           movement_costs_(o.movement_costs_),
           hold_position_(o.hold_position_),
           end_turn_(o.end_turn_),
           resting_(o.resting_),
           attacks_left_(o.attacks_left_),
           max_attacks_(o.max_attacks_),

           states_(o.states_),
           variables_(o.variables_),
           emit_zoc_(o.emit_zoc_),
           state_(o.state_),

           overlays_(o.overlays_),

           role_(o.role_),
           ai_special_(o.ai_special_),
           attacks_(o.attacks_),
           attacks_b_(o.attacks_b_),
           facing_(o.facing_),

           traits_description_(o.traits_description_),
           unit_value_(o.unit_value_),
           goto_(o.goto_),
           interrupted_move_(o.interrupted_move_),
           flying_(o.flying_),
           is_fearless_(o.is_fearless_),
           is_healthy_(o.is_healthy_),

           modification_descriptions_(o.modification_descriptions_),

           animations_(o.animations_),

           anim_(NULL),

           frame_begin_time_(o.frame_begin_time_),
           offset_(o.offset_),
           unit_halo_(o.unit_halo_),
           unit_anim_halo_(o.unit_anim_halo_),
           getsHit_(o.getsHit_),
           refreshing_(o.refreshing_),
           hidden_(o.hidden_),
           draw_bars_(o.draw_bars_),

           modifications_(o.modifications_),
           gamedata_(o.gamedata_),
           units_(o.units_),
           map_(o.map_),
           gamestatus_(o.gamestatus_),
           teams_(o.teams_)
{
	next_idling_ = 0;
	unit_halo_ = halo::NO_HALO;
	unit_anim_halo_ = halo::NO_HALO;
}

//! Initilizes a unit from a config.
unit::unit(const game_data* gamedata, unit_map* unitmap, const gamemap* map,
           const gamestatus* game_status, const std::vector<team>* teams,const config& cfg,
		   bool use_traits) : movement_(0), hold_position_(false), resting_(false),
		   state_(STATE_STANDING), facing_(gamemap::location::NORTH_EAST), flying_(false),
           anim_(NULL), next_idling_(0), frame_begin_time_(0), unit_halo_(halo::NO_HALO),
           unit_anim_halo_(halo::NO_HALO), draw_bars_(false),gamedata_(gamedata),
           units_(unitmap), map_(map), gamestatus_(game_status),teams_(teams)
{
	read(cfg, use_traits);
	getsHit_=0;
	end_turn_ = false;
	refreshing_  = false;
	hidden_ = false;
	offset_ = 0;
	game_config::add_color_info(cfg);
}

unit::unit(const game_data& gamedata,const config& cfg,bool use_traits) : movement_(0),
           hold_position_(false), resting_(false), state_(STATE_STANDING),
           facing_(gamemap::location::NORTH_EAST),
           flying_(false),anim_(NULL),next_idling_(0),frame_begin_time_(0),
           unit_halo_(halo::NO_HALO),unit_anim_halo_(halo::NO_HALO),draw_bars_(false),
           gamedata_(&gamedata), units_(NULL),map_(NULL), gamestatus_(NULL)
{
	read(cfg,use_traits);
	getsHit_=0;
	end_turn_ = false;
	refreshing_  = false;
	hidden_ = false;
	offset_ = 0;
}

void unit::clear_status_caches()
{
	for(std::vector<const unit *>::const_iterator itor = units_with_cache.begin();
			itor != units_with_cache.end(); ++itor) {
		(*itor)->clear_visibility_cache();
	}

	units_with_cache.clear();
}

unit_race::GENDER unit::generate_gender(const unit_type& type, bool gen)
{
	const std::vector<unit_race::GENDER>& genders = type.genders();
	// Once random gender is used, don't do it again.
	// Such as when restoring a saved character.
	cfg_["random_gender"] = "no";
	if(genders.empty() == false) {
		return gen ? genders[get_random()%genders.size()] : genders.front();
	} else {
		return unit_race::MALE;
	}
}

//! Initializes a unit from a unit type.
unit::unit(const game_data* gamedata, unit_map* unitmap, const gamemap* map,
           const gamestatus* game_status, const std::vector<team>* teams, const unit_type* t,
           int side, bool use_traits, bool dummy_unit, unit_race::GENDER gender, std::string variation) :
           variation_(variation), gender_(dummy_unit ? gender : generate_gender(*t,use_traits)), resting_(false),
           state_(STATE_STANDING), facing_(gamemap::location::NORTH_EAST),draw_bars_(false),
           gamedata_(gamedata),units_(unitmap),map_(map),gamestatus_(game_status),teams_(teams)
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
		// Units that don't have traits generated are just generic units,
		// so they shouldn't get a description either.
		custom_unit_description_ = generate_description();
	}
	generate_traits(!use_traits);
	apply_modifications();
	if(underlying_description_.empty()){
		char buf[80];
		if(!custom_unit_description_.empty()){
			snprintf(buf, sizeof(buf), "%s-%d-%s",type()->id().c_str(), get_random(), custom_unit_description_.c_str());
		} else {
			snprintf(buf, sizeof(buf), "%s-%d",type()->id().c_str(), get_random());
		}
		underlying_description_ = buf;
	}

	unrenamable_ = false;
	anim_ = NULL;
	getsHit_=0;
	end_turn_ = false;
	hold_position_ = false;
	offset_ = 0;
	next_idling_ = 0;
	frame_begin_time_ = 0;
	unit_halo_ = halo::NO_HALO;
	unit_anim_halo_ = halo::NO_HALO;
}
unit::unit(const unit_type* t, int side, bool use_traits, bool dummy_unit, unit_race::GENDER gender, std::string variation) :
           variation_(variation),gender_(dummy_unit ? gender : generate_gender(*t,use_traits)),
           state_(STATE_STANDING),facing_(gamemap::location::NORTH_EAST),draw_bars_(false),
           gamedata_(NULL), units_(NULL),map_(NULL),gamestatus_(NULL),teams_(NULL)
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
		// Units that don't have traits generated are just generic units,
		// so they shouldn't get a description either.
		custom_unit_description_ = generate_description();
	}
	generate_traits(!use_traits);
	apply_modifications();
	if(underlying_description_.empty()){
		char buf[80];
		if(!custom_unit_description_.empty()){
			snprintf(buf, sizeof(buf), "%s-%d-%s",type()->id().c_str(), get_random(), custom_unit_description_.c_str());
		}else{
			snprintf(buf, sizeof(buf), "%s-%d",type()->id().c_str(), get_random());
		}
		underlying_description_ = buf;
	}

	unrenamable_ = false;
	next_idling_ = 0;
	frame_begin_time_ = 0;
	anim_ = NULL;
	getsHit_=0;
	end_turn_ = false;
	hold_position_ = false;
	offset_ = 0;
}

unit::~unit()
{
	clear_haloes();

	delete anim_;

	// Remove us from the status cache
	std::vector<const unit *>::iterator itor =
	std::find(units_with_cache.begin(), units_with_cache.end(), this);

	if(itor != units_with_cache.end()) {
		units_with_cache.erase(itor);
	}
}



unit& unit::operator=(const unit& u)
{
	// Use copy constructor to make sure we are coherant
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

	// In case the unit carries EventWML, apply it now
	game_events::add_events(cfg_.get_children("event"),id_);
}


void unit::add_trait(std::string /*trait*/)
{
	//modifications_.add_child("trait", cfg);
	apply_modifications();
}

// Apply mandatory traits (e.g. undead, mechanical) to a unit and then
// fill out with avaiable (leaders have a restircted set of available traits)
// traits until no more are available or the unit has its maximum number
// of traits.
// This routine does not apply the effects of added traits to a unit.
// That must be done by the caller.
// Note that random numbers used in config files don't work in multiplayer,
// so that leaders should be barred from all random traits until that
// is fixed. Later the restrictions will be based on play balance.
// musthavepnly is true when you don't want to generate random traits or
// you don't want to give any optional traits to a unit.

void unit::generate_traits(bool musthaveonly)
{
	wassert(gamedata_ != NULL);
	LOG_UT << "Generating a trait for unit type " << id() << " with musthaveonly " << musthaveonly << "\n";
	const game_data::unit_type_map::const_iterator type = gamedata_->unit_types.find(id());
	// Calculate the unit's traits
	if (type == gamedata_->unit_types.end()) {
		std::string error_message = _("Unknown unit type '$type|'");
		utils::string_map symbols;
		symbols["type"] = id();
		error_message = utils::interpolate_variables_into_string(error_message, &symbols);
		LOG_STREAM(err, engine) << "unit of type " << id() << " not found!\n";
		throw game::game_error(error_message);
	}
	std::vector<config*> candidate_traits = type->second.possible_traits();
	std::vector<config*> traits;

	// First remove traits the unit already has from consideration.
	// And count them so that we can figure out how many more are needed.
	size_t t = 0;
	config::child_list const &mods = modifications_.get_children("trait");
	for(config::child_list::const_iterator j = mods.begin(), j_end = mods.end(); j != j_end; ++j) {
		++t;
		size_t m = 0;
		for(size_t n = 0; n < candidate_traits.size(); ++n) {
			if((**(candidate_traits.begin()+m))["id"] == (**j)["id"]) {
				candidate_traits.erase(candidate_traits.begin()+m);
			} else {
				++m;
			}
		}
	}

	// Next add in any mandatory traits. These aren't limited by the
	// number of traits allowed for a unit. They also don't use
	// any random numbers for assignment. (And hence don't cause
	// problems for multiplayer.)
	size_t num_traits = candidate_traits.size();
	size_t m = 0;
	for(size_t n = 0; n < num_traits; ++n) {
		if(!(**(candidate_traits.begin()+m))["availability"].empty() &&
		(**(candidate_traits.begin()+m))["availability"] == "musthave") {
			traits.push_back(candidate_traits[m]);
			candidate_traits.erase(candidate_traits.begin()+m);
			++t;
		} else {
			++m;
		}
	}

	// If musthaveonly then don't generate any random/optional traits
	if(!musthaveonly) {
		// Next for leaders remove any traits that are not available to
		// the "any" category.
		if(can_recruit()) {
			num_traits = candidate_traits.size();
			m = 0;
			for(size_t n = 0; n < num_traits; ++n) {
				if(!(**(candidate_traits.begin()+m))["availability"].empty() ||
				(**(candidate_traits.begin()+m))["availability"] != "any") {
					candidate_traits.erase(candidate_traits.begin()+m);
				} else {
					++m;
				}
			}
		}

		// Now randomly fill out to the number of traits required or until
		// there aren't any more traits.
		num_traits = type->second.num_traits();
		for(size_t n = t; n < num_traits && candidate_traits.empty() == false; ++n) {
			const size_t num = get_random()%candidate_traits.size();
			traits.push_back(candidate_traits[num]);
			candidate_traits.erase(candidate_traits.begin()+num);
		}

		// Once random traits are added, don't do it again.
		// Such as when restoring a saved character.
		cfg_["random_traits"]="no";
	}

	for(std::vector<config*>::const_iterator j2 = traits.begin(); j2 != traits.end(); ++j2) {
		modifications_.add_child("trait",**j2);
	}
}

//! Advance this unit to another type
void unit::advance_to(const unit_type* t, bool use_traits)
{
	t = &t->get_gender_unit_type(gender_).get_variation(variation_);
	reset_modifications();
	// Remove old animations
	cfg_.clear_children("animation");

	cfg_.clear_children("attack");
	cfg_.clear_children("abilities");
	// Clear cache of movement costs
	movement_costs_.clear();

	if(t->movement_type().get_parent()) {
		cfg_.merge_with(t->movement_type().get_parent()->get_cfg());
	}
	// If unit has specific profile, remember it and have it after advaces
	bool specific_profile = false;
	std::string profile;
	if (type() != NULL)
	{
		specific_profile = (cfg_["profile"] != type()->cfg_["profile"]);
		if (specific_profile)
		{
			profile = cfg_["profile"];
		}
	}
	cfg_.merge_with(t->cfg_);
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

	animations_ = t->animations_;

	flag_rgb_ = t->flag_rgb();

	backup_state();

	bool do_heal = false; // Track whether unit should get fully healed.

	if(utils::string_bool(cfg_["random_gender"], false)) {
		generate_gender(*t,true);
	}

	if(id()!=t->id() || cfg_["gender"] != cfg_["gender_id"]) {
		do_heal = true; // Can't heal until after mods applied.
		id_ = t->id();
		cfg_["id"] = id_;
		cfg_["gender_id"] = cfg_["gender"];
	}

	if(utils::string_bool(cfg_["random_traits"], true)) {
		generate_traits(!use_traits);
	} else {
		// This will add any "musthave" traits to the new unit that it doesn't already have.
		// This covers the Dark Sorcerer advancing to Lich and gaining the "undead" trait,
		// but random and/or optional traits are not added,
		// and neither are inappropiate traits removed.
		generate_traits(true);
	}

	// Apply modifications etc, refresh the unit.
	// This needs to be after type and gender are fixed,
	// since there can be filters on the modifications
	// that may result in different effects after the advancement.
	apply_modifications();

	// Not that the unit has all of its modifications applied, it is
	// OK to heal it.
	if (do_heal) {
		heal_all();
	}

	game_events::add_events(cfg_.get_children("event"),id_);
	cfg_.clear_children("event");

	set_state("poisoned","");
	set_state("slowed","");
	set_state("stoned","");
	end_turn_ = false;
	refreshing_  = false;
	hidden_ = false;
}

const unit_type* unit::type() const
{
	wassert(gamedata_ != NULL);
	std::map<std::string,unit_type>::const_iterator i = gamedata_->unit_types.find(id());
	if(i != gamedata_->unit_types.end()) {
		return &i->second;
	}
	if (!id().empty())
		LOG_STREAM(err, engine) << "type not found for nonempty unit " << id() << ", returning NULL!\n";

	return NULL;
}

//! The unit's profile.
const std::string& unit::profile() const
{
	if(cfg_["profile"] != "" && cfg_["profile"] != "unit_image") {
		return cfg_["profile"];
	}
	return absolute_image();
}

//! Colors for the unit's hitpoints.
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

//! Colors for the unit's XP.
SDL_Colour unit::xp_color() const
{
	const SDL_Color near_advance_colour = {255,255,255,0};
	const SDL_Color mid_advance_colour  = {150,255,255,0};
	const SDL_Color far_advance_colour  = {0,205,205,0};
	const SDL_Color normal_colour 	  = {0,160,225,0};
	const SDL_Color near_amla_colour	  = {225,0,255,0};
	const SDL_Color mid_amla_colour	  = {169,30,255,0};
	const SDL_Color far_amla_colour	  = {139,0,237,0};
	const SDL_Color amla_colour		  = {100,0,150,0};
	const bool near_advance = max_experience() - experience() <= game_config::kill_experience;
	const bool mid_advance  = max_experience() - experience() <= game_config::kill_experience*2;
	const bool far_advance  = max_experience() - experience() <= game_config::kill_experience*3;
	
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

void unit::set_movement(int moves)
{
	hold_position_ = false;
	end_turn_ = false;
	movement_ = maximum<int>(0,minimum<int>(moves,max_movement_));
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
	if((movement_ != total_movement()) && !utils::string_bool(get_state("not_moved")) && (!is_healthy_ || attacks_left_ < max_attacks_)) {
		resting_ = false;
	}
	set_state("not_moved","");
	// Clear interrupted move
	set_interrupted_move(gamemap::location());
}
void unit::new_level()
{
	role_ = "";
	ai_special_ = "";

	// Set the goto-command to be going to no-where
	goto_ = gamemap::location();

	remove_temporary_modifications();

	// Re-apply all permanent modifications
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

void unit::remove_ability_by_id(const std::string &ability)
{
	config* abil = cfg_.child("abilities");
	if(abil) {
		config::all_children_iterator i = abil->ordered_begin();
		while(i != abil->ordered_end()) {
			if(i.get_child()["id"] == ability) {
				i = abil->erase(i);
			} else {
				++i;
			}
		}
	}
}

bool unit::matches_filter(const vconfig& cfg, const gamemap::location& loc, bool use_flat_tod) const
{
	bool matches = true;

	if(loc.valid()) {
		wassert(units_ != NULL);
		scoped_xy_unit auto_store("this_unit", loc.x, loc.y, *units_);
		matches = internal_matches_filter(cfg, loc, use_flat_tod);
	} else {
		// If loc is invalid, then this is a recall list unit (already been scoped)
		matches = internal_matches_filter(cfg, loc, use_flat_tod);
	}

	// Handle [and], [or], and [not] with in-order precedence
	config::all_children_iterator cond = cfg.get_config().ordered_begin();
	config::all_children_iterator cond_end = cfg.get_config().ordered_end();
	while(cond != cond_end)
	{

		const std::string& cond_name = *((*cond).first);
		const vconfig cond_filter(&(*((*cond).second)));

		// Handle [and]
		if(cond_name == "and") {
			matches = matches && matches_filter(cond_filter,loc,use_flat_tod);
		}
		// Handle [or]
		else if(cond_name == "or") {
			matches = matches || matches_filter(cond_filter,loc,use_flat_tod);
		}
		// Handle [not]
		else if(cond_name == "not") {
			matches = matches && !matches_filter(cond_filter,loc,use_flat_tod);
		}

		++cond;
	}
	return matches;
}

bool unit::internal_matches_filter(const vconfig& cfg, const gamemap::location& loc, bool use_flat_tod) const
{
	const t_string& t_description = cfg["description"];
	const t_string& t_speaker = cfg["speaker"];
	const t_string& t_type = cfg["type"];
	const t_string& t_ability = cfg["ability"];
	const t_string& t_side = cfg["side"];
	const t_string& t_weapon = cfg["has_weapon"];
	const t_string& t_role = cfg["role"];
	const t_string& t_ai_special = cfg["ai_special"];
	const t_string& t_race = cfg["race"];
	const t_string& t_gender = cfg["gender"];
	const t_string& t_canrecruit = cfg["canrecruit"];
	const t_string& t_level = cfg["level"];
	const t_string& t_defense = cfg["defense"];
	const t_string& t_movement_cost = cfg["movement_cost"];

	const std::string& description = t_description;
	const std::string& speaker = t_speaker;
	const std::string& type = t_type;
	const std::string& ability = t_ability;
	const std::string& side = t_side;
	const std::string& weapon = t_weapon;
	const std::string& role = t_role;
	const std::string& ai_special = t_ai_special;
	const std::string& race = t_race;
	const std::string& gender = t_gender;
	const std::string& canrecruit = t_canrecruit;
	const std::string& level = t_level;
	const std::string& defense = t_defense;
	const std::string& mvt_cost = t_movement_cost;

	if(description.empty() == false && description != this->underlying_description()) {
		return false;
	}

	// Allow 'speaker' as an alternative to description, since people use it so often
	if(speaker.empty() == false && speaker != this->underlying_description()) {
		return false;
	}

	if(cfg.has_child("filter_location")) {
		wassert(map_ != NULL);
		wassert(gamestatus_ != NULL);
		wassert(units_ != NULL);
		bool res = terrain_matches_filter(*map_, loc, cfg.child("filter_location"), *gamestatus_, *units_, use_flat_tod);
		if(res == false) {
			return false;
		}
	}
	// Also allow filtering on location ranges outside of the location filter
	const t_string& cfg_x = cfg["x"];
	const t_string& cfg_y = cfg["y"];
	if(!cfg_x.empty() || !cfg_y.empty()){
		if(cfg_x == "recall" && cfg_y == "recall") {
			//locations on the map are considered to not be on a recall list
			if((!map_ && loc.valid()) || (map_ && map_->on_board(loc))) {
				return false;
			}
		} else if(!loc.matches_range(cfg_x, cfg_y)) {
			return false;
		}
	}

	const std::string& this_type = id();

	// The type could be a comma separated list of types
	if(type.empty() == false && type != this_type) {

		// We only do the full CSV search if we find a comma in there,
		// and if the subsequence is found within the main sequence.
		// This is because doing the full CSV split is expensive.
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

	if(race.empty() == false && race_->id() != race) {
		return false;
	}

	if(gender.empty() == false) {
		if(string_gender(gender) != this->gender()) {
			return false;
		}
	}

	if(side.empty() == false && this->side() != lexical_cast_default<unsigned>(side)) {
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

		if(!has_weapon) {
			return false;
		}
	}

	if(role.empty() == false && role_ != role) {
		return false;
	}

	if(ai_special.empty() == false && ai_special_ != ai_special) {
		return false;
	}

	if(canrecruit.empty() == false && utils::string_bool(canrecruit) != can_recruit()) {
		return false;
	}

	if(level.empty() == false && level_ != lexical_cast_default<int>(level,-1)) {
		return false;
	}

	if(defense.empty() == false && defense_modifier(map_->get_terrain(loc)) != lexical_cast_default<int>(defense,-1)) {
		return false;
	}

	if(mvt_cost.empty() == false && movement_cost(map_->get_terrain(loc)) != lexical_cast_default<int>(mvt_cost,-1)) {
		return false;
	}

	// Now start with the new WML based comparison.
	// If a key is in the unit and in the filter, they should match
	// filter only => not for us
	// unit only => not filtered
	const vconfig::child_list& wmlcfgs = cfg.get_children("wml_filter");
	if (!wmlcfgs.empty()) {
		config unit_cfg;
		write(unit_cfg);
		// Now, match the kids, WML based
		for(unsigned int i=0; i < wmlcfgs.size(); ++i) {
			if(!unit_cfg.matches(wmlcfgs[i].get_parsed_config())) {
				return false;
			}
		}
	}

	if (cfg.has_child("filter_adjacent")) {
		wassert(units_ && map_ && gamestatus_);
		gamemap::location adjacent[6];
		get_adjacent_tiles(loc, adjacent);
		vconfig::child_list::const_iterator i, i_end;
		const vconfig::child_list& adj_filt = cfg.get_children("filter_adjacent");
		for (i = adj_filt.begin(), i_end = adj_filt.end(); i != i_end; ++i) {
			int match_count=0;
			static std::vector<gamemap::location::DIRECTION> default_dirs
				= gamemap::location::parse_directions("n,ne,se,s,sw,nw");
			std::vector<gamemap::location::DIRECTION> dirs = (*i).has_attribute("adjacent")
				? gamemap::location::parse_directions((*i)["adjacent"]) : default_dirs;
			std::vector<gamemap::location::DIRECTION>::const_iterator j, j_end = dirs.end();
			for (j = dirs.begin(); j != j_end; ++j) {
				unit_map::const_iterator unit_itor = units_->find(adjacent[*j]);
				if (unit_itor == units_->end() 
				|| !unit_itor->second.matches_filter(*i, unit_itor->first, use_flat_tod)) {
					continue;
				}
				if (!(*i).has_attribute("is_enemy")
				|| utils::string_bool((*i)["is_enemy"]) == (*gamestatus_->teams)[this->side()-1].is_enemy(unit_itor->second.side())) {
					++match_count;
				}
			}
			static std::vector<std::pair<int,int> > default_counts = utils::parse_ranges("1-6");
			std::vector<std::pair<int,int> > counts = (*i).has_attribute("count") 
				? utils::parse_ranges((*i)["count"]) : default_counts;
			std::vector<std::pair<int,int> >::const_iterator count, count_end = counts.end();
			bool count_matches = false;
			for (count = counts.begin(); count != count_end && !count_matches; ++count) {
				if(count->first <= match_count && match_count <= count->second) {
					count_matches = true;
				}
			}
			if(!count_matches) {
				return false;
			}
		}
	}

	if(cfg.has_attribute("find_in")) {
		// Allow filtering by searching a stored variable of units
		wassert(gamestatus_ != NULL);
		variable_info vi(cfg["find_in"], false, variable_info::TYPE_CONTAINER);
		if(!vi.is_valid) return false;
		if(vi.explicit_index) {
			if(description_ != (vi.vars->get_children(vi.key)[vi.index])->get_attribute("description")) {
				return false;
			}
		} else {
			config::child_itors ch_itors = vi.vars->child_range(vi.key);
			for(; ch_itors.first != ch_itors.second; ++ch_itors.first) {
				if(description_ == (*ch_itors.first)->get_attribute("description")) {
					break;
				}
			}
			if(ch_itors.first == ch_itors.second) {
				return false;
			}
		}
	}

	return true;
}


//! Initialize this unit from a cfg object.
//!
//! @param cfg			Configuration object from which to read the unit
//- @param use_traits	??
void unit::read(const config& cfg, bool use_traits)
{
	if(cfg["id"].empty() && cfg["type"].empty()) {
		throw game::load_game_failed("Attempt to de-serialize an empty unit");
	}
	cfg_ = cfg;
	side_ = lexical_cast_default<int>(cfg["side"]);
	if(side_ <= 0) {
		side_ = 1;
	}

	validate_side(side_);
	bool id_set = cfg["id"] != "";

	// Prevent un-initialized variables
	max_hit_points_=1;
	hit_points_=1;
	max_movement_=0;
	max_experience_=0;
	/* */

	gender_ = string_gender(cfg["gender"]);

	variation_ = cfg["variation"];

	wassert(gamedata_ != NULL);
	description_ = cfg["description"];
	custom_unit_description_ = cfg["user_description"];
	std::string custom_unit_desc = cfg["unit_description"];

	underlying_description_ = cfg["description"];
	if(underlying_description_.empty()){
		char buf[80];
		snprintf(buf, sizeof(buf), "%s-%d",cfg["type"].c_str(), get_random());
		underlying_description_ = buf;
	}
	if(description_.empty()) {
		description_ = cfg["type"].c_str();
	}

	role_ = cfg["role"];
	ai_special_ = cfg["ai_special"];
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
	if(!(cfg["type"].empty() || cfg["type"] == cfg["id"]) || cfg["gender"] != cfg["gender_id"]) {
		std::map<std::string,unit_type>::const_iterator i = gamedata_->unit_types.find(cfg["type"]);
		if(i != gamedata_->unit_types.end()) {
			
			advance_to(&i->second.get_gender_unit_type(gender_), use_traits);
			type_set = true;
		} else {
			std::string error_message = _("Unknown unit type '$type|'");
			utils::string_map symbols;
			symbols["type"] = cfg["type"];
			error_message = utils::interpolate_variables_into_string(error_message, &symbols);
			LOG_STREAM(err, engine) << "unit of type " << cfg["type"] << " not found!\n";
			throw game::game_error(error_message);
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
			t_atks.merge_with(u_atks);
			for(range = t_atks.child_range("attack");
				range.first != range.second; ++range.first) {
				attacks_.push_back(attack_type(**range.first));
			}
		} else {
			for(config::const_child_itors range = cfg.child_range("attack");
				range.first != range.second; ++range.first) {
				attacks_.push_back(attack_type(**range.first));
			}
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
		if(utils::string_bool(cfg_["random_traits"], true)) {
			generate_traits(!use_traits);
		}
		apply_modifications();
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
			animations_ = ut->animations_;
			cfg_.clear_children("animation");
		} else {
			unit_animation::initialize_anims(animations_,cfg_);
		}
	} else {
		// Remove animations from private cfg, since they're not needed there now
		cfg_.clear_children("animation");

		cfg_.clear_children("defend");
		cfg_.clear_children("teleport_anim");
		cfg_.clear_children("extra_anim");
		cfg_.clear_children("death");
		cfg_.clear_children("movement_anim");
		cfg_.clear_children("standing_anim");
		cfg_.clear_children("healing_anim");
		cfg_.clear_children("victory_anim");
		cfg_.clear_children("idle_anim");
		cfg_.clear_children("levelin_anim");
		cfg_.clear_children("levelout_anim");
		cfg_.clear_children("healed_anim");
		cfg_.clear_children("poison_anim");
	}
	game_events::add_events(cfg_.get_children("event"),id_);
	cfg_.clear_children("event");
	// Make the default upkeep "full"
	if(cfg_["upkeep"].empty()) {
		cfg_["upkeep"] = "full";
	}
}
void unit::write(config& cfg) const
{
	// If a location has been saved in the config, keep it
	std::string x = cfg["x"];
	std::string y = cfg["y"];
	cfg.append(cfg_);
	cfg.clear_children("movement_costs");
	cfg.clear_children("defense");
	cfg.clear_children("resistance");
	cfg.clear_children("abilities");
	cfg.add_child("movement_costs",movement_b_);
	cfg.add_child("defense",defense_b_);
	cfg.add_child("resistance",resistance_b_);
	cfg.add_child("abilities",abilities_b_);
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

	cfg["gender"] = gender_string(gender_);
	cfg["gender_id"] = gender_string(gender_);

	cfg["variation"] = variation_;

	cfg["role"] = role_;
	cfg["ai_special"] = ai_special_;
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

	cfg["race"] = race_->id();
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

	cfg["recruits"] = utils::join(recruits_);
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


const surface unit::still_image(bool scaled) const
{
	image::locator image_loc;

#ifdef LOW_MEM
	image_loc = image::locator(absolute_image());
#else
	std::string mods=image_mods();
	if(mods.size()){
		image_loc = image::locator(absolute_image(),mods);
	} else {
		image_loc = image::locator(absolute_image());
	}
#endif

	surface unit_image(image::get_image(image_loc, scaled ? image::SCALED_TO_ZOOM : image::UNSCALED));
	return unit_image;
}

void unit::set_standing(const game_display &disp,const gamemap::location& loc, bool with_bars)
{
	state_ = STATE_STANDING;
	start_animation(disp,loc,choose_animation(disp,loc,"standing"),with_bars,true);
}
void unit::set_defending(const game_display &disp,const gamemap::location& loc, int damage,const attack_type* attack,const attack_type* secondary_attack,int swing_num)
{
	state_ =  STATE_DEFENDING;

	unit_animation::hit_type hit_type;
	if(damage >= hitpoints()) {
		hit_type = unit_animation::KILL;
	} else if(damage > 0) {
		hit_type = unit_animation::HIT;
	}else {
		hit_type = unit_animation::MISS;
	}
	start_animation(disp,loc,choose_animation(disp,loc,"defend",damage,hit_type,attack,secondary_attack,swing_num),true);
}

void unit::set_extra_anim(const game_display &disp,const gamemap::location& loc, std::string flag)
{
	state_ =  STATE_EXTRA;
	start_animation(disp,loc,choose_animation(disp,loc,flag),false);

}

void unit::set_attacking(const game_display &disp,const gamemap::location& loc,int damage,const attack_type& type,const attack_type* secondary_attack,int swing_num)
{
	state_ =  STATE_ATTACKING;
	unit_animation::hit_type hit_type;
	if(damage >= hitpoints()) {
		hit_type = unit_animation::KILL;
	} else if(damage > 0) {
		hit_type = unit_animation::HIT;
	}else {
		hit_type = unit_animation::MISS;
	}
	start_animation(disp,loc,choose_animation(disp,loc,"attack",damage,hit_type,&type,secondary_attack,swing_num),true);

}
void unit::set_leading(const game_display &disp,const gamemap::location& loc)
{
	state_ = STATE_LEADING;
	start_animation(disp,loc,choose_animation(disp,loc,"leading"),true);
}
void unit::set_leveling_in(const game_display &disp,const gamemap::location& loc)
{
	state_ = STATE_LEVELIN;
	start_animation(disp,loc,choose_animation(disp,loc,"levelin"),false);
}
void unit::set_leveling_out(const game_display &disp,const gamemap::location& loc)
{
	state_ = STATE_LEVELOUT;
	start_animation(disp,loc,choose_animation(disp,loc,"levelout"),false);
}
void unit::set_recruited(const game_display &disp,const gamemap::location& loc)
{
	state_ = STATE_RECRUITED;
	start_animation(disp,loc,choose_animation(disp,loc,"recruited"),false);
}
void unit::set_healed(const game_display &disp,const gamemap::location& loc, int healing)
{
	state_ = STATE_HEALED;
	start_animation(disp,loc,choose_animation(disp,loc,"healed",healing),true);
}
void unit::set_poisoned(const game_display &disp,const gamemap::location& loc, int damage)
{
	state_ = STATE_POISONED;
	start_animation(disp,loc,choose_animation(disp,loc,"poisoned",damage),true);
}

void unit::set_teleporting(const game_display &disp,const gamemap::location& loc)
{
	state_ = STATE_TELEPORT;
	start_animation(disp,loc,choose_animation(disp,loc,"teleport"),false);
}

void unit::set_dying(const game_display &disp,const gamemap::location& loc,const attack_type* attack,const attack_type* secondary_attack)
{
	state_ = STATE_DYING;
	start_animation(disp,loc,choose_animation(disp,loc,"death",0,unit_animation::KILL,attack,secondary_attack),false);
}
void unit::set_healing(const game_display &disp,const gamemap::location& loc,int healing)
{
	state_ = STATE_HEALING;
	start_animation(disp,loc,choose_animation(disp,loc,"healing",healing),true);
}
void unit::set_victorious(const game_display &disp,const gamemap::location& loc,const attack_type* attack,const attack_type* secondary_attack)
{
	state_ = STATE_VICTORIOUS;
	start_animation(disp,loc,choose_animation(disp,loc,"victory",0,unit_animation::KILL,attack,secondary_attack),true);
}

void unit::set_walking(const game_display &disp,const gamemap::location& loc)
{
	if(state_ == STATE_WALKING && anim_ != NULL && anim_->matches(disp,loc,this,"movement") >unit_animation::MATCH_FAIL) {
		return; // finish current animation, don't start a new one
	}
	state_ = STATE_WALKING;
	start_animation(disp,loc,choose_animation(disp,loc,"movement"),false);
}


void unit::set_idling(const game_display &disp,const gamemap::location& loc)
{
	if (state_ == STATE_STANDING || state_ == STATE_IDLING || state_ == STATE_SELECTING) {
		state_ = STATE_IDLING;
		start_animation(disp,loc,choose_animation(disp,loc,"idling"),true);
	}
}

void unit::set_selecting(const game_display &disp,const gamemap::location& loc)
{
	if (state_ == STATE_STANDING || state_ == STATE_IDLING || state_ == STATE_SELECTING) {
		state_ = STATE_SELECTING;
		start_animation(disp,loc,choose_animation(disp,loc,"selected"),true);
	}
}

void unit::start_animation(const game_display &disp, const gamemap::location &loc,const unit_animation * animation,bool with_bars,bool cycles)
{
	if(!animation) {
		set_standing(disp,loc,with_bars);
		return ;
	}
	draw_bars_ =  with_bars;
	offset_=0;
	if(anim_) delete anim_;
	anim_ = new unit_animation(*animation);
	anim_->start_animation(anim_->get_begin_time(),loc, loc.get_direction(facing_), cycles, disp.turbo_speed());
	frame_begin_time_ = anim_->get_begin_time() -1;
	if (disp.idle_anim()) {
		next_idling_ = get_current_animation_tick()
			+ static_cast<int>((20000 + rand() % 20000) * disp.idle_anim_rate());
	} else {
		next_idling_ = INT_MAX;
	}
}

void unit::restart_animation(const game_display& disp,int start_time, bool cycles) {
	if(!anim_) return;
	anim_->start_animation(start_time,gamemap::location::null_location, gamemap::location::null_location, cycles, disp.turbo_speed());
	frame_begin_time_ = start_time -1;
}

void unit::set_facing(gamemap::location::DIRECTION dir) {
	if(dir != gamemap::location::NDIRECTIONS) {
		facing_ = dir;
	}
	// Else look at yourself (not available so continue to face the same direction)
}

void unit::redraw_unit(game_display& disp, const gamemap::location& loc)
{
	const gamemap & map = disp.get_map();
	if(!loc.valid() || hidden_ || disp.fogged(loc)
	|| (invisible(loc,disp.get_units(),disp.get_teams())
	&& disp.get_teams()[disp.viewing_team()].is_enemy(side())))
	{
		clear_haloes();
		if(anim_) {
			anim_->update_last_draw_time();
		}
		return;
	}
	if(refreshing_) {
		return;
	}
	refreshing_ = true;

	bool facing_west = facing_ == gamemap::location::NORTH_WEST || facing_ == gamemap::location::SOUTH_WEST;
	const gamemap::location dst = loc.get_direction(facing_);
	const int xsrc = disp.get_location_x(loc);
	const int ysrc = disp.get_location_y(loc);
	const int xdst = disp.get_location_x(dst);
	const int ydst = disp.get_location_y(dst);

	const t_translation::t_letter terrain = map.get_terrain(loc);
	const terrain_type& terrain_info = map.get_terrain_info(terrain);
	const double submerge = is_flying() ? 0.0 : terrain_info.unit_submerge();
	int height_adjust = static_cast<int>(terrain_info.unit_height_adjust() * disp.get_zoom_factor());
	if (is_flying() && height_adjust < 0) {
		height_adjust = 0;
	}
	const int ysrc_adjusted = ysrc - height_adjust;

	if(!anim_) {
		set_standing(disp,loc);
	}
	anim_->update_last_draw_time();

	if(frame_begin_time_ != anim_->get_current_frame_begin_time()) {
		frame_begin_time_ = anim_->get_current_frame_begin_time();
		if(!anim_->sound().empty()) {
			sound::play_sound(anim_->sound());
		}
	}

	double tmp_offset = anim_->offset(offset_);
	int d2 = disp.hex_size() / 2;
	const int x = static_cast<int>(tmp_offset * xdst + (1.0-tmp_offset) * xsrc) + d2;
	const int y = static_cast<int>(tmp_offset * ydst + (1.0-tmp_offset) * ysrc) + d2 - height_adjust;


	if(unit_halo_ == halo::NO_HALO && !image_halo().empty()) {
		unit_halo_ = halo::add(0, 0, image_halo(), gamemap::location(-1, -1));
	}
	if(unit_halo_ != halo::NO_HALO) {
		halo::set_location(unit_halo_, x, y);
	}

	if(unit_anim_halo_ != halo::NO_HALO) {
		halo::remove(unit_anim_halo_);
		unit_anim_halo_ = halo::NO_HALO;
	}
	if(!anim_->halo().empty()) {
		int dx = static_cast<int>(anim_->halo_x() * disp.get_zoom_factor());
		int dy = static_cast<int>(anim_->halo_y() * disp.get_zoom_factor());
		if (facing_west) dx = -dx;
		unit_anim_halo_ = halo::add(x + dx, y + dy,
			anim_->halo(), gamemap::location(-1, -1),
			facing_west ? halo::HREVERSE : halo::NORMAL);
	}


	image::locator image_loc;
	image_loc = anim_->image();
	if(image_loc.is_void()) {
		image_loc = absolute_image();
	}
#ifndef LOW_MEM
	std::string mod=image_mods();
	if(mod.size()){
		image_loc = image::locator(image_loc,mod);
	}
#endif

	surface image(image::get_image(image_loc,
				image::SCALED_TO_ZOOM,
#ifndef LOW_MEM
				true
#else
				state_ == STATE_STANDING?true:false
#endif
				));

	if(image == NULL) {
		image = still_image(true);
	}

	bool stoned = utils::string_bool(get_state("stoned"));

	fixed_t highlight_ratio = minimum<fixed_t>(alpha(),anim_->highlight_ratio());
	if(invisible(loc,disp.get_units(),disp.get_teams()) &&
			highlight_ratio > ftofxp(0.5)) {
		highlight_ratio = ftofxp(0.5);
	}
	if(loc == disp.selected_hex() && highlight_ratio == ftofxp(1.0)) {
		highlight_ratio = ftofxp(1.5);
	}

	Uint32 blend_with = anim_->blend_with();
	double blend_ratio = anim_->blend_ratio();
	//if(blend_ratio == 0) { blend_with = disp.rgb(0,0,0); }
	if (utils::string_bool(get_state("poisoned")) && blend_ratio == 0){
		blend_with = disp.rgb(0,255,0);
		blend_ratio = 0.25;
	}

	// We draw bars only if wanted and visible on the map view
	bool draw_bars = draw_bars_;
	if (draw_bars) {
		const int d = disp.hex_size();
		SDL_Rect unit_rect = {xsrc, ysrc_adjusted, d, d};
		draw_bars = rects_overlap(unit_rect, disp.map_outside_area());
	}

	surface ellipse_front(NULL);
	surface ellipse_back(NULL);
	int ellipse_floating = 0;
	if(draw_bars && preferences::show_side_colours()) {
		// The division by 2 seems to have no real meaning,
		// It just works fine with the current center of ellipse
		// and prevent a too large adjust if submerge = 1.0
		ellipse_floating = static_cast<int>(submerge * disp.hex_size() / 2);

		std::string ellipse=image_ellipse();
		if(ellipse.empty()){
			ellipse="misc/ellipse";
		}

		const char* const selected = disp.selected_hex() == loc ? "selected-" : "";

		// Load the ellipse parts recolored to match team color
		char buf[100];
		std::string tc=team::get_side_colour_index(side_);

		snprintf(buf,sizeof(buf),"%s-%stop.png~RC(ellipse_red>%s)",ellipse.c_str(),selected,tc.c_str());
		ellipse_back.assign(image::get_image(image::locator(buf), image::SCALED_TO_ZOOM));
		snprintf(buf,sizeof(buf),"%s-%sbottom.png~RC(ellipse_red>%s)",ellipse.c_str(),selected,tc.c_str());
		ellipse_front.assign(image::get_image(image::locator(buf), image::SCALED_TO_ZOOM));
	}


	if (ellipse_back != NULL) {
		disp.video().blit_surface(xsrc, ysrc_adjusted-ellipse_floating, ellipse_back);
	}

	if (image != NULL) {
		int tmp_x = x - image->w/2;
		int tmp_y = y - image->h/2;
		disp.render_unit_image(tmp_x, tmp_y, image, facing_west, stoned,
				highlight_ratio, blend_with, blend_ratio, submerge);
	}

	if (ellipse_front != NULL) {
		disp.video().blit_surface(xsrc, ysrc_adjusted-ellipse_floating, ellipse_front);
	}

	if(draw_bars) {
		const std::string* movement_file = NULL;
		const std::string* energy_file = &game_config::energy_image;
		const fixed_t bar_alpha = highlight_ratio < ftofxp(1.0) && blend_with == 0 ? highlight_ratio : (loc == disp.mouseover_hex() ? ftofxp(1.0): ftofxp(0.7));

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
				} else if(unit_can_move(loc,disp.get_units(),map,disp.get_teams())) {
					movement_file = &game_config::partmoved_ball_image;
				}
			}
		}

		surface orb(image::get_image(*movement_file,image::SCALED_TO_ZOOM));
		if (orb != NULL) {
			disp.video().blit_surface(xsrc, ysrc_adjusted, orb);
		}

		double unit_energy = 0.0;
		if(max_hitpoints() > 0) {
			unit_energy = double(hitpoints())/double(max_hitpoints());
		}
#ifdef USE_TINY_GUI
		const int bar_shift = static_cast<int>(-2.5*disp.get_zoom_factor());
#else
		const int bar_shift = static_cast<int>(-5*disp.get_zoom_factor());
#endif
		disp.draw_bar(*energy_file, xsrc+bar_shift, ysrc_adjusted, (max_hitpoints()*2)/3, unit_energy,hp_color(), bar_alpha);

		if(experience() > 0 && can_advance()) {
			const double filled = double(experience())/double(max_experience());
			const int level = maximum<int>(level_,1);

			SDL_Color colour=xp_color();
			disp.draw_bar(*energy_file, xsrc, ysrc_adjusted, max_experience()/(level*2), filled, colour, bar_alpha);
		}

		if (can_recruit()) {
			surface crown(image::get_image("misc/leader-crown.png",image::SCALED_TO_ZOOM));
			if(!crown.null()) {
				//if(bar_alpha != ftofxp(1.0)) {
				//	crown = adjust_surface_alpha(crown, bar_alpha);
				//}
				disp.video().blit_surface(xsrc,ysrc_adjusted,crown);
			}
		}

		for(std::vector<std::string>::const_iterator ov = overlays().begin(); ov != overlays().end(); ++ov) {
			const surface ov_img(image::get_image(*ov, image::SCALED_TO_ZOOM));
			if(ov_img != NULL) {
				disp.video().blit_surface(xsrc, ysrc_adjusted, ov_img);
			}
		}
	}

	anim_->redraw();
	refreshing_ = false;
}

void unit::clear_haloes()
{
	if(unit_halo_ != halo::NO_HALO) {
		halo::remove(unit_halo_);
		unit_halo_ = halo::NO_HALO;
	}
	if(unit_anim_halo_ != halo::NO_HALO) {
		halo::remove(unit_anim_halo_);
		unit_anim_halo_ = halo::NO_HALO;
	}
}

std::set<gamemap::location> unit::overlaps(const gamemap::location &loc) const
{
	std::set<gamemap::location> over;

	if (state_ == STATE_STANDING) {
		// Standing units only overlaps if height is adjusted
		int height_adjust = map_->get_terrain_info(map_->get_terrain(loc)).unit_height_adjust();
		if (is_flying() && height_adjust < 0) height_adjust = 0;

		if (height_adjust > 0) {
			over.insert(loc.get_direction(gamemap::location::NORTH));
			over.insert(loc.get_direction(gamemap::location::NORTH_WEST));
			over.insert(loc.get_direction(gamemap::location::NORTH_EAST));
		} else if (height_adjust < 0) {
			over.insert(loc.get_direction(gamemap::location::SOUTH));
			over.insert(loc.get_direction(gamemap::location::SOUTH_WEST));
			over.insert(loc.get_direction(gamemap::location::SOUTH_EAST));
		}
	} else {
		// Animated units overlaps adjacent hexes
		gamemap::location arr[6];
		get_adjacent_tiles(loc, arr);
		for (unsigned int i = 0; i < 6; i++) {
			over.insert(arr[i]);
		}
	}

	// Very early calls, anim not initialized yet
	double tmp_offset=offset_;
	if(anim_)tmp_offset= anim_->offset(offset_);

	// Invalidate adjacent neighbours if we don't stay in our hex
	if(tmp_offset != 0) {
		gamemap::location::DIRECTION dir = (tmp_offset > 0) ? facing_ : loc.get_opposite_dir(facing_);
		gamemap::location adj_loc = loc.get_direction(dir);
		over.insert(adj_loc);
		gamemap::location arr[6];
		get_adjacent_tiles(adj_loc, arr);
		for (unsigned int i = 0; i < 6; i++) {
			over.insert(arr[i]);
		}
	}

	return over;
}

int unit::upkeep() const
{
	// Leaders do not incur upkeep.
	if(can_recruit()) {
		return 0;
	}
	if(cfg_["upkeep"] == "full") {
		return level();
	}
	if(cfg_["upkeep"] == "loyal") {
		return 0;
	}
	return lexical_cast_default<int>(cfg_["upkeep"]);
}

int unit::movement_cost_internal(const t_translation::t_letter terrain, const int recurse_count) const
{
	const int impassable = 10000000;

	const std::map<t_translation::t_letter,int>::const_iterator i =
	movement_costs_.find(terrain);

	if(i != movement_costs_.end()) {
		return i->second;
	}

	wassert(map_ != NULL);
	// If this is an alias, then select the best of all underlying terrains
	const t_translation::t_list& underlying = map_->underlying_mvt_terrain(terrain);

	wassert(!underlying.empty());
	if(underlying.size() != 1 || underlying.front() != terrain) { // We fail here, but first test underlying_mvt_terrain
		bool revert = (underlying.front() == t_translation::MINUS ? true : false);
		if(recurse_count >= 100) {
			return impassable;
		}

		int ret_value = revert ? 0: impassable;
		for(t_translation::t_list::const_iterator i = underlying.begin();
		i != underlying.end(); ++i) {
			if(*i == t_translation::PLUS) {
				revert = false;
				continue;
			} else if(*i == t_translation::MINUS) {
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

		movement_costs_.insert(std::pair<t_translation::t_letter, int>(terrain, ret_value));
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
		const std::string& id = map_->get_terrain_info(underlying.front()).id();
		const std::string& val = (*movement_costs)[id];
		if(val != "") {
			res = atoi(val.c_str());
		}
	}

	if(res <= 0) {
		res = impassable;
	}

	movement_costs_.insert(std::pair<t_translation::t_letter, int>(terrain,res));
	return res;
}

int unit::movement_cost(const t_translation::t_letter terrain) const
{
	const int res = movement_cost_internal(terrain, 0);
	if(utils::string_bool(get_state("slowed"))) {
		return res*2;
	}
	return res;
}

int unit::defense_modifier(t_translation::t_letter terrain, int recurse_count) const
{
//	const std::map<terrain_type::TERRAIN,int>::const_iterator i = defense_mods_.find(terrain);
//	if(i != defense_mods_.end()) {
//		return i->second;
//	}

	wassert(map_ != NULL);
	// If this is an alias, then select the best of all underlying terrains
	const t_translation::t_list& underlying = map_->underlying_def_terrain(terrain);
	wassert(underlying.size() > 0);
	if(underlying.size() != 1 || underlying.front() != terrain) {
		bool revert = (underlying.front() == t_translation::MINUS ? true : false);
		if(recurse_count >= 90) {
			LOG_STREAM(err, config) << "infinite defense_modifier recursion: " << t_translation::write_letter(terrain) << " depth " << recurse_count << "\n";
		}
		if(recurse_count >= 100) {
			return 100;
		}

		int ret_value = revert?0:100;
		t_translation::t_list::const_iterator i = underlying.begin();
		for(; i != underlying.end(); ++i) {
			if(*i == t_translation::PLUS) {
				revert = false;
				continue;
			} else if(*i == t_translation::MINUS) {
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

//		defense_mods_.insert(std::pair<terrain_type::TERRAIN,int>(terrain,ret_value));

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

		const std::string& id = map_->get_terrain_info(underlying.front()).id();
		const std::string& val = (*defense)[id];
		if(val != "") {
			res = atoi(val.c_str());
		}
	}
	if(res <= 0) {
		res = 50;
	}

//	defense_mods_.insert(std::pair<terrain_type::TERRAIN,int>(terrain,res));
	return res;
}

bool unit::resistance_filter_matches(const config& cfg,bool attacker,const std::string& damage_name) const
{
	if(!(cfg["active_on"]=="" || (attacker && cfg["active_on"]=="offense") || (!attacker && cfg["active_on"]=="defense"))) {
		return false;
	}
	const std::string& apply_to = cfg["apply_to"];
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


int unit::resistance_against(const std::string& damage_name,bool attacker,const gamemap::location& loc) const
{
	int res = 100;

	const config* const resistance = cfg_.child("resistance");
	if(resistance != NULL) {
		const std::string& val = (*resistance)[damage_name];
		if(val != "") {
			res = 100 - lexical_cast_default<int>(val);
		}
	}

	unit_ability_list resistance_abilities = get_abilities("resistance",loc);
	for(std::vector<std::pair<config*,gamemap::location> >::iterator i = resistance_abilities.cfgs.begin(); i != resistance_abilities.cfgs.end();) {
		if(!resistance_filter_matches(*i->first,attacker,damage_name)) {
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

string_map unit::get_base_resistances() const
{
	const config* const resistance = cfg_.child("resistance");
	if(resistance != NULL) {
		return resistance->values;
	}
	return string_map();
}

#if 0
std::map<terrain_type::TERRAIN,int> unit::movement_type() const
{
	return movement_costs_;
}
#endif

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
	cfg_.clear_children("abilities");
	cfg_.add_child("movement_costs",movement_b_);
	cfg_.add_child("defense",defense_b_);
	cfg_.add_child("resistance",resistance_b_);
	cfg_.add_child("abilities",abilities_b_);
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
	if(cfg_.child("abilities")) {
		abilities_b_ = *cfg_.child("abilities");
	} else {
		abilities_b_ = config();
	}
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

//! Helper function for add_modifications
static void mod_mdr_merge(config& dst, const config& mod, bool delta)
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

void unit::add_modification(const std::string& type, const config& mod, bool no_add)
{
	if(no_add == false) {
		modifications_.add_child(type,mod);
	}

	std::vector<t_string> effects_description;
	for(config::const_child_itors i = mod.child_range("effect");
		i.first != i.second; ++i.first) {

		// See if the effect only applies to certain unit types
		const std::string& type_filter = (**i.first)["unit_type"];
		if(type_filter.empty() == false) {
			const std::vector<std::string>& types = utils::split(type_filter);
			if(std::find(types.begin(),types.end(),id()) == types.end()) {
				continue;
			}
		}
		// See if the effect only applies to certain genders
		const std::string& gender_filter = (**i.first)["unit_gender"];
		if(gender_filter.empty() == false) {
			const std::string& gender = gender_string(gender_);
			const std::vector<std::string>& genders = utils::split(gender_filter);
			if(std::find(genders.begin(),genders.end(),gender) == genders.end()) {
				continue;
			}
		}

		const std::string& apply_to = (**i.first)["apply_to"];
		const std::string& apply_times = (**i.first)["times"];
		int times = 1;
		t_string description;

		if (apply_times == "per level")
			times = level_;
		if (times) {
			while (times > 0) {
				times --;

				// Apply variations -- only apply if we are adding this
				// for the first time.
				if(apply_to == "variation" && no_add == false) {
					variation_ = (**i.first)["name"];
					wassert(gamedata_ != NULL);
					const game_data::unit_type_map::const_iterator var = gamedata_->unit_types.find(id());
										if(var == gamedata_->unit_types.end()) {
						std::string error_message = _("Unknown unit type '$type|'");
						utils::string_map symbols;
						symbols["type"] = id();
						error_message = utils::interpolate_variables_into_string(error_message, &symbols);
						LOG_STREAM(err, engine) << "unit of type " << id() << " not found!\n";
						throw game::game_error(error_message);
										}
					advance_to(&var->second.get_variation(variation_));
				} else if(apply_to == "profile") {
					const std::string& portrait = (**i.first)["portrait"];
					const std::string& description = (**i.first)["description"];
					if(!portrait.empty()) cfg_["profile"] = portrait;
					if(!description.empty()) cfg_["unit_description"] = description;
					//help::unit_topic_generator(*this, (**i.first)["help_topic"]);
				} else if(apply_to == "new_attack") {
					attacks_.push_back(attack_type(**i.first));
				} else if(apply_to == "remove_attacks") {
					int num_attacks= attacks_.size();
					for(std::vector<attack_type>::iterator a = attacks_.begin(); a != attacks_.end(); ++a) {
						if (a->matches_filter(**i.first,false)) {
							if (num_attacks > 1) {
								attacks_.erase(a--);
								num_attacks--;
							} else {
								// Don't remove the last attack
								LOG_STREAM(err, config) << "[effect] tried to remove the last attack : ignored.\n";
							}
						}
					}
				} else if(apply_to == "attack") {

					bool first_attack = true;

					std::string attack_names;
					std::string desc;
					for(std::vector<attack_type>::iterator a = attacks_.begin();
						a != attacks_.end(); ++a) {
						bool affected = a->apply_modification(**i.first,&desc);
						if(affected && desc != "") {
							if(first_attack) {
								first_attack = false;
							} else {
								if (!times)
									attack_names += t_string(N_(" and "), "wesnoth");
							}

							if (!times)
								attack_names += t_string(a->name(), "wesnoth");
						}
					}
					if (attack_names.empty() == false) {
						utils::string_map symbols;
						symbols["attack_list"] = attack_names;
						symbols["effect_description"] = desc;
						description += vgettext("$attack_list|: $effect_description", symbols);
					}
				} else if(apply_to == "hitpoints") {
					LOG_UT << "applying hitpoint mod..." << hit_points_ << "/" << max_hit_points_ << "\n";
					const std::string& increase_hp = (**i.first)["increase"];
					const std::string& heal_full = (**i.first)["heal_full"];
					const std::string& increase_total = (**i.first)["increase_total"];
					const std::string& set_hp = (**i.first)["set"];
					const std::string& set_total = (**i.first)["set_total"];

					// If the hitpoints are allowed to end up greater than max hitpoints
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
						if (!times)
							description += (increase_total[0] != '-' ? "+" : "") +
								increase_total + " " +
								t_string(N_("HP"), "wesnoth");

						// A percentage on the end means increase by that many percent
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
						if (!times)
							description += (increase[0] != '-' ? "+" : "") +
								increase + " " +
								t_string(N_("moves"), "wesnoth");

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
						if (!times)
							description += (increase[0] != '-' ? "+" : "") +
								increase + " " +
								t_string(N_("XP to advance"), "wesnoth");

						max_experience_ = utils::apply_modifier(max_experience_, increase, 1);
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
				} else if (apply_to == "zoc") {
					const std::string& zoc_value = (**i.first)["value"];
					if(!zoc_value.empty()) {
						emit_zoc_ = lexical_cast_default<int>(zoc_value);
					}
				} else if (apply_to == "new_ability") {
					config *ab_effect;
					config *ab = cfg_.child("abilities");
					if(!ab) {
						ab = &cfg_.add_child("abilities");
					}
					ab_effect = (**i.first).child("abilities");
					if (ab_effect) {
						config to_append;
						config::all_children_iterator j, j_end = ab_effect->ordered_end();
						for(j = ab_effect->ordered_begin(); j != j_end; ++j) {
							if(!has_ability_by_id(j.get_child()["id"])) {
								to_append.add_child(j.get_key(), j.get_child());
							}
						}
						ab->append(to_append);
					}
				} else if (apply_to == "remove_ability") {
					config *ab_effect = (**i.first).child("abilities");
					if (ab_effect) {
						config::all_children_iterator j, j_end = ab_effect->ordered_end();
						for(j = ab_effect->ordered_begin(); j != j_end; ++j) {
							remove_ability_by_id(j.get_child()["id"]);
						}
					}
				} else if (apply_to == "image_mod") {
					LOG_UT << "applying image_mod \n";
					std::string mod = (**i.first)["replace"];
					if (!mod.empty()){
						image_mods_ = mod;
					}
					LOG_UT << "applying image_mod \n";
					mod = (**i.first)["add"];
					if (!mod.empty()){
						image_mods_ += mod;
					}

					game_config::add_color_info(**i.first);
					LOG_UT << "applying image_mod \n";
				} else if (apply_to == "new_animation") {
					unit_animation::initialize_anims(animations_,**i.first);
				}
			} // end while
		} else { // for times = per level & level = 0 we still need to rebuild the descriptions
			if(apply_to == "attack") {

				bool first_attack = true;

				for(std::vector<attack_type>::iterator a = attacks_.begin();
					a != attacks_.end(); ++a) {
					std::string desc;
					bool affected = a->describe_modification(**i.first,&desc);
					if(affected && desc != "") {
						if(first_attack) {
							first_attack = false;
						} else {
							description += t_string(N_(" and "), "wesnoth");
						}

						description += t_string(a->name(), "wesnoth") + ": " + desc;
					}
				}
			} else if(apply_to == "hitpoints") {
				const std::string& increase_total = (**i.first)["increase_total"];

				if(increase_total.empty() == false) {
					description += (increase_total[0] != '-' ? "+" : "") +
						increase_total + " " +
						t_string(N_("HP"), "wesnoth");
				}
			} else if(apply_to == "movement") {
				const std::string& increase = (**i.first)["increase"];

				if(increase.empty() == false) {
					description += (increase[0] != '-' ? "+" : "") +
						increase + t_string(N_(" move"), "wesnoth");
				}
			} else if(apply_to == "max_experience") {
				const std::string& increase = (**i.first)["increase"];

				if(increase.empty() == false) {
					description += (increase[0] != '-' ? "+" : "") +
						increase + " " +
						t_string(N_("XP to advance"), "wesnoth");
				}
			}
		}

		if (apply_times == "per level" && !times) {
			utils::string_map symbols;
			symbols["effect_description"] = description;
			description = vgettext("$effect_description per level", symbols);
		}
		if(!description.empty())
			effects_description.push_back(description);

	}

	t_string& description = modification_descriptions_[type];
	t_string trait_description;

	// Punctuation should be translatable: not all languages use latin punctuation.
	// (However, there maybe is a better way to do it)
	if (!mod["description"].empty()) {
		trait_description += mod["description"] + " ";
	}
	if(effects_description.empty() == false) {
		//trait_description += t_string(N_("("), "wesnoth");
		for(std::vector<t_string>::const_iterator i = effects_description.begin();
				i != effects_description.end(); ++i) {
			trait_description += *i;
			if(i+1 != effects_description.end())
				trait_description += t_string(N_(" and "), "wesnoth");
		}
		//trait_description += t_string(N_(")"), "wesnoth");
	}
	
	if (!mod["name"].empty()) {
		utils::string_map symbols;
		symbols["trait_name"] = mod["name"];
		symbols["trait_description"] = trait_description;
		description += vgettext("$trait_name|: $trait_description ", symbols);
	} else if (!trait_description.empty()) {
		description += trait_description;
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



const unit_animation* unit::choose_animation(const game_display& disp, const gamemap::location& loc,const std::string& event,const int value,const unit_animation::hit_type hit,const attack_type* attack,const attack_type* second_attack, int swing_num) const
{
	// Select one of the matching animations at random
	std::vector<const unit_animation*> options;
	int max_val = unit_animation::MATCH_FAIL;
	for(std::vector<unit_animation>::const_iterator i = animations_.begin(); i != animations_.end(); ++i) {
		int matching = i->matches(disp,loc,this,event,value,hit,attack,second_attack,swing_num);
		if(matching > unit_animation::MATCH_FAIL && matching == max_val) {
			options.push_back(&*i);
		} else if(matching > max_val) {
			max_val = matching;
			options.erase(options.begin(),options.end());
			options.push_back(&*i);
		}
	}

	if(max_val == unit_animation::MATCH_FAIL) {
		return NULL;
	}
	return options[rand()%options.size()];
}


void unit::apply_modifications()
{
	log_scope("apply mods");
	reset_modifications();
	modification_descriptions_.clear();

	traits_description_ = "";

	std::vector< t_string > traits;
	is_fearless_ = false;
	is_healthy_ = false;
	config::child_list const &mods = modifications_.get_children("trait");
	for(config::child_list::const_iterator j = mods.begin(), j_end = mods.end(); j != j_end; ++j) {
		is_fearless_ = is_fearless_ || (**j)["id"] == "fearless";
		is_healthy_ = is_healthy_ || (**j)["id"] == "healthy";
		const std::string gender_string = gender_ == unit_race::FEMALE ? "female_name" : "male_name";
		t_string const &gender_specific_name = (**j)[gender_string];
		if (!gender_specific_name.empty()) {
			traits.push_back(gender_specific_name);
			(**j)["name"] = gender_specific_name;
		} else {
			t_string const &name = (**j)["name"];
			if (!name.empty()) {
				traits.push_back(name);
			}
		}
	}

	for(size_t i = 0; i != NumModificationTypes; ++i) {
		const std::string& mod = ModificationTypes[i];
		const config::child_list& mods = modifications_.get_children(mod);
		for(config::child_list::const_iterator j = mods.begin(); j != mods.end(); ++j) {
			log_scope("add mod");
			add_modification(ModificationTypes[i],**j,true);
		}
	}

	std::vector< t_string >::iterator k = traits.begin(), k_end = traits.end();
	if (k != k_end) {
		// We want to make sure the traits always have a consistent ordering.
			// Try out not sorting, since quick,resilient can give different HP
			// to resilient,quick so rather preserve order
		// std::sort(k, k_end);
		for(;;) {
			traits_description_ += *(k++);
			if (k == k_end) break;
			traits_description_ += ", ";
		}
	}
}

bool unit::invisible(const gamemap::location& loc,
		const unit_map& units,const std::vector<team>& teams, bool see_all) const
{
	// Fetch from cache
	//! @todo FIXME: We use the cache only when using the default see_all=true
	// Maybe add a second cache if the see_all=false become more frequent.
	if(see_all) {
		std::map<gamemap::location, bool>::const_iterator itor = invisibility_cache_.find(loc);
		if(itor != invisibility_cache_.end()) {
			return itor->second;
		}
	}

	// Test hidden status
	static const std::string hides("hides");
	bool is_inv = (utils::string_bool(get_state(hides)) && get_ability_bool(hides,loc));
	if(is_inv){
		for(unit_map::const_iterator u = units.begin(); u != units.end(); ++u) {
			if(teams[side_-1].is_enemy(u->second.side()) && tiles_adjacent(loc,u->first)) {
				// Enemy spotted in adjacent tiles, check if we can see him.
				// Watch out to call invisible with see_all=true to avoid infinite recursive calls!
				if(see_all) {
					is_inv = false;
					break;
				} else if(!teams[side_-1].fogged(u->first.x,u->first.y)
				&& !u->second.invisible(u->first, units,teams,true)) {
					is_inv = false;
					break;
				}
			}
		}
	}

	if(see_all) {
		// Add to caches
		if(invisibility_cache_.empty()) {
			units_with_cache.push_back(this);
		}
		invisibility_cache_[loc] = is_inv;
	}

	return is_inv;
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

unit_map::iterator find_visible_unit(unit_map& units,
		const gamemap::location loc,
		const gamemap& map,
		  const std::vector<team>& teams, const team& current_team,
		bool see_all)
{
	unit_map::iterator u = units.find(loc);
	if(map.on_board(loc) && !see_all){
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
		const std::vector<team>& teams, const team& current_team,
		bool see_all)
{
	unit_map::const_iterator u = units.find(loc);
	if(map.on_board(loc) && !see_all){
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
	res.teamname = tm.user_team_name();
	return res;
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

std::string unit::image_mods() const{
	std::stringstream modifier;
	if(flag_rgb_.size()){
		modifier << "~RC("<< flag_rgb_ << ">" << team::get_side_colour_index(side()) << ")";
	}
	if(image_mods_.size()){
		modifier << "~" << image_mods_;
	}
	return modifier.str();
}



void unit::set_hidden(bool state) {
	hidden_ = state;
	if(!state) return;
	// We need to get rid of haloes immediately to avoid display glitches
	clear_haloes();
}
std::string get_checksum(const unit& u, const bool discard_description)
{
	config unit_config;
	u.write(unit_config);
	unit_config["controller"] = "";
	// Since the ai messes up the 'moves' attribute, ignore that for the checksum
	unit_config["moves"] = "";

	if(discard_description) {
		unit_config["description"] = "";
		unit_config["user_description"] = "";
	}

	return unit_config.hash();
}

