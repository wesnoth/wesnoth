/* $Id$ */
/*
   Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
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

#include "callable_objects.hpp"
#include "foreach.hpp"
#include "formula.hpp"
#include "game_display.hpp"
#include "game_preferences.hpp"
#include "gamestatus.hpp"
#include "gettext.hpp"
#include "halo.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "unit_id.hpp"
#include "unit_abilities.hpp"
#include "terrain_filter.hpp"
#include "formula_string_utils.hpp"
#include "scripting/lua.hpp"
#include "play_controller.hpp"

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



namespace{
	//Static tokens are replacements for string literals in code
	//They allow for fast comparison operations.
	static const config::t_token z_gender("gender", false);
	static const config::t_token z_type("type", false);
	static const config::t_token z_id("id", false);
	static const config::t_token z_name("name", false);
	static const config::t_token z_random_gender("random_gender", false);
	static const config::t_token z_x("x", false);
	static const config::t_token z_y("y", false);
	static const config::t_token z_variation("variation", false);
	static const config::t_token z_canrecruit("canrecruit", false);
	static const config::t_token z_role("role", false);
	static const config::t_token z_ai_special("ai_special", false);
	static const config::t_token z_side("side", false);
	static const config::t_token z_underlying_id("underlying_id", false);
	static const config::t_token z_overlays("overlays", false);
	static const config::t_token z_variables("variables", false);
	static const config::t_token z_facing("facing", false);
	static const config::t_token z_flying("flying", false);
	static const config::t_token z_modification("modifications", false);
	static const config::t_token z_race("race", false);
	static const config::t_token z_undead_variation("undead_variation", false);
	static const config::t_token z_max_attacks("max_attacks", false);
	static const config::t_token z_attacks_left("attacks_left", false);
	static const config::t_token z_alpha("alpha", false);
	static const config::t_token z_zoc("zoc", false);
	//	static const config::t_token z_description("description", false);
	static const config::t_token z_cost("cost", false);
	//	static const config::t_token z_halo("halo", false);
	static const config::t_token z_profile("profile", false);
	static const config::t_token z_small_profile("small_profile", false);
	static const config::t_token z_max_hitpoints("max_hitpoints", false);
	static const config::t_token z_max_moves("max_moves", false);
	static const config::t_token z_max_experience("max_experience", false);
	static const config::t_token z_advances_to("advances_to", false);
	static const config::t_token z_null("null", false);
	static const config::t_token z_ai("ai", false);
	static const config::t_token z_formula("formula", false);
	static const config::t_token z_loop_formula("loop_formula", false);
	static const config::t_token z_priority("priority", false);
	static const config::t_token z_vars("vars", false);
	static const config::t_token z_guardian("guardian", false);
	static const config::t_token z_hitpoints("hitpoints", false);
	static const config::t_token z_goto_x("goto_x", false);
	static const config::t_token z_goto_y("goto_y", false);
	static const config::t_token z_experience("experience", false);
	static const config::t_token z_resting("resting", false);
	static const config::t_token z_unrenamable("unrenamable", false);
	static const config::t_token z_lawful("lawful", false);
	static const config::t_token z_neutral("neutral", false);
	static const config::t_token z_chaotic("chaotic", false);
	static const config::t_token z_liminal("liminal", false);
	static const config::t_token z_upkeep("upkeep", false);
	static const config::t_token z_full("full", false);
	static const config::t_token z_flag_rgb("flag_rgb", false);
	static const config::t_token z_language_name("language_name", false);
	//	static const config::t_token z_ellipse("ellipse", false);
	//	static const config::t_token z_usage("usage", false);
	static const config::t_token z_generate_name("generate_name", false);
	static const config::t_token z_availability("availability", false);
	static const config::t_token z_musthave("musthave", false);
	//	static const config::t_token z_trait("trait", false);
	static const config::t_token z_random_traits("random_traits", false);
	//	static const config::t_token z_description("description", false);
	static const config::t_token z_male("male", false);
	static const config::t_token z_female("female", false);
	static const config::t_token z_unit_image("unit_image", false);
	static const config::t_token z_duration("duration", false);
	static const config::t_token z_prev_type("prev_type", false);
	static const config::t_token z_forever("forever", false);
	static const config::t_token z_abilities("abilities", false);
	static const config::t_token z_speaker("speaker", false);
	static const config::t_token z_has_weapon("has_weapon", false);
	static const config::t_token z_defense("defense", false);
	static const config::t_token z_visible("visible", false);
	static const config::t_token z_viewing_side("viewing_side", false);
	static const config::t_token z_filter_vision("filter_vision", false);
	static const config::t_token z_adjacent("adjacent", false);
	static const config::t_token z_is_enemy("is_enemy", false);
	static const config::t_token z_count("count", false);
	static const config::t_token z_find_in("find_in", false);
	static const config::t_token z_lua_function("lua_function", false);
	static const config::t_token z_status("status", false);
	static const config::t_token z_loyal("loyal", false);
	static const config::t_token z_free("free", false);
	static const config::t_token z_active_on("active_on", false);
	static const config::t_token z_apply_to("apply_to", false);
	static const config::t_token z_icon("icon", false);
	static const config::t_token z_strict_amla("strict_amla", false);
	static const config::t_token z_require_amla("require_amla", false);
	static const config::t_token z_times("times", false);
	static const config::t_token z_modifications("modifications", false);
	static const config::t_token z_level("level", false);
	static const config::t_token z_attack("attack", false);
	static const config::t_token z_resistance("resistance", false);
	static const config::t_token z_healable("healable", false);
	static const config::t_token z_unhealable("unhealable", false);
	static const config::t_token z_waypoints("waypoints", false);
	static const config::t_token z_moves("moves", false);
	static const config::t_token z_alignment("alignment", false);
	static const config::t_token z_placement("placement", false);
	static const config::t_token z_do_not_list("do_not_list", false);
	static const config::t_token z_event("event", false);
	static const config::t_token z_any("any", false);
	static const config::t_token z_die("die", false);
	static const config::t_token z_flies("flies", false);
	static const config::t_token z_inherit("inherit", false);
	static const config::t_token z_variation_name("variation_name", false);
	static const config::t_token z_ignore_race_traits("ignore_race_traits", false);
	static const config::t_token z_hide_help("hide_help", false);
	static const config::t_token z_yes("yes", false);
	static const config::t_token z_no("no", false);
	static const config::t_token z_slowed("slowed", false);
	static const config::t_token z_poisoned("poisoned", false);
	static const config::t_token z_petrified("petrified", false);
	static const config::t_token z_uncovered("uncovered", false);
	static const config::t_token z_not_moved("not_moved", false);
	static const config::t_token z_this_unit("this_unit", false);
	static const config::t_token z_and("and", false);
	static const config::t_token z_or("or", false);
	static const config::t_token z_not("not", false);
	static const config::t_token z_filter("filter", false);
	static const config::t_token z_recall("recall", false);
	static const config::t_token z_ability("ability", false);
	static const config::t_token z_movement_cost("movement_cost", false);
	static const config::t_token z_movement_costs("movement_costs", false);
	static const config::t_token z_filter_wml("filter_wml", false);
	static const config::t_token z_filter_adjacent("filter_adjacent", false);
	static const config::t_token z_standing("standing", false);
	static const config::t_token z_ghosted("ghosted", false);
	static const config::t_token z_disabled_ghosted("disabled_ghosted", false);
	//	static const config::t_token z_image("image", false);
	static const config::t_token z_movement("movement", false);
	static const config::t_token z_die_sound("die_sound", false);
	static const config::t_token z_movement_type("movement_type", false);
	static const config::t_token z_filter_location("filter_location", false);
	static const config::t_token z__disabled_("_disabled_", false);
	static const config::t_token z_idling("idling", false);
	static const config::t_token z_selected("selected", false);
	static const config::t_token z__disabled_selected_("_disabled_selected_", false);
	static const config::t_token z_offense("offense", false);
	static const config::t_token z_max_value("max_value", false);
	static const config::t_token z_advance("advance", false);
	static const config::t_token z_max_times("max_times", false);
	static const config::t_token z_fearless("fearless", false);
	static const config::t_token z_healthy("healthy", false);
	static const config::t_token z_effect("effect", false);
	static const config::t_token z_unit_type("unit_type", false);
	static const config::t_token z_unit_gender("unit_gender", false);
	static const config::t_token z_portrait("portriat", false);
	static const config::t_token z_small_portrait("small_portriat", false);
	static const config::t_token z_new_attack("new_attack", false);
	static const config::t_token z_remove_attacks("remove_attacks", false);
	static const config::t_token z_wesnoth("wesnoth", false);
	static const config::t_token z_attack_list("attack_list", false);
	static const config::t_token z_effect_description("effect_description", false);
	static const config::t_token z_increase("increase", false);
	static const config::t_token z_increase_total("increase_total", false);
	static const config::t_token z_set("set", false);
	static const config::t_token z_set_total("set_total", false);
	static const config::t_token z_violate_maximum("violate_maximum", false);
	static const config::t_token z_HP("HP", false);
	static const config::t_token z_remove("remove", false);
	static const config::t_token z_replace("replace", false);
	static const config::t_token z_heal_full("heal_full", false);
	static const config::t_token z_add("add", false);
	static const config::t_token z_value("value", false);
	static const config::t_token z_new_ability("new_ability", false);
	static const config::t_token z_remove_ability("remove_ability", false);
	static const config::t_token z_image_mod("image_mod", false);
	static const config::t_token z_new_animation("new_animation", false);
	static const config::t_token z_female_name("female_name", false);
	static const config::t_token z_male_name("male_name", false);
	static const config::t_token z_ignore_global_traits("ignore_global_traits", false);
	static const config::t_token z_range("range", false);
	static const config::t_token z_damage("damage", false);
	static const config::t_token z_number("number", false);
	static const config::t_token z_specials("specials", false);
	static const config::t_token z_description_inactive("description_inactive", false);
	static const config::t_token z_name_inactive("name_inactive", false);
	static const config::t_token z_advance_from("advance_from", false);
	static const config::t_token z_hides("hides", false);
	static const config::t_token z_Unit("Unit", false);
     static const config::t_token z_object("object", false);
	static const config::t_token z_extra_recruit("extra_recruit", false);
	static const config::t_token z_filter_recall("filter_recall", false);


    const config::t_token ModificationTypes[] = { z_advance, z_trait, z_object };
	const size_t NumModificationTypes = sizeof(ModificationTypes)/
										sizeof(*ModificationTypes);

	/**
	 * Pointers to units which have data in their internal caches. The
	 * destructor of an unit removes itself from the cache, so the pointers are
	 * always valid.
	 */
	static std::vector<const unit *> units_with_cache;
}

static const unit_type &get_unit_type(const std::string &type_id)
{
	const unit_type *i = unit_types.find(type_id);
	if (!i) throw game::game_error("unknown unit type: " + type_id);
	return *i;
}

static unit_race::GENDER generate_gender(const std::string &type_id, bool random_gender, game_state *state)
{
	const unit_type &type = get_unit_type(type_id);
	const std::vector<unit_race::GENDER>& genders = type.genders();

	if(genders.empty()) {
		return unit_race::MALE;
	} else if(random_gender == false || genders.size() == 1) {
		return genders.front();
	} else {
		int random = state ? state->rng().get_next_random() : get_random_nocheck();
		return genders[random % genders.size()];
	}
}

static unit_race::GENDER generate_gender(const config &cfg, game_state *state)
{
	const config::t_token& gender = cfg[z_gender];
	if(!gender.empty())
		return string_gender(gender);
	const config::t_token &type = cfg[z_type];
	if (type.empty())
		return unit_race::MALE;

	bool random_gender = cfg[z_random_gender].to_bool();
	return generate_gender(type, random_gender, state);
}

// Copy constructor
unit::unit(const unit& o):
           cfg_(o.cfg_),
           loc_(o.loc_),
           advances_to_(o.advances_to_),
           type_(o.type_),
           race_(o.race_),
           id_(o.id_),
           name_(o.name_),
           underlying_id_(o.underlying_id_),
           type_name_(o.type_name_),
           undead_variation_(o.undead_variation_),
           variation_(o.variation_),

           hit_points_(o.hit_points_),
           max_hit_points_(o.max_hit_points_),
           experience_(o.experience_),
           max_experience_(o.max_experience_),
           level_(o.level_),
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
           movement_costs_(o.movement_costs_),
           defense_mods_(o.defense_mods_),
           hold_position_(o.hold_position_),
           end_turn_(o.end_turn_),
           resting_(o.resting_),
           attacks_left_(o.attacks_left_),
           max_attacks_(o.max_attacks_),

           states_(o.states_),
           known_boolean_states_(o.known_boolean_states_),
           variables_(o.variables_),
           emit_zoc_(o.emit_zoc_),
           state_(o.state_),

           overlays_(o.overlays_),

           role_(o.role_),
           ai_special_(o.ai_special_),
           attacks_(o.attacks_),
           facing_(o.facing_),

           trait_names_(o.trait_names_),
           trait_descriptions_(o.trait_descriptions_),
           unit_value_(o.unit_value_),
           goto_(o.goto_),
           interrupted_move_(o.interrupted_move_),
           flying_(o.flying_),
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

           modifications_(o.modifications_),
		   invisibility_cache_()
{
}

unit::unit(const config &cfg, bool use_traits, game_state* state) :
	cfg_(),
	loc_(cfg[z_x] - 1, cfg[z_y] - 1),
	advances_to_(),
	type_(cfg[z_type].token()),
	race_(NULL),
	id_(cfg[z_id].token()),
	name_(cfg[z_name].t_str()),
	underlying_id_(0),
	type_name_(),
	undead_variation_(),
	variation_(cfg[z_variation].token()),
	hit_points_(1),
	max_hit_points_(0),
	experience_(0),
	max_experience_(0),
	level_(0),
	canrecruit_(cfg[z_canrecruit].to_bool()),
	recruit_list_(),
	alignment_(),
	flag_rgb_(),
	image_mods_(),
	unrenamable_(false),
	side_(0),
	gender_(generate_gender(cfg, state)),
	alpha_(),
	unit_formula_(),
	unit_loop_formula_(),
	unit_priority_formula_(),
	formula_vars_(),
	movement_(0),
	max_movement_(0),
	movement_costs_(),
	defense_mods_(),
	hold_position_(false),
	end_turn_(false),
	resting_(false),
	attacks_left_(0),
	max_attacks_(0),
	states_(),
	known_boolean_states_(known_boolean_state_names_.size(),false),
	variables_(),
	emit_zoc_(0),
	state_(STATE_STANDING),
	overlays_(),
	role_(cfg[z_role].token()),
	ai_special_(cfg[z_ai_special].token()),
	attacks_(),
	facing_(map_location::NDIRECTIONS),
	trait_names_(),
	trait_descriptions_(),
	unit_value_(),
	goto_(),
	interrupted_move_(),
	flying_(false),
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
	static const config::t_token z_empty("", false);
	if (type_.empty()) {
		throw game::game_error("creating unit with an empty type field");
	}

	side_ = cfg[z_side];
	if(side_ <= 0) {
		side_ = 1;
	}

	validate_side(side_);

	underlying_id_ = cfg[z_underlying_id];
	set_underlying_id();

	overlays_ = utils::parenthetical_split(cfg[z_overlays], ',');
	if(overlays_.size() == 1 && overlays_.front() == z_empty) {
		overlays_.clear();
	}
	if (const config &variables = cfg.child(z_variables)) {
		variables_ = variables;
	}

	facing_ = map_location::parse_direction(cfg[z_facing]);
	if(facing_ == map_location::NDIRECTIONS) facing_ = static_cast<map_location::DIRECTION>(rand()%map_location::NDIRECTIONS);

	if (const config &mods = cfg.child(z_modifications)) {
		modifications_ = mods;
	}

	advance_to(cfg, type(), use_traits, state);
	if (const config::attribute_value *v = cfg.get(z_race)) {
		if (const unit_race *r = unit_types.find_race(*v)) {
			race_ = r;
		} else {
			static const unit_race dummy_race;
			race_ = &dummy_race;
		}
	}
	level_ = cfg[z_level].to_int(level_);
	if (const config::attribute_value *v = cfg.get(z_undead_variation)) {
		undead_variation_ = v->token();
	}
	if(const config::attribute_value *v = cfg.get(z_max_attacks)) {
		max_attacks_ = std::max(0, v->to_int(1));
	}
	attacks_left_ = std::max(0, cfg[z_attacks_left].to_int(max_attacks_));

	if (const config::attribute_value *v = cfg.get(z_alpha)) {
		alpha_ = lexical_cast_default<fixed_t>(*v);
	}
	if (const config::attribute_value *v = cfg.get(z_zoc)) {
		emit_zoc_ = v->to_bool(level_ > 0);
	}
	if (const config::attribute_value *v = cfg.get(z_flying)) {
		flying_ = v->to_bool();
	}
	if (const config::attribute_value *v = cfg.get(z_description)) {
		cfg_[z_description] = *v;
	}
	if (const config::attribute_value *v = cfg.get(z_cost)) {
		unit_value_ = *v;
	}
	if (const config::attribute_value *v = cfg.get(z_halo)) {
		clear_haloes();
		cfg_[z_halo] = *v;
	}
	if (const config::attribute_value *v = cfg.get(z_profile)) {
		config::t_token big = *v, small = cfg[z_small_profile];
		adjust_profile(small, big, z_empty);
		cfg_[z_profile] = big;
		cfg_[z_small_profile] = small;
	}
	max_hit_points_ = std::max(1, cfg[z_max_hitpoints].to_int(max_hit_points_));
	max_movement_ = std::max(0, cfg[z_max_moves].to_int(max_movement_));
	max_experience_ = std::max(1, cfg[z_max_experience].to_int(max_experience_));

	std::vector<config::t_token> temp_advances = utils::split_token(cfg[z_advances_to]);
	if(temp_advances.size() == 1 && temp_advances.front() == z_null) {
		advances_to_.clear();
	}else if(temp_advances.size() >= 1 && temp_advances.front() !=  z_empty ) {
		advances_to_ = temp_advances;
	}

	if (const config &ai = cfg.child(z_ai))
	{
		unit_formula_ = ai[z_formula].str();
		unit_loop_formula_ = ai[z_loop_formula].str();
		unit_priority_formula_ = ai[z_priority].str();

		if (const config &ai_vars = ai.child(z_vars))
		{
			formula_vars_ = new game_logic::map_formula_callable;

			variant var;
			foreach (const config::attribute &i, ai_vars.attribute_range()) {
				var.serialize_from_string(i.second);
				formula_vars_->add(i.first, var);
			}
		} else {
			formula_vars_ = game_logic::map_formula_callable_ptr();
		}
	}

	//don't use the unit_type's attacks if this config has its own defined
	config::const_child_itors cfg_range = cfg.child_range(z_attack);
	if(cfg_range.first != cfg_range.second) {
		attacks_.clear();
		do {
			attacks_.push_back(attack_type(*cfg_range.first));
		} while(++cfg_range.first != cfg_range.second);
	}

	//don't use the unit_type's abilities if this config has its own defined
	cfg_range = cfg.child_range(z_abilities);
	if(cfg_range.first != cfg_range.second) {
		cfg_.clear_children(z_abilities);
		config &target = cfg_.add_child(z_abilities);
		do {
			target.append(*cfg_range.first);
		} while(++cfg_range.first != cfg_range.second);
	}

	//adjust the unit_type's defense if this config has its own defined
	cfg_range = cfg.child_range(z_defense);
	if(cfg_range.first != cfg_range.second) {
		config &target = cfg_.child_or_add(z_defense);
		do {
			target.append(*cfg_range.first);
		} while(++cfg_range.first != cfg_range.second);
	}

	//adjust the unit_type's movement costs if this config has its own defined
	cfg_range = cfg.child_range(z_movement_costs);
	if(cfg_range.first != cfg_range.second) {
		config &target = cfg_.child_or_add(z_movement_costs);
		do {
			target.append(*cfg_range.first);
		} while(++cfg_range.first != cfg_range.second);
	}

	//adjust the unit_type's resistance if this config has its own defined
	cfg_range = cfg.child_range(z_resistance);
	if(cfg_range.first != cfg_range.second) {
		config &target = cfg_.child_or_add(z_resistance);
		do {
			target.append(*cfg_range.first);
		} while(++cfg_range.first != cfg_range.second);
	}

	if (const config &status_flags = cfg.child(z_status))
	{
		foreach (const config::attribute &st, status_flags.attribute_range()) {
			if (st.first == z_healable) {
				///@deprecated 1.9.2 'healable' instead of 'unhealable'
				ERR_UT << "Usage of 'healable' is deprecated, use 'unhealable' instead, "
					"support will be removed in 1.9.2.\n";
				if (!st.second.to_bool(true))
					set_state(z_unhealable, true);
			} else if (st.second.to_bool()) {
				set_state(st.first, true);
			}
		}
	}
	if(cfg[z_ai_special] == z_guardian) {
		set_state(z_guardian, true);
	}

	// Remove animations from private cfg, they're not needed there now
	foreach(const std::string& tag_name, unit_animation::all_tag_names()) {
		cfg_.clear_children(tag_name);
	}

	if (const config::attribute_value *v = cfg.get(z_hitpoints)) {
		hit_points_ = *v;
	} else {
		hit_points_ = max_hit_points_;
	}

	goto_.x = cfg[z_goto_x].to_int() - 1;
	goto_.y = cfg[z_goto_y].to_int() - 1;

	if (const config::attribute_value *v = cfg.get(z_moves)) {
		movement_ = *v;
		if(movement_ < 0) {
			attacks_left_ = 0;
			movement_ = 0;
		}
	} else {
		movement_ = max_movement_;
	}
	experience_ = cfg[z_experience];
	resting_ = cfg[z_resting].to_bool();
	unrenamable_ = cfg[z_unrenamable].to_bool();

	const std::string& align = cfg[z_alignment];
	if(align == z_lawful) {
		alignment_ = unit_type::LAWFUL;
	} else if(align == z_neutral) {
		alignment_ = unit_type::NEUTRAL;
	} else if(align == z_chaotic) {
		alignment_ = unit_type::CHAOTIC;
	} else if(align == z_liminal) {
		alignment_ = unit_type::LIMINAL;
	} else if(align.empty()==false){
		alignment_ = unit_type::NEUTRAL;
	}

	generate_name(state ? &(state->rng()) : 0);

	// Make the default upkeep z_full
	if(cfg_[z_upkeep].empty()) {
		cfg_[z_upkeep] = z_full;
	}

	set_recruits(utils::split_token(cfg[z_extra_recruit]));
	cfg_.add_child(z_filter_recall, cfg.child_or_empty(z_filter_recall));

	/** @todo Are these modified by read? if not they can be removed. */
	getsHit_=0;
	end_turn_ = false;
	refreshing_  = false;
	hidden_ = false;
	game_config::add_color_info(cfg);

	config input_cfg;
	input_cfg.merge_attributes(cfg);

	static config::t_token const * internalized_attrs[] = { &z_type, &z_id, &z_name,
		&z_gender, &z_random_gender, &z_variation, &z_role, &z_ai_special,
		&z_side, &z_underlying_id, &z_overlays, &z_facing, &z_race,
		&z_level, &z_undead_variation, &z_max_attacks,
		&z_attacks_left, &z_alpha, &z_zoc, &z_flying, &z_cost,
		&z_max_hitpoints, &z_max_moves, &z_max_experience,
		&z_advances_to, &z_hitpoints, &z_goto_x, &z_goto_y, &z_moves,
		&z_experience, &z_resting, &z_unrenamable, &z_alignment,
		&z_canrecruit, &z_extra_recruit, &z_x, &z_y, &z_placement,
		// Useless attributes created when saving units to WML:
		&z_flag_rgb, &z_language_name };
	foreach (const  config::t_token * attr, internalized_attrs) {
		input_cfg.remove_attribute(*attr);
		cfg_.remove_attribute(*attr);
	}

	static config::t_token const * raw_attrs[] = { &z_description, &z_halo,
		&z_profile, &z_small_profile, &z_upkeep, &z_usage, &z_ellipse,
		&z_image, &z_random_traits, &z_generate_name };
	foreach (const  config::t_token * attr, raw_attrs) {
		input_cfg.remove_attribute(*attr);
	}

	foreach (const config::attribute &attr, input_cfg.attribute_range()) {
		if (attr.first == z_do_not_list) continue;
		WRN_UT << "Unknown attribute '" << attr.first << "' discarded.\n";
	}

}

void unit::clear_status_caches()
{
	for(std::vector<const unit *>::const_iterator itor = units_with_cache.begin();
			itor != units_with_cache.end(); ++itor) {
		(*itor)->clear_visibility_cache();
	}

	units_with_cache.clear();
}

unit::unit(const unit_type *t, int side, bool real_unit,
	unit_race::GENDER gender) :
	cfg_(),
	loc_(),
	advances_to_(),
	type_(),
	race_(NULL),
	id_(),
	name_(),
	underlying_id_(real_unit? 0: n_unit::id_manager::instance().next_fake_id()),
	type_name_(),
	undead_variation_(),
	variation_(),
	hit_points_(0),
	max_hit_points_(0),
	experience_(0),
	max_experience_(0),
	level_(0),
	canrecruit_(false),
	recruit_list_(),
	alignment_(),
	flag_rgb_(),
	image_mods_(),
	unrenamable_(false),
	side_(side),
	gender_(gender != unit_race::NUM_GENDERS ?
		gender : generate_gender(t->id(), real_unit, NULL)),
	alpha_(),
	unit_formula_(),
	unit_loop_formula_(),
    unit_priority_formula_(),
	formula_vars_(),
	movement_(0),
	max_movement_(0),
	movement_costs_(),
	defense_mods_(),
	hold_position_(false),
	end_turn_(false),
	resting_(false),
	attacks_left_(0),
	max_attacks_(0),
	states_(),
	known_boolean_states_(known_boolean_state_names_.size(),false),
	variables_(),
	emit_zoc_(0),
	state_(STATE_STANDING),
	overlays_(),
	role_(),
	ai_special_(),
	attacks_(),
	facing_(static_cast<map_location::DIRECTION>(rand()%map_location::NDIRECTIONS)),
	trait_names_(),
	trait_descriptions_(),
	unit_value_(),
	goto_(),
	interrupted_move_(),
	flying_(false),
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

	cfg_[z_upkeep]=z_full;
	advance_to(t, real_unit);

	if(real_unit) {
		generate_name();
	}
	set_underlying_id();

	// fill those after traits and modifs to have correct max
	movement_ = max_movement_;
	hit_points_ = max_hit_points_;
	attacks_left_ = max_attacks_;

	/**
	 * @todo Test whether the calls above modify these values if not they can
	 * removed, since already set in the initialization list.
	 */
	unrenamable_ = false;
	anim_ = NULL;
	getsHit_ = 0;
	end_turn_ = false;
	hold_position_ = false;
	next_idling_ = 0;
	frame_begin_time_ = 0;
	unit_halo_ = halo::NO_HALO;

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


void unit::set_game_context()
{
	// In case the unit carries EventWML, apply it now
	game_events::add_events(cfg_.child_range(z_event), type_);
	cfg_.clear_children(z_event);
}

void unit::generate_name(rand_rng::simple_rng* rng)
{
	if (!name_.empty() || !cfg_[z_generate_name].to_bool(true)) return;

	name_ = race_->generate_name(gender_, rng);
	cfg_[z_generate_name] = false;
}

// Apply mandatory traits (e.g. undead, mechanical) to a unit and then
// fill out with available (leaders have a restricted set of available traits)
// traits until no more are available or the unit has its maximum number
// of traits.
// This routine does not apply the effects of added traits to a unit.
// That must be done by the caller.
// Note that random numbers used in config files don't work in multiplayer,
// so that leaders should be barred from all random traits until that
// is fixed. Later the restrictions will be based on play balance.
// @musthaveonly is true when you don't want to generate random traits or
// you don't want to give any optional traits to a unit.

void unit::generate_traits(bool musthaveonly, game_state* state)
{
	LOG_UT << "Generating a trait for unit type " << type_id() << " with musthaveonly " << musthaveonly << "\n";
	const unit_type *type = unit_types.find(type_id());
	// Calculate the unit's traits
	if (!type) {
		std::string error_message = _("Unknown unit type '$type|' while generating traits");
		utils::string_map symbols;
		symbols[z_type] = type_id();
		error_message = utils::interpolate_variables_into_string(error_message, &symbols);
		ERR_NG << "unit of type " << type_id() << " not found!\n";
		throw game::game_error(error_message);
	}

	config::const_child_itors current_traits = modifications_.child_range(z_trait);
	std::vector<config> candidate_traits;

	foreach (const config &t, type->possible_traits())
	{
		// Skip the trait if the unit already has it.
		const std::string &tid = t[z_id];
		bool already = false;
		foreach (const config &mod, current_traits)
		{
			if (mod[z_id] == tid) {
				already = true;
				break;
			}
		}
		if (already) continue;

		// Add the trait if it is mandatory.
		const std::string &avl = t[z_availability];
		if (avl == z_musthave)
		{
			modifications_.add_child(z_trait, t);
			current_traits = modifications_.child_range(z_trait);
			continue;
		}

		// The trait is still available, mark it as a candidate for randomizing.
		// For leaders, only traits with availability "any" are considered.
		if (!musthaveonly && (!can_recruit() || avl == z_any))
			candidate_traits.push_back(t);
	}

	if (musthaveonly) return;

	// Now randomly fill out to the number of traits required or until
	// there aren't any more traits.
	int nb_traits = std::distance(current_traits.first, current_traits.second);
	int max_traits = type->num_traits();
	for (; nb_traits < max_traits && !candidate_traits.empty(); ++nb_traits)
	{
		int num = (state ? state->rng().get_next_random() : get_random_nocheck())
		          % candidate_traits.size();
		modifications_.add_child(z_trait, candidate_traits[num]);
		candidate_traits.erase(candidate_traits.begin() + num);
	}

	// Once random traits are added, don't do it again.
	// Such as when restoring a saved character.
	cfg_[z_random_traits] = false;
}

std::vector<config::t_token> unit::get_traits_list() const
{
	std::vector<config::t_token> res;

	foreach (const config &mod, modifications_.child_range(z_trait))
	{
			config::t_token const &id = mod[z_id];
			if (!id.empty())
				res.push_back(id);
	}
	return res;
}

void unit::advance_to(const config &old_cfg, const unit_type *t,
	bool use_traits, game_state *state)
{
	t = &t->get_gender_unit_type(gender_).get_variation(variation_);

	// Reset the scalar values first
	trait_names_.clear();
	trait_descriptions_.clear(),
	is_fearless_ = false;
	is_healthy_ = false;

	// Clear modification-related caches
	modification_descriptions_.clear();
	movement_costs_.clear();
	defense_mods_.clear();

	// Clear the stored config and replace it with the one from the unit type,
	// except for a few attributes.
	config new_cfg;
	static config::t_token const *persistent_attrs[] = { &z_upkeep, &z_ellipse,
		&z_image, &z_usage, &z_random_traits, &z_generate_name };
	foreach (const config::t_token *attr, persistent_attrs) {
		if (const config::attribute_value *v = old_cfg.get(*attr)) {
			new_cfg[*attr] = *v;
		}
	}

	//copy �nformation what this unit is able to recall
	new_cfg.add_child("filter_recall", old_cfg.child_or_empty("filter_recall"));

	if(t->movement_type().get_parent()) {
		new_cfg.merge_with(t->movement_type().get_parent()->get_cfg());
	}

	new_cfg.merge_with(t->cfg_);

	// Remove pure unit_type attributes.
	static config::t_token const *unit_type_attrs[] = { &z_movement, &z_movement_type,
		&z_die_sound, &z_flies, &z_inherit, &z_variation_name,
		&z_ignore_race_traits, &z_hide_help };
	foreach (const config::t_token *attr, unit_type_attrs) {
		new_cfg.remove_attribute(*attr);
	}

	// If unit has specific profile, remember it and keep it after advancing
	const unit_type *u_type = type();
	config::t_token profile = old_cfg[z_profile].token();
	if (!profile.empty() && (!u_type || profile != u_type->big_profile())) {
		new_cfg[z_profile] = profile;
	} else if (t) {
		new_cfg[z_profile] = t->big_profile();
	}
	profile = old_cfg[z_small_profile].token();
	if (!profile.empty() && (!u_type || profile != u_type->small_profile())) {
		new_cfg[z_small_profile] = profile;
	} else if (t) {
		new_cfg[z_small_profile] = t->small_profile();
	}

	cfg_.swap(new_cfg);
	cfg_.clear_children(z_male);
	cfg_.clear_children(z_female);

	advances_to_ = t->advances_to();

	race_ = t->race_;
	type_name_ = t->type_name();
	cfg_[z_description] = t->unit_description();
	undead_variation_ = t->undead_variation();
	max_experience_ = t->experience_needed(false);
	level_ = t->level();
	alignment_ = t->alignment();
	alpha_ = t->alpha();
	hit_points_ = t->hitpoints();
	max_hit_points_ = t->hitpoints();
	max_movement_ = t->movement();
	emit_zoc_ = t->has_zoc();
	attacks_ = t->attacks();
	unit_value_ = t->cost();
	flying_ = t->movement_type().is_flying();

	max_attacks_ = t->max_attacks();

	animations_ = t->animations();

	flag_rgb_ = t->flag_rgb();


	bool do_heal = false; // Track whether unit should get fully healed.

	if(type_id()!=t->id()) {
		do_heal = true; // Can't heal until after mods applied.
		type_ = t->id();
	}

	if (cfg_[z_random_traits].to_bool(true)) {
		generate_traits(!use_traits, state);
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

	// Not that the unit has all of its modifications applied, it is
	// OK to heal it.
	if (do_heal) {
		heal_all();
	}

	set_game_context();

	refreshing_ = false;
	delete anim_;
	anim_ = NULL;
}

const unit_type* unit::type() const
{
	if (type_.empty()) return NULL;
	const unit_type &i = get_unit_type(type_);
	return &i.get_gender_unit_type(gender_).get_variation(variation_);
}

config::t_token const & unit::big_profile() const
{
	const config::t_token &prof = cfg_[z_profile];
	if (!prof.empty() && prof != z_unit_image) {
		return prof;
	}
	return absolute_image();
}

config::t_token const & unit::small_profile() const
{
	const config::t_token &prof = cfg_[z_small_profile];
	if (!prof.empty() && prof != z_unit_image) {
		return prof;
	}
	return absolute_image();
}

SDL_Color unit::hp_color() const
{
	double unit_energy = 0.0;
	SDL_Color energy_color = {0,0,0,0};

	if(max_hitpoints() > 0) {
		unit_energy = double(hitpoints())/double(max_hitpoints());
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

SDL_Color unit::xp_color() const
{
	const SDL_Color near_advance_color = {255,255,255,0};
	const SDL_Color mid_advance_color  = {150,255,255,0};
	const SDL_Color far_advance_color  = {0,205,205,0};
	const SDL_Color normal_color 	  = {0,160,225,0};
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

void unit::set_recruits(const std::vector<config::t_token>& recruits)
{
	unit_types.check_types(recruits);
	recruit_list_ = recruits;
	//TODO crab
	//info_.minimum_recruit_price = 0;
	//ai::manager::raise_recruit_list_changed();
}

void unit::set_advances_to(const std::vector<config::t_token>& advances_to)
{
	unit_types.check_types(advances_to);
	advances_to_ = advances_to;
}

std::string unit::side_id() const {return teams_manager::get_teams()[side()-1].save_id(); }

void unit::set_movement(int moves)
{
	//FIXME: we shouldn't set those here, other code use this a simple setter.
	hold_position_ = false;
	end_turn_ = false;
	movement_ = std::max<int>(0, moves);
}

void unit::new_turn()
{
	end_turn_ = false;
	movement_ = total_movement();
	attacks_left_ = max_attacks_;
	set_state(STATE_UNCOVERED, false);

	if (hold_position_) {
		end_turn_ = true;
	}
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
	static const config::t_token z_empty("", false);
	ai_special_ = z_empty;

	// Set the goto-command to be going to no-where
	goto_ = map_location();

	bool rebuild_from_type = false;

	for(unsigned int i = 0; i != NumModificationTypes; ++i) {
		const std::string& mod_name = ModificationTypes[i];
		for (int j = modifications_.child_count(mod_name) - 1; j >= 0; --j)
		{
			const config &mod = modifications_.child(mod_name, j);
			const std::string& duration = mod[z_duration];
			if (!duration.empty() && duration != z_forever) {
				if (const config::attribute_value *v = mod.get(z_prev_type)) {
					type_ = v->token();
				}
				modifications_.remove_child(mod_name, j);
				rebuild_from_type = true;
			}
		}
	}
	if(rebuild_from_type) {
		advance_to(type());
	}

	heal_all();
	set_state(STATE_SLOWED, false);
	set_state(STATE_POISONED, false);
	set_state(STATE_PETRIFIED, false);
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

const std::map<config::t_token,config::t_token> unit::get_states() const
{
	std::map<config::t_token, config::t_token> all_states;
	foreach (config::t_token const &s, states_) {
		all_states[s] = z_yes;
	}
	for (std::map<config::t_token, state_t>::const_iterator i = known_boolean_state_names_.begin(),
	     i_end = known_boolean_state_names_.end(); i != i_end; ++i)
	{
		if (get_state(i->second)) {
			all_states.insert(std::make_pair(i->first, z_yes));
		}

	}
	return all_states;
}

bool unit::get_state(const config::t_token &state) const
{
	state_t known_boolean_state_id = get_known_boolean_state_id(state);
	if (known_boolean_state_id!=STATE_UNKNOWN){
		return get_state(known_boolean_state_id);
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

unit::state_t unit::get_known_boolean_state_id(const config::t_token &state) {
	std::map<config::t_token, state_t>::const_iterator i = known_boolean_state_names_.find(state);
	if (i != known_boolean_state_names_.end()) {
		return i->second;
	}
	return STATE_UNKNOWN;
}

std::map<config::t_token, unit::state_t> unit::known_boolean_state_names_ = get_known_boolean_state_names();

std::map<config::t_token, unit::state_t> unit::get_known_boolean_state_names()
{
	std::map<config::t_token, state_t> known_boolean_state_names_map;
	known_boolean_state_names_map.insert(std::make_pair(z_slowed,STATE_SLOWED));
	known_boolean_state_names_map.insert(std::make_pair(z_poisoned,STATE_POISONED));
	known_boolean_state_names_map.insert(std::make_pair(z_petrified,STATE_PETRIFIED));
	known_boolean_state_names_map.insert(std::make_pair(z_uncovered, STATE_UNCOVERED));
	known_boolean_state_names_map.insert(std::make_pair(z_not_moved,STATE_NOT_MOVED));
	known_boolean_state_names_map.insert(std::make_pair(z_unhealable,STATE_UNHEALABLE));
	//not sure if "guardian" is a yes/no state.
	//known_boolean_state_names_map.insert(std::make_pair("guardian",STATE_GUARDIAN));
	return known_boolean_state_names_map;
}

void unit::set_state(const config::t_token &state, bool value)
{
	state_t known_boolean_state_id = get_known_boolean_state_id(state);
	if (known_boolean_state_id != STATE_UNKNOWN) {
		set_state(known_boolean_state_id, value);
		return;
	}
	if (value)
		states_.insert(state);
	else
		states_.erase(state);
}


bool unit::has_ability_by_id(const std::string& ability) const
{
	if (const config &abil = cfg_.child(z_abilities))
	{
		foreach (const config::any_child &ab, abil.all_children_range()) {
			if (ab.cfg[z_id] == ability)
				return true;
		}
	}
	return false;
}

void unit::remove_ability_by_id(const std::string &ability)
{
	if (config &abil = cfg_.child(z_abilities))
	{
		config::all_children_iterator i = abil.ordered_begin();
		while (i != abil.ordered_end()) {
			if (i->cfg[z_id] == ability) {
				i = abil.erase(i);
			} else {
				++i;
			}
		}
	}
}

bool unit::matches_filter(const vconfig& cfg, const map_location& loc, bool use_flat_tod
						  , gamemap const & game_map, unit_map const & units, 
						  t_teams const & teams, LuaKernel & lua_kernel, 
						  tod_manager const & tod_manager) const
{
	bool matches = true;

	if(loc.valid()) {
		//assert(units != NULL);

		scoped_xy_unit auto_store(z_this_unit, loc.x, loc.y, units);

		matches = internal_matches_filter(cfg, loc, use_flat_tod, game_map, units, teams, lua_kernel, tod_manager);
	} else {
		// If loc is invalid, then this is a recall list unit (already been scoped)
		matches = internal_matches_filter(cfg, loc, use_flat_tod, game_map, units, teams, lua_kernel, tod_manager);
	}

	// Handle [and], [or], and [not] with in-order precedence
	vconfig::all_children_iterator cond = cfg.ordered_begin();
	vconfig::all_children_iterator cond_end = cfg.ordered_end();
	while(cond != cond_end)
	{

		const std::string& cond_name = cond.get_key();
		const vconfig& cond_filter = cond.get_child();

		// Handle [and]
		if(cond_name == z_and) {
			matches = matches && matches_filter(cond_filter,loc,use_flat_tod, game_map, units, teams, lua_kernel, tod_manager);
		}
		// Handle [or]
		else if(cond_name == z_or) {
			matches = matches || matches_filter(cond_filter,loc,use_flat_tod, game_map, units, teams, lua_kernel, tod_manager);
		}
		// Handle [not]
		else if(cond_name == z_not) {
			matches = matches && !matches_filter(cond_filter,loc,use_flat_tod,game_map, units, teams, lua_kernel, tod_manager);
		}

		++cond;
	}
	return matches;
}

bool unit::internal_matches_filter(const vconfig& cfg, const map_location& loc, bool use_flat_tod
								   , gamemap const & game_map, unit_map const & units, 
								   t_teams const & teams, LuaKernel & lua_kernel, 
								   tod_manager const & tod_manager) const
{
	config::attribute_value cfg_name = cfg[z_name];
	if (!cfg_name.empty() && cfg_name.str() != name_) {
		return false;
	}

	const config::attribute_value cfg_id = cfg[z_id];
	if (!cfg_id.empty()) {
		const std::string& id = cfg_id;
		const std::string& this_id = this->id();

		if (id == this_id) {
		}
		else if (std::find(id.begin(), id.end(), ',') == id.end()){
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
	config::attribute_value cfg_speaker = cfg[z_speaker];
	if (!cfg_speaker.empty() && cfg_speaker.str() != id()) {
		return false;
	}

	const vconfig& t_cfg = cfg.child(z_filter_location);
	if(! t_cfg.null() ) {
		terrain_filter t_filter(t_cfg, units, use_flat_tod);
		if(!t_filter.match( loc)) {
			return false;
		}
	}
	// Also allow filtering on location ranges outside of the location filter
	config::attribute_value cfg_x = cfg[z_x];
	config::attribute_value cfg_y = cfg[z_y];
	if (!cfg_x.empty() || !cfg_y.empty()){
		if(cfg_x == z_recall && cfg_y == z_recall) {
			//locations on the map are considered to not be on a recall list
			if ( game_map.on_board(loc))  {
				return false;
			}
		} else if(cfg_x.empty() && cfg_y.empty()) {
			return false;
		} else if(!loc.matches_range(cfg_x, cfg_y)) {
			return false;
		}
	}

	// The type could be a comma separated list of types
	config::attribute_value cfg_type = cfg[z_type];
	if (!cfg_type.empty())
	{
		std::string type = cfg_type;
		const std::string& this_type = type_id();

		// We only do the full CSV search if we find a comma in there,
		// and if the subsequence is found within the main sequence.
		// This is because doing the full CSV split is expensive.
		if(type == this_type) {
			// pass
		} else if(std::find(type.begin(),type.end(),',') != type.end() &&
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

	config::attribute_value cfg_ability = cfg[z_ability];
	if (!cfg_ability.empty())
	{
		std::string ability = cfg_ability;
		if(has_ability_by_id(ability)) {
			// pass
		} else if(std::find(ability.begin(),ability.end(),',') != ability.end()) {
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

	config::attribute_value cfg_race = cfg[z_race];
	if (!cfg_race.empty() && cfg_race.str() != race_->id()) {
		return false;
	}

	config::attribute_value cfg_gender = cfg[z_gender];
	if (!cfg_gender.empty() && string_gender(cfg_gender) != gender()) {
		return false;
	}

	config::attribute_value cfg_side = cfg[z_side];
	if (!cfg_side.empty() && cfg_side.to_int() != side()) {
		std::string side = cfg_side;
		if (std::find(side.begin(), side.end(), ',') == side.end()) {
			return false;
		}
		std::vector<std::string> vals = utils::split(side);
		if (std::find(vals.begin(), vals.end(), str_cast(side_)) == vals.end()) {
			return false;
		}
	}

	config::attribute_value cfg_has_weapon = cfg[z_has_weapon];
	if (!cfg_has_weapon.empty()) {
		config::t_token const & weapon = cfg_has_weapon;
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

	config::attribute_value cfg_role = cfg[z_role];
	if (!cfg_role.empty() && cfg_role.str() != role_) {
		return false;
	}

	config::attribute_value cfg_ai_special = cfg[z_ai_special];
	if (!cfg_ai_special.empty() && cfg_ai_special.str() != ai_special_) {
		return false;
	}

	config::attribute_value cfg_canrecruit = cfg[z_canrecruit];
	if (!cfg_canrecruit.empty() && cfg_canrecruit.to_bool() != can_recruit()) {
		return false;
	}

	config::attribute_value cfg_level = cfg[z_level];
	if (!cfg_level.empty() && cfg_level.to_int(-1) != level_) {
		return false;
	}

	config::attribute_value cfg_defense = cfg[z_defense];
	if (!cfg_defense.empty() && cfg_defense.to_int(-1) != defense_modifier(game_map.get_terrain(loc))) {
		return false;
	}

	config::attribute_value cfg_movement = cfg[z_movement_cost];
	if (!cfg_movement.empty() && cfg_movement.to_int(-1) != movement_cost(game_map.get_terrain(loc))) {
		return false;
	}

	// Now start with the new WML based comparison.
	// If a key is in the unit and in the filter, they should match
	// filter only => not for us
	// unit only => not filtered
	const vconfig::child_list& wmlcfgs = cfg.get_children(z_filter_wml);
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
			    ci.first->key == z_variables) {
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

	if (cfg.has_child(z_filter_vision)) {
		const vconfig::child_list& vis_filt = cfg.get_children(z_filter_vision);
		vconfig::child_list::const_iterator i, i_end = vis_filt.end();
		for (i = vis_filt.begin(); i != i_end; ++i) {
			bool visible = (*i)[z_visible].to_bool(true);
			std::set<int> viewers;
			if (i->has_attribute(z_viewing_side)) {
				std::vector<std::pair<int,int> > ranges = utils::parse_ranges((*i)[z_viewing_side]);
				std::vector<std::pair<int,int> >::const_iterator range, range_end = ranges.end();
				for (range = ranges.begin(); range != range_end; ++range) {
					for (int i=range->first; i<=range->second; ++i) {
						if (i > 0 && static_cast<size_t>(i) <= teams_manager::get_teams().size()) {
							viewers.insert(i);
						}
					}
				}
			} else {
				//if viewing_side is not defined, default to all enemies
				const team& my_team = teams_manager::get_teams()[this->side()-1];
				for (size_t i = 1; i <= teams_manager::get_teams().size(); ++i) {
					if (my_team.is_enemy(i)) {
						viewers.insert(i);
					}
				}
			}
			if (viewers.empty()) {
				return false;
			}
			std::set<int>::const_iterator viewer, viewer_end = viewers.end();
			for (viewer = viewers.begin(); viewer != viewer_end; ++viewer) {
				bool not_fogged = !teams_manager::get_teams()[*viewer - 1].fogged(loc);
				bool not_hiding = !this->invisible(loc/*, false(?) */);
				if (visible != not_fogged && not_hiding) {
					return false;
				}
			}
		}
	}

	if (cfg.has_child(z_filter_adjacent)) {
		//assert(units && game_map);
		map_location adjacent[6];
		get_adjacent_tiles(loc, adjacent);
		vconfig::child_list::const_iterator i, i_end;
		const vconfig::child_list& adj_filt = cfg.get_children(z_filter_adjacent);
		for (i = adj_filt.begin(), i_end = adj_filt.end(); i != i_end; ++i) {
			int match_count=0;
			static std::vector<map_location::DIRECTION> default_dirs
				= map_location::parse_directions("n,ne,se,s,sw,nw");
			config::attribute_value i_adjacent = (*i)[z_adjacent];
			std::vector<map_location::DIRECTION> dirs = !i_adjacent.empty() ?
				map_location::parse_directions(i_adjacent) : default_dirs;
			std::vector<map_location::DIRECTION>::const_iterator j, j_end = dirs.end();
			for (j = dirs.begin(); j != j_end; ++j) {
				unit_map::const_iterator unit_itor = units.find(adjacent[*j]);
				if (unit_itor == units.end()
					|| !unit_itor->matches_filter(*i, unit_itor->get_location(), use_flat_tod, game_map, units, teams, lua_kernel, tod_manager)) {
					continue;
				}
				config::attribute_value i_is_enemy = (*i)[z_is_enemy];
				if (i_is_enemy.empty() || i_is_enemy.to_bool() ==
				    teams_manager::get_teams()[this->side() - 1].is_enemy(unit_itor->side())) {
					++match_count;
				}
			}
			static std::vector<std::pair<int,int> > default_counts = utils::parse_ranges("1-6");
			config::attribute_value i_count = (*i)[z_count];
			std::vector<std::pair<int,int> > counts = !i_count.empty()
				? utils::parse_ranges(i_count) : default_counts;
			if(!in_ranges(match_count, counts)) {
				return false;
			}
		}
	}

	config::attribute_value cfg_find_in = cfg[z_find_in];
	if (!cfg_find_in.empty()) {
		// Allow filtering by searching a stored variable of units
		variable_info vi(cfg_find_in, false, variable_info::TYPE_CONTAINER);
		if(!vi.is_valid()) return false;
		if(vi.is_explicit_index()) {
			config::const_child_iterator i = vi.vars->child_range(vi.key).first;
			std::advance(i, vi.index);
			if ((*i)[z_id] != id_) {
				return false;
			}
		} else {
			if (!vi.vars->find_child(vi.key, z_id, id_))
				return false;
		}
	}
	config::attribute_value cfg_formula = cfg[z_formula];
	if (!cfg_formula.empty()) {
		const unit_callable callable(std::pair<map_location, unit>(loc,*this));
		const game_logic::formula form(cfg_formula);
		if(!form.evaluate(callable).as_bool()) {///@todo use formula_ai
			return false;
		}
	}

	config::attribute_value cfg_lua_function = cfg[z_lua_function];
	if (!cfg_lua_function.empty()) {
		bool b = lua_kernel.run_filter(cfg_lua_function.str().c_str(), *this);
		if (!b) return false;
	}

	return true;
}

void unit::write(config& cfg) const
{
	cfg.append(cfg_);
	const unit_type *ut = unit_types.find(type_id());
	if (ut) {
		ut = &ut->get_gender_unit_type(gender_).get_variation(variation_);
	}
	if(ut && cfg[z_description] == ut->unit_description()) {
		cfg.remove_attribute(z_description);
	}

	cfg[z_hitpoints] = hit_points_;
	cfg[z_max_hitpoints] = max_hit_points_;

	cfg[z_experience] = experience_;
	cfg[z_max_experience] = max_experience_;

	cfg[z_side] = side_;

	cfg[z_type] = type_id();

	//support for unit formulas in [ai] and unit-specific variables in [ai] [vars]

        if ( has_formula() || has_loop_formula() || (formula_vars_ && formula_vars_->empty() == false) ) {

		config &ai = cfg.add_child(z_ai);

		if (has_formula())
			ai[z_formula] = unit_formula_;

		if (has_loop_formula())
			ai[z_loop_formula] = unit_loop_formula_;

		if (has_priority_formula())
			ai[z_priority] = unit_priority_formula_;


		if (formula_vars_ && formula_vars_->empty() == false)
		{
			config &ai_vars = ai.add_child(z_vars);

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

	cfg[z_gender] = gender_string(gender_);

	cfg[z_variation] = variation_;

	cfg[z_role] = role_;
	cfg[z_ai_special] = ai_special_;
	cfg[z_flying] = flying_;

	config status_flags;
	std::map<config::t_token,config::t_token> all_states = get_states();
	for(std::map<config::t_token,config::t_token>::const_iterator st = all_states.begin(); st != all_states.end(); ++st) {
		status_flags[st->first] = st->second;
	}

	cfg.clear_children(z_variables);
	cfg.add_child(z_variables,variables_);
	cfg.clear_children(z_status);
	cfg.add_child(z_status,status_flags);

	cfg[z_overlays] = utils::join(overlays_);

	cfg[z_name] = name_;
	cfg[z_id] = id_;
	cfg[z_underlying_id] = str_cast(underlying_id_);

	if(can_recruit())
		cfg[z_canrecruit] = true;

	cfg[z_extra_recruit] = utils::join(recruit_list_);

	cfg[z_facing] = map_location::write_direction(facing_);

	cfg[z_goto_x] = goto_.x + 1;
	cfg[z_goto_y] = goto_.y + 1;



	cfg[z_moves] = movement_;
	cfg[z_max_moves] = max_movement_;

	cfg[z_resting] = resting_;

	cfg[z_advances_to] = utils::join(advances_to_);

	cfg[z_race] = race_->id();
	cfg[z_language_name] = type_name_;
	cfg[z_undead_variation] = undead_variation_;
	cfg[z_variation] = variation_;
	cfg[z_level] = level_;
	switch(alignment_) {
		case unit_type::LAWFUL:
			cfg[z_alignment] = z_lawful;
			break;
		case unit_type::NEUTRAL:
			cfg[z_alignment] = z_neutral;
			break;
		case unit_type::CHAOTIC:
			cfg[z_alignment] = z_chaotic;
			break;
		case unit_type::LIMINAL:
			cfg["alignment"] = "liminal";
			break;
		default:
			cfg[z_alignment] = z_neutral;
	}
	cfg[z_flag_rgb] = flag_rgb_;
	cfg[z_unrenamable] = unrenamable_;
	cfg[z_alpha] = str_cast(alpha_);

	cfg[z_attacks_left] = attacks_left_;
	cfg[z_max_attacks] = max_attacks_;
	cfg[z_zoc] = emit_zoc_;
	cfg.clear_children(z_attack);
	for(std::vector<attack_type>::const_iterator i = attacks_.begin(); i != attacks_.end(); ++i) {
		cfg.add_child(z_attack,i->get_cfg());
	}
	cfg[z_cost] = unit_value_;
	cfg.clear_children(z_modifications);
	cfg.add_child(z_modifications,modifications_);

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
		image_loc = image::locator(absolute_image(), config::t_token(mods));
	} else {
		image_loc = image::locator(absolute_image());
	}
#endif

	surface unit_image(image::get_image(image_loc, scaled ? image::SCALED_TO_ZOOM : image::UNSCALED));
	return unit_image;
}

void unit::set_standing(bool with_bars) {
	static const config::t_token z_empty("", false);

	game_display *disp = game_display::get_singleton();
	if (preferences::show_standing_animations()&& !incapacitated()) {
		start_animation(INT_MAX, choose_animation(*disp, loc_, z_standing),
			with_bars,  z_empty, 0, STATE_STANDING);
	} else {
		start_animation(INT_MAX, choose_animation(*disp, loc_, z__disabled_),
			with_bars,  z_empty, 0, STATE_STANDING);
	}
}

void unit::set_ghosted(bool with_bars)
{
	game_display *disp = game_display::get_singleton();
	start_animation(INT_MAX, choose_animation(*disp, loc_, z_ghosted),
			with_bars);
}

void unit::set_disabled_ghosted(bool with_bars)
{
	game_display *disp = game_display::get_singleton();
	start_animation(INT_MAX, choose_animation(*disp, loc_, z_disabled_ghosted),
			with_bars);
}

void unit::set_idling()
{
	static const config::t_token z_empty("", false);
	game_display *disp = game_display::get_singleton();
	start_animation(INT_MAX, choose_animation(*disp, loc_, z_idling),
		true, z_empty, 0, STATE_FORGET);
}

void unit::set_selecting()
{
	static const config::t_token z_empty("", false);
	const game_display *disp =  game_display::get_singleton();
	if (preferences::show_standing_animations() && !get_state(STATE_PETRIFIED)) {
		start_animation(INT_MAX, choose_animation(*disp, loc_, z_selected),
			true, z_empty, 0, STATE_FORGET);
	} else {
		start_animation(INT_MAX, choose_animation(*disp, loc_, z__disabled_selected_),
			true, z_empty, 0, STATE_FORGET);
	}
}

void unit::start_animation(int start_time, const unit_animation *animation,
	bool with_bars,  const n_token::t_token &text, Uint32 text_color, STATE state) {
	const game_display * disp =  game_display::get_singleton();
	state_ = state;
	if (!animation) {
		if (state != STATE_STANDING)
			set_standing(with_bars);
		return ;
	}
	// everything except standing select and idle
	bool accelerate = (state != STATE_FORGET && state != STATE_STANDING);
	draw_bars_ =  with_bars;
	delete anim_;
	anim_ = new unit_animation(*animation);
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
	game_display &disp = *game_display::get_singleton();
	const gamemap &map = disp.get_map();
	if (!loc_.valid() || hidden_ || disp.fogged(loc_) ||
	    (invisible(loc_)
	&& disp.get_teams()[disp.viewing_team()].is_enemy(side())))
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
	if (get_state(STATE_POISONED)){
		params.blend_with = disp.rgb(0,255,0);
		params.blend_ratio = 0.25;
	}
	//hackish : see unit_frame::merge_parameters
	// we use image_mod on the primary image
	// and halo_mod on secondary images and all haloes
	params.image_mod = image_mods();
	params.halo_mod = TC_image_mods();
	params.image= static_cast<std::string const &>(absolute_image());


	if(get_state(STATE_PETRIFIED)) params.image_mod = config::t_token(params.image_mod + "~GS()" );
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
	if(draw_bars && preferences::show_side_colors()) {
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

		const char* const selected = disp.selected_hex() == loc_ ? "selected-" : "";

		// Load the ellipse parts recolored to match team color
		char buf[100];
		std::string tc=team::get_side_color_index(side_);

		snprintf(buf,sizeof(buf),"%s-%stop.png~RC(ellipse_red>%s)",ellipse.c_str(),selected,tc.c_str());
		ellipse_back.assign(image::get_image(image::locator(buf), image::SCALED_TO_ZOOM));
		snprintf(buf,sizeof(buf),"%s-%sbottom.png~RC(ellipse_red>%s)",ellipse.c_str(),selected,tc.c_str());
		ellipse_front.assign(image::get_image(image::locator(buf), image::SCALED_TO_ZOOM));
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
		static const image::locator enemy_orb(game_config::images::enemy_orb);
		static const image::locator ally_orb(game_config::images::ally_orb);
		static const image::locator moved_orb(game_config::images::moved_orb);
		static const image::locator unmoved_orb(game_config::images::unmoved_orb);
		static const image::locator partmoved_orb(game_config::images::partmoved_orb);

		const config::t_token* energy_file = &game_config::images::energy;

		if(size_t(side()) != disp.viewing_team()+1) {
			if(disp.team_valid() &&
			   disp.get_teams()[disp.viewing_team()].is_enemy(side())) {
				orb_img = &enemy_orb;
			} else {
				orb_img = &ally_orb;
			}
		} else {
			orb_img = &moved_orb;
			if(disp.playing_team() == disp.viewing_team() && !user_end_turn()) {
				if (movement_left() == total_movement()) {
					orb_img = &unmoved_orb;
				} else if (unit_can_move(*this)) {
					orb_img = &partmoved_orb;
				}
			}
		}
		assert(orb_img != NULL);
		surface orb(image::get_image(*orb_img,image::SCALED_TO_ZOOM));
		if (orb != NULL) {
			disp.drawing_buffer_add(display::LAYER_UNIT_BAR,
				loc_, xsrc, ysrc +adjusted_params.y, orb);
		}

		double unit_energy = 0.0;
		if(max_hitpoints() > 0) {
			unit_energy = double(hitpoints())/double(max_hitpoints());
		}
		const int bar_shift = static_cast<int>(-5*disp.get_zoom_factor());
		const int hp_bar_height = static_cast<int>(max_hitpoints()*game_config::hp_bar_scaling);

		const fixed_t bar_alpha = (loc_ == disp.mouseover_hex() || loc_ == disp.selected_hex()) ? ftofxp(1.0): ftofxp(0.8);

		disp.draw_bar(*energy_file, xsrc+bar_shift, ysrc +adjusted_params.y,
			loc_, hp_bar_height, unit_energy,hp_color(), bar_alpha);

		if(experience() > 0 && can_advance()) {
			const double filled = double(experience())/double(max_experience());

			const int xp_bar_height = static_cast<int>(max_experience()*game_config::xp_bar_scaling / std::max<int>(level_,1));

			SDL_Color color=xp_color();
			disp.draw_bar(*energy_file, xsrc, ysrc +adjusted_params.y,
				loc_, xp_bar_height, filled, color, bar_alpha);
		}

		if (can_recruit()) {
			surface crown(image::get_image("misc/leader-crown.png",image::SCALED_TO_ZOOM));
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
		const game_display * disp =  game_display::get_singleton();
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
	if(cfg_[z_upkeep] == z_full) {
		return level();
	}
	if(cfg_[z_upkeep] == z_loyal) {
		return 0;
	}
	if(cfg_[z_upkeep] == z_free) {
		return 0;
	}
	return cfg_[z_upkeep];
}

bool unit::loyal() const
{
	return cfg_[z_upkeep] == z_loyal || cfg_[z_upkeep] == z_free;
}

int unit::movement_cost(const t_translation::t_terrain terrain, gamemap const & game_map) const
{
	const int res = movement_cost_internal(movement_costs_, cfg_, NULL, game_map, terrain);

	if (res == unit_movement_type::UNREACHABLE) {
		return res;
	} else if(get_state(STATE_SLOWED)) {
		return res*2;
	}
	return res;
}

int unit::defense_modifier(t_translation::t_terrain terrain, gamemap const & game_map) const
{
	//assert(resources::game_map != NULL);
	int def = defense_modifier_internal(defense_mods_, cfg_, NULL, game_map, terrain);
#if 0
	// A [defense] ability is too costly and doesn't take into account target locations.
	// Left as a comment in case someone ever wonders why it isn't a good idea.
	unit_ability_list defense_abilities = get_abilities(z_defense);
	if (!defense_abilities.empty()) {
		unit_abilities::effect defense_effect(defense_abilities, def, false);
		def = defense_effect.get_composite_value();
	}
#endif
	return def;
}

bool unit::resistance_filter_matches(const config& cfg, bool attacker, const std::string& damage_name, int res) const
{
	static const config::t_token z_empty("", false);
	if(!(cfg[z_active_on]==z_empty || (attacker && cfg[z_active_on]==z_offense) || (!attacker && cfg[z_active_on]==z_defense))) {
		return false;
	}
	const std::string& apply_to = cfg[z_apply_to];
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
	if (!unit_abilities::filter_base_matches(cfg, res)) return false;
	return true;
}


int unit::resistance_against(const std::string& damage_name,bool attacker,const map_location& loc) const
{
	int res = 0;

	if (const config &resistance = cfg_.child(z_resistance)) {
		res = 100 - resistance[damage_name].to_int(100);
	}

	unit_ability_list resistance_abilities = get_abilities(z_resistance,loc);
	for (std::vector<std::pair<const config *,map_location> >::iterator i = resistance_abilities.cfgs.begin(); i != resistance_abilities.cfgs.end();) {
		if(!resistance_filter_matches(*i->first, attacker, damage_name, res)) {
			i = resistance_abilities.cfgs.erase(i);
		} else {
			++i;
		}
	}
	if(!resistance_abilities.empty()) {
		unit_abilities::effect resist_effect(resistance_abilities,res,false);

		res = std::min<int>(resist_effect.get_composite_value(),resistance_abilities.highest(z_max_value).first);
	}
	return 100 - res;
}

utils::string_map unit::get_base_resistances() const
{
	if (const config &resistance = cfg_.child(z_resistance))
	{
		utils::string_map res;
		foreach (const config::attribute &i, resistance.attribute_range()) {
			res[i.first] = i.second;
		}
		return res;
	}
	return utils::string_map();
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
	if (!can_advance())
		return temp;

	if (!advances_to_.empty())
	{
		std::ostringstream tooltip;
		const std::string &image = game_config::images::level;
		foreach (const std::string &s, advances_to())
		{
			if (!s.empty())
				tooltip << s << '\n';
		}
		temp[image] = tooltip.str();
	}

	foreach (const config &adv, get_modification_advances())
	{
		const std::string &image = adv[z_image];
		if (image.empty()) continue;
		std::ostringstream tooltip;
		tooltip << temp[image];
		const std::string &tt = adv[z_description];
		if (!tt.empty())
			tooltip << tt << '\n';
		temp[image] = tooltip.str();
	}
	return(temp);
}
std::vector<std::pair<std::string,std::string> > unit::amla_icons() const
{
	std::vector<std::pair<std::string,std::string> > temp;
	std::pair<std::string,std::string> icon; //<image,tooltip>

	foreach (const config &adv, get_modification_advances())
	{
		icon.first = adv[z_icon].str();
		icon.second = adv[z_description].str();

		for (unsigned j = 0, j_count = modification_count(z_advance, adv[z_id]);
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
	foreach (const config &adv, modification_advancements())
	{
		if (adv[z_strict_amla].to_bool() && !advances_to_.empty())
			continue;
		if (modification_count(z_advance, adv[z_id]) >= unsigned(adv[z_max_times].to_int(1)))
			continue;

		std::vector<std::string> temp = utils::split(adv[z_require_amla]);
		if (temp.empty()) {
			res.push_back(adv);
			continue;
		}

		std::sort(temp.begin(), temp.end());
		std::vector<std::string> uniq;
		std::unique_copy(temp.begin(), temp.end(), std::back_inserter(uniq));

		bool requirements_done = true;
		foreach (const std::string &s, uniq)
		{
			int required_num = std::count(temp.begin(), temp.end(), s);
			int mod_num = modification_count(z_advance, s);
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

size_t unit::modification_count(const config::t_token& type, const std::string& id) const
{
	size_t res = 0;
	foreach (const config &item, modifications_.child_range(type)) {
		if (item[z_id] == config::t_token(id)) {
			++res;
		}
	}

	return res;
}

/** Helper function for add_modifications */
static void mod_mdr_merge(config& dst, const config& mod, bool delta)
{
	foreach (const config::attribute &i, mod.attribute_range()) {
		int v = 0;
		if (delta) v = dst[i.first];
		dst[i.first] = v + i.second.to_int();
	}
}

void unit::add_modification(const config::t_token& type, const config& mod, bool no_add
							, gamemap const & game_map, unit_map const & units, 
							t_teams const & teams, LuaKernel & lua_kernel, 
							tod_manager const & tod_manager)
{

	static const config::t_token z_empty("", false);

	//some trait activate specific flags
	if(type == z_trait) {
		const config::t_token& id = mod[z_id];
		is_fearless_ = is_fearless_ || id == z_fearless;
		is_healthy_ = is_healthy_ || id == z_healthy;
	}

	config *new_child = NULL;
	if(no_add == false) {
		new_child = &modifications_.add_child(type,mod);
	}
	config last_effect;
	std::vector<t_string> effects_description;
	foreach (const config &effect, mod.child_range(z_effect))
	{
		// See if the effect only applies to certain unit types
		const std::string &type_filter = effect[z_unit_type];
		if(type_filter.empty() == false) {
			const std::vector<std::string>& types = utils::split(type_filter);
			if(std::find(types.begin(),types.end(),type_id()) == types.end()) {
				continue;
			}
		}
		// See if the effect only applies to certain genders
		const config::t_token &gender_filter = effect[z_unit_gender];
		if(gender_filter.empty() == false) {
			const std::string& gender = gender_string(gender_);
			const std::vector<std::string>& genders = utils::split(gender_filter);
			if(std::find(genders.begin(),genders.end(),gender) == genders.end()) {
				continue;
			}
		}
		/** @todo The above two filters can be removed in 1.7 they're covered by the SUF. */
		// Apply SUF. (Filtering on location is probably a bad idea though.)
		if (const config &afilter = effect.child(z_filter))
		    if (!matches_filter(vconfig(afilter), map_location(cfg_, NULL), false ,game_map, units, teams, lua_kernel, tod_manager)) continue;

		const config::t_token &apply_to = effect[z_apply_to];
		const std::string &apply_times = effect[z_times];
		int times = 1;
		t_string description;

		if (apply_times == "per level")
			times = level_;
		if (times) {
			while (times > 0) {
				times --;

				// Apply unit type/variation changes last to avoid double applying effects on advance.
				if ((apply_to == z_variation || apply_to == z_type) && no_add == false) {
					last_effect = effect;
				} else if(apply_to == z_profile) {
					if (const config::attribute_value *v = effect.get(z_portrait)) {
						config::t_token big = *v;
						config::t_token small = effect[z_small_portrait];
						adjust_profile(small, big, z_empty);
						cfg_[z_profile] = big;
						cfg_[z_small_profile] = small;
					}
					if (const config::attribute_value *v = effect.get(z_description))
						cfg_[z_description] = *v;
					//help::unit_topic_generator(*this, (**i.first)[z_help_topic]);
				} else if(apply_to == z_new_attack) {
					attacks_.push_back(attack_type(effect));
				} else if(apply_to == z_remove_attacks) {
					for(std::vector<attack_type>::iterator a = attacks_.begin(); a != attacks_.end(); ++a) {
						if (a->matches_filter(effect, false)) {
							attacks_.erase(a--);
						}
					}
				} else if(apply_to == z_attack) {

					bool first_attack = true;

					std::string attack_names;
					std::pair<bool, config::t_token> affected;
					for(std::vector<attack_type>::iterator a = attacks_.begin();
						a != attacks_.end(); ++a) {
						affected = a->apply_modification(effect);
						if(affected.first && affected.second != z_empty) {
							if(first_attack) {
								first_attack = false;
							} else {
								if (!times)
									attack_names += t_string(N_(" and "), z_wesnoth);
							}

							if (!times)
								attack_names += t_string(a->name(), z_wesnoth);
						}
					}
					if (attack_names.empty() == false) {
						utils::string_map symbols;
						symbols[z_attack_list] = attack_names;
						symbols[z_effect_description] = affected.second;;
						description += vgettext("$attack_list|: $effect_description", symbols);
					}
				} else if(apply_to == z_hitpoints) {
					LOG_UT << "applying hitpoint mod..." << hit_points_ << "/" << max_hit_points_ << "\n";
					const std::string &increase_hp = effect[z_increase];
					const std::string &increase_total = effect[z_increase_total];
					const std::string &set_hp = effect[z_set];
					const std::string &set_total = effect[z_set_total];

					// If the hitpoints are allowed to end up greater than max hitpoints
					const bool violate_max = effect[z_violate_maximum].to_bool();

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
								t_string(N_(z_HP), z_wesnoth);

						// A percentage on the end means increase by that many percent
						max_hit_points_ = utils::apply_modifier(max_hit_points_, increase_total);
					}

					if(max_hit_points_ < 1)
						max_hit_points_ = 1;

					if (effect[z_heal_full].to_bool()) {
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
				} else if(apply_to == z_movement) {
					const std::string &increase = effect[z_increase];

					if(increase.empty() == false) {
						if (!times)
							description += utils::print_modifier(increase) + " " +
								t_string(N_(z_moves), z_wesnoth);

						max_movement_ = utils::apply_modifier(max_movement_, increase, 1);
					}

					max_movement_ = effect[z_set].to_int(max_movement_);

					if(movement_ > max_movement_)
						movement_ = max_movement_;
				} else if(apply_to == z_max_experience) {
					const std::string &increase = effect[z_increase];

					if(increase.empty() == false) {
						if (!times)
							description += utils::print_modifier(increase) + " " +
								t_string(N_("XP to advance"), z_wesnoth);

						max_experience_ = utils::apply_modifier(max_experience_, increase, 1);
					}

				} else if(apply_to == z_loyal) {
					cfg_[z_upkeep] = z_loyal;
				} else if(apply_to == z_status) {
					const config::t_token &add = effect[z_add];
					const config::t_token &remove = effect[z_remove];

					if(add.empty() == false) {
						set_state(add, true);
					}

					if(remove.empty() == false) {
						set_state(remove, false);
					}
				} else if (apply_to == z_movement_costs) {
					config &mv = cfg_.child_or_add(z_movement_costs);
					if (const config &ap = effect.child(z_movement_costs)) {
						mod_mdr_merge(mv, ap, !effect[z_replace].to_bool());
					}
					movement_costs_.clear();
				} else if (apply_to == z_defense) {
					config &def = cfg_.child_or_add(z_defense);
					if (const config &ap = effect.child(z_defense)) {
						bool replace = effect[z_replace].to_bool();
						foreach (const config::attribute &i, ap.attribute_range()) {
							int v = i.second.to_int();
							config::attribute_value &dst = def[i.first];
							if (!replace) {
								int w = dst.to_int(100);
								v += w;
								if ((w >= 0 && v < 0) || (w < 0 && v > 0)) v = 0;
								else if (v < -100) v = -100;
								else if (v > 100) v = 100;
							}
							dst = v;
						}
					}
					defense_mods_.clear();
				} else if (apply_to == z_resistance) {
					config &mv = cfg_.child_or_add(z_resistance);
					if (const config &ap = effect.child(z_resistance)) {
						mod_mdr_merge(mv, ap, !effect[z_replace].to_bool());
					}
				} else if (apply_to == z_zoc) {
					if (const config::attribute_value *v = effect.get(z_value)) {
						emit_zoc_ = v->to_bool();
					}
				} else if (apply_to == z_new_ability) {
					config &ab = cfg_.child_or_add(z_abilities);
					if (const config &ab_effect = effect.child(z_abilities)) {
						config to_append;
						foreach (const config::any_child &ab, ab_effect.all_children_range()) {
							if(!has_ability_by_id(ab.cfg[z_id])) {
								to_append.add_child(ab.key, ab.cfg);
							}
						}
						ab.append(to_append);
					}
				} else if (apply_to == z_remove_ability) {
					if (const config &ab_effect = effect.child(z_abilities)) {
						foreach (const config::any_child &ab, ab_effect.all_children_range()) {
							remove_ability_by_id(ab.cfg[z_id]);
						}
					}
				} else if (apply_to == z_image_mod) {
					LOG_UT << "applying image_mod \n";
					std::string mod = effect[z_replace];
					if (!mod.empty()){
						image_mods_ = mod;
					}
					LOG_UT << "applying image_mod \n";
					mod = effect[z_add].str();
					if (!mod.empty()){
						image_mods_ += mod;
					}

					game_config::add_color_info(effect);
					LOG_UT << "applying image_mod \n";
				} else if (apply_to == z_new_animation) {
					if(effect[z_id].empty()) {
						unit_animation::add_anims(animations_, effect);
					} else {
						std::vector<unit_animation> &built = resources::controller->animation_cache[effect[z_id]];
						if(built.empty()) {
							unit_animation::add_anims(built, effect);
						}
						animations_.insert(animations_.end(),built.begin(),built.end());
					}

				} else if (apply_to == z_ellipse) {
					cfg_[z_ellipse] = effect[z_ellipse];
				}
			} // end while
		} else { // for times = per level & level = 0 we still need to rebuild the descriptions
			if(apply_to == z_attack) {

				bool first_attack = true;

				for(std::vector<attack_type>::iterator a = attacks_.begin();
					a != attacks_.end(); ++a) {
					std::pair<bool, config::t_token> affected = a->describe_modification(effect);
					if(affected.first && affected.second != z_empty) {
						std::string const & desc = affected.second;
						if(first_attack) {
							first_attack = false;
						} else {
							description += t_string(N_(" and "), z_wesnoth);
						}

						description += t_string(a->name(), z_wesnoth) + ": " + desc;
					}
				}
			} else if(apply_to == z_hitpoints) {
				const std::string &increase_total = effect[z_increase_total];

				if(increase_total.empty() == false) {
					description += utils::print_modifier(increase_total) + " " +
						t_string(N_(z_HP), z_wesnoth);
				}
			} else if(apply_to == z_movement) {
				const std::string &increase = effect[z_increase];

				if(increase.empty() == false) {
					description += utils::print_modifier(increase) + t_string(N_(" move"), z_wesnoth);
				}
			} else if(apply_to == z_max_experience) {
				const std::string &increase = effect[z_increase];

				if(increase.empty() == false) {
					description += utils::print_modifier(increase) + " " +
						t_string(N_("XP to advance"), z_wesnoth);
				}
			}
		}

		if (apply_times == "per level" && !times) {
			utils::string_map symbols;
			symbols[z_effect_description] = description;
			description = vgettext("$effect_description per level", symbols);
		}
		if(!description.empty())
			effects_description.push_back(description);

	}
	// Apply variations -- only apply if we are adding this for the first time.
	if (!last_effect.empty() && no_add == false) {
		if ((last_effect)[z_apply_to] == z_variation) {
			variation_ = last_effect[z_name].token();
			advance_to(this->type());
		} else if ((last_effect)["apply_to"] == "type") {
			config::attribute_value &prev_type = (*new_child)["prev_type"];
			if (prev_type.blank()) prev_type = type_id();
			const std::string& type_id = last_effect["name"];
			const unit_type* type = unit_types.find(type_id);
			if(type) {
				const bool heal_full = last_effect["heal_full"].to_bool(false);
				int hit_points = hit_points_;
				advance_to(type);
				preferences::encountered_units().insert(type_id);
				if(!heal_full) {
					hit_points_ = hit_points;
				}
			} else {
				WRN_UT << "unknown type= in [effect]apply_to=type, ignoring\n";
			}
		}
	}

	t_string description;

	const t_string& mod_description = mod[z_description];
	if (!mod_description.empty()) {
		description = mod_description + " ";
	}

	// Punctuation should be translatable: not all languages use Latin punctuation.
	// (However, there maybe is a better way to do it)
	if(effects_description.empty() == false) {
		for(std::vector<t_string>::const_iterator i = effects_description.begin();
				i != effects_description.end(); ++i) {
			description += *i;
			if(i+1 != effects_description.end())
				description += t_string(N_(" and "), z_wesnoth);
		}
	}

	// store trait info
	if(type == z_trait) {
		add_trait_description(mod, description);
	}

	//NOTE: if not a trait, description is currently not used
}

void unit::add_trait_description(const config& trait, const t_string& description)
{
	const std::string& gender_string = gender_ == unit_race::FEMALE ? z_female_name : z_male_name;
	t_string const &gender_specific_name = trait[gender_string];

	// if this is a t_string& instead of a t_string, msvc9 compiled windows binaries
	// choke on the case where both gender_specific_name and trait[z_name] are empty.
	const t_string& name = gender_specific_name.empty() ?
		 trait[z_name] : gender_specific_name;

	if(!name.empty()) {
		trait_names_.push_back(name);
		trait_descriptions_.push_back(description);
	}
}

const unit_animation* unit::choose_animation(const game_display& disp, const map_location& loc,const n_token::t_token& event,
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

	for(size_t i = 0; i != NumModificationTypes; ++i) {
		const std::string& mod = ModificationTypes[i];
		foreach (const config &m, modifications_.child_range(mod)) {
			log_scope("add mod");
			add_modification(ModificationTypes[i], m, true);
		}
	}

	//apply the experience acceleration last
	int exp_accel = unit_type::experience_accelerator::get_acceleration();
	max_experience_ = std::max<int>(1, (max_experience_ * exp_accel + 50)/100);
}

bool unit::invisible(const map_location& loc, bool see_all, unit_map const & units, t_teams const & teams) const
{
	// Fetch from cache
	/**
	 * @todo FIXME: We use the cache only when using the default see_all=true
	 * Maybe add a second cache if the see_all=false become more frequent.
	 */
	if(see_all) {
		boost::unordered_map<map_location, bool>::const_iterator itor = invisibility_cache_.find(loc);
		if(itor != invisibility_cache_.end()) {
			return itor->second;
		}
	}

	// Test hidden status
	static const config::t_token hides(z_hides);
	bool is_inv = !get_state(STATE_UNCOVERED) && get_ability_bool(hides,loc);
	if(is_inv){
		foreach (const unit &u, units)
		{
			const map_location &u_loc = u.get_location();
			if (teams[side_-1].is_enemy(u.side()) && tiles_adjacent(loc, u_loc)) {
				// Enemy spotted in adjacent tiles, check if we can see him.
				// Watch out to call invisible with see_all=true to avoid infinite recursive calls!
				if(see_all) {
					is_inv = false;
					break;
				} else if (!teams[side_-1].fogged(u_loc)
						   //&& !u.invisible(u_loc, true)) {
						   && !u.invisible(u_loc, true,units,teams)) {
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

void unit::set_underlying_id() {
	if(underlying_id_ == 0){
		underlying_id_ = n_unit::id_manager::instance().next_id();
	}
	if (id_.empty()) {
		std::stringstream ss;
		ss << (type_.empty()?z_Unit:type_) << "-" << underlying_id_;
		id_ = config::t_token(ss.str());
	}
}

unit& unit::clone(bool is_temporary)
{
	if(is_temporary) {
		underlying_id_ = n_unit::id_manager::instance().next_fake_id();
	} else {
		underlying_id_ = n_unit::id_manager::instance().next_id();
		std::string::size_type pos = static_cast< std::string const &>(id_).find_last_of('-');
		if(pos != std::string::npos && pos+1 < static_cast< std::string const &>(id_).size()
		   && static_cast< std::string const &>(id_).find_first_not_of("0123456789", pos+1) == std::string::npos) {
			// this appears to be a duplicate of a generic unit, so give it a new id
			WRN_UT << "assigning new id to clone of generic unit " << id_ << "\n";
			static const config::t_token z_empty("", false);
			id_ = z_empty;
			set_underlying_id();
		}
	}
	return *this;
}


unit_movement_resetter::unit_movement_resetter(unit &u, bool operate) :
	u_(u), moves_(u.movement_)
{
	if (operate) {
		u.movement_ = u.total_movement();
	}
}

unit_movement_resetter::~unit_movement_resetter()
{
	u_.movement_ = moves_;
}

int side_units(int side, unit_map const & units)
{
	int res = 0;
	foreach (const unit &u, units) {
		if (u.side() == side) ++res;
	}
	return res;
}

int side_units_cost(int side, unit_map const & units)
{
	int res = 0;
	foreach (const unit &u, units) {
		if (u.side() == side) res += u.cost();
	}
	return res;
}

int side_upkeep(int side, unit_map const & units)
{
	int res = 0;
	foreach (const unit &u, units) {
		if (u.side() == side) res += u.upkeep();
	}
	return res;
}

unit_map::const_iterator find_visible_unit( unit_map const& units, const map_location &loc, const team& current_team, bool const see_all, gamemap const & map)
{
	if (!map.on_board(loc)) return units.end();
	unit_map::const_iterator u = units.find(loc);
	if (see_all) return u;
	if (!u.valid() || current_team.fogged(loc) ||
	    (current_team.is_enemy(u->side()) &&
	     u->invisible(loc)))
		return units.end();
	return u;
}

unit_map::iterator find_visible_unit(unit_map & units, const map_location &loc, const team& current_team, bool const see_all, gamemap const & map)
{
	if (!map.on_board(loc)) return units.end();
	unit_map::iterator u = units.find(loc);
	if (see_all) return u;
	if (!u.valid() || current_team.fogged(loc) ||
	    (current_team.is_enemy(u->side()) &&
	     u->invisible(loc)))
		return units.end();
	return u;
}

unit *get_visible_unit(const map_location &loc, const team &current_team, bool const see_all, gamemap const & map, unit_map & units)
{
	unit_map::iterator ui = find_visible_unit(units, loc, current_team, see_all, map);
	if (ui == units.end()) return NULL;
	return &*ui;
}

void unit::refresh()
{
	if (state_ == STATE_FORGET && anim_ && anim_->animation_finished_potential())
	{
		set_standing();
		return;
	}
	game_display &disp = *game_display::get_singleton();
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
	res.expenses = std::max<int>(0,res.upkeep - res.villages);
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

temporary_unit_mover::temporary_unit_mover(unit_map& m, const map_location& src,  const map_location& dst)
	: m_(m), src_(src), dst_(dst), temp_(m.extract(dst))
{
	m.move(src_, dst_);
}

temporary_unit_mover::~temporary_unit_mover()
{
	m_.move(dst_, src_);
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
	modifier << TC_image_mods();
	if(!image_mods_.empty()){
		modifier << "~" << image_mods_;
	}
	return modifier.str();
}

const tportrait* unit::portrait(
		const unsigned size, const tportrait::tside side) const
{
	foreach(const tportrait& portrait, (type()->portraits())) {
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
	set_movement(0);
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
	static const config::t_token z_empty("", false);

	static const n_token::t_token main_keys[] =
		{ z_advances_to,
		  z_alignment,
		  z_cost,
		  z_experience,
		  z_gender,
		  z_hitpoints,
		  z_ignore_race_traits,
		  z_ignore_global_traits,
		  z_level,
		  z_max_attacks,
		  z_max_experience,
		  z_max_hitpoints,
		  z_max_moves,
		  z_movement,
		  z_movement_type,
		  z_race,
		  z_random_traits,
		  z_resting,
		  z_undead_variation,
		  z_upkeep,
		  z_zoc };

	foreach(n_token::t_token const & k, main_keys) {
		wcfg[k ] = unit_config[ k ];
	}
	const config::t_token attack_keys[] =
		{ z_name,
		  z_type,
		  z_range,
		  z_damage,
		  z_number };

	foreach (const config &att, unit_config.child_range(z_attack)) {
		config& child = wcfg.add_child(z_attack);
		foreach (n_token::t_token const & k, attack_keys){
			child[k] = att[k];
		}
		foreach (const config &spec, att.child_range(z_specials)) {
			config& child_spec = child.add_child(z_specials, spec);
			child_spec.recursive_clear_value(z_description);
		}

	}

	foreach (const config &abi, unit_config.child_range(z_abilities)) {
		config& child = wcfg.add_child(z_abilities, abi);
		child.recursive_clear_value(z_description);
		child.recursive_clear_value(z_description_inactive);
		child.recursive_clear_value(z_name);
		child.recursive_clear_value(z_name_inactive);
	}

	foreach (const config &trait, unit_config.child_range(z_trait)) {
		config& child = wcfg.add_child(z_trait, trait);
		child.recursive_clear_value(z_description);
		child.recursive_clear_value(z_male_name);
		child.recursive_clear_value(z_female_name);
		child.recursive_clear_value(z_name);
	}

	static const n_token::t_token child_keys[] = {z_advance_from, z_defense, z_movement_costs, z_resistance };

	foreach (n_token::t_token const & k, child_keys) {
		foreach (const config &c, unit_config.child_range( k )) {
			wcfg.add_child(k, c);
		}
	} 
	DBG_UT << wcfg;

	return wcfg.hash();
}

