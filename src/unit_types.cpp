/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
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

#include "global.hpp"

#include "unit_types.hpp"

#include "game_config.hpp"
#include "gettext.hpp"
#include "loadscreen.hpp"
#include "log.hpp"
#include "portrait.hpp"
#include "unit.hpp"
#include "unit_abilities.hpp"
#include "unit_animation.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
#define WRN_CF LOG_STREAM(warn, log_config)
#define LOG_CONFIG LOG_STREAM(info, log_config)
#define DBG_CF LOG_STREAM(debug, log_config)

static lg::log_domain log_unit("unit");
#define DBG_UT LOG_STREAM(debug, log_unit)
#define ERR_UT LOG_STREAM(err, log_unit)


/* ** attack_type ** */


attack_type::attack_type(const config& cfg) :
	self_loc_(),
	other_loc_(),
	is_attacker_(false),
	other_attack_(NULL),
	cfg_(cfg),
	description_(cfg["description"].t_str()),
	id_(cfg["name"]),
	type_(cfg["type"]),
	icon_(cfg["icon"]),
	range_(cfg["range"]),
	min_range_(cfg["min_range"].to_int(1)),
	max_range_(cfg["max_range"].to_int(1)),
	damage_(cfg["damage"]),
	num_attacks_(cfg["number"]),
	attack_weight_(cfg["attack_weight"].to_double(1.0)),
	defense_weight_(cfg["defense_weight"].to_double(1.0)),
	accuracy_(cfg["accuracy"]),
	movement_used_(cfg["movement_used"].to_int(100000)),
	parry_(cfg["parry"])

{
	if (description_.empty())
		description_ = egettext(id_.c_str());

	if(icon_.empty()){
		if (id_ != "")
			icon_ = "attacks/" + id_ + ".png";
		else
			icon_ = "attacks/blank-attack.png";
	}
}

attack_type::~attack_type()
{
}

std::string attack_type::accuracy_parry_description() const
{
	if(accuracy_ == 0 && parry_ == 0) {
		return "";
	}

	std::ostringstream s;
	s << utils::signed_percent(accuracy_);

	if(parry_ != 0) {
		s << "/" << utils::signed_percent(parry_);
	}

	return s.str();
}

/**
 * Returns whether or not *this matches the given @a filter, ignoring the
 * complexities introduced by [and], [or], and [not].
 */
static bool matches_simple_filter(const attack_type & attack, const config & filter)
{
	const std::vector<std::string>& filter_range = utils::split(filter["range"]);
	const std::string& filter_damage = filter["damage"];
	const std::vector<std::string> filter_name = utils::split(filter["name"]);
	const std::vector<std::string> filter_type = utils::split(filter["type"]);
	const std::string filter_special = filter["special"];

	if ( !filter_range.empty() && std::find(filter_range.begin(), filter_range.end(), attack.range()) == filter_range.end() )
		return false;

	if ( !filter_damage.empty() && !in_ranges(attack.damage(), utils::parse_ranges(filter_damage)) )
		return false;

	if ( !filter_name.empty() && std::find(filter_name.begin(), filter_name.end(), attack.id()) == filter_name.end() )
		return false;

	if ( !filter_type.empty() && std::find(filter_type.begin(), filter_type.end(), attack.type()) == filter_type.end() )
		return false;

	if ( !filter_special.empty() && !attack.get_special_bool(filter_special, true) )
		return false;

	// Passed all tests.
	return true;
}

/**
 * Returns whether or not *this matches the given @a filter.
 */
bool attack_type::matches_filter(const config& filter) const
{
	// Handle the basic filter.
	bool matches = matches_simple_filter(*this, filter);

	// Handle [and], [or], and [not] with in-order precedence
	BOOST_FOREACH( const config::any_child &condition, filter.all_children_range() )
	{
		// Handle [and]
		if ( condition.key == "and" )
			matches = matches && matches_filter(condition.cfg);

		// Handle [or]
		else if ( condition.key == "or" )
			matches = matches || matches_filter(condition.cfg);

		// Handle [not]
		else if ( condition.key == "not" )
			matches = matches && !matches_filter(condition.cfg);
	}

	return matches;
}


/**
 * Modifies *this using the specifications in @a cfg, but only if *this matches
 * @a cfg viewed as a filter.
 *
 * If *description is provided, it will be set to a (translated) description
 * of the modification(s) applied (currently only changes to the number of
 * strikes, damage, accuracy, and parry are included in this description).
 *
 * @returns whether or not @c this matched the @a cfg as a filter.
 */
bool attack_type::apply_modification(const config& cfg,std::string* description)
{
	if( !matches_filter(cfg) )
		return false;

	const std::string& set_name = cfg["set_name"];
	const t_string& set_desc = cfg["set_description"];
	const std::string& set_type = cfg["set_type"];
	const std::string& set_icon = cfg["set_icon"];
	const std::string& del_specials = cfg["remove_specials"];
	const config &set_specials = cfg.child("set_specials");
	const std::string& increase_damage = cfg["increase_damage"];
	const std::string& increase_attacks = cfg["increase_attacks"];
	const std::string& set_attack_weight = cfg["attack_weight"];
	const std::string& set_defense_weight = cfg["defense_weight"];
	const std::string& increase_accuracy = cfg["increase_accuracy"];
	const std::string& increase_parry = cfg["increase_parry"];

	std::stringstream desc;

	if(set_name.empty() == false) {
		id_ = set_name;
		cfg_["name"] = id_;
	}

	if(set_desc.empty() == false) {
		description_ = set_desc;
		cfg_["description"] = description_;
	}

	if(set_type.empty() == false) {
		type_ = set_type;
		cfg_["type"] = type_;
	}

	if(set_icon.empty() == false) {
		icon_ = set_icon;
		cfg_["icon"] = icon_;
	}

	if(del_specials.empty() == false) {
		const std::vector<std::string>& dsl = utils::split(del_specials);
		if (config &specials = cfg_.child("specials"))
		{
			config new_specials;
			BOOST_FOREACH(const config::any_child &vp, specials.all_children_range()) {
				std::vector<std::string>::const_iterator found_id =
					std::find(dsl.begin(), dsl.end(), vp.cfg["id"].str());
				if (found_id == dsl.end()) {
					new_specials.add_child(vp.key, vp.cfg);
				}
			}
			cfg_.clear_children("specials");
			cfg_.add_child("specials",new_specials);
		}
	}

	if (set_specials) {
		const std::string &mode = set_specials["mode"];
		if (mode != "append") {
			cfg_.clear_children("specials");
		}
		config &new_specials = cfg_.child_or_add("specials");
		BOOST_FOREACH(const config::any_child &value, set_specials.all_children_range()) {
			new_specials.add_child(value.key, value.cfg);
		}
	}

	if(increase_damage.empty() == false) {
		damage_ = utils::apply_modifier(damage_, increase_damage, 0);
		if (damage_ < 0) {
			damage_ = 0;
		}
		cfg_["damage"] = damage_;

		if(description != NULL) {
			int inc_damage = lexical_cast<int>(increase_damage);
			desc << utils::print_modifier(increase_damage) << " "
				 << _n("damage","damage", inc_damage);
		}
	}

	if(increase_attacks.empty() == false) {
		num_attacks_ = utils::apply_modifier(num_attacks_, increase_attacks, 1);
		cfg_["number"] = num_attacks_;

		if(description != NULL) {
			int inc_attacks = lexical_cast<int>(increase_attacks);
			desc << utils::print_modifier(increase_attacks) << " "
				 << _n("strike", "strikes", inc_attacks);
		}
	}

	if(increase_accuracy.empty() == false) {
		accuracy_ = utils::apply_modifier(accuracy_, increase_accuracy, 1);
		cfg_["accuracy"] = accuracy_;

		if(description != NULL) {
			int inc_acc = lexical_cast<int>(increase_accuracy);
			// Help xgettext with a directive to recognize the string as a non C printf-like string
			// xgettext:no-c-format
			desc << utils::signed_value(inc_acc) << _("% accuracy");
		}
	}

	if(increase_parry.empty() == false) {
		parry_ = utils::apply_modifier(parry_, increase_parry, 1);
		cfg_["parry"] = parry_;

		if(description != NULL) {
			int inc_parry = lexical_cast<int>(increase_parry);
			// xgettext:no-c-format
			desc << utils::signed_value(inc_parry) << _("% parry");
		}
	}

	if(set_attack_weight.empty() == false) {
		attack_weight_ = lexical_cast_default<double>(set_attack_weight,1.0);
		cfg_["attack_weight"] = attack_weight_;
	}

	if(set_defense_weight.empty() == false) {
		defense_weight_ = lexical_cast_default<double>(set_defense_weight,1.0);
		cfg_["defense_weight"] = defense_weight_;
	}

	if(description != NULL) {
		*description = desc.str();
	}

	return true;
}

/**
 * Trimmed down version of apply_modification(), with no modifications actually
 * made. This can be used to get a description of the modification(s) specified
 * by @a cfg (if *this matches cfg as a filter).
 *
 * If *description is provided, it will be set to a (translated) description
 * of the modification(s) that would be applied to the number of strikes
 * and damage.
 *
 * @returns whether or not @c this matched the @a cfg as a filter.
 */
bool attack_type::describe_modification(const config& cfg,std::string* description)
{
	if( !matches_filter(cfg) )
		return false;

	// Did the caller want the description?
	if(description != NULL) {
		const std::string& increase_damage = cfg["increase_damage"];
		const std::string& increase_attacks = cfg["increase_attacks"];

		std::stringstream desc;

		if(increase_damage.empty() == false) {
			int inc_damage = lexical_cast<int>(increase_damage);
			desc << utils::print_modifier(increase_damage) << " "
				 << _n("damage","damage", inc_damage);
		}

		if(increase_attacks.empty() == false) {
			int inc_attacks = lexical_cast<int>(increase_attacks);
			desc << utils::print_modifier(increase_attacks) << " "
				 << _n("strike", "strikes", inc_attacks);
		}

		*description = desc.str();
	}

	return true;
}


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
	big_profile_(o.big_profile_),
	flag_rgb_(o.flag_rgb_),
	num_traits_(o.num_traits_),
	variations_(o.variations_),
	default_variation_(o.default_variation_),
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
	possibleTraits_(o.possibleTraits_),
	genders_(o.genders_),
	animations_(o.animations_),
    build_status_(o.build_status_),
	portraits_(o.portraits_)
{
	gender_types_[0] = o.gender_types_[0] != NULL ? new unit_type(*o.gender_types_[0]) : NULL;
	gender_types_[1] = o.gender_types_[1] != NULL ? new unit_type(*o.gender_types_[1]) : NULL;

	for(variations_map::const_iterator i = o.variations_.begin(); i != o.variations_.end(); ++i) {
		variations_[i->first] = new unit_type(*i->second);
	}
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
	big_profile_(),
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
	alignment_(),
	movement_type_(),
	possibleTraits_(),
	genders_(),
	animations_(),
	build_status_(NOT_BUILT),
	portraits_()
{
	gender_types_[0] = NULL;
	gender_types_[1] = NULL;
}

unit_type::~unit_type()
{
	delete gender_types_[0];
	delete gender_types_[1];

	for(variations_map::iterator i = variations_.begin(); i != variations_.end(); ++i) {
		delete i->second;
	}
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

	const std::string& align = cfg_["alignment"];
	if(align == "lawful")
		alignment_ = LAWFUL;
	else if(align == "chaotic")
		alignment_ = CHAOTIC;
	else if(align == "neutral")
		alignment_ = NEUTRAL;
	else if(align == "liminal")
		alignment_ = LIMINAL;
	else {
		if ( !align.empty() ) {
			ERR_CF << "Invalid alignment found for " << log_id() << ": '" << align << "'\n";
		}
		alignment_ = NEUTRAL;
	}

	if ( race_ != &unit_race::null_race )
	{
		if (!race_->uses_global_traits()) {
			possibleTraits_.clear();
		}
		if ( cfg_["ignore_race_traits"].to_bool() ) {
			possibleTraits_.clear();
		} else {
			BOOST_FOREACH(const config &t, race_->additional_traits())
			{
				if (alignment_ != NEUTRAL || t["id"] != "fearless")
					possibleTraits_.add_child("trait", t);
			}
		}
		if (undead_variation_.empty()) {
			undead_variation_ = race_->undead_variation();
		}
	}

	// Insert any traits that are just for this unit type
	BOOST_FOREACH(const config &trait, cfg_.child_range("trait"))
	{
		possibleTraits_.add_child("trait", trait);
	}

	zoc_ = cfg_["zoc"].to_bool(level_ > 0);

	const config::attribute_value & alpha_blend = cfg_["alpha"];
	if(alpha_blend.empty() == false) {
		alpha_ = ftofxp(alpha_blend.to_double());
	}

	game_config::add_color_info(cfg_);

	BOOST_FOREACH(const config &portrait, cfg_.child_range("portrait")) {
		portraits_.push_back(tportrait(portrait));
	}

	hp_bar_scaling_ = cfg_["hp_bar_scaling"].to_double(game_config::hp_bar_scaling);
	xp_bar_scaling_ = cfg_["xp_bar_scaling"].to_double(game_config::xp_bar_scaling);

	// Propagate the build to the variations.
	BOOST_FOREACH(variations_map::value_type & variation, variations_) {
		variation.second->build_full(mv_types, races, traits);
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
	big_profile_ = cfg_["profile"].str();
	adjust_profile(small_profile_, big_profile_, image_);

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
		BOOST_FOREACH(const config::any_child &ab, abil_cfg.all_children_range()) {
			const config::attribute_value &name = ab.cfg["name"];
			if (!name.empty()) {
				abilities_.push_back(name.t_str());
				ability_tooltips_.push_back( legacy::ability_description(ab.cfg["description"].t_str()) );
			}
		}
	}

	BOOST_FOREACH(const config &adv, cfg_.child_range("advancement"))
	{
		BOOST_FOREACH(const config &effect, adv.child_range("effect"))
		{
			const config &abil_cfg = effect.child("abilities");
			if (!abil_cfg || effect["apply_to"] != "new_ability") {
				continue;
			}
			BOOST_FOREACH(const config::any_child &ab, abil_cfg.all_children_range()) {
				const config::attribute_value &name = ab.cfg["name"];
				if (!name.empty()) {
					adv_abilities_.push_back(name.t_str());
					adv_ability_tooltips_.push_back( legacy::ability_description(ab.cfg["description"].t_str()) );
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

	BOOST_FOREACH(const config &t, traits)
	{
		possibleTraits_.add_child("trait", t);
	}
	BOOST_FOREACH(const config &var_cfg, cfg_.child_range("variation"))
	{
		const std::string& var_id = var_cfg["variation_id"].empty() ?
				var_cfg["variation_name"] : var_cfg["variation_id"];

		unit_type *ut = new unit_type(var_cfg, id_);
		ut->debug_id_ = debug_id_ + " [" + var_id + "]";
		ut->base_id_ = base_id_;  // In case this is not id_.
		ut->build_help_index(mv_types, races, traits);
		variations_.insert(std::make_pair(var_id, ut));
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

	// These should still be NULL from the constructor.
	assert(gender_types_[0] == NULL);
	assert(gender_types_[1] == NULL);

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

	case WITHOUT_ANIMATIONS:
		// Animations are now built when they are accessed, so fall down to FULL.
	case FULL:
		build_full(movement_types, races, traits);
		return;

	default:
		ERR_UT << "Build of unit_type to unrecognized status (" << status << ") requested.\n";
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
	&& gender_types_[i] != NULL) {
		return *gender_types_[i];
	}

	return *this;
}

const unit_type& unit_type::get_variation(const std::string& id) const
{
	const variations_map::const_iterator i = variations_.find(id);
	if(i != variations_.end()) {
		return *i->second;
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

std::vector<attack_type> unit_type::attacks() const
{
	std::vector<attack_type> res;
	BOOST_FOREACH(const config &att, cfg_.child_range("attack")) {
		res.push_back(attack_type(att));
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

const char* unit_type::alignment_description(unit_type::ALIGNMENT align, unit_race::GENDER gender)
{
	static const char* aligns[] = { N_("lawful"), N_("neutral"), N_("chaotic"), N_("liminal") };
	static const char* aligns_female[] = { N_("female^lawful"), N_("female^neutral"), N_("female^chaotic"), N_("female^liminal") };
	const char** tlist = (gender == unit_race::MALE ? aligns : aligns_female);

	return (sgettext(tlist[align]));
}

const char* unit_type::alignment_id(unit_type::ALIGNMENT align)
{
	static const char* aligns[] = { "lawful", "neutral", "chaotic", "liminal" };
	return (aligns[align]);
}

bool unit_type::has_ability_by_id(const std::string& ability) const
{
	if (const config &abil = cfg_.child("abilities"))
	{
		BOOST_FOREACH(const config::any_child &ab, abil.all_children_range()) {
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

	BOOST_FOREACH(const config::any_child &ab, abilities.all_children_range()) {
		const std::string &id = ab.cfg["id"];
		if (!id.empty())
			res.push_back(id);
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
		if(gender_types_[gender] == NULL) continue;
		if(to_unit.gender_types_[gender] == NULL) {
			WRN_CF << to_unit.log_id() << " does not support gender " << gender << "\n";
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
			v->second->add_advancement(to_unit,xp);
		}
	}
}

static void advancement_tree_internal(const std::string& id, std::set<std::string>& tree)
{
	const unit_type *ut = unit_types.find(id);
	if (!ut)
		return;

	BOOST_FOREACH(const std::string& adv, ut->advances_to()) {
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
	BOOST_FOREACH(const unit_type_data::unit_type_map::value_type &ut, unit_types.types())
	{
		BOOST_FOREACH(const std::string& adv, ut.second.advances_to()) {
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
	BOOST_FOREACH(const config &mod, possible_traits())
	{
		if (mod["availability"] != "musthave")
			continue;

		BOOST_FOREACH(const config &effect, mod.child_range("effect"))
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

bool unit_type::has_random_traits() const
{
	if (num_traits() == 0) return false;
	config::const_child_itors t = possible_traits();
	while(t.first != t.second) {
		const config::attribute_value& availability = (*t.first)["availability"];
		if(availability.blank()) return true;
		if(strcmp(availability.str().c_str(), "musthave") != 0) return true;
		++t.first;
	}
	return false;
}

std::vector<std::string> unit_type::variations() const
{
	std::vector<std::string> retval;
	retval.reserve(variations_.size());
	BOOST_FOREACH(const variations_map::value_type &val, variations_) {
		retval.push_back(val.first);
	}
	return retval;
}

bool unit_type::has_variation(const std::string& variation_id) const
{
	return variations_.find(variation_id) != variations_.end();
}

/**
 * Generates (and returns) a trimmed config suitable for use with units.
 */
const config & unit_type::build_unit_cfg() const
{
	// We start with everything.
	unit_cfg_ = cfg_;

	// Remove "pure" unit_type attributes (attributes that do not get directly
	// copied to units; some do get copied, but under different keys).
	static char const *unit_type_attrs[] = { "attacks", "base_ids", "die_sound",
		"experience", "flies", "healed_sound", "hide_help", "hitpoints",
		"id", "ignore_race_traits", "inherit", "movement", "movement_type",
		"name", "num_traits", "variation_id", "variation_name", "recall_cost" };
	BOOST_FOREACH(const char *attr, unit_type_attrs) {
		unit_cfg_.remove_attribute(attr);
	}

	// Remove gendered children.
	unit_cfg_.clear_children("male");
	unit_cfg_.clear_children("female");

	// Remove movement type data (it will be received via a movetype object).
	unit_cfg_.clear_children("movement_costs");
	unit_cfg_.clear_children("vision_costs");
	unit_cfg_.clear_children("jamming_costs");
	unit_cfg_.clear_children("defense");
	unit_cfg_.clear_children("resistance");

	built_unit_cfg_ = true;
	return unit_cfg_;
}

int unit_type::resistance_against(const std::string& damage_name, bool attacker) const
{
	int resistance = movement_type_.resistance_against(damage_name);
	unit_ability_list resistance_abilities;
	if (const config &abilities = cfg_.child("abilities")) {
		BOOST_FOREACH(const config& cfg, abilities.child_range("resistance")) {
			if (!cfg["affect_self"].to_bool(true)) {
				continue;
			}
			if (!resistance_filter_matches(cfg, attacker, damage_name, 100 - resistance)) {
				continue;
			}
			resistance_abilities.push_back(unit_ability(&cfg, map_location::null_location));
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
	if(!(cfg["active_on"]=="" || (attacker && cfg["active_on"]=="offense") || (!attacker && cfg["active_on"]=="defense"))) {
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
/* ** unit_type_data ** */


unit_type_data::unit_type_data() :
	types_(),
	movement_types_(),
	races_(),
	hide_help_all_(false),
	hide_help_type_(),
	hide_help_race_(),
	unit_cfg_(NULL),
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
		BOOST_FOREACH(const std::string &step, base_tree)
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
		ERR_CF << "unit type not found: " << key << "\n";
		ERR_CF << all_types << "\n";
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
		BOOST_FOREACH (config & base, ut_cfg.child_range("base_unit") )
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
		BOOST_FOREACH(const std::string & base_id, base_ids) {
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
		BOOST_FOREACH (config &var_cfg, variations.child_range("variation"))
			fill_unit_sub_type(var_cfg, ut_cfg, false);

		// Restore the variations.
		ut_cfg.splice_children(variations, "variation");
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

	BOOST_FOREACH(const config &mt, cfg.child_range("movetype"))
	{
		movement_types_.insert(std::make_pair(mt["name"].str(), movetype(mt)));
		loadscreen::increment_progress();
	}

	BOOST_FOREACH(const config &r, cfg.child_range("race"))
	{
		const unit_race race(r);
		races_.insert(std::pair<std::string,unit_race>(race.id(),race));
		loadscreen::increment_progress();
	}

	// Apply base units.
	BOOST_FOREACH(config &ut, cfg.child_range("unit_type"))
	{
		if ( ut.has_child("base_unit") ) {
			// Derived units must specify a new id.
			// (An error message will be emitted later if id is empty.)
			const std::string id = ut["id"];
			if ( !id.empty() ) {
				std::vector<std::string> base_tree(1, id);
				apply_base_unit(ut, cfg, base_tree);
				loadscreen::increment_progress();
			}
		}
	}

	// Handle inheritance and recording of unit types.
	BOOST_FOREACH(config &ut, cfg.child_range("unit_type"))
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
			ERR_CF << "Multiple [unit_type]s with id=" << id << " encountered.\n";
		}

		loadscreen::increment_progress();
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
	if (key.empty() || key == "random") return NULL;

	DBG_CF << "trying to find " << key  << " in unit_type list (unit_type_data.unit_types)\n";
    const unit_type_map::iterator itor = types_.find(key);

    //This might happen if units of another era are requested (for example for savegames)
    if (itor == types_.end()){
        /*
        for (unit_type_map::const_iterator ut = types_.begin(); ut != types_.end(); ut++)
            DBG_UT << "Known unit_types: key = '" << ut->first << "', id = '" << ut->second.log_id() << "'\n";
        */
		return NULL;
    }

    // Make sure the unit_type is built to the requested level.
    build_unit_type(itor->second, status);

	return &itor->second;
}

void unit_type_data::check_types(const std::vector<std::string>& types) const
{
	BOOST_FOREACH(const std::string& type, types) {
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
	assert(unit_cfg_ != NULL);

	for (unit_type_map::iterator u = types_.begin(), u_end = types_.end(); u != u_end; ++u) {
		build_unit_type(u->second, status);
		loadscreen::increment_progress();
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
	BOOST_FOREACH(const std::string& t_id, trees) {
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

    BOOST_FOREACH(const config &af, cfg.child_range("advancefrom"))
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
	return i != races_.end() ? &i->second : NULL;
}

unit_type_data unit_types;


/* **  ** */


void adjust_profile(std::string &small, std::string &big, std::string const &def)
{
	if (big.empty())
	{
		// No profile data; use the default image.
		small = def;
		big = def;
	}
	else if (small.empty())
	{
		// No small profile; use the current profile for it and
		// try to infer the big one.
		small = big;
		std::string::size_type offset = big.find('~');
		offset = big.find_last_of('/', offset);
		if (offset != std::string::npos) {
			big.insert(offset, "/transparent");
		} else {
			big = "transparent/" + big;
		}
		if (!image::locator(big).file_exists())
			big = small;
	}
}


namespace legacy {
	/**
	 * Strips the name of an ability/special from its description.
	 * This adapts the pre-1.11.1 style of "<name>:\n<description>" to
	 * the current style of simply "<description>".
	 */
	t_string ability_description(const t_string & description)
	{
		///	@deprecated This function is legacy support. Remove it post-1.12.

		// We identify the legacy format by a colon ending the first line.
		std::string desc_str = description.str();
		const size_t colon_pos = desc_str.find(':');
		const size_t first_end_line = desc_str.find_first_of("\r\n");
		if ( colon_pos != std::string::npos  &&  colon_pos + 1 == first_end_line )
		{
			// Try to avoid spamming the deprecation message.
			static std::set< std::string > reported;
			const std::string name = desc_str.substr(0, colon_pos);
			if ( reported.count(name) == 0 )
			{
				reported.insert(name);
				// Inform the player.
				lg::wml_error << '"' << name << '"'
				              << " follows a deprecated format for its description,"
				              << " using its name as the first line. Support"
				              << " for that format will be removed in 1.12.\n";
			}

			// Strip the name from the description.
			// This is sort of bad in that it messes with retranslating, if the
			// player changes the game's language while playing. However, this
			// is temporary, so I think simplicity trumps in this case.
			desc_str.erase(0, colon_pos + 2);
			return t_string(desc_str);
		}

		// Not legacy, so this function just falls through.
		return description;
	}
}

