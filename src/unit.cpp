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
 *  Routines to manage units.
 */

#include "unit.hpp"

#include "actions/move.hpp"
#include "callable_objects.hpp"
#include "formula.hpp"
#include "game_display.hpp"
#include "game_events/handlers.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "halo.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "unit_id.hpp"
#include "unit_abilities.hpp"
#include "terrain_filter.hpp"
#include "formula_string_utils.hpp"
#include "random_new.hpp"
#include "scripting/lua.hpp"
#include "side_filter.hpp"
#include "play_controller.hpp"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

static lg::log_domain log_unit("unit");
#define DBG_UT LOG_STREAM(debug, log_unit)
#define LOG_UT LOG_STREAM(info, log_unit)
#define WRN_UT LOG_STREAM(warn, log_unit)
#define ERR_UT LOG_STREAM(err, log_unit)

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)

static lg::log_domain log_config("config");
#define WRN_CF LOG_STREAM(warn, log_config)
#define ERR_CONFIG LOG_STREAM(err, log_config)

static lg::log_domain log_enginerefac("enginerefac");
#define LOG_RG LOG_STREAM(info, log_enginerefac)

namespace {
	const std::string ModificationTypes[] = { "advance", "trait", "object" };
	const size_t NumModificationTypes = sizeof(ModificationTypes)/
										sizeof(*ModificationTypes);

	/**
	 * Pointers to units which have data in their internal caches. The
	 * destructor of an unit removes itself from the cache, so the pointers are
	 * always valid.
	 */
	static std::vector<const unit *> units_with_cache;

	const std::string leader_crown_path = "misc/leader-crown.png";
}

/**
 * Converts a string ID to a unit_type.
 * Throws a game_error exception if the string does not correspond to a type.
 */
static const unit_type &get_unit_type(const std::string &type_id)
{
	if ( type_id.empty() )
		throw game::game_error("creating unit with an empty type field");

	const unit_type *i = unit_types.find(type_id);
	if (!i) throw game::game_error("unknown unit type: " + type_id);
	return *i;
}

static unit_race::GENDER generate_gender(const unit_type & type, bool random_gender)
{
	const std::vector<unit_race::GENDER>& genders = type.genders();
	assert( genders.size() > 0 );

	if ( random_gender == false  ||  genders.size() == 1 ) {
		return genders.front();
	} else {
		int random = random_new::generator->next_random();
		return genders[random % genders.size()];
		// Note: genders is guaranteed to be non-empty, so this is not a
		// potential division by zero.
		// Note: Whoever wrote this code, you should have used an assertion, to save others hours of work...
		// If the assertion size>0 is failing for you, one possible cause is that you are constructing a unit
		// from a unit type which has not been ``built'' using the unit_type_data methods.
	}
}

static unit_race::GENDER generate_gender(const unit_type & u_type, const config &cfg)
{
	const std::string& gender = cfg["gender"];
	if(!gender.empty())
		return string_gender(gender);

	return generate_gender(u_type, cfg["random_gender"].to_bool());
}

const std::string& unit::leader_crown()
{
	return leader_crown_path;
}

// Copy constructor
unit::unit(const unit& o):
           cfg_(o.cfg_),
           loc_(o.loc_),
           advances_to_(o.advances_to_),
           type_(o.type_),
           type_name_(o.type_name_),
           race_(o.race_),
           id_(o.id_),
           name_(o.name_),
           underlying_id_(o.underlying_id_),
           undead_variation_(o.undead_variation_),
           variation_(o.variation_),

           hit_points_(o.hit_points_),
           max_hit_points_(o.max_hit_points_),
           experience_(o.experience_),
           max_experience_(o.max_experience_),
           level_(o.level_),
           recall_cost_(o.recall_cost_),
           canrecruit_(o.canrecruit_),
           recruit_list_(o.recruit_list_),
           alignment_(o.alignment_),
           flag_rgb_(o.flag_rgb_),
           image_mods_(o.image_mods_),

           unrenamable_(o.unrenamable_),
           side_(o.side_),
           gender_(o.gender_),

           alpha_(o.alpha_),

           unit_formula_(o.unit_formula_),
           unit_loop_formula_(o.unit_loop_formula_),
           unit_priority_formula_(o.unit_priority_formula_),
           formula_vars_(o.formula_vars_ ? new game_logic::map_formula_callable(*o.formula_vars_) : o.formula_vars_),

           movement_(o.movement_),
           max_movement_(o.max_movement_),
           vision_(o.vision_),
           jamming_(o.jamming_),
           movement_type_(o.movement_type_),
           hold_position_(o.hold_position_),
           end_turn_(o.end_turn_),
           resting_(o.resting_),
           attacks_left_(o.attacks_left_),
           max_attacks_(o.max_attacks_),

           states_(o.states_),
           known_boolean_states_(o.known_boolean_states_),
           variables_(o.variables_),
           events_(o.events_),
           filter_recall_(o.filter_recall_),
           emit_zoc_(o.emit_zoc_),
           state_(o.state_),

           overlays_(o.overlays_),

           role_(o.role_),
           attacks_(o.attacks_),
           facing_(o.facing_),

           trait_names_(o.trait_names_),
           trait_descriptions_(o.trait_descriptions_),
           unit_value_(o.unit_value_),
           goto_(o.goto_),
           interrupted_move_(o.interrupted_move_),
           is_fearless_(o.is_fearless_),
           is_healthy_(o.is_healthy_),

           modification_descriptions_(o.modification_descriptions_),

           animations_(o.animations_),

           anim_(NULL),
		   next_idling_(0),

           frame_begin_time_(o.frame_begin_time_),
           unit_halo_(halo::NO_HALO),
           getsHit_(o.getsHit_),
           refreshing_(o.refreshing_),
           hidden_(o.hidden_),
           draw_bars_(o.draw_bars_),
           hp_bar_scaling_(o.hp_bar_scaling_),
           xp_bar_scaling_(o.xp_bar_scaling_),

           modifications_(o.modifications_),
		   invisibility_cache_()
{
}

unit::unit(const config &cfg, bool use_traits, const vconfig* vcfg) :
	cfg_(),
	loc_(cfg["x"] - 1, cfg["y"] - 1),
	advances_to_(),
	type_(&get_unit_type(cfg["parent_type"].blank() ? cfg["type"] : cfg["parent_type"])),
	type_name_(),
	race_(&unit_race::null_race),
	id_(cfg["id"]),
	name_(cfg["name"].t_str()),
	underlying_id_(0),
	undead_variation_(),
	variation_(cfg["variation"].empty() ? type_->default_variation() : cfg["variation"]),
	hit_points_(1),
	max_hit_points_(0),
	experience_(0),
	max_experience_(0),
	level_(0),
	recall_cost_(-1),
	canrecruit_(cfg["canrecruit"].to_bool()),
	recruit_list_(),
	alignment_(),
	flag_rgb_(),
	image_mods_(),
	unrenamable_(false),
	side_(0),
	gender_(generate_gender(*type_, cfg)),
	alpha_(),
	unit_formula_(),
	unit_loop_formula_(),
	unit_priority_formula_(),
	formula_vars_(),
	movement_(0),
	max_movement_(0),
	vision_(-1),
	jamming_(0),
	movement_type_(),
	hold_position_(false),
	end_turn_(false),
	resting_(false),
	attacks_left_(0),
	max_attacks_(0),
	states_(),
	known_boolean_states_(known_boolean_state_names_.size(),false),
	variables_(),
	events_(),
	filter_recall_(),
	emit_zoc_(0),
	state_(STATE_STANDING),
	overlays_(),
	role_(cfg["role"]),
	attacks_(),
	facing_(map_location::NDIRECTIONS),
	trait_names_(),
	trait_descriptions_(),
	unit_value_(),
	goto_(),
	interrupted_move_(),
	is_fearless_(false),
	is_healthy_(false),
	modification_descriptions_(),
	animations_(),
	anim_(NULL),
	next_idling_(0),
	frame_begin_time_(0),
	unit_halo_(halo::NO_HALO),
	getsHit_(0),
	refreshing_(false),
	hidden_(false),
	draw_bars_(false),
	hp_bar_scaling_(cfg["hp_bar_scaling"].blank() ? type_->hp_bar_scaling() : cfg["hp_bar_scaling"]),
	xp_bar_scaling_(cfg["xp_bar_scaling"].blank() ? type_->xp_bar_scaling() : cfg["xp_bar_scaling"]),
	modifications_(),
	invisibility_cache_()
{
	side_ = cfg["side"];
	if(side_ <= 0) {
		side_ = 1;
	}

	validate_side(side_);

	underlying_id_ = cfg["underlying_id"];
	set_underlying_id();

	overlays_ = utils::parenthetical_split(cfg["overlays"], ',');
	if(overlays_.size() == 1 && overlays_.front() == "") {
		overlays_.clear();
	}
	if (const config &variables = cfg.child("variables")) {
		variables_ = variables;
	}

	if(vcfg) {
		const vconfig& filter_recall = vcfg->child("filter_recall");
		if(!filter_recall.null())
			filter_recall_ = filter_recall.get_config();

		const vconfig::child_list& events = vcfg->get_children("event");
		BOOST_FOREACH(const vconfig& e, events) {
			events_.add_child("event", e.get_config());
		}
	}
	else
	{
		filter_recall_ = cfg.child_or_empty("filter_recall");

		BOOST_FOREACH(const config& unit_event, cfg.child_range("event")) {
			events_.add_child("event", unit_event);
		}
	}
	game_events::add_events(events_.child_range("event"));


	facing_ = map_location::parse_direction(cfg["facing"]);
	if(facing_ == map_location::NDIRECTIONS) facing_ = static_cast<map_location::DIRECTION>(rand()%map_location::NDIRECTIONS);

	if (const config &mods = cfg.child("modifications")) {
		modifications_ = mods;
	}

	// Apply the unit type's data to this unit.
	advance_to(cfg, *type_, use_traits);

	if (const config::attribute_value *v = cfg.get("race")) {
		if (const unit_race *r = unit_types.find_race(*v)) {
			race_ = r;
		} else {
			race_ = &unit_race::null_race;
		}
	}
	level_ = cfg["level"].to_int(level_);
	if (const config::attribute_value *v = cfg.get("undead_variation")) {
		undead_variation_ = v->str();
	}
	if(const config::attribute_value *v = cfg.get("max_attacks")) {
		max_attacks_ = std::max(0, v->to_int(1));
	}
	attacks_left_ = std::max(0, cfg["attacks_left"].to_int(max_attacks_));

	if (const config::attribute_value *v = cfg.get("alpha")) {
		alpha_ = lexical_cast_default<fixed_t>(*v);
	}
	if (const config::attribute_value *v = cfg.get("zoc")) {
		emit_zoc_ = v->to_bool(level_ > 0);
	}
	if (const config::attribute_value *v = cfg.get("description")) {
		cfg_["description"] = *v;
	}
	if (const config::attribute_value *v = cfg.get("cost")) {
		unit_value_ = *v;
	}
	if (const config::attribute_value *v = cfg.get("halo")) {
		clear_haloes();
		cfg_["halo"] = *v;
	}
	if (const config::attribute_value *v = cfg.get("profile")) {
		std::string big = *v, small = cfg["small_profile"];
		adjust_profile(small, big, "");
		cfg_["profile"] = big;
		cfg_["small_profile"] = small;
	}
	max_hit_points_ = std::max(1, cfg["max_hitpoints"].to_int(max_hit_points_));
	max_movement_ = std::max(0, cfg["max_moves"].to_int(max_movement_));
	max_experience_ = std::max(1, cfg["max_experience"].to_int(max_experience_));

	vision_ = cfg["vision"].to_int(vision_);

	std::vector<std::string> temp_advances = utils::split(cfg["advances_to"]);
	if(temp_advances.size() == 1 && temp_advances.front() == "null") {
		advances_to_.clear();
	}else if(temp_advances.size() >= 1 && temp_advances.front() != "") {
		advances_to_ = temp_advances;
	}

	if (const config &ai = cfg.child("ai"))
	{
		unit_formula_ = ai["formula"].str();
		unit_loop_formula_ = ai["loop_formula"].str();
		unit_priority_formula_ = ai["priority"].str();

		if (const config &ai_vars = ai.child("vars"))
		{
			formula_vars_ = new game_logic::map_formula_callable;

			variant var;
			BOOST_FOREACH(const config::attribute &i, ai_vars.attribute_range()) {
				var.serialize_from_string(i.second);
				formula_vars_->add(i.first, var);
			}
		} else {
			formula_vars_ = game_logic::map_formula_callable_ptr();
		}
	}

	//don't use the unit_type's attacks if this config has its own defined
	config::const_child_itors cfg_range = cfg.child_range("attack");
	if(cfg_range.first != cfg_range.second) {
		attacks_.clear();
		do {
			attacks_.push_back(attack_type(*cfg_range.first));
		} while(++cfg_range.first != cfg_range.second);
	}

	//don't use the unit_type's abilities if this config has its own defined
	cfg_range = cfg.child_range("abilities");
	if(cfg_range.first != cfg_range.second) {
		cfg_.clear_children("abilities");
		config &target = cfg_.add_child("abilities");
		do {
			target.append(*cfg_range.first);
		} while(++cfg_range.first != cfg_range.second);
	}

	// Adjust the unit's defense, movement, vision, jamming, resistances, and
	// flying status if this config has its own defined.
	movement_type_.merge(cfg);

	if (const config &status_flags = cfg.child("status"))
	{
		BOOST_FOREACH(const config::attribute &st, status_flags.attribute_range()) {
			if (st.second.to_bool()) {
				set_state(st.first, true);
			}
		}
	}
	if(cfg["ai_special"] == "guardian") {
		set_state(STATE_GUARDIAN, true);
	}

	// Remove animations from private cfg, they're not needed there now
	BOOST_FOREACH(const std::string& tag_name, unit_animation::all_tag_names()) {
		cfg_.clear_children(tag_name);
	}

	if (const config::attribute_value *v = cfg.get("hitpoints")) {
		hit_points_ = *v;
	} else {
		hit_points_ = max_hit_points_;
	}

	goto_.x = cfg["goto_x"].to_int() - 1;
	goto_.y = cfg["goto_y"].to_int() - 1;

	if (const config::attribute_value *v = cfg.get("moves")) {
		movement_ = *v;
		if(movement_ < 0) {
			attacks_left_ = 0;
			movement_ = 0;
		}
	} else {
		movement_ = max_movement_;
	}
	experience_ = cfg["experience"];
	resting_ = cfg["resting"].to_bool();
	unrenamable_ = cfg["unrenamable"].to_bool();

	/* We need to check to make sure that the cfg is not blank and if it
	isn't pull that value otherwise it goes with the default of -1.  */
	if(!cfg["recall_cost"].blank()) {
		recall_cost_ = cfg["recall_cost"].to_int(recall_cost_);
	}

	const std::string& align = cfg["alignment"];
	if(align == "lawful") {
		alignment_ = unit_type::LAWFUL;
	} else if(align == "neutral") {
		alignment_ = unit_type::NEUTRAL;
	} else if(align == "chaotic") {
		alignment_ = unit_type::CHAOTIC;
	} else if(align == "liminal") {
		alignment_ = unit_type::LIMINAL;
	} else if(align.empty()==false) {
		alignment_ = unit_type::NEUTRAL;
	}

	generate_name();

	// Make the default upkeep "full"
	if(cfg_["upkeep"].empty()) {
		cfg_["upkeep"] = "full";
	}

	set_recruits(utils::split(cfg["extra_recruit"]));

	game_config::add_color_info(cfg);

	config input_cfg;
	input_cfg.merge_attributes(cfg);

	static char const *internalized_attrs[] = { "type", "id", "name",
		"gender", "random_gender", "variation", "role", "ai_special",
		"side", "underlying_id", "overlays", "facing", "race",
		"level", "recall_cost", "undead_variation", "max_attacks",
		"attacks_left", "alpha", "zoc", "flying", "cost",
		"max_hitpoints", "max_moves", "vision", "jamming", "max_experience",
		"advances_to", "hitpoints", "goto_x", "goto_y", "moves",
		"experience", "resting", "unrenamable", "alignment",
		"canrecruit", "extra_recruit", "x", "y", "placement",
		"parent_type",
		// Useless attributes created when saving units to WML:
		"flag_rgb", "language_name" };
	BOOST_FOREACH(const char *attr, internalized_attrs) {
		input_cfg.remove_attribute(attr);
		cfg_.remove_attribute(attr);
	}

	static char const *raw_attrs[] = { "description", "halo",
		"profile", "small_profile", "upkeep", "usage", "ellipse",
		"image", "image_icon", "random_traits", "generate_name" };
	BOOST_FOREACH(const char *attr, raw_attrs) {
		input_cfg.remove_attribute(attr);
	}

	BOOST_FOREACH(const config::attribute &attr, input_cfg.attribute_range()) {
		if (attr.first == "do_not_list") continue;
		WRN_UT << "Unknown attribute '" << attr.first << "' discarded.\n";
	}

	//debug unit animations for units as they appear in game
	/*for(std::vector<unit_animation>::const_iterator i = animations_.begin(); i != animations_.end(); ++i) {
		std::cout << (*i).debug();
	}*/
}

void unit::clear_status_caches()
{
	for(std::vector<const unit *>::const_iterator itor = units_with_cache.begin();
			itor != units_with_cache.end(); ++itor) {
		(*itor)->clear_visibility_cache();
	}

	units_with_cache.clear();
}

unit::unit(const unit_type &u_type, int side, bool real_unit,
	unit_race::GENDER gender) :
	cfg_(),
	loc_(),
	advances_to_(),
	type_(&u_type),
	type_name_(),
	race_(&unit_race::null_race),
	id_(),
	name_(),
	underlying_id_(real_unit? 0: n_unit::id_manager::instance().next_fake_id()),
	undead_variation_(),
	variation_(type_->default_variation()),
	hit_points_(0),
	max_hit_points_(0),
	experience_(0),
	max_experience_(0),
	level_(0),
	recall_cost_(-1),
	canrecruit_(false),
	recruit_list_(),
	alignment_(),
	flag_rgb_(),
	image_mods_(),
	unrenamable_(false),
	side_(side),
	gender_(gender != unit_race::NUM_GENDERS ?
		gender : generate_gender(u_type, real_unit)),
	alpha_(),
	unit_formula_(),
	unit_loop_formula_(),
	unit_priority_formula_(),
	formula_vars_(),
	movement_(0),
	max_movement_(0),
	vision_(-1),
	jamming_(0),
	movement_type_(),
	hold_position_(false),
	end_turn_(false),
	resting_(false),
	attacks_left_(0),
	max_attacks_(0),
	states_(),
	known_boolean_states_( get_known_boolean_state_names().size(),false),
	variables_(),
	events_(),
	filter_recall_(),
	emit_zoc_(0),
	state_(STATE_STANDING),
	overlays_(),
	role_(),
	attacks_(),
	facing_(static_cast<map_location::DIRECTION>(rand()%map_location::NDIRECTIONS)),
	trait_names_(),
	trait_descriptions_(),
	unit_value_(),
	goto_(),
	interrupted_move_(),
	is_fearless_(false),
	is_healthy_(false),
	modification_descriptions_(),
	animations_(),
	anim_(NULL),
	next_idling_(0),
	frame_begin_time_(0),
	unit_halo_(halo::NO_HALO),
	getsHit_(0),
	refreshing_(false),
	hidden_(false),
	draw_bars_(false),
	modifications_(),
	invisibility_cache_()
{
	cfg_["upkeep"]="full";

	// Apply the unit type's data to this unit.
	advance_to(u_type, real_unit);

	if(real_unit) {
		generate_name();
	}
	set_underlying_id();

	// Set these after traits and modifications have set the maximums.
	movement_ = max_movement_;
	hit_points_ = max_hit_points_;
	attacks_left_ = max_attacks_;
}

unit::~unit()
{
	clear_haloes();

	// Remove us from the status cache
	std::vector<const unit *>::iterator itor =
	std::find(units_with_cache.begin(), units_with_cache.end(), this);

	if(itor != units_with_cache.end()) {
		units_with_cache.erase(itor);
	}
}



/**
 * Assignment operator.
 *
 * This function is unsuitable for derived classes and MUST be overridden.
 * Furthermore, derived classes must not explicitly call this version.
 *
 * The overriding function can be almost the same, except "new (this)" should
 * be followed by the derived class instead of "unit(u)".
 * (There was only one derived class when this was written, so this approach
 * might be simplest.)
 */
unit& unit::operator=(const unit& u)
{
	// Use copy constructor to make sure we are coherent
	if (this != &u) {
		this->~unit();
		new (this) unit(u) ;
	}
	return *this ;
}


void unit::generate_name()
{
	if (!name_.empty() || !cfg_["generate_name"].to_bool(true)) return;

	name_ = race_->generate_name(gender_);
	cfg_["generate_name"] = false;
}


/**
 * Apply mandatory traits (e.g. undead, mechanical) to a unit and then
 * fill out with available (leaders have a restricted set of available traits)
 * traits until no more are available or the unit has its maximum number
 * of traits.
 * This routine does not apply the effects of added traits to a unit.
 * That must be done by the caller.
 * Note that random numbers used in config files don't work in multiplayer,
 * so that leaders should be barred from all random traits until that
 * is fixed. Later the restrictions will be based on play balance.
 * @a musthaveonly is true when you don't want to generate random traits or
 * you don't want to give any optional traits to a unit.
 */
void unit::generate_traits(bool musthaveonly)
{
	LOG_UT << "Generating a trait for unit type " << type().log_id() << " with musthaveonly " << musthaveonly << "\n";
	const unit_type &u_type = type();

	// Calculate the unit's traits
	config::const_child_itors current_traits = modifications_.child_range("trait");
	std::vector<config> candidate_traits;

	BOOST_FOREACH(const config &t, u_type.possible_traits())
	{
		// Skip the trait if the unit already has it.
		const std::string &tid = t["id"];
		bool already = false;
		BOOST_FOREACH(const config &mod, current_traits)
		{
			if (mod["id"] == tid) {
				already = true;
				break;
			}
		}
		if (already) continue;

		// Add the trait if it is mandatory.
		const std::string &avl = t["availability"];
		if (avl == "musthave")
		{
			modifications_.add_child("trait", t);
			current_traits = modifications_.child_range("trait");
			continue;
		}

		// The trait is still available, mark it as a candidate for randomizing.
		// For leaders, only traits with availability "any" are considered.
		if (!musthaveonly && (!can_recruit() || avl == "any"))
			candidate_traits.push_back(t);
	}

	if (musthaveonly) return;

	// Now randomly fill out to the number of traits required or until
	// there aren't any more traits.
	int nb_traits = std::distance(current_traits.first, current_traits.second);
	int max_traits = u_type.num_traits();
	for (; nb_traits < max_traits && !candidate_traits.empty(); ++nb_traits)
	{
		int num = random_new::generator->next_random() % candidate_traits.size();
		modifications_.add_child("trait", candidate_traits[num]);
		candidate_traits.erase(candidate_traits.begin() + num);
	}

	// Once random traits are added, don't do it again.
	// Such as when restoring a saved character.
	cfg_["random_traits"] = false;
}

std::vector<std::string> unit::get_traits_list() const
{
	std::vector<std::string> res;

	BOOST_FOREACH(const config &mod, modifications_.child_range("trait"))
	{
			std::string const &id = mod["id"];
			if (!id.empty())
				res.push_back(id);
	}
	return res;
}


/**
 * Advances this unit to the specified type.
 * Experience is left unchanged.
 * Current hit point total is left unchanged unless it would violate max HP.
 * Assumes gender_ and variation_ are set to their correct values.
 */
void unit::advance_to(const config &old_cfg, const unit_type &u_type,
	bool use_traits)
{
	// For reference, the type before this advancement.
	const unit_type & old_type = type();
	// Adjust the new type for gender and variation.
	const unit_type & new_type = u_type.get_gender_unit_type(gender_).get_variation(variation_);

	// Reset the scalar values first
	trait_names_.clear();
	trait_descriptions_.clear(),
	is_fearless_ = false;
	is_healthy_ = false;

	// Clear modification-related caches
	modification_descriptions_.clear();

	// Clear the stored config and replace it with the one from the unit type,
	// except for a few attributes.
	config new_cfg;
	static char const *persistent_attrs[] = { "upkeep", "ellipse",
		"image", "image_icon", "usage", "random_traits", "generate_name" };
	BOOST_FOREACH(const char *attr, persistent_attrs) {
		if (const config::attribute_value *v = old_cfg.get(attr)) {
			new_cfg[attr] = *v;
		}
	}

	// Inherit from the new unit type.
	new_cfg.merge_with(new_type.get_cfg_for_units());

	// If unit has specific profile, remember it and keep it after advancing
	std::string profile = old_cfg["profile"].str();
	if ( !profile.empty()  &&  profile != old_type.big_profile() ) {
		new_cfg["profile"] = profile;
	} else {
		new_cfg["profile"] = new_type.big_profile();
	}
	profile = old_cfg["small_profile"].str();
	if ( !profile.empty()  &&  profile != old_type.small_profile() ) {
		new_cfg["small_profile"] = profile;
	} else {
		new_cfg["small_profile"] = new_type.small_profile();
	}

	cfg_.swap(new_cfg);
	// NOTE: There should be no need to access old_cfg (or new_cfg) after this
	//       line. Particularly since the swap might have affected old_cfg.

	advances_to_ = new_type.advances_to();

	race_ = new_type.race();
	type_ = &new_type;
	type_name_ = new_type.type_name();
	cfg_["description"] = new_type.unit_description();
	undead_variation_ = new_type.undead_variation();
	max_experience_ = new_type.experience_needed(false);
	level_ = new_type.level();
	recall_cost_ = new_type.recall_cost();
	/* Need to add a check to see if the unit's old cost is equal 
	to the unit's old unit_type cost first.  If it is change the cost
	otherwise keep the old cost. */
	if(old_type.recall_cost() == recall_cost_) {
		recall_cost_ = new_type.recall_cost();
	}
	alignment_ = new_type.alignment();
	alpha_ = new_type.alpha();
	max_hit_points_ = new_type.hitpoints();
	hp_bar_scaling_ = new_type.hp_bar_scaling();
	xp_bar_scaling_ = new_type.xp_bar_scaling();
	max_movement_ = new_type.movement();
	vision_ = new_type.vision(true);
	jamming_ = new_type.jamming();
	movement_type_ = new_type.movement_type();
	emit_zoc_ = new_type.has_zoc();
	attacks_ = new_type.attacks();
	unit_value_ = new_type.cost();

	max_attacks_ = new_type.max_attacks();

	animations_ = new_type.animations();

	flag_rgb_ = new_type.flag_rgb();


	if (cfg_["random_traits"].to_bool(true)) {
		generate_traits(!use_traits);
	} else {
		// This will add any "musthave" traits to the new unit that it doesn't already have.
		// This covers the Dark Sorcerer advancing to Lich and gaining the "undead" trait,
		// but random and/or optional traits are not added,
		// and neither are inappropriate traits removed.
		generate_traits(true);
	}

	// Apply modifications etc, refresh the unit.
	// This needs to be after type and gender are fixed,
	// since there can be filters on the modifications
	// that may result in different effects after the advancement.
	apply_modifications();

	// Now that modifications are done modifying traits, check if poison should
	// be cleared.
	if ( get_state("unpoisonable") )
		set_state(STATE_POISONED, false);

	// Now that modifications are done modifying the maximum hit points,
	// enforce this maximum.
	if ( hit_points_ > max_hit_points_ )
		hit_points_ = max_hit_points_;

	// In case the unit carries EventWML, apply it now
	game_events::add_events(cfg_.child_range("event"), new_type.id());
	cfg_.clear_children("event");

	refreshing_ = false;
        anim_.reset();
}

std::string unit::big_profile() const
{
	const std::string &prof = cfg_["profile"];
	if (!prof.empty() && prof != "unit_image") {
		return prof;
	}
	return absolute_image();
}

std::string unit::small_profile() const
{
	const std::string &prof = cfg_["small_profile"];
	if (!prof.empty() && prof != "unit_image") {
		return prof;
	}
	return absolute_image();
}

static SDL_Color hp_color_(int hitpoints, int max_hitpoints)
{
	double unit_energy = 0.0;
	SDL_Color energy_color = {0,0,0,0};

	if(max_hitpoints > 0) {
		unit_energy = double(hitpoints)/double(max_hitpoints);
	}

	if(1.0 == unit_energy){
		energy_color.r = 33;
		energy_color.g = 225;
		energy_color.b = 0;
	} else if(unit_energy > 1.0) {
		energy_color.r = 100;
		energy_color.g = 255;
		energy_color.b = 100;
	} else if(unit_energy >= 0.75) {
		energy_color.r = 170;
		energy_color.g = 255;
		energy_color.b = 0;
	} else if(unit_energy >= 0.5) {
		energy_color.r = 255;
		energy_color.g = 175;
		energy_color.b = 0;
	} else if(unit_energy >= 0.25) {
		energy_color.r = 255;
		energy_color.g = 155;
		energy_color.b = 0;
	} else {
		energy_color.r = 255;
		energy_color.g = 0;
		energy_color.b = 0;
	}
	return energy_color;
}

SDL_Color unit::hp_color() const
{
	return hp_color_(hitpoints(), max_hitpoints());
}

SDL_Color unit::hp_color(int new_hitpoints) const
{
	return hp_color_(new_hitpoints, hitpoints());
}

SDL_Color unit::xp_color() const
{
	const SDL_Color near_advance_color = {255,255,255,0};
	const SDL_Color mid_advance_color  = {150,255,255,0};
	const SDL_Color far_advance_color  = {0,205,205,0};
	const SDL_Color normal_color	  = {0,160,225,0};
	const SDL_Color near_amla_color	  = {225,0,255,0};
	const SDL_Color mid_amla_color	  = {169,30,255,0};
	const SDL_Color far_amla_color	  = {139,0,237,0};
	const SDL_Color amla_color		  = {170,0,255,0};
	const bool near_advance = max_experience() - experience() <= game_config::kill_experience;
	const bool mid_advance  = max_experience() - experience() <= game_config::kill_experience*2;
	const bool far_advance  = max_experience() - experience() <= game_config::kill_experience*3;

	SDL_Color color=normal_color;
	if(advances_to().size()){
		if(near_advance){
			color=near_advance_color;
		} else if(mid_advance){
			color=mid_advance_color;
		} else if(far_advance){
			color=far_advance_color;
		}
	} else if (get_modification_advances().size()){
		if(near_advance){
			color=near_amla_color;
		} else if(mid_advance){
			color=mid_amla_color;
		} else if(far_advance){
			color=far_amla_color;
		} else {
			color=amla_color;
		}
	}
	return(color);
}

void unit::set_recruits(const std::vector<std::string>& recruits)
{
	unit_types.check_types(recruits);
	recruit_list_ = recruits;
	//TODO crab
	//info_.minimum_recruit_price = 0;
	//ai::manager::raise_recruit_list_changed();
}

const std::vector<std::string> unit::advances_to_translated() const
{
	std::vector<std::string> result;
	BOOST_FOREACH(std::string adv_type_id, advances_to_)
	{
		const unit_type *adv_type = unit_types.find(adv_type_id);
		if ( adv_type )
			result.push_back(adv_type->type_name());
		else
			WRN_UT << "unknown unit in advances_to list of type "
			<< type().log_id() << ": " << adv_type_id << "\n";
	}
	return result;
}

void unit::set_advances_to(const std::vector<std::string>& advances_to)
{
	unit_types.check_types(advances_to);
	advances_to_ = advances_to;
}

std::string unit::side_id() const {return teams_manager::get_teams()[side()-1].save_id(); }

/**
 * Set the unit's remaining movement to @a moves.
 * If @a unit_action is set to true, then additionally the "end turn" and
 * "hold position" flags will be cleared (as they should be if a unit acts,
 * as opposed to the movement being set by the engine for other reasons).
 */
void unit::set_movement(int moves, bool unit_action)
{
	// If this was because the unit acted, clear its "not acting" flags.
	if ( unit_action )
		end_turn_ = hold_position_ = false;

	movement_ = std::max<int>(0, moves);
}


/**
 * Determines if @a mod_dur "matches" @a goal_dur.
 * If goal_dur is not empty, they match if they are equal.
 * If goal_dur is empty, they match if mod_dur is neither empty nor "forever".
 * Helper function for expire_modifications().
 */
inline bool mod_duration_match(const std::string & mod_dur,
                               const std::string & goal_dur)
{
	if ( goal_dur.empty() )
		// Default is all temporary modifications.
		return !mod_dur.empty()  &&  mod_dur != "forever";
	else
		return mod_dur == goal_dur;
}

/**
 * Clears those modifications whose duration has expired.
 * If @a duration is empty, then all temporary modifications (those not
 * lasting forever) have expired. Otherwise, modifications whose duration
 * equals @a duration have expired.
 */
void unit::expire_modifications(const std::string & duration)
{
	// If any modifications expire, then we will need to rebuild the unit.
	const unit_type * rebuild_from = NULL;

	// Loop through all types of modifications.
	for(unsigned int i = 0; i != NumModificationTypes; ++i) {
		const std::string& mod_name = ModificationTypes[i];
		// Loop through all modifications of this type.
		// Looping in reverse since we may delete the current modification.
		for (int j = modifications_.child_count(mod_name)-1; j >= 0; --j)
		{
			const config &mod = modifications_.child(mod_name, j);

			if ( mod_duration_match(mod["duration"], duration) ) {
				// If removing this mod means reverting the unit's type:
				if ( const config::attribute_value *v = mod.get("prev_type") ) {
					rebuild_from = &get_unit_type(v->str());
				}
				// Else, if we have not already specified a type to build from:
				else if ( rebuild_from == NULL )
					rebuild_from = &type();

				modifications_.remove_child(mod_name, j);
			}
		}
	}

	if ( rebuild_from != NULL ) {
		clear_haloes();
		advance_to(*rebuild_from);
	}
}


void unit::new_turn()
{
	expire_modifications("turn");

	end_turn_ = hold_position_;
	movement_ = total_movement();
	attacks_left_ = max_attacks_;
	set_state(STATE_UNCOVERED, false);
}

void unit::end_turn()
{
	set_state(STATE_SLOWED,false);
	if((movement_ != total_movement()) && !(get_state(STATE_NOT_MOVED))) {
		resting_ = false;
	}
	set_state(STATE_NOT_MOVED,false);
	// Clear interrupted move
	set_interrupted_move(map_location());
}

void unit::new_scenario()
{
	// Set the goto-command to be going to no-where
	goto_ = map_location();

	// Expire all temporary modifications.
	expire_modifications("");

	heal_all();
	set_state(STATE_SLOWED, false);
	set_state(STATE_POISONED, false);
	set_state(STATE_PETRIFIED, false);
	set_state(STATE_GUARDIAN, false);
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

const std::map<std::string,std::string> unit::get_states() const
{
	std::map<std::string, std::string> all_states;
	BOOST_FOREACH(std::string const &s, states_) {
		all_states[s] = "yes";
	}
	for (std::map<std::string, state_t>::const_iterator i = known_boolean_state_names_.begin(),
	     i_end = known_boolean_state_names_.end(); i != i_end; ++i)
	{
		if (get_state(i->second)) {
			all_states.insert(make_pair(i->first, "yes"));
		}

	}
	// Backwards compatibility for not_living. Don't remove before 1.12
	if (all_states.find("undrainable") != all_states.end() &&
		all_states.find("unpoisonable") != all_states.end() &&
		all_states.find("unplagueable") != all_states.end())
		all_states["not_living"] = "yes";
	return all_states;
}

bool unit::get_state(const std::string &state) const
{
	state_t known_boolean_state_id = get_known_boolean_state_id(state);
	if (known_boolean_state_id!=STATE_UNKNOWN){
		return get_state(known_boolean_state_id);
	}
	// Backwards compatibility for not_living. Don't remove before 1.12
	if (state == "not_living") {
		return get_state("undrainable") &&
			get_state("unpoisonable") &&
			get_state("unplagueable");
	}
	return states_.find(state) != states_.end();
}

void unit::set_state(state_t state, bool value)
{
	known_boolean_states_[state] = value;
}

bool unit::get_state(state_t state) const
{
	return known_boolean_states_[state];
}

unit::state_t unit::get_known_boolean_state_id(const std::string &state) {
	std::map<std::string, state_t>::const_iterator i = known_boolean_state_names_.find(state);
	if (i != known_boolean_state_names_.end()) {
		return i->second;
	}
	return STATE_UNKNOWN;
}

std::map<std::string, unit::state_t> unit::known_boolean_state_names_ = get_known_boolean_state_names();

std::map<std::string, unit::state_t> unit::get_known_boolean_state_names()
{
	std::map<std::string, state_t> known_boolean_state_names_map;
	known_boolean_state_names_map.insert(std::make_pair("slowed",STATE_SLOWED));
	known_boolean_state_names_map.insert(std::make_pair("poisoned",STATE_POISONED));
	known_boolean_state_names_map.insert(std::make_pair("petrified",STATE_PETRIFIED));
	known_boolean_state_names_map.insert(std::make_pair("uncovered", STATE_UNCOVERED));
	known_boolean_state_names_map.insert(std::make_pair("not_moved",STATE_NOT_MOVED));
	known_boolean_state_names_map.insert(std::make_pair("unhealable",STATE_UNHEALABLE));
	known_boolean_state_names_map.insert(std::make_pair("guardian",STATE_GUARDIAN));
	return known_boolean_state_names_map;
}

void unit::set_state(const std::string &state, bool value)
{
	state_t known_boolean_state_id = get_known_boolean_state_id(state);
	if (known_boolean_state_id != STATE_UNKNOWN) {
		set_state(known_boolean_state_id, value);
		return;
	}
	// Backwards compatibility for not_living. Don't remove before 1.12
	if (state == "not_living") {
		set_state("undrainable", value);
		set_state("unpoisonable", value);
		set_state("unplagueable", value);
	}
	if (value)
		states_.insert(state);
	else
		states_.erase(state);
}


bool unit::has_ability_by_id(const std::string& ability) const
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

void unit::remove_ability_by_id(const std::string &ability)
{
	if (config &abil = cfg_.child("abilities"))
	{
		config::all_children_iterator i = abil.ordered_begin();
		while (i != abil.ordered_end()) {
			if (i->cfg["id"] == ability) {
				i = abil.erase(i);
			} else {
				++i;
			}
		}
	}
}

bool unit::matches_filter(const vconfig& cfg, const map_location& loc, bool use_flat_tod) const
{
	bool matches = true;

	if(loc.valid()) {
		assert(resources::units != NULL);
		scoped_xy_unit auto_store("this_unit", loc.x, loc.y, *resources::units);
		matches = internal_matches_filter(cfg, loc, use_flat_tod);
	} else {
		// If loc is invalid, then this is a recall list unit (already been scoped)
		matches = internal_matches_filter(cfg, loc, use_flat_tod);
	}

	// Handle [and], [or], and [not] with in-order precedence
	vconfig::all_children_iterator cond = cfg.ordered_begin();
	vconfig::all_children_iterator cond_end = cfg.ordered_end();
	while(cond != cond_end)
	{

		const std::string& cond_name = cond.get_key();
		const vconfig& cond_filter = cond.get_child();

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

bool unit::internal_matches_filter(const vconfig& cfg, const map_location& loc, bool use_flat_tod) const
{
	config::attribute_value cfg_name = cfg["name"];
	if (!cfg_name.blank() && cfg_name.str() != name_) {
		return false;
	}

	const config::attribute_value cfg_id = cfg["id"];
	if (!cfg_id.blank()) {
		const std::string& id = cfg_id;
		const std::string& this_id = this->id();

		if (id == this_id) {
		}
		else if ( id.find(',') == std::string::npos ){
			return false;
		}
		else {
			const std::vector<std::string>& ids = utils::split(id);
			if (std::find(ids.begin(), ids.end(), this_id) == ids.end()) {
				return false;
			}
		}
	}

	// Allow 'speaker' as an alternative to id, since people use it so often
	config::attribute_value cfg_speaker = cfg["speaker"];
	if (!cfg_speaker.blank() && cfg_speaker.str() != id()) {
		return false;
	}

	if(cfg.has_child("filter_location")) {
		assert(resources::game_map != NULL);
		assert(resources::teams != NULL);
		assert(resources::tod_manager != NULL);
		assert(resources::units != NULL);
		const vconfig& t_cfg = cfg.child("filter_location");
		terrain_filter t_filter(t_cfg, *resources::units, use_flat_tod);
		if(!t_filter.match(loc)) {
			return false;
		}
	}

	const vconfig& filter_side = cfg.child("filter_side");
	if(!filter_side.null()) {
		side_filter s_filter(filter_side);
		if(!s_filter.match(this->side()))
			return false;
	}

	// Also allow filtering on location ranges outside of the location filter
	config::attribute_value cfg_x = cfg["x"];
	config::attribute_value cfg_y = cfg["y"];
	if (!cfg_x.blank() || !cfg_y.blank()){
		if(cfg_x == "recall" && cfg_y == "recall") {
			//locations on the map are considered to not be on a recall list
			if ((!resources::game_map && loc.valid()) ||
			    (resources::game_map && resources::game_map->on_board(loc)))
			{
				return false;
			}
		} else if(cfg_x.empty() && cfg_y.empty()) {
			return false;
		} else if(!loc.matches_range(cfg_x, cfg_y)) {
			return false;
		}
	}

	// The type could be a comma separated list of types
	config::attribute_value cfg_type = cfg["type"];
	if (!cfg_type.blank())
	{
		const std::string type_ids = cfg_type.str();
		const std::string& this_type = type_id();

		// We only do the full CSV search if we find a comma in there,
		// and if the subsequence is found within the main sequence.
		// This is because doing the full CSV split is expensive.
		if ( type_ids == this_type ) {
			// pass
		} else if ( type_ids.find(',') != std::string::npos  &&
		            type_ids.find(this_type) != std::string::npos ) {
			const std::vector<std::string>& vals = utils::split(type_ids);

			if(std::find(vals.begin(),vals.end(),this_type) == vals.end()) {
				return false;
			}
		} else {
			return false;
		}
	}

	// The variation_type could be a comma separated list of types
	config::attribute_value cfg_variation_type = cfg["variation"];
	if (!cfg_variation_type.blank())
	{
		const std::string type_ids = cfg_variation_type.str();
		const std::string& this_type = variation_;

		// We only do the full CSV search if we find a comma in there,
		// and if the subsequence is found within the main sequence.
		// This is because doing the full CSV split is expensive.
		if ( type_ids == this_type ) {
			// pass
		} else if ( type_ids.find(',') != std::string::npos  &&
    				type_ids.find(this_type) != std::string::npos ) {
			const std::vector<std::string>& vals = utils::split(type_ids);

			if(std::find(vals.begin(),vals.end(),this_type) == vals.end()) {
				return false;
			}
		} else {
			return false;
		}
	}

	// The has_variation_type could be a comma separated list of types
	config::attribute_value cfg_has_variation_type = cfg["has_variation"];
	if (!cfg_has_variation_type.blank())
	{
		const std::string& var_ids  = cfg_has_variation_type.str();
		const std::string& this_var = variation_;

		if ( var_ids == this_var ) {
			// pass
		} else {

			bool match = false;
			const std::vector<std::string>& variation_types = utils::split(var_ids);
			// If this unit is a variation itself then search in the base unit's variations.
			const unit_type* const type = this_var.empty() ? type_ : unit_types.find(type_->base_id());
			assert(type);

			BOOST_FOREACH(const std::string& variation_id, variation_types) {
				if (type->has_variation(variation_id)) {
					match = true;
					break;
				}
			}
			if (!match) return false;
		}
	}

	config::attribute_value cfg_ability = cfg["ability"];
	if (!cfg_ability.blank())
	{
		std::string ability = cfg_ability;
		if(has_ability_by_id(ability)) {
			// pass
		} else if ( ability.find(',') != std::string::npos ) {
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

	config::attribute_value cfg_race = cfg["race"];
	if (!cfg_race.blank()) {
		std::string race = cfg_race;

		if(race != race_->id()) {
			const std::vector<std::string>& vals = utils::split(race);
			if(std::find(vals.begin(), vals.end(), race_->id()) == vals.end()) {
				return false;
			}
		}
	}

	config::attribute_value cfg_gender = cfg["gender"];
	if (!cfg_gender.blank() && string_gender(cfg_gender) != gender()) {
		return false;
	}

	config::attribute_value cfg_side = cfg["side"];
	if (!cfg_side.blank() && cfg_side.to_int() != side()) {
		std::string side = cfg_side;
		if ( side.find(',') == std::string::npos ) {
			return false;
		}
		std::vector<std::string> vals = utils::split(side);
		if (std::find(vals.begin(), vals.end(), str_cast(side_)) == vals.end()) {
			return false;
		}
	}

	config::attribute_value cfg_has_weapon = cfg["has_weapon"];
	if (!cfg_has_weapon.blank()) {
		std::string weapon = cfg_has_weapon;
		bool has_weapon = false;
		const std::vector<attack_type>& attacks = this->attacks();
		for(std::vector<attack_type>::const_iterator i = attacks.begin();
			i != attacks.end(); ++i) {
			if(i->id() == weapon) {
				has_weapon = true;
				break;
			}
		}
		if(!has_weapon) {
			return false;
		}
	}

	config::attribute_value cfg_role = cfg["role"];
	if (!cfg_role.blank() && cfg_role.str() != role_) {
		return false;
	}

	config::attribute_value cfg_ai_special = cfg["ai_special"];
	if (!cfg_ai_special.blank() && ((cfg_ai_special.str() == "guardian")  != get_state(STATE_GUARDIAN))) {
		return false;
	}

	config::attribute_value cfg_canrecruit = cfg["canrecruit"];
	if (!cfg_canrecruit.blank() && cfg_canrecruit.to_bool() != can_recruit()) {
		return false;
	}
	
	config::attribute_value cfg_recall_cost = cfg["recall_cost"];
	if (!cfg_recall_cost.blank() && cfg_recall_cost.to_int(-1) != recall_cost_) {
		return false;
	}
	
	config::attribute_value cfg_level = cfg["level"];
	if (!cfg_level.blank() && cfg_level.to_int(-1) != level_) {
		return false;
	}

	config::attribute_value cfg_defense = cfg["defense"];
	if (!cfg_defense.blank() && cfg_defense.to_int(-1) != defense_modifier(resources::game_map->get_terrain(loc))) {
		return false;
	}

	config::attribute_value cfg_movement = cfg["movement_cost"];
	if (!cfg_movement.blank() && cfg_movement.to_int(-1) != movement_cost(resources::game_map->get_terrain(loc))) {
		return false;
	}

	// Now start with the new WML based comparison.
	// If a key is in the unit and in the filter, they should match
	// filter only => not for us
	// unit only => not filtered
	const vconfig::child_list& wmlcfgs = cfg.get_children("filter_wml");
	if (!wmlcfgs.empty()) {
		config unit_cfg;
		for (unsigned i = 0; i < wmlcfgs.size(); ++i)
		{
			config fwml = wmlcfgs[i].get_parsed_config();
			/* Check if the filter only cares about variables.
			   If so, no need to serialize the whole unit. */
			config::const_attr_itors ai = fwml.attribute_range();
			config::all_children_itors ci = fwml.all_children_range();
			if (std::distance(ai.first, ai.second) == 0 &&
			    std::distance(ci.first, ci.second) == 1 &&
			    ci.first->key == "variables") {
				if (!variables_.matches(ci.first->cfg))
					return false;
			} else {
				if (unit_cfg.empty())
					write(unit_cfg);
				if (!unit_cfg.matches(fwml))
					return false;
			}
		}
	}

	if (cfg.has_child("filter_vision")) {
		const vconfig::child_list& vis_filt = cfg.get_children("filter_vision");
		vconfig::child_list::const_iterator i, i_end = vis_filt.end();
		for (i = vis_filt.begin(); i != i_end; ++i) {
			bool visible = (*i)["visible"].to_bool(true);
			std::set<int> viewers;
			// Use standard side filter
			side_filter ssf(*i);
			std::vector<int> sides = ssf.get_teams();
			viewers.insert(sides.begin(), sides.end());
			if (viewers.empty()) {
				return false;
			}
			std::set<int>::const_iterator viewer, viewer_end = viewers.end();
			for (viewer = viewers.begin(); viewer != viewer_end; ++viewer) {
				bool fogged = teams_manager::get_teams()[*viewer - 1].fogged(loc);
				bool hiding = this->invisible(loc/*, false(?) */);
				bool unit_hidden = fogged || hiding;
				if (visible == unit_hidden) return false;
			}
		}
	}

	if (cfg.has_child("filter_adjacent")) {
		assert(resources::units && resources::game_map);
		const unit_map& units = *resources::units;
		map_location adjacent[6];
		get_adjacent_tiles(loc, adjacent);
		vconfig::child_list::const_iterator i, i_end;
		const vconfig::child_list& adj_filt = cfg.get_children("filter_adjacent");
		for (i = adj_filt.begin(), i_end = adj_filt.end(); i != i_end; ++i) {
			int match_count=0;
			static std::vector<map_location::DIRECTION> default_dirs
				= map_location::parse_directions("n,ne,se,s,sw,nw");
			config::attribute_value i_adjacent = (*i)["adjacent"];
			std::vector<map_location::DIRECTION> dirs = !i_adjacent.blank() ?
				map_location::parse_directions(i_adjacent) : default_dirs;
			std::vector<map_location::DIRECTION>::const_iterator j, j_end = dirs.end();
			for (j = dirs.begin(); j != j_end; ++j) {
				unit_map::const_iterator unit_itor = units.find(adjacent[*j]);
				if (unit_itor == units.end()
				|| !unit_itor->matches_filter(*i, unit_itor->get_location(), use_flat_tod)) {
					continue;
				}
				config::attribute_value i_is_enemy = (*i)["is_enemy"];
				if (i_is_enemy.blank() || i_is_enemy.to_bool() ==
				    teams_manager::get_teams()[this->side() - 1].is_enemy(unit_itor->side())) {
					++match_count;
				}
			}
			static std::vector<std::pair<int,int> > default_counts = utils::parse_ranges("1-6");
			config::attribute_value i_count = (*i)["count"];
			std::vector<std::pair<int,int> > counts = !i_count.blank()
				? utils::parse_ranges(i_count) : default_counts;
			if(!in_ranges(match_count, counts)) {
				return false;
			}
		}
	}

	config::attribute_value cfg_find_in = cfg["find_in"];
	if (!cfg_find_in.blank()) {
		// Allow filtering by searching a stored variable of units
		variable_info vi(cfg_find_in, false, variable_info::TYPE_CONTAINER);
		if(!vi.is_valid) return false;
		if(vi.explicit_index) {
			config::const_child_iterator i = vi.vars->child_range(vi.key).first;
			std::advance(i, vi.index);
			if ((*i)["id"] != id_) {
				return false;
			}
		} else {
			if (!vi.vars->find_child(vi.key, "id", id_))
				return false;
		}
	}
	config::attribute_value cfg_formula = cfg["formula"];
	if (!cfg_formula.blank()) {
		const unit_callable callable(std::pair<map_location, unit>(loc,*this));
		const game_logic::formula form(cfg_formula);
		if(!form.evaluate(callable).as_bool()) {///@todo use formula_ai
			return false;
		}
	}

	config::attribute_value cfg_lua_function = cfg["lua_function"];
	if (!cfg_lua_function.blank()) {
		bool b = resources::lua_kernel->run_filter(cfg_lua_function.str().c_str(), *this);
		if (!b) return false;
	}

	return true;
}

void unit::write(config& cfg) const
{
	cfg.append(cfg_);
	movement_type_.write(cfg);

	if ( cfg["description"] == type().unit_description() ) {
		cfg.remove_attribute("description");
	}

	cfg["hitpoints"] = hit_points_;
	cfg["max_hitpoints"] = max_hit_points_;

	cfg["experience"] = experience_;
	cfg["max_experience"] = max_experience_;
	cfg["recall_cost"] = recall_cost_;
	
	cfg["side"] = side_;

	cfg["type"] = type_id();
	if ( type_id() != type().base_id() )
		cfg["parent_type"] = type().base_id();

	//support for unit formulas in [ai] and unit-specific variables in [ai] [vars]

	if ( has_formula() || has_loop_formula() || (formula_vars_ && formula_vars_->empty() == false) ) {

		config &ai = cfg.add_child("ai");

		if (has_formula())
			ai["formula"] = unit_formula_;

		if (has_loop_formula())
			ai["loop_formula"] = unit_loop_formula_;

		if (has_priority_formula())
			ai["priority"] = unit_priority_formula_;


		if (formula_vars_ && formula_vars_->empty() == false)
		{
			config &ai_vars = ai.add_child("vars");

			std::string str;
			for(game_logic::map_formula_callable::const_iterator i = formula_vars_->begin(); i != formula_vars_->end(); ++i)
			{
				i->second.serialize_to_string(str);
				if (!str.empty())
				{
					ai_vars[i->first] = str;
					str.clear();
				}
			}
		}
	}

	cfg["gender"] = gender_string(gender_);
	cfg["variation"] = variation_;
	cfg["role"] = role_;

	config status_flags;
	std::map<std::string,std::string> all_states = get_states();
	for(std::map<std::string,std::string>::const_iterator st = all_states.begin(); st != all_states.end(); ++st) {
		status_flags[st->first] = st->second;
	}

	cfg.clear_children("variables");
	cfg.add_child("variables",variables_);
	cfg.clear_children("events");
	cfg.append(events_);
	cfg.clear_children("filter_recall");
	cfg.add_child("filter_recall", filter_recall_);
	cfg.clear_children("status");
	cfg.add_child("status",status_flags);

	cfg["overlays"] = utils::join(overlays_);

	cfg["name"] = name_;
	cfg["id"] = id_;
	cfg["underlying_id"] = str_cast(underlying_id_);

	if(can_recruit())
		cfg["canrecruit"] = true;

	cfg["extra_recruit"] = utils::join(recruit_list_);

	cfg["facing"] = map_location::write_direction(facing_);

	cfg["goto_x"] = goto_.x + 1;
	cfg["goto_y"] = goto_.y + 1;

	cfg["moves"] = movement_;
	cfg["max_moves"] = max_movement_;
	cfg["vision"] = vision_;
	cfg["jamming"] = jamming_;

	cfg["resting"] = resting_;

	cfg["advances_to"] = utils::join(advances_to_);

	cfg["race"] = race_->id();
	cfg["language_name"] = type_name_;
	cfg["undead_variation"] = undead_variation_;
	cfg["level"] = level_;
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
		case unit_type::LIMINAL:
			cfg["alignment"] = "liminal";
			break;
		default:
			cfg["alignment"] = "neutral";
	}
	cfg["flag_rgb"] = flag_rgb_;
	cfg["unrenamable"] = unrenamable_;
	cfg["alpha"] = str_cast(alpha_);

	cfg["attacks_left"] = attacks_left_;
	cfg["max_attacks"] = max_attacks_;
	cfg["zoc"] = emit_zoc_;
	cfg.clear_children("attack");
	for(std::vector<attack_type>::const_iterator i = attacks_.begin(); i != attacks_.end(); ++i) {
		cfg.add_child("attack",i->get_cfg());
	}
	cfg["cost"] = unit_value_;
	cfg.clear_children("modifications");
	cfg.add_child("modifications",modifications_);

}

void unit::add_formula_var(std::string str, variant var) {
	if(!formula_vars_) formula_vars_ = new game_logic::map_formula_callable;
	formula_vars_->add(str, var);
}

const surface unit::still_image(bool scaled) const
{
	image::locator image_loc;

#ifdef LOW_MEM
	image_loc = image::locator(absolute_image());
#else
	std::string mods=image_mods();
	if(!mods.empty()){
		image_loc = image::locator(absolute_image(),mods);
	} else {
		image_loc = image::locator(absolute_image());
	}
#endif

	surface unit_image(image::get_image(image_loc, scaled ? image::SCALED_TO_ZOOM : image::UNSCALED));
	return unit_image;
}

void unit::set_standing(bool with_bars)
{
	display *disp = display::get_singleton();
	if (preferences::show_standing_animations()&& !incapacitated()) {
		start_animation(INT_MAX, choose_animation(*disp, loc_, "standing"),
			with_bars,  "", 0, STATE_STANDING);
	} else {
		start_animation(INT_MAX, choose_animation(*disp, loc_, "_disabled_"),
			with_bars,  "", 0, STATE_STANDING);
	}
}

void unit::set_ghosted(bool with_bars)
{
	display *disp = display::get_singleton();
	start_animation(INT_MAX, choose_animation(*disp, loc_, "ghosted"),
			with_bars);
}

void unit::set_disabled_ghosted(bool with_bars)
{
	display *disp = display::get_singleton();
	start_animation(INT_MAX, choose_animation(*disp, loc_, "disabled_ghosted"),
			with_bars);
}

void unit::set_idling()
{
	display *disp = display::get_singleton();
	start_animation(INT_MAX, choose_animation(*disp, loc_, "idling"),
		true, "", 0, STATE_FORGET);
}

void unit::set_selecting()
{
	const display *disp =  display::get_singleton();
	if (preferences::show_standing_animations() && !get_state(STATE_PETRIFIED)) {
		start_animation(INT_MAX, choose_animation(*disp, loc_, "selected"),
			true, "", 0, STATE_FORGET);
	} else {
		start_animation(INT_MAX, choose_animation(*disp, loc_, "_disabled_selected_"),
			true, "", 0, STATE_FORGET);
	}
}

void unit::start_animation(int start_time, const unit_animation *animation,
	bool with_bars,  const std::string &text, Uint32 text_color, STATE state)
{
	const display * disp =  display::get_singleton();
	if (!animation) {
		if (state == STATE_STANDING)
			state_ = state;
		if (!anim_ && state_ != STATE_STANDING)
			set_standing(with_bars);
		return ;
	}
	state_ = state;
	// everything except standing select and idle
	bool accelerate = (state != STATE_FORGET && state != STATE_STANDING);
	draw_bars_ =  with_bars;
	anim_.reset(new unit_animation(*animation));
	const int real_start_time = start_time == INT_MAX ? anim_->get_begin_time() : start_time;
	anim_->start_animation(real_start_time, loc_, loc_.get_direction(facing_),
		 text, text_color, accelerate);
	frame_begin_time_ = anim_->get_begin_time() -1;
	if (disp->idle_anim()) {
		next_idling_ = get_current_animation_tick()
			+ static_cast<int>((20000 + rand() % 20000) * disp->idle_anim_rate());
	} else {
		next_idling_ = INT_MAX;
	}
}


void unit::set_facing(map_location::DIRECTION dir) {
	if(dir != map_location::NDIRECTIONS) {
		facing_ = dir;
	}
	// Else look at yourself (not available so continue to face the same direction)
}

void unit::redraw_unit()
{
	display &disp = *display::get_singleton();
	const gamemap &map = disp.get_map();

	if ( hidden_ || !is_visible_to_team(disp.get_teams()[disp.viewing_team()],disp.show_everything(),map) )
	{
		clear_haloes();
		if(anim_) {
			anim_->update_last_draw_time();
		}
		return;
	}

	if (!anim_) {
		set_standing();
		if (!anim_) return;
	}

	if (refreshing_) return;
	refreshing_ = true;

	anim_->update_last_draw_time();
	frame_parameters params;
	const t_translation::t_terrain terrain = map.get_terrain(loc_);
	const terrain_type& terrain_info = map.get_terrain_info(terrain);
	// do not set to 0 so we can distinguish the flying from the "not on submerge terrain"
	// instead use -1.0 (as in "negative depth", it will be ignored by rendering)
	params.submerge= is_flying() ? -1.0 : terrain_info.unit_submerge();

	if (invisible(loc_) &&
			params.highlight_ratio > 0.5) {
		params.highlight_ratio = 0.5;
	}
	if (loc_ == disp.selected_hex() && params.highlight_ratio == 1.0) {
		params.highlight_ratio = 1.5;
	}
	int height_adjust = static_cast<int>(terrain_info.unit_height_adjust() * disp.get_zoom_factor());
	if (is_flying() && height_adjust < 0) {
		height_adjust = 0;
	}
	params.y -= height_adjust;
	params.halo_y -= height_adjust;

	int red = 0,green = 0,blue = 0,tints = 0;
	double blend_ratio = 0;
	// Add future colored states here
	if(get_state(STATE_POISONED)) {
		green += 255;
		blend_ratio += 0.25;
		tints += 1;
	}
	if(get_state(STATE_SLOWED)) {
		red += 191;
		green += 191;
		blue += 255;
		blend_ratio += 0.25;
		tints += 1;
	}
	if(tints > 0) {
		params.blend_with = disp.rgb((red/tints),(green/tints),(blue/tints));
		params.blend_ratio = ((blend_ratio/tints));
	}

	//hackish : see unit_frame::merge_parameters
	// we use image_mod on the primary image
	// and halo_mod on secondary images and all haloes
	params.image_mod = image_mods();
	params.halo_mod = TC_image_mods();
	params.image= absolute_image();


	if(get_state(STATE_PETRIFIED)) params.image_mod +="~GS()";
	params.primary_frame = t_true;

	const frame_parameters adjusted_params = anim_->get_current_params(params);



	const map_location dst = loc_.get_direction(facing_);
	const int xsrc = disp.get_location_x(loc_);
	const int ysrc = disp.get_location_y(loc_);
	const int xdst = disp.get_location_x(dst);
	const int ydst = disp.get_location_y(dst);
	int d2 = disp.hex_size() / 2;




	const int x = static_cast<int>(adjusted_params.offset * xdst + (1.0-adjusted_params.offset) * xsrc) + d2;
	const int y = static_cast<int>(adjusted_params.offset * ydst + (1.0-adjusted_params.offset) * ysrc) + d2;


	if(unit_halo_ == halo::NO_HALO && !image_halo().empty()) {
		unit_halo_ = halo::add(0, 0, image_halo()+TC_image_mods(), map_location(-1, -1));
	}
	if(unit_halo_ != halo::NO_HALO && image_halo().empty()) {
		halo::remove(unit_halo_);
		unit_halo_ = halo::NO_HALO;
	} else if(unit_halo_ != halo::NO_HALO) {
		halo::set_location(unit_halo_, x, y - height_adjust);
	}



	// We draw bars only if wanted, visible on the map view
	bool draw_bars = draw_bars_ ;
	if (draw_bars) {
		const int d = disp.hex_size();
		SDL_Rect unit_rect = create_rect(xsrc, ysrc +adjusted_params.y, d, d);
		draw_bars = rects_overlap(unit_rect, disp.map_outside_area());
	}

	surface ellipse_front(NULL);
	surface ellipse_back(NULL);
	int ellipse_floating = 0;
	// Always show the ellipse for selected units
	if(draw_bars && (preferences::show_side_colors() || disp.selected_hex() == loc_)) {
		if(adjusted_params.submerge > 0.0) {
			// The division by 2 seems to have no real meaning,
			// It just works fine with the current center of ellipse
			// and prevent a too large adjust if submerge = 1.0
			ellipse_floating = static_cast<int>(adjusted_params.submerge * disp.hex_size() / 2);
		}

		std::string ellipse=image_ellipse();
		if(ellipse.empty()){
			ellipse="misc/ellipse";
		}

		if(ellipse != "none") {
			// check if the unit has a ZoC or can recruit
			const char* const nozoc = emit_zoc_ ? "" : "nozoc-";
			const char* const leader = can_recruit() ? "leader-" : "";
			const char* const selected = disp.selected_hex() == loc_ ? "selected-" : "";

			// Load the ellipse parts recolored to match team color
			char buf[100];
			std::string tc=team::get_side_color_index(side_);

			snprintf(buf,sizeof(buf),"%s-%s%s%stop.png~RC(ellipse_red>%s)",ellipse.c_str(),leader,nozoc,selected,tc.c_str());
			ellipse_back.assign(image::get_image(image::locator(buf), image::SCALED_TO_ZOOM));
			snprintf(buf,sizeof(buf),"%s-%s%s%sbottom.png~RC(ellipse_red>%s)",ellipse.c_str(),leader,nozoc,selected,tc.c_str());
			ellipse_front.assign(image::get_image(image::locator(buf), image::SCALED_TO_ZOOM));
		}
	}

	if (ellipse_back != NULL) {
		//disp.drawing_buffer_add(display::LAYER_UNIT_BG, loc,
		disp.drawing_buffer_add(display::LAYER_UNIT_FIRST, loc_,
			xsrc, ysrc +adjusted_params.y-ellipse_floating, ellipse_back);
	}

	if (ellipse_front != NULL) {
		//disp.drawing_buffer_add(display::LAYER_UNIT_FG, loc,
		disp.drawing_buffer_add(display::LAYER_UNIT_FIRST, loc_,
			xsrc, ysrc +adjusted_params.y-ellipse_floating, ellipse_front);
	}
	if(draw_bars) {
		const image::locator* orb_img = NULL;
		/*static*/ const image::locator partmoved_orb(game_config::images::orb + "~RC(magenta>" +
						preferences::partial_color() + ")"  );
		/*static*/ const image::locator moved_orb(game_config::images::orb + "~RC(magenta>" +
						preferences::moved_color() + ")"  );
		/*static*/ const image::locator ally_orb(game_config::images::orb + "~RC(magenta>" +
						preferences::allied_color() + ")"  );
		/*static*/ const image::locator enemy_orb(game_config::images::orb + "~RC(magenta>" +
						preferences::enemy_color() + ")"  );
		/*static*/ const image::locator unmoved_orb(game_config::images::orb + "~RC(magenta>" +
						preferences::unmoved_color() + ")"  );

		const std::string* energy_file = &game_config::images::energy;

		if(size_t(side()) != disp.viewing_team()+1) {
			if(disp.team_valid() &&
			   disp.get_teams()[disp.viewing_team()].is_enemy(side())) {
				if (preferences::show_enemy_orb())
					orb_img = &enemy_orb;
				else
					orb_img = NULL;
			} else {
				if (preferences::show_allied_orb())
					orb_img = &ally_orb;
				else orb_img = NULL;
			}
		} else {
			if (preferences::show_moved_orb())
				orb_img = &moved_orb;
			else orb_img = NULL;

			if(disp.playing_team() == disp.viewing_team() && !user_end_turn()) {
				if (movement_left() == total_movement()) {
					if (preferences::show_unmoved_orb())
						orb_img = &unmoved_orb;
					else orb_img = NULL;
				} else if ( actions::unit_can_move(*this) ) {
					if (preferences::show_partial_orb())
						orb_img = &partmoved_orb;
					else orb_img = NULL;
				}
			}
		}

		if (orb_img != NULL) {
			surface orb(image::get_image(*orb_img,image::SCALED_TO_ZOOM));
			disp.drawing_buffer_add(display::LAYER_UNIT_BAR,
				loc_, xsrc, ysrc +adjusted_params.y, orb);
		}

		double unit_energy = 0.0;
		if(max_hitpoints() > 0) {
			unit_energy = double(hitpoints())/double(max_hitpoints());
		}
		const int bar_shift = static_cast<int>(-5*disp.get_zoom_factor());
		const int hp_bar_height = static_cast<int>(max_hitpoints() * hp_bar_scaling_);

		const fixed_t bar_alpha = (loc_ == disp.mouseover_hex() || loc_ == disp.selected_hex()) ? ftofxp(1.0): ftofxp(0.8);

		disp.draw_bar(*energy_file, xsrc+bar_shift, ysrc +adjusted_params.y,
			loc_, hp_bar_height, unit_energy,hp_color(), bar_alpha);

		if(experience() > 0 && can_advance()) {
			const double filled = double(experience())/double(max_experience());

			const int xp_bar_height = static_cast<int>(max_experience() * xp_bar_scaling_ / std::max<int>(level_,1));

			SDL_Color color=xp_color();
			disp.draw_bar(*energy_file, xsrc, ysrc +adjusted_params.y,
				loc_, xp_bar_height, filled, color, bar_alpha);
		}

		if (can_recruit()) {
			surface crown(image::get_image(leader_crown(),image::SCALED_TO_ZOOM));
			if(!crown.null()) {
				//if(bar_alpha != ftofxp(1.0)) {
				//	crown = adjust_surface_alpha(crown, bar_alpha);
				//}
				disp.drawing_buffer_add(display::LAYER_UNIT_BAR,
					loc_, xsrc, ysrc +adjusted_params.y, crown);
			}
		}

		for(std::vector<std::string>::const_iterator ov = overlays().begin(); ov != overlays().end(); ++ov) {
			const surface ov_img(image::get_image(*ov, image::SCALED_TO_ZOOM));
			if(ov_img != NULL) {
				disp.drawing_buffer_add(display::LAYER_UNIT_BAR,
					loc_, xsrc, ysrc +adjusted_params.y, ov_img);
			}
		}
	}

	anim_->redraw(params);
	refreshing_ = false;
}

void unit::clear_haloes()
{
	if(unit_halo_ != halo::NO_HALO) {
		halo::remove(unit_halo_);
		unit_halo_ = halo::NO_HALO;
	}
	if(anim_ ) anim_->clear_haloes();
}
bool unit::invalidate(const map_location &loc)
{
	bool result = false;

	// Very early calls, anim not initialized yet
	if(get_animation()) {
		frame_parameters params;
		const display * disp =  display::get_singleton();
		const gamemap & map = disp->get_map();
		const t_translation::t_terrain terrain = map.get_terrain(loc);
		const terrain_type& terrain_info = map.get_terrain_info(terrain);

		int height_adjust = static_cast<int>(terrain_info.unit_height_adjust() * disp->get_zoom_factor());
		if (is_flying() && height_adjust < 0) {
			height_adjust = 0;
		}
		params.y -= height_adjust;
		params.halo_y -= height_adjust;
		params.image_mod = image_mods();
		params.halo_mod = TC_image_mods();
		params.image= absolute_image();

		result |= get_animation()->invalidate(params);
	}

	return result;

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
	if(cfg_["upkeep"] == "free") {
		return 0;
	}
	return cfg_["upkeep"];
}

bool unit::loyal() const
{
	return cfg_["upkeep"] == "loyal" || cfg_["upkeep"] == "free";
}

int unit::defense_modifier(const t_translation::t_terrain & terrain) const
{
	int def = movement_type_.defense_modifier(terrain);
#if 0
	// A [defense] ability is too costly and doesn't take into account target locations.
	// Left as a comment in case someone ever wonders why it isn't a good idea.
	unit_ability_list defense_abilities = get_abilities("defense");
	if (!defense_abilities.empty()) {
		unit_abilities::effect defense_effect(defense_abilities, def, false);
		def = defense_effect.get_composite_value();
	}
#endif
	return def;
}

bool unit::resistance_filter_matches(const config& cfg, bool attacker, const std::string& damage_name, int res) const
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


int unit::resistance_against(const std::string& damage_name,bool attacker,const map_location& loc) const
{
	int res = movement_type_.resistance_against(damage_name);

	unit_ability_list resistance_abilities = get_abilities("resistance",loc);
	for (unit_ability_list::iterator i = resistance_abilities.begin(); i != resistance_abilities.end();) {
		if(!resistance_filter_matches(*i->first, attacker, damage_name, 100-res)) {
			i = resistance_abilities.erase(i);
		} else {
			++i;
		}
	}
	if(!resistance_abilities.empty()) {
		unit_abilities::effect resist_effect(resistance_abilities, 100-res, false);

		res = 100 - std::min<int>(resist_effect.get_composite_value(),
		                          resistance_abilities.highest("max_value").first);
	}
	return res;
}

std::map<std::string,std::string> unit::advancement_icons() const
{
	std::map<std::string,std::string> temp;
	if (!can_advance())
		return temp;

	if (!advances_to_.empty())
	{
		std::ostringstream tooltip;
		const std::string &image = game_config::images::level;
		BOOST_FOREACH(const std::string &s, advances_to())
		{
			if (!s.empty())
				tooltip << s << '\n';
		}
		temp[image] = tooltip.str();
	}

	BOOST_FOREACH(const config &adv, get_modification_advances())
	{
		const std::string &image = adv["image"];
		if (image.empty()) continue;
		std::ostringstream tooltip;
		tooltip << temp[image];
		const std::string &tt = adv["description"];
		if (!tt.empty())
			tooltip << tt << '\n';
		temp[image] = tooltip.str();
	}
	return(temp);
}
std::vector<std::pair<std::string,std::string> > unit::amla_icons() const
{
	std::vector<std::pair<std::string,std::string> > temp;
	std::pair<std::string,std::string> icon; // <image,tooltip>

	BOOST_FOREACH(const config &adv, get_modification_advances())
	{
		icon.first = adv["icon"].str();
		icon.second = adv["description"].str();

		for (unsigned j = 0, j_count = modification_count("advance", adv["id"]);
		     j < j_count; ++j)
		{
			temp.push_back(icon);
		}
	}
	return(temp);
}

std::vector<config> unit::get_modification_advances() const
{
	std::vector<config> res;
	BOOST_FOREACH(const config &adv, modification_advancements())
	{
		if (adv["strict_amla"].to_bool() && !advances_to_.empty())
			continue;
		if (modification_count("advance", adv["id"]) >= unsigned(adv["max_times"].to_int(1)))
			continue;

		std::vector<std::string> temp = utils::split(adv["require_amla"]);
		if (temp.empty()) {
			res.push_back(adv);
			continue;
		}

		std::sort(temp.begin(), temp.end());
		std::vector<std::string> uniq;
		std::unique_copy(temp.begin(), temp.end(), std::back_inserter(uniq));

		bool requirements_done = true;
		BOOST_FOREACH(const std::string &s, uniq)
		{
			int required_num = std::count(temp.begin(), temp.end(), s);
			int mod_num = modification_count("advance", s);
			if (required_num > mod_num) {
				requirements_done = false;
				break;
			}
		}
		if (requirements_done)
			res.push_back(adv);
	}

	return res;
}

size_t unit::modification_count(const std::string& mod_type, const std::string& id) const
{
	size_t res = 0;
	BOOST_FOREACH(const config &item, modifications_.child_range(mod_type)) {
		if (item["id"] == id) {
			++res;
		}
	}

	return res;
}

void unit::add_modification(const std::string& mod_type, const config& mod, bool no_add)
{
	bool generate_description = true;

	//some trait activate specific flags
	if ( mod_type == "trait" ) {
		const std::string& id = mod["id"];
		is_fearless_ = is_fearless_ || id == "fearless";
		is_healthy_ = is_healthy_ || id == "healthy";
		if (!mod["generate_description"].empty()) {
			generate_description = mod["generate_description"];
		}
	}

	config *new_child = NULL;
	if(no_add == false) {
		new_child = &modifications_.add_child(mod_type, mod);
	}
	bool set_poisoned = false; // Tracks if the poisoned state was set after the type or variation was changed.
	config last_effect;
	std::vector<t_string> effects_description;
	BOOST_FOREACH(const config &effect, mod.child_range("effect"))
	{
		// Apply SUF.
		if (const config &afilter = effect.child("filter"))
			if (!matches_filter(vconfig(afilter), loc_)) continue;

		const std::string &apply_to = effect["apply_to"];
		const std::string &apply_times = effect["times"];
		int times = 1;
		t_string description;

		if (apply_times == "per level")
			times = level_;
		if (times) {
			while (times > 0) {
				times --;

				// Apply unit type/variation changes last to avoid double applying effects on advance.
				if ((apply_to == "variation" || apply_to == "type") && no_add == false) {
					set_poisoned = false;
					last_effect = effect;
				} else if(apply_to == "profile") {
					if (const config::attribute_value *v = effect.get("portrait")) {
						std::string big = *v, small = effect["small_portrait"];
						adjust_profile(small, big, "");
						cfg_["profile"] = big;
						cfg_["small_profile"] = small;
					}
					if (const config::attribute_value *v = effect.get("description"))
						cfg_["description"] = *v;
					//help::unit_topic_generator(*this, (**i.first)["help_topic"]);
				} else if(apply_to == "new_attack") {
					attacks_.push_back(attack_type(effect));
				} else if(apply_to == "remove_attacks") {
					std::vector<attack_type>::iterator a = attacks_.begin();
					while(a != attacks_.end()) {
						if(a->matches_filter(effect)) {
							a = attacks_.erase(a);
							continue;
						}
						++a;
					}
				} else if(apply_to == "attack") {

					bool first_attack = true;

					std::string attack_names;
					std::string desc;
					for(std::vector<attack_type>::iterator a = attacks_.begin();
						a != attacks_.end(); ++a) {
						bool affected = a->apply_modification(effect, &desc);
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
					const std::string &increase_hp = effect["increase"];
					const std::string &increase_total = effect["increase_total"];
					const std::string &set_hp = effect["set"];
					const std::string &set_total = effect["set_total"];

					// If the hitpoints are allowed to end up greater than max hitpoints
					const bool violate_max = effect["violate_maximum"].to_bool();

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
							description += utils::print_modifier(increase_total) + " " +
								t_string(N_("HP"), "wesnoth");

						// A percentage on the end means increase by that many percent
						max_hit_points_ = utils::apply_modifier(max_hit_points_, increase_total);
					}

					if(max_hit_points_ < 1)
						max_hit_points_ = 1;

					if (effect["heal_full"].to_bool()) {
						heal_all();
					}

					if(increase_hp.empty() == false) {
						hit_points_ = utils::apply_modifier(hit_points_, increase_hp);
					}

					LOG_UT << "modded to " << hit_points_ << "/" << max_hit_points_ << "\n";
					if(hit_points_ > max_hit_points_ && !violate_max) {
						LOG_UT << "resetting hp to max\n";
						hit_points_ = max_hit_points_;
					}

					if(hit_points_ < 1)
						hit_points_ = 1;
				} else if(apply_to == "movement") {
					const std::string &increase = effect["increase"];

					if(increase.empty() == false) {
						if (!times)
							description += utils::print_modifier(increase) + " " +
								t_string(N_("moves"), "wesnoth");

						max_movement_ = utils::apply_modifier(max_movement_, increase, 1);
					}

					max_movement_ = effect["set"].to_int(max_movement_);

					if(movement_ > max_movement_)
						movement_ = max_movement_;
				} else if(apply_to == "experience") {
					const std::string &increase = effect["increase"];
					const std::string &set = effect["set"];

					if(set.empty() == false) {
						if(set[set.size()-1] == '%') {
							experience_ = lexical_cast_default<int>(set)*max_experience_/100;
						} else {
							experience_ = lexical_cast_default<int>(set);
						}
					}

					if(increase.empty() == false) {
						experience_ = utils::apply_modifier(experience_, increase, 1);
					}
				} else if(apply_to == "max_experience") {
					const std::string &increase = effect["increase"];

					if(increase.empty() == false) {
						if (!times)
							description += utils::print_modifier(increase) + " " +
								t_string(N_("XP to advance"), "wesnoth");

						max_experience_ = utils::apply_modifier(max_experience_, increase, 1);
					}

				} else if(apply_to == "loyal") {
					cfg_["upkeep"] = "loyal";
				} else if(apply_to == "status") {
					const std::string &add = effect["add"];
					const std::string &remove = effect["remove"];

					if(add.empty() == false) {
						set_state(add, true);
						set_poisoned = set_poisoned  ||  add == "poisoned";
					}

					if(remove.empty() == false) {
						set_state(remove, false);
						set_poisoned = set_poisoned  &&  remove != "poisoned";
					}
				// Note: It would not be hard to define a new "applies_to=" that
				//       combines the next five options (the movetype effects).
				} else if (apply_to == "movement_costs") {
					if (const config &ap = effect.child("movement_costs")) {
						movement_type_.get_movement().merge(ap, effect["replace"].to_bool());
					}
				} else if (apply_to == "vision_costs") {
					if (const config &ap = effect.child("vision_costs")) {
						movement_type_.get_vision().merge(ap, effect["replace"].to_bool());
					}
				} else if (apply_to == "jamming_costs") {
					if (const config &ap = effect.child("jamming_costs")) {
						movement_type_.get_jamming().merge(ap, effect["replace"].to_bool());
					}
				} else if (apply_to == "defense") {
					if (const config &ap = effect.child("defense")) {
						movement_type_.get_defense().merge(ap, effect["replace"].to_bool());
					}
				} else if (apply_to == "resistance") {
					if (const config &ap = effect.child("resistance")) {
						movement_type_.get_resistances().merge(ap, effect["replace"].to_bool());
					}
				} else if (apply_to == "zoc") {
					if (const config::attribute_value *v = effect.get("value")) {
						emit_zoc_ = v->to_bool();
					}
				} else if (apply_to == "new_ability") {
					config &ab = cfg_.child_or_add("abilities");
					if (const config &ab_effect = effect.child("abilities")) {
						config to_append;
						BOOST_FOREACH(const config::any_child &ab, ab_effect.all_children_range()) {
							if(!has_ability_by_id(ab.cfg["id"])) {
								to_append.add_child(ab.key, ab.cfg);
							}
						}
						ab.append(to_append);
					}
				} else if (apply_to == "remove_ability") {
					if (const config &ab_effect = effect.child("abilities")) {
						BOOST_FOREACH(const config::any_child &ab, ab_effect.all_children_range()) {
							remove_ability_by_id(ab.cfg["id"]);
						}
					}
				} else if (apply_to == "image_mod") {
					LOG_UT << "applying image_mod \n";
					std::string mod = effect["replace"];
					if (!mod.empty()){
						image_mods_ = mod;
					}
					LOG_UT << "applying image_mod \n";
					mod = effect["add"].str();
					if (!mod.empty()){
						if(!image_mods_.empty()) {
							image_mods_ += '~';
						}

						image_mods_ += mod;
					}

					game_config::add_color_info(effect);
					LOG_UT << "applying image_mod \n";
				} else if (apply_to == "new_animation") {
					if(effect["id"].empty()) {
						unit_animation::add_anims(animations_, effect);
					} else {
						std::vector<unit_animation> &built = resources::controller->animation_cache[effect["id"]];
						if(built.empty()) {
							unit_animation::add_anims(built, effect);
						}
						animations_.insert(animations_.end(),built.begin(),built.end());
					}

				} else if (apply_to == "ellipse") {
					cfg_["ellipse"] = effect["ellipse"];

				} else if (apply_to == "halo") {
					clear_haloes();
					cfg_["halo"] = effect["halo"];

				} else if (apply_to == "overlay") {
					const std::string &add = effect["add"];
					const std::string &replace = effect["replace"];

					if (!add.empty()) {
						std::vector<std::string> temp_overlays = utils::parenthetical_split(add, ',');
						std::vector<std::string>::iterator it;
						for (it=temp_overlays.begin();it<temp_overlays.end();++it) {
							overlays_.push_back( *it );
						}
					}
					else if (!replace.empty()) {
						overlays_ = utils::parenthetical_split(replace, ',');
					}
				}
			} // end while
		} else { // for times = per level & level = 0 we still need to rebuild the descriptions
			if(apply_to == "attack") {

				bool first_attack = true;

				for(std::vector<attack_type>::iterator a = attacks_.begin();
					a != attacks_.end(); ++a) {
					std::string desc;
					bool affected = a->describe_modification(effect, &desc);
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
				const std::string &increase_total = effect["increase_total"];

				if(increase_total.empty() == false) {
					description += utils::print_modifier(increase_total) + " " +
						t_string(N_("HP"), "wesnoth");
				}
			} else if(apply_to == "movement") {
				const std::string &increase = effect["increase"];

				if(increase.empty() == false) {
					description += utils::print_modifier(increase) + t_string(N_(" move"), "wesnoth");
				}
			} else if(apply_to == "max_experience") {
				const std::string &increase = effect["increase"];

				if(increase.empty() == false) {
					description += utils::print_modifier(increase) + " " +
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
	// Apply variations -- only apply if we are adding this for the first time.
	if (!last_effect.empty() && no_add == false) {
		if ((last_effect)["apply_to"] == "variation") {
			variation_ = last_effect["name"].str();
			const unit_type * base_type = unit_types.find(type().base_id());
			assert(base_type != NULL);
			advance_to(*base_type);
		} else if ((last_effect)["apply_to"] == "type") {
			config::attribute_value &prev_type = (*new_child)["prev_type"];
			if (prev_type.blank()) prev_type = type().base_id();
			const std::string& new_type_id = last_effect["name"];
			const unit_type* new_type = unit_types.find(new_type_id);
			if ( new_type ) {
				const bool heal_full = last_effect["heal_full"].to_bool(false);
				advance_to(*new_type);
				preferences::encountered_units().insert(new_type_id);
				if( heal_full ) {
					heal_all();
				}
			} else {
				WRN_UT << "unknown type= in [effect]apply_to=type, ignoring\n";
			}
		}
		if ( set_poisoned )
			// An effect explicitly set the poisoned state, and this
			// should override the unit being immune to poison.
			set_state(STATE_POISONED, true);
	}

	t_string description;

	const t_string& mod_description = mod["description"];
	if (!mod_description.empty()) {
		description = mod_description + " ";
	}

	// Punctuation should be translatable: not all languages use Latin punctuation.
	// (However, there maybe is a better way to do it)
	if(effects_description.empty() == false && generate_description == true) {
		for(std::vector<t_string>::const_iterator i = effects_description.begin();
				i != effects_description.end(); ++i) {
			description += *i;
			if(i+1 != effects_description.end())
				description += t_string(N_(" and "), "wesnoth");
		}
	}

	// store trait info
	if ( mod_type == "trait" ) {
		add_trait_description(mod, description);
	}

	//NOTE: if not a trait, description is currently not used
}

void unit::add_trait_description(const config& trait, const t_string& description)
{
	const std::string& gender_string = gender_ == unit_race::FEMALE ? "female_name" : "male_name";
	t_string const &gender_specific_name = trait[gender_string];

	// if this is a t_string& instead of a t_string, msvc9 compiled windows binaries
	// choke on the case where both gender_specific_name and trait["name"] are empty.
	const t_string name = gender_specific_name.empty() ?
		 trait["name"] : gender_specific_name;

	if(!name.empty()) {
		trait_names_.push_back(name);
		trait_descriptions_.push_back(description);
	}
}

std::string unit::absolute_image() const {
	return cfg_["image_icon"].empty() ? cfg_["image"] : cfg_["image_icon"];
}

const unit_animation* unit::choose_animation(const display& disp, const map_location& loc,const std::string& event,
		const map_location& second_loc,const int value,const unit_animation::hit_type hit,
		const attack_type* attack, const attack_type* second_attack, int swing_num) const
{
	// Select one of the matching animations at random
	std::vector<const unit_animation*> options;
	int max_val = unit_animation::MATCH_FAIL;
	for(std::vector<unit_animation>::const_iterator i = animations_.begin(); i != animations_.end(); ++i) {
		int matching = i->matches(disp,loc,second_loc,this,event,value,hit,attack,second_attack,swing_num);
		if(matching > unit_animation::MATCH_FAIL && matching == max_val) {
			options.push_back(&*i);
		} else if(matching > max_val) {
			max_val = matching;
			options.clear();
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

	for(size_t i = 0; i != NumModificationTypes; ++i) {
		const std::string& mod = ModificationTypes[i];
		BOOST_FOREACH(const config &m, modifications_.child_range(mod)) {
			log_scope("add mod");
			add_modification(ModificationTypes[i], m, true);
		}
	}

	//apply the experience acceleration last
	int exp_accel = unit_type::experience_accelerator::get_acceleration();
	max_experience_ = std::max<int>(1, (max_experience_ * exp_accel + 50)/100);
}

bool unit::invisible(const map_location& loc, bool see_all) const
{
	// This is a quick condition to check, and it does not depend on the
	// location (so might as well bypass the location-based cache).
	if ( get_state(STATE_UNCOVERED) )
		return false;

	// Fetch from cache
	/**
	 * @todo FIXME: We use the cache only when using the default see_all=true
	 * Maybe add a second cache if the see_all=false become more frequent.
	 */
	if(see_all) {
		std::map<map_location, bool>::const_iterator itor = invisibility_cache_.find(loc);
		if(itor != invisibility_cache_.end()) {
			return itor->second;
		}
	}

	// Test hidden status
	static const std::string hides("hides");
	bool is_inv = get_ability_bool(hides,loc);
	if(is_inv){
		const std::vector<team>& teams = *resources::teams;
		BOOST_FOREACH(const unit &u, *resources::units)
		{
			const map_location &u_loc = u.get_location();
			if (teams[side_-1].is_enemy(u.side()) && !u.incapacitated() && tiles_adjacent(loc, u_loc)) {
				// Enemy spotted in adjacent tiles, check if we can see him.
				// Watch out to call invisible with see_all=true to avoid infinite recursive calls!
				if(see_all) {
					is_inv = false;
					break;
				} else if (!teams[side_-1].fogged(u_loc)
				&& !u.invisible(u_loc, true)) {
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


bool unit::is_visible_to_team(team const& team, bool const see_all, gamemap const& map) const
{
	map_location const& loc = get_location();
	if (!map.on_board(loc))
		return false;
	if (see_all)
		return true;
	if (resources::screen->is_blindfolded())
		return false;
	if (team.is_enemy(side()) && invisible(loc))
		return false;
	if (team.is_enemy(side()) && team.fogged(loc))
		return false;
	if (team.fogged(loc)  && !(*resources::teams)[side() - 1].share_view())
		return false;

	return true;
}

void unit::set_underlying_id() {
	if(underlying_id_ == 0){
		underlying_id_ = n_unit::id_manager::instance().next_id();
	}
	if (id_.empty()) {
		std::stringstream ss;
		ss << (type_id().empty() ? "Unit" : type_id()) << "-" << underlying_id_;
		id_ = ss.str();
	}
}

unit& unit::clone(bool is_temporary)
{
	if(is_temporary) {
		underlying_id_ = n_unit::id_manager::instance().next_fake_id();
	} else {
		underlying_id_ = n_unit::id_manager::instance().next_id();
		std::string::size_type pos = id_.find_last_of('-');
		if(pos != std::string::npos && pos+1 < id_.size()
		&& id_.find_first_not_of("0123456789", pos+1) == std::string::npos) {
			// this appears to be a duplicate of a generic unit, so give it a new id
			WRN_UT << "assigning new id to clone of generic unit " << id_ << "\n";
			id_.clear();
			set_underlying_id();
		}
	}
	return *this;
}


unit_movement_resetter::unit_movement_resetter(unit &u, bool operate) :
	u_(u), moves_(u.movement_left(true))
{
	if (operate) {
		u.set_movement(u.total_movement());
	}
}

unit_movement_resetter::~unit_movement_resetter()
{
	assert(resources::units);

	if(!resources::units->has_unit(&u_)) {
		/*
		 * It might be valid that the unit is not in the unit map.
		 * It might also mean a no longer valid unit will be assigned to.
		 */
		DBG_UT << "The unit to be removed is not in the unit map.\n";
	}
	u_.set_movement(moves_);
}

bool unit::matches_id(const std::string& unit_id) const
{
        return id_ == unit_id;
}

/**
 * Used to find units in vectors by their ID. (Convenience wrapper)
 * @returns what std::find_if() returns.
 */
std::vector<unit>::iterator find_if_matches_id(
		std::vector<unit> &unit_list, // Not const so we can get a non-const iterator to return.
		const std::string &unit_id)
{
	return std::find_if(unit_list.begin(), unit_list.end(),
	                    boost::bind(&unit::matches_id, _1, unit_id));
}

/**
 * Used to find units in vectors by their ID. (Convenience wrapper; const version)
 * @returns what std::find_if() returns.
 */
std::vector<unit>::const_iterator find_if_matches_id(
		const std::vector<unit> &unit_list,
		const std::string &unit_id)
{
	return std::find_if(unit_list.begin(), unit_list.end(),
	                    boost::bind(&unit::matches_id, _1, unit_id));
}

/**
 * Used to erase units from vectors by their ID. (Convenience wrapper)
 * @returns what std::vector<>::erase() returns.
 */
std::vector<unit>::iterator erase_if_matches_id(
		std::vector<unit> &unit_list,
		const std::string &unit_id)
{
	return unit_list.erase(std::remove_if(unit_list.begin(), unit_list.end(),
	                                      boost::bind(&unit::matches_id, _1, unit_id)),
	                       unit_list.end());
}

int side_units(int side)
{
	int res = 0;
	BOOST_FOREACH(const unit &u, *resources::units) {
		if (u.side() == side) ++res;
	}
	return res;
}

int side_units_cost(int side)
{
	int res = 0;
	BOOST_FOREACH(const unit &u, *resources::units) {
		if (u.side() == side) res += u.cost();
	}
	return res;
}

int side_upkeep(int side)
{
	int res = 0;
	BOOST_FOREACH(const unit &u, *resources::units) {
		if (u.side() == side) res += u.upkeep();
	}
	return res;
}

unit_map::iterator find_visible_unit(const map_location &loc,
	const team& current_team, bool see_all)
{
	unit_map& units = *resources::units;
	if (!resources::game_map->on_board(loc)) return units.end();
	unit_map::iterator u = units.find(loc);
	if (!u.valid() || !u->is_visible_to_team(current_team, see_all))
		return units.end();
	return u;
}

unit *get_visible_unit(const map_location &loc,
	const team &current_team, bool see_all)
{
	unit_map::iterator ui = find_visible_unit(loc,
		current_team, see_all);
	if (ui == resources::units->end()) return NULL;
	return &*ui;
}

void unit::refresh()
{
	if (state_ == STATE_FORGET && anim_ && anim_->animation_finished_potential())
	{
		set_standing();
		return;
	}
	display &disp = *display::get_singleton();
	if (state_ != STATE_STANDING || get_current_animation_tick() < next_idling_ ||
	    !disp.tile_nearly_on_screen(loc_) || incapacitated())
	{
		return;
	}
	if (get_current_animation_tick() > next_idling_ + 1000)
	{
		// prevent all units animating at the same time
		if (disp.idle_anim()) {
			next_idling_ = get_current_animation_tick()
				+ static_cast<int>((20000 + rand() % 20000) * disp.idle_anim_rate());
		} else {
			next_idling_ = INT_MAX;
		}
	} else {
		set_idling();
	}
}

team_data calculate_team_data(const team& tm, int side)
{
	team_data res;
	res.units = side_units(side);
	res.upkeep = side_upkeep(side);
	res.villages = tm.villages().size();
	res.expenses = std::max<int>(0,res.upkeep - tm.support());
	res.net_income = tm.total_income() - res.expenses;
	res.gold = tm.gold();
	res.teamname = tm.user_team_name();
	return res;
}

temporary_unit_placer::temporary_unit_placer(unit_map& m, const map_location& loc, unit& u)
	: m_(m), loc_(loc), temp_(m.extract(loc))
{
	u.clone();
	m.add(loc, u);
}

temporary_unit_placer::~temporary_unit_placer()
{
	m_.erase(loc_);
	if(temp_) {
		m_.insert(temp_);
	}
}

temporary_unit_remover::temporary_unit_remover(unit_map& m, const map_location& loc)
	: m_(m), loc_(loc), temp_(m.extract(loc))
{
}

temporary_unit_remover::~temporary_unit_remover()
{
	if(temp_) {
		m_.insert(temp_);
	}
}

/**
 * Constructor
 * This version will change the unit's current movement to @a new_moves while
 * the unit is moved (and restored to its previous value upon this object's
 * destruction).
 */
temporary_unit_mover::temporary_unit_mover(unit_map& m, const map_location& src,
                                           const map_location& dst, int new_moves)
	: m_(m), src_(src), dst_(dst), old_moves_(-1),
	  temp_(src == dst ? NULL : m.extract(dst))
{
	std::pair<unit_map::iterator, bool> move_result = m.move(src_, dst_);

	// Set the movement.
	if ( move_result.second )
	{
		old_moves_ = move_result.first->movement_left(true);
		move_result.first->set_movement(new_moves);
	}
}

/**
 * Constructor
 * This version does not change (nor restore) the unit's movement.
 */
temporary_unit_mover::temporary_unit_mover(unit_map& m, const map_location& src,
                                           const map_location& dst)
	: m_(m), src_(src), dst_(dst), old_moves_(-1),
	  temp_(src == dst ? NULL : m.extract(dst))
{
	m.move(src_, dst_);
}

temporary_unit_mover::~temporary_unit_mover()
{
	std::pair<unit_map::iterator, bool> move_result = m_.move(dst_, src_);

	// Restore the movement?
	if ( move_result.second  &&  old_moves_ >= 0 )
		move_result.first->set_movement(old_moves_);

	// Restore the extracted unit?
	if(temp_) {
		m_.insert(temp_);
	}
}

std::string unit::TC_image_mods() const{
	std::stringstream modifier;
	if(!flag_rgb_.empty()){
		modifier << "~RC("<< flag_rgb_ << ">" << team::get_side_color_index(side()) << ")";
	}
	return modifier.str();
}
std::string unit::image_mods() const{
	std::stringstream modifier;
	if(!image_mods_.empty()){
		modifier << "~" << image_mods_;
	}
	modifier << TC_image_mods();
	return modifier.str();
}

const std::string& unit::effect_image_mods() const{
	return image_mods_;
}

const tportrait* unit::portrait(
		const unsigned size, const tportrait::tside side) const
{
	BOOST_FOREACH(const tportrait& portrait, type().portraits() ) {
		if(portrait.size == size
				&& (side ==  portrait.side || portrait.side == tportrait::BOTH)) {

			return &portrait;
		}
	}

	return NULL;
}

void unit::remove_attacks_ai()
{
	if (attacks_left_ == max_attacks_) {
		//TODO: add state_not_attacked
	}
	set_attacks(0);
}


void unit::remove_movement_ai()
{
	if (movement_left() == total_movement()) {
		set_state(STATE_NOT_MOVED,true);
	}
	set_movement(0, true);
}


void unit::set_hidden(bool state) {
	hidden_ = state;
	if(!state) return;
	// We need to get rid of haloes immediately to avoid display glitches
	clear_haloes();
}

// Filters unimportant stats from the unit config and returns a checksum of
// the remaining config.
std::string get_checksum(const unit& u) {
	config unit_config;
	config wcfg;
	u.write(unit_config);
	const std::string main_keys[] =
		{ "advances_to",
		"alignment",
		"cost",
		"experience",
		"gender",
		"hitpoints",
		"ignore_race_traits",
		"ignore_global_traits",
		"level",
		"recall_cost",
		"max_attacks",
		"max_experience",
		"max_hitpoints",
		"max_moves",
		"movement",
		"movement_type",
		"race",
		"random_traits",
		"resting",
		"undead_variation",
		"upkeep",
		"zoc",
		""};

	for (int i = 0; !main_keys[i].empty(); ++i)
	{
		wcfg[main_keys[i]] = unit_config[main_keys[i]];
	}
	const std::string attack_keys[] =
		{ "name",
		  "type",
		  "range",
		  "damage",
		  "number",
		  ""};

	BOOST_FOREACH(const config &att, unit_config.child_range("attack"))
	{
		config& child = wcfg.add_child("attack");
		for (int i = 0; !attack_keys[i].empty(); ++i) {
			child[attack_keys[i]] = att[attack_keys[i]];
		}
		BOOST_FOREACH(const config &spec, att.child_range("specials")) {
			config& child_spec = child.add_child("specials", spec);
			child_spec.recursive_clear_value("description");
		}

	}

	BOOST_FOREACH(const config &abi, unit_config.child_range("abilities"))
	{
		config& child = wcfg.add_child("abilities", abi);
		child.recursive_clear_value("description");
		child.recursive_clear_value("description_inactive");
		child.recursive_clear_value("name");
		child.recursive_clear_value("name_inactive");
	}

	BOOST_FOREACH(const config &trait, unit_config.child_range("trait"))
	{
		config& child = wcfg.add_child("trait", trait);
		child.recursive_clear_value("description");
		child.recursive_clear_value("male_name");
		child.recursive_clear_value("female_name");
		child.recursive_clear_value("name");
	}

	const std::string child_keys[] = {"advance_from", "defense", "movement_costs", "vision_costs", "jamming_costs" "resistance", ""};

	for (int i = 0; !child_keys[i].empty(); ++i)
	{
		BOOST_FOREACH(const config &c, unit_config.child_range(child_keys[i])) {
			wcfg.add_child(child_keys[i], c);
		}
	}
	DBG_UT << wcfg;

	return wcfg.hash();
}

