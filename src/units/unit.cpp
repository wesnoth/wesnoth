/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
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

#include "units/unit.hpp"
#include "global.hpp"

#include "display_context.hpp"
#include "formula/string_utils.hpp"     // for vgettext
#include "game_board.hpp"               // for game_board
#include "game_data.hpp"
#include "game_config.hpp"              // for add_color_info, etc
#include "game_errors.hpp"              // for game_error
#include "game_events/manager.hpp"     // for add_events
#include "game_preferences.hpp"         // for encountered_units
#include "gettext.hpp"                  // for N_
#include "log.hpp"                      // for LOG_STREAM, logger, etc
#include "utils/make_enum.hpp"                // for operator<<, operator>>
#include "map/map.hpp"       // for gamemap
#include "random_new.hpp"               // for generator, rng
#include "resources.hpp"                // for units, gameboard, teams, etc
#include "scripting/game_lua_kernel.hpp"            // for game_lua_kernel
#include "synced_context.hpp"
#include "side_filter.hpp"              // for side_filter
#include "team.hpp"                     // for team, get_teams, etc
#include "terrain/filter.hpp"           // for terrain_filter
#include "units/abilities.hpp"           // for effect, filter_base_matches
#include "units/animation.hpp"           // for unit_animation
#include "units/animation_component.hpp"  // for unit_animation_component
#include "units/filter.hpp"
#include "units/formula_manager.hpp"     // for unit_formula_manager
#include "units/id.hpp"
#include "units/map.hpp"      // for unit_map, etc
#include "variable.hpp"                 // for vconfig, etc

#include <boost/bind.hpp>
#include <boost/foreach.hpp>            // for auto_any_base, etc
#include <boost/intrusive_ptr.hpp>      // for intrusive_ptr
#include <boost/function_output_iterator.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4510 4610)
#endif
#include <boost/range/algorithm.hpp>
#ifdef _MSC_VER
#pragma warning (pop)
#endif
#include <cassert>                     // for assert
#include <cstdlib>                     // for rand
#include <exception>                    // for exception
#include <iterator>                     // for back_insert_iterator, etc
#include <new>                          // for operator new
#include <ostream>                      // for operator<<, basic_ostream, etc
#include <SDL_video.h>                  // for SDL_Color


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
	// "advance" only kept around for backwards compatibility; only "advancement" should be used
	const std::string ModificationTypes[] = { "advancement", "advance", "trait", "object" };
	const size_t NumModificationTypes = sizeof(ModificationTypes)/
										sizeof(*ModificationTypes);

	/**
	 * Pointers to units which have data in their internal caches. The
	 * destructor of an unit removes itself from the cache, so the pointers are
	 * always valid.
	 */
	static std::vector<const unit *> units_with_cache;

	const std::string leader_crown_path = "misc/leader-crown.png";
	static std::string internalized_attrs[] = { "type", "id", "name",
		"gender", "random_gender", "variation", "role", "ai_special",
		"side", "underlying_id", "overlays", "facing", "race",
		"level", "recall_cost", "undead_variation", "max_attacks",
		"attacks_left", "alpha", "zoc", "flying", "cost",
		"max_hitpoints", "max_moves", "vision", "jamming", "max_experience",
		"advances_to", "hitpoints", "goto_x", "goto_y", "moves",
		"experience", "resting", "unrenamable", "alignment",
		"canrecruit", "extra_recruit", "x", "y", "placement",
		"parent_type", "description", "usage", "halo", "ellipse",
		"random_taits", "upkeep", "random_traits", "generate_name",
		"profile", "small_profile",
		// Useless attributes created when saving units to WML:
		"flag_rgb", "language_name", "image", "image_icon"
	};
	//Sort the array to make set_difference below work.
	struct t_internalized_attrs_sorter {
		t_internalized_attrs_sorter()
		{
			std::sort(boost::begin(internalized_attrs), boost::end(internalized_attrs));
		}
	} internalized_attrs_sorter;

	void warn_unknown_attribute(const config::const_attr_itors& cfg)
	{
		config::const_attribute_iterator cur = cfg.first;
		config::const_attribute_iterator end = cfg.second;
		const std::string* cur_known = boost::begin(internalized_attrs);
		const std::string* end_known = boost::end(internalized_attrs);
		while(cur_known != end_known) {
			if(cur == end) {
				return;
			}
			int comp = cur->first.compare(*cur_known);
			if(comp < 0) {
				WRN_UT << "Unknown attribute '" << cur->first << "' discarded." << std::endl;
				++cur;
			}
			else if (comp == 0) {
				++cur;
				++cur_known;
			}
			else {
				++cur_known;
			}
		}
		while(cur != end) {
			WRN_UT << "Unknown attribute '" << cur->first << "' discarded." << std::endl;
			++cur;
		}
	}
}

/**
 * Intrusive Pointer interface
 *
 **/

void intrusive_ptr_add_ref(const unit * u)
{
	assert(u->ref_count_ >= 0);
	// the next code line is to notice possible wrongly initialized units.
	// The 100000 is picked rather randomly. If you are in the situation
	// that you can actually have more then 100000 intrusive_ptr to one unit
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
		return genders[random_new::generator->get_random_int(0,genders.size()-1)];
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
namespace {
	template<typename T>
	T* copy_or_null(const boost::scoped_ptr<T>& ptr)
	{
		return ptr ? new T(*ptr) : nullptr;
	}
}
// Copy constructor
unit::unit(const unit& o)
	: ref_count_(0)
	, loc_(o.loc_)
	, advances_to_(o.advances_to_)
	, type_(o.type_)
	, type_name_(o.type_name_)
	, race_(o.race_)
	, id_(o.id_)
	, name_(o.name_)
	, underlying_id_(o.underlying_id_)
	, undead_variation_(o.undead_variation_)
	, variation_(o.variation_)
	, hit_points_(o.hit_points_)
	, max_hit_points_(o.max_hit_points_)
	, experience_(o.experience_)
	, max_experience_(o.max_experience_)
	, level_(o.level_)
	, recall_cost_(o.recall_cost_)
	, canrecruit_(o.canrecruit_)
	, recruit_list_(o.recruit_list_)
	, alignment_(o.alignment_)
	, flag_rgb_(o.flag_rgb_)
	, image_mods_(o.image_mods_)
	, unrenamable_(o.unrenamable_)
	, side_(o.side_)
	, gender_(o.gender_)
	, alpha_(o.alpha_)
	, formula_man_(new unit_formula_manager(o.formula_manager()))
	, movement_(o.movement_)
	, max_movement_(o.max_movement_)
	, vision_(o.vision_)
	, jamming_(o.jamming_)
	, movement_type_(o.movement_type_)
	, hold_position_(o.hold_position_)
	, end_turn_(o.end_turn_)
	, resting_(o.resting_)
	, attacks_left_(o.attacks_left_)
	, max_attacks_(o.max_attacks_)
	, states_(o.states_)
	, known_boolean_states_(o.known_boolean_states_)
	, variables_(o.variables_)
	, events_(o.events_)
	, filter_recall_(o.filter_recall_)
	, emit_zoc_(o.emit_zoc_)
	, overlays_(o.overlays_)
	, role_(o.role_)
	, attacks_(o.attacks_)
	, facing_(o.facing_)
	, trait_names_(o.trait_names_)
	, trait_descriptions_(o.trait_descriptions_)
	, unit_value_(o.unit_value_)
	, goto_(o.goto_)
	, interrupted_move_(o.interrupted_move_)
	, is_fearless_(o.is_fearless_)
	, is_healthy_(o.is_healthy_)
	, modification_descriptions_(o.modification_descriptions_)
	, anim_comp_(new unit_animation_component(*this, *o.anim_comp_))
	, getsHit_(o.getsHit_)
	, hidden_(o.hidden_)
	, hp_bar_scaling_(o.hp_bar_scaling_)
	, xp_bar_scaling_(o.xp_bar_scaling_)
	, modifications_(o.modifications_)
	, abilities_(o.abilities_)
	, advancements_(o.advancements_)
	, description_(o.description_)
	, usage_(copy_or_null(o.usage_))
	, halo_(copy_or_null(o.halo_))
	, ellipse_(copy_or_null(o.ellipse_))
	, random_traits_(o.random_traits_)
	, generate_name_(o.generate_name_)
	, upkeep_(o.upkeep_)
	, profile_(o.profile_)
	, small_profile_(o.small_profile_)
	, invisibility_cache_()
{
}

struct ptr_vector_pushback
{
	ptr_vector_pushback(boost::ptr_vector<config>&  vec) : vec_(&vec) {}
	void operator()(const config& cfg)
	{
		vec_->push_back(new config(cfg));
	}
	//Dont use reference to be copyable.
	boost::ptr_vector<config>* vec_;
};

unit::unit(const config &cfg, bool use_traits, const vconfig* vcfg, n_unit::id_manager* id_manager)
	: ref_count_(0)
	, loc_(cfg["x"] - 1, cfg["y"] - 1)
	, advances_to_()
	, type_(&get_unit_type(cfg["parent_type"].blank() ? cfg["type"] : cfg["parent_type"]))
	, type_name_()
	, race_(&unit_race::null_race)
	, id_(cfg["id"])
	, name_(cfg["name"].t_str())
	, underlying_id_(0)
	, undead_variation_()
	, variation_(cfg["variation"].empty() ? type_->default_variation() : cfg["variation"])
	, hit_points_(1)
	, max_hit_points_(0)
	, experience_(0)
	, max_experience_(0)
	, level_(0)
	, recall_cost_(-1)
	, canrecruit_(cfg["canrecruit"].to_bool())
	, recruit_list_()
	, alignment_()
	, flag_rgb_()
	, image_mods_()
	, unrenamable_(false)
	, side_(0)
	, gender_(generate_gender(*type_, cfg))
	, alpha_()
	, formula_man_(new unit_formula_manager())
	, movement_(0)
	, max_movement_(0)
	, vision_(-1)
	, jamming_(0)
	, movement_type_()
	, hold_position_(false)
	, end_turn_(false)
	, resting_(false)
	, attacks_left_(0)
	, max_attacks_(0)
	, states_()
	, known_boolean_states_(known_boolean_state_names_.size(),false)
	, variables_()
	, events_()
	, filter_recall_()
	, emit_zoc_(0)
	, overlays_()
	, role_(cfg["role"])
	, attacks_()
	, facing_(map_location::NDIRECTIONS)
	, trait_names_()
	, trait_descriptions_()
	, unit_value_()
	, goto_()
	, interrupted_move_()
	, is_fearless_(false)
	, is_healthy_(false)
	, modification_descriptions_()
	, anim_comp_(new unit_animation_component(*this))
	, getsHit_(0)
	, hidden_(false)
	, hp_bar_scaling_(cfg["hp_bar_scaling"].blank() ? type_->hp_bar_scaling() : cfg["hp_bar_scaling"])
	, xp_bar_scaling_(cfg["xp_bar_scaling"].blank() ? type_->xp_bar_scaling() : cfg["xp_bar_scaling"])
	, modifications_()
	, abilities_()
	, advancements_()
	, description_()
	, usage_()
	, halo_()
	, ellipse_()
	, random_traits_(true)
	, generate_name_(true)
	, upkeep_()
	, invisibility_cache_()
{
	side_ = cfg["side"];
	if(side_ <= 0) {
		side_ = 1;
	}

	validate_side(side_);
	underlying_id_ = n_unit::unit_id::create_real(cfg["underlying_id"].to_int());
	set_underlying_id(id_manager ? *id_manager : resources::gameboard->unit_id_manager());

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

	if (resources::game_events) {
		resources::game_events->add_events(events_.child_range("event"));
	}

	random_traits_ = cfg["random_traits"].to_bool(true);
	facing_ = map_location::parse_direction(cfg["facing"]);
	if(facing_ == map_location::NDIRECTIONS) facing_ = static_cast<map_location::DIRECTION>(rand()%map_location::NDIRECTIONS);

	if (const config &mods = cfg.child("modifications")) {
		modifications_ = mods;
	}
	generate_name_ = cfg["generate_name"].to_bool(true);
	// Apply the unit type's data to this unit.
	advance_to(*type_, use_traits);

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
		description_ = *v;
	}
	if (const config::attribute_value *v = cfg.get("cost")) {
		unit_value_ = *v;
	}
	if (const config::attribute_value *v = cfg.get("ellipse")) {
		set_image_ellipse(*v);
	}
	if (const config::attribute_value *v = cfg.get("halo")) {
		set_image_halo(*v);
	}
	if (const config::attribute_value *v = cfg.get("usage")) {
		set_usage(*v);
	}

	if (const config::attribute_value *v = cfg.get("profile")) {
		std::string profile = (*v).str();
		adjust_profile(profile);
		profile_ = profile;
	}
	if (const config::attribute_value *v = cfg.get("small_profile")) {
		small_profile_ = (*v).str();
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
		this->advancements_.clear();
		boost::copy( cfg.child_range("advancement"), boost::make_function_output_iterator(ptr_vector_pushback(advancements_)));
	}

	//don't use the unit_type's abilities if this config has its own defined
	//Why do we allow multiple [abilities] tags?
	cfg_range = cfg.child_range("abilities");
	if(cfg_range.first != cfg_range.second) {
		this->abilities_.clear();
		BOOST_FOREACH(const config& abilities, cfg_range)
		{
			this->abilities_.append(abilities);
		}
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

	alignment_.parse(cfg["alignment"].str());

	generate_name();

	parse_upkeep(cfg["upkeep"]);

	set_recruits(utils::split(cfg["extra_recruit"]));

	game_config::add_color_info(cfg);
	warn_unknown_attribute(cfg.attribute_range());
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

unit::unit(const unit_type &u_type, int side, bool real_unit, unit_race::GENDER gender)
	: ref_count_(0)
	, loc_()
	, advances_to_()
	, type_(&u_type)
	, type_name_()
	, race_(&unit_race::null_race)
	, id_()
	, name_()
	, underlying_id_(real_unit? n_unit::unit_id(0) : resources::gameboard->unit_id_manager().next_fake_id())
	, undead_variation_()
	, variation_(type_->default_variation())
	, hit_points_(0)
	, max_hit_points_(0)
	, experience_(0)
	, max_experience_(0)
	, level_(0)
	, recall_cost_(-1)
	, canrecruit_(false)
	, recruit_list_()
	, alignment_()
	, flag_rgb_()
	, image_mods_()
	, unrenamable_(false)
	, side_(side)
	, gender_(gender != unit_race::NUM_GENDERS ?
		gender : generate_gender(u_type, real_unit))
	, alpha_()
	, formula_man_(new unit_formula_manager())
	, movement_(0)
	, max_movement_(0)
	, vision_(-1)
	, jamming_(0)
	, movement_type_()
	, hold_position_(false)
	, end_turn_(false)
	, resting_(false)
	, attacks_left_(0)
	, max_attacks_(0)
	, states_()
	, known_boolean_states_( get_known_boolean_state_names().size(),false)
	, variables_()
	, events_()
	, filter_recall_()
	, emit_zoc_(0)
	, overlays_()
	, role_()
	, attacks_()
	, facing_(static_cast<map_location::DIRECTION>(rand()%map_location::NDIRECTIONS))
	, trait_names_()
	, trait_descriptions_()
	, unit_value_()
	, goto_()
	, interrupted_move_()
	, is_fearless_(false)
	, is_healthy_(false)
	, modification_descriptions_()
	, anim_comp_(new unit_animation_component(*this))
	, getsHit_(0)
	, hidden_(false)
	, modifications_()
	, abilities_()
	, advancements_()
	, description_()
	, usage_()
	, halo_()
	, ellipse_()
	, random_traits_(true)
	, generate_name_(true)
	, upkeep_()
	, invisibility_cache_()
{
	upkeep_ = upkeep_full();

	// Apply the unit type's data to this unit.
	advance_to(u_type, real_unit);

	if(real_unit) {
		generate_name();
	}
	set_underlying_id(resources::gameboard->unit_id_manager());

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
 * Swap, for copy and swap idiom
 */
void unit::swap(unit & o)
{
	using std::swap;

	// Don't swap reference count, or it will be incorrect...
	swap(loc_, o.loc_);
	swap(advances_to_, o.advances_to_);
	swap(type_, o.type_);
	swap(type_name_, o.type_name_);
	swap(race_, o.race_);
	swap(id_, o.id_);
	swap(name_, o.name_);
	swap(underlying_id_, o.underlying_id_);
	swap(undead_variation_, o.undead_variation_);
	swap(variation_, o.variation_);
	swap(hit_points_, o.hit_points_);
	swap(max_hit_points_, o.max_hit_points_);
	swap(experience_, o.experience_);
	swap(max_experience_, o.max_experience_);
	swap(level_, o.level_);
	swap(recall_cost_, o.recall_cost_);
	swap(canrecruit_, o.canrecruit_);
	swap(recruit_list_, o.recruit_list_);
	swap(alignment_, o.alignment_);
	swap(flag_rgb_, o.flag_rgb_);
	swap(image_mods_, o.image_mods_);
	swap(unrenamable_, o.unrenamable_);
	swap(side_, o.side_);
	swap(gender_, o.gender_);
	swap(alpha_, o.alpha_);
	swap(formula_man_, o.formula_man_);
	swap(movement_, o.movement_);
	swap(max_movement_, o.max_movement_);
	swap(vision_, o.vision_);
	swap(jamming_, o.jamming_);
	swap(movement_type_, o.movement_type_);
	swap(hold_position_, o.hold_position_);
	swap(end_turn_, o.end_turn_);
	swap(resting_, o.resting_);
	swap(attacks_left_, o.attacks_left_);
	swap(max_attacks_, o.max_attacks_);
	swap(states_, o.states_);
	swap(known_boolean_states_, o.known_boolean_states_);
	swap(variables_, o.variables_);
	swap(events_, o.events_);
	swap(filter_recall_, o.filter_recall_);
	swap(emit_zoc_, o.emit_zoc_);
	swap(overlays_, o.overlays_);
	swap(role_, o.role_);
	swap(attacks_, o.attacks_);
	swap(facing_, o.facing_);
	swap(trait_names_, o.trait_names_);
	swap(trait_descriptions_, o.trait_descriptions_);
	swap(unit_value_, o.unit_value_);
	swap(goto_, o.goto_);
	swap(interrupted_move_, o.interrupted_move_);
	swap(is_fearless_, o.is_fearless_);
	swap(is_healthy_, o.is_healthy_);
	swap(modification_descriptions_, o.modification_descriptions_);
	swap(anim_comp_, o.anim_comp_);
	swap(getsHit_, o.getsHit_);
	swap(hidden_, o.hidden_);
	swap(modifications_, o.modifications_);
	swap(invisibility_cache_, o.invisibility_cache_);
}

/**
 * Assignment operator.
 */
unit& unit::operator=(unit other)
{
	swap(other);
	return *this;
}


void unit::generate_name()
{
	if (!name_.empty() || !generate_name_) {
		return;
	}
	name_ = race_->generate_name(gender_);
	generate_name_ = false;
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
		int num = random_new::generator->get_random_int(0,candidate_traits.size()-1);
		modifications_.add_child("trait", candidate_traits[num]);
		candidate_traits.erase(candidate_traits.begin() + num);
	}

	// Once random traits are added, don't do it again.
	// Such as when restoring a saved character.
	random_traits_ = false;
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
void unit::advance_to(const unit_type &u_type,
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

	// build unit type ready to create units. Not sure if needed.
	new_type.get_cfg_for_units();

	if(!new_type.usage().empty()) {
		 set_usage(new_type.usage());
	}
	set_image_halo(new_type.halo());
	set_image_ellipse(new_type.ellipse());
	generate_name_ &= new_type.generate_name();
	abilities_ = new_type.abilities_cfg();
	advancements_.clear();
	BOOST_FOREACH(const config& advancement, new_type.advancements()) {
		advancements_.push_back(new config(advancement));
	}
 	// If unit has specific profile, remember it and keep it after advancing
	if(small_profile_.empty() || small_profile_ == old_type.small_profile()) {
		small_profile_ = new_type.small_profile();
	}
	if(profile_.empty() || profile_ == old_type.big_profile()) {
		profile_ = new_type.big_profile();
	}
	// NOTE: There should be no need to access old_cfg (or new_cfg) after this
	//       line. Particularly since the swap might have affected old_cfg.

	advances_to_ = new_type.advances_to();

	race_ = new_type.race();
	type_ = &new_type;
	type_name_ = new_type.type_name();
	description_ = new_type.unit_description();
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

	anim_comp_->reset_after_advance(&new_type);

	if (random_traits_) {
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
	if (resources::game_events) {
		resources::game_events->add_events(new_type.events(), new_type.id());
	}
}

std::string unit::big_profile() const
{
	if (!profile_.empty() && profile_ != "unit_image") {
		return profile_;
	}
	return absolute_image();
}

std::string unit::small_profile() const
{
	if (!small_profile_.empty() && small_profile_ != "unit_image") {
		return small_profile_;
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
	const unit_type * rebuild_from = nullptr;

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
				else if ( rebuild_from == nullptr )
					rebuild_from = &type();

				modifications_.remove_child(mod_name, j);
			}
		}
	}

	if ( rebuild_from != nullptr ) {
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
	expire_modifications("turn end");

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
	BOOST_FOREACH(const config::any_child &ab, this->abilities_.all_children_range()) {
		if (ab.cfg["id"] == ability) {
			return true;
		}
	}
	return false;
}

void unit::remove_ability_by_id(const std::string &ability)
{
	config::all_children_iterator i = this->abilities_.ordered_begin();
	while (i != this->abilities_.ordered_end()) {
		if (i->cfg["id"] == ability) {
			i = this->abilities_.erase(i);
		} else {
			++i;
		}
	}
}

void unit::write(config& cfg) const
{
	movement_type_.write(cfg);
	cfg["small_profile"] = small_profile_;
	cfg["profile"] = profile_;
	if ( description_ != type().unit_description() ) {
		cfg["description"] = description_;
	}
	if(halo_.get()) {
		cfg["halo"] = *halo_;
	}
	if(ellipse_.get()) {
		cfg["ellipse"] = *ellipse_;
	}
	if(usage_.get()) {
		cfg["usage"] = *usage_;
	}
	write_upkeep(cfg["upkeep"]);
	cfg["hitpoints"] = hit_points_;
	cfg["max_hitpoints"] = max_hit_points_;

	cfg["image_icon"] = type().icon();
	cfg["image"] = type().image();
	cfg["random_traits"] = random_traits_;
	cfg["generate_name"] = generate_name_;
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
	cfg["underlying_id"] = underlying_id_.value;
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
	cfg["alignment"] = alignment_.to_string();
	cfg["flag_rgb"] = flag_rgb_;
	cfg["unrenamable"] = unrenamable_;
	cfg["alpha"] = std::to_string(alpha_);

	cfg["attacks_left"] = attacks_left_;
	cfg["max_attacks"] = max_attacks_;
	cfg["zoc"] = emit_zoc_;
	cfg.clear_children("attack");
	for(std::vector<attack_type>::const_iterator i = attacks_.begin(); i != attacks_.end(); ++i) {
		i->write(cfg.add_child("attack"));
	}
	cfg["cost"] = unit_value_;
	cfg.clear_children("modifications");
	cfg.add_child("modifications", modifications_);
	cfg.clear_children("abilities");
	cfg.add_child("abilities", abilities_);
	cfg.clear_children("advancement");
	BOOST_FOREACH(const config& advancement, this->advancements_)
	{
		cfg.add_child("advancement", advancement);
	}

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
	else if(boost::get<upkeep_full>(&upkeep_) != nullptr) {
		return level();
	}
	else if(boost::get<upkeep_loyal>(&upkeep_) != nullptr) {
		return 0;
	}
	else {
		return boost::get<int>(upkeep_);
	}
}

bool unit::loyal() const
{
	return boost::get<upkeep_loyal>(&upkeep_) != nullptr;
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

		for (unsigned j = 0, j_count = modification_count("advancement", adv["id"]);
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
		if (modification_count("advancement", adv["id"]) >= unsigned(adv["max_times"].to_int(1)))
			continue;

		std::vector<std::string> temp_require = utils::split(adv["require_amla"]);
		std::vector<std::string> temp_exclude = utils::split(adv["exclude_amla"]);
		if (temp_require.empty() && temp_exclude.empty()) {
			res.push_back(adv);
			continue;
		}

		std::sort(temp_require.begin(), temp_require.end());
		std::sort(temp_exclude.begin(), temp_exclude.end());
		std::vector<std::string> uniq_require, uniq_exclude;
		std::unique_copy(temp_require.begin(), temp_require.end(), std::back_inserter(uniq_require));
		std::unique_copy(temp_exclude.begin(), temp_exclude.end(), std::back_inserter(uniq_exclude));

		bool exclusion_found = false;
		BOOST_FOREACH(const std::string &s, uniq_exclude)
		{
			int max_num = std::count(temp_exclude.begin(), temp_exclude.end(), s);
			int mod_num = modification_count("advancement", s);
			if (mod_num >= max_num) {
				exclusion_found = true;
				break;
			}
		}
		if (exclusion_found) {
			continue;
		}

		bool requirements_done = true;
		BOOST_FOREACH(const std::string &s, uniq_require)
		{
			int required_num = std::count(temp_require.begin(), temp_require.end(), s);
			int mod_num = modification_count("advancement", s);
			if (required_num > mod_num) {
				requirements_done = false;
				break;
			}
		}
		if (requirements_done) {
			res.push_back(adv);
		}
	}

	return res;
}

void unit::set_advancements(std::vector<config> advancements)
{
	this->advancements_.clear();
	BOOST_FOREACH(config& advancement, advancements)
	{
		this->advancements_.push_back(new config());
		this->advancements_.back().swap(advancement);
	}
}

size_t unit::modification_count(const std::string& mod_type, const std::string& id) const
{
	size_t res = 0;
	BOOST_FOREACH(const config &item, modifications_.child_range(mod_type)) {
		if (item["id"] == id) {
			++res;
		}
	}

	// For backwards compatibility, if asked for "advancement", also count "advance"
	if (mod_type == "advancement") {
		res += modification_count("advance", id);
	}

	return res;
}

const std::set<std::string> unit::builtin_effects = {
	"alignment", "attack", "defense", "ellipse", "experience", "fearless",
	"halo", "healthy", "hitpoints", "image_mod", "jamming", "jamming_costs",
	"loyal", "max_attacks", "max_experience", "movement", "movement_costs",
	"new_ability", "new_advancement", "new_animation", "new_attack", "overlay", "profile",
	"recall_cost", "remove_ability", "remove_advancement", "remove_attacks", "resistance",
	"status", "type", "variation", "vision", "vision_costs", "zoc"};

std::string unit::describe_builtin_effect(std::string apply_to, const config& effect)
{
	if(apply_to == "attack") {
		std::string attack_names;
		bool first_attack = true;

		std::string desc;
		for(std::vector<attack_type>::iterator a = attacks_.begin();
			a != attacks_.end(); ++a) {
			bool affected = a->describe_modification(effect, &desc);
			if(affected && desc != "") {
				if(first_attack) {
					first_attack = false;
				} else {
					attack_names += t_string(N_(" and "), "wesnoth");
				}

				attack_names += t_string(a->name(), "wesnoth-units");
			}
		}
		if (!attack_names.empty()) {
			utils::string_map symbols;
			symbols["attack_list"] = attack_names;
			symbols["effect_description"] = desc;
			return vgettext("$attack_list|: $effect_description", symbols);
		}
	} else if(apply_to == "hitpoints") {
		const std::string &increase_total = effect["increase_total"];

		if(!increase_total.empty()) {
			return utils::print_modifier(increase_total) + " " +
			t_string(N_("HP"), "wesnoth");
		}
	} else if(apply_to == "movement") {
		const std::string &increase = effect["increase"];

		if(!increase.empty()) {
			int n = lexical_cast<int>(increase);
			return utils::print_modifier(increase) + " " +
				_n("move", "moves", n);
		}
	} else if(apply_to == "vision") {
		const std::string &increase = effect["increase"];

		if(!increase.empty()) {
			return utils::print_modifier(increase) + " " + t_string(N_("vision"), "wesnoth");
		}
	} else if(apply_to == "jamming") {
		const std::string &increase = effect["increase"];

		if(!increase.empty()) {
			return utils::print_modifier(increase) + " " + t_string(N_("jamming"), "wesnoth");
		}
	} else if(apply_to == "max_experience") {
		const std::string &increase = effect["increase"];

		if(!increase.empty()) {
			return utils::print_modifier(increase) + " " +
			t_string(N_("XP to advance"), "wesnoth");
		}
	} else if (apply_to == "max_attacks") {
		const std::string &increase = effect["increase"];

		std::string description = utils::print_modifier(increase) + " ";
		const char* const singular = N_("attack per turn");
		const char* const plural = N_("attacks per turn");
		if (increase[increase.size()-1] == '%' || abs(lexical_cast<int>(increase)) != 1) {
			description += t_string(plural, "wesnoth");
		} else {
			description += t_string(singular, "wesnoth");
		}
		return description;
	} else if (apply_to == "recall_cost") {
		const std::string &increase = effect["increase"];
		return utils::print_modifier(increase) + " " +
				t_string(N_("cost to recall"), "wesnoth");
	}
	return "";
}

void unit::apply_builtin_effect(std::string apply_to, const config& effect)
{
	if(apply_to == "fearless")
	{
		is_fearless_ = effect["set"].to_bool(true);
	}
	else if(apply_to == "healthy")
	{
		is_healthy_ = effect["set"].to_bool(true);
	}
	else if(apply_to == "profile") {
		if (const config::attribute_value *v = effect.get("portrait")) {
			std::string portrait = (*v).str();
			adjust_profile(portrait);
			profile_ = portrait;
		}
		if (const config::attribute_value *v = effect.get("small_portrait")) {
			small_profile_ = (*v).str();
		}
		if (const config::attribute_value *v = effect.get("description")) {
			description_ = *v;
		}
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
		for(std::vector<attack_type>::iterator a = attacks_.begin();
			a != attacks_.end(); ++a) {
			a->apply_modification(effect);
		}
	} else if(apply_to == "hitpoints") {
		LOG_UT << "applying hitpoint mod..." << hit_points_ << "/" << max_hit_points_ << "\n";
		const std::string &increase_hp = effect["increase"];
		const std::string &increase_total = effect["increase_total"];
		const std::string &set_hp = effect["set"];
		const std::string &set_total = effect["set_total"];

		// If the hitpoints are allowed to end up greater than max hitpoints
		const bool violate_max = effect["violate_maximum"].to_bool();

		if(!set_hp.empty()) {
			if(set_hp[set_hp.size()-1] == '%') {
				hit_points_ = lexical_cast_default<int>(set_hp)*max_hit_points_/100;
			} else {
				hit_points_ = lexical_cast_default<int>(set_hp);
			}
		}
		if(!set_total.empty()) {
			if(set_total[set_total.size()-1] == '%') {
				max_hit_points_ = lexical_cast_default<int>(set_total)*max_hit_points_/100;
			} else {
				max_hit_points_ = lexical_cast_default<int>(set_total);
			}
		}

		if(!increase_total.empty()) {
			// A percentage on the end means increase by that many percent
			max_hit_points_ = utils::apply_modifier(max_hit_points_, increase_total);
		}

		if(max_hit_points_ < 1)
			max_hit_points_ = 1;

		if (effect["heal_full"].to_bool()) {
			heal_all();
		}

		if(!increase_hp.empty()) {
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

		if(!increase.empty()) {
			max_movement_ = utils::apply_modifier(max_movement_, increase, 1);
		}

		max_movement_ = effect["set"].to_int(max_movement_);

		if(movement_ > max_movement_)
			movement_ = max_movement_;
	} else if(apply_to == "vision") {
		const std::string &increase = effect["increase"];

		if(!increase.empty()) {
			const int current_vision = vision_ < 0 ? max_movement_ : vision_;
			vision_ = utils::apply_modifier(current_vision, increase, 1);
		}

		vision_ = effect["set"].to_int(vision_);
	} else if(apply_to == "jamming") {
		const std::string &increase = effect["increase"];

		if(!increase.empty()) {
			jamming_ = utils::apply_modifier(jamming_, increase, 1);
		}

		jamming_ = effect["set"].to_int(jamming_);
	} else if(apply_to == "experience") {
		const std::string &increase = effect["increase"];
		const std::string &set = effect["set"];

		if(!set.empty()) {
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
		const std::string &set = effect["set"];

		if(set.empty() == false) {
			if(set[set.size()-1] == '%') {
				max_experience_ = lexical_cast_default<int>(set)*max_experience_/100;
			} else {
				max_experience_ = lexical_cast_default<int>(set);
			}
		}

		if(increase.empty() == false) {
			max_experience_ = utils::apply_modifier(max_experience_, increase, 1);
		}
	} else if(apply_to == "loyal") {
		upkeep_ = upkeep_loyal();;
	} else if(apply_to == "status") {
		const std::string& add = effect["add"];
		const std::string& remove = effect["remove"];

		BOOST_FOREACH(const std::string& to_add, utils::split(add))
		{
			set_state(to_add, true);
		}

		BOOST_FOREACH(const std::string& to_remove, utils::split(remove))
		{
			set_state(to_remove, false);
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
		if (const config &ab_effect = effect.child("abilities")) {
			config to_append;
			BOOST_FOREACH(const config::any_child &ab, ab_effect.all_children_range()) {
				if(!has_ability_by_id(ab.cfg["id"])) {
					to_append.add_child(ab.key, ab.cfg);
				}
			}
			this->abilities_.append(to_append);
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
		set_image_ellipse(effect["ellipse"]);
	} else if (apply_to == "halo") {
		set_image_halo(effect["halo"]);
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
	} else if (apply_to == "new_advancement") {
		const std::string &types = effect["types"];
		const bool replace = effect["replace"].to_bool(false);

		if (!types.empty()) {
			if (replace) {
				advances_to_ = utils::parenthetical_split(types, ',');
			} else {
				std::vector<std::string> temp_advances = utils::parenthetical_split(types, ',');
				std::copy(temp_advances.begin(), temp_advances.end(), std::back_inserter(advances_to_));
			}
		}

		if (effect.has_child("advancement")) {
			if (replace) {
				advancements_.clear();
			}
			config temp = effect;
			boost::copy(effect.child_range("advancement"), boost::make_function_output_iterator(ptr_vector_pushback(advancements_)));
		}
	} else if (apply_to == "remove_advancement") {
		const std::string &types = effect["types"];
		const std::string &amlas = effect["amlas"];

		std::vector<std::string> temp_advances = utils::parenthetical_split(types, ',');
		std::vector<std::string>::iterator iter;
		BOOST_FOREACH(const std::string& unit, temp_advances) {
			iter = std::find(advances_to_.begin(), advances_to_.end(), unit);
			if (iter != advances_to_.end()) {
				advances_to_.erase(iter);
			}
		}

		temp_advances = utils::parenthetical_split(amlas, ',');
		std::vector<size_t> remove_indices;

		for(int i = advancements_.size() - 1; i >= 0; i--) {
			if(std::find(temp_advances.begin(), temp_advances.end(), advancements_[i]["id"]) != temp_advances.end()) {
				advancements_.erase(advancements_.begin() + i);
			}
		}
	} else if (apply_to == "alignment") {
		unit_type::ALIGNMENT new_align;
		if(new_align.parse(effect["set"])) {
			alignment_ = new_align;
		}
	} else if (apply_to == "max_attacks") {
		const std::string &increase = effect["increase"];

		if(!increase.empty()) {
			max_attacks_ = utils::apply_modifier(max_attacks_, increase, 1);
		}
	} else if (apply_to == "recall_cost") {
		const std::string &increase = effect["increase"];
		const std::string &set = effect["set"];
		const int recall_cost = recall_cost_ < 0 ? resources::teams->at(side_).recall_cost() : recall_cost_;

		if(!set.empty()) {
			if(set[set.size()-1] == '%') {
				recall_cost_ = lexical_cast_default<int>(set)*recall_cost/100;
			} else {
				recall_cost_ = lexical_cast_default<int>(set);
			}
		}

		if(!increase.empty()) {
			recall_cost_ = utils::apply_modifier(recall_cost, increase, 1);
		}
	} else if (effect["apply_to"] == "variation") {
		variation_ = effect["name"].str();
		const unit_type * base_type = unit_types.find(type().base_id());
		assert(base_type != nullptr);
		advance_to(*base_type);
	} else if (effect["apply_to"] == "type") {
		std::string prev_type = effect["prev_type"];
		if (prev_type.empty()) {
			prev_type = type().base_id();
		}
		const std::string& new_type_id = effect["name"];
		const unit_type* new_type = unit_types.find(new_type_id);
		if (new_type) {
			const bool heal_full = effect["heal_full"].to_bool(false);
			advance_to(*new_type);
			preferences::encountered_units().insert(new_type_id);
			if(heal_full) {
				heal_all();
			}
		} else {
			WRN_UT << "unknown type= in [effect]apply_to=type, ignoring" << std::endl;
		}
	}
}

void unit::add_modification(const std::string& mod_type, const config& mod, bool no_add)
{
	bool generate_description = mod["generate_description"].to_bool(true);

	if(no_add == false) {
		modifications_.add_child(mod_type, mod);
	}
	bool set_poisoned = false; // Tracks if the poisoned state was set after the type or variation was changed.
	config last_effect;
	std::vector<t_string> effects_description;
	BOOST_FOREACH(const config &effect, mod.child_range("effect"))
	{
		// Apply SUF.
		if (const config &afilter = effect.child("filter")) {
			// @FIXME: during gamestate construction resources::filter_con is not available
			if (resources::filter_con && !unit_filter(vconfig(afilter), resources::filter_con).matches(*this, loc_)) continue;
		}
		const std::string &apply_to = effect["apply_to"];
		int times = effect["times"].to_int(1);
		t_string description;

		if (effect["times"] == "per level")
			times = level_;

		if (times) {
			while (times > 0) {
				times --;

				bool was_poisoned = get_state(STATE_POISONED);
				if ((apply_to == "variation" || apply_to == "type") && no_add == false) {
					// Apply unit type/variation changes last to avoid double applying effects on advance.
					set_poisoned = false;
					last_effect = effect;
					continue;
				}
				std::string description_component;
				if (resources::lua_kernel) {
					description_component = resources::lua_kernel->apply_effect(apply_to, *this, effect, true);
				} else if (builtin_effects.count(apply_to)) {
					// Normally, the built-in effects are dispatched through Lua so that a user
					// can override them if desired. However, since they're built-in, we can still
					// apply them if the lua kernel is unavailable.
					apply_builtin_effect(apply_to, effect);
					description_component = describe_builtin_effect(apply_to, effect);
				}
				if (!times) {
					description += description_component;
				}
				if (!was_poisoned && get_state(STATE_POISONED)) {
					set_poisoned = true;
				} else if(was_poisoned && !get_state(STATE_POISONED)) {
					set_poisoned = false;
				}
			} // end while
		} else { // for times = per level & level = 0 we still need to rebuild the descriptions
			if (resources::lua_kernel) {
				description += resources::lua_kernel->apply_effect(apply_to, *this, effect, false);
			} else if(builtin_effects.count(apply_to)) {
				description += describe_builtin_effect(apply_to, effect);
			}
		}

		if (effect["times"] == "per level" && !times) {
			utils::string_map symbols;
			symbols["effect_description"] = description;
			description = vgettext("$effect_description per level", symbols);
		}
		if(!description.empty())
			effects_description.push_back(description);

	}
	// Apply variations -- only apply if we are adding this for the first time.
	if (!last_effect.empty() && no_add == false) {
		std::string description;
		if (resources::lua_kernel) {
			description = resources::lua_kernel->apply_effect(last_effect["apply_to"], *this, last_effect, true);
		} else if (builtin_effects.count(last_effect["apply_to"])) {
			apply_builtin_effect(last_effect["apply_to"], last_effect);
			description = describe_builtin_effect(last_effect["apply_to"], last_effect);
		}
		effects_description.push_back(description);
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
			if (i->empty()) {
				continue;
			}
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

	return type().icon().empty() ? type().image() : type().icon();
}

std::string unit::default_anim_image() const {
	return type().image().empty() ? type().icon() : type().image();
}

void unit::apply_modifications()
{
	log_scope("apply mods");

	for(size_t i = 0; i != NumModificationTypes; ++i) {
		const std::string& mod = ModificationTypes[i];
		if(mod == "advance" && modifications_.has_child(mod)) {
			lg::wml_error() << "[modifications][advance] is deprecated, use [advancement] instead\n";
		}
		BOOST_FOREACH(const config &m, modifications_.child_range(mod)) {
			log_scope("add mod");
			add_modification(ModificationTypes[i], m, true);
		}
	}

	//apply the experience acceleration last
	int exp_accel = unit_experience_accelerator::get_acceleration();
	max_experience_ = std::max<int>(1, (max_experience_ * exp_accel + 50)/100);
}

bool unit::invisible(const map_location& loc, bool see_all) const
{
	if (loc != get_location()) {
		DBG_UT << "unit::invisible called: id = " << id() << " loc = " << loc << " get_loc = " << get_location() << std::endl;
	}

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
		is_inv = (resources::gameboard ? !resources::gameboard->would_be_discovered(loc, side_,see_all) : true);
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
	// allied planned moves are also visible under fog. (we assume that fake units on the map are always whiteboard markers)
	if (!team.is_enemy(side()) && underlying_id_.is_fake())
		return true;
	if (team.fogged(loc))
		return false;
	return true;
}

void unit::set_underlying_id(n_unit::id_manager& id_manager)
{
	if(underlying_id_.value == 0) {

		if(synced_context::is_synced() || !resources::gamedata || resources::gamedata->phase() == game_data::INITIAL) {
			underlying_id_ = id_manager.next_id();
		}
		else {
			underlying_id_ = id_manager.next_fake_id();
		}
	}
	if (id_.empty() /*&& !underlying_id_.is_fake()*/) {
		std::stringstream ss;
		ss << (type_id().empty() ? "Unit" : type_id()) << "-" << underlying_id_.value;
		id_ = ss.str();
	}
}

unit& unit::clone(bool is_temporary)
{
	if(is_temporary) {
		underlying_id_ = resources::gameboard->unit_id_manager().next_fake_id();
	} else {
		if(synced_context::is_synced() || !resources::gamedata || resources::gamedata->phase() == game_data::INITIAL) {
			underlying_id_ = resources::gameboard->unit_id_manager().next_id();
		}
		else {
			underlying_id_ = resources::gameboard->unit_id_manager().next_fake_id();
		}
		std::string::size_type pos = id_.find_last_of('-');
		if(pos != std::string::npos && pos+1 < id_.size()
		&& id_.find_first_not_of("0123456789", pos+1) == std::string::npos) {
			// this appears to be a duplicate of a generic unit, so give it a new id
			WRN_UT << "assigning new id to clone of generic unit " << id_ << std::endl;
			id_.clear();
			set_underlying_id(resources::gameboard->unit_id_manager());
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

void unit::set_image_halo(const std::string& halo)
{
	anim_comp_->clear_haloes();
	halo_.reset(new std::string(halo));
}



void unit::parse_upkeep(const config::attribute_value& upkeep)
{
	if (upkeep.empty()) {
		return;
	}
	//TODO: create abetter way to check whether it is actually an int.
	int upkeep_int = upkeep.to_int(-99);
	if(upkeep_int != -99) {
		upkeep_ = upkeep_int;
	}
	else if(upkeep == "loyal" || upkeep == "free") {
		upkeep_ = upkeep_loyal();
	}
	else if(upkeep == "full") {
		upkeep_ = upkeep_full();
	}
	else {
		WRN_UT << "Fund invalid upkeep=\"" << upkeep <<  "\" in a unit\n";
		upkeep_ = upkeep_full();
	}
}
void unit::write_upkeep(config::attribute_value& upkeep) const
{
	if(boost::get<upkeep_full>(&upkeep_) != nullptr) {
		upkeep = "full";
	}
	else if(boost::get<upkeep_loyal>(&upkeep_) != nullptr) {
		upkeep = "loyal";
	}
	else {
		upkeep = boost::get<int>(upkeep_);
	}
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

	const std::string child_keys[] = {"advance_from", "defense", "movement_costs", "vision_costs", "jamming_costs", "resistance", ""};

	for (int i = 0; !child_keys[i].empty(); ++i)
	{
		BOOST_FOREACH(const config &c, unit_config.child_range(child_keys[i])) {
			wcfg.add_child(child_keys[i], c);
		}
	}
	DBG_UT << wcfg;

	return wcfg.hash();
}

