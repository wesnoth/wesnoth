/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file
 *  Handle unit-type specific attributes, animations, advancement.
 */

#include "units/types.hpp"

#include "game_config.hpp"
#include "game_errors.hpp" //thrown sometimes
//#include "gettext.hpp"
#include "log.hpp"
#include "utils/make_enum.hpp"
#include "units/unit.hpp"
#include "units/abilities.hpp"
#include "units/animation.hpp"
#include "utils/iterable_pair.hpp"

#include "gui/auxiliary/typed_formula.hpp"
#include "gui/dialogs/loading_screen.hpp"

#include <boost/regex.hpp>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
#define WRN_CF LOG_STREAM(warn, log_config)
#define LOG_CONFIG LOG_STREAM(info, log_config)
#define DBG_CF LOG_STREAM(debug, log_config)

static lg::log_domain log_unit("unit");
#define DBG_UT LOG_STREAM(debug, log_unit)
#define ERR_UT LOG_STREAM(err, log_unit)


/* ** unit_type ** */


unit_type::unit_type(const unit_type& o) :
	cfg_(o.cfg_),
	unit_cfg_(),  // Not copied; will be re-created if needed.
	built_unit_cfg_(false),
	id_(o.id_),
	debug_id_(o.debug_id_),
	base_id_(o.base_id_),
	type_name_(o.type_name_),
	description_(o.description_),
	hitpoints_(o.hitpoints_),
	hp_bar_scaling_(o.hp_bar_scaling_),
	xp_bar_scaling_(o.xp_bar_scaling_),
	level_(o.level_),
	recall_cost_(o.recall_cost_),
	movement_(o.movement_),
	vision_(o.vision_),
	jamming_(o.jamming_),
	max_attacks_(o.max_attacks_),
	cost_(o.cost_),
	usage_(o.usage_),
	undead_variation_(o.undead_variation_),
	image_(o.image_),
	icon_(o.icon_),
	small_profile_(o.small_profile_),
	profile_(o.profile_),
	flag_rgb_(o.flag_rgb_),
	num_traits_(o.num_traits_),
	variations_(o.variations_),
	default_variation_(o.default_variation_),
	variation_name_(o.variation_name_),
	race_(o.race_),
	alpha_(o.alpha_),
	abilities_(o.abilities_),
	adv_abilities_(o.adv_abilities_),
	ability_tooltips_(o.ability_tooltips_),
	adv_ability_tooltips_(o.adv_ability_tooltips_),
	zoc_(o.zoc_),
	hide_help_(o.hide_help_),
	do_not_list_(o.do_not_list_),
	advances_to_(o.advances_to_),
	experience_needed_(o.experience_needed_),
	in_advancefrom_(o.in_advancefrom_),
	alignment_(o.alignment_),
	movement_type_(o.movement_type_),
	possible_traits_(o.possible_traits_),
	genders_(o.genders_),
	animations_(o.animations_),
    build_status_(o.build_status_)
{
	gender_types_[0] = o.gender_types_[0] != nullptr ? new unit_type(*o.gender_types_[0]) : nullptr;
	gender_types_[1] = o.gender_types_[1] != nullptr ? new unit_type(*o.gender_types_[1]) : nullptr;
}


unit_type::unit_type(const config &cfg, const std::string & parent_id) :
	cfg_(cfg),
	unit_cfg_(),
	built_unit_cfg_(false),
	id_(cfg_.has_attribute("id") ? cfg_["id"].str() : parent_id),
	debug_id_(),
	base_id_(!parent_id.empty() ? parent_id : id_),
	type_name_(cfg_["name"].t_str()),
	description_(),
	hitpoints_(0),
	hp_bar_scaling_(0.0),
	xp_bar_scaling_(0.0),
	level_(0),
	recall_cost_(),
	movement_(0),
	vision_(-1),
	jamming_(0),
	max_attacks_(0),
	cost_(0),
	usage_(),
	undead_variation_(),
	image_(cfg_["image"].str()),
	icon_(),
	small_profile_(),
	profile_(),
	flag_rgb_(cfg_["flag_rgb"].str()),
	num_traits_(0),
	gender_types_(),
	variations_(),
	default_variation_(cfg_["variation"]),
	variation_name_(cfg_["variation_name"].t_str()),
	race_(&unit_race::null_race),
	alpha_(ftofxp(1.0)),
	abilities_(),
	adv_abilities_(),
	ability_tooltips_(),
	adv_ability_tooltips_(),
	zoc_(false),
	hide_help_(false),
	do_not_list_(cfg_["do_not_list"].to_bool(false)),
	advances_to_(),
	experience_needed_(0),
	in_advancefrom_(false),
	alignment_(unit_type::ALIGNMENT::NEUTRAL),
	movement_type_(),
	possible_traits_(),
	genders_(),
	animations_(),
	build_status_(NOT_BUILT)
{
	check_id(id_);
	check_id(base_id_);
	gender_types_[0] = nullptr;
	gender_types_[1] = nullptr;
}

unit_type::~unit_type()
{
	delete gender_types_[0];
	delete gender_types_[1];
}

/**
 * Load data into an empty unit_type (build to FULL).
 */
void unit_type::build_full(const movement_type_map &mv_types,
	const race_map &races, const config::const_child_itors &traits)
{
	// Don't build twice.
	if ( FULL <= build_status_ )
		return;
	// Make sure we are built to the preceding build level.
	build_help_index(mv_types, races, traits);

	for (int i = 0; i < 2; ++i) {
		if (gender_types_[i])
			gender_types_[i]->build_full(mv_types, races, traits);
	}

	if ( race_ != &unit_race::null_race )
	{
		if (undead_variation_.empty()) {
			undead_variation_ = race_->undead_variation();
		}
	}

	zoc_ = cfg_["zoc"].to_bool(level_ > 0);

	const config::attribute_value & alpha_blend = cfg_["alpha"];
	if(alpha_blend.empty() == false) {
		alpha_ = ftofxp(alpha_blend.to_double());
	}

	game_config::add_color_info(cfg_);

	hp_bar_scaling_ = cfg_["hp_bar_scaling"].to_double(game_config::hp_bar_scaling);
	xp_bar_scaling_ = cfg_["xp_bar_scaling"].to_double(game_config::xp_bar_scaling);

	// Propagate the build to the variations.
	for (variations_map::value_type & variation : variations_) {
		variation.second.build_full(mv_types, races, traits);
	}

	// Deprecation messages, only seen when unit is parsed for the first time.

	build_status_ = FULL;
}

/**
 * Partially load data into an empty unit_type (build to HELP_INDEXED).
 */
void unit_type::build_help_index(const movement_type_map &mv_types,
	const race_map &races, const config::const_child_itors &traits)
{
	// Don't build twice.
	if ( HELP_INDEXED <= build_status_ )
		return;
	// Make sure we are built to the preceding build level.
	build_created(mv_types, races, traits);

	type_name_ = cfg_["name"];
	description_ = cfg_["description"];
	hitpoints_ = cfg_["hitpoints"].to_int(1);
	level_ = cfg_["level"];
	recall_cost_ = cfg_["recall_cost"].to_int(-1);
	movement_ = cfg_["movement"].to_int(1);
	vision_ = cfg_["vision"].to_int(-1);
	jamming_ = cfg_["jamming"].to_int(0);
	max_attacks_ = cfg_["attacks"].to_int(1);
	cost_ = cfg_["cost"].to_int(1);
	usage_ = cfg_["usage"].str();
	undead_variation_ = cfg_["undead_variation"].str();
	image_ = cfg_["image"].str();
	icon_ = cfg_["image_icon"].str();
	small_profile_ = cfg_["small_profile"].str();
	profile_ = cfg_["profile"].str();
	adjust_profile(profile_);

	alignment_ = unit_type::ALIGNMENT::NEUTRAL;
	alignment_.parse(cfg_["alignment"].str());

	for (int i = 0; i < 2; ++i) {
		if (gender_types_[i])
			gender_types_[i]->build_help_index(mv_types, races, traits);
	}

	const race_map::const_iterator race_it = races.find(cfg_["race"]);
	if(race_it != races.end()) {
		race_ = &race_it->second;
	} else {
		race_ = &unit_race::null_race;
	}

	// if num_traits is not defined, we use the num_traits from race
	num_traits_ = cfg_["num_traits"].to_int(race_->num_traits());

	const std::vector<std::string> genders = utils::split(cfg_["gender"]);
	for(std::vector<std::string>::const_iterator g = genders.begin(); g != genders.end(); ++g) {
		genders_.push_back(string_gender(*g));
	}
	// For simplicity in other parts of the code, we must have at least one gender.
	if(genders_.empty()) {
		genders_.push_back(unit_race::MALE);
	}

	if (const config &abil_cfg = cfg_.child("abilities"))
	{
		for (const config::any_child &ab : abil_cfg.all_children_range()) {
			const config::attribute_value &name = ab.cfg["name"];
			if (!name.empty()) {
				abilities_.push_back(name.t_str());
				ability_tooltips_.push_back( ab.cfg["description"].t_str() );
			}
		}
	}

	for (const config &adv : cfg_.child_range("advancement"))
	{
		for (const config &effect : adv.child_range("effect"))
		{
			const config &abil_cfg = effect.child("abilities");
			if (!abil_cfg || effect["apply_to"] != "new_ability") {
				continue;
			}
			for (const config::any_child &ab : abil_cfg.all_children_range()) {
				const config::attribute_value &name = ab.cfg["name"];
				if (!name.empty()) {
					adv_abilities_.push_back(name.t_str());
					adv_ability_tooltips_.push_back( ab.cfg["description"].t_str() );
				}
			}
		}
	}

	// Set the movement type.
	const std::string move_type = cfg_["movement_type"];
	const movement_type_map::const_iterator find_it = mv_types.find(move_type);
	if ( find_it != mv_types.end() ) {
		DBG_UT << "inheriting from movement_type '" << move_type << "'\n";
		movement_type_ = find_it->second;
	}
	else if ( !move_type.empty() ) {
		DBG_UT << "movement_type '" << move_type << "' not found\n";
	}
	// Override parts of the movement type with what is in our config.
	movement_type_.merge(cfg_);

	for (const config &t : traits)
	{
		possible_traits_.add_child("trait", t);
	}
	if ( race_ != &unit_race::null_race )
	{
		if (!race_->uses_global_traits()) {
			possible_traits_.clear();
		}
		if ( cfg_["ignore_race_traits"].to_bool() ) {
			possible_traits_.clear();
		} else {
			for (const config &t : race_->additional_traits())
			{
				if (alignment_ != unit_type::ALIGNMENT::NEUTRAL || t["id"] != "fearless")
					possible_traits_.add_child("trait", t);
			}
		}
		if (undead_variation_.empty()) {
			undead_variation_ = race_->undead_variation();
		}
	}
	// Insert any traits that are just for this unit type
	for (const config &trait : cfg_.child_range("trait"))
	{
		possible_traits_.add_child("trait", trait);
	}

	for (const config &var_cfg : cfg_.child_range("variation"))
	{
		const std::string& var_id = var_cfg["variation_id"].empty() ?
				var_cfg["variation_name"] : var_cfg["variation_id"];

		variations_map::iterator ut;
		bool success;
		std::tie(ut, success) = variations_.emplace(var_id, unit_type(var_cfg, id_));
		if(success) {
			ut->second.debug_id_ = debug_id_ + " [" + var_id + "]";
			ut->second.base_id_ = base_id_;  // In case this is not id_.
			ut->second.build_help_index(mv_types, races, traits);
		} else {
			ERR_CF << "Skipping duplicate unit variation ID: " << var_id << "\n";
		}
	}

	hide_help_= cfg_["hide_help"].to_bool();

	build_status_ = HELP_INDEXED;
}

/**
 * Load the most needed data into an empty unit_type (build to CREATE).
 * This creates the gender-specific types (if needed) and also defines how much
 * experience is needed to advance as well as what this advances to.
 */
void unit_type::build_created(const movement_type_map &mv_types,
	const race_map &races, const config::const_child_itors &traits)
{
	// Don't build twice.
	if ( CREATED <= build_status_ )
		return;
	// There is no preceding build level (other than being constructed).

	// These should still be nullptr from the constructor.
	assert(gender_types_[0] == nullptr);
	assert(gender_types_[1] == nullptr);

	if ( const config &male_cfg = cfg_.child("male") ) {
		gender_types_[0] = new unit_type(male_cfg, id_);
		gender_types_[0]->debug_id_ = debug_id_ + " (male)";
	}

	if ( const config &female_cfg = cfg_.child("female") ) {
		gender_types_[1] = new unit_type(female_cfg, id_);
		gender_types_[1]->debug_id_ = debug_id_ + " (female)";
	}

	for (int i = 0; i < 2; ++i) {
		if (gender_types_[i])
			gender_types_[i]->build_created(mv_types, races, traits);
	}

    const std::string& advances_to_val = cfg_["advances_to"];
    if(advances_to_val != "null" && advances_to_val != "")
        advances_to_ = utils::split(advances_to_val);
    DBG_UT << "unit_type '" << log_id() << "' advances to : " << advances_to_val << "\n";

	experience_needed_ = cfg_["experience"].to_int(500);

	build_status_ = CREATED;
}

/**
 * Performs a build of this to the indicated stage.
 */
void unit_type::build(BUILD_STATUS status, const movement_type_map &movement_types,
                      const race_map &races, const config::const_child_itors &traits)
{
	DBG_UT << "Building unit type " << log_id() << ", level " << status << '\n';

	switch (status) {
	case NOT_BUILT:
		// Already done in the constructor.
		return;

	case CREATED:
		// Build the basic data.
		build_created(movement_types, races, traits);
		return;

	case VARIATIONS: // Implemented as part of HELP_INDEXED
	case HELP_INDEXED:
		// Build the data needed to feed the help index.
		build_help_index(movement_types, races, traits);
		return;

	case FULL:
		build_full(movement_types, races, traits);
		return;

	default:
		ERR_UT << "Build of unit_type to unrecognized status (" << status << ") requested." << std::endl;
		// Build as much as possible.
		build_full(movement_types, races, traits);
		return;
	}
}


const unit_type& unit_type::get_gender_unit_type(std::string gender) const
{
	if (gender == unit_race::s_female) return get_gender_unit_type(unit_race::FEMALE);
	else if (gender == unit_race::s_male) return get_gender_unit_type(unit_race::MALE);
	else return *this;
}

const unit_type& unit_type::get_gender_unit_type(unit_race::GENDER gender) const
{
	const size_t i = gender;
	if(i < sizeof(gender_types_)/sizeof(*gender_types_)
	&& gender_types_[i] != nullptr) {
		return *gender_types_[i];
	}

	return *this;
}

const unit_type& unit_type::get_variation(const std::string& id) const
{
	const variations_map::const_iterator i = variations_.find(id);
	if(i != variations_.end()) {
		return i->second;
	} else {
		return *this;
	}
}

t_string unit_type::unit_description() const
{
	if(description_.empty()) {
		return (_("No description available."));
	} else {
		return description_;
	}
}

const std::vector<unit_animation>& unit_type::animations() const {
	if (animations_.empty()) {
		unit_animation::fill_initial_animations(animations_,cfg_);
	}

	return animations_;
}

const_attack_itors unit_type::attacks() const
{
	if(!attacks_cache_.empty()) {
		return make_attack_itors(attacks_cache_);
	}

	for (const config &att : cfg_.child_range("attack")) {
		attacks_cache_.emplace_back(new attack_type(att));
	}

	return make_attack_itors(attacks_cache_);
}


namespace {
	int experience_modifier = 100;
}

unit_experience_accelerator::unit_experience_accelerator(int modifier) : old_value_(experience_modifier)
{
	experience_modifier = modifier;
}

unit_experience_accelerator::~unit_experience_accelerator()
{
	experience_modifier = old_value_;
}

int unit_experience_accelerator::get_acceleration()
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
/*
const char* unit_type::alignment_description(unit_type::ALIGNMENT align, unit_race::GENDER gender)
{
	static const char* aligns[] = { N_("lawful"), N_("neutral"), N_("chaotic"), N_("liminal") };
	static const char* aligns_female[] = { N_("female^lawful"), N_("female^neutral"), N_("female^chaotic"), N_("female^liminal") };
	const char** tlist = (gender == unit_race::MALE ? aligns : aligns_female);

	return (translation::sgettext(tlist[align]));
}*/

bool unit_type::has_ability_by_id(const std::string& ability) const
{
	if (const config &abil = cfg_.child("abilities"))
	{
		for (const config::any_child &ab : abil.all_children_range()) {
			if (ab.cfg["id"] == ability)
				return true;
		}
	}
	return false;
}

std::vector<std::string> unit_type::get_ability_list() const
{
	std::vector<std::string> res;

	const config &abilities = cfg_.child("abilities");
	if (!abilities) return res;

	for (const config::any_child &ab : abilities.all_children_range()) {
		std::string id = ab.cfg["id"];
		if (!id.empty())
			res.push_back(std::move(id));
	}

	return res;
}

bool unit_type::hide_help() const {
	return hide_help_ || unit_types.hide_help(id_, race_->id());
}

void unit_type::add_advancement(const unit_type &to_unit,int xp)
{
	const std::string &to_id = to_unit.id_;

	// Add extra advancement path to this unit type
	LOG_CONFIG << "adding advancement from " << log_id() << " to " << to_unit.log_id() << "\n";
	if(std::find(advances_to_.begin(), advances_to_.end(), to_id) == advances_to_.end()) {
		advances_to_.push_back(to_id);
	} else {
		LOG_CONFIG << "advancement from " << log_id() << " to "
		           << to_unit.log_id() << " already known, ignoring.\n";
		return;
	}

	if(xp > 0) {
		//xp is 0 in case experience= wasn't given.
		if(!in_advancefrom_) {
			//This function is called for and only for an [advancefrom] tag in a unit_type referencing this unit_type.
			in_advancefrom_ = true;
			experience_needed_ = xp;
			DBG_UT << "Changing experience_needed from " << experience_needed_
			       << " to " << xp << " due to (first) [advancefrom] of "
			       << to_unit.log_id() << "\n";
		}
		else if(experience_needed_ > xp) {
			experience_needed_ = xp;
			DBG_UT << "Lowering experience_needed from " << experience_needed_
			       << " to " << xp << " due to (multiple, lower) [advancefrom] of "
			       << to_unit.log_id() << "\n";
		}
		else
			DBG_UT << "Ignoring experience_needed change from " << experience_needed_
			       << " to " << xp << " due to (multiple, higher) [advancefrom] of "
			       << to_unit.log_id() << "\n";
	}

	// Add advancements to gendered subtypes, if supported by to_unit
	for(int gender=0; gender<=1; ++gender) {
		if(gender_types_[gender] == nullptr) continue;
		if(to_unit.gender_types_[gender] == nullptr) {
			WRN_CF << to_unit.log_id() << " does not support gender " << gender << std::endl;
			continue;
		}
		LOG_CONFIG << "gendered advancement " << gender << ": ";
		gender_types_[gender]->add_advancement(*(to_unit.gender_types_[gender]),xp);
	}

	if ( cfg_.has_child("variation") ) {
		// Make sure the variations are created.
		unit_types.build_unit_type(*this, VARIATIONS);

		// Add advancements to variation subtypes.
		// Since these are still a rare and special-purpose feature,
		// we assume that the unit designer knows what they're doing,
		// and don't block advancements that would remove a variation.
		for(variations_map::iterator v=variations_.begin();
			v!=variations_.end(); ++v) {
			LOG_CONFIG << "variation advancement: ";
			v->second.add_advancement(to_unit,xp);
		}
	}
}

static void advancement_tree_internal(const std::string& id, std::set<std::string>& tree)
{
	const unit_type *ut = unit_types.find(id);
	if (!ut)
		return;

	for (const std::string& adv : ut->advances_to()) {
		if (tree.insert(adv).second) {
			// insertion succeed, expand the new type
			advancement_tree_internal(adv, tree);
		}
	}
}

std::set<std::string> unit_type::advancement_tree() const
{
	std::set<std::string> tree;
	advancement_tree_internal(id_, tree);
	return tree;
}

const std::vector<std::string> unit_type::advances_from() const
{
	// currently not needed (only help call us and already did it)
	unit_types.build_all(unit_type::HELP_INDEXED);

	std::vector<std::string> adv_from;
	for (const unit_type_data::unit_type_map::value_type &ut : unit_types.types())
	{
		for (const std::string& adv : ut.second.advances_to()) {
			if (adv == id_)
				adv_from.push_back(ut.second.id());
		}
	}
	return adv_from;
}


// This function is only meant to return the likely state a given status
// for a new recruit of this type. It should not be used to check if
// a particular unit has it, use get_state(status_name) for that.
bool unit_type::musthave_status(const std::string& status_name) const
{
	// Statuses default to absent.
	bool current_status = false;

	// Look at all of the "musthave" traits to see if the
	// status gets changed. In the unlikely event it gets changed
	// multiple times, we want to try to do it in the same order
	// that unit::apply_modifications does things.
	for (const config &mod : possible_traits())
	{
		if (mod["availability"] != "musthave")
			continue;

		for (const config &effect : mod.child_range("effect"))
		{
			// See if the effect only applies to
			// certain unit types But don't worry
			// about gender checks, since we don't
			// know what the gender of the
			// hypothetical recruit is.
			const std::string &ut = effect["unit_type"];
			if (!ut.empty()) {
				const std::vector<std::string> &types = utils::split(ut);
				if(std::find(types.begin(), types.end(), id()) == types.end())
					continue;
			}

			// We're only interested in status changes.
			if (effect["apply_to"] != "status") {
				continue;
			}
			if (effect["add"] == status_name) {
				current_status = true;
			}
			if (effect["remove"] == status_name) {
				current_status = false;
			}
		}
	}

	return current_status;
}

const std::string& unit_type::flag_rgb() const {
	return flag_rgb_.empty() ? game_config::unit_rgb : flag_rgb_;
}

bool unit_type::has_random_traits() const
{
	if (num_traits() == 0) return false;
	for(const auto& cfg : possible_traits()) {
		const config::attribute_value& availability = cfg["availability"];
		if(availability.blank()) return true;
		if(strcmp(availability.str().c_str(), "musthave") != 0) return true;
	}
	return false;
}

std::vector<std::string> unit_type::variations() const
{
	std::vector<std::string> retval;
	retval.reserve(variations_.size());
	for (const variations_map::value_type &val : variations_) {
		retval.push_back(val.first);
	}
	return retval;
}

bool unit_type::has_variation(const std::string& variation_id) const
{
	return variations_.find(variation_id) != variations_.end();
}

bool unit_type::show_variations_in_help() const
{
	for (const variations_map::value_type &val : variations_) {
		if (!val.second.hide_help()) {
			return true;
		}
	}

	return false;
}

/**
 * Generates (and returns) a trimmed config suitable for use with units.
 */
const config & unit_type::build_unit_cfg() const
{
	// We start with all attributes.
	assert(unit_cfg_.empty());
	unit_cfg_.append_attributes(cfg_);

	// Remove "pure" unit_type attributes (attributes that do not get directly
	// copied to units; some do get copied, but under different keys).
	static char const *unit_type_attrs[] = { "attacks", "base_ids", "die_sound",
		"experience", "flies", "healed_sound", "hide_help", "hitpoints",
		"id", "ignore_race_traits", "inherit", "movement", "movement_type",
		"name", "num_traits", "variation_id", "variation_name", "recall_cost",
		"cost", "level", "gender", "flag_rgb", "alignment", "advances_to", "do_not_list"
	};
	for (const char *attr : unit_type_attrs) {
		unit_cfg_.remove_attribute(attr);
	}
	built_unit_cfg_ = true;
	return unit_cfg_;
}

int unit_type::resistance_against(const std::string& damage_name, bool attacker) const
{
	int resistance = movement_type_.resistance_against(damage_name);
	unit_ability_list resistance_abilities;
	if (const config &abilities = cfg_.child("abilities")) {
		for (const config& cfg : abilities.child_range("resistance")) {
			if (!cfg["affect_self"].to_bool(true)) {
				continue;
			}
			if (!resistance_filter_matches(cfg, attacker, damage_name, 100 - resistance)) {
				continue;
			}
			resistance_abilities.push_back(unit_ability(&cfg, map_location::null_location()));
		}
	}
	if (!resistance_abilities.empty()) {
		unit_abilities::effect resist_effect(resistance_abilities, 100 - resistance, false);
		resistance = 100 - std::min<int>(resist_effect.get_composite_value(),
				resistance_abilities.highest("max_value").first);
	}
	return resistance;
}

bool unit_type::resistance_filter_matches(const config& cfg, bool attacker, const std::string& damage_name, int res) const
{
	if(!(cfg["active_on"].empty() || (attacker && cfg["active_on"]=="offense") || (!attacker && cfg["active_on"]=="defense"))) {
		return false;
	}
	const std::string& apply_to = cfg["apply_to"];
	if(!apply_to.empty()) {
		if(damage_name != apply_to) {
			if ( apply_to.find(',') != std::string::npos  &&
			     apply_to.find(damage_name) != std::string::npos ) {
				const std::vector<std::string>& vals = utils::split(apply_to);
				if(std::find(vals.begin(),vals.end(),damage_name) == vals.end()) {
					return false;
				}
			} else {
				return false;
			}
		}
	}
	if (!unit_abilities::filter_base_matches(cfg, res)) return false;
	return true;
}

/** Implementation detail of unit_type::alignment_description */

MAKE_ENUM (ALIGNMENT_FEMALE_VARIATION,
	(LAWFUL, N_("female^lawful"))
	(FEMALE_NEUTRAL, N_("female^neutral"))
	(CHAOTIC, N_("female^chaotic"))
	(LIMINAL, N_("female^liminal"))
)

std::string unit_type::alignment_description(ALIGNMENT align, unit_race::GENDER gender)
{
	static_assert(ALIGNMENT_FEMALE_VARIATION::count == ALIGNMENT::count, "ALIGNMENT_FEMALE_VARIATION and ALIGNMENT do not have the same number of values");
	assert(align.valid());
	std::string str = std::string();
	if (gender == unit_race::FEMALE) {
		ALIGNMENT_FEMALE_VARIATION fem = align.cast<ALIGNMENT_FEMALE_VARIATION::type>();
		str = fem.to_string();
	} else {
		str = align.to_string();
	}
	return translation::sgettext(str.c_str());
}


/* ** unit_type_data ** */


unit_type_data::unit_type_data() :
	types_(),
	movement_types_(),
	races_(),
	hide_help_all_(false),
	hide_help_type_(),
	hide_help_race_(),
	unit_cfg_(nullptr),
	build_status_(unit_type::NOT_BUILT)
{
}


namespace { // Helpers for set_config()
	/**
	 * Spits out an error message and throws a config::error.
	 * Called when apply_base_unit() detects a cycle.
	 * (This exists merely to take the error message out of that function.)
	 */
	void throw_base_unit_recursion_error(
		const std::vector<std::string> & base_tree, const std::string & base_id)
	{
		std::stringstream ss;
		ss << "[base_unit] recursion loop in [unit_type] ";
		for (const std::string &step : base_tree)
			ss << step << "->";
		ss << base_id;
		ERR_CF << ss.str() << '\n';
		throw config::error(ss.str());
	}

	/**
	 * Locates the config for the unit type with id= @a key within @a all_types.
	 * Throws a config::error if the unit type cannot be found.
	 */
	config & find_unit_type_config(const std::string & key, config & all_types)
	{
		config & cfg = all_types.find_child("unit_type", "id", key);
		if ( cfg )
			return cfg;

		// Bad WML!
		ERR_CF << "unit type not found: " << key << std::endl;
		ERR_CF << all_types << std::endl;
		throw config::error("unit type not found: " + key);
	}

	/**
	 * Modifies the provided config by merging all base units into it.
	 * The @a base_tree parameter is used solely for detecting and reporting
	 * cycles of base units; it is no longer needed to prevent infinite loops.
	 */
	void apply_base_unit(config & ut_cfg, config & all_types,
	                     std::vector<std::string> &base_tree)
	{
		// Get a list of base units to apply.
		std::vector<std::string> base_ids;
		for (config & base : ut_cfg.child_range("base_unit"))
			base_ids.push_back(base["id"]);

		if ( base_ids.empty() )
			// Nothing to do.
			return;

		// Store the base ids for the help system.
		ut_cfg["base_ids"] = utils::join(base_ids);

		// Clear the base units (otherwise they could interfere with the merge).
		// This has the side-effect of breaking cycles, hence base_tree is
		// merely for error detection, not error recovery.
		ut_cfg.clear_children("base_unit");

		// Merge the base units, in order.
		for (const std::string & base_id : base_ids) {
			// Detect recursion so the WML author is made aware of an error.
			if ( std::find(base_tree.begin(), base_tree.end(), base_id) != base_tree.end() )
				throw_base_unit_recursion_error(base_tree, base_id);

			// Find the base unit.
			config & base_cfg = find_unit_type_config(base_id, all_types);
			// Make sure the base unit has had its base units accounted for.
			base_tree.push_back(base_id);
			apply_base_unit(base_cfg, all_types, base_tree);
			base_tree.pop_back();

			// Merge the base unit "under" our config.
			ut_cfg.inherit_from(base_cfg);
		}
	}

	/**
	 * Handles inheritance for configs of [male], [female], and [variation].
	 * Also removes gendered children, as those serve no purpose.
	 * @a default_inherit is the default value for inherit=.
	 */
	void fill_unit_sub_type(config & var_cfg, const config & parent,
	                        bool default_inherit)
	{
		if ( var_cfg["inherit"].to_bool(default_inherit) ) {
			var_cfg.inherit_from(parent);
		}
		var_cfg.clear_children("male");
		var_cfg.clear_children("female");
	}

	/**
	 * Processes [variation] tags of @a ut_cfg, handling inheritance and
	 * child clearing.
	 */
	void handle_variations(config & ut_cfg)
	{
		// Most unit types do not have variations.
		if ( !ut_cfg.has_child("variation") )
			return;

		// Pull the variations out of the base unit type.
		config variations;
		variations.splice_children(ut_cfg, "variation");

		// Handle each variation's inheritance.
		for (config &var_cfg : variations.child_range("variation"))
			fill_unit_sub_type(var_cfg, ut_cfg, false);

		// Restore the variations.
		ut_cfg.splice_children(variations, "variation");
	}

	const boost::regex fai_identifier("[a-zA-Z_]+");

	template<typename MoveT>
	void patch_movetype(MoveT& mt, const std::string& new_key, const std::string& formula_str, int default_val, bool replace) {
		config temp_cfg, original_cfg;
		mt.write(original_cfg);
		if (!replace && !original_cfg[new_key].blank()) {
			// Don't replace if the key already exists in the config (even if empty).
			return;
		}
		gui2::typed_formula<int> formula(formula_str);
		wfl::map_formula_callable original;
		boost::sregex_iterator m(formula_str.begin(), formula_str.end(), fai_identifier);
		for (const boost::sregex_iterator::value_type& p : std::make_pair(m, boost::sregex_iterator())) {
			const std::string var_name = p.str();
			wfl::variant val(original_cfg[var_name].to_int(default_val));
			original.add(var_name, val);
		}
		temp_cfg[new_key] = formula(original);
		mt.merge(temp_cfg, true);
	}
}// unnamed namespace

/**
 * Resets all data based on the provided config.
 * This includes some processing of the config, such as expanding base units.
 * A pointer to the config is stored, so the config must be persistent.
 */
void unit_type_data::set_config(config &cfg)
{
	DBG_UT << "unit_type_data::set_config, name: " << cfg["name"] << "\n";

	clear();
	unit_cfg_ = &cfg;

	for (const config &mt : cfg.child_range("movetype"))
	{
		movement_types_.insert(std::make_pair(mt["name"].str(), movetype(mt)));
		gui2::dialogs::loading_screen::progress();
	}

	for (const config &r : cfg.child_range("race"))
	{
		const unit_race race(r);
		races_.insert(std::pair<std::string,unit_race>(race.id(),race));
		gui2::dialogs::loading_screen::progress();
	}

	// Movetype resistance patching
	for (const config &r : cfg.child_range("resistance_defaults"))
	{
		const std::string& dmg_type = r["id"];
		config temp_cfg;
		for (const config::attribute &attr : r.attribute_range()) {
			const std::string &mt = attr.first;
			if (mt == "id" || mt == "default" || movement_types_.find(mt) == movement_types_.end()) {
				continue;
			}
			patch_movetype(movement_types_[mt].get_resistances(), dmg_type, attr.second, 100, true);
		}
		if (r.has_attribute("default")) {
			for (movement_type_map::value_type &mt : movement_types_) {
				// Don't apply a default if a value is explicitly specified.
				if (r.has_attribute(mt.first)) {
					continue;
				}
				patch_movetype(mt.second.get_resistances(), dmg_type, r["default"], 100, false);
			}
		}
	}

	// Movetype move/defend patching
	for (const config &terrain : cfg.child_range("terrain_defaults"))
	{
		const std::string& ter_type = terrain["id"];
		config temp_cfg;
		static const std::string terrain_info_tags[] = {"movement", "vision", "jamming", "defense"};
		for (const std::string &tag : terrain_info_tags) {
			if (!terrain.has_child(tag)) {
				continue;
			}
			const config& info = terrain.child(tag);
			for (const config::attribute &attr : info.attribute_range()) {
				const std::string &mt = attr.first;
				if (mt == "default" || movement_types_.find(mt) == movement_types_.end()) {
					continue;
				}
				if (tag == "defense") {
					patch_movetype(movement_types_[mt].get_defense(), ter_type, attr.second, 100, true);
				} else if (tag == "vision") {
					patch_movetype(movement_types_[mt].get_vision(), ter_type, attr.second, 99, true);
				} else if (tag == "movement") {
					patch_movetype(movement_types_[mt].get_movement(), ter_type, attr.second, 99, true);
				} else if (tag == "jamming") {
					patch_movetype(movement_types_[mt].get_jamming(), ter_type, attr.second, 99, true);
				}
			}
			if (info.has_attribute("default")) {
				for (movement_type_map::value_type &mt : movement_types_) {
					// Don't apply a default if a value is explicitly specified.
					if (info.has_attribute(mt.first)) {
						continue;
					}
					if (tag == "defense") {
						patch_movetype(mt.second.get_defense(), ter_type, info["default"], 100, false);
					} else if (tag == "vision") {
						patch_movetype(mt.second.get_vision(), ter_type, info["default"], 99, false);
					} else if (tag == "movement") {
						patch_movetype(mt.second.get_movement(), ter_type, info["default"], 99, false);
					} else if (tag == "jamming") {
						patch_movetype(mt.second.get_jamming(), ter_type, info["default"], 99, false);
					}
				}
			}
		}
	}

	// Apply base units.
	for (config &ut : cfg.child_range("unit_type"))
	{
		if ( ut.has_child("base_unit") ) {
			// Derived units must specify a new id.
			// (An error message will be emitted later if id is empty.)
			const std::string id = ut["id"];
			if ( !id.empty() ) {
				std::vector<std::string> base_tree(1, id);
				apply_base_unit(ut, cfg, base_tree);
				gui2::dialogs::loading_screen::progress();
			}
		}
	}

	// Handle inheritance and recording of unit types.
	for (config &ut : cfg.child_range("unit_type"))
	{
		std::string id = ut["id"];
		// Every type is required to have an id.
		if ( id.empty() ) {
			ERR_CF << "[unit_type] with empty id=, ignoring:\n" << ut.debug();
			continue;
		}

		// Complete the gender-specific children of the config.
		if ( config &male_cfg = ut.child("male") ) {
			fill_unit_sub_type(male_cfg, ut, true);
			handle_variations(male_cfg);
		}
		if ( config &female_cfg = ut.child("female") ) {
			fill_unit_sub_type(female_cfg, ut, true);
			handle_variations(female_cfg);
		}

		// Complete the variation-defining children of the config.
		handle_variations(ut);

		// Record this unit type.
		if ( insert(std::make_pair(id, unit_type(ut))).second ) {
			LOG_CONFIG << "added " << id << " to unit_type list (unit_type_data.unit_types)\n";
		} else {
			ERR_CF << "Multiple [unit_type]s with id=" << id << " encountered." << std::endl;
		}

		gui2::dialogs::loading_screen::progress();
	}

	// Build all unit types. (This was not done within the loop for performance.)
	build_all(unit_type::CREATED);

	// Suppress some unit types (presumably used as base units) from the help.
	if (const config &hide_help = cfg.child("hide_help")) {
		hide_help_all_ = hide_help["all"].to_bool();
		read_hide_help(hide_help);
	}
}

/**
 * Finds a unit_type by its id() and makes sure it is built to the specified level.
 */
const unit_type *unit_type_data::find(const std::string& key, unit_type::BUILD_STATUS status) const
{
	if (key.empty() || key == "random") return nullptr;

	DBG_CF << "trying to find " << key  << " in unit_type list (unit_type_data.unit_types)\n";
    const unit_type_map::iterator itor = types_.find(key);

    //This might happen if units of another era are requested (for example for savegames)
    if (itor == types_.end()){
        /*
        for (unit_type_map::const_iterator ut = types_.begin(); ut != types_.end(); ut++)
            DBG_UT << "Known unit_types: key = '" << ut->first << "', id = '" << ut->second.log_id() << "'\n";
        */
		return nullptr;
    }

    // Make sure the unit_type is built to the requested level.
    build_unit_type(itor->second, status);

	return &itor->second;
}

void unit_type_data::check_types(const std::vector<std::string>& types) const
{
	for (const std::string& type : types) {
		if(!find(type)) throw game::game_error("unknown unit type: " + type);
	}
}

void unit_type_data::clear()
{
	types_.clear();
	movement_types_.clear();
	races_.clear();
	build_status_ = unit_type::NOT_BUILT;

	hide_help_all_ = false;
	hide_help_race_.clear();
	hide_help_type_.clear();
}

void unit_type_data::build_all(unit_type::BUILD_STATUS status)
{
	// Nothing to do if already built to the requested level.
	if ( status <= build_status_ )
		return;
	assert(unit_cfg_ != nullptr);

	for (unit_type_map::iterator u = types_.begin(), u_end = types_.end(); u != u_end; ++u) {
		build_unit_type(u->second, status);
		gui2::dialogs::loading_screen::progress();
	}
	// Handle [advancefrom] (once) after building to (at least) the CREATED level.
	// (Currently, this could be simply a test for build_status_ == NOT_BUILT,
	// but to guard against future changes, use a more thorough test.)
	if ( build_status_ < unit_type::CREATED  &&  unit_type::CREATED <= status )
		for (unit_type_map::iterator u = types_.begin(), u_end = types_.end(); u != u_end; ++u) {
			add_advancement(u->second);
		}

	build_status_ = status;
}

void unit_type_data::read_hide_help(const config& cfg)
{
	if (!cfg)
		return;

	hide_help_race_.push_back(std::set<std::string>());
	hide_help_type_.push_back(std::set<std::string>());

	std::vector<std::string> races = utils::split(cfg["race"]);
	hide_help_race_.back().insert(races.begin(), races.end());

	std::vector<std::string> types = utils::split(cfg["type"]);
	hide_help_type_.back().insert(types.begin(), types.end());

	std::vector<std::string> trees = utils::split(cfg["type_adv_tree"]);
	hide_help_type_.back().insert(trees.begin(), trees.end());
	for (const std::string& t_id : trees) {
		unit_type_map::iterator ut = types_.find(t_id);
		if (ut != types_.end()) {
			std::set<std::string> adv_tree = ut->second.advancement_tree();
			hide_help_type_.back().insert(adv_tree.begin(), adv_tree.end());
		}
	}

	// we call recursively all the imbricated [not] tags
	read_hide_help(cfg.child("not"));
}

bool unit_type_data::hide_help(const std::string& type, const std::string& race) const
{
	bool res = hide_help_all_;
	int lvl = hide_help_all_ ? 1 : 0; // first level is covered by 'all=yes'

	// supposed to be equal but let's be cautious
	int lvl_nb = std::min(hide_help_race_.size(), hide_help_type_.size());

	for (; lvl < lvl_nb; ++lvl) {
		if (hide_help_race_[lvl].count(race) || hide_help_type_[lvl].count(type))
			res = !res; // each level is a [not]
	}
	return res;
}

void unit_type_data::add_advancement(unit_type& to_unit) const
{
    const config& cfg = to_unit.get_cfg();

    for (const config &af : cfg.child_range("advancefrom"))
    {
        const std::string &from = af["unit"];
        int xp = af["experience"];

        unit_type_data::unit_type_map::iterator from_unit = types_.find(from);

		if (from_unit == types_.end()) {
			std::ostringstream msg;
			msg << "unit type '" << from << "' not found when resolving [advancefrom] tag for '"
				<< to_unit.log_id() << "'";
			throw config::error(msg.str());
		}

        // Fix up advance_from references
        from_unit->second.add_advancement(to_unit, xp);

        DBG_UT << "Added advancement ([advancefrom]) from " << from << " to " << to_unit.log_id() << "\n";
    }
}

const unit_race *unit_type_data::find_race(const std::string &key) const
{
	race_map::const_iterator i = races_.find(key);
	return i != races_.end() ? &i->second : nullptr;
}

void unit_type::check_id(std::string& id)
{
	assert(!id.empty());
	//we don't allow leading whitepaces
	if (id[0] == ' ') {
		throw error("Found unit type id with a leading whitespace \"" + id + "\"");
	}
	bool gave_wanrning = false;
	for (size_t pos = 0; pos < id.size(); ++pos) {
		const char c = id[pos];
		const bool valid = c == '_' || c == ' ' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
		if (!valid) {
			if (!gave_wanrning) {
				ERR_UT << "Found unit type id with invalid chracters: \"" << id << "\"\n";
				gave_wanrning = true;
			}
			id[pos] = '_';
		}
	}

}

unit_type_data unit_types;


void adjust_profile(std::string& profile)
{
	// Create a temp copy
	std::string temp = profile;

	static const std::string path_adjust = "/transparent";
	const std::string::size_type offset = profile.find_last_of('/', profile.find('~'));

	// If the path already refers to /transparent...
	if(profile.find(path_adjust) != std::string::npos && offset != std::string::npos) {
		if(!image::locator(profile).file_exists()) {
			profile.replace(profile.find(path_adjust), path_adjust.length(), "");
		}

		return;
	}

	// else, check for the file with /transparent appended...
	offset != std::string::npos ?
		temp.insert(offset, path_adjust) : temp = path_adjust + temp;

	// and use that path if it exists
	if(image::locator(temp).file_exists()) {
		profile = temp;
	}
}
