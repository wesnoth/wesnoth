/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "formula/callable_objects.hpp"
#include "game_config.hpp"
#include "game_errors.hpp" //thrown sometimes
#include "language.hpp" // for string_table
#include "log.hpp"
#include "units/abilities.hpp"
#include "units/animation.hpp"
#include "units/unit.hpp"

#include "gui/auxiliary/typed_formula.hpp"
#include "gui/dialogs/loading_screen.hpp"

#include <boost/range/algorithm_ext/erase.hpp>

#include <array>
#include <locale>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
#define WRN_CF LOG_STREAM(warn, log_config)
#define LOG_CONFIG LOG_STREAM(info, log_config)
#define DBG_CF LOG_STREAM(debug, log_config)

static lg::log_domain log_unit("unit");
#define DBG_UT LOG_STREAM(debug, log_unit)
#define LOG_UT LOG_STREAM(info, log_unit)
#define ERR_UT LOG_STREAM(err, log_unit)

/* ** unit_type ** */

unit_type::unit_type(const unit_type& o)
	: cfg_(o.cfg_)
	, id_(o.id_)
	, debug_id_(o.debug_id_)
	, parent_id_(o.parent_id_)
	, base_unit_id_(o.base_unit_id_)
	, type_name_(o.type_name_)
	, description_(o.description_)
	, hitpoints_(o.hitpoints_)
	, hp_bar_scaling_(o.hp_bar_scaling_)
	, xp_bar_scaling_(o.xp_bar_scaling_)
	, level_(o.level_)
	, recall_cost_(o.recall_cost_)
	, movement_(o.movement_)
	, vision_(o.vision_)
	, jamming_(o.jamming_)
	, max_attacks_(o.max_attacks_)
	, cost_(o.cost_)
	, usage_(o.usage_)
	, undead_variation_(o.undead_variation_)
	, image_(o.image_)
	, icon_(o.icon_)
	, small_profile_(o.small_profile_)
	, profile_(o.profile_)
	, flag_rgb_(o.flag_rgb_)
	, num_traits_(o.num_traits_)
	, variations_(o.variations_)
	, default_variation_(o.default_variation_)
	, variation_name_(o.variation_name_)
	, race_(o.race_)
	, abilities_(o.abilities_)
	, adv_abilities_(o.adv_abilities_)
	, zoc_(o.zoc_)
	, hide_help_(o.hide_help_)
	, do_not_list_(o.do_not_list_)
	, advances_to_(o.advances_to_)
	, advancements_(o.advancements_)
	, experience_needed_(o.experience_needed_)
	, alignment_(o.alignment_)
	, movement_type_(o.movement_type_)
	, possible_traits_(o.possible_traits_)
	, genders_(o.genders_)
	, animations_(o.animations_)
	, build_status_(o.build_status_)
{
	gender_types_[0].reset(gender_types_[0] != nullptr ? new unit_type(*o.gender_types_[0]) : nullptr);
	gender_types_[1].reset(gender_types_[1] != nullptr ? new unit_type(*o.gender_types_[1]) : nullptr);
}

unit_type::unit_type(defaut_ctor_t, const config& cfg, const std::string & parent_id)
	: cfg_(nullptr)
	, built_cfg_()
	, has_cfg_build_()
	, id_(cfg.has_attribute("id") ? cfg["id"].str() : parent_id)
	, debug_id_()
	, parent_id_(!parent_id.empty() ? parent_id : id_)
	, base_unit_id_()
	, type_name_()
	, description_()
	, hitpoints_(0)
	, hp_bar_scaling_(0.0)
	, xp_bar_scaling_(0.0)
	, level_(0)
	, recall_cost_()
	, movement_(0)
	, vision_(-1)
	, jamming_(0)
	, max_attacks_(0)
	, cost_(0)
	, usage_()
	, undead_variation_()
	, image_()
	, icon_()
	, small_profile_()
	, profile_()
	, flag_rgb_()
	, num_traits_(0)
	, gender_types_()
	, variations_()
	, default_variation_()
	, variation_name_()
	, race_(&unit_race::null_race)
	, abilities_()
	, adv_abilities_()
	, zoc_(false)
	, hide_help_(false)
	, do_not_list_()
	, advances_to_()
	, advancements_(cfg.child_range("advancement"))
	, experience_needed_(0)
	, alignment_(unit_alignments::type::neutral)
	, movement_type_()
	, possible_traits_()
	, genders_()
	, animations_()
	, build_status_(NOT_BUILT)
{
	if(auto base_unit = cfg.optional_child("base_unit")) {
		base_unit_id_ = base_unit["id"].str();
		LOG_UT << "type '" <<  id_ << "' has base unit '" << base_unit_id_ << "'";
	}
	check_id(id_);
	check_id(parent_id_);
}
unit_type::unit_type(const config& cfg, const std::string & parent_id)
	: unit_type(defaut_ctor_t(), cfg, parent_id)
{
	cfg_ = &cfg;

}

unit_type::unit_type(config&& cfg, const std::string & parent_id)
	: unit_type(defaut_ctor_t(), cfg, parent_id)
{
	built_cfg_ = std::make_unique<config>(std::move(cfg));
}


unit_type::~unit_type()
{
}

unit_type::ability_metadata::ability_metadata(const config& cfg)
	: id(cfg["id"])
	, name(cfg["name"].t_str())
	, name_inactive(cfg["name_inactive"].t_str())
	, female_name(cfg["female_name"].t_str())
	, female_name_inactive(cfg["female_name_inactive"].t_str())
	, description(cfg["description"].t_str())
	, description_inactive(cfg["description_inactive"].t_str())
	, affect_self(cfg["affect_self"].to_bool())
	, affect_allies(cfg["affect_allies"].to_bool())
	, affect_enemies(cfg["affect_enemies"].to_bool())
	, cumulative(cfg["cumulative"].to_bool())
{
}

/**
 * Load data into an empty unit_type (build to FULL).
 */
void unit_type::build_full(
		const movement_type_map& mv_types, const race_map& races, const config_array_view& traits)
{
	// Don't build twice.
	if(FULL <= build_status_) {
		return;
	}

	// Make sure we are built to the preceding build level.
	build_help_index(mv_types, races, traits);

	for(int i = 0; i < 2; ++i) {
		if(gender_types_[i]) {
			gender_types_[i]->build_full(mv_types, races, traits);
		}
	}

	if(race_ != &unit_race::null_race) {
		if(undead_variation_.empty()) {
			undead_variation_ = race_->undead_variation();
		}
	}

	zoc_ = get_cfg()["zoc"].to_bool(level_ > 0);

	game_config::add_color_info(game_config_view::wrap(get_cfg()));

	hp_bar_scaling_ = get_cfg()["hp_bar_scaling"].to_double(game_config::hp_bar_scaling);
	xp_bar_scaling_ = get_cfg()["xp_bar_scaling"].to_double(game_config::xp_bar_scaling);

	// Propagate the build to the variations.
	for(variations_map::value_type& variation : variations_) {
		variation.second.build_full(mv_types, races, traits);
	}

	// Deprecation messages, only seen when unit is parsed for the first time.

	build_status_ = FULL;
}

/**
 * Partially load data into an empty unit_type (build to HELP_INDEXED).
 */
void unit_type::build_help_index(
		const movement_type_map& mv_types, const race_map& races, const config_array_view& traits)
{
	// Don't build twice.
	if(HELP_INDEXED <= build_status_) {
		return;
	}

	// Make sure we are built to the preceding build level.
	build_created();

	const config& cfg = get_cfg();

	type_name_ = cfg["name"];
	description_ = cfg["description"];
	hitpoints_ = cfg["hitpoints"].to_int(1);
	level_ = cfg["level"].to_int();
	recall_cost_ = cfg["recall_cost"].to_int(-1);
	movement_ = cfg["movement"].to_int(1);
	vision_ = cfg["vision"].to_int(-1);
	jamming_ = cfg["jamming"].to_int(0);
	max_attacks_ = cfg["attacks"].to_int(1);
	usage_ = cfg["usage"].str();
	undead_variation_ = cfg["undead_variation"].str();
	default_variation_ = cfg["variation"].str();
	icon_ = cfg["image_icon"].str();
	small_profile_ = cfg["small_profile"].str();
	profile_ = cfg["profile"].str();
	flag_rgb_ = cfg["flag_rgb"].str();
	do_not_list_ = cfg["do_not_list"].to_bool(false);

	for(const config& sn : cfg.child_range("special_note")) {
		special_notes_.push_back(sn["note"]);
	}

	adjust_profile(profile_);

	alignment_ = unit_alignments::get_enum(cfg["alignment"].str()).value_or(unit_alignments::type::neutral);

	for(int i = 0; i < 2; ++i) {
		if(gender_types_[i]) {
			gender_types_[i]->build_help_index(mv_types, races, traits);
		}
	}

	for(auto& pair : variations_) {
		pair.second.build_help_index(mv_types, races, traits);
	}

	const race_map::const_iterator race_it = races.find(cfg["race"]);
	if(race_it != races.end()) {
		race_ = &race_it->second;
	} else {
		race_ = &unit_race::null_race;
	}

	// if num_traits is not defined, we use the num_traits from race
	num_traits_ = cfg["num_traits"].to_int(race_->num_traits());

	for(const std::string& g : utils::split(cfg["gender"])) {
		genders_.push_back(string_gender(g));
	}

	// For simplicity in other parts of the code, we must have at least one gender.
	if(genders_.empty()) {
		genders_.push_back(unit_race::MALE);
	}

	if(auto abil_cfg = cfg.optional_child("abilities")) {
		for(const auto [key, cfg] : abil_cfg->all_children_view()) {
			abilities_.emplace_back(cfg);
		}
	}

	for(const config& adv : cfg.child_range("advancement")) {
		for(const config& effect : adv.child_range("effect")) {
			auto abil_cfg = effect.optional_child("abilities");

			if(!abil_cfg || effect["apply_to"] != "new_ability") {
				continue;
			}

			for(const auto [key, cfg] : abil_cfg->all_children_view()) {
				adv_abilities_.emplace_back(cfg);
			}
		}
	}

	// Set the movement type.
	const std::string move_type = cfg["movement_type"];
	movement_type_id_ = move_type;
	const movement_type_map::const_iterator find_it = mv_types.find(move_type);

	if(find_it != mv_types.end()) {
		DBG_UT << "inheriting from movement_type '" << move_type << "'";
		movement_type_ = find_it->second;
	} else if(!move_type.empty()) {
		DBG_UT << "movement_type '" << move_type << "' not found";
	}

	// Override parts of the movement type with what is in our config.
	movement_type_.merge(cfg);

	for(const config& t : traits) {
		possible_traits_.add_child("trait", t);
	}

	if(race_ != &unit_race::null_race) {
		if(!race_->uses_global_traits()) {
			possible_traits_.clear();
		}

		if(cfg["ignore_race_traits"].to_bool()) {
			possible_traits_.clear();
		} else {
			for(const config& t : race_->additional_traits()) {
				if(alignment_ != unit_alignments::type::neutral || t["id"] != "fearless")
					possible_traits_.add_child("trait", t);
			}
		}

		if(undead_variation_.empty()) {
			undead_variation_ = race_->undead_variation();
		}
	}

	// Insert any traits that are just for this unit type
	for(const config& trait : cfg.child_range("trait")) {
		possible_traits_.add_child("trait", trait);
	}

	hide_help_ = cfg["hide_help"].to_bool();

	build_status_ = HELP_INDEXED;
}

/**
 * Load the most needed data into an empty unit_type (build to CREATE).
 * This creates the gender-specific types (if needed) and also defines how much
 * experience is needed to advance as well as what this advances to.
 */
void unit_type::build_created()
{
	// Don't build twice.
	if(CREATED <= build_status_) {
		return;
	}


	for(unsigned i = 0; i < gender_types_.size(); ++i) {
		if(gender_types_[i]) {
			gender_types_[i]->build_created();
		}
	}

	for(auto& pair : variations_) {
		pair.second.build_created();
	}


	const config& cfg = get_cfg();

	const std::string& advances_to_val = cfg["advances_to"];
	if(advances_to_val != "null" && !advances_to_val.empty()) {
		advances_to_ = utils::split(advances_to_val);
	}


	type_name_ = cfg["name"].t_str();
	variation_name_ = cfg["variation_name"].t_str();

	DBG_UT << "unit_type '" << log_id() << "' advances to : " << advances_to_val;

	experience_needed_ = cfg["experience"].to_int(500);
	cost_ = cfg["cost"].to_int(1);

	//needed by the editor.
	image_ = cfg["image"].str();
	build_status_ = CREATED;
}

/**
 * Performs a build of this to the indicated stage.
 */
void unit_type::build(BUILD_STATUS status,
		const movement_type_map& movement_types,
		const race_map& races,
		const config_array_view& traits)
{
	DBG_UT << "Building unit type " << log_id() << ", level " << status;

	switch(status) {
	case NOT_BUILT:
		// Already done in the constructor.
		return;

	case CREATED:
		// Build the basic data.
		build_created();
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
		ERR_UT << "Build of unit_type to unrecognized status (" << status << ") requested.";
		// Build as much as possible.
		build_full(movement_types, races, traits);
		return;
	}
}

const unit_type& unit_type::get_gender_unit_type(const std::string& gender) const
{
	if(gender == unit_race::s_female) {
		return get_gender_unit_type(unit_race::FEMALE);
	} else if(gender == unit_race::s_male) {
		return get_gender_unit_type(unit_race::MALE);
	}

	return *this;
}

const unit_type& unit_type::get_gender_unit_type(unit_race::GENDER gender) const
{
	const std::size_t i = gender;
	if(i < gender_types_.size() && gender_types_[i] != nullptr) {
		return *gender_types_[i];
	}

	return *this;
}

const unit_type& unit_type::get_variation(const std::string& id) const
{
	const variations_map::const_iterator i = variations_.find(id);
	if(i != variations_.end()) {
		return i->second;
	}

	return *this;
}

t_string unit_type::unit_description() const
{
	if(description_.empty()) {
		return (_("No description available."));
	} else {
		return description_;
	}
}

std::vector<t_string> unit_type::special_notes() const {
	return combine_special_notes(special_notes_, abilities_cfg(), attacks(), movement_type());
}

static void append_special_note(std::vector<t_string>& notes, const t_string& new_note) {
	if(new_note.empty()) return;
	std::string_view note_plain = new_note.c_str();
	utils::trim(note_plain);
	if(note_plain.empty()) return;
	auto iter = std::find(notes.begin(), notes.end(), new_note);
	if(iter != notes.end()) return;
	notes.push_back(new_note);
}

std::vector<t_string> combine_special_notes(const std::vector<t_string>& direct, const config& abilities, const const_attack_itors& attacks, const movetype& mt)
{
	std::vector<t_string> notes;
	for(const auto& note : direct) {
		append_special_note(notes, note);
	}
	for(const auto [key, cfg] : abilities.all_children_view()) {
		if(cfg.has_attribute("special_note")) {
			append_special_note(notes, cfg["special_note"].t_str());
		}
	}
	for(const auto& attack : attacks) {
		for(const auto [key, cfg] : attack.specials().all_children_view()) {
			if(cfg.has_attribute("special_note")) {
				append_special_note(notes, cfg["special_note"].t_str());
			}
		}
		if(auto attack_type_note = string_table.find("special_note_damage_type_" + attack.type()); attack_type_note != string_table.end()) {
			append_special_note(notes, attack_type_note->second);
		}
	}
	for(const auto& move_note : mt.special_notes()) {
		append_special_note(notes, move_note);
	}
	return notes;
}

const std::vector<unit_animation>& unit_type::animations() const
{
	if(animations_.empty()) {
		unit_animation::fill_initial_animations(animations_, get_cfg());
	}

	return animations_;
}

const_attack_itors unit_type::attacks() const
{
	if(!attacks_cache_.empty()) {
		return make_attack_itors(attacks_cache_);
	}

	for(const config& att : get_cfg().child_range("attack")) {
		attacks_cache_.emplace_back(new attack_type(att));
	}

	return make_attack_itors(attacks_cache_);
}

namespace
{
int experience_modifier = 100;
}

unit_experience_accelerator::unit_experience_accelerator(int modifier)
	: old_value_(experience_modifier)
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
		int exp = (experience_needed_ * experience_modifier + 50) / 100;
		if(exp < 1) {
			exp = 1;
		}

		return exp;
	}

	return experience_needed_;
}

bool unit_type::has_ability_by_id(const std::string& ability) const
{
	if(auto abil = get_cfg().optional_child("abilities")) {
		for(const auto [key, cfg] : abil->all_children_view()) {
			if(cfg["id"] == ability) {
				return true;
			}
		}
	}

	return false;
}

std::vector<std::string> unit_type::get_ability_list() const
{
	std::vector<std::string> res;

	auto abilities = get_cfg().optional_child("abilities");
	if(!abilities) {
		return res;
	}

	for(const auto [key, cfg] : abilities->all_children_view()) {
		std::string id = cfg["id"];

		if(!id.empty()) {
			res.push_back(std::move(id));
		}
	}

	return res;
}

bool unit_type::hide_help() const
{
	return hide_help_ || unit_types.hide_help(id_, race_->id());
}


static void advancement_tree_internal(const std::string& id, std::set<std::string>& tree)
{
	const unit_type* ut = unit_types.find(id);
	if(!ut) {
		return;
	}

	for(const std::string& adv : ut->advances_to()) {
		if(tree.insert(adv).second) {
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
	// Currently not needed (only help call us and already did it)/
	unit_types.build_all(unit_type::HELP_INDEXED);

	std::vector<std::string> adv_from;
	for(const unit_type_data::unit_type_map::value_type& ut : unit_types.types()) {
		for(const std::string& adv : ut.second.advances_to()) {
			if(adv == id_) {
				adv_from.push_back(ut.second.id());
			}
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
	for(const config& mod : possible_traits()) {
		if(mod["availability"] != "musthave") {
			continue;
		}

		for(const config& effect : mod.child_range("effect")) {
			// See if the effect only applies to
			// certain unit types But don't worry
			// about gender checks, since we don't
			// know what the gender of the
			// hypothetical recruit is.
			const std::string& ut = effect["unit_type"];

			if(!ut.empty()) {
				const std::vector<std::string>& types = utils::split(ut);

				if(std::find(types.begin(), types.end(), id()) == types.end()) {
					continue;
				}
			}

			// We're only interested in status changes.
			if(effect["apply_to"] != "status") {
				continue;
			}

			if(effect["add"] == status_name) {
				current_status = true;
			}

			if(effect["remove"] == status_name) {
				current_status = false;
			}
		}
	}

	return current_status;
}

const std::string& unit_type::flag_rgb() const
{
	return flag_rgb_.empty() ? game_config::unit_rgb : flag_rgb_;
}

bool unit_type::has_random_traits() const
{
	if(num_traits() == 0) {
		return false;
	}

	for(const auto& cfg : possible_traits()) {
		const config::attribute_value& availability = cfg["availability"];
		if(availability.blank()) {
			return true;
		}

		if(availability.str() != "musthave") {
			return true;
		}
	}

	return false;
}

std::vector<std::string> unit_type::variations() const
{
	std::vector<std::string> retval;
	retval.reserve(variations_.size());

	for(const variations_map::value_type& val : variations_) {
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
	for(const variations_map::value_type& val : variations_) {
		if(!val.second.hide_help()) {
			return true;
		}
	}

	return false;
}

int unit_type::resistance_against(const std::string& damage_name, bool attacker) const
{
	int resistance = movement_type_.resistance_against(damage_name);
	unit_ability_list resistance_abilities;

	if(auto abilities = get_cfg().optional_child("abilities")) {
		for(const config& cfg : abilities->child_range("resistance")) {
			if(!cfg["affect_self"].to_bool(true)) {
				continue;
			}

			if(!resistance_filter_matches(cfg, attacker, damage_name, 100 - resistance)) {
				continue;
			}

			resistance_abilities.emplace_back(&cfg, map_location::null_location(), map_location::null_location());
		}
	}

	if(!resistance_abilities.empty()) {
		unit_abilities::effect resist_effect(resistance_abilities, 100 - resistance);

		resistance = 100 - std::min<int>(
			resist_effect.get_composite_value(),
			resistance_abilities.highest("max_value").first
		);
	}

	return resistance;
}

bool unit_type::resistance_filter_matches(
		const config& cfg, bool attacker, const std::string& damage_name, int res) const
{
	if(!(cfg["active_on"].empty() ||
		(attacker  && cfg["active_on"] == "offense") ||
		(!attacker && cfg["active_on"] == "defense"))
	) {
		return false;
	}

	const std::string& apply_to = cfg["apply_to"];

	if(!apply_to.empty()) {
		if(damage_name != apply_to) {
			if(apply_to.find(',') != std::string::npos && apply_to.find(damage_name) != std::string::npos) {
				const std::vector<std::string>& vals = utils::split(apply_to);

				if(std::find(vals.begin(), vals.end(), damage_name) == vals.end()) {
					return false;
				}
			} else {
				return false;
			}
		}
	}

	if(!unit_abilities::filter_base_matches(cfg, res)) {
		return false;
	}

	return true;
}

/** Implementation detail of unit_type::alignment_description */

std::string unit_type::alignment_description(unit_alignments::type align, unit_race::GENDER gender)
{
	if(gender == unit_race::FEMALE) {
		switch(align)
		{
			case unit_alignments::type::lawful:
				return _("female^lawful");
			case unit_alignments::type::neutral:
				return _("female^neutral");
			case unit_alignments::type::chaotic:
				return _("female^chaotic");
			case unit_alignments::type::liminal:
				return _("female^liminal");
			default:
				return _("female^lawful");
		}
	} else {
		switch(align)
		{
			case unit_alignments::type::lawful:
				return _("lawful");
			case unit_alignments::type::neutral:
				return _("neutral");
			case unit_alignments::type::chaotic:
				return _("chaotic");
			case unit_alignments::type::liminal:
				return _("liminal");
			default:
				return _("lawful");
		}
	}
}

/* ** unit_type_data ** */

unit_type_data::unit_type_data()
	: types_()
	, movement_types_()
	, races_()
	, hide_help_all_(false)
	, hide_help_type_()
	, hide_help_race_()
	, units_cfg_()
	, build_status_(unit_type::NOT_BUILT)
{
}


// Helpers for set_config()

namespace
{
/**
 * Spits out an error message and throws a config::error.
 * Called when apply_base_unit() detects a cycle.
 * (This exists merely to take the error message out of that function.)
 */
void throw_base_unit_recursion_error(const std::vector<std::string>& base_tree, const std::string& base_id)
{
	std::stringstream ss;
	ss << "[base_unit] recursion loop in [unit_type] ";

	for(const std::string& step : base_tree) {
		ss << step << "->";
	}

	ss << base_id;
	ERR_CF << ss.str();

	throw config::error(ss.str());
}

/**
 * Insert a new value into a movetype, possibly calculating the value based on
 * the existing values in the target movetype.
 */
void patch_movetype(movetype& mt,
	const std::string& type_to_patch,
	const std::string& new_key,
	const std::string& formula_str,
	int default_val,
	bool replace)
{
	LOG_CONFIG << "Patching " << new_key << " into movetype." << type_to_patch;
	config mt_cfg;
	mt.write(mt_cfg, false);

	if(!replace && mt_cfg.child_or_empty(type_to_patch).has_attribute(new_key)) {
		// Don't replace if this type already exists in the config
		return;
	}

	// Make movement_costs.flat, defense.castle, etc available to the formula.
	// The formula uses config_callables which take references to data in mt;
	// the enclosing scope is to run all the destructors before calling mt's
	// non-const merge() function. Probably unnecessary, but I'd rather write
	// it this way than debug it afterwards.
	config temp_cfg;
	{
		// Instances of wfl::config_callable take a reference to a config,
		// which means that the "cumulative_values" variable below needs to be
		// copied so that movement costs aren't overwritten by vision costs
		// before the formula is evaluated.
		std::list<config> config_copies;

		gui2::typed_formula<int> formula(formula_str, default_val);
		wfl::map_formula_callable original;

		// These three need to follow movetype's fallback system, where values for
		// movement costs are used for vision too.
		const std::array fallback_children {"movement_costs", "vision_costs", "jamming_costs"};
		config cumulative_values;
		for(const auto& x : fallback_children) {
			if(mt_cfg.has_child(x)) {
				cumulative_values.merge_with(mt_cfg.mandatory_child(x));
			}
			config_copies.emplace_back(cumulative_values);
			auto val = std::make_shared<wfl::config_callable>(config_copies.back());
			original.add(x, val);

			// Allow "flat" to work as "vision_costs.flat" when patching vision_costs, etc
			if(type_to_patch == x) {
				original.set_fallback(val);
			}
		}

		// These don't need the fallback system
		const std::array child_names {"defense", "resistance"};
		for(const auto& x : child_names) {
			if(mt_cfg.has_child(x)) {
				const auto& subtag = mt_cfg.mandatory_child(x);
				auto val = std::make_shared<wfl::config_callable>(subtag);
				original.add(x, val);

				// Allow "arcane" to work as well as "resistance.arcane", etc
				if(type_to_patch == x) {
					original.set_fallback(val);
				}
			}
		}

		LOG_CONFIG << " formula=" << formula_str << ", resolves to " << formula(original);
		temp_cfg[new_key] = formula(original);
	}
	mt.merge(temp_cfg, type_to_patch, true);
}
} // unnamed namespace

/**
 * Modifies the provided config by merging all base units into it.
 * The @a base_tree parameter is used for detecting and reporting
 * cycles of base units and in particular to prevent infinite loops.
 */

void unit_type_data::apply_base_unit(unit_type& type, std::vector<std::string>& base_tree)
{
	// Nothing to do.
	if(type.base_unit_id_.empty()) {
		return;
	}

	// Detect recursion so the WML author is made aware of an error.
	if(std::find(base_tree.begin(), base_tree.end(), type.base_unit_id_) != base_tree.end()) {
		throw_base_unit_recursion_error(base_tree, type.base_unit_id_);
	}

	// Find the base unit.
	const unit_type_map::iterator itor = types_.find(type.base_unit_id_);
	if(itor != types_.end()) {

		unit_type& base_type = itor->second;

		// Make sure the base unit has had its base units accounted for.
		base_tree.push_back(type.base_unit_id_);

		apply_base_unit(base_type, base_tree);

		base_tree.pop_back();

		// Merge the base unit "under" our config.
		type.writable_cfg().inherit_from(base_type.get_cfg());
	}
	else {
		ERR_CF << "[base_unit]: unit type not found: " << type.base_unit_id_;
		throw config::error("unit type not found: " + type.base_unit_id_);
	}
}

/**
 * Handles inheritance for configs of [male], [female], and [variation].
 * Also removes gendered children, as those serve no purpose.
 * @a default_inherit is the default value for inherit=.
 */
std::unique_ptr<unit_type> unit_type::create_sub_type(const config& var_cfg, bool default_inherit)
{
	config var_copy =  var_cfg;
	if(var_cfg["inherit"].to_bool(default_inherit)) {
		var_copy.inherit_from(get_cfg());
	}

	var_copy.clear_children("male");
	var_copy.clear_children("female");

	return std::make_unique<unit_type>(std::move(var_copy), parent_id());
}

/**
 * Processes [variation] tags of @a ut_cfg, handling inheritance and
 * child clearing.
 */
void unit_type::fill_variations()
{
	// Most unit types do not have variations.
	if(!get_cfg().has_child("variation")) {
		return;
	}

	// Handle each variation's inheritance.
	for(const config& var_cfg : get_cfg().child_range("variation")) {

		std::unique_ptr<unit_type> var = create_sub_type(var_cfg, false);

		var->built_cfg_->remove_children("variation");
		var->variation_id_ = var_cfg["variation_id"].str();
		var->debug_id_ = debug_id_ + " [" + var->variation_id_ + "]";

		variations_map::iterator ut;
		bool success;
		std::tie(ut, success) = variations_.emplace(var_cfg["variation_id"].str(), std::move(*var));
		if(!success) {
			ERR_CF << "Skipping duplicate unit variation ID: '" << var_cfg["variation_id"]
				<< "' of unit_type '" << get_cfg()["id"] << "'";
		}
	}


}


void unit_type::fill_variations_and_gender()
{
	// Complete the gender-specific children of the config.
	if(auto male_cfg = get_cfg().optional_child("male")) {
		gender_types_[0] = create_sub_type(*male_cfg, true);
		gender_types_[0]->fill_variations();
	}

	if(auto female_cfg = get_cfg().optional_child("female")) {
		gender_types_[1] = create_sub_type(*female_cfg, true);
		gender_types_[1]->fill_variations();
	}

	// Complete the variation-defining children of the config.
	fill_variations();

	gui2::dialogs::loading_screen::progress();
}
/**
 * Resets all data based on the provided config.
 * This includes some processing of the config, such as expanding base units.
 * A pointer to the config is stored, so the config must be persistent.
 */
void unit_type_data::set_config(const game_config_view& cfg)
{
	LOG_UT << "unit_type_data::set_config, nunits: " << cfg.child_range("unit_type").size();

	clear();
	units_cfg_ = cfg;

	for(const config& mt : cfg.child_range("movetype")) {
		movement_types_.emplace(mt["name"].str(), movetype(mt));

		gui2::dialogs::loading_screen::progress();
	}

	for(const config& r : cfg.child_range("race")) {
		const unit_race race(r);
		races_.emplace(race.id(), race);

		gui2::dialogs::loading_screen::progress();
	}

	// Movetype resistance patching
	DBG_CF << "Start of movetype patching";
	for(const config& r : cfg.child_range("resistance_defaults")) {
		const std::string& dmg_type = r["id"];

		for(const auto& [mt, value] : r.attribute_range()) {
			if(mt == "id" || mt == "default" || movement_types_.find(mt) == movement_types_.end()) {
				continue;
			}

			DBG_CF << "Patching specific movetype " << mt;
			patch_movetype(movement_types_[mt], "resistance", dmg_type, value, 100, true);
		}

		if(r.has_attribute("default")) {
			for(movement_type_map::value_type& mt : movement_types_) {
				// Don't apply a default if a value is explicitly specified.
				if(r.has_attribute(mt.first)) {
					continue;
				}
				// The "none" movetype expects everything to have the default value (to be UNREACHABLE)
				if(mt.first == "none") {
					continue;
				}

				patch_movetype(mt.second, "resistance", dmg_type, r["default"], 100, false);
			}
		}
	}
	DBG_CF << "Split between resistance and cost patching";

	// Movetype move/defend patching
	for(const config& terrain : cfg.child_range("terrain_defaults")) {
		const std::string& ter_type = terrain["id"];

		struct ter_defs_to_movetype
		{
			/** The data to read from is in [terrain_defaults][subtag], and corresponds to [movetype][subtag] */
			std::string subtag;
			/** Deprecated names used in 1.14.0's [terrain_defaults]. For [defense] the name didn't change. */
			std::string alias;
			int default_val;
		};
		const std::array terrain_info_tags{
			ter_defs_to_movetype{{"movement_costs"}, {"movement"}, movetype::UNREACHABLE},
			ter_defs_to_movetype{{"vision_costs"}, {"vision"}, movetype::UNREACHABLE},
			ter_defs_to_movetype{{"jamming_costs"}, {"jamming"}, movetype::UNREACHABLE},
			ter_defs_to_movetype{{"defense"}, {"defense"}, 100}
		};

		for(const auto& cost_type : terrain_info_tags) {
			const std::string* src_tag = nullptr;
			if(terrain.has_child(cost_type.subtag)) {
				src_tag = &cost_type.subtag;
			}
			else if(terrain.has_child(cost_type.alias)) {
				// Check for the deprecated name, no deprecation warnings are printed.
				src_tag = &cost_type.alias;
			}
			if(!src_tag) {
				continue;
			}

			const config& info = terrain.mandatory_child(*src_tag);

			for(const auto& [mt, value] : info.attribute_range()) {
				if(mt == "default" || movement_types_.find(mt) == movement_types_.end()) {
					continue;
				}

				patch_movetype(
					movement_types_[mt], cost_type.subtag, ter_type, value, cost_type.default_val, true);
			}

			if(info.has_attribute("default")) {
				for(movement_type_map::value_type& mt : movement_types_) {
					// Don't apply a default if a value is explicitly specified.
					if(info.has_attribute(mt.first)) {
						continue;
					}
					// The "none" movetype expects everything to have the default value
					if(mt.first == "none") {
						continue;
					}

					patch_movetype(
						mt.second, cost_type.subtag, ter_type, info["default"], cost_type.default_val, false);
				}
			}
		}
	}
	DBG_CF << "End of movetype patching";

	for(const config& ut : cfg.child_range("unit_type")) {
		// Every type is required to have an id.
		std::string id = ut["id"].str();
		if(id.empty()) {
			ERR_CF << "[unit_type] with empty id=, ignoring:\n" << ut.debug();
			continue;
		}

		if(types_.emplace(id, unit_type(ut)).second) {
			LOG_CONFIG << "added " << id << " to unit_type list (unit_type_data.unit_types)";
		} else {
			ERR_CF << "Multiple [unit_type]s with id=" << id << " encountered.";
		}
	}

	// Apply base units.
	for(auto& type : types_) {
		std::vector<std::string> base_tree(1, type.second.id());
		apply_base_unit(type.second, base_tree);

		gui2::dialogs::loading_screen::progress();
	}

	//handle [male], [female], [variation]
	for(auto& type : types_) {
		type.second.fill_variations_and_gender();

		gui2::dialogs::loading_screen::progress();
	}

	// Build all unit types. (This was not done within the loop for performance.)
	build_all(unit_type::CREATED);

	// Suppress some unit types (presumably used as base units) from the help.
	if(auto hide_help = cfg.optional_child("hide_help")) {
		hide_help_all_ = hide_help["all"].to_bool();
		read_hide_help(*hide_help);
	}
	DBG_UT << "Finished creating unit types";
}

void unit_type_data::build_unit_type(const unit_type & ut, unit_type::BUILD_STATUS status) const
{
	ut.build(status, movement_types_, races_, units_cfg().child_range("trait"));
}

/**
 * Finds a unit_type by its id() and makes sure it is built to the specified level.
 */
const unit_type* unit_type_data::find(const std::string& key, unit_type::BUILD_STATUS status) const
{
	if(key.empty() || key == "random") {
		return nullptr;
	}

	DBG_CF << "trying to find " << key << " in unit_type list (unit_type_data.unit_types)";
	const unit_type_map::iterator itor = types_.find(key);

	// This might happen if units of another era are requested (for example for savegames)
	if(itor == types_.end()) {
		DBG_CF << "unable to find " << key << " in unit_type list (unit_type_data.unit_types)";
		return nullptr;
	}

	// Make sure the unit_type is built to the requested level.
	build_unit_type(itor->second, status);

	return &itor->second;
}

void unit_type_data::check_types(const std::vector<std::string>& types) const
{
	for(const std::string& type : types) {
		if(!find(type)) {
			throw game::game_error("unknown unit type: " + type);
		}
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
	if(status <= build_status_) {
		return;
	}

	for(const auto& type : types_) {
		build_unit_type(type.second, status);

		gui2::dialogs::loading_screen::progress();
	}

	build_status_ = status;
}

void unit_type_data::read_hide_help(const config& cfg)
{
	hide_help_race_.emplace_back();
	hide_help_type_.emplace_back();

	std::vector<std::string> races = utils::split(cfg["race"]);
	hide_help_race_.back().insert(races.begin(), races.end());

	std::vector<std::string> types = utils::split(cfg["type"]);
	hide_help_type_.back().insert(types.begin(), types.end());

	std::vector<std::string> trees = utils::split(cfg["type_adv_tree"]);
	hide_help_type_.back().insert(trees.begin(), trees.end());

	for(const std::string& t_id : trees) {
		unit_type_map::iterator ut = types_.find(t_id);

		if(ut != types_.end()) {
			std::set<std::string> adv_tree = ut->second.advancement_tree();
			hide_help_type_.back().insert(adv_tree.begin(), adv_tree.end());
		}
	}

	// We recursively call all the imbricated [not] tags
	if(auto cfgnot = cfg.optional_child("not")) {
		read_hide_help(*cfgnot);
	}
}

bool unit_type_data::hide_help(const std::string& type, const std::string& race) const
{
	bool res = hide_help_all_;
	int lvl = hide_help_all_ ? 1 : 0; // first level is covered by 'all=yes'

	// supposed to be equal but let's be cautious
	int lvl_nb = std::min(hide_help_race_.size(), hide_help_type_.size());

	for(; lvl < lvl_nb; ++lvl) {
		if(hide_help_race_[lvl].count(race) || hide_help_type_[lvl].count(type)) {
			res = !res; // each level is a [not]
		}
	}

	return res;
}

const unit_race* unit_type_data::find_race(const std::string& key) const
{
	race_map::const_iterator i = races_.find(key);
	return i != races_.end() ? &i->second : nullptr;
}

void unit_type::apply_scenario_fix(const config& cfg)
{
	build_created();
	if(auto p_setxp = cfg.get("set_experience")) {
		experience_needed_ = p_setxp->to_int();
	}
	if(auto attr = cfg.get("set_advances_to")) {
		advances_to_ = utils::split(attr->str());
	}
	if(auto attr = cfg.get("set_cost")) {
		cost_ = attr->to_int(1);
	}
	if(auto attr = cfg.get("add_advancement")) {
		for(const auto& str : utils::split(attr->str())) {
			if(!utils::contains(advances_to_, str)) {
				advances_to_.push_back(str);
			}
		}
	}
	if(auto attr = cfg.get("remove_advancement")) {
		for(const auto& str : utils::split(attr->str())) {
			boost::remove_erase(advances_to_, str);
		}
	}

	if(cfg.has_child("advancement")) {
		advancements_ = cfg.child_range("advancement");
	}

	// apply recursively to subtypes.
	for(int gender = 0; gender <= 1; ++gender) {
		if(!gender_types_[gender]) {
			continue;
		}
		gender_types_[gender]->apply_scenario_fix(cfg);
		std::string gender_str = gender == 0 ? "male" : "female";
		if(cfg.has_child(gender_str)) {
			auto gender_cfg = cfg.optional_child(gender_str);
			if(gender_cfg){
				gender_types_[gender]->apply_scenario_fix(*gender_cfg);
			}
		}
	}

	if(get_cfg().has_child("variation")) {
		// Make sure the variations are created.
		unit_types.build_unit_type(*this, VARIATIONS);
		for (auto& cv : cfg.child_range("variation")){
			for(auto& v : variations_) {
				if(v.first == cv["variation_id"]){
					v.second.apply_scenario_fix(cv);
				}
			}
		}
	}
}

void unit_type_data::apply_scenario_fix(const config& cfg)
{
	unit_type_map::iterator itor = types_.find(cfg["type"].str());
	// This might happen if units of another era are requested (for example for savegames)
	if(itor != types_.end()) {
		itor->second.apply_scenario_fix(cfg);
	}
	else {
		// should we give an error message?
	}
}

void unit_type::remove_scenario_fixes()
{
	advances_to_.clear();
	const std::string& advances_to_val = get_cfg()["advances_to"];
	if(advances_to_val != "null" && !advances_to_val.empty()) {
		advances_to_ = utils::split(advances_to_val);
	}
	experience_needed_ = get_cfg()["experience"].to_int(500);
	cost_ = get_cfg()["cost"].to_int(1);

	// apply recursively to subtypes.
	for(int gender = 0; gender <= 1; ++gender) {
		if(!gender_types_[gender]) {
			continue;
		}
		gender_types_[gender]->remove_scenario_fixes();
	}
	for(auto& v : variations_) {
		v.second.remove_scenario_fixes();
	}
	advancements_ = get_cfg().child_range("advancement");
}

void unit_type_data::remove_scenario_fixes()
{
	for(auto& pair : types_) {
		pair.second.remove_scenario_fixes();
	}
}

void unit_type::check_id(std::string& id)
{
	assert(!id.empty());

	// We don't allow leading whitepaces.
	if(id[0] == ' ') {
		throw error("Found unit type id with a leading whitespace \"" + id + "\"");
	}

	if(id == "random" || id == "null") {
		throw error("Found unit type using a 'random' or 'null' as an id");
	}

	bool gave_warning = false;

	for(std::size_t pos = 0; pos < id.size(); ++pos) {
		const char c = id[pos];
		const bool valid = std::isalnum(c, std::locale::classic()) || c == '_' || c == ' ';

		if(!valid) {
			if(!gave_warning) {
				ERR_UT << "Found unit type id with invalid characters: \"" << id << "\"";
				gave_warning = true;
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
		if(!image::exists(profile)) {
			profile.replace(profile.find(path_adjust), path_adjust.length(), "");
		}

		return;
	}

	// else, check for the file with /transparent appended...
	offset != std::string::npos ? temp.insert(offset, path_adjust) : temp = path_adjust + temp;

	// and use that path if it exists.
	if(image::exists(temp)) {
		profile = temp;
	}
}
