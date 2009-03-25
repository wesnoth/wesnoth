/* $Id$ */
/*
   Copyright (C) 2006 - 2009 by Jeremy Rosen <jeremy.rosen@enst-bretagne.fr>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
   */

#include "global.hpp"
#include "map.hpp"
#include "halo.hpp"
#include "unit.hpp"
#include "foreach.hpp"

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
		names.push_back("standing_anim");
		names.push_back("teleport_anim");
		names.push_back("victory_anim");
	}
	std::vector<std::string> names;
};
namespace {
	tag_name_manager anim_tags;
} //end anonymous namespace

const std::vector<std::string>& unit_animation::all_tag_names() {
	return anim_tags.names;
}

config unit_animation::prepare_animation(const config &cfg,const std::string animation_tag)
{
	config expanded_animations;
	std::vector<config> unexpanded_anims;
	// store all the anims we have to analyze
	foreach (const config &anim, cfg.child_range(animation_tag)) {
		unexpanded_anims.push_back(anim);
	}
	while(!unexpanded_anims.empty()) {
		// take one anim out of the unexpanded list
		const config analyzed_anim = unexpanded_anims.back();
		unexpanded_anims.pop_back();
		config expanded_anim;
		expanded_anim.merge_attributes(analyzed_anim);
		config::all_children_itors children = analyzed_anim.all_children_range();
		for (config::all_children_iterator child = children.first; child != children.second; )
		{
			if (child->key == "if") {
				std::vector<config> to_add;
				config expanded_chunk = expanded_anim;
				// add the content of if
				expanded_chunk.append(child->cfg);
				to_add.push_back(expanded_chunk);
				++child;
				if (child != children.second && child->key == "else") {
					while (child != children.second && child->key == "else") {
						expanded_chunk = expanded_anim;
						// add the content of else to the stored one
						expanded_chunk.append(child->cfg);
						to_add.push_back(expanded_chunk);
						// store the partially expanded string for later analyzis
						++child;
					}
				} else {
					// add an anim with the if part removed
					to_add.push_back(expanded_anim);
				}
				// copy the end of the anim "as is" other if will be treated later
				for (; child != children.second; ++child) {
					foreach (config &c, to_add) {
						c.add_child(child->key, child->cfg);
					}
				}
				unexpanded_anims.insert(unexpanded_anims.end(),to_add.begin(),to_add.end());
			} else {
				// add the current node
				expanded_anim.add_child(child->key, child->cfg);
				++child;
				if (child == children.second)
					expanded_animations.add_child(animation_tag,expanded_anim);
			}
		}
	}
	return expanded_animations;
}

unit_animation::unit_animation(int start_time,
	const unit_frame & frame, const std::string& event, const int variation) :
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
		swing_num_(),
		sub_anims_(),
		unit_anim_(start_time),
		src_(),
		dst_()
{
	add_frame(frame.duration(),frame,!frame.does_not_change());
}

unit_animation::unit_animation(const config& cfg,const std::string frame_string ) :
	terrain_types_(t_translation::read_list(cfg["terrain"])),
	unit_filter_(),
	secondary_unit_filter_(),
	directions_(),
	frequency_(0),
	base_score_(0),
	event_(),
	value_(),
	primary_attack_filter_(),
	secondary_attack_filter_(),
	hits_(),
	swing_num_(),
	sub_anims_(),
	unit_anim_(cfg,frame_string),
	src_(),
	dst_()
{
//	if(!cfg["debug"].empty()) printf("DEBUG WML: FINAL\n%s\n\n",cfg.debug().c_str());
	config::child_map::const_iterator frame_itor =cfg.all_children().begin();
	for( /*null*/; frame_itor != cfg.all_children().end() ; frame_itor++) {
		if(frame_itor->first == frame_string) continue;
		if(frame_itor->first.find("_frame",frame_itor->first.size() -6 ) == std::string::npos) continue;
		sub_anims_[frame_itor->first] = particule(cfg,frame_itor->first.substr(0,frame_itor->first.size() -5));
	}
	event_ =utils::split(cfg["apply_to"]);

	const std::vector<std::string>& my_directions = utils::split(cfg["direction"]);
	for(std::vector<std::string>::const_iterator i = my_directions.begin(); i != my_directions.end(); ++i) {
		const map_location::DIRECTION d = map_location::parse_direction(*i);
		directions_.push_back(d);
	}
	foreach (const config &filter, cfg.child_range("filter")) {
		unit_filter_.push_back(filter);
	}

	foreach (const config &filter, cfg.child_range("secondary_unit_filter")) {
		secondary_unit_filter_.push_back(filter);
	}
	frequency_ = atoi(cfg["frequency"].c_str());

	std::vector<std::string> value_str = utils::split(cfg["value"]);
	std::vector<std::string>::iterator value;
	for(value=value_str.begin() ; value != value_str.end() ; value++) {
		value_.push_back(atoi(value->c_str()));
	}

	std::vector<std::string> hits_str = utils::split(cfg["hits"]);
	std::vector<std::string>::iterator hit;
	for(hit=hits_str.begin() ; hit != hits_str.end() ; hit++) {
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
	std::vector<std::string> swing_str = utils::split(cfg["swing"]);
	std::vector<std::string>::iterator swing;
	for(swing=swing_str.begin() ; swing != swing_str.end() ; swing++) {
		swing_num_.push_back(atoi(swing->c_str()));
	}
	foreach (const config &filter, cfg.child_range("filter_attack")) {
		primary_attack_filter_.push_back(filter);
	}
	foreach (const config &filter, cfg.child_range("filter_second_attack")) {
		secondary_attack_filter_.push_back(filter);
	}

}

int unit_animation::matches(const game_display &disp,const map_location& loc, const unit* my_unit,const std::string & event,const int value,hit_type hit,const attack_type* attack,const attack_type* second_attack, int swing_num) const
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
		for(myitor = unit_filter_.begin(); myitor != unit_filter_.end(); myitor++) {
			if(!my_unit->matches_filter(&(*myitor),loc)) return MATCH_FAIL;
			result++;
		}
		if(!secondary_unit_filter_.empty()) {
			const map_location facing_loc = loc.get_direction(my_unit->facing());
			unit_map::const_iterator unit;
			for(unit=disp.get_const_units().begin() ; unit != disp.get_const_units().end() ; unit++) {
				if(unit->first == facing_loc) {
					std::vector<config>::const_iterator second_itor;
					for(second_itor = secondary_unit_filter_.begin(); second_itor != secondary_unit_filter_.end(); second_itor++) {
						if(!unit->second.matches_filter(&(*second_itor),facing_loc)) return MATCH_FAIL;
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
	if(swing_num_.empty() == false ) {
		if (std::find(swing_num_.begin(),swing_num_.end(),swing_num)== swing_num_.end()) {
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
	for(myitor = primary_attack_filter_.begin(); myitor != primary_attack_filter_.end(); myitor++) {
		if(!attack->matches_filter(*myitor)) return MATCH_FAIL;
		result++;
	}
	if(!second_attack) {
		if(!secondary_attack_filter_.empty())
			return MATCH_FAIL;
	}
	for(myitor = secondary_attack_filter_.begin(); myitor != secondary_attack_filter_.end(); myitor++) {
		if(!second_attack->matches_filter(*myitor)) return MATCH_FAIL;
		result++;
	}
	return result;

}


void unit_animation::fill_initial_animations( std::vector<unit_animation> & animations, const config & cfg)
{
	const image::locator default_image = image::locator(cfg["image"]);
	std::vector<unit_animation>  animation_base;
	std::vector<unit_animation>::const_iterator itor;
	add_anims(animations,cfg);
	for(itor = animations.begin(); itor != animations.end() ; itor++) {
		if (std::find(itor->event_.begin(),itor->event_.end(),std::string("default"))!= itor->event_.end()) {
			animation_base.push_back(*itor);
			animation_base.back().base_score_ = unit_animation::DEFAULT_ANIM;
			animation_base.back().event_.clear();
		}
	}

	const std::string& default_image_in_hex = cfg["in_hex"];
	//const std::string& default_image_in_hex = is_in_hex(default_image) ? "yes" : "no";

	if( animation_base.empty() )
		animation_base.push_back(unit_animation(0,frame_builder().image(default_image).duration(1).in_hex(default_image_in_hex),"",unit_animation::DEFAULT_ANIM));

	//the default standing anim when disbabled use the main in_hex key
		animations.push_back(unit_animation(0,frame_builder().image(default_image).duration(1).in_hex(default_image_in_hex),"_disabled_",0));
	animations.push_back(unit_animation(0,frame_builder().image(default_image).duration(1).
					blend("0.0~0.3:100,0.3~0.0:200",display::rgb(255,255,255)).in_hex(default_image_in_hex),"_disabled_selected_",0));
	for(itor = animation_base.begin() ; itor != animation_base.end() ; itor++ ) {
		unit_animation tmp_anim = *itor;
		// provide all default anims
		//no event, providing a catch all anim
		animations.push_back(tmp_anim);

		tmp_anim = *itor;
		tmp_anim.event_ = utils::split("standing");
		animations.push_back(tmp_anim);

		tmp_anim = *itor;
		tmp_anim.unit_anim_.override(0,300,"","0.0~0.3:100,0.3~0.0:200",display::rgb(255,255,255));
		tmp_anim.event_ = utils::split("selected");
		animations.push_back(tmp_anim);

		tmp_anim = *itor;
		tmp_anim.unit_anim_.override(0,600,"0~1:600");
		tmp_anim.event_ = utils::split("recruited");
		animations.push_back(tmp_anim);

		tmp_anim = *itor;
		tmp_anim.unit_anim_.override(0,600,"","1~0:600",display::rgb(255,255,255));
		tmp_anim.event_ = utils::split("levelin");
		animations.push_back(tmp_anim);

		tmp_anim = *itor;
		tmp_anim.unit_anim_.override(0,600,"","0~1:600,1",display::rgb(255,255,255));
		tmp_anim.event_ = utils::split("levelout");
		animations.push_back(tmp_anim);

		tmp_anim = *itor;
		tmp_anim.unit_anim_.override(0,5100,"","",0,"0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,",lexical_cast<std::string>(display::LAYER_UNIT_MOVE_DEFAULT-display::LAYER_UNIT_FIRST));
		tmp_anim.event_ = utils::split("movement");
		animations.push_back(tmp_anim);

		tmp_anim = *itor;
		tmp_anim.unit_anim_.override(0,225,"","0.0,0.5:75,0.0:75,0.5:75,0.0",game_display::rgb(255,0,0));
		tmp_anim.hits_.push_back(HIT);
		tmp_anim.hits_.push_back(KILL);
		tmp_anim.event_ = utils::split("defend");
		animations.push_back(tmp_anim);

		tmp_anim = *itor;
		tmp_anim.unit_anim_.override(0,1,"");
		tmp_anim.event_ = utils::split("defend");
		animations.push_back(tmp_anim);

		tmp_anim = *itor;
		tmp_anim.unit_anim_.override(-150,300,"","",0,"0~0.6:150,0.6~0:150",lexical_cast<std::string>(display::LAYER_UNIT_MOVE_DEFAULT-display::LAYER_UNIT_FIRST));
		tmp_anim.event_ = utils::split("attack");
		tmp_anim.primary_attack_filter_.push_back(config());
		tmp_anim.primary_attack_filter_.back()["range"] = "melee";
		animations.push_back(tmp_anim);

		tmp_anim = *itor;
		tmp_anim.unit_anim_.override(-150,150);
		tmp_anim.event_ = utils::split("attack");
		tmp_anim.primary_attack_filter_.push_back(config());
		tmp_anim.primary_attack_filter_.back()["range"] = "ranged";
		animations.push_back(tmp_anim);

		tmp_anim = *itor;
		tmp_anim.unit_anim_.override(0,600,"1~0:600");
		tmp_anim.event_ = utils::split("death");
		animations.push_back(tmp_anim);
		animations.back().sub_anims_["_death_sound"] = particule();
		animations.back().sub_anims_["_death_sound"].add_frame(1,frame_builder());
		animations.back().sub_anims_["_death_sound"].add_frame(1,frame_builder().sound(cfg["die_sound"]),true);


		tmp_anim = *itor;
		tmp_anim.unit_anim_.override(0,150,"1~0:150");
		tmp_anim.event_ = utils::split("pre_teleport");
		animations.push_back(tmp_anim);

		tmp_anim = *itor;
		tmp_anim.unit_anim_.override(0,150,"0~1:150,1");
		tmp_anim.event_ = utils::split("post_teleport");
		animations.push_back(tmp_anim);

		tmp_anim = *itor;
		tmp_anim.unit_anim_.override(0,300,"","0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30",display::rgb(255,255,255));
		tmp_anim.event_ = utils::split("healed");
		animations.push_back(tmp_anim);
		animations.back().sub_anims_["_healed_sound"] = particule();
		animations.back().sub_anims_["_healed_sound"].add_frame(1,frame_builder());
		animations.back().sub_anims_["_healed_sound"].add_frame(1,frame_builder().sound("heal.wav"),true);

		tmp_anim = *itor;
		tmp_anim.unit_anim_.override(0,300,"","0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30",display::rgb(0,255,0));
		tmp_anim.event_ = utils::split("poisoned");
		animations.push_back(tmp_anim);
		animations.back().sub_anims_["_poison_sound"] = particule();
		animations.back().sub_anims_["_poison_sound"].add_frame(1,frame_builder());
		animations.back().sub_anims_["_poison_sound"].add_frame(1,frame_builder().sound("poison.ogg"),true);

	}

}

void unit_animation::add_anims( std::vector<unit_animation> & animations, const config & cfg)
{
	config expanded_cfg;

	expanded_cfg = unit_animation::prepare_animation(cfg,"animation");
	foreach (const config &anim, expanded_cfg.child_range("animation")) {
		animations.push_back(unit_animation(anim));
	}

	std::string default_layer = lexical_cast<std::string>
		(display::LAYER_UNIT_DEFAULT - display::LAYER_UNIT_FIRST);
	std::string move_layer = lexical_cast<std::string>
		(display::LAYER_UNIT_MOVE_DEFAULT - display::LAYER_UNIT_FIRST);
	std::string missile_layer = lexical_cast<std::string>
		(display::LAYER_UNIT_MISSILE_DEFAULT - display::LAYER_UNIT_FIRST);

	expanded_cfg = unit_animation::prepare_animation(cfg,"resistance_anim");
	foreach (config &anim, expanded_cfg.child_range("resistance_anim"))
	{
		anim["apply_to"] = "resistance";
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		animations.push_back(unit_animation(anim));
	}

	expanded_cfg = unit_animation::prepare_animation(cfg,"leading_anim");
	foreach (config &anim, expanded_cfg.child_range("leading_anim"))
	{
		anim["apply_to"] = "leading";
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		animations.push_back(unit_animation(anim));
	}

	expanded_cfg = unit_animation::prepare_animation(cfg,"recruit_anim");
	foreach (config &anim, expanded_cfg.child_range("recruit_anim"))
	{
		anim["apply_to"] = "recruited";
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		animations.push_back(unit_animation(anim));
	}

	expanded_cfg = unit_animation::prepare_animation(cfg,"standing_anim");
	foreach (config &anim, expanded_cfg.child_range("standing_anim"))
	{
		anim["apply_to"] = "standing,default";
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		animations.push_back(unit_animation(anim));
	}

	expanded_cfg = unit_animation::prepare_animation(cfg,"idle_anim");
	foreach (config &anim, expanded_cfg.child_range("idle_anim"))
	{
		anim["apply_to"] = "idling";
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		animations.push_back(unit_animation(anim));
	}

	expanded_cfg = unit_animation::prepare_animation(cfg,"levelin_anim");
	foreach (config &anim, expanded_cfg.child_range("levelin_anim"))
	{
		anim["apply_to"] = "levelin";
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		animations.push_back(unit_animation(anim));
	}

	expanded_cfg = unit_animation::prepare_animation(cfg,"levelout_anim");
	foreach (config &anim, expanded_cfg.child_range("levelout_anim"))
	{
		anim["apply_to"] = "levelout";
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		animations.push_back(unit_animation(anim));
	}

	expanded_cfg = unit_animation::prepare_animation(cfg,"healing_anim");
	foreach (config &anim, expanded_cfg.child_range("healing_anim"))
	{
		anim["apply_to"] = "healing";
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		anim["value"] = anim["damage"];
		animations.push_back(unit_animation(anim));
	}

	expanded_cfg = unit_animation::prepare_animation(cfg,"healed_anim");
	foreach (config &anim, expanded_cfg.child_range("healed_anim"))
	{
		anim["apply_to"] = "healed";
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		anim["value"] = anim["healing"];
		animations.push_back(unit_animation(anim));
		animations.back().sub_anims_["_healed_sound"] = particule();
		animations.back().sub_anims_["_healed_sound"].add_frame(1,frame_builder());
		animations.back().sub_anims_["_healed_sound"].add_frame(1,frame_builder().sound("heal.wav"),true);
	}

	expanded_cfg = unit_animation::prepare_animation(cfg,"poison_anim");
	foreach (config &anim, expanded_cfg.child_range("poison_anim"))
	{
		anim["apply_to"] ="poisoned";
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		anim["value"] = anim["damage"];
		animations.push_back(unit_animation(anim));
		animations.back().sub_anims_["_poison_sound"] = particule();
		animations.back().sub_anims_["_poison_sound"].add_frame(1,frame_builder());
		animations.back().sub_anims_["_poison_sound"].add_frame(1,frame_builder().sound("poison.ogg"),true);
	}

	expanded_cfg = unit_animation::prepare_animation(cfg,"movement_anim");
	foreach (config &anim, expanded_cfg.child_range("movement_anim"))
	{
		if (anim["offset"].empty()) {
			anim["offset"] = "0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,0~1:150,";
		}
		anim["apply_to"] = "movement";
		if (anim["layer"].empty()) anim["layer"] = move_layer;
		animations.push_back(unit_animation(anim));
	}

	expanded_cfg = unit_animation::prepare_animation(cfg,"defend");
	foreach (config &anim, expanded_cfg.child_range("defend"))
	{
		anim["apply_to"] = "defend";
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		if (!anim["damage"].empty() && anim["value"].empty()) {
			anim["value"] = anim["damage"];
		}
		if (anim["hits"].empty())
		{
			anim["hits"] = "no";
			animations.push_back(unit_animation(anim));
			anim["hits"] = "yes";
			animations.push_back(unit_animation(anim));
			animations.back().add_frame(225,frame_builder()
					.image(animations.back().get_last_frame().parameters(0).image)
					.duration(225)
					.blend("0.0,0.5:75,0.0:75,0.5:75,0.0",game_display::rgb(255,0,0)));
		}
		else
		{
			std::vector<std::string> v = utils::split(anim["hits"]);
			foreach (const std::string &hit_type, v)
			{
				config tmp = anim;
				tmp["hits"] = hit_type;
				animations.push_back(unit_animation(tmp));
				if(hit_type == "yes" || hit_type == "hit" || hit_type=="kill") {
					animations.back().add_frame(225,frame_builder()
							.image(animations.back().get_last_frame().parameters(0).image)
							.duration(225)
							.blend("0.0,0.5:75,0.0:75,0.5:75,0.0",game_display::rgb(255,0,0)));
				}
			}
		}
	}

	expanded_cfg = unit_animation::prepare_animation(cfg,"attack_anim");
	foreach (config &anim, expanded_cfg.child_range("attack_anim"))
	{
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
			tmp["duration"]="1";
			anim.add_child("missile_frame", tmp);
			anim.add_child_at("missile_frame", tmp, 0);
		}

		animations.push_back(unit_animation(anim));
	}

	expanded_cfg = unit_animation::prepare_animation(cfg,"death");
	foreach (config &anim, expanded_cfg.child_range("death"))
	{
		anim["apply_to"] = "death";
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		animations.push_back(unit_animation(anim));
		image::locator image_loc = animations.back().get_last_frame().parameters(0).image;
		animations.back().add_frame(600,frame_builder().image(image_loc).duration(600).highlight("1~0:600"));
		if(!cfg["die_sound"].empty()) {
			animations.back().sub_anims_["_death_sound"] = particule();
			animations.back().sub_anims_["_death_sound"].add_frame(1,frame_builder());
			animations.back().sub_anims_["_death_sound"].add_frame(1,frame_builder().sound(cfg["die_sound"]),true);
		}
	}

	expanded_cfg = unit_animation::prepare_animation(cfg,"victory_anim");
	foreach (config &anim, expanded_cfg.child_range("victory_anim"))
	{
		anim["apply_to"] = "victory";
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		animations.push_back(unit_animation(anim));
	}

	expanded_cfg = unit_animation::prepare_animation(cfg,"extra_anim");
	foreach (config &anim, expanded_cfg.child_range("extra_anim"))
	{
		anim["apply_to"] = anim["flag"];
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		animations.push_back(unit_animation(anim));
	}

	expanded_cfg = unit_animation::prepare_animation(cfg,"teleport_anim");
	foreach (config &anim, expanded_cfg.child_range("teleport_anim"))
	{
		if (anim["layer"].empty()) anim["layer"] = default_layer;
		anim["apply_to"] = "pre_teleport";
		animations.push_back(unit_animation(anim));
		animations.back().unit_anim_.remove_frames_after(0);
		anim["apply_to"] ="post_teleport";
		animations.push_back(unit_animation(anim));
		animations.back().unit_anim_.remove_frames_until(0);
	}
}

void unit_animation::particule::override( int start_time,int duration, const std::string highlight,const std::string blend_ratio ,Uint32 blend_color ,const std::string offset,const std::string layer)
{
	set_begin_time(start_time);
	if(!highlight.empty()) parameters_.highlight(highlight);
	if(!offset.empty()) parameters_.offset(offset);
	if(!blend_ratio.empty()) parameters_.blend(blend_ratio,blend_color);
	if(!layer.empty()) parameters_.drawing_layer(layer);


	parameters_.duration(duration);
	parameters_.recalculate_duration();
	if(get_animation_duration() < duration) {
		const unit_frame & last_frame = get_last_frame();
		add_frame(duration -get_animation_duration(), last_frame);
	} else if(get_animation_duration() > duration) {
		remove_frames_after(duration);
	}


}

bool unit_animation::particule::need_update() const
{
	if(animated<unit_frame>::need_update()) return true;
	if(get_current_frame().need_update()) return true;
	if(parameters_.need_update()) return true;
	return false;
}

unit_animation::particule::particule(
	const config& cfg, const std::string frame_string ) :
		animated<unit_frame>(),
		accelerate(true),
		parameters_(cfg,frame_string),
		halo_id_(0),
		last_frame_begin_time_(0),
		invalidated_(false)
{
	config::const_child_itors range = cfg.child_range(frame_string+"frame");
	config::const_child_iterator itor;
	starting_frame_time_=INT_MAX;
	if(cfg[frame_string+"start_time"].empty() &&range.first != range.second) {
		foreach (const config &frame, range) {
			starting_frame_time_ = std::min(starting_frame_time_, atoi(frame["begin"].c_str()));
		}
	} else {
		starting_frame_time_ = atoi(cfg[frame_string+"start_time"].c_str());
	}

	foreach (const config &frame, range)
	{
		unit_frame tmp_frame(frame);
		add_frame(tmp_frame.duration(),tmp_frame,!tmp_frame.does_not_change());
	}
	parameters_.duration(get_animation_duration());
	if(!parameters_.does_not_change()  ) {
			force_change();
	}
}

bool unit_animation::need_update() const
{
	if(unit_anim_.need_update()) return true;
	std::map<std::string,particule>::const_iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; anim_itor++) {
		if(anim_itor->second.need_update()) return true;
	}
	return false;
}

bool unit_animation::animation_finished() const
{
	if(!unit_anim_.animation_finished()) return false;
	std::map<std::string,particule>::const_iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; anim_itor++) {
		if(!anim_itor->second.animation_finished()) return false;
	}
	return true;
}

bool unit_animation::animation_finished_potential() const
{
	if(!unit_anim_.animation_finished_potential()) return false;
	std::map<std::string,particule>::const_iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; anim_itor++) {
		if(!anim_itor->second.animation_finished_potential()) return false;
	}
	return true;
}

void unit_animation::update_last_draw_time()
{
	double acceleration = unit_anim_.accelerate ? game_display::get_singleton()->turbo_speed() : 1.0;
	unit_anim_.update_last_draw_time(acceleration);
	std::map<std::string,particule>::iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; anim_itor++) {
		anim_itor->second.update_last_draw_time(acceleration);
	}
}

int unit_animation::get_end_time() const
{
	int result = unit_anim_.get_end_time();
	std::map<std::string,particule>::const_iterator anim_itor =sub_anims_.end();
	for( /*null*/; anim_itor != sub_anims_.end() ; anim_itor++) {
		result= std::max<int>(result,anim_itor->second.get_end_time());
	}
	return result;
}

int unit_animation::get_begin_time() const
{
	int result = unit_anim_.get_begin_time();
	std::map<std::string,particule>::const_iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; anim_itor++) {
		result= std::min<int>(result,anim_itor->second.get_begin_time());
	}
	return result;
}

void unit_animation::start_animation(int start_time,const map_location &src, const map_location &dst, bool cycles, const std::string text, const Uint32 text_color,const bool accelerate)
{
	unit_anim_.accelerate = accelerate;
	src_ = src;
	dst_ = dst;
	unit_anim_.start_animation(start_time, cycles);
	if(!text.empty()) {
		particule crude_build;
		crude_build.add_frame(1,frame_builder());
		crude_build.add_frame(1,frame_builder().text(text,text_color),true);
		sub_anims_["_add_text"] = crude_build;
	}
	std::map<std::string,particule>::iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; anim_itor++) {
		anim_itor->second.accelerate = accelerate;
		anim_itor->second.start_animation(start_time,cycles);
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
	for( /*null*/; anim_itor != sub_anims_.end() ; anim_itor++) {
		anim_itor->second.pause_animation();
	}
}
void unit_animation::restart_animation()
{

	std::map<std::string,particule>::iterator anim_itor =sub_anims_.begin();
	unit_anim_.restart_animation();
	for( /*null*/; anim_itor != sub_anims_.end() ; anim_itor++) {
		anim_itor->second.restart_animation();
	}
}
void unit_animation::redraw(const frame_parameters& value)
{

	std::map<std::string,particule>::iterator anim_itor =sub_anims_.begin();
	unit_anim_.redraw(value,src_,dst_,true);
	for( /*null*/; anim_itor != sub_anims_.end() ; anim_itor++) {
		anim_itor->second.redraw( value,src_,dst_);
	}
}
bool unit_animation::invalidate(const frame_parameters& value)
{

	bool result = false;
	std::map<std::string,particule>::iterator anim_itor =sub_anims_.begin();
	result |= unit_anim_.invalidate(value,src_,dst_,true);
	for( /*null*/; anim_itor != sub_anims_.end() ; anim_itor++) {
		result |= anim_itor->second.invalidate(value,src_,dst_);
	}
	return result;
}
void unit_animation::particule::redraw(const frame_parameters& value,const map_location &src, const map_location &dst, const bool primary)
{
	invalidated_=false;
	const unit_frame& current_frame= get_current_frame();
	const frame_parameters default_val = parameters_.parameters(get_animation_time() -get_begin_time());
	if(get_current_frame_begin_time() != last_frame_begin_time_ ) {
		last_frame_begin_time_ = get_current_frame_begin_time();
		current_frame.redraw(get_current_frame_time(),true,src,dst,&halo_id_,default_val,value,primary);
	} else {
		current_frame.redraw(get_current_frame_time(),false,src,dst,&halo_id_,default_val,value,primary);
	}
}
bool unit_animation::particule::invalidate(const frame_parameters& value,const map_location &src, const map_location &dst, const bool primary )
{
	if(invalidated_) return false;
	const unit_frame& current_frame= get_current_frame();
	const frame_parameters default_val = parameters_.parameters(get_animation_time() -get_begin_time());
	invalidated_ = current_frame.invalidate(need_update(),get_current_frame_time(),src,dst,default_val,value,primary);
	return invalidated_;
}

unit_animation::particule::~particule()
{
	halo::remove(halo_id_);
	halo_id_ = halo::NO_HALO;
}

void unit_animation::particule::start_animation(int start_time, bool cycles)
{
	halo::remove(halo_id_);
	halo_id_ = halo::NO_HALO;
	parameters_.duration(get_animation_duration());
	animated<unit_frame>::start_animation(start_time,cycles);
	last_frame_begin_time_ = get_begin_time() -1;
}

void unit_animator::add_animation(unit* animated_unit,const std::string& event,
		const map_location &src , const int value,bool with_bars,bool cycles,
		const std::string text,const Uint32 text_color,
		const unit_animation::hit_type hit_type,
		const attack_type* attack, const attack_type* second_attack, int swing_num)
{
	if(!animated_unit) return;
	anim_elem tmp;
	game_display*disp = game_display::get_singleton();
	tmp.my_unit = animated_unit;
	tmp.text = text;
	tmp.text_color = text_color;
	tmp.src = src;
	tmp.with_bars= with_bars;
	tmp.cycles = cycles;
	tmp.animation = animated_unit->choose_animation(*disp,src,event,value,hit_type,attack,second_attack,swing_num);
	if(!tmp.animation) return;



	start_time_ = std::max<int>(start_time_,tmp.animation->get_begin_time());
	animated_units_.push_back(tmp);
}
void unit_animator::replace_anim_if_invalid(unit* animated_unit,const std::string& event,
		const map_location &src , const int value,bool with_bars,bool cycles,
		const std::string text,const Uint32 text_color,
		const unit_animation::hit_type hit_type,
		const attack_type* attack, const attack_type* second_attack, int swing_num)
{
	if(!animated_unit) return;
	game_display*disp = game_display::get_singleton();
	if(animated_unit->get_animation() &&
			!animated_unit->get_animation()->animation_finished_potential() &&
			animated_unit->get_animation()->matches(*disp,src,animated_unit,event,value,hit_type,attack,second_attack,swing_num) >unit_animation::MATCH_FAIL) {
		anim_elem tmp;
		tmp.my_unit = animated_unit;
		tmp.text = text;
		tmp.text_color = text_color;
		tmp.src = src;
		tmp.with_bars= with_bars;
		tmp.cycles = cycles;
		tmp.animation = NULL;
		animated_units_.push_back(tmp);
	}else {
		add_animation(animated_unit,event,src,value,with_bars,cycles,text,text_color,hit_type,attack,second_attack,swing_num);
	}
}
void unit_animator::start_animations()
{
	int begin_time = INT_MAX;
	std::vector<anim_elem>::iterator anim;
	for(anim = animated_units_.begin(); anim != animated_units_.end();anim++) {
		if(anim->my_unit->get_animation()) {
			if(anim->animation) {
				begin_time = std::min<int>(begin_time,anim->animation->get_begin_time());
			} else  {
				begin_time = std::min<int>(begin_time,anim->my_unit->get_animation()->get_begin_time());
			}
		}
	}
	for(anim = animated_units_.begin(); anim != animated_units_.end();anim++) {
		if(anim->animation) {
			anim->my_unit->start_animation(begin_time,anim->src, anim->animation,anim->with_bars, anim->cycles,anim->text,anim->text_color);
			anim->animation = NULL;
		} else {
			anim->my_unit->get_animation()->update_parameters(anim->src,anim->src.get_direction(anim->my_unit->facing()));
		}

	}
}

bool unit_animator::would_end() const
{
	bool finished = true;
	for(std::vector<anim_elem>::const_iterator anim = animated_units_.begin(); anim != animated_units_.end();anim++) {
		finished &= anim->my_unit->get_animation()->animation_finished_potential();
	}
	return finished;
}
void unit_animator::wait_until(int animation_time) const
{
	game_display*disp = game_display::get_singleton();
	double speed = disp->turbo_speed();
        disp->draw();
        events::pump();
	int end_tick = animated_units_[0].my_unit->get_animation()->time_to_tick(animation_time);
	while (SDL_GetTicks() < (unsigned int)end_tick - std::min<int>((unsigned int)(20/speed),20)) {
		disp->delay(std::max<int>(0,
			std::min<int>(10,
			static_cast<int>((animation_time - get_animation_time()) * speed))));
		disp->draw();
		events::pump();
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
		disp->draw();
		events::pump();
		disp->delay(10);
		finished = true;
		for(std::vector<anim_elem>::const_iterator anim = animated_units_.begin(); anim != animated_units_.end();anim++) {
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
        for(std::vector<anim_elem>::const_iterator anim = animated_units_.begin(); anim != animated_units_.end();anim++) {
	       if(anim->my_unit->get_animation()) {
                end_time = std::max<int>(end_time,anim->my_unit->get_animation()->get_end_time());
	       }
        }
        return end_time;
}
void unit_animator::pause_animation()
{
        for(std::vector<anim_elem>::iterator anim = animated_units_.begin(); anim != animated_units_.end();anim++) {
	       if(anim->my_unit->get_animation()) {
                anim->my_unit->get_animation()->pause_animation();
	       }
        }
}
void unit_animator::restart_animation()
{
        for(std::vector<anim_elem>::iterator anim = animated_units_.begin(); anim != animated_units_.end();anim++) {
	       if(anim->my_unit->get_animation()) {
                anim->my_unit->get_animation()->restart_animation();
	       }
        }
}
void unit_animator::set_all_standing()
{
        for(std::vector<anim_elem>::iterator anim = animated_units_.begin(); anim != animated_units_.end();anim++) {
                anim->my_unit->set_standing(anim->src);
        }
}
