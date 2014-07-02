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
#include "global.hpp"

#include "formula_string_utils.hpp"     // for vgettext
#include "game_board.hpp"               // for game_board
#include "game_config.hpp"              // for add_color_info, etc
#include "game_errors.hpp"              // for game_error
#include "game_events/handlers.hpp"     // for add_events
#include "game_preferences.hpp"         // for encountered_units
#include "gettext.hpp"                  // for N_
#include "log.hpp"                      // for LOG_STREAM, logger, etc
#include "make_enum.hpp"                // for operator<<, operator>>
#include "map.hpp"       // for gamemap
#include "random_new.hpp"               // for generator, rng
#include "resources.hpp"                // for units, gameboard, teams, etc
#include "scripting/lua.hpp"            // for LuaKernel
#include "side_filter.hpp"              // for side_filter
#include "team.hpp"                     // for team, get_teams, etc
#include "terrain_filter.hpp"           // for terrain_filter
#include "unit_abilities.hpp"           // for effect, filter_base_matches
#include "unit_animation.hpp"           // for unit_animation
#include "unit_animation_component.hpp"  // for unit_animation_component
#include "unit_formula_manager.hpp"     // for unit_formula_manager
#include "unit_id.hpp"
#include "unit_map.hpp"      // for unit_map, etc
#include "variable.hpp"                 // for vconfig, etc

#include <boost/bind.hpp>
#include <boost/foreach.hpp>            // for auto_any_base, etc
#include <boost/intrusive_ptr.hpp>      // for intrusive_ptr
#include <boost/function_output_iterator.hpp>
#include <boost/range/algorithm.hpp>
#include <cassert>                     // for assert
#include <cstdlib>                     // for NULL, rand
#include <exception>                    // for exception
#include <iterator>                     // for back_insert_iterator, etc
#include <new>                          // for operator new
#include <ostream>                      // for operator<<, basic_ostream, etc
#include "SDL_video.h"                  // for SDL_Color

namespace t_translation { struct t_terrain; }

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
 * Intrusive Pointer interface
 *
 **/

void intrusive_ptr_add_ref(const unit * u)
{
	assert(u->ref_count_ >= 0);
	// the next code line is to notice possible wrongly intilized units.
	// The 100000 is picked rather randomly. If you are in the situation
	// that you can actualy have more then 100000 intrusive_ptr to one unit
	// or if you are sure that the refcounting system works 
	// then feel free to remove the next line
	assert(u->ref_count_ < 100000);
	LOG_UT << "Adding a reference to a unit: id = " << u->id() << ", uid = " << u->underlying_id() << ", refcount = " << u->ref_count() << " ptr:" << u << std::endl;
        if (u->ref_count_ == 0) {
		LOG_UT << "Freshly constructed" << std::endl;
	}
	++(u->ref_count_);
}

void intrusive_ptr_release(const unit * u)
{
	assert(u->ref_count_ >= 1); 
	assert(u->ref_count_ < 100000); //See comment in intrusive_ptr_add_ref
	LOG_UT << "Removing a reference to a unit: id = " << u->id() << ", uid = " << u->underlying_id() << ", refcount = " << u->ref_count() << " ptr:" << u << std::endl;
	if (--(u->ref_count_) == 0)
	{
		LOG_UT << "Deleting a unit: id = " << u->id() << ", uid = " << u->underlying_id() << std::endl;
		delete u;
	}
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
	   ref_count_(0),
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

           formula_man_(new unit_formula_manager(o.formula_manager())),

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

           anim_comp_(new unit_animation_component(*this, *o.anim_comp_)),

           getsHit_(o.getsHit_),
           hidden_(o.hidden_),
           hp_bar_scaling_(o.hp_bar_scaling_),
           xp_bar_scaling_(o.xp_bar_scaling_),

           modifications_(o.modifications_),
		   invisibility_cache_()
{
}

unit::unit(const config &cfg, bool use_traits, const vconfig* vcfg) :
	ref_count_(0),
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
	alignment_(lexical_cast_default<unit_type::ALIGNMENT> (cfg["alignment"].str(), unit_type::NEUTRAL)),
	flag_rgb_(),
	image_mods_(),
	unrenamable_(false),
	side_(0),
	gender_(generate_gender(*type_, cfg)),
	alpha_(),
	formula_man_(new unit_formula_manager()),
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
	anim_comp_(new unit_animation_component(*this)),
	getsHit_(0),
	hidden_(false),
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
		anim_comp_->clear_haloes();
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
		formula_man_->read(ai);
	}

	//don't use the unit_type's attacks if this config has its own defined
	config::const_child_itors cfg_range = cfg.child_range("attack");
	if(cfg_range.first != cfg_range.second) {
		attacks_.clear();
		do {
			attacks_.push_back(attack_type(*cfg_range.first));
		} while(++cfg_range.first != cfg_range.second);
	}

	//If cfg specifies [advancement]s, replace this [advancement]s with them.
	if(cfg.has_child("advancement"))
	{
		cfg_.clear_children("advancement");
		boost::copy( cfg.child_range("advancement")
			, boost::make_function_output_iterator(
				boost::bind(
					  /* The static_cast is required to select the proper overload in C++11 mode. */
					  static_cast<config&(config::*)(const std::string &, const config&)>(&config::add_child)
					, boost::ref(cfg_) /*thisptr*/
					, "advancement"
					, _1 )) );
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
		WRN_UT << "Unknown attribute '" << attr.first << "' discarded." << std::endl;
	}

	//debug unit animations for units as they appear in game
	/*for(std::vector<unit_animation>::const_iterator i = anim_comp_->animations_.begin(); i != anim_comp_->animations_.end(); ++i) {
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
	ref_count_(0),
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
	formula_man_(new unit_formula_manager()),
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
	anim_comp_(new unit_animation_component(*this)),
	getsHit_(0),
	hidden_(false),
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
	try {
	anim_comp_->clear_haloes();

	// Remove us from the status cache
	std::vector<const unit *>::iterator itor =
	std::find(units_with_cache.begin(), units_with_cache.end(), this);

	if(itor != units_with_cache.end()) {
		units_with_cache.erase(itor);
	}
	} catch (std::exception & e) {
		ERR_UT << "Caught exception when destroying unit: " << e.what() << std::endl;
	} catch (...) {}
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
			// Make sure to return empty id trait strings as otherwise
			// names will not match in length (Bug #21967)
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

	anim_comp_->reset_after_advance(&new_type);
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
		anim_comp_->clear_haloes();
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
		assert(resources::gameboard != NULL);
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
			if ((!resources::gameboard && loc.valid()) ||
			    (resources::gameboard && resources::gameboard->map().on_board(loc)))
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
	if (!cfg_defense.blank() && cfg_defense.to_int(-1) != defense_modifier(resources::gameboard->map().get_terrain(loc))) {
		return false;
	}

	config::attribute_value cfg_movement = cfg["movement_cost"];
	if (!cfg_movement.blank() && cfg_movement.to_int(-1) != movement_cost(resources::gameboard->map().get_terrain(loc))) {
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
		assert(resources::units && resources::gameboard);
		const unit_map& units = *resources::units;
		map_location adjacent[6];
		get_adjacent_tiles(loc, adjacent);
		vconfig::child_list::const_iterator i, i_end;
		const vconfig::child_list& adj_filt = cfg.get_children("filter_adjacent");
		for (i = adj_filt.begin(), i_end = adj_filt.end(); i != i_end; ++i) {
			int match_count=0;
			config::attribute_value i_adjacent = (*i)["adjacent"];
			std::vector<map_location::DIRECTION> dirs = !i_adjacent.blank() ?
				map_location::parse_directions(i_adjacent) : map_location::default_dirs();
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
		if (!formula_man_->matches_filter(cfg_formula, loc, *this)) {
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

	formula_man_->write(cfg);


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
	cfg["alignment"] = lexical_cast<std::string> (alignment_);
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

void unit::set_facing(map_location::DIRECTION dir) const {
	if(dir != map_location::NDIRECTIONS) {
		facing_ = dir;
	}
	// Else look at yourself (not available so continue to face the same direction)
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
			generate_description = mod["generate_description"].to_bool();
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
					anim_comp_->apply_new_animation_effect(effect);
				} else if (apply_to == "ellipse") {
					cfg_["ellipse"] = effect["ellipse"];

				} else if (apply_to == "halo") {
					anim_comp_->clear_haloes();
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
				WRN_UT << "unknown type= in [effect]apply_to=type, ignoring" << std::endl;
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

		map_location adjs[6];
		get_adjacent_tiles(loc,adjs);

		BOOST_FOREACH(const map_location &u_loc, adjs)
		{
			unit_map::iterator u_it = resources::units->find(u_loc);
			if (!u_it.valid()) {
				continue;
			}
			unit & u = *u_it;
			if (teams[side_-1].is_enemy(u.side()) && !u.incapacitated()) {
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


bool unit::is_visible_to_team(team const& team, gamemap const& map, bool const see_all) const
{
	map_location const& loc = get_location();
	if (!map.on_board(loc))
		return false;
	if (see_all)
		return true;
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
			WRN_UT << "assigning new id to clone of generic unit " << id_ << std::endl;
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
	try {

	if(!resources::units->has_unit(&u_)) {
		/*
		 * It might be valid that the unit is not in the unit map.
		 * It might also mean a no longer valid unit will be assigned to.
		 */
		DBG_UT << "The unit to be removed is not in the unit map.\n";
	}
	u_.set_movement(moves_);

	} catch (...) {}
}

bool unit::matches_id(const std::string& unit_id) const
{
        return id_ == unit_id;
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


void unit::set_hidden(bool state) const {
	hidden_ = state;
	if(!state) return;
	// We need to get rid of haloes immediately to avoid display glitches
	anim_comp_->clear_haloes();
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

