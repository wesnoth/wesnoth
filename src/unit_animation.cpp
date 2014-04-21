/*
   Copyright (C) 2006 - 2014 by Jeremy Rosen <jeremy.rosen@enst-bretagne.fr>
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

#include "game_display.hpp"
#include "halo.hpp"
#include "map.hpp"
#include "unit.hpp"
#include "variable.hpp"
#include "resources.hpp"
#include "play_controller.hpp"

#include <boost/foreach.hpp>

#include <algorithm>

struct tag_name_manager {
	tag_name_manager() : names() {
		names.push_back("animation");
		names.push_back("attack_anim");
		names.push_back("death");
		names.push_back("defend");
		names.push_back("extra_anim");
		names.push_back("healed_anim");
		names.push_back("healing_anim");
		names.push_back("idle_anim");
		names.push_back("leading_anim");
		names.push_back("resistance_anim");
		names.push_back("levelin_anim");
		names.push_back("levelout_anim");
		names.push_back("movement_anim");
		names.push_back("poison_anim");
		names.push_back("recruit_anim");
		names.push_back("recruiting_anim");
		names.push_back("standing_anim");
		names.push_back("teleport_anim");
		names.push_back("pre_movement_anim");
		names.push_back("post_movement_anim");
		names.push_back("draw_weapon_anim");
		names.push_back("sheath_weapon_anim");
		names.push_back("victory_anim");
		names.push_back("_transparent"); // Used for WB
	}
	std::vector<std::string> names;
};
namespace {
	tag_name_manager anim_tags;
} //end anonymous namespace

const std::vector<std::string>& unit_animation::all_tag_names() {
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
		BOOST_FOREACH(const config::all_children_iterator &i, children) {
			result.add_child(i->key, i->cfg);
		}
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
		// If similar 'if' condition in parent branches, we need to
		// cull the branches where there are partial matches.
		// Hence the need to check if the condition has come up before.
		// Also, the attributes are merged here between branches.
		bool previously_hits_set = false;
		bool previously_direction_set = false;
		bool previously_terrain_set = false;
		bool previously_value_set = false;
		bool previously_value_2nd_set = false;
		std::string s_cfg_hits = cfg["hits"];
		std::string s_cfg_direction = cfg["direction"];
		std::string s_cfg_terrain = cfg["terrain_types"];
		std::string s_cfg_value = cfg["value"];
		std::string s_cfg_value_2nd = cfg["value_2nd"];
		for (std::list<animation_branch>::iterator bi = branches.begin();
			 bi != branches.end(); ++bi)
		{
			std::string s_branch_hits = (*bi).attributes["hits"];
			std::string s_branch_direction = (*bi).attributes["direction"];
			std::string s_branch_terrain = (*bi).attributes["terrain_types"];
			std::string s_branch_value = (*bi).attributes["value"];
			std::string s_branch_value_2nd = (*bi).attributes["value_second"];
			if (s_branch_hits != "" && s_branch_hits == s_cfg_hits) {
				previously_hits_set = true;
			}
			if (s_branch_direction != "" && s_branch_direction == s_cfg_direction) {
				previously_direction_set = true;
			}
			if (s_branch_terrain != "" && s_branch_terrain == s_cfg_terrain) {
				previously_terrain_set = true;
			}
			if (s_branch_value != "" && s_branch_value == s_cfg_value) {
				previously_value_set = true;
			}
			if (s_branch_value_2nd != "" && s_branch_value_2nd == s_cfg_value_2nd) {
				previously_value_2nd_set = true;
			}
		}
		// Merge all frames that have new matches and prune any impossible
		// matches, e.g. hits='yes' and hits='no'
		for (std::list<animation_branch>::iterator bi = branches.begin();
			 bi != branches.end(); /* nothing */)
		{
			std::string s_branch_hits = (*bi).attributes["hits"];
			std::string s_branch_direction = (*bi).attributes["direction"];
			std::string s_branch_terrain = (*bi).attributes["terrain_types"];
			std::string s_branch_value = (*bi).attributes["value"];
			std::string s_branch_value_2nd = (*bi).attributes["value_second"];
			bool hits_match = (previously_hits_set && s_branch_hits != s_cfg_hits);
			bool direction_match = (previously_direction_set && s_branch_direction != s_cfg_direction);
			bool terrain_match = (previously_terrain_set && s_branch_terrain != s_cfg_terrain);
			bool value_match = (previously_value_set && s_branch_value != s_cfg_value);
			bool value_2nd_match = (previously_value_2nd_set && s_branch_value_2nd != s_cfg_value_2nd);
			if ( (!previously_hits_set || hits_match) &&
			     (!previously_direction_set || direction_match) &&
			     (!previously_terrain_set || terrain_match) &&
			     (!previously_value_set || value_match) &&
			     (!previously_value_2nd_set || value_2nd_match) &&
			     (hits_match || direction_match || terrain_match || value_match || value_2nd_match) )
			{
				branches.erase(bi++);
			}
			else {
				(*bi).attributes.merge_attributes(cfg);
				++bi;
			}
		}
		// Then we prune all parent branches with similar matches as they
		// now will not have the full frame list
		for (std::list<animation_branch>::iterator bi = parent->branches.begin();
			 bi != parent->branches.end(); /* nothing */)
		{
			std::string s_branch_hits = (*bi).attributes["hits"];
			std::string s_branch_direction = (*bi).attributes["direction"];
			std::string s_branch_terrain = (*bi).attributes["terrain_types"];
			std::string s_branch_value = (*bi).attributes["value"];
			std::string s_branch_value_2nd = (*bi).attributes["value_second"];
			bool hits_match = (previously_hits_set && s_branch_hits == s_cfg_hits);
			bool direction_match = (previously_direction_set && s_branch_direction == s_cfg_direction);
			bool terrain_match = (previously_terrain_set && s_branch_terrain == s_cfg_terrain);
			bool value_match = (previously_value_set && s_branch_value == s_cfg_value);
			bool value_2nd_match = (previously_value_2nd_set && s_branch_value_2nd == s_cfg_value_2nd);
			if ( (!previously_hits_set || hits_match) &&
			     (!previously_direction_set || direction_match) &&
			     (!previously_terrain_set || terrain_match) &&
			     (!previously_value_set || value_match) &&
			     (!previously_value_2nd_set || value_2nd_match) &&
			     (hits_match || direction_match || terrain_match || value_match || value_2nd_match) )
			{
				parent->branches.erase(bi++);
			}
			else ++bi;
		}
	}
};

static void prepare_single_animation(const config &anim_cfg, animation_branches &expanded_anims)
{
	/* The anim_cursor holds the current parsing through the config and the
	   branches hold the data that will be interpreted as the actual animation.
	   The branches store the config attributes for each block and the
	   children of those branches make up all the 'frame', 'missile_frame', etc.
	   individually (so 2 instances of 'frame' would be stored as 2 children) */
	std::list<animation_cursor> anim_cursors;
	anim_cursors.push_back(animation_cursor(anim_cfg));
	while (!anim_cursors.empty())
	{
		animation_cursor &ac = anim_cursors.back();

		// Reached end of sub-tag config block
		if (ac.itors.first == ac.itors.second) {
			if (!ac.parent) break;
			// Merge all the current branches into the parent.
			ac.parent->branches.splice(ac.parent->branches.end(), ac.branches);
			anim_cursors.pop_back();
			continue;
		}
		if (ac.itors.first->key != "if") {
			// Append current config object to all the branches in scope.
			BOOST_FOREACH(animation_branch &ab, ac.branches) {
				ab.children.push_back(ac.itors.first);
			}
			++ac.itors.first;
			continue;
		}
		int count = 0;
		do {
			/* Copies the current branches to each cursor created for the
			   conditional clauses. Merge attributes of the clause into them. */
			anim_cursors.push_back(animation_cursor(ac.itors.first->cfg, &ac));
			++ac.itors.first;
			++count;
		} while (ac.itors.first != ac.itors.second && ac.itors.first->key == "else");
		if (count > 1) {
			// When else statements present, clear all branches before 'if'
			ac.branches.clear();
		}
	}

	//debug
	/*BOOST_FOREACH(animation_branch &ab, anim_cursors.back().branches) {
		std::cout << "--branch--\n" << ab.attributes;
		BOOST_FOREACH(config::all_children_iterator &ci, ab.children) {
			std::cout << "--branchcfg--\n" << ci->cfg;
		}
		std::cout << "\n";
	}*/

	// Create the config object describing each branch.
	assert(anim_cursors.size() == 1);
	animation_cursor &ac = anim_cursors.back();
	expanded_anims.splice(expanded_anims.end(),
		ac.branches, ac.branches.begin(), ac.branches.end());
}

static animation_branches prepare_animation(const config &cfg, const std::string &animation_tag)
{
	animation_branches expanded_animations;
	BOOST_FOREACH(const config &anim, cfg.child_range(animation_tag)) {
		prepare_single_animation(anim, expanded_animations);
	}
	return expanded_animations;
}

unit_animation::unit_animation(int start_time,
	const unit_frame & frame, const std::string& event, const int variation, const frame_builder & builder) :
		terrain_types_(),
		unit_filter_(),
		secondary_unit_filter_(),
		directions_(),
		frequency_(0),
		base_score_(variation),
		event_(utils::split(event)),
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

unit_animation::unit_animation(const config& cfg,const std::string& frame_string ) :
	terrain_types_(t_translation::read_list(cfg["terrain_type"])),
	unit_filter_(),
	secondary_unit_filter_(),
	directions_(),
	frequency_(cfg["frequency"]),
	base_score_(cfg["base_score"]),
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
//	if(!cfg["debug"].empty()) printf("DEBUG WML: FINAL\n%s\n\n",cfg.debug().c_str());
	BOOST_FOREACH(const config::any_child &fr, cfg.all_children_range())
	{
		if (fr.key == frame_string) continue;
		if (fr.key.find("_frame", fr.key.size() - 6) == std::string::npos) continue;
		if (sub_anims_.find(fr.key) != sub_anims_.end()) continue;
		sub_anims_[fr.key] = particule(cfg, fr.key.substr(0, fr.key.size() - 5));
	}
	event_ =utils::split(cfg["apply_to"]);

	const std::vector<std::string>& my_directions = utils::split(cfg["direction"]);
	for(std::vector<std::string>::const_iterator i = my_directions.begin(); i != my_directions.end(); ++i) {
		const map_location::DIRECTION d = map_location::parse_direction(*i);
		directions_.push_back(d);
	}
	BOOST_FOREACH(const config &filter, cfg.child_range("filter")) {
		unit_filter_.push_back(filter);
	}

	BOOST_FOREACH(const config &filter, cfg.child_range("filter_second")) {
		secondary_unit_filter_.push_back(filter);
	}

	std::vector<std::string> value_str = utils::split(cfg["value"]);
	std::vector<std::string>::iterator value;
	for(value=value_str.begin() ; value != value_str.end() ; ++value) {
		value_.push_back(atoi(value->c_str()));
	}

	std::vector<std::string> hits_str = utils::split(cfg["hits"]);
	std::vector<std::string>::iterator hit;
	for(hit=hits_str.begin() ; hit != hits_str.end() ; ++hit) {
		if(*hit == "yes" || *hit == "hit") {
			hits_.push_back(HIT);
		}
		if(*hit == "no" || *hit == "miss") {
			hits_.push_back(MISS);
		}
		if(*hit == "yes" || *hit == "kill" ) {
			hits_.push_back(KILL);
		}
	}
	std::vector<std::string> value2_str = utils::split(cfg["value_second"]);
	std::vector<std::string>::iterator value2;
	for(value2=value2_str.begin() ; value2 != value2_str.end() ; ++value2) {
		value2_.push_back(atoi(value2->c_str()));
	}
	BOOST_FOREACH(const config &filter, cfg.child_range("filter_attack")) {
		primary_attack_filter_.push_back(filter);
	}
	BOOST_FOREACH(const config &filter, cfg.child_range("filter_second_attack")) {
		secondary_attack_filter_.push_back(filter);
	}
	play_offscreen_ = cfg["offscreen"].to_bool(true);

}

int unit_animation::matches(const display &disp,const map_location& loc,const map_location& second_loc, const unit* my_unit,const std::string & event,const int value,hit_type hit,const attack_type* attack,const attack_type* second_attack, int value2) const
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
	const std::string default_image = cfg["image"];
	std::vector<unit_animation>  animation_base;
	std::vector<unit_animation>::const_iterator itor;
	add_anims(animations,cfg);
	for(itor = animations.begin(); itor != animations.end() ; ++itor) {
		if (std::find(itor->event_.begin(),itor->event_.end(),std::string("default"))!= itor->event_.end()) {
			animation_base.push_back(*itor);
			animation_base.back().base_score_ += unit_animation::DEFAULT_ANIM;
			animation_base.back().event_.clear();
		}
	}


	if( animation_base.empty() )
		animation_base.push_back(unit_animation(0,frame_builder().image(default_image).duration(1),"",unit_animation::DEFAULT_ANIM));

	animations.push_back(unit_animation(0,frame_builder().image(default_image).duration(1),"_disabled_",0));
	animations.push_back(unit_animation(0,frame_builder().image(default_image).duration(300).
					blend("0.0~0.3:100,0.3~0.0:200",display::rgb(255,255,255)),"_disabled_selected_",0));
	for(itor = animation_base.begin() ; itor != animation_base.end() ; ++itor ) {
		//unit_animation tmp_anim = *itor;
		// provide all default anims
		//no event, providing a catch all anim
		//animations.push_back(tmp_anim);

		animations.push_back(*itor);
		animations.back().event_ = utils::split("standing");
		animations.back().play_offscreen_ = false;

		animations.push_back(*itor);
		animations.back().event_ = utils::split("ghosted");
		animations.back().unit_anim_.override(0,animations.back().unit_anim_.get_animation_duration(),particule::UNSET,"0.9","",0,"","","~GS()");

		animations.push_back(*itor);
		animations.back().event_ = utils::split("disabled_ghosted");
		animations.back().unit_anim_.override(0,1,particule::UNSET,"0.4","",0,"","","~GS()");

		animations.push_back(*itor);
		animations.back().event_ = utils::split("selected");
		animations.back().unit_anim_.override(0,300,particule::UNSET,"","0.0~0.3:100,0.3~0.0:200",display::rgb(255,255,255));

		animations.push_back(*itor);
		animations.back().event_ = utils::split("recruited");
		animations.back().unit_anim_.override(0,600,particule::NO_CYCLE,"0~1:600");

		animations.push_back(*itor);
		animations.back().event_ = utils::split("levelin");
		animations.back().unit_anim_.override(0,600,particule::NO_CYCLE,"","1~0:600",display::rgb(255,255,255));

		animations.push_back(*itor);
		animations.back().event_ = utils::split("levelout");
		animations.back().unit_anim_.override(0,600,particule::NO_CYCLE,"","0~1:600,1",display::rgb(255,255,255));

		animations.push_back(*itor);
		animations.back().event_ = utils::split("pre_movement");
		animations.back().unit_anim_.override(0,1,particule::NO_CYCLE);

		animations.push_back(*itor);
		animations.back().event_ = utils::split("post_movement");
		animations.back().unit_anim_.override(0,1,particule::NO_CYCLE);

		animations.push_back(*itor);
		animations.back().event_ = utils::split("movement");
		animations.back().unit_anim_.override(0,200,particule::NO_CYCLE,"","",0,"0~1:200",lexical_cast<std::string>(display::LAYER_UNIT_MOVE_DEFAULT-display::LAYER_UNIT_FIRST));

		animations.push_back(*itor);
		animations.back().event_ = utils::split("defend");
		animations.back().unit_anim_.override(0,animations.back().unit_anim_.get_animation_duration(),particule::NO_CYCLE,"","0.0,0.5:75,0.0:75,0.5:75,0.0",game_display::rgb(255,0,0));
		animations.back().hits_.push_back(HIT);
		animations.back().hits_.push_back(KILL);

		animations.push_back(*itor);
		animations.back().event_ = utils::split("defend");

		animations.push_back(*itor);
		animations.back().event_ = utils::split("attack");
		animations.back().unit_anim_.override(-150,300,particule::NO_CYCLE,"","",0,"0~0.6:150,0.6~0:150",lexical_cast<std::string>(display::LAYER_UNIT_MOVE_DEFAULT-display::LAYER_UNIT_FIRST));
		animations.back().primary_attack_filter_.push_back(config());
		animations.back().primary_attack_filter_.back()["range"] = "melee";

		animations.push_back(*itor);
		animations.back().event_ = utils::split("attack");
		animations.back().unit_anim_.override(-150,150,particule::NO_CYCLE);
		animations.back().primary_attack_filter_.push_back(config());
		animations.back().primary_attack_filter_.back()["range"] = "ranged";

		animations.push_back(*itor);
		animations.back().event_ = utils::split("death");
		animations.back().unit_anim_.override(0,600,particule::NO_CYCLE,"1~0:600");
		animations.back().sub_anims_["_death_sound"] = particule();
		animations.back().sub_anims_["_death_sound"].add_frame(1,frame_builder().sound(cfg["die_sound"]),true);

		animations.push_back(*itor);
		animations.back().event_ = utils::split("victory");
		animations.back().unit_anim_.override(0,animations.back().unit_anim_.get_animation_duration(),particule::CYCLE);

		animations.push_back(*itor);
		animations.back().unit_anim_.override(0,150,particule::NO_CYCLE,"1~0:150");
		animations.back().event_ = utils::split("pre_teleport");

		animations.push_back(*itor);
		animations.back().unit_anim_.override(0,150,particule::NO_CYCLE,"0~1:150,1");
		animations.back().event_ = utils::split("post_teleport");

		animations.push_back(*itor);
		animations.back().event_ = utils::split("healing");

		animations.push_back(*itor);
		animations.back().event_ = utils::split("healed");
		animations.back().unit_anim_.override(0,300,particule::NO_CYCLE,"","0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30",display::rgb(255,255,255));
		std::string healed_sound;
		if (cfg["healed_sound"].empty()) {
			healed_sound = "heal.wav";
		} else {
			healed_sound = cfg["healed_sound"].str();
		}
		animations.back().sub_anims_["_healed_sound"].add_frame(1,frame_builder().sound(healed_sound),true);

		animations.push_back(*itor);
		animations.back().event_ = utils::split("poisoned");
		animations.back().unit_anim_.override(0,300,particule::NO_CYCLE,"","0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30",display::rgb(0,255,0));
		animations.back().sub_anims_["_poison_sound"] = particule();
		animations.back().sub_anims_["_poison_sound"].add_frame(1,frame_builder().sound("poison.ogg"),true);

	}

}

static void add_simple_anim(std::vector<unit_animation> &animations,
	const config &cfg, char const *tag_name, char const *apply_to,
	display::tdrawing_layer layer = display::LAYER_UNIT_DEFAULT,
	bool offscreen = true)
{
	BOOST_FOREACH(const animation_branch &ab, prepare_animation(cfg, tag_name))
	{
		config anim = ab.merge();
		anim["apply_to"] = apply_to;
		if (!offscreen) {
			config::attribute_value &v = anim["offscreen"];
			if (v.empty()) v = false;
		}
		config::attribute_value &v = anim["layer"];
		if (v.empty()) v = layer - display::LAYER_UNIT_FIRST;
		animations.push_back(unit_animation(anim));
	}
}

void unit_animation::add_anims( std::vector<unit_animation> & animations, const config & cfg)
{
	BOOST_FOREACH(const animation_branch &ab, prepare_animation(cfg, "animation")) {
		animations.push_back(unit_animation(ab.merge()));
	}

	const int default_layer = display::LAYER_UNIT_DEFAULT - display::LAYER_UNIT_FIRST;
	const int move_layer = display::LAYER_UNIT_MOVE_DEFAULT - display::LAYER_UNIT_FIRST;
	const int missile_layer = display::LAYER_UNIT_MISSILE_DEFAULT - display::LAYER_UNIT_FIRST;

	add_simple_anim(animations, cfg, "resistance_anim", "resistance");
	add_simple_anim(animations, cfg, "leading_anim", "leading");
	add_simple_anim(animations, cfg, "recruit_anim", "recruited");
	add_simple_anim(animations, cfg, "recruiting_anim", "recruiting");
	add_simple_anim(animations, cfg, "idle_anim", "idling", display::LAYER_UNIT_DEFAULT, false);
	add_simple_anim(animations, cfg, "levelin_anim", "levelin");
	add_simple_anim(animations, cfg, "levelout_anim", "levelout");

	BOOST_FOREACH(const animation_branch &ab, prepare_animation(cfg, "standing_anim"))
	{
		config anim = ab.merge();
		anim["apply_to"] = "standing";
		anim["cycles"] = "true";
		// add cycles to all frames within a standing animation block
		BOOST_FOREACH(config::all_children_iterator ci, ab.children)
		{
			std::string sub_frame_name = ci->key;
			size_t pos = sub_frame_name.find("_frame");
			if (pos != std::string::npos) {
				anim[sub_frame_name.substr(0,pos)+"_cycles"] = "true";
			}
		}
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		if (anim["offscreen"].empty()) anim["offscreen"] = false;
		animations.push_back(unit_animation(anim));
	}
	// standing animations are also used as default animations
	BOOST_FOREACH(const animation_branch &ab, prepare_animation(cfg, "standing_anim"))
	{
		config anim = ab.merge();
		anim["apply_to"] = "default";
		anim["cycles"] = "true";
		BOOST_FOREACH(config::all_children_iterator ci, ab.children)
		{
			std::string sub_frame_name = ci->key;
			size_t pos = sub_frame_name.find("_frame");
			if (pos != std::string::npos) {
				anim[sub_frame_name.substr(0,pos)+"_cycles"] = "true";
			}
		}
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		if (anim["offscreen"].empty()) anim["offscreen"] = false;
		animations.push_back(unit_animation(anim));
	}
	BOOST_FOREACH(const animation_branch &ab, prepare_animation(cfg, "healing_anim"))
	{
		config anim = ab.merge();
		anim["apply_to"] = "healing";
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		anim["value"] = anim["damage"];
		animations.push_back(unit_animation(anim));
	}

	BOOST_FOREACH(const animation_branch &ab, prepare_animation(cfg, "healed_anim"))
	{
		config anim = ab.merge();
		anim["apply_to"] = "healed";
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		anim["value"] = anim["healing"];
		animations.push_back(unit_animation(anim));
		animations.back().sub_anims_["_healed_sound"] = particule();
		std::string healed_sound;
		if (cfg["healed_sound"].empty()) {
			healed_sound = "heal.wav";
		} else {
			healed_sound = cfg["healed_sound"].str();
		}
		animations.back().sub_anims_["_healed_sound"].add_frame(1,frame_builder().sound(healed_sound),true);
	}

	BOOST_FOREACH(const animation_branch &ab, prepare_animation(cfg, "poison_anim"))
	{
		config anim = ab.merge();
		anim["apply_to"] ="poisoned";
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		anim["value"] = anim["damage"];
		animations.push_back(unit_animation(anim));
		animations.back().sub_anims_["_poison_sound"] = particule();
		animations.back().sub_anims_["_poison_sound"].add_frame(1,frame_builder().sound("poison.ogg"),true);
	}

	add_simple_anim(animations, cfg, "pre_movement_anim", "pre_movement", display::LAYER_UNIT_MOVE_DEFAULT);

	BOOST_FOREACH(const animation_branch &ab, prepare_animation(cfg, "movement_anim"))
	{
		config anim = ab.merge();
		if (anim["offset"].empty()) {
			anim["offset"] = "0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,0~1:200,";
		}
		anim["apply_to"] = "movement";
		if (anim["layer"].empty()) anim["layer"] = move_layer;
		animations.push_back(unit_animation(anim));
	}

	add_simple_anim(animations, cfg, "post_movement_anim", "post_movement", display::LAYER_UNIT_MOVE_DEFAULT);

	BOOST_FOREACH(const animation_branch &ab, prepare_animation(cfg, "defend"))
	{
		config anim = ab.merge();
		anim["apply_to"] = "defend";
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		if (!anim["damage"].empty() && anim["value"].empty()) {
			anim["value"] = anim["damage"];
		}
		if (anim["hits"].empty())
		{
			anim["hits"] = false;
			animations.push_back(unit_animation(anim));
			animations.back().base_score_--; //so default doesn't interefere with 'if' block
			anim["hits"] = true;
			animations.push_back(unit_animation(anim));
			animations.back().base_score_--;
			image::locator image_loc = animations.back().get_last_frame().end_parameters().image;
			animations.back().add_frame(225,frame_builder()
					.image(image_loc.get_filename()+image_loc.get_modifications())
					.duration(225)
					.blend("0.0,0.5:75,0.0:75,0.5:75,0.0",game_display::rgb(255,0,0)));
		}
		else
		{
			std::vector<std::string> v = utils::split(anim["hits"]);
			BOOST_FOREACH(const std::string &hit_type, v)
			{
				config tmp = anim;
				tmp["hits"] = hit_type;
				animations.push_back(unit_animation(tmp));
				image::locator image_loc = animations.back().get_last_frame().end_parameters().image;
				if(hit_type == "yes" || hit_type == "hit" || hit_type=="kill") {
					animations.back().add_frame(225,frame_builder()
							.image(image_loc.get_filename()+image_loc.get_modifications())
							.duration(225)
							.blend("0.0,0.5:75,0.0:75,0.5:75,0.0",game_display::rgb(255,0,0)));
				}
			}
		}
	}

	add_simple_anim(animations, cfg, "draw_weapon_anim", "draw_wepaon", display::LAYER_UNIT_MOVE_DEFAULT);
	add_simple_anim(animations, cfg, "sheath_weapon_anim", "sheath_wepaon", display::LAYER_UNIT_MOVE_DEFAULT);

	BOOST_FOREACH(const animation_branch &ab, prepare_animation(cfg, "attack_anim"))
	{
		config anim = ab.merge();
		anim["apply_to"] = "attack";
		if (anim["layer"].empty()) anim["layer"] = move_layer;
		config::const_child_itors missile_fs = anim.child_range("missile_frame");
		if (anim["offset"].empty() && missile_fs.first == missile_fs.second) {
			anim["offset"] ="0~0.6,0.6~0";
		}
		if (missile_fs.first != missile_fs.second) {
			if (anim["missile_offset"].empty()) anim["missile_offset"] = "0~0.8";
			if (anim["missile_layer"].empty()) anim["missile_layer"] = missile_layer;
			config tmp;
			tmp["duration"] = 1;
			anim.add_child("missile_frame", tmp);
			anim.add_child_at("missile_frame", tmp, 0);
		}

		animations.push_back(unit_animation(anim));
	}

	BOOST_FOREACH(const animation_branch &ab, prepare_animation(cfg, "death"))
	{
		config anim = ab.merge();
		anim["apply_to"] = "death";
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		animations.push_back(unit_animation(anim));
		image::locator image_loc = animations.back().get_last_frame().end_parameters().image;

		animations.back().add_frame(600,frame_builder()
									.image(image_loc.get_filename()+image_loc.get_modifications())
									.duration(600)
									.highlight("1~0:600"));
		if(!cfg["die_sound"].empty()) {
			animations.back().sub_anims_["_death_sound"] = particule();
			animations.back().sub_anims_["_death_sound"].add_frame(1,frame_builder().sound(cfg["die_sound"]),true);
		}
	}

	add_simple_anim(animations, cfg, "victory_anim", "victory");

	BOOST_FOREACH(const animation_branch &ab, prepare_animation(cfg, "extra_anim"))
	{
		config anim = ab.merge();
		anim["apply_to"] = anim["flag"];
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		animations.push_back(unit_animation(anim));
	}

	BOOST_FOREACH(const animation_branch &ab, prepare_animation(cfg, "teleport_anim"))
	{
		config anim = ab.merge();
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		anim["apply_to"] = "pre_teleport";
		animations.push_back(unit_animation(anim));
		animations.back().unit_anim_.set_end_time(0);
		anim["apply_to"] ="post_teleport";
		animations.push_back(unit_animation(anim));
		animations.back().unit_anim_.remove_frames_until(0);
	}
}

void unit_animation::particule::override(int start_time
		, int duration
		, const cycle_state cycles
		, const std::string& highlight
		, const std::string& blend_ratio
		, Uint32 blend_color
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
	if(parameters_.need_update()) return true;
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
	const config& cfg, const std::string& frame_string ) :
		animated<unit_frame>(),
		accelerate(true),
		parameters_(),
		halo_id_(0),
		last_frame_begin_time_(0),
		cycles_(false)
{
	config::const_child_itors range = cfg.child_range(frame_string+"frame");
	starting_frame_time_=INT_MAX;
	if(cfg[frame_string+"start_time"].empty() &&range.first != range.second) {
		BOOST_FOREACH(const config &frame, range) {
			starting_frame_time_ = std::min(starting_frame_time_, frame["begin"].to_int());
		}
	} else {
		starting_frame_time_ = cfg[frame_string+"start_time"];
	}

	BOOST_FOREACH(const config &frame, range)
	{
		unit_frame tmp_frame(frame);
		add_frame(tmp_frame.duration(),tmp_frame,!tmp_frame.does_not_change());
	}
	cycles_  = cfg[frame_string+"cycles"].to_bool(false);
	parameters_ = frame_parsed_parameters(frame_builder(cfg,frame_string),get_animation_duration());
	if(!parameters_.does_not_change()  ) {
			force_change();
	}
}

bool unit_animation::need_update() const
{
	if(unit_anim_.need_update()) return true;
	std::map<std::string,particule>::const_iterator anim_itor =sub_anims_.begin();
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
	std::map<std::string,particule>::const_iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
		if(anim_itor->second.need_minimal_update()) return true;
	}
	return false;
}

bool unit_animation::animation_finished() const
{
	if(!unit_anim_.animation_finished()) return false;
	std::map<std::string,particule>::const_iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
		if(!anim_itor->second.animation_finished()) return false;
	}
	return true;
}

bool unit_animation::animation_finished_potential() const
{
	if(!unit_anim_.animation_finished_potential()) return false;
	std::map<std::string,particule>::const_iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
		if(!anim_itor->second.animation_finished_potential()) return false;
	}
	return true;
}

void unit_animation::update_last_draw_time()
{
	double acceleration = unit_anim_.accelerate ? game_display::get_singleton()->turbo_speed() : 1.0;
	unit_anim_.update_last_draw_time(acceleration);
	std::map<std::string,particule>::iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
		anim_itor->second.update_last_draw_time(acceleration);
	}
}

int unit_animation::get_end_time() const
{
	int result = unit_anim_.get_end_time();
	std::map<std::string,particule>::const_iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
		result= std::max<int>(result,anim_itor->second.get_end_time());
	}
	return result;
}

int unit_animation::get_begin_time() const
{
	int result = unit_anim_.get_begin_time();
	std::map<std::string,particule>::const_iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
		result= std::min<int>(result,anim_itor->second.get_begin_time());
	}
	return result;
}

void unit_animation::start_animation(int start_time
		, const map_location &src
		, const map_location &dst
		, const std::string& text
		, const Uint32 text_color
		, const bool accelerate)
{
	unit_anim_.accelerate = accelerate;
	src_ = src;
	dst_ = dst;
	unit_anim_.start_animation(start_time);
	if(!text.empty()) {
		particule crude_build;
		crude_build.add_frame(1,frame_builder());
		crude_build.add_frame(1,frame_builder().text(text,text_color),true);
		sub_anims_["_add_text"] = crude_build;
	}
	std::map<std::string,particule>::iterator anim_itor =sub_anims_.begin();
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

	std::map<std::string,particule>::iterator anim_itor =sub_anims_.begin();
	unit_anim_.pause_animation();
	for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
		anim_itor->second.pause_animation();
	}
}
void unit_animation::restart_animation()
{

	std::map<std::string,particule>::iterator anim_itor =sub_anims_.begin();
	unit_anim_.restart_animation();
	for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
		anim_itor->second.restart_animation();
	}
}
void unit_animation::redraw(frame_parameters& value)
{

	invalidated_=false;
	overlaped_hex_.clear();
	std::map<std::string,particule>::iterator anim_itor =sub_anims_.begin();
	value.primary_frame = t_true;
	unit_anim_.redraw(value,src_,dst_);
	value.primary_frame = t_false;
	for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
		anim_itor->second.redraw( value,src_,dst_);
	}
}
void unit_animation::clear_haloes()
{

	std::map<std::string,particule>::iterator anim_itor =sub_anims_.begin();
	unit_anim_.clear_halo();
	for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
		anim_itor->second.clear_halo();
	}
}
bool unit_animation::invalidate(frame_parameters& value)
{
	if(invalidated_) return false;
	display*disp = display::get_singleton();
	bool complete_redraw =disp->tile_nearly_on_screen(src_) || disp->tile_nearly_on_screen(dst_);
	if(overlaped_hex_.empty()) {
		if(complete_redraw) {
			std::map<std::string,particule>::iterator anim_itor =sub_anims_.begin();
			value.primary_frame = t_true;
			overlaped_hex_ = unit_anim_.get_overlaped_hex(value,src_,dst_);
			value.primary_frame = t_false;
			for( /*null*/; anim_itor != sub_anims_.end() ; ++anim_itor) {
				std::set<map_location> tmp = anim_itor->second.get_overlaped_hex(value,src_,dst_);
				overlaped_hex_.insert(tmp.begin(),tmp.end());
			}
		} else {
			// off screen animations only invalidate their own hex, no propagation,
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

std::ostream& operator << (std::ostream& outstream, const unit_animation& u_animation)
{
	std::cout << "[";
	int i=0;
	BOOST_FOREACH(std::string event, u_animation.event_) {
		if (i>0) std::cout << ','; i++;
		std::cout << event;
	}
	std::cout << "]\n";

	std::cout << "\tstart_time=" << u_animation.get_begin_time() << '\n';

	if (u_animation.hits_.size() > 0) {
		std::cout << "\thits=";
		i=0;
		BOOST_FOREACH(const unit_animation::hit_type hit_type, u_animation.hits_) {
			if (i>0) std::cout << ','; i++;
			switch (hit_type) {
				case (unit_animation::HIT)     : std::cout << "hit"; break;
				case (unit_animation::MISS)    : std::cout << "miss"; break;
				case (unit_animation::KILL)    : std::cout << "kill"; break;
				case (unit_animation::INVALID) : std::cout << "invalid"; break;
			}
		}
		std::cout << '\n';
	}
	if (u_animation.directions_.size() > 0) {
		std::cout << "\tdirections=";
		i=0;
		BOOST_FOREACH(const map_location::DIRECTION direction, u_animation.directions_) {
			if (i>0) std::cout << ','; i++;
			switch (direction) {
				case (map_location::NORTH)     : std::cout << "n"; break;
				case (map_location::NORTH_EAST): std::cout << "ne"; break;
				case (map_location::SOUTH_EAST): std::cout << "se"; break;
				case (map_location::SOUTH)     : std::cout << "s"; break;
				case (map_location::SOUTH_WEST): std::cout << "sw"; break;
				case (map_location::NORTH_WEST): std::cout << "nw"; break;
				default: break;
			}
		}
		std::cout << '\n';
	}
	if (u_animation.terrain_types_.size() > 0) {
		i=0;
		std::cout << "\tterrain=";
		BOOST_FOREACH(const t_translation::t_terrain terrain, u_animation.terrain_types_) {
			if (i>0) std::cout << ','; i++;
			std::cout << terrain;
		}
		std::cout << '\n';
	}
	if (u_animation.frequency_>0) std::cout << "frequency=" << u_animation.frequency_ << '\n';

	if (u_animation.unit_filter_.size() > 0) {
		std::cout << "[filter]\n";
		BOOST_FOREACH(const config cfg, u_animation.unit_filter_) {
			std::cout << cfg.debug();
		}
		std::cout << "[/filter]\n";
	}
	if (u_animation.secondary_unit_filter_.size() > 0) {
		std::cout << "[filter_second]\n";
		BOOST_FOREACH(const config cfg, u_animation.secondary_unit_filter_) {
			std::cout << cfg.debug();
		}
		std::cout << "[/filter_second]\n";
	}
	if (u_animation.primary_attack_filter_.size() > 0) {
		std::cout << "[filter_attack]\n";
		BOOST_FOREACH(const config cfg, u_animation.primary_attack_filter_) {
			std::cout << cfg.debug();
		}
		std::cout << "[/filter_attack]\n";
	}
	if (u_animation.secondary_attack_filter_.size() > 0) {
		std::cout << "[filter_second_attack]\n";
		BOOST_FOREACH(const config cfg, u_animation.secondary_attack_filter_) {
			std::cout << cfg.debug();
		}
		std::cout << "[/filter_second_attack]\n";
	}

	for (size_t i=0; i<u_animation.unit_anim_.get_frames_count(); i++) {
		std::cout << "\t[frame]\n";
		BOOST_FOREACH(const std::string frame_string, u_animation.unit_anim_.get_frame(i).debug_strings()) {
			std::cout << "\t\t" << frame_string <<"\n";
		}
		std::cout << "\t[/frame]\n";
	}

	std::pair<std::string, unit_animation::particule> p;
	BOOST_FOREACH (p, u_animation.sub_anims_) {
		for (size_t i=0; i<p.second.get_frames_count(); i++) {
			std::string sub_frame_name = p.first;
			size_t pos = sub_frame_name.find("_frame");
			if (pos != std::string::npos) sub_frame_name = sub_frame_name.substr(0,pos);
			std::cout << "\t" << sub_frame_name << "_start_time=" << p.second.get_begin_time() << '\n';
			std::cout << "\t[" << p.first << "]\n";
			BOOST_FOREACH(const std::string frame_string, p.second.get_frame(i).debug_strings()) {
				std::cout << "\t\t" << frame_string << '\n';
			}
			std::cout << "\t[/" << p.first << "]\n";
		}
	}

	std::cout << "[/";
	i=0;
	BOOST_FOREACH(std::string event, u_animation.event_) {
		if (i>0) std::cout << ','; i++;
		std::cout << event;
	}
	std::cout << "]\n";
	return outstream;
}


void unit_animation::particule::redraw(const frame_parameters& value,const map_location &src, const map_location &dst)
{
	const unit_frame& current_frame= get_current_frame();
	const int animation_time = get_animation_time();
	const frame_parameters default_val = parameters_.parameters(animation_time -get_begin_time());

	// everything is relative to the first frame in an attack/defense/etc. block.
	// so we need to check if this particular frame is due to be shown at this time
	bool in_scope_of_frame = (animation_time >= get_current_frame_begin_time() ? true: false);
	if (animation_time > get_current_frame_end_time()) in_scope_of_frame = false;

	// sometimes even if the frame is not due to be shown, a frame image still must be shown.
	// i.e. in a defense animation that is shorter than an attack animation.
	// the halos should not persist though and use the 'in_scope_of_frame' variable.

	// for sound frames we want the first time variable set only after the frame has started.
	if(get_current_frame_begin_time() != last_frame_begin_time_ && animation_time >= get_current_frame_begin_time()) {
		last_frame_begin_time_ = get_current_frame_begin_time();
		current_frame.redraw(get_current_frame_time(),true,in_scope_of_frame,src,dst,&halo_id_,default_val,value);
	} else {
		current_frame.redraw(get_current_frame_time(),false,in_scope_of_frame,src,dst,&halo_id_,default_val,value);
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
	const frame_parameters default_val = parameters_.parameters(get_animation_time() -get_begin_time());
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
	parameters_.override(get_animation_duration());
	animated<unit_frame>::start_animation(start_time,cycles_);
	last_frame_begin_time_ = get_begin_time() -1;
}



void unit_animator::add_animation(unit* animated_unit
		, const std::string& event
		, const map_location &src
		, const map_location &dst
		, const int value
		, bool with_bars
		, const std::string& text
		, const Uint32 text_color
		, const unit_animation::hit_type hit_type
		, const attack_type* attack
		, const attack_type* second_attack
		, int value2)
{
	if(!animated_unit) return;
	anim_elem tmp;
	display*disp = display::get_singleton();
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
		, const std::string& text
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
		, const std::string& event
		, const map_location &src
		, const map_location & dst
		, const int value
		, bool with_bars
		, const std::string& text
		, const Uint32 text_color
		, const unit_animation::hit_type hit_type
		, const attack_type* attack
		, const attack_type* second_attack
		, int value2)
{
	if(!animated_unit) return;
	display*disp = display::get_singleton();
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
	display*disp = display::get_singleton();
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

namespace {
class reentry_preventer {
public:
	class entry {
	public:
		bool valid() {
			return valid_;
		}
		operator bool() {
			return valid();
		}
		~entry() {
			--parent_->depth;
		}
	private:
		entry(reentry_preventer *p) : parent_(p), valid_(++p->depth == 1) {}
		reentry_preventer *parent_;
		bool valid_;
		friend class reentry_preventer;
	};

	reentry_preventer() : depth(0) {}
	entry enter() {
		return entry(this);
	}
private:
	unsigned depth;
};
}

void unit_animator::wait_for_end() const
{
	if (game_config::no_delay) return;
	static reentry_preventer rp;
	reentry_preventer::entry rpe = rp.enter();
	assert(rpe || (false && "Reentered a unit animation. See bug #18921")); //Catches reentry
	bool finished = false;
	display*disp = display::get_singleton();
	while(!finished) {
		resources::controller->play_slice(false);
		// Replacing the below assert with a conditional break will fix the local segfault,
		// but this just exposes a different one.
		// It's also unnecessary given the one a few lines up.
		assert(rpe || (false && "Reentered a unit animation. See bug #18921")); //Catches a past reentry
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
