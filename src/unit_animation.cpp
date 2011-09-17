/* $Id$ */
/*
   Copyright (C) 2006 - 2011 by Jeremy Rosen <jeremy.rosen@enst-bretagne.fr>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
   */

#include "global.hpp"

#include "unit_animation.hpp"

#include "foreach.hpp"
#include "game_display.hpp"
#include "halo.hpp"
#include "map.hpp"
#include "unit.hpp"
#include "variable.hpp"
#include "resources.hpp"
#include "play_controller.hpp"

#include <algorithm>

struct tag_name_manager {
	tag_name_manager() : names() {
	static const config::t_token z_animation("animation", false);
	static const config::t_token z_attack_anim("attack_anim", false);
	static const config::t_token z_death("death", false);
	static const config::t_token z_defend("defend", false);
	static const config::t_token z_extra_anim("extra_anim", false);
	static const config::t_token z_healed_anim("healed_anim", false);
	static const config::t_token z_healing_anim("healing_anim", false);
	static const config::t_token z_idle_anim("idle_anim", false);
	static const config::t_token z_leading_anim("leading_anim", false);
	static const config::t_token z_resistance_anim("resistance_anim", false);
	static const config::t_token z_levelin_anim("levelin_anim", false);
	static const config::t_token z_levelout_anim("levelout_anim", false);
	static const config::t_token z_movement_anim("movement_anim", false);
	static const config::t_token z_poison_anim("poison_anim", false);
	static const config::t_token z_recruit_anim("recruit_anim", false);
	static const config::t_token z_recruiting_anim("recruiting_anim", false);
	static const config::t_token z_standing_anim("standing_anim", false);
	static const config::t_token z_teleport_anim("teleport_anim", false);
	static const config::t_token z_pre_movement_anim("pre_movement_anim", false);
	static const config::t_token z_post_movement_anim("post_movement_anim", false);
	static const config::t_token z_draw_weapon_anim("draw_weapon_anim", false);
	static const config::t_token z_sheath_weapon_anim("sheath_weapon_anim", false);
	static const config::t_token z_victory_anim("victory_anim", false);
	static const config::t_token z__transparent("_transparent", false);

		names.push_back(z_animation);
		names.push_back(z_attack_anim);
		names.push_back(z_death);
		names.push_back(z_defend);
		names.push_back(z_extra_anim);
		names.push_back(z_healed_anim);
		names.push_back(z_healing_anim);
		names.push_back(z_idle_anim);
		names.push_back(z_leading_anim);
		names.push_back(z_resistance_anim);
		names.push_back(z_levelin_anim);
		names.push_back(z_levelout_anim);
		names.push_back(z_movement_anim);
		names.push_back(z_poison_anim);
		names.push_back(z_recruit_anim);
		names.push_back(z_recruiting_anim);
		names.push_back(z_standing_anim);
		names.push_back(z_teleport_anim);
		names.push_back(z_pre_movement_anim);
		names.push_back(z_post_movement_anim);
		names.push_back(z_draw_weapon_anim);
		names.push_back(z_sheath_weapon_anim);
		names.push_back(z_victory_anim);
		names.push_back(z__transparent); // Used for WB
	}
	std::vector<n_token::t_token> names;
};
namespace {
	tag_name_manager anim_tags;
} //end anonymous namespace

const std::vector<n_token::t_token>& unit_animation::all_tag_names() {
	return anim_tags.names;
}

struct animation_branch
{
	animation_branch()
		: attributes()
		, children()
	{
	}

	config attributes;
	std::vector<config::all_children_iterator> children;
	config merge() const
	{
		config result = attributes;
		foreach (const config::all_children_iterator &i, children)
			result.add_child(i->key, i->cfg);
		return result;
	}
};

typedef std::list<animation_branch> animation_branches;

struct animation_cursor
{
	config::all_children_itors itors;
	animation_branches branches;
	animation_cursor *parent;
	animation_cursor(const config &cfg):
		itors(cfg.all_children_range()), branches(1), parent(NULL)
	{
		branches.back().attributes.merge_attributes(cfg);
	}
	animation_cursor(const config &cfg, animation_cursor *p):
		itors(cfg.all_children_range()), branches(p->branches), parent(p)
	{
		foreach (animation_branch &ab, branches)
			ab.attributes.merge_attributes(cfg);
	}
};

static void prepare_single_animation(const config &anim_cfg, animation_branches &expanded_anims)
{
	static const config::t_token z_if("if", false);
	static const config::t_token z_else("else", false);

	std::list<animation_cursor> anim_cursors;
	anim_cursors.push_back(animation_cursor(anim_cfg));
	while (!anim_cursors.empty())
	{
		animation_cursor &ac = anim_cursors.back();
		if (ac.itors.first == ac.itors.second) {
			if (!ac.parent) break;
			// Merge all the current branches into the parent.
			ac.parent->branches.splice(ac.parent->branches.end(),
				ac.branches, ac.branches.begin(), ac.branches.end());
			anim_cursors.pop_back();
			continue;
		}
		if (ac.itors.first->key != z_if)
		{
			// Append current config object to all the branches in scope.
			foreach (animation_branch &ab, ac.branches) {
				ab.children.push_back(ac.itors.first);
			}
			++ac.itors.first;
			continue;
		}
		int count = 0;
		do {
			/* Copies the current branches to each cursor created
			   for the conditional clauses. Merge the attributes
			   of the clause into them. */
			anim_cursors.push_back(animation_cursor(ac.itors.first->cfg, &ac));
			++ac.itors.first;
			++count;
		} while (ac.itors.first != ac.itors.second && ac.itors.first->key == z_else);
		if (count > 1) {
			/* There are some z_else clauses, discard the branches
			   from the current cursor. */
			ac.branches.clear();
		}
	}

	// Create the config object describing each branch.
	assert(anim_cursors.size() == 1);
	animation_cursor &ac = anim_cursors.back();
	expanded_anims.splice(expanded_anims.end(),
		ac.branches, ac.branches.begin(), ac.branches.end());
}

static animation_branches prepare_animation(const config &cfg, const n_token::t_token &animation_tag)
{
	animation_branches expanded_animations;
	foreach (const config &anim, cfg.child_range(animation_tag)) {
		prepare_single_animation(anim, expanded_animations);
	}
	return expanded_animations;
}

unit_animation::unit_animation(int start_time,
	const unit_frame & frame, const n_token::t_token& event, const int variation, const frame_builder & builder) :
		terrain_types_(),
		unit_filter_(),
		secondary_unit_filter_(),
		directions_(),
		frequency_(0),
		base_score_(variation),
		event_(utils::split_token(event)),
		value_(),
		primary_attack_filter_(),
		secondary_attack_filter_(),
		hits_(),
		value2_(),
		sub_anims_(),
		unit_anim_(start_time,builder),
		src_(),
		dst_(),
		invalidated_(false),
		play_offscreen_(true),
		overlaped_hex_()
{
	add_frame(frame.duration(),frame,!frame.does_not_change());
}
namespace{
	DEFAULT_TOKEN_BODY(z_terrain_type_default, "terrain_type")
	DEFAULT_TOKEN_BODY(z_frequency_default, "frequency")
	DEFAULT_TOKEN_BODY(z_base_score_default, "base_score")
}
unit_animation::unit_animation(const config& cfg,const n_token::t_token& frame_string ) :
	terrain_types_(t_translation::read_list(cfg[z_terrain_type_default()])),
	unit_filter_(),
	secondary_unit_filter_(),
	directions_(),
	frequency_(cfg[z_frequency_default()]),
	base_score_(cfg[z_base_score_default()]),
	event_(),
	value_(),
	primary_attack_filter_(),
	secondary_attack_filter_(),
	hits_(),
	value2_(),
	sub_anims_(),
	unit_anim_(cfg,frame_string),
	src_(),
	dst_(),
	invalidated_(false),
	play_offscreen_(true),
	overlaped_hex_()
{
	static const config::t_token z_apply_to("apply_to", false);
	static const config::t_token z_direction("direction", false);
	static const config::t_token z_filter("filter", false);
	static const config::t_token z_filter_second("filter_second", false);
	static const config::t_token z_value("value", false);
	static const config::t_token z_hits("hits", false);
	static const config::t_token z_yes("yes", false);
	static const config::t_token z_hit("hit", false);
	static const config::t_token z_no("no", false);
	static const config::t_token z_miss("miss", false);
	static const config::t_token z_kill("kill", false);
	static const config::t_token z_value_second("value_second", false);
	static const config::t_token z_filter_attack("filter_attack", false);
	static const config::t_token z_filter_second_attack("filter_second_attack", false);
	static const config::t_token z_offscreen("offscreen", false);

//	if(!cfg[z_debug].empty()) printf("DEBUG WML: FINAL\n%s\n\n",cfg.debug().c_str());
	foreach (const config::any_child &fr, cfg.all_children_range())
	{
		if (fr.key == frame_string) continue;
		if ((*fr.key).find("_frame", (*fr.key).size() - 6) == std::string::npos) continue;
		if (sub_anims_.find(fr.key) != sub_anims_.end()) continue;
		sub_anims_[fr.key] = particule(cfg, n_token::t_token((*fr.key).substr(0, (*fr.key).size() - 5)));
	}
	event_ =utils::split_token(cfg[z_apply_to]);

	const std::vector<n_token::t_token>& my_directions = utils::split_token(cfg[z_direction]);
	for(std::vector<n_token::t_token>::const_iterator i = my_directions.begin(); i != my_directions.end(); ++i) {
		const map_location::DIRECTION d = map_location::parse_direction(*i);
		directions_.push_back(d);
	}
	foreach (const config &filter, cfg.child_range(z_filter)) {
		unit_filter_.push_back(filter);
	}

	foreach (const config &filter, cfg.child_range(z_filter_second)) {
		secondary_unit_filter_.push_back(filter);
	}

	std::vector<n_token::t_token> value_str = utils::split_token(cfg[z_value]);
	std::vector<n_token::t_token>::iterator value;
	for(value=value_str.begin() ; value != value_str.end() ; ++value) {
		value_.push_back(atoi(value->c_str()));
	}

	std::vector<n_token::t_token> hits_str = utils::split_token(cfg[z_hits]);
	std::vector<n_token::t_token>::iterator hit;
	for(hit=hits_str.begin() ; hit != hits_str.end() ; ++hit) {
		if(*hit == z_yes || *hit == z_hit) {
			hits_.push_back(HIT);
		}
		if(*hit == z_no || *hit == z_miss) {
			hits_.push_back(MISS);
		}
		if(*hit == z_yes || *hit == z_kill ) {
			hits_.push_back(KILL);
		}
	}
	std::vector<n_token::t_token> value2_str = utils::split_token(cfg[z_value_second]);
	std::vector<n_token::t_token>::iterator value2;
	for(value2=value2_str.begin() ; value2 != value2_str.end() ; ++value2) {
		value2_.push_back(atoi(value2->c_str()));
	}
	foreach (const config &filter, cfg.child_range(z_filter_attack)) {
		primary_attack_filter_.push_back(filter);
	}
	foreach (const config &filter, cfg.child_range(z_filter_second_attack)) {
		secondary_attack_filter_.push_back(filter);
	}
	play_offscreen_ = cfg[z_offscreen].to_bool(true);

}

int unit_animation::matches(const game_display &disp,const map_location& loc,const map_location& second_loc, const unit* my_unit,const n_token::t_token & event,const int value,hit_type hit,const attack_type* attack,const attack_type* second_attack, int value2) const
{
	int result = base_score_;
	if(!event.empty()&&!event_.empty()) {
		if (std::find(event_.begin(),event_.end(),event)== event_.end()) {
			return MATCH_FAIL;
		} else {
			result ++;
		}
	}
	if(terrain_types_.empty() == false) {
		if(t_translation::terrain_matches(disp.get_map().get_terrain(loc), terrain_types_)) {
			result ++;
		} else {
			return MATCH_FAIL;
		}
	}

	if(value_.empty() == false ) {
		if (std::find(value_.begin(),value_.end(),value)== value_.end()) {
			return MATCH_FAIL;
		} else {
			result ++;
		}
	}
	if(my_unit) {
		if(directions_.empty()== false) {
			if (std::find(directions_.begin(),directions_.end(),my_unit->facing())== directions_.end()) {
				return MATCH_FAIL;
			} else {
				result ++;
			}
		}
		std::vector<config>::const_iterator myitor;
		for(myitor = unit_filter_.begin(); myitor != unit_filter_.end(); ++myitor) {
			if (!my_unit->matches_filter(vconfig(*myitor), loc)) return MATCH_FAIL;
			++result;
		}
		if(!secondary_unit_filter_.empty()) {
			unit_map::const_iterator unit;
			for(unit=disp.get_const_units().begin() ; unit != disp.get_const_units().end() ; ++unit) {
				if (unit->get_location() == second_loc) {
					std::vector<config>::const_iterator second_itor;
					for(second_itor = secondary_unit_filter_.begin(); second_itor != secondary_unit_filter_.end(); ++second_itor) {
						if (!unit->matches_filter(vconfig(*second_itor), second_loc)) return MATCH_FAIL;
						result++;
					}

					break;
				}
			}
			if(unit == disp.get_const_units().end()) return MATCH_FAIL;
		}

	} else if (!unit_filter_.empty()) return MATCH_FAIL;
	if(frequency_ && !(rand()%frequency_)) return MATCH_FAIL;


	if(hits_.empty() == false ) {
		if (std::find(hits_.begin(),hits_.end(),hit)== hits_.end()) {
			return MATCH_FAIL;
		} else {
			result ++;
		}
	}
	if(value2_.empty() == false ) {
		if (std::find(value2_.begin(),value2_.end(),value2)== value2_.end()) {
			return MATCH_FAIL;
		} else {
			result ++;
		}
	}
	if(!attack) {
		if(!primary_attack_filter_.empty())
			return MATCH_FAIL;
	}
	std::vector<config>::const_iterator myitor;
	for(myitor = primary_attack_filter_.begin(); myitor != primary_attack_filter_.end(); ++myitor) {
		if(!attack->matches_filter(*myitor)) return MATCH_FAIL;
		result++;
	}
	if(!second_attack) {
		if(!secondary_attack_filter_.empty())
			return MATCH_FAIL;
	}
	for(myitor = secondary_attack_filter_.begin(); myitor != secondary_attack_filter_.end(); ++myitor) {
		if(!second_attack->matches_filter(*myitor)) return MATCH_FAIL;
		result++;
	}
	return result;

}


void unit_animation::fill_initial_animations( std::vector<unit_animation> & animations, const config & cfg)
{
static const config::t_token z_GS("~GS()", false);
static const config::t_token z_local_heal_wav("heal.wav", false);
static const config::t_token z_local_pois_sound("poison.ogg", false);
	static const config::t_token z_image("image", false);
	static const config::t_token z_default("default", false);
	static const config::t_token z__disabled_("_disabled_", false);
	static const config::t_token z__disabled_selected_("_disabled_selected_", false);
	static const config::t_token z_standing("standing", false);
	static const config::t_token z_ghosted("ghosted", false);
	static const config::t_token z_disabled_ghosted("disabled_ghosted", false);
	static const config::t_token z_selected("selected", false);
	static const config::t_token z_recruited("recruited", false);
	static const config::t_token z_levelin("levelin", false);
	static const config::t_token z_levelout("levelout", false);
	static const config::t_token z_pre_movement("pre_movement", false);
	static const config::t_token z_post_movement("post_movement", false);
	static const config::t_token z_movement("movement", false);
	static const config::t_token z_defend("defend", false);
	static const config::t_token z_attack("attack", false);
	static const config::t_token z_range("range", false);
	static const config::t_token z_melee("melee", false);
	static const config::t_token z_ranged("ranged", false);
	static const config::t_token z_death("death", false);
	static const config::t_token z__death_sound("_death_sound", false);
	static const config::t_token z_die_sound("die_sound", false);
	static const config::t_token z_victory("victory", false);
	static const config::t_token z_pre_teleport("pre_teleport", false);
	static const config::t_token z_post_teleport("post_teleport", false);
	static const config::t_token z_healed("healed", false);
	static const config::t_token z__healed_sound("_healed_sound", false);
	static const config::t_token z_poisoned("poisoned", false);
	static const config::t_token z__poison_sound("_poison_sound", false);

	const image::locator default_image = image::locator(cfg[z_image].token());
	std::vector<unit_animation>  animation_base;
	std::vector<unit_animation>::const_iterator itor;
	add_anims(animations,cfg);
	for(itor = animations.begin(); itor != animations.end() ; ++itor) {
		if (std::find(itor->event_.begin(),itor->event_.end(),z_default)!= itor->event_.end()) {
			animation_base.push_back(*itor);
			animation_base.back().base_score_ += unit_animation::DEFAULT_ANIM;
			animation_base.back().event_.clear();
		}
	}

	if( animation_base.empty() )
		animation_base.push_back(unit_animation(0,frame_builder().image(default_image).duration(1),n_token::t_token::z_empty(),unit_animation::DEFAULT_ANIM));

	animations.push_back(unit_animation(0,frame_builder().image(default_image).duration(1),z__disabled_,0));
	{
		static const config::t_token z_local_blend("0.0~0.3:100,0.3~0.0:200", false);
		animations.push_back(unit_animation(0,frame_builder().image(default_image).duration(300).
											blend(z_local_blend,display::rgb(255,255,255)),z__disabled_selected_,0));
	}
	for(itor = animation_base.begin() ; itor != animation_base.end() ; ++itor ) {
		//unit_animation tmp_anim = *itor;
		// provide all default anims
		//no event, providing a catch all anim
		//animations.push_back(tmp_anim);

		animations.push_back(*itor);
		animations.back().event_ = utils::split_token(z_standing);
		animations.back().play_offscreen_ = false;

		animations.push_back(*itor);
		animations.back().event_ = utils::split_token(z_ghosted);

		static const config::t_token z_local_0p9("0.9", false);
		animations.back().unit_anim_.override(0,animations.back().unit_anim_.get_animation_duration(),particule::UNSET,z_local_0p9,n_token::t_token::z_empty(),0,n_token::t_token::z_empty(),n_token::t_token::z_empty(), z_GS);

		animations.push_back(*itor);
		animations.back().event_ = utils::split_token(z_disabled_ghosted);
		static const config::t_token z_local_0p4("0.4", false);
		animations.back().unit_anim_.override(0,1,particule::UNSET,z_local_0p4 ,n_token::t_token::z_empty(),0,n_token::t_token::z_empty(),n_token::t_token::z_empty(),z_GS);

		animations.push_back(*itor);
		animations.back().event_ = utils::split_token(z_selected);
		static const config::t_token z_local_t1("0.0~0.3:100,0.3~0.0:200", false);
		animations.back().unit_anim_.override(0,300,particule::UNSET,n_token::t_token::z_empty(),z_local_t1,display::rgb(255,255,255));

		animations.push_back(*itor);
		animations.back().event_ = utils::split_token(z_recruited);
		static const config::t_token z_local_0_1_600("0~1:600", false);
		animations.back().unit_anim_.override(0,600,particule::NO_CYCLE, z_local_0_1_600);

		animations.push_back(*itor);
		animations.back().event_ = utils::split_token(z_levelin);
		animations.back().unit_anim_.override(0,600,particule::NO_CYCLE,n_token::t_token::z_empty(),z_local_0_1_600,display::rgb(255,255,255));

		animations.push_back(*itor);
		animations.back().event_ = utils::split_token(z_levelout);
		static const config::t_token z_local_0_1_600_1("0~1:600,1", false);
		animations.back().unit_anim_.override(0,600,particule::NO_CYCLE,n_token::t_token::z_empty(), z_local_0_1_600_1,display::rgb(255,255,255));

		animations.push_back(*itor);
		animations.back().event_ = utils::split_token(z_pre_movement);

		animations.push_back(*itor);
		animations.back().event_ = utils::split_token(z_post_movement);

		animations.push_back(*itor);
		animations.back().event_ = utils::split_token(z_movement);
		static const config::t_token z_local_try_and_parse_me("0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,", false);
		static const n_token::t_token z_local_move_layer(lexical_cast<std::string>(display::LAYER_UNIT_MOVE_DEFAULT-display::LAYER_UNIT_FIRST), false);
		animations.back().unit_anim_.override(0,6800,particule::NO_CYCLE,n_token::t_token::z_empty(),n_token::t_token::z_empty(),0,z_local_try_and_parse_me , z_local_move_layer);

		animations.push_back(*itor);
		animations.back().event_ = utils::split_token(z_defend);
		static const config::t_token z_local_t2("0.0,0.5:75,0.0:75,0.5:75,0.0", false);
		animations.back().unit_anim_.override(0,animations.back().unit_anim_.get_animation_duration(),particule::NO_CYCLE,n_token::t_token::z_empty(), z_local_t2,game_display::rgb(255,0,0));
		animations.back().hits_.push_back(HIT);
		animations.back().hits_.push_back(KILL);

		animations.push_back(*itor);
		animations.back().event_ = utils::split_token(z_defend);

		animations.push_back(*itor);
		animations.back().event_ = utils::split_token(z_attack);
		static const config::t_token z_local_t4("0~0.6:150,0.6~0:150", false);
		animations.back().unit_anim_.override(-150,300,particule::NO_CYCLE,n_token::t_token::z_empty(),n_token::t_token::z_empty(),0, z_local_t4,z_local_move_layer);
		animations.back().primary_attack_filter_.push_back(config());
		animations.back().primary_attack_filter_.back()[z_range] = z_melee;

		animations.push_back(*itor);
		animations.back().event_ = utils::split_token(z_attack);
		animations.back().unit_anim_.override(-150,150,particule::NO_CYCLE);
		animations.back().primary_attack_filter_.push_back(config());
		animations.back().primary_attack_filter_.back()[z_range] = z_ranged;

		animations.push_back(*itor);
		animations.back().event_ = utils::split_token(z_death);
		static const config::t_token z_local_1_0_600("1~0:600", false);
		animations.back().unit_anim_.override(0,600,particule::NO_CYCLE, z_local_1_0_600);
		animations.back().sub_anims_[z__death_sound] = particule();
		animations.back().sub_anims_[z__death_sound].add_frame(1,frame_builder());
		animations.back().sub_anims_[z__death_sound].add_frame(1,frame_builder().sound(cfg[z_die_sound]),true);

		animations.push_back(*itor);
		animations.back().event_ = utils::split_token(z_victory);

		animations.push_back(*itor);
		static const config::t_token z_local_pretele("1~0:150", false);
		animations.back().unit_anim_.override(0,150,particule::NO_CYCLE,z_local_pretele);
		animations.back().event_ = utils::split_token(z_pre_teleport);

		animations.push_back(*itor);
		static const config::t_token z_local_posttele("0~1:150,1", false);
		animations.back().unit_anim_.override(0,150,particule::NO_CYCLE, z_local_posttele);
		animations.back().event_ = utils::split_token(z_post_teleport);

		animations.push_back(*itor);
		animations.back().event_ = utils::split_token(z_healed);
		static const config::t_token z_local_healed("0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30", false);
		animations.back().unit_anim_.override(0,300,particule::NO_CYCLE,n_token::t_token::z_empty(),z_local_healed,display::rgb(255,255,255));
		animations.back().sub_anims_[z__healed_sound] = particule();
		animations.back().sub_anims_[z__healed_sound].add_frame(1,frame_builder());
		animations.back().sub_anims_[z__healed_sound].add_frame(1,frame_builder().sound(z_local_heal_wav),true);

		animations.push_back(*itor);
		animations.back().event_ = utils::split_token(z_poisoned);
		static const config::t_token z_local_pois("0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30", false);
		animations.back().unit_anim_.override(0,300,particule::NO_CYCLE,n_token::t_token::z_empty(), z_local_pois,display::rgb(0,255,0));
		animations.back().sub_anims_[z__poison_sound] = particule();
		animations.back().sub_anims_[z__poison_sound].add_frame(1,frame_builder());
		animations.back().sub_anims_[z__poison_sound].add_frame(1,frame_builder().sound(z_local_pois_sound),true);
	}

}

static void add_simple_anim(std::vector<unit_animation> &animations, const config &cfg
							, n_token::t_token const & tag_name, n_token::t_token const & apply_to,
	display::tdrawing_layer layer = display::LAYER_UNIT_DEFAULT,
	bool offscreen = true)
{
	static const config::t_token z_apply_to("apply_to", false);
	static const config::t_token z_offscreen("offscreen", false);
	static const config::t_token z_layer("layer", false);

	foreach (const animation_branch &ab, prepare_animation(cfg, tag_name))
	{
		config anim = ab.merge();
		anim[z_apply_to] = apply_to;
		if (!offscreen) {
			config::attribute_value &v = anim[z_offscreen];
			if (v.empty()) v = false;
		}
		config::attribute_value &v = anim[z_layer];
		if (v.empty()) v = layer - display::LAYER_UNIT_FIRST;
		animations.push_back(unit_animation(anim));
	}
}

void unit_animation::add_anims( std::vector<unit_animation> & animations, const config & cfg) {
	static const config::t_token z_animation("animation", false);
	static const config::t_token z_resistance_anim("resistance_anim", false);
	static const config::t_token z_resistance("resistance", false);
	static const config::t_token z_leading_anim("leading_anim", false);
	static const config::t_token z_leading("leading", false);
	static const config::t_token z_recruit_anim("recruit_anim", false);
	static const config::t_token z_recruited("recruited", false);
	static const config::t_token z_recruiting_anim("recruiting_anim", false);
	static const config::t_token z_recruiting("recruiting", false);
	static const config::t_token z_idle_anim("idle_anim", false);
	static const config::t_token z_idling("idling", false);
	static const config::t_token z_levelin_anim("levelin_anim", false);
	static const config::t_token z_levelin("levelin", false);
	static const config::t_token z_levelout_anim("levelout_anim", false);
	static const config::t_token z_levelout("levelout", false);
	static const config::t_token z_standing_anim("standing_anim", false);
	static const config::t_token z_apply_to("apply_to", false);
	static const config::t_token z_standing_c_default("standing,default", false);
	static const config::t_token z_cycles("cycles", false);
	static const config::t_token z_true("true", false);
	static const config::t_token z_layer("layer", false);
	static const config::t_token z_offscreen("offscreen", false);
	static const config::t_token z_healing_anim("healing_anim", false);
	static const config::t_token z_healing("healing", false);
	static const config::t_token z_value("value", false);
	static const config::t_token z_healed_anim("healed_anim", false);
	static const config::t_token z_healed("healed", false);
	static const config::t_token z__healed_sound("_healed_sound", false);
	static const config::t_token z_local_heal_wav("local_heal_wav", false);
	static const config::t_token z_poison_anim("poison_anim", false);
	static const config::t_token z_poisoned("poisoned", false);
	static const config::t_token z_damage("damage", false);
	static const config::t_token z__poison_sound("_poison_sound", false);
	static const config::t_token z_local_pois_sound("local_pois_sound", false);
	static const config::t_token z_pre_movement_anim("pre_movement_anim", false);
	static const config::t_token z_pre_movement("pre_movement", false);
	static const config::t_token z_movement_anim("movement_anim", false);
	static const config::t_token z_offset("offset", false);
	static const config::t_token z_movement("movement", false);
	static const config::t_token z_post_movement_anim("post_movement_anim", false);
	static const config::t_token z_post_movement("post_movement", false);
	static const config::t_token z_defend("defend", false);
	static const config::t_token z_hits("hits", false);
	static const config::t_token z_yes("yes", false);
	static const config::t_token z_hit("hit", false);
	static const config::t_token z_kill("kill", false);
	static const config::t_token z_draw_weapon_anim("draw_weapon_anim", false);
	static const config::t_token z_draw_weapon("draw_weapon", false);
	static const config::t_token z_sheath_weapon_anim("sheath_weapon_anim", false);
	static const config::t_token z_sheath_weapon("sheath_weapon", false);
	static const config::t_token z_attack_anim("attack_anim", false);
	static const config::t_token z_attack("attack", false);
	static const config::t_token z_missile_frame("missile_frame", false);
	static const config::t_token z_missile_offset("missile_offset", false);
	static const config::t_token z_missile_layer("missile_layer", false);
	static const config::t_token z_duration("duration", false);
	static const config::t_token z_death("death", false);
	static const config::t_token z_die_sound("die_sound", false);
	static const config::t_token z__death_sound("_death_sound", false);
	static const config::t_token z_victory_anim("victory_anim", false);
	static const config::t_token z_victory("victory", false);
	static const config::t_token z_extra_anim("extra_anim", false);
	static const config::t_token z_flag("flag", false);
	static const config::t_token z_teleport_anim("teleport_anim", false);
	static const config::t_token z_pre_teleport("pre_teleport", false);
	static const config::t_token z_post_teleport("post_teleport", false);

	foreach (const animation_branch &ab, prepare_animation(cfg, z_animation)) {
		animations.push_back(unit_animation(ab.merge())); }

	const int default_layer = display::LAYER_UNIT_DEFAULT - display::LAYER_UNIT_FIRST;
	const int move_layer = display::LAYER_UNIT_MOVE_DEFAULT - display::LAYER_UNIT_FIRST;
	const int missile_layer = display::LAYER_UNIT_MISSILE_DEFAULT - display::LAYER_UNIT_FIRST;

	add_simple_anim(animations, cfg, z_resistance_anim, z_resistance);
	add_simple_anim(animations, cfg, z_leading_anim, z_leading);
	add_simple_anim(animations, cfg, z_recruit_anim, z_recruited);
	add_simple_anim(animations, cfg, z_recruiting_anim, z_recruiting);
	add_simple_anim(animations, cfg, z_idle_anim, z_idling, display::LAYER_UNIT_DEFAULT, false);
	add_simple_anim(animations, cfg, z_levelin_anim, z_levelin);
	add_simple_anim(animations, cfg, z_levelout_anim, z_levelout);

	foreach (const animation_branch &ab, prepare_animation(cfg, z_standing_anim))
	{
		config anim = ab.merge();
		anim[z_apply_to] = z_standing_c_default;
		anim[z_cycles] = z_true;
		if (anim[z_layer].empty()) anim[z_layer] = default_layer;
		if (anim[z_offscreen].empty()) anim[z_offscreen] = false;
		animations.push_back(unit_animation(anim));
	}
	foreach (const animation_branch &ab, prepare_animation(cfg, z_healing_anim))
	{
		config anim = ab.merge();
		anim[z_apply_to] = z_healing;
		if (anim[z_layer].empty()) anim[z_layer] = default_layer;
		anim[z_value] = anim[z_damage];
		animations.push_back(unit_animation(anim));
	}

	foreach (const animation_branch &ab, prepare_animation(cfg, z_healed_anim))
	{
		config anim = ab.merge();
		anim[z_apply_to] = z_healed;
		if (anim[z_layer].empty()) anim[z_layer] = default_layer;
		anim[z_value] = anim[z_healing];
		animations.push_back(unit_animation(anim));
		animations.back().sub_anims_[z__healed_sound] = particule();
		animations.back().sub_anims_[z__healed_sound].add_frame(1,frame_builder());
		animations.back().sub_anims_[z__healed_sound].add_frame(1,frame_builder().sound(z_local_heal_wav),true);
	}

	foreach (const animation_branch &ab, prepare_animation(cfg, z_poison_anim))
	{
		config anim = ab.merge();
		anim[z_apply_to] =z_poisoned;
		if (anim[z_layer].empty()) anim[z_layer] = default_layer;
		anim[z_value] = anim[z_damage];
		animations.push_back(unit_animation(anim));
		animations.back().sub_anims_[z__poison_sound] = particule();
		animations.back().sub_anims_[z__poison_sound].add_frame(1,frame_builder());
		animations.back().sub_anims_[z__poison_sound].add_frame(1,frame_builder().sound(z_local_pois_sound),true);
	}

	add_simple_anim(animations, cfg, z_pre_movement_anim, z_pre_movement, display::LAYER_UNIT_MOVE_DEFAULT);

	foreach (const animation_branch &ab, prepare_animation(cfg, z_movement_anim))
	{
		config anim = ab.merge();
		if (anim[z_offset].empty()) {
			anim[z_offset] = "0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,";
		}
		anim[z_apply_to] = z_movement;
		if (anim[z_layer].empty()) anim[z_layer] = move_layer;
		animations.push_back(unit_animation(anim));
	}

	add_simple_anim(animations, cfg, z_post_movement_anim, z_post_movement, display::LAYER_UNIT_MOVE_DEFAULT);

	foreach (const animation_branch &ab, prepare_animation(cfg, z_defend))
	{
		config anim = ab.merge();
		anim[z_apply_to] = z_defend;
		if (anim[z_layer].empty()) anim[z_layer] = default_layer;
		if (!anim[z_damage].empty() && anim[z_value].empty()) {
			anim[z_value] = anim[z_damage];
		}
		if (anim[z_hits].empty())
		{
			anim[z_hits] = false;
			animations.push_back(unit_animation(anim));
			anim[z_hits] = true;
			animations.push_back(unit_animation(anim));
			static const config::t_token z_local_blend("0.0,0.5:75,0.0:75,0.5:75,0.0", false);
			animations.back().add_frame(225,frame_builder()
					.image(animations.back().get_last_frame().parameters(0).image)
					.duration(225)
					.blend(z_local_blend,game_display::rgb(255,0,0)));
		}
		else
		{
			std::vector<n_token::t_token> v = utils::split_token(anim[z_hits]);
			foreach (const n_token::t_token &hit_type, v)
			{
				config tmp = anim;
				tmp[z_hits] = hit_type;
				animations.push_back(unit_animation(tmp));
				if(hit_type == z_yes || hit_type == z_hit || hit_type==z_kill) {
					static const config::t_token z_local_blend("0.0,0.5:75,0.0:75,0.5:75,0.0", false);
					animations.back().add_frame(225,frame_builder()
							.image(animations.back().get_last_frame().parameters(0).image)
							.duration(225)
							.blend(z_local_blend,game_display::rgb(255,0,0)));
				}
			}
		}
	}

	add_simple_anim(animations, cfg, z_draw_weapon_anim, z_draw_weapon, display::LAYER_UNIT_MOVE_DEFAULT);
	add_simple_anim(animations, cfg, z_sheath_weapon_anim, z_sheath_weapon, display::LAYER_UNIT_MOVE_DEFAULT);

	foreach (const animation_branch &ab, prepare_animation(cfg, z_attack_anim))
	{
		config anim = ab.merge();
		anim[z_apply_to] = z_attack;
		if (anim[z_layer].empty()) anim[z_layer] = move_layer;
		config::const_child_itors missile_fs = anim.child_range(z_missile_frame);
		static const config::t_token z_local_missile_t1("0~0.6,0.6~0", false);
		if (anim[z_offset].empty() && missile_fs.first == missile_fs.second) {
			anim[z_offset] = z_local_missile_t1;
		}
		if (missile_fs.first != missile_fs.second) {
			static const config::t_token z_local_missile_t2("0~0.8", false);
			if (anim[z_missile_offset].empty()) anim[z_missile_offset] = z_local_missile_t2;
			if (anim[z_missile_layer].empty()) anim[z_missile_layer] = missile_layer;
			config tmp;
			tmp[z_duration] = 1;
			anim.add_child(z_missile_frame, tmp);
			anim.add_child_at(z_missile_frame, tmp, 0);
		}

		animations.push_back(unit_animation(anim));
	}

	foreach (const animation_branch &ab, prepare_animation(cfg, z_death))
	{
		config anim = ab.merge();
		anim[z_apply_to] = z_death;
		if (anim[z_layer].empty()) anim[z_layer] = default_layer;
		animations.push_back(unit_animation(anim));
		image::locator image_loc = animations.back().get_last_frame().parameters(0).image;
		static const config::t_token z_local_1_0_600("1~0:600", false);
		animations.back().add_frame(600,frame_builder().image(image_loc).duration(600).highlight(z_local_1_0_600));
		if(!cfg[z_die_sound].empty()) {
			animations.back().sub_anims_[z__death_sound] = particule();
			animations.back().sub_anims_[z__death_sound].add_frame(1,frame_builder());
			animations.back().sub_anims_[z__death_sound].add_frame(1,frame_builder().sound(cfg[z_die_sound]),true);
		}
	}

	add_simple_anim(animations, cfg, z_victory_anim, z_victory);

	foreach (const animation_branch &ab, prepare_animation(cfg, z_extra_anim))
	{
		config anim = ab.merge();
		anim[z_apply_to] = anim[z_flag];
		if (anim[z_layer].empty()) anim[z_layer] = default_layer;
		animations.push_back(unit_animation(anim));
	}

	foreach (const animation_branch &ab, prepare_animation(cfg, z_teleport_anim))
	{
		config anim = ab.merge();
		if (anim[z_layer].empty()) anim[z_layer] = default_layer;
		anim[z_apply_to] = z_pre_teleport;
		animations.push_back(unit_animation(anim));
		animations.back().unit_anim_.set_end_time(0);
		anim[z_apply_to] =z_post_teleport;
		animations.push_back(unit_animation(anim));
		animations.back().unit_anim_.remove_frames_until(0);
	}
}

void unit_animation::particule::override(int start_time
		, int duration
		, const cycle_state cycles
		, const n_token::t_token& highlight
		, const n_token::t_token& blend_ratio
		, Uint32 blend_color
		, const n_token::t_token& offset
		, const n_token::t_token& layer
		, const n_token::t_token& modifiers)
{
	set_begin_time(start_time);

	frame_parsed_parameters fp(*parameters_);
	fp.override(duration,highlight,blend_ratio,blend_color,offset,layer,modifiers);
	parameters_ = t_frame_parameter_token(fp);

	if(cycles == CYCLE) {
		cycles_=true;

	} else if(cycles==NO_CYCLE) {
		cycles_=false;
	}else {
		//nothing
	}
	if(get_animation_duration() < duration) {
		const unit_frame & last_frame = get_last_frame();
		add_frame(duration -get_animation_duration(), last_frame);
	} else if(get_animation_duration() > duration) {
		set_end_time(duration);
	}

}

bool unit_animation::particule::need_update() const
{
	if(animated<unit_frame>::need_update()) return true;
	if(get_current_frame().need_update()) return true;
	if(parameters_->need_update()) return true;
	return false;
}

bool unit_animation::particule::need_minimal_update() const
{
	if(get_current_frame_begin_time() != last_frame_begin_time_ ) {
		return true;
	}
	return false;
}

unit_animation::particule::particule(
	const config& cfg, const n_token::t_token& frame_string ) :
		animated<unit_frame>(),
		accelerate(true),
		parameters_(),
		halo_id_(0),
		last_frame_begin_time_(0),
		cycles_(false)
{
	static const config::t_token z_frame("frame", false);
	static const config::t_token z_start_time("start_time", false);
	static const config::t_token z_begin("begin", false);
	static const config::t_token z_cycles("cycles", false);


	config::const_child_itors range = cfg.child_range(frame_string+z_frame);
	starting_frame_time_=INT_MAX;
	if(cfg[frame_string + z_start_time].empty() &&range.first != range.second) {
		foreach (const config &frame, range) {
			starting_frame_time_ = std::min(starting_frame_time_, frame[z_begin].to_int());
		}
	} else {
		starting_frame_time_ = cfg[frame_string+z_start_time];
	}

	foreach (const config &frame, range)
	{
		unit_frame tmp_frame(frame);
		add_frame(tmp_frame.duration(),tmp_frame,!tmp_frame.does_not_change());
	}
	cycles_  = cfg[frame_string+z_cycles].to_bool(false);
	parameters_ = t_frame_parameter_token(frame_parsed_parameters(frame_builder(cfg,frame_string),get_animation_duration()) );
	if(!parameters_->does_not_change()  ) {
			force_change();
	}
}

bool unit_animation::need_update() const
{
	if(unit_anim_.need_update()) return true;
	std::map<n_token::t_token,particule>::const_iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
		if(anim_itor->second.need_update()) return true;
	}
	return false;
}

bool unit_animation::need_minimal_update() const
{
	if(!play_offscreen_) {
		return false;
	}
	if(unit_anim_.need_minimal_update()) return true;
	std::map<n_token::t_token,particule>::const_iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
		if(anim_itor->second.need_minimal_update()) return true;
	}
	return false;
}

bool unit_animation::animation_finished() const
{
	if(!unit_anim_.animation_finished()) return false;
	std::map<n_token::t_token,particule>::const_iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
		if(!anim_itor->second.animation_finished()) return false;
	}
	return true;
}

bool unit_animation::animation_finished_potential() const
{
	if(!unit_anim_.animation_finished_potential()) return false;
	std::map<n_token::t_token,particule>::const_iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
		if(!anim_itor->second.animation_finished_potential()) return false;
	}
	return true;
}

void unit_animation::update_last_draw_time()
{
	double acceleration = unit_anim_.accelerate ? game_display::get_singleton()->turbo_speed() : 1.0;
	unit_anim_.update_last_draw_time(acceleration);
	std::map<n_token::t_token,particule>::iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
		anim_itor->second.update_last_draw_time(acceleration);
	}
}

int unit_animation::get_end_time() const
{
	int result = unit_anim_.get_end_time();
	std::map<n_token::t_token,particule>::const_iterator anim_itor =sub_anims_.end();
	for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
		result= std::max<int>(result,anim_itor->second.get_end_time());
	}
	return result;
}

int unit_animation::get_begin_time() const
{
	int result = unit_anim_.get_begin_time();
	std::map<n_token::t_token,particule>::const_iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
		result= std::min<int>(result,anim_itor->second.get_begin_time());
	}
	return result;
}

void unit_animation::start_animation(int start_time
		, const map_location &src
		, const map_location &dst
		, const n_token::t_token& text
		, const Uint32 text_color
		, const bool accelerate)
{
	unit_anim_.accelerate = accelerate;
	src_ = src;
	dst_ = dst;
	unit_anim_.start_animation(start_time);
	if(!text.empty()) {
	static const config::t_token z__add_text("_add_text", false);

		particule crude_build;
		crude_build.add_frame(1,frame_builder());
		crude_build.add_frame(1,frame_builder().text(text,text_color),true);
		sub_anims_[z__add_text] = crude_build;
	}
	std::map<n_token::t_token,particule>::iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
		anim_itor->second.accelerate = accelerate;
		anim_itor->second.start_animation(start_time);
	}
}

void unit_animation::update_parameters(const map_location &src, const map_location &dst)
{
	src_ = src;
	dst_ = dst;
}
void unit_animation::pause_animation()
{

	std::map<n_token::t_token,particule>::iterator anim_itor =sub_anims_.begin();
	unit_anim_.pause_animation();
	for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
		anim_itor->second.pause_animation();
	}
}
void unit_animation::restart_animation()
{

	std::map<n_token::t_token,particule>::iterator anim_itor =sub_anims_.begin();
	unit_anim_.restart_animation();
	for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
		anim_itor->second.restart_animation();
	}
}
void unit_animation::redraw(frame_parameters& value)
{

	invalidated_=false;
	overlaped_hex_.clear();
	std::map<n_token::t_token,particule>::iterator anim_itor =sub_anims_.begin();
	value.primary_frame = t_true;
	unit_anim_.redraw(value,src_,dst_);
	value.primary_frame = t_false;
	for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
		anim_itor->second.redraw( value,src_,dst_);
	}
}
void unit_animation::clear_haloes()
{

	std::map<n_token::t_token,particule>::iterator anim_itor =sub_anims_.begin();
	unit_anim_.clear_halo();
	for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
		anim_itor->second.clear_halo();
	}
}
bool unit_animation::invalidate(frame_parameters& value)
{
	if(invalidated_) return false;
	game_display*disp = game_display::get_singleton();
	bool complete_redraw =disp->tile_nearly_on_screen(src_) || disp->tile_nearly_on_screen(dst_);
	if(overlaped_hex_.empty()) {
		if(complete_redraw) {
			std::map<n_token::t_token,particule>::iterator anim_itor =sub_anims_.begin();
			value.primary_frame = t_true;
			overlaped_hex_ = unit_anim_.get_overlaped_hex(value,src_,dst_);
			value.primary_frame = t_false;
			for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
				std::set<map_location> tmp = anim_itor->second.get_overlaped_hex(value,src_,dst_);
				overlaped_hex_.insert(tmp.begin(),tmp.end());
			}
		} else {
			// off screen animations only invalidate their own hex, no propagation,
			// but we stil need this to play sounds
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



void unit_animation::particule::redraw(const frame_parameters& value,const map_location &src, const map_location &dst)
{
	const unit_frame& current_frame= get_current_frame();
	const frame_parameters default_val = parameters_->parameters(get_animation_time() -get_begin_time());
	if(get_current_frame_begin_time() != last_frame_begin_time_ ) {
		last_frame_begin_time_ = get_current_frame_begin_time();
		current_frame.redraw(get_current_frame_time(),true,src,dst,&halo_id_,default_val,value);
	} else {
		current_frame.redraw(get_current_frame_time(),false,src,dst,&halo_id_,default_val,value);
	}
}
void unit_animation::particule::clear_halo()
{
	if(halo_id_ != halo::NO_HALO) {
		halo::remove(halo_id_);
		halo_id_ = halo::NO_HALO;
	}
}
std::set<map_location> unit_animation::particule::get_overlaped_hex(const frame_parameters& value,const map_location &src, const map_location &dst)
{
	const unit_frame& current_frame= get_current_frame();
	const frame_parameters default_val = parameters_->parameters(get_animation_time() -get_begin_time());
	return current_frame.get_overlaped_hex(get_current_frame_time(),src,dst,default_val,value);

}

unit_animation::particule::~particule()
{
	halo::remove(halo_id_);
	halo_id_ = halo::NO_HALO;
}

void unit_animation::particule::start_animation(int start_time)
{
	halo::remove(halo_id_);
	halo_id_ = halo::NO_HALO;
	frame_parsed_parameters fp(*parameters_);
	fp.override(get_animation_duration());
	parameters_=t_frame_parameter_token(fp);
	animated<unit_frame>::start_animation(start_time,cycles_);
	last_frame_begin_time_ = get_begin_time() -1;
}

void unit_animator::add_animation(unit* animated_unit
		, const n_token::t_token& event
		, const map_location &src
		, const map_location &dst
		, const int value
		, bool with_bars
		, const n_token::t_token& text
		, const Uint32 text_color
		, const unit_animation::hit_type hit_type
		, const attack_type* attack
		, const attack_type* second_attack
		, int value2)
{
	if(!animated_unit) return;
	anim_elem tmp;
	game_display*disp = game_display::get_singleton();
	tmp.my_unit = animated_unit;
	tmp.text = text;
	tmp.text_color = text_color;
	tmp.src = src;
	tmp.with_bars= with_bars;
	tmp.animation = animated_unit->choose_animation(*disp,src,event,dst,value,hit_type,attack,second_attack,value2);
	if(!tmp.animation) return;



	start_time_ = std::max<int>(start_time_,tmp.animation->get_begin_time());
	animated_units_.push_back(tmp);
}
void unit_animator::add_animation(unit* animated_unit
		, const unit_animation* anim
		, const map_location &src
		, bool with_bars
		, const n_token::t_token& text
		, const Uint32 text_color)
{
	if(!animated_unit) return;
	anim_elem tmp;
	tmp.my_unit = animated_unit;
	tmp.text = text;
	tmp.text_color = text_color;
	tmp.src = src;
	tmp.with_bars= with_bars;
	tmp.animation = anim;
	if(!tmp.animation) return;



	start_time_ = std::max<int>(start_time_,tmp.animation->get_begin_time());
	animated_units_.push_back(tmp);
}
void unit_animator::replace_anim_if_invalid(unit* animated_unit
		, const n_token::t_token& event
		, const map_location &src
		, const map_location & dst
		, const int value
		, bool with_bars
		, const n_token::t_token& text
		, const Uint32 text_color
		, const unit_animation::hit_type hit_type
		, const attack_type* attack
		, const attack_type* second_attack
		, int value2)
{
	if(!animated_unit) return;
	game_display*disp = game_display::get_singleton();
	if(animated_unit->get_animation() &&
			!animated_unit->get_animation()->animation_finished_potential() &&
			animated_unit->get_animation()->matches(*disp,src,dst,animated_unit,event,value,hit_type,attack,second_attack,value2) >unit_animation::MATCH_FAIL) {
		anim_elem tmp;
		tmp.my_unit = animated_unit;
		tmp.text = text;
		tmp.text_color = text_color;
		tmp.src = src;
		tmp.with_bars= with_bars;
		tmp.animation = NULL;
		animated_units_.push_back(tmp);
	}else {
		add_animation(animated_unit,event,src,dst,value,with_bars,text,text_color,hit_type,attack,second_attack,value2);
	}
}
void unit_animator::start_animations()
{
	int begin_time = INT_MAX;
	std::vector<anim_elem>::iterator anim;
	for(anim = animated_units_.begin(); anim != animated_units_.end();++anim) {
		if(anim->my_unit->get_animation()) {
			if(anim->animation) {
				begin_time = std::min<int>(begin_time,anim->animation->get_begin_time());
			} else  {
				begin_time = std::min<int>(begin_time,anim->my_unit->get_animation()->get_begin_time());
			}
		}
	}
	for(anim = animated_units_.begin(); anim != animated_units_.end();++anim) {
		if(anim->animation) {
			anim->my_unit->start_animation(begin_time, anim->animation,
				anim->with_bars,  anim->text, anim->text_color);
			anim->animation = NULL;
		} else {
			anim->my_unit->get_animation()->update_parameters(anim->src,anim->src.get_direction(anim->my_unit->facing()));
		}

	}
}

bool unit_animator::would_end() const
{
	bool finished = true;
	for(std::vector<anim_elem>::const_iterator anim = animated_units_.begin(); anim != animated_units_.end();++anim) {
		finished &= anim->my_unit->get_animation()->animation_finished_potential();
	}
	return finished;
}
void unit_animator::wait_until(int animation_time) const
{
	game_display*disp = game_display::get_singleton();
	double speed = disp->turbo_speed();
	resources::controller->play_slice(false);
	int end_tick = animated_units_[0].my_unit->get_animation()->time_to_tick(animation_time);
	while (SDL_GetTicks() < static_cast<unsigned int>(end_tick)
				- std::min<int>(static_cast<unsigned int>(20/speed),20)) {

		disp->delay(std::max<int>(0,
			std::min<int>(10,
			static_cast<int>((animation_time - get_animation_time()) * speed))));
		resources::controller->play_slice(false);
                end_tick = animated_units_[0].my_unit->get_animation()->time_to_tick(animation_time);
	}
	disp->delay(std::max<int>(0,end_tick - SDL_GetTicks() +5));
	new_animation_frame();
}
void unit_animator::wait_for_end() const
{
	if (game_config::no_delay) return;
	bool finished = false;
	game_display*disp = game_display::get_singleton();
	while(!finished) {
		resources::controller->play_slice(false);
		disp->delay(10);
		finished = true;
		for(std::vector<anim_elem>::const_iterator anim = animated_units_.begin(); anim != animated_units_.end();++anim) {
			finished &= anim->my_unit->get_animation()->animation_finished_potential();
		}
	}
}
int unit_animator::get_animation_time() const{
	return animated_units_[0].my_unit->get_animation()->get_animation_time() ;
}

int unit_animator::get_animation_time_potential() const{
	return animated_units_[0].my_unit->get_animation()->get_animation_time_potential() ;
}

int unit_animator::get_end_time() const
{
        int end_time = INT_MIN;
        for(std::vector<anim_elem>::const_iterator anim = animated_units_.begin(); anim != animated_units_.end();++anim) {
	       if(anim->my_unit->get_animation()) {
                end_time = std::max<int>(end_time,anim->my_unit->get_animation()->get_end_time());
	       }
        }
        return end_time;
}
void unit_animator::pause_animation()
{
        for(std::vector<anim_elem>::iterator anim = animated_units_.begin(); anim != animated_units_.end();++anim) {
	       if(anim->my_unit->get_animation()) {
                anim->my_unit->get_animation()->pause_animation();
	       }
        }
}
void unit_animator::restart_animation()
{
        for(std::vector<anim_elem>::iterator anim = animated_units_.begin(); anim != animated_units_.end();++anim) {
	       if(anim->my_unit->get_animation()) {
                anim->my_unit->get_animation()->restart_animation();
	       }
        }
}
void unit_animator::set_all_standing()
{
        for(std::vector<anim_elem>::iterator anim = animated_units_.begin(); anim != animated_units_.end();++anim) {
		anim->my_unit->set_standing();
        }
}
