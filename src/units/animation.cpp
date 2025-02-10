/*
	Copyright (C) 2006 - 2024
	by Jeremy Rosen <jeremy.rosen@enst-bretagne.fr>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "units/animation.hpp"

#include "color.hpp"
#include "display.hpp"
#include "global.hpp"
#include "map/map.hpp"
#include "play_controller.hpp"
#include "random.hpp"
#include "resources.hpp"
#include "serialization/chrono.hpp"
#include "units/animation_component.hpp"
#include "units/filter.hpp"
#include "units/unit.hpp"
#include "variable.hpp"
#include "log.hpp" 
#include "game_events/action_wml.cpp"

#include <algorithm>
#include <thread>

using namespace std::chrono_literals;

static lg::log_domain log_scripting_lua("scripting/lua");
#define DBG_LUA LOG_STREAM(debug, log_scripting_lua)
#define LOG_LUA LOG_STREAM(info, log_scripting_lua)
#define WRN_LUA LOG_STREAM(warn, log_scripting_lua)
#define ERR_LUA LOG_STREAM(err, log_scripting_lua)


static std::string get_heal_sound(const config& cfg)
{
	return cfg["healed_sound"].empty() ? "heal.wav" : cfg["healed_sound"].str();
}

struct animation_branch
{
	config merge() const
	{
		config result = attributes;
		for(const config::const_all_children_iterator& i : children) {
			result.add_child(i->key, i->cfg);
		}

		return result;
	}

	config attributes;
	std::vector<config::const_all_children_iterator> children;
};

typedef std::list<animation_branch> animation_branches;

struct animation_cursor
{
	animation_cursor(const config& cfg)
		: itors(cfg.all_children_range()), branches(1), parent(nullptr)
	{
		branches.back().attributes.merge_attributes(cfg);
	}

	animation_cursor(const config&cfg, animation_cursor *p)
		: itors(cfg.all_children_range()), branches(p->branches), parent(p)
	{
		// If similar 'if' condition in parent branches, we need to
		// cull the branches where there are partial matches.
		// Hence the need to check if the condition has come up before.
		// Also, the attributes are merged here between branches.
		bool previously_hits_set      = false;
		bool previously_direction_set = false;
		bool previously_terrain_set   = false;
		bool previously_value_set     = false;
		bool previously_value_2nd_set = false;

		const std::string s_cfg_hits      = cfg["hits"];
		const std::string s_cfg_direction = cfg["direction"];
		const std::string s_cfg_terrain   = cfg["terrain_types"];
		const std::string s_cfg_value     = cfg["value"];
		const std::string s_cfg_value_2nd = cfg["value_2nd"];

		for(const auto& branch : branches) {
			const std::string s_branch_hits      = branch.attributes["hits"];
			const std::string s_branch_direction = branch.attributes["direction"];
			const std::string s_branch_terrain   = branch.attributes["terrain_types"];
			const std::string s_branch_value     = branch.attributes["value"];
			const std::string s_branch_value_2nd = branch.attributes["value_second"];

			if(!s_branch_hits.empty() && s_branch_hits == s_cfg_hits) {
				previously_hits_set = true;
			}

			if(!s_branch_direction.empty() && s_branch_direction == s_cfg_direction) {
				previously_direction_set = true;
			}

			if(!s_branch_terrain.empty() && s_branch_terrain == s_cfg_terrain) {
				previously_terrain_set = true;
			}

			if(!s_branch_value.empty() && s_branch_value == s_cfg_value) {
				previously_value_set = true;
			}

			if(!s_branch_value_2nd.empty() && s_branch_value_2nd == s_cfg_value_2nd) {
				previously_value_2nd_set = true;
			}
		}

		// Merge all frames that have new matches and prune any impossible
		// matches, e.g. hits='yes' and hits='no'
		for(auto iter = branches.begin(); iter != branches.end(); /* nothing */) {
			const std::string s_branch_hits      = (*iter).attributes["hits"];
			const std::string s_branch_direction = (*iter).attributes["direction"];
			const std::string s_branch_terrain   = (*iter).attributes["terrain_types"];
			const std::string s_branch_value     = (*iter).attributes["value"];
			const std::string s_branch_value_2nd = (*iter).attributes["value_second"];

			const bool hits_match      = (previously_hits_set && s_branch_hits != s_cfg_hits);
			const bool direction_match = (previously_direction_set && s_branch_direction != s_cfg_direction);
			const bool terrain_match   = (previously_terrain_set && s_branch_terrain != s_cfg_terrain);
			const bool value_match     = (previously_value_set && s_branch_value != s_cfg_value);
			const bool value_2nd_match = (previously_value_2nd_set && s_branch_value_2nd != s_cfg_value_2nd);

			if((!previously_hits_set      || hits_match) &&
			   (!previously_direction_set || direction_match) &&
			   (!previously_terrain_set   || terrain_match) &&
			   (!previously_value_set     || value_match)  &&
			   (!previously_value_2nd_set || value_2nd_match) &&
			    (hits_match || direction_match || terrain_match || value_match || value_2nd_match))
			{
				branches.erase(iter++);
			} else {
				(*iter).attributes.merge_attributes(cfg);
				++iter;
			}
		}

		// Then we prune all parent branches with similar matches as they
		// now will not have the full frame list
		for(auto iter = parent->branches.begin(); iter != parent->branches.end(); /* nothing */) {
			const std::string s_branch_hits      = (*iter).attributes["hits"];
			const std::string s_branch_direction = (*iter).attributes["direction"];
			const std::string s_branch_terrain   = (*iter).attributes["terrain_types"];
			const std::string s_branch_value     = (*iter).attributes["value"];
			const std::string s_branch_value_2nd = (*iter).attributes["value_second"];

			const bool hits_match      = (previously_hits_set && s_branch_hits == s_cfg_hits);
			const bool direction_match = (previously_direction_set && s_branch_direction == s_cfg_direction);
			const bool terrain_match   = (previously_terrain_set && s_branch_terrain == s_cfg_terrain);
			const bool value_match     = (previously_value_set && s_branch_value == s_cfg_value);
			const bool value_2nd_match = (previously_value_2nd_set && s_branch_value_2nd == s_cfg_value_2nd);

			if((!previously_hits_set        || hits_match) &&
			     (!previously_direction_set || direction_match) &&
			     (!previously_terrain_set   || terrain_match) &&
			     (!previously_value_set     || value_match) &&
			     (!previously_value_2nd_set || value_2nd_match) &&
			     (hits_match || direction_match || terrain_match || value_match || value_2nd_match))
			{
				parent->branches.erase(iter++);
			} else {
				++iter;
			}
		}
	}

	config::const_all_children_itors itors;

	animation_branches branches;
	animation_cursor* parent;
};

static void prepare_single_animation(const config& anim_cfg, animation_branches& expanded_anims)
{
	/* The anim_cursor holds the current parsing through the config and the branches hold the data
	 * that will be interpreted as the actual animation. The branches store the config attributes
	 * for each block and the children of those branches make up all the 'frame', 'missile_frame',
	 * etc. individually (so 2 instances of 'frame' would be stored as 2 children).
	 */
	std::list<animation_cursor> anim_cursors;
	anim_cursors.emplace_back(anim_cfg);

	while(!anim_cursors.empty()) {
		animation_cursor& ac = anim_cursors.back();

		// Reached end of sub-tag config block
		if(ac.itors.empty()) {
			if(!ac.parent) break;

			// Merge all the current branches into the parent.
			ac.parent->branches.splice(ac.parent->branches.end(), ac.branches);
			anim_cursors.pop_back();
			continue;
		}

		if(ac.itors.front().key != "if") {
			// Append current config object to all the branches in scope.
			for(animation_branch &ab : ac.branches) {
				ab.children.push_back(ac.itors.begin());
			}

			ac.itors.pop_front();
			continue;
		}

		int count = 0;
		do {
			// Copies the current branches to each cursor created for the conditional clauses.
			// Merge attributes of the clause into them.
			anim_cursors.emplace_back(ac.itors.front().cfg, &ac);
			ac.itors.pop_front();
			++count;
		} while (!ac.itors.empty() && ac.itors.front().key == "else");

		if(count > 1) {
			// When else statements present, clear all branches before 'if'
			ac.branches.clear();
		}
	}

#if 0
	// Debug aid
	for(animation_branch& ab : anim_cursors.back().branches) {
		std::cout << "--branch--\n" << ab.attributes;
		for(config::all_children_iterator &ci : ab.children) {
			std::cout << "--branchcfg--\n" << ci->cfg;
		}
		std::cout << "\n";
	}
#endif

	// Create the config object describing each branch.
	assert(anim_cursors.size() == 1);
	animation_cursor& ac = anim_cursors.back();
	expanded_anims.splice(expanded_anims.end(), ac.branches, ac.branches.begin(), ac.branches.end());
}

static animation_branches prepare_animation(const config& cfg, const std::string& animation_tag)
{
	animation_branches expanded_animations;
	for(const config &anim : cfg.child_range(animation_tag)) {
		prepare_single_animation(anim, expanded_animations);
	}

	return expanded_animations;
}

unit_animation::unit_animation(const std::chrono::milliseconds& start_time,
		const unit_frame& frame, const std::string& event, const int variation, const frame_builder& builder)
	: terrain_types_()
	, unit_filter_()
	, secondary_unit_filter_()
	, directions_()
	, frequency_(0)
	, base_score_(variation)
	, event_(utils::split(event))
	, value_()
	, primary_attack_filter_()
	, secondary_attack_filter_()
	, hits_()
	, value2_()
	, sub_anims_()
	, unit_anim_(start_time,builder)
	, src_()
	, dst_()
	, invalidated_(false)
	, play_offscreen_(true)
	, overlaped_hex_()
{
	add_frame(frame.duration(),frame,!frame.does_not_change());
}

unit_animation::unit_animation(const config& cfg,const std::string& frame_string )
	: terrain_types_(t_translation::read_list(cfg["terrain_type"].str()))
	, unit_filter_()
	, secondary_unit_filter_()
	, directions_()
	, frequency_(cfg["frequency"].to_int())
	, base_score_(cfg["base_score"].to_int())
	, event_()
	, value_()
	, primary_attack_filter_()
	, secondary_attack_filter_()
	, hits_()
	, value2_()
	, sub_anims_()
	, unit_anim_(cfg,frame_string)
	, src_()
	, dst_()
	, invalidated_(false)
	, play_offscreen_(true)
	, overlaped_hex_()
{
	//if(!cfg["debug"].empty()) printf("DEBUG WML: FINAL\n%s\n\n",cfg.debug().c_str());

	for(const auto [key, frame] : cfg.all_children_view()) {
		if(key == frame_string) {
			continue;
		}

		if(key.find("_frame", key.size() - 6) == std::string::npos) {
			continue;
		}

		if(sub_anims_.find(key) != sub_anims_.end()) {
			continue;
		}

		sub_anims_[key] = particle(cfg, key.substr(0, key.size() - 5));
	}

	event_ = utils::split(cfg["apply_to"]);

	const std::vector<std::string>& my_directions = utils::split(cfg["direction"]);
	for(const auto& direction :  my_directions) {
		const map_location::direction d = map_location::parse_direction(direction);
		directions_.push_back(d);
	}

	/*const filter_context* fc = game_display::get_singleton();
	if(!fc) {
		// This is a pointer to the gamestate. Would prefer to tie unit animations only to the display, but for now this
		// is an acceptable fallback. It seems to be relevant because when a second game is created, it seems that the
		// game_display is null at the time that units are being constructed, and hence at the time that this code is running.
		// A different solution might be to delay the team_builder stage 2 call until after the gui is initialized. Note that
		// the current set up could conceivably cause problems with the editor, iirc it doesn't initialize a filter context.
		fc = resources::filter_con;
		assert(fc);
	}*/

	for(const config& filter : cfg.child_range("filter")) {
		unit_filter_.push_back(filter);
	}

	for(const config& filter : cfg.child_range("filter_second")) {
		secondary_unit_filter_.push_back(filter);
	}

	for(const auto& v : utils::split(cfg["value"])) {
		value_.push_back(atoi(v.c_str()));
	}

	for(const auto& h : utils::split(cfg["hits"])) {
		if(h == "yes" || h == strike_result::hit) {
			hits_.push_back(strike_result::type::hit);
		}

		if(h == "no" || h == strike_result::miss) {
			hits_.push_back(strike_result::type::miss);
		}

		if(h == "yes" || h == strike_result::kill ) {
			hits_.push_back(strike_result::type::kill);
		}
	}

	for(const auto& v2 : utils::split(cfg["value_second"])) {
		value2_.push_back(atoi(v2.c_str()));
	}

	for(const config& filter : cfg.child_range("filter_attack")) {
		primary_attack_filter_.push_back(filter);
	}

	for(const config& filter : cfg.child_range("filter_second_attack")) {
		secondary_attack_filter_.push_back(filter);
	}

	play_offscreen_ = cfg["offscreen"].to_bool(true);
}
int unit_animation::matches(const map_location& loc, const map_location& second_loc,
		unit_const_ptr my_unit, const std::string& event, const int value, strike_result::type hit, const_attack_ptr attack,
		const_attack_ptr second_attack, int value2) const
{
	int result = base_score_;
	const display& disp = *display::get_singleton();

	if(!event.empty() && !event_.empty()) {
		if(std::find(event_.begin(), event_.end(), event) == event_.end()) {
			return MATCH_FAIL;
		}

		result++;
	}

	if(!terrain_types_.empty()) {
		if(!t_translation::terrain_matches(disp.context().map().get_terrain(loc), terrain_types_)) {
			return MATCH_FAIL;
		}

		result++;
	}

	if(!value_.empty()) {
		if(std::find(value_.begin(), value_.end(), value) == value_.end()) {
			return MATCH_FAIL;
		}

		result++;
	}

	if(my_unit) {
		if(!directions_.empty()) {
			if(std::find(directions_.begin(), directions_.end(), my_unit->facing()) == directions_.end()) {
				return MATCH_FAIL;
			}

			result++;
		}

		for(const auto& filter : unit_filter_) {
			unit_filter f{ vconfig(filter) };
			if(!f(*my_unit, loc)) return MATCH_FAIL;
			++result;
		}

		if(!secondary_unit_filter_.empty()) {
			unit_map::const_iterator unit = disp.context().units().find(second_loc);
			if(!unit.valid()) {
				return MATCH_FAIL;
			}

			for(const config& c : secondary_unit_filter_) {
				unit_filter f{ vconfig(c) };
				if(!f(*unit, second_loc)) return MATCH_FAIL;
				result++;
			}
		}
	} else if(!unit_filter_.empty()) {
		return MATCH_FAIL;
	}

	if(frequency_ && !(randomness::rng::default_instance().get_random_int(0, frequency_-1))) {
		return MATCH_FAIL;
	}

	if(!hits_.empty()) {
		if(std::find(hits_.begin(),hits_.end(),hit) == hits_.end()) {
			return MATCH_FAIL;
		}

		result ++;
	}

	if(!value2_.empty()) {
		if(std::find(value2_.begin(),value2_.end(),value2) == value2_.end()) {
			return MATCH_FAIL;
		}

		result ++;
	}

	if(!attack) {
		if(!primary_attack_filter_.empty()) {
			return MATCH_FAIL;
		}
	}

	for(const auto& iter : primary_attack_filter_) {
		if(!attack->matches_filter(iter)) return MATCH_FAIL;
		result++;
	}

	if(!second_attack) {
		if(!secondary_attack_filter_.empty()) {
			return MATCH_FAIL;
		}
	}

	for(const auto& iter : secondary_attack_filter_) {
		if(!second_attack->matches_filter(iter)) return MATCH_FAIL;
		result++;
	}

	return result;
}

void unit_animation::fill_initial_animations(std::vector<unit_animation>& animations, const config& cfg)
{
	add_anims(animations, cfg);

	std::vector<unit_animation> animation_base;
	for(const auto& anim : animations) {
		if(std::find(anim.event_.begin(), anim.event_.end(), "default") != anim.event_.end()) {
			animation_base.push_back(anim);
			animation_base.back().base_score_ += unit_animation::DEFAULT_ANIM;
			animation_base.back().event_.clear();
		}
	}

	const std::string default_image = cfg["image"];

	if(animation_base.empty()) {
		animation_base.push_back(unit_animation(0ms, frame_builder().image(default_image).duration(1ms), "", unit_animation::DEFAULT_ANIM));
	}

	animations.push_back(unit_animation(0ms, frame_builder().image(default_image).duration(1ms), "_disabled_", 0));
	animations.push_back(unit_animation(0ms,
		frame_builder().image(default_image).duration(300ms).blend("0.0~0.3:100,0.3~0.0:200", {255,255,255}),
		"_disabled_selected_", 0));

	for(const auto& base : animation_base) {
		animations.push_back(base);
		animations.back().event_ = { "standing" };
		animations.back().play_offscreen_ = false;

		animations.push_back(base);
		animations.back().event_ = { "_ghosted_" };
		animations.back().unit_anim_.override(0ms, animations.back().unit_anim_.get_animation_duration(),particle::UNSET,"0.9", "", {0,0,0}, "", "", "~GS()");

		animations.push_back(base);
		animations.back().event_ = { "_disabled_ghosted_" };
		animations.back().unit_anim_.override(0ms, 1ms, particle::UNSET, "0.4", "", {0,0,0}, "", "", "~GS()");

		animations.push_back(base);
		animations.back().event_ = { "selected" };
		animations.back().unit_anim_.override(0ms, 300ms, particle::UNSET, "", "0.0~0.3:100,0.3~0.0:200", {255,255,255});

		animations.push_back(base);
		animations.back().event_ = { "recruited" };
		animations.back().unit_anim_.override(0ms, 600ms, particle::NO_CYCLE, "0~1:600");

		animations.push_back(base);
		animations.back().event_ = { "levelin" };
		animations.back().unit_anim_.override(0ms, 600ms, particle::NO_CYCLE, "", "1~0:600", {255,255,255});

		animations.push_back(base);
		animations.back().event_ = { "levelout" };
		animations.back().unit_anim_.override(0ms, 600ms, particle::NO_CYCLE, "", "0~1:600,1", {255,255,255});

		animations.push_back(base);
		animations.back().event_ = { "pre_movement" };
		animations.back().unit_anim_.override(0ms, 1ms, particle::NO_CYCLE);

		animations.push_back(base);
		animations.back().event_ = { "post_movement" };
		animations.back().unit_anim_.override(0ms, 1ms, particle::NO_CYCLE);

		animations.push_back(base);
		animations.back().event_ = { "movement" };
		animations.back().unit_anim_.override(0ms, 200ms,
			particle::NO_CYCLE, "", "", {0,0,0}, "0~1:200", std::to_string(get_abs_frame_layer(drawing_layer::unit_move_default)));

		animations.push_back(base);
		animations.back().event_ = { "defend" };
		animations.back().unit_anim_.override(0ms, animations.back().unit_anim_.get_animation_duration(),
			particle::NO_CYCLE, "", "0.0,0.5:75,0.0:75,0.5:75,0.0", {255,0,0});
		animations.back().hits_.push_back(strike_result::type::hit);
		animations.back().hits_.push_back(strike_result::type::kill);

		animations.push_back(base);
		animations.back().event_ = { "defend" };

		animations.push_back(base);
		animations.back().event_ = { "attack" };
		animations.back().unit_anim_.override(-150ms, 300ms, particle::NO_CYCLE, "", "", {0,0,0}, "0~0.6:150,0.6~0:150", std::to_string(get_abs_frame_layer(drawing_layer::unit_move_default)));
		animations.back().primary_attack_filter_.emplace_back("range", "melee");

		animations.push_back(base);
		animations.back().event_ = { "attack" };
		animations.back().unit_anim_.override(-150ms, 150ms, particle::NO_CYCLE);
		animations.back().primary_attack_filter_.emplace_back("range", "ranged");

		animations.push_back(base);
		animations.back().event_ = { "death" };
		animations.back().unit_anim_.override(0ms, 600ms, particle::NO_CYCLE, "1~0:600");
		animations.back().sub_anims_["_death_sound"] = particle();
		animations.back().sub_anims_["_death_sound"].add_frame(1ms, frame_builder().sound(cfg["die_sound"]), true);

		animations.push_back(base);
		animations.back().event_ = { "victory" };
		animations.back().unit_anim_.override(0ms, animations.back().unit_anim_.get_animation_duration(), particle::CYCLE);

		animations.push_back(base);
		animations.back().unit_anim_.override(0ms, 150ms, particle::NO_CYCLE, "1~0:150");
		animations.back().event_ = { "pre_teleport" };

		animations.push_back(base);
		animations.back().unit_anim_.override(0ms, 150ms, particle::NO_CYCLE, "0~1:150,1");
		animations.back().event_ = { "post_teleport" };

		animations.push_back(base);
		animations.back().event_ = { "healing" };

		animations.push_back(base);
		animations.back().event_ = { "healed" };
		animations.back().unit_anim_.override(0ms, 300ms, particle::NO_CYCLE, "", "0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30", {255,255,255});

		const std::string healed_sound = get_heal_sound(cfg);

		animations.back().sub_anims_["_healed_sound"].add_frame(1ms, frame_builder().sound(healed_sound), true);

		animations.push_back(base);
		animations.back().event_ = { "poisoned" };
		animations.back().unit_anim_.override(0ms, 300ms, particle::NO_CYCLE, "", "0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30", {0,255,0});
		animations.back().sub_anims_["_poison_sound"] = particle();
		animations.back().sub_anims_["_poison_sound"].add_frame(1ms, frame_builder().sound(game_config::sounds::status::poisoned), true);
	}
}

static void add_simple_anim(std::vector<unit_animation>& animations,
	const config& cfg, char const* tag_name, char const* apply_to,
	drawing_layer layer = drawing_layer::unit_default,
	bool offscreen = true)
{
	for(const animation_branch& ab : prepare_animation(cfg, tag_name)) {
		config anim = ab.merge();
		anim["apply_to"] = apply_to;

		if(!offscreen) {
			config::attribute_value& v = anim["offscreen"];
			if(v.empty()) v = false;
		}

		config::attribute_value& v = anim["layer"];
		if(v.empty()) v = get_abs_frame_layer(layer);

		animations.emplace_back(anim);
	}
}

void unit_animation::add_anims( std::vector<unit_animation> & animations, const config & cfg)
{
	for(const animation_branch& ab : prepare_animation(cfg, "animation")) {
		animations.emplace_back(ab.merge());
	}

	constexpr int default_layer = get_abs_frame_layer(drawing_layer::unit_default);
	constexpr int move_layer    = get_abs_frame_layer(drawing_layer::unit_move_default);
	constexpr int missile_layer = get_abs_frame_layer(drawing_layer::unit_missile_default);

	add_simple_anim(animations, cfg, "resistance_anim", "resistance");
	add_simple_anim(animations, cfg, "leading_anim", "leading");
	add_simple_anim(animations, cfg, "teaching_anim", "teaching");
	add_simple_anim(animations, cfg, "recruit_anim", "recruited");
	add_simple_anim(animations, cfg, "recruiting_anim", "recruiting");
	add_simple_anim(animations, cfg, "idle_anim", "idling", drawing_layer::unit_default, false);
	add_simple_anim(animations, cfg, "levelin_anim", "levelin");
	add_simple_anim(animations, cfg, "levelout_anim", "levelout");

	for(const animation_branch& ab : prepare_animation(cfg, "standing_anim")) {
		config anim = ab.merge();
		anim["apply_to"] = "standing";
		anim["cycles"] = true;

		// Add cycles to all frames within a standing animation block
		for(config::const_all_children_iterator ci : ab.children) {
			std::string sub_frame_name = ci->key;
			std::size_t pos = sub_frame_name.find("_frame");
			if(pos != std::string::npos) {
				anim[sub_frame_name.substr(0, pos) + "_cycles"] = true;
			}
		}

		if(anim["layer"].empty()) {
			anim["layer"] = default_layer;
		}

		if(anim["offscreen"].empty()) {
			anim["offscreen"] = false;
		}

		animations.emplace_back(anim);
	}

	// Standing animations are also used as default animations
	for(const animation_branch& ab : prepare_animation(cfg, "standing_anim")) {
		config anim = ab.merge();
		anim["apply_to"] = "default";
		anim["cycles"] = true;

		for(config::const_all_children_iterator ci : ab.children) {
			std::string sub_frame_name = ci->key;
			std::size_t pos = sub_frame_name.find("_frame");
			if(pos != std::string::npos) {
				anim[sub_frame_name.substr(0, pos) + "_cycles"] = true;
			}
		}

		if(anim["layer"].empty()) {
			anim["layer"] = default_layer;
		}

		if(anim["offscreen"].empty()) {
			anim["offscreen"] = false;
		}

		animations.emplace_back(anim);
	}

	for(const animation_branch& ab : prepare_animation(cfg, "healing_anim")) {
		config anim = ab.merge();
		anim["apply_to"] = "healing";
		anim["value"] = anim["damage"];

		if(anim["layer"].empty()) {
			anim["layer"] = default_layer;
		}

		animations.emplace_back(anim);
	}

	for(const animation_branch& ab : prepare_animation(cfg, "healed_anim")) {
		config anim = ab.merge();
		anim["apply_to"] = "healed";
		anim["value"] = anim["healing"];

		if(anim["layer"].empty()) {
			anim["layer"] = default_layer;
		}

		animations.emplace_back(anim);
		animations.back().sub_anims_["_healed_sound"] = particle();

		const std::string healed_sound = get_heal_sound(cfg);
		animations.back().sub_anims_["_healed_sound"].add_frame(1ms,frame_builder().sound(healed_sound),true);
	}

	for(const animation_branch &ab : prepare_animation(cfg, "poison_anim")) {
		config anim = ab.merge();
		anim["apply_to"] = "poisoned";
		anim["value"] = anim["damage"];

		if(anim["layer"].empty()) {
			anim["layer"] = default_layer;
		}

		animations.emplace_back(anim);
		animations.back().sub_anims_["_poison_sound"] = particle();
		animations.back().sub_anims_["_poison_sound"].add_frame(1ms,frame_builder().sound(game_config::sounds::status::poisoned),true);
	}

	add_simple_anim(animations, cfg, "pre_movement_anim", "pre_movement", drawing_layer::unit_move_default);

	for(const animation_branch& ab : prepare_animation(cfg, "movement_anim")) {
		config anim = ab.merge();
		anim["apply_to"] = "movement";

		if(anim["offset"].empty()) {
			anim["offset"] = "0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,";
		}

		if(anim["layer"].empty()) {
			anim["layer"] = move_layer;
		}

		animations.emplace_back(anim);
	}

	add_simple_anim(animations, cfg, "post_movement_anim", "post_movement", drawing_layer::unit_move_default);

	for(const animation_branch& ab : prepare_animation(cfg, "defend")) {
		config anim = ab.merge();
		anim["apply_to"] = "defend";

		if(anim["layer"].empty()) {
			anim["layer"] = default_layer;
		}

		if(!anim["damage"].empty() && anim["value"].empty()) {
			anim["value"] = anim["damage"];
		}

		if(anim["hits"].empty()) {
			anim["hits"] = false;
			animations.emplace_back(anim);
			animations.back().base_score_--; //so default doesn't interfere with 'if' block

			anim["hits"] = true;
			animations.emplace_back(anim);
			animations.back().base_score_--;

			image::locator image_loc = animations.back().get_last_frame().end_parameters().image;
			animations.back().add_frame(225ms, frame_builder()
				.image(image_loc.get_filename()+image_loc.get_modifications())
				.duration(225ms)
				.blend("0.0,0.5:75,0.0:75,0.5:75,0.0", {255,0,0}));
		} else {
			for(const std::string& hit_type : utils::split(anim["hits"])) {
				config tmp = anim;
				tmp["hits"] = hit_type;

				animations.emplace_back(tmp);

				image::locator image_loc = animations.back().get_last_frame().end_parameters().image;
				if(hit_type == "yes" || hit_type == strike_result::hit || hit_type == strike_result::kill) {
					animations.back().add_frame(225ms, frame_builder()
						.image(image_loc.get_filename() + image_loc.get_modifications())
						.duration(225ms)
						.blend("0.0,0.5:75,0.0:75,0.5:75,0.0", {255,0,0}));
				}
			}
		}
	}

	add_simple_anim(animations, cfg, "draw_weapon_anim", "draw_weapon", drawing_layer::unit_move_default);
	add_simple_anim(animations, cfg, "sheath_weapon_anim", "sheath_weapon", drawing_layer::unit_move_default);

	for(const animation_branch& ab : prepare_animation(cfg, "attack_anim")) {
		config anim = ab.merge();
		anim["apply_to"] = "attack";

		if(anim["layer"].empty()) {
			anim["layer"] = move_layer;
		}

		config::const_child_itors missile_fs = anim.child_range("missile_frame");
		if(anim["offset"].empty() && missile_fs.empty()) {
			anim["offset"] ="0~0.6,0.6~0";
		}

		if(!missile_fs.empty()) {
			if(anim["missile_offset"].empty()) {
				anim["missile_offset"] = "0~0.8";
			}

			if(anim["missile_layer"].empty()) {
				anim["missile_layer"] = missile_layer;
			}

			config tmp;
			tmp["duration"] = 1;

			anim.add_child("missile_frame", tmp);
			anim.add_child_at("missile_frame", tmp, 0);
		}

		animations.emplace_back(anim);
	}

	for(const animation_branch& ab : prepare_animation(cfg, "death")) {
		config anim = ab.merge();
		anim["apply_to"] = "death";

		if(anim["layer"].empty()) {
			anim["layer"] = default_layer;
		}

		animations.emplace_back(anim);
		image::locator image_loc = animations.back().get_last_frame().end_parameters().image;

		animations.back().add_frame(600ms, frame_builder()
			.image(image_loc.get_filename()+image_loc.get_modifications())
			.duration(600ms)
			.highlight("1~0:600"));

		if(!cfg["die_sound"].empty()) {
			animations.back().sub_anims_["_death_sound"] = particle();
			animations.back().sub_anims_["_death_sound"].add_frame(1ms,frame_builder().sound(cfg["die_sound"]),true);
		}
	}

	add_simple_anim(animations, cfg, "victory_anim", "victory");

	for(const animation_branch& ab : prepare_animation(cfg, "extra_anim")) {
		config anim = ab.merge();
		anim["apply_to"] = anim["flag"];

		if(anim["layer"].empty()) {
			anim["layer"] = default_layer;
		}

		animations.emplace_back(anim);
	}

	for(const animation_branch& ab : prepare_animation(cfg, "teleport_anim")) {
		config anim = ab.merge();
		if(anim["layer"].empty()) {
			anim["layer"] = default_layer;
		}

		anim["apply_to"] = "pre_teleport";
		animations.emplace_back(anim);
		animations.back().unit_anim_.set_end_time(0ms);

		anim["apply_to"] ="post_teleport";
		animations.emplace_back(anim);
		animations.back().unit_anim_.remove_frames_until(0ms);
	}
}

void unit_animation::particle::override(const std::chrono::milliseconds& start_time
	, const std::chrono::milliseconds& duration
	, const cycle_state cycles
	, const std::string& highlight
	, const std::string& blend_ratio
	, color_t blend_color
	, const std::string& offset
	, const std::string& layer
	, const std::string& modifiers)
{
	set_begin_time(start_time);
	parameters_.override(duration,highlight,blend_ratio,blend_color,offset,layer,modifiers);

	if(cycles == CYCLE) {
		cycles_=true;
	} else if(cycles==NO_CYCLE) {
		cycles_=false;
	}

	if(get_animation_duration() < duration) {
		add_frame(duration -get_animation_duration(), get_last_frame());
	} else if(get_animation_duration() > duration) {
		set_end_time(duration);
	}
}

bool unit_animation::particle::need_update() const
{
	if(animated<unit_frame>::need_update()) return true;
	if(get_current_frame().need_update()) return true;
	if(parameters_.need_update()) return true;
	return false;
}

bool unit_animation::particle::need_minimal_update() const
{
	return get_current_frame_begin_time() != last_frame_begin_time_;
}

unit_animation::particle::particle(const config& cfg, const std::string& frame_string)
	: animated<unit_frame>()
	, accelerate(true)
	, parameters_()
	, halo_id_()
	, last_frame_begin_time_(0)
	, cycles_(false)
{
	starting_frame_time_ = std::chrono::milliseconds::max();

	config::const_child_itors range = cfg.child_range(frame_string + "frame");
	if(!range.empty() && cfg[frame_string + "start_time"].empty()) {
		for(const config& frame : range) {
			starting_frame_time_ = std::min(starting_frame_time_, chrono::parse_duration<std::chrono::milliseconds>(frame["begin"]));
		}
	} else {
		starting_frame_time_ = chrono::parse_duration<std::chrono::milliseconds>(cfg[frame_string + "start_time"]);
	}

	for(const config& frame : range) {
		unit_frame tmp_frame(frame);
		add_frame(tmp_frame.duration(), tmp_frame, !tmp_frame.does_not_change());
	}

	cycles_  = cfg[frame_string + "cycles"].to_bool(false);
	parameters_ = frame_parsed_parameters(frame_builder(cfg,frame_string), get_animation_duration());

	if(!parameters_.does_not_change()) {
		force_change();
	}
}

bool unit_animation::need_update() const
{
	if(unit_anim_.need_update()) return true;
	for(const auto& anim : sub_anims_) {
		if(anim.second.need_update()) return true;
	}

	return false;
}

bool unit_animation::need_minimal_update() const
{
	if(!play_offscreen_) {
		return false;
	}

	if(unit_anim_.need_minimal_update()) return true;

	for(const auto& anim : sub_anims_) {
		if(anim.second.need_minimal_update()) return true;
	}

	return false;
}

bool unit_animation::animation_finished() const
{
	if(!unit_anim_.animation_finished()) return false;
	for(const auto& anim : sub_anims_) {
		if(!anim.second.animation_finished()) return false;
	}

	return true;
}

bool unit_animation::animation_finished_potential() const
{
	if(!unit_anim_.animation_finished_potential()) return false;
	for(const auto& anim : sub_anims_) {
		if(!anim.second.animation_finished_potential()) return false;
	}

	return true;
}

void unit_animation::update_last_draw_time()
{
	double acceleration = unit_anim_.accelerate ? display::get_singleton()->turbo_speed() : 1.0;
	unit_anim_.update_last_draw_time(acceleration);
	for(auto& anim : sub_anims_) {
		anim.second.update_last_draw_time(acceleration);
	}
}

std::chrono::milliseconds unit_animation::get_end_time() const
{
	auto result = unit_anim_.get_end_time();
	for(const auto& anim : sub_anims_) {
		result = std::max(result, anim.second.get_end_time());
	}

	return result;
}

std::chrono::milliseconds unit_animation::get_begin_time() const
{
	auto result = unit_anim_.get_begin_time();
	for(const auto& anim : sub_anims_) {
		result = std::min(result, anim.second.get_begin_time());
	}

	return result;
}
//mark start_animation_a after testing, facing seems can only be impl by actually change the facing status of unit before and after animating.
//in other words: target only affect how the pool of chosen animations generated will be (before bug fix, this even don't work neither),
//don't actually have anything to do with unit facing according to related impl codes.
//in testing, first functionality has been proved to work.
//so the next step is the second: finding out how to impl facing change with acceptable side effects.

void unit_animation::start_animation(const std::chrono::milliseconds& start_time
	, const map_location& src
	, const map_location& dst
	, const std::string& text
	, const color_t text_color
	, const bool accelerate)
{
	if(need_process_) {
		unit_anim_.accelerate = accelerate;
		LOG_LUA << "now dst in start_anim is " << dst_;
	} else {
		unit_anim_.accelerate = accelerate;
		src_ = src;
		dst_ = dst;
	}

	unit_anim_.start_animation(start_time);

	if(!text.empty()) {
		particle crude_build;
		crude_build.add_frame(1ms, frame_builder());
		crude_build.add_frame(1ms, frame_builder().text(text, text_color), true);
		sub_anims_["_add_text"] = crude_build;
	}

	for(auto& anim : sub_anims_) {
		anim.second.accelerate = accelerate;
		anim.second.start_animation(start_time);
	}
}

void unit_animation::update_parameters(const map_location& src, const map_location& dst)
{
	src_ = src;
	dst_ = dst;
}

void unit_animation::update_needproc(bool need_process)
{
	need_process_ = need_process;
}

bool unit_animation::need_process() const
{
	return need_process_;
}

void unit_animation::pause_animation()
{
	unit_anim_.pause_animation();

	for(auto& anim : sub_anims_) {
		anim.second.pause_animation();
	}
}

void unit_animation::restart_animation()
{
	unit_anim_.restart_animation();

	for(auto& anim : sub_anims_) {
		anim.second.restart_animation();
	}
}

void unit_animation::redraw(frame_parameters& value, halo::manager& halo_man)
{
	invalidated_ = false;
	overlaped_hex_.clear();

	value.primary_frame = true;
	unit_anim_.redraw(value,src_,dst_, halo_man);

	value.primary_frame = false;
	for(auto& anim : sub_anims_) {
		anim.second.redraw(value, src_, dst_, halo_man);
	}
}

void unit_animation::clear_haloes()
{
	unit_anim_.clear_halo();

	for(auto& anim : sub_anims_) {
		anim.second.clear_halo();
	}
}

bool unit_animation::invalidate(frame_parameters& value)
{
	if(invalidated_) return false;

	display* disp = display::get_singleton();
	const bool complete_redraw = disp->tile_nearly_on_screen(src_) || disp->tile_nearly_on_screen(dst_);

	if(overlaped_hex_.empty()) {
		if(complete_redraw) {
			value.primary_frame = true;
			overlaped_hex_ = unit_anim_.get_overlaped_hex(value, src_, dst_);
			value.primary_frame = false;

			for(auto& anim : sub_anims_) {
				std::set<map_location> tmp = anim.second.get_overlaped_hex(value, src_, dst_);
				overlaped_hex_.insert(tmp.begin(), tmp.end());
			}
		} else {
			// Offscreen animations only invalidate their own hex, no propagation,
			// but we still need this to play sounds
			overlaped_hex_.insert(src_);
		}
	}

	if(complete_redraw) {
		if( need_update()) {
			disp->invalidate(overlaped_hex_);
			invalidated_ = true;
			return true;
		} else {
			invalidated_ = disp->propagate_invalidation(overlaped_hex_);
			return invalidated_;
		}
	} else {
		if(need_minimal_update()) {
			disp->invalidate(overlaped_hex_);
			invalidated_ = true;
			return true;
		} else {
			return false;
		}
	}
}

std::string unit_animation::debug() const
{
	std::ostringstream outstream;
	outstream << *this;
	return outstream.str();
}

std::ostream& operator<<(std::ostream& outstream, const unit_animation& u_animation)
{
	std::string events_string = utils::join(u_animation.event_);
	outstream << "[" << events_string << "]\n";

	outstream << "\tstart_time=" << u_animation.get_begin_time().count() << '\n';

	if(u_animation.hits_.size() > 0) {
		std::vector<std::string> hits;
		std::transform(u_animation.hits_.begin(), u_animation.hits_.end(), std::back_inserter(hits), strike_result::get_string);
		outstream << "\thits=" << utils::join(hits) << '\n';
	}

	if(u_animation.directions_.size() > 0) {
		std::vector<std::string> dirs;
		std::transform(u_animation.directions_.begin(), u_animation.directions_.end(), std::back_inserter(dirs), map_location::write_direction);
		outstream << "\tdirections=" << utils::join(dirs) << '\n';
	}

	if(u_animation.terrain_types_.size() > 0) {
		outstream << "\tterrain=" << utils::join(u_animation.terrain_types_) << '\n';
	}

	if(u_animation.frequency_ > 0) outstream << "frequency=" << u_animation.frequency_ << '\n';

	if(u_animation.unit_filter_.size() > 0) {
		outstream << "[filter]\n";
		for(const config& cfg : u_animation.unit_filter_) {
			outstream << cfg.debug();
		}

		outstream << "[/filter]\n";
	}

	if(u_animation.secondary_unit_filter_.size() > 0) {
		outstream << "[filter_second]\n";
		for(const config& cfg : u_animation.secondary_unit_filter_) {
			outstream << cfg.debug();
		}

		outstream << "[/filter_second]\n";
	}

	if(u_animation.primary_attack_filter_.size() > 0) {
		outstream << "[filter_attack]\n";
		for(const config& cfg : u_animation.primary_attack_filter_) {
			outstream << cfg.debug();
		}

		outstream << "[/filter_attack]\n";
	}

	if(u_animation.secondary_attack_filter_.size() > 0) {
		outstream << "[filter_second_attack]\n";
		for(const config& cfg : u_animation.secondary_attack_filter_) {
			outstream << cfg.debug();
		}

		outstream << "[/filter_second_attack]\n";
	}

	for(std::size_t i = 0; i < u_animation.unit_anim_.get_frames_count(); i++) {
		outstream << "\t[frame]\n";
		for(const std::string& frame_string : u_animation.unit_anim_.get_frame(i).debug_strings()) {
			outstream << "\t\t" << frame_string <<"\n";
		}
		outstream << "\t[/frame]\n";
	}

	for(std::pair<std::string, unit_animation::particle> p : u_animation.sub_anims_) {
		for(std::size_t i = 0; i < p.second.get_frames_count(); i++) {
			std::string sub_frame_name = p.first;
			std::size_t pos = sub_frame_name.find("_frame");
			if(pos != std::string::npos) sub_frame_name = sub_frame_name.substr(0, pos);

			outstream << "\t" << sub_frame_name << "_start_time=" << p.second.get_begin_time().count() << '\n';
			outstream << "\t[" << p.first << "]\n";

			for(const std::string& frame_string : p.second.get_frame(i).debug_strings()) {
				outstream << "\t\t" << frame_string << '\n';
			}

			outstream << "\t[/" << p.first << "]\n";
		}
	}

	outstream << "[/" << events_string << "]\n";
	return outstream;
}

void unit_animation::particle::redraw(const frame_parameters& value,const map_location& src, const map_location& dst, halo::manager& halo_man)
{
	const unit_frame& current_frame = get_current_frame();
	const auto animation_time = get_animation_time();
	const frame_parameters default_val = parameters_.parameters(animation_time - get_begin_time());

	// Everything is relative to the first frame in an attack/defense/etc. block.
	// so we need to check if this particular frame is due to be shown at this time
	bool in_scope_of_frame = (animation_time >= get_current_frame_begin_time() ? true: false);
	if(animation_time > get_current_frame_end_time()) in_scope_of_frame = false;

	// Sometimes even if the frame is not due to be shown, a frame image still must be shown.
	// i.e. in a defense animation that is shorter than an attack animation.
	// the halos should not persist though and use the 'in_scope_of_frame' variable.

	// For sound frames we want the first time variable set only after the frame has started.
	if(get_current_frame_begin_time() != last_frame_begin_time_ && animation_time >= get_current_frame_begin_time()) {
		last_frame_begin_time_ = get_current_frame_begin_time();
		current_frame.redraw(get_current_frame_time(), true, in_scope_of_frame, src, dst, halo_id_, halo_man, default_val, value);
	} else {
		current_frame.redraw(get_current_frame_time(), false, in_scope_of_frame, src, dst, halo_id_, halo_man, default_val, value);
	}
}

void unit_animation::particle::clear_halo()
{
	halo_id_.reset();
}

std::set<map_location> unit_animation::particle::get_overlaped_hex(const frame_parameters& value, const map_location& src, const map_location& dst)
{
	const unit_frame& current_frame = get_current_frame();
	const frame_parameters default_val = parameters_.parameters(get_animation_time() - get_begin_time());
	return current_frame.get_overlaped_hex(get_current_frame_time(), src, dst, default_val,value);
}

unit_animation::particle::~particle()
{
	halo_id_.reset();
}

void unit_animation::particle::start_animation(const std::chrono::milliseconds& start_time)
{
	halo_id_.reset();
	parameters_.override(get_animation_duration());
	animated<unit_frame>::start_animation(start_time,cycles_);
	last_frame_begin_time_ = get_begin_time() - 1ms;
}
//mark add_anim
//need to implement functionality of 2 or more animations running at the same time, for #9163
void unit_animator::add_animation(unit_const_ptr animated_unit
		, const std::string& event
		, const map_location &src
		, const map_location &dst
		, const int value
		, bool with_bars
		, const std::string& text
		, const color_t text_color
		, const strike_result::type hit_type
		, const_attack_ptr attack
		, const_attack_ptr second_attack
		, int value2
		, bool need_process
		, unit_ptr move_unit_p
		, bool use_lockstep
		, bool coherence)
{
	if(!animated_unit) return;

	// when dst is invalid, it shouldn't be processed whatsoever.
	// need_process actually means the need for updating unit facing and destination before animation. Might change name of this param later.
	// movement animation with move_unit_p defined don't actually use the animation generated. It's the later called wml function which uses generated animation.
	// so in this case nullptr is enough.
	const unit_animation* anim = move_unit_p ? nullptr
											 : animated_unit->anim_comp().choose_animation(src, event, dst, value,
												   hit_type, attack, second_attack, value2, need_process);
	if(anim) {
		start_time_ = std::max(start_time_, anim->get_begin_time());
	} else if(!move_unit_p)
		return;

	auto&& au_rvalue = std::move(animated_unit);
	if(need_process) {
		if(move_unit_p != nullptr) {
			// Process movement animation
			LOG_LUA << "Processing movement animation, dst = " << dst;
			if(!animated_units_.empty() && animated_units_[animated_units_.size() - 1].is_movement) {
				if(!use_lockstep) {
					last_movement_serino_++;
				}
				LOG_LUA << "	serial number is " << last_movement_serino_;
				animated_units_.AGGREGATE_EMPLACE(au_rvalue, anim, text, text_color, src, with_bars,
					au_rvalue->facing(), dst, src.get_relative_dir(dst), move_unit_p, true, last_movement_serino_, coherence);
			} else {
				last_movement_serino_ = 0;
				animated_units_.AGGREGATE_EMPLACE(au_rvalue, anim, text, text_color, src, with_bars,
					au_rvalue->facing(), dst, src.get_relative_dir(dst), move_unit_p, true, last_movement_serino_, coherence);
			}
		} else {
			// Process non-movement animation
			LOG_LUA << "Processing non-movement animation, dst = " << dst;
			animated_units_.AGGREGATE_EMPLACE(
				au_rvalue, anim, text, text_color, src, with_bars, au_rvalue->facing(), dst, src.get_relative_dir(dst));
		}
		/* facing shouldn't be changed before animation runs. It should be changed right after run() is called, by
		sequence. should remove these after main stuff proved to work.
		au_rvalue->set_facing(src.get_relative_dir(dst));
		display* disp = display::get_singleton();
		disp->invalidate(src);
		disp->update();
		LOG_LUA << "facing set to " << src.get_relative_dir(dst) << " expected, and actually is "
				 au_rvalue->facing();*/
	} else{
		animated_units_.AGGREGATE_EMPLACE(au_rvalue, anim, text, text_color, src, with_bars);
		LOG_LUA << "facing setting skipped for invalid dst or operation not from lua.";
	}
}

void unit_animator::add_animation(unit_const_ptr animated_unit
	, const unit_animation* anim
	, const map_location &src
	, bool with_bars
	, const std::string& text
	, const color_t text_color)
{
	if(!animated_unit || !anim) return;
	start_time_ = std::max(start_time_, anim->get_begin_time());
	animated_units_.AGGREGATE_EMPLACE(std::move(animated_unit), anim, text, text_color, src, with_bars);
}

// The return value indicates if the processing of movement animations should pursue.
// Notice: position of the moved unit should only be set after run(), in the last movement.
// need to implement a "sequence" and a "is_lockstep" for movement anim, which once latter is false, automatically
// increases former and run animation queued by sequence.
// this function should be implemented for serving as a more convinient option than using [move_unit_fake] and [move_units_fake].
// mark move_unit_fake_queue
bool unit_animator::move_unit_fake(int& index_movement_anim)
{
	LOG_LUA << "Running move_unit_fake";
	const std::string tag = "move_unit_fake";
	game_events::wml_action::map m = game_events::wml_action::registry();// m[tag] is call wml func by tag.
	game_display* screen_p = game_display::get_singleton();
	display* disp = display::get_singleton();
	bool result;
	bool coherent_moving = false;
	map_location u_initial_loc;
	while (index_movement_anim < animated_units_.size() && animated_units_[index_movement_anim].is_movement)
	{
		if(index_movement_anim + 1 < animated_units_.size()
			&& animated_units_[index_movement_anim].serial_no == animated_units_[index_movement_anim + 1].serial_no) {
			return true;
		}
		LOG_LUA << "serial_no in muf:" << animated_units_[index_movement_anim].serial_no;
		auto& anim = animated_units_[index_movement_anim];
		auto* last_anim_p = index_movement_anim > 0 ? &animated_units_[index_movement_anim - 1] : nullptr;
		// only when start location and animated unit are the same with what in last animation, should the next movement animation run in coherence.
		coherent_moving = last_anim_p && anim.coherence && last_anim_p->is_movement
			&& last_anim_p->move_up == anim.move_up && last_anim_p->src == anim.src;
		unit_ptr& m_up = anim.move_up;
		unit& m_u = *m_up;
		map_location& m_src = coherent_moving ? last_anim_p->move_dst : anim.src;
		map_location& m_dst = anim.move_dst;
		// when animation is run, loc of the real unit could be different from m_src.
		//use coherent_moving to indicate whether the location of real unit should be temporarily set to destination of last movement animation.
		u_initial_loc = m_u.get_location();
		m_u.set_hidden(true);
		// temporarily changes the unit location to make space.
		if (coherent_moving){
			m_u.set_location(last_anim_p->move_dst);
		} else {
			m_u.set_location(map_location::null_location());
		}
		disp->invalidate(u_initial_loc);
		m_dst = find_vacant_tile(m_dst, pathfind::VACANT_ANY, m_up.get());
		config cfg;
		cfg["type"] = m_u.type().id();
		cfg["gender"] = m_u.gender();
		cfg["variation"] = m_u.variation();
		cfg["side"] = m_u.side();
		cfg["x"] = std::to_string(m_src.wml_x()) + "," + std::to_string(m_dst.wml_x());
		cfg["y"] = std::to_string(m_src.wml_y()) + "," + std::to_string(m_dst.wml_y());
		cfg["force_scroll"] = true;
		vconfig vcfg = vconfig(cfg);
		LOG_LUA << "Now vcfg has an y of " << vcfg["y"] << " and cfg has an y of " << cfg["y"];
		LOG_LUA << "Now m_src.wml_y() is " << m_src.wml_y() << " and m_dst.y is " << m_dst.wml_y() << " and added is "
				<< std::to_string(m_src.wml_y()) + "," + std::to_string(m_dst.wml_y());
		LOG_LUA << "Now vcfg has an x of " << vcfg["x"] << " and cfg has an x of " << cfg["x"];
		LOG_LUA << "Now m_src.wml_x() is " << m_src.wml_x() << " and m_dst.x is " << m_dst.wml_x() << " and added is "
				<< std::to_string(m_src.wml_x()) + "," + std::to_string(m_dst.wml_x());
		LOG_LUA << "src = " << m_src << " and dst = " << m_dst;
		m[tag]({"","",map_location(),map_location(),config()},vcfg);
		result = screen_p->maybe_rebuild();
		if (!result){
			screen_p->invalidate_all();
		}
		m_u.set_location(u_initial_loc);
		//m_u.set_hidden(false);
		//disp->invalidate(u_initial_loc);
		index_movement_anim++;
	}
	disp->update();
	return index_movement_anim < animated_units_.size() && animated_units_[index_movement_anim].is_movement;
}
// The return value indicates if the processing of movement animations should pursue.
// from animated_units_[index] to animated_units_[index+size-1], make sure these elements are all movements with the same sequence.
// coherent moving here: unit a : A->B->C, b : B->C->D, want an convinient way to make it run in lockstep just by input like this:
// animator: (moving a from A to B, uselockstep = true), animator: (a,B->C,true)...animator: (b,C->D,true), don't need to worry about
// correcting locations of these two units and such, they will be the same as before these fake moves, should be much more convinient.
// however, it's also expected to work when manulally corrected locations of animated units.
// the issue that units disappear after this function runned comes out because they are not assigned to the last valid location respectively.
// their location don't need to be revert iteratively, and where they are actually also don't matter to the animation after the configs have been generated.
// so just remember the last valid location of units respectively, and reset them before or after animation, should solve the issue.
//mark move_units_fake
bool unit_animator::move_units_fake(int& index, const int& size)
{
	if(size < 1) {
		if(index < animated_units_.size()) {
			return animated_units_[index].is_movement;
		} else {
			return false;
		}
	}
	const int start = index;
	int temp_index = index;
	LOG_LUA << "Running move_units_fake";
	const std::string tag = "move_units_fake";
	game_events::wml_action::map m = game_events::wml_action::registry(); // m[tag] is call wml func by tag.
	game_display* screen_p = game_display::get_singleton();
	display* disp = display::get_singleton();
	bool result;
	bool coherent_moving = false;
	// use vector to temporarily store data needed to judge if the animation should run in coherence.
	// map<pair src, move_up>
	// tree : processed_anim_queues -> up -> src. first iterate over animated_units_ to fill the 2D processed_anim_queues, then
	// iterate over it to produce real src, i.e. m_src,
	// and create vconfig to create children to config in the iteration. after that, call wml.
	std::vector<std::vector<std::reference_wrapper<anim_elem>>> processed_anim_queues;

	while(temp_index < animated_units_.size() && temp_index < start + size) {
		auto& anim = animated_units_[temp_index];
		// either create a new vector when fail to match existing unit, or append to last matched unit vector.
		bool new_queue = true;
		for(auto iter = processed_anim_queues.begin(); iter != processed_anim_queues.cend(); iter++) {
			auto& sorted_anim = (*iter)[0].get();
			if(sorted_anim.move_up == anim.move_up) {
				(*iter).emplace_back(std::ref(anim));
				new_queue = false;
				break;
			}
		}
		if(new_queue) {
			processed_anim_queues.emplace_back(std::vector<std::reference_wrapper<anim_elem>>{std::ref(anim)});
		}
		temp_index++;
	}
	anim_elem* last_anim_p = nullptr;
	std::vector<std::vector<config>> uconfig_queues;
	std::vector<std::pair<std::reference_wrapper<unit_ptr>, map_location>> initial_uloc_pairs;
	// generate configs judged if the moving can be coherent
	for(auto& vec : processed_anim_queues) {
		std::vector<config> judged_uconfigs;
		for(auto iter = vec.begin(); iter != vec.cend(); iter++) {
			auto& anim = (*iter).get();
			coherent_moving
				= last_anim_p && anim.coherence && last_anim_p->move_up == anim.move_up && last_anim_p->src == anim.src;
			LOG_LUA << "Now coherent has an value of " << coherent_moving;
			unit_ptr& m_up = anim.move_up;
			unit& m_u = *m_up;
			map_location& m_src = coherent_moving ? last_anim_p->move_dst : anim.src;
			map_location& m_dst = anim.move_dst;

			bool pair_found = false;
			for (auto& elem : initial_uloc_pairs) {
				if (elem.first.get() == m_up) {
					pair_found = true;
					break;
				}
			}
			if (!pair_found) {
				initial_uloc_pairs.emplace_back(std::ref(m_up), m_u.get_location());
			}

			m_u.set_hidden(true);
			if(coherent_moving) {
				m_u.set_location(last_anim_p->move_dst);
			} else {
				m_u.set_location(map_location::null_location());
			}
			disp->invalidate(m_u.get_location());
			m_dst = find_vacant_tile(m_dst, pathfind::VACANT_ANY, m_up.get());
			config cfg;
			cfg["type"] = m_u.type().id();
			cfg["gender"] = m_u.gender();
			cfg["variation"] = m_u.variation();
			cfg["side"] = m_u.side();
			cfg["x"] = std::to_string(m_src.wml_x()) + "," + std::to_string(m_dst.wml_x());
			cfg["y"] = std::to_string(m_src.wml_y()) + "," + std::to_string(m_dst.wml_y());
			LOG_LUA << "Now cfg has an y of " << cfg["y"];
			LOG_LUA << "Now m_src.wml_y() is " << m_src.wml_y() << " and m_dst.y is " << m_dst.wml_y()
					<< " and added is " << std::to_string(m_src.wml_y()) + "," + std::to_string(m_dst.wml_y());
			LOG_LUA << "Now cfg has an x of " << cfg["x"];
			LOG_LUA << "Now m_src.wml_x() is " << m_src.wml_x() << " and m_dst.x is " << m_dst.wml_x()
					<< " and added is " << std::to_string(m_src.wml_x()) + "," + std::to_string(m_dst.wml_x());
			LOG_LUA << "src = " << m_src << " and dst = " << m_dst;
			judged_uconfigs.emplace_back(cfg);
			last_anim_p = &anim;
		}
		uconfig_queues.emplace_back(judged_uconfigs);
	}
	// reset locations so that when the animated units are reverted from hidden, they will show up on screen normally.
	for (auto iter = initial_uloc_pairs.begin(); iter != initial_uloc_pairs.cend(); iter++) {
		auto& uloc = *(iter);
		unit& m_u = *(uloc.first.get());
		m_u.set_location(uloc.second);
		m_u.anim_comp().set_standing();
	}
	// generate the needed fake_units by extracting one from each unit's config sequence
	while(!uconfig_queues.empty()) {
		config cfg;
		for(auto index = 0; index < uconfig_queues.size(); index++) {
			auto& vec = uconfig_queues[index];
			if(vec.empty()) {
				uconfig_queues.erase(uconfig_queues.begin() + index);
				index--;
				continue;
			}
			cfg.add_child("fake_unit", vec[0]);
			vec.erase(vec.begin());
		}
		cfg["force_scroll"] = true;
		vconfig vcfg = vconfig(cfg);
		m[tag]({"", "", map_location(), map_location(), config()}, vcfg);
		result = screen_p->maybe_rebuild();
		if(!result) {
			screen_p->invalidate_all();
		}
	}
	index += size;
	return index < animated_units_.size() && animated_units_[index].is_movement;
}

//mark mufq
void unit_animator::move_units_fake_queue(int& index_movement_anim)
{
	LOG_LUA << "Running move_units_fake_queue";
	// check which tag to use.
	// this is where the check happen, and also check for coherent moving, make 2 branches
	int simu_size;
	while ((move_unit_fake(index_movement_anim)))
	{
		// Indicates how many animations are actually expected to run simultaneously. There will at least be two here.
		simu_size = 2;
		const int& serino = animated_units_[index_movement_anim].serial_no;
		int serino_tocompare = animated_units_[index_movement_anim + simu_size].serial_no;
		while(serino == serino_tocompare) {
			LOG_LUA << "move_units_fake_queue found a match for serino_tocompare = " << serino_tocompare;
			simu_size++;
			serino_tocompare = animated_units_[index_movement_anim + simu_size].serial_no;
			LOG_LUA << "simu_size = " << simu_size;
		}
		if(!move_units_fake(index_movement_anim, simu_size)) {
			break;
		}
	}
}

bool unit_animator::has_animation(unit_const_ptr animated_unit
		, const std::string& event
		, const map_location &src
		, const map_location &dst
		, const int value
		, const strike_result::type hit_type
		, const_attack_ptr attack
		, const_attack_ptr second_attack
		, int value2) const
{
	return (animated_unit && animated_unit->anim_comp().choose_animation(src, event, dst, value, hit_type, attack, second_attack, value2));
}
//mark replace anim
void unit_animator::replace_anim_if_invalid(unit_const_ptr animated_unit
	, const std::string& event
	, const map_location &src
	, const map_location & dst
	, const int value
	, bool with_bars
	, const std::string& text
	, const color_t text_color
	, const strike_result::type hit_type
	, const_attack_ptr attack
	, const_attack_ptr second_attack
	, int value2)
{
	if(!animated_unit) return;

	if(animated_unit->anim_comp().get_animation() &&
		!animated_unit->anim_comp().get_animation()->animation_finished_potential() &&
		 animated_unit->anim_comp().get_animation()->matches(
			src, dst, animated_unit, event, value, hit_type, attack, second_attack, value2) > unit_animation::MATCH_FAIL)
	{
		animated_units_.AGGREGATE_EMPLACE(animated_unit, nullptr, text, text_color, src, with_bars);
	} else {
		if (event == "movement")
			LOG_LUA << "add_anim called by replace_anim";
		add_animation(animated_unit,event,src,dst,value,with_bars,text,text_color,hit_type,attack,second_attack,value2);
	}
}
//mark start_anims call start_anim_ac
//mark issue now: if movement and non-movement animations all operate at the same unit, non-movement animations will be override.
void unit_animator::start_animations()
{
	auto begin_time = std::chrono::milliseconds::max();

	for(const auto& anim : animated_units_) {
		if(!anim.is_movement && anim.my_unit->anim_comp().get_animation()) {
			if(anim.animation) {
				begin_time = std::min(begin_time, anim.animation->get_begin_time());
			} else  {
				begin_time = std::min(begin_time, anim.my_unit->anim_comp().get_animation()->get_begin_time());
			}
		}
	}
	int index = 0;
	while(index < animated_units_.size()) {
		unit_animator::anim_elem& anim = animated_units_[index];
		if(anim.is_movement) {
			wait_for_end();
			move_units_fake_queue(index);
			continue;
		}
		LOG_LUA << "Running non-movement animation";
		if(anim.animation) {
			LOG_LUA << "now dst in start_anims if is " << anim.animation->get_dst();
			LOG_LUA << "and facing is " << anim.src.get_direction(anim.my_unit->facing());
			// temporarily changing the animated unit's facing to match expected animation.
			if (anim.changed_facing != map_location::direction::indeterminate){
				anim.my_unit->set_facing(anim.changed_facing);
				display* disp = display::get_singleton();
				disp->invalidate(anim.my_unit->get_location());
				disp->update();
				LOG_LUA << "facing in start_anim set to " << anim.changed_facing << " expected, and actually is "
				<< anim.my_unit->facing();
			}

			anim.my_unit->anim_comp().start_animation(
				begin_time, anim.animation, anim.with_bars, anim.text, anim.text_color);
			// mark anim set to nptr this is why crash
			anim.animation = nullptr;
		} else {
			LOG_LUA << "now dst in start_anims else is null";
			LOG_LUA << "and facing is " << anim.src.get_direction(anim.my_unit->facing());
			anim.my_unit->anim_comp().get_animation()->update_parameters(
				anim.src, anim.src.get_direction(anim.my_unit->facing()));
		}
		index++;
	}
}
//mark revert
void unit_animator::revert_facing()
{
	display* disp = display::get_singleton();
	for(auto& anim : animated_units_) {
		//mark crash
		anim.my_unit->set_hidden(false);
		if(anim.original_facing != map_location::direction::indeterminate) {
			LOG_LUA << "unit facing reset from " << anim.my_unit->facing();
			anim.my_unit->set_facing(anim.original_facing);
			LOG_LUA << " to " << anim.my_unit->facing();
			//disp->invalidate(anim.my_unit->get_location()); there's a chance that the unit's position is changed now, so this won't work properly
		}
	}
	disp->invalidate_all();
	disp->update();
}

// returns null location if no movement animation of the given unit is found.
// according to celmin: `movement` as a function name need this as an arg. if don't needed, then `movement` shouldn't be part of it.
// so this name needs change
const map_location& unit_animator::get_unit_last_move_anim_dst(unit_const_ptr ucp) const
{
	if (animated_units_.empty()) {
		return map_location::null_location();
	}
	for (auto iter = animated_units_.end() - 1; iter >= animated_units_.begin(); iter--) {
		auto& anim = *iter;
		//mark might go wrong if the equation of const unit ptr don't hold for same units.
		if (anim.is_movement && ucp == anim.my_unit) {
			return anim.move_dst;
		}
		if (iter == animated_units_.begin()) {
			break;
		}
	}
	return map_location::null_location();
}

bool unit_animator::would_end() const
{
	bool finished = true;
	for(const auto& anim : animated_units_) {
		finished &= anim.my_unit->anim_comp().get_animation()->animation_finished_potential();
	}

	return finished;
}

void unit_animator::wait_until(const std::chrono::milliseconds& animation_time) const
{
	if(animated_units_.empty()) {
		return;
	}
	// important to set a max animation time so that the time does not go past this value for movements.
	// fix for bug #1565
	animated_units_[0].my_unit->anim_comp().get_animation()->set_max_animation_time(animation_time);

	display* disp = display::get_singleton();
	double speed = disp->turbo_speed();

	resources::controller->play_slice();

	using std::chrono::steady_clock;
	auto end_tick = animated_units_[0].my_unit->anim_comp().get_animation()->time_to_tick(animation_time);

	while(steady_clock::now() < end_tick - std::min(std::chrono::floor<std::chrono::milliseconds>(20ms / speed), 20ms)) {
		auto rest = std::chrono::floor<std::chrono::milliseconds>((animation_time - get_animation_time()) * speed);
		std::this_thread::sleep_for(std::clamp(rest, 0ms, 10ms));

		resources::controller->play_slice();
		end_tick = animated_units_[0].my_unit->anim_comp().get_animation()->time_to_tick(animation_time);
	}

	auto rest = std::max<steady_clock::duration>(0ms, end_tick - steady_clock::now() + 5ms);
	std::this_thread::sleep_for(rest);

	new_animation_frame();
	animated_units_[0].my_unit->anim_comp().get_animation()->set_max_animation_time(0ms);
}

void unit_animator::wait_for_end() const
{
	bool finished = false;
	while(!finished) {
		resources::controller->play_slice();

		std::this_thread::sleep_for(10ms);

		finished = true;
		for(const auto& anim : animated_units_) {
			finished &= anim.my_unit->anim_comp().get_animation()->animation_finished_potential();
		}
	}
}

std::chrono::milliseconds unit_animator::get_animation_time() const
{
	if(animated_units_.empty()) {
		return 0ms;
	}
	return animated_units_[0].my_unit->anim_comp().get_animation()->get_animation_time() ;
}

std::chrono::milliseconds unit_animator::get_animation_time_potential() const
{
	if(animated_units_.empty()) {
		return 0ms;
	}
	return animated_units_[0].my_unit->anim_comp().get_animation()->get_animation_time_potential() ;
}

std::chrono::milliseconds unit_animator::get_end_time() const
{
	auto end_time = std::chrono::milliseconds::min();
	for(const auto& anim : animated_units_) {
		if(anim.my_unit->anim_comp().get_animation()) {
			end_time = std::max(end_time, anim.my_unit->anim_comp().get_animation()->get_end_time());
		}
	}

	return end_time;
}

void unit_animator::pause_animation()
{
	for(const auto& anim : animated_units_) {
		if(anim.my_unit->anim_comp().get_animation()) {
			anim.my_unit->anim_comp().get_animation()->pause_animation();
		}
	}
}

void unit_animator::restart_animation()
{
	for(const auto& anim : animated_units_) {
		if(anim.my_unit->anim_comp().get_animation()) {
			anim.my_unit->anim_comp().get_animation()->restart_animation();
		}
	}
}

void unit_animator::set_all_standing()
{
	for(const auto& anim : animated_units_) {
		anim.my_unit->anim_comp().set_standing();
	}
}
