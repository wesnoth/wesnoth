/* $Id$ */
/*
   Copyright (C) 2006 - 2007 by Jeremy Rosen <jeremy.rosen@enst-bretagne.fr>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
   */

#include "global.hpp"

#include "color_range.hpp"
#include "game_display.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "halo.hpp"
#include "pathutils.hpp"
#include "unit.hpp"
#include "unit_animation.hpp"
#include "unit_types.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "sound.hpp"
#include "wassert.hpp"
#include "serialization/string_utils.hpp"


#include <algorithm>
#include <climits>
#include <cstdlib>
#include <iostream>

config unit_animation::prepare_animation(const config &cfg,const std::string animation_tag)
{
	config expanded_animations;
	config::const_child_itors all_anims = cfg.child_range(animation_tag);
	config::const_child_iterator current_anim;
	std::vector<config> unexpanded_anims;
	// store all the anims we have to analyze
	for(current_anim = all_anims.first; current_anim != all_anims.second ; current_anim++) {
		unexpanded_anims.push_back(**current_anim);
	}
	while(!unexpanded_anims.empty()) {
		// take one anim out of the unexpanded list
		const config analyzed_anim = unexpanded_anims.back();
		unexpanded_anims.pop_back();
		config::all_children_iterator child = analyzed_anim.ordered_begin();
		config expanded_anim;
		expanded_anim.values =  analyzed_anim.values;
		while(child != analyzed_anim.ordered_end()) {
			if(*(*child).first == "if") {
				std::vector<config> to_add;
				config expanded_chunk = expanded_anim;
				// add the content of if
				expanded_chunk.append(*(*child).second);
				to_add.push_back(expanded_chunk);
				child++;
				if(child != analyzed_anim.ordered_end() && *(*child).first == "else") {
					while(child != analyzed_anim.ordered_end() && *(*child).first == "else") {
						expanded_chunk = expanded_anim;
						// add the content of else to the stored one
						expanded_chunk.append(*(*child).second);
						to_add.push_back(expanded_chunk);
						// store the partially expanded string for later analyzis
						child++;
					}

				} else {
					// add an anim with the if part removed
					to_add.push_back(expanded_anim);
				}
				// copy the end of the anim "as is" other if will be treated later
				while(child != analyzed_anim.ordered_end()) {
					for(std::vector<config>::iterator itor= to_add.begin(); itor != to_add.end();itor++) {
						itor->add_child(*(*child).first,*(*child).second);

					}
					child++;
				}
				unexpanded_anims.insert(unexpanded_anims.end(),to_add.begin(),to_add.end());
				// stop this one which had an if we have resolved, parse the next one
				continue;
			} else {
				// add the current node
				expanded_anim.add_child(*(*child).first,*(*child).second);
				child++;
			}
		}
		expanded_animations.add_child(animation_tag,expanded_anim);
	}
	return expanded_animations;
}

unit_animation::unit_animation(int start_time,const unit_frame & frame , const std::string & event,const int variation):
	frequency_(0),base_score_(variation), unit_anim_(start_time)
{
	event_.push_back(event);
	add_frame(frame.duration(),frame,!frame.does_not_change());
}

unit_animation::unit_animation(const config& cfg,const std::string frame_string ) :
	terrain_types_(t_translation::read_list(cfg["terrain"])),base_score_(0),
	unit_anim_(cfg,frame_string)
{
	config::child_map::const_iterator frame_itor =cfg.all_children().begin();
	for( /*null*/; frame_itor != cfg.all_children().end() ; frame_itor++) {
		if(frame_itor->first == frame_string) continue;
		if(frame_itor->first.find("_frame",frame_itor->first.size() -6 ) == std::string::npos) continue;
		sub_anims_[frame_itor->first] = crude_animation(cfg,frame_itor->first.substr(0,frame_itor->first.size() -5));
	}
	event_ =utils::split(cfg["apply_to"]);

	const std::vector<std::string>& my_directions = utils::split(cfg["direction"]);
	for(std::vector<std::string>::const_iterator i = my_directions.begin(); i != my_directions.end(); ++i) {
		const gamemap::location::DIRECTION d = gamemap::location::parse_direction(*i);
		directions_.push_back(d);
	}
	config::const_child_iterator itor;
	for(itor = cfg.child_range("unit_filter").first; itor <cfg.child_range("unit_filter").second;itor++) {
		unit_filter_.push_back(**itor);
	}

	for(itor = cfg.child_range("secondary_unit_filter").first; itor <cfg.child_range("secondary_unit_filter").second;itor++) {
		secondary_unit_filter_.push_back(**itor);
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
	for(itor = cfg.child_range("attack_filter").first; itor <cfg.child_range("attack_filter").second;itor++) {
		primary_attack_filter_.push_back(**itor);
	}
	for(itor = cfg.child_range("secondary_attack_filter").first; itor <cfg.child_range("secondary_attack_filter").second;itor++) {
		secondary_attack_filter_.push_back(**itor);
	}

}

int unit_animation::matches(const game_display &disp,const gamemap::location& loc, const unit* my_unit,const std::string & event,const int value,hit_type hit,const attack_type* attack,const attack_type* second_attack, int swing_num) const
{
	int result = base_score_;
	if(!event.empty()) {
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
			const gamemap::location facing_loc = loc.get_direction(my_unit->facing());
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


void unit_animation::back_compat_add_name(const std::string name,const std::string range)
{
	config tmp;
	event_.push_back("attack");
	if(!name.empty()) {
		tmp["name"] = name;
		primary_attack_filter_.push_back(tmp);
	}
	if(!range.empty()) {
		tmp["range"] = range;
		primary_attack_filter_.push_back(tmp);
	}
}


void unit_animation::initialize_anims( std::vector<unit_animation> & animations, const config & cfg, std::vector<attack_type> tmp_attacks)
{
	config expanded_cfg;
	config::child_list::const_iterator anim_itor;

	expanded_cfg = unit_animation::prepare_animation(cfg,"animation");
	const config::child_list& parsed_animations = expanded_cfg.get_children("animation");
	for(anim_itor = parsed_animations.begin(); anim_itor != parsed_animations.end(); ++anim_itor) {
		animations.push_back(unit_animation(**anim_itor));
	}


	expanded_cfg = unit_animation::prepare_animation(cfg,"leading_anim");
	const config::child_list& leading_anims = expanded_cfg.get_children("leading_anim");
	for(anim_itor = leading_anims.begin(); anim_itor != leading_anims.end(); ++anim_itor) {
		(**anim_itor)["apply_to"] ="leading";
		animations.push_back(unit_animation(**anim_itor));
		//lg::wml_error<<"leading animations  are deprecate, support will be removed in 1.3.11 (in unit "<<cfg["name"]<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=leading flag\n";
	}
	animations.push_back(unit_animation(0,unit_frame(image::locator(cfg["image"]),150),"leading",unit_animation::DEFAULT_ANIM));
	expanded_cfg = unit_animation::prepare_animation(cfg,"recruit_anim");
	const config::child_list& recruit_anims = expanded_cfg.get_children("recruit_anim");
	for(anim_itor = recruit_anims.begin(); anim_itor != recruit_anims.end(); ++anim_itor) {
		(**anim_itor)["apply_to"] ="recruited";
		animations.push_back(unit_animation(**anim_itor));
		//lg::wml_error<<"recruit animations  are deprecate, support will be removed in 1.3.11 (in unit "<<cfg["name"]<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=recruited flag\n";
	}
	animations.push_back(unit_animation(0,unit_frame(image::locator(cfg["image"]),600,"0~1:600"),"recruited",unit_animation::DEFAULT_ANIM));
	expanded_cfg = unit_animation::prepare_animation(cfg,"standing_anim");
	const config::child_list& standing_anims = expanded_cfg.get_children("standing_anim");
	for(anim_itor = standing_anims.begin(); anim_itor != standing_anims.end(); ++anim_itor) {
		(**anim_itor)["apply_to"] ="standing";
		animations.push_back(unit_animation(**anim_itor));
		//lg::wml_error<<"standing animations  are deprecate, support will be removed in 1.3.11 (in unit "<<cfg["name"]<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=standing flag\n";
	}
	// Always have a standing animation
	animations.push_back(unit_animation(0,unit_frame(image::locator(cfg["image"]),0),"standing",unit_animation::DEFAULT_ANIM));
	expanded_cfg = unit_animation::prepare_animation(cfg,"idle_anim");
	const config::child_list& idle_anims = expanded_cfg.get_children("idle_anim");
	for(anim_itor = idle_anims.begin(); anim_itor != idle_anims.end(); ++anim_itor) {
		(**anim_itor)["apply_to"] ="idling";
		animations.push_back(unit_animation(**anim_itor));
		//lg::wml_error<<"idling animations  are deprecate, support will be removed in 1.3.11 (in unit "<<cfg["name"]<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=idling flag\n";
	}
	// Idle anims can be empty
	expanded_cfg = unit_animation::prepare_animation(cfg,"levelin_anim");
	const config::child_list& levelin_anims = expanded_cfg.get_children("levelin_anim");
	for(anim_itor = levelin_anims.begin(); anim_itor != levelin_anims.end(); ++anim_itor) {
		(**anim_itor)["apply_to"] ="levelin";
		animations.push_back(unit_animation(**anim_itor));
		//lg::wml_error<<"levelin animations  are deprecate, support will be removed in 1.3.11 (in unit "<<cfg["name"]<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=levelin flag\n";
	}
	animations.push_back(unit_animation(0,unit_frame(image::locator(cfg["image"]),600,"1.0","",display::rgb(255,255,255),"1~0:600"),"levelin",unit_animation::DEFAULT_ANIM));
	// Always have a levelin animation
	expanded_cfg = unit_animation::prepare_animation(cfg,"levelout_anim");
	const config::child_list& levelout_anims = expanded_cfg.get_children("levelout_anim");
	for(anim_itor = levelout_anims.begin(); anim_itor != levelout_anims.end(); ++anim_itor) {
		(**anim_itor)["apply_to"] ="levelout";
		animations.push_back(unit_animation(**anim_itor));
		//lg::wml_error<<"levelout animations  are deprecate, support will be removed in 1.3.11 (in unit "<<cfg["name"]<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=levelout flag\n";
	}
	animations.push_back(unit_animation(0,unit_frame(image::locator(cfg["image"]),600,"1.0","",display::rgb(255,255,255),"0~1:600"),"levelout",unit_animation::DEFAULT_ANIM));
	// Always have a levelout animation
	expanded_cfg = unit_animation::prepare_animation(cfg,"healing_anim");
	const config::child_list& healing_anims = expanded_cfg.get_children("healing_anim");
	for(anim_itor = healing_anims.begin(); anim_itor != healing_anims.end(); ++anim_itor) {
		(**anim_itor)["apply_to"] ="healing";
		(**anim_itor)["value"]=(**anim_itor)["damage"];
		animations.push_back(unit_animation(**anim_itor));
		//lg::wml_error<<"healing animations  are deprecate, support will be removed in 1.3.11 (in unit "<<cfg["name"]<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=healing flag\n";
	}
	animations.push_back(unit_animation(0,unit_frame(image::locator(cfg["image"]),500),"healing",unit_animation::DEFAULT_ANIM));
	// Always have a healing animation
	expanded_cfg = unit_animation::prepare_animation(cfg,"healed_anim");
	const config::child_list& healed_anims = expanded_cfg.get_children("healed_anim");
	for(anim_itor = healed_anims.begin(); anim_itor != healed_anims.end(); ++anim_itor) {
		(**anim_itor)["apply_to"] ="healed";
		(**anim_itor)["value"]=(**anim_itor)["healing"];
		animations.push_back(unit_animation(**anim_itor));
		//lg::wml_error<<"healed animations  are deprecate, support will be removed in 1.3.11 (in unit "<<cfg["name"]<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=healed flag\n";
	}
	animations.push_back(unit_animation(0,unit_frame(image::locator(cfg["image"]),240,"1.0","",display::rgb(255,255,255),"0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30"),"healed",unit_animation::DEFAULT_ANIM));
	// Always have a healed animation
	expanded_cfg = unit_animation::prepare_animation(cfg,"poison_anim");
	const config::child_list& poison_anims = expanded_cfg.get_children("poison_anim");
	for(anim_itor = poison_anims.begin(); anim_itor != poison_anims.end(); ++anim_itor) {
		(**anim_itor)["apply_to"] ="poison";
		(**anim_itor)["value"]=(**anim_itor)["damage"];
		animations.push_back(unit_animation(**anim_itor));
		//lg::wml_error<<"poison animations  are deprecate, support will be removed in 1.3.11 (in unit "<<cfg["name"]<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=poison flag\n";
	}
	animations.push_back(unit_animation(0,unit_frame(image::locator(cfg["image"]),240,"1.0","",display::rgb(0,255,0),"0:30,0.5:30,0:30,0.5:30,0:30,0.5:30,0:30,0.5:30"),"poison",unit_animation::DEFAULT_ANIM));
	// Always have a poison animation
	expanded_cfg = unit_animation::prepare_animation(cfg,"movement_anim");
	const config::child_list& movement_anims = expanded_cfg.get_children("movement_anim");
	for(anim_itor = movement_anims.begin(); anim_itor != movement_anims.end(); ++anim_itor) {
		(**anim_itor)["apply_to"] ="movement";
		animations.push_back(unit_animation(**anim_itor));
		//lg::wml_error<<"movement animations  are deprecate, support will be removed in 1.3.11 (in unit "<<cfg["name"]<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=movement flag\n";
	}
	animations.push_back(unit_animation(0,unit_frame(image::locator(cfg["image"]),150),"movement",unit_animation::DEFAULT_ANIM));
	// Always have a movement animation
	expanded_cfg = unit_animation::prepare_animation(cfg,"defend");
	const config::child_list& defends = expanded_cfg.get_children("defend");
	for(anim_itor = defends.begin(); anim_itor != defends.end(); ++anim_itor) {
		(**anim_itor)["apply_to"] ="defend";
		if(!(**anim_itor)["damage"].empty()) {
			(**anim_itor)["value"]=(**anim_itor)["damage"];
			animations.push_back(unit_animation(**anim_itor));
			if(atoi((**anim_itor)["value"].c_str()) != 0) {
				animations.back().add_frame(100,unit_frame(animations.back().get_last_frame().image(),100,"1.0","",game_display::rgb(255,0,0),"0.5:50,0.0:50"));
			}
		} else {
			(**anim_itor)["value"]="0";
			animations.push_back(unit_animation(**anim_itor)),
			(**anim_itor)["value"]="";
			animations.push_back(unit_animation(**anim_itor)),
			animations.back().add_frame(100,unit_frame(animations.back().get_last_frame().image(),100,"1.0","",game_display::rgb(255,0,0),"0.5:50,0.0:50"));
		}
		//lg::wml_error<<"defend animations  are deprecate, support will be removed in 1.3.11 (in unit "<<cfg["name"]<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=defend flag\n";
	}
	animations.push_back(unit_animation(-150,unit_frame(image::locator(cfg["image"]),300),"defend",unit_animation::DEFAULT_ANIM));
	// Always have a defensive animation
	expanded_cfg = unit_animation::prepare_animation(cfg,"attack_anim");
	const config::child_list& attack_anims = expanded_cfg.get_children("attack_anim");
	for(config::child_list::const_iterator d = attack_anims.begin(); d != attack_anims.end(); ++d) {
		(**d)["apply_to"] ="attack";
		animations.push_back(unit_animation(**d));
		//lg::wml_error<<"attack animations  are deprecate, support will be removed in 1.3.11 (in unit "<<cfg["name"]<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=attack flag\n";
	}
	// get old animation format, to be removed circum 1.3.10
	for(std::vector<attack_type>::iterator attacks_itor = tmp_attacks.begin() ; attacks_itor!= tmp_attacks.end();attacks_itor++) {
		animations.insert(animations.end(),attacks_itor->animation_.begin(),attacks_itor->animation_.end());
		// this has been detected elsewhere, no deprecation message needed here
	}
	animations.push_back(unit_animation(-150,unit_frame(image::locator(cfg["image"]),300),"attack",unit_animation::DEFAULT_ANIM));
	if(!cfg["image_short"].empty()) {
		animations.push_back(unit_animation(-150,unit_frame(image::locator(cfg["image_short"]),300),"attack",unit_animation::DEFAULT_ANIM));
		animations.back().back_compat_add_name("","melee");
		lg::wml_error<<"image_short is deprecated, support will be removed in 1.3.10 (in unit "<<cfg["name"]<<")\n";
	}
	if(!cfg["image_long"].empty()) {
		animations.push_back(unit_animation(-150,unit_frame(image::locator(cfg["image_long"]),300),"attack",unit_animation::DEFAULT_ANIM));
		animations.back().back_compat_add_name("","ranged");
		lg::wml_error<<"image_long is deprecated, support will be removed in 1.3.10 (in unit "<<cfg["name"]<<")\n";
	}
	// always have an attack animation
	expanded_cfg = unit_animation::prepare_animation(cfg,"death");
	const config::child_list& deaths = expanded_cfg.get_children("death");
	for(anim_itor = deaths.begin(); anim_itor != deaths.end(); ++anim_itor) {
		(**anim_itor)["apply_to"] ="death";
		animations.push_back(unit_animation(**anim_itor));
		image::locator image_loc = animations.back().get_last_frame().image();
		animations.back().add_frame(600,unit_frame(image_loc,600,"1~0:600"));
		//lg::wml_error<<"death animations  are deprecate, support will be removed in 1.3.11 (in unit "<<cfg["name"]<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=death flag\n";
	}
	animations.push_back(unit_animation(0,unit_frame(image::locator(cfg["image"]),600,"1~0:600"),"death",unit_animation::DEFAULT_ANIM));
	// Always have a defensive animation
	expanded_cfg = unit_animation::prepare_animation(cfg,"victory_anim");
	const config::child_list& victory_anims = expanded_cfg.get_children("victory_anim");
	for(anim_itor = victory_anims.begin(); anim_itor != victory_anims.end(); ++anim_itor) {
		(**anim_itor)["apply_to"] ="victory";
		animations.push_back(unit_animation(**anim_itor));
		//lg::wml_error<<"victory animations  are deprecate, support will be removed in 1.3.11 (in unit "<<cfg["name"]<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=victory flag\n";
	}
	animations.push_back(unit_animation(0,unit_frame(image::locator(cfg["image"]),1),"victory",unit_animation::DEFAULT_ANIM));
	// Always have a victory animation
	expanded_cfg = unit_animation::prepare_animation(cfg,"extra_anim");
	const config::child_list& extra_anims = expanded_cfg.get_children("extra_anim");
	for(anim_itor = extra_anims.begin(); anim_itor != extra_anims.end(); ++anim_itor) {
		(**anim_itor)["apply_to"] =(**anim_itor)["flag"];
		animations.push_back(unit_animation(**anim_itor));
		//lg::wml_error<<"extra animations  are deprecate, support will be removed in 1.3.11 (in unit "<<cfg["name"]<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=extra flag\n";
	}
	expanded_cfg = unit_animation::prepare_animation(cfg,"teleport_anim");
	const config::child_list& teleports = expanded_cfg.get_children("teleport_anim");
	for(anim_itor = teleports.begin(); anim_itor != teleports.end(); ++anim_itor) {
		(**anim_itor)["apply_to"] ="teleport";
		animations.push_back(unit_animation(**anim_itor));
		//lg::wml_error<<"teleport animations  are deprecate, support will be removed in 1.3.11 (in unit "<<cfg["name"]<<")\n";
		//lg::wml_error<<"please put it with an [animation] tag and apply_to=teleport flag\n";
	}
	animations.push_back(unit_animation(-20,unit_frame(image::locator(cfg["image"]),40),"teleport",unit_animation::DEFAULT_ANIM));
	// Always have a defensive animation

}

const std::string &unit_animation::crude_animation::halo(const std::string&default_val ) const
{
	return get_current_frame().halo(get_current_frame_time(),halo_.get_current_element(get_animation_time() - get_begin_time(),default_val));
}

int unit_animation::crude_animation::halo_x(const int default_val) const 
{
	return get_current_frame().halo_x(get_current_frame_time(),halo_x_.get_current_element(get_animation_time() - get_begin_time(),default_val));
}
int unit_animation::crude_animation::halo_y(const int default_val) const 
{
	return get_current_frame().halo_y(get_current_frame_time(),halo_y_.get_current_element(get_animation_time() - get_begin_time(),default_val)); 
}
double unit_animation::crude_animation::blend_ratio(const double default_val) const
{
	return get_current_frame().blend_ratio(
		get_current_frame_time(),
		ftofxp(blend_ratio_.get_current_element(get_animation_time() - get_begin_time(),default_val))); 
}

fixed_t unit_animation::crude_animation::highlight_ratio(const float default_val) const
{
	return get_current_frame().highlight_ratio(get_current_frame_time(),highlight_ratio_.get_current_element(get_animation_time() - get_begin_time(),default_val));
}

double unit_animation::crude_animation::offset(double default_val) const
{
	return get_current_frame().offset(get_current_frame_time(),offset_.get_current_element(get_animation_time() - get_begin_time(),default_val))  ; 
}

bool unit_animation::crude_animation::need_update() const
{
	if(animated<unit_frame>::need_update()) return true;
	if(get_current_frame().need_update()) return true;
	if(!halo_.does_not_change() ||
			!halo_x_.does_not_change() ||
			!halo_y_.does_not_change() ||
			!blend_ratio_.does_not_change() ||
			!highlight_ratio_.does_not_change() ||
			!offset_.does_not_change() ) {
			return true;
	}
	return false;
}

unit_animation::crude_animation::crude_animation(const config& cfg,const std::string frame_string ) 
{
	config::const_child_itors range = cfg.child_range(frame_string+"frame");
	if(cfg[frame_string+"start_time"].empty() &&range.first != range.second) {
		starting_frame_time_ = atoi((**range.first)["begin"].c_str());
	} else {
		starting_frame_time_ = atoi(cfg[frame_string+"start_time"].c_str());
	}
	if(frame_string == "missile_") {
		// lg::wml_error To be deprecated eventually
		add_frame(1,unit_frame("",1),true);
	}

	for(; range.first != range.second; ++range.first) {
		unit_frame tmp_frame(**range.first);
		add_frame(tmp_frame.duration(),tmp_frame,!tmp_frame.does_not_change());
	}
	halo_ = progressive_string(cfg[frame_string+"halo"],get_animation_duration());
	halo_x_ = progressive_int(cfg[frame_string+"halo_x"],get_animation_duration());
	halo_y_ = progressive_int(cfg[frame_string+"halo_y"],get_animation_duration());
	std::vector<std::string> tmp_blend=utils::split(cfg[frame_string+"blend_color"]);
	if(tmp_blend.size() ==3) blend_with_= display::rgb(atoi(tmp_blend[0].c_str()),atoi(tmp_blend[1].c_str()),atoi(tmp_blend[2].c_str()));
	blend_ratio_ = progressive_double(cfg[frame_string+"blend_ratio"],get_animation_duration());
	highlight_ratio_ = progressive_double(cfg[frame_string+"alpha"],get_animation_duration());
	offset_ = progressive_double(cfg[frame_string+"offset"],get_animation_duration());
	if(offset_.does_not_change() && frame_string == "missile_") {
		// lg::wml_error To be deprecated eventually
		offset_=progressive_double("0~0.8",get_animation_duration());
	}
	if( frame_string == "missile_") {
		// lg::wml_error To be deprecated eventually
		add_frame(1,unit_frame("",1),true);
	}

	if(!halo_.does_not_change() ||
			!halo_x_.does_not_change() ||
			!halo_y_.does_not_change() ||
			!blend_ratio_.does_not_change() ||
			!highlight_ratio_.does_not_change() ||
			!offset_.does_not_change() ) {
			force_change();
	}
}



bool unit_animation::need_update() const
{
	if(unit_anim_.need_update()) return true;
	std::map<std::string,crude_animation>::const_iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; anim_itor++) {
		if(anim_itor->second.need_update()) return true;
	}
	return false;
};
bool unit_animation::animation_finished() const
{
	if(!unit_anim_.animation_finished()) return false;
	std::map<std::string,crude_animation>::const_iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; anim_itor++) {
		if(!anim_itor->second.animation_finished()) return false;
	}
	return true;
};
bool unit_animation::animation_would_finish() const
{
	if(!unit_anim_.animation_would_finish()) return false;
	std::map<std::string,crude_animation>::const_iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; anim_itor++) {
		if(!anim_itor->second.animation_would_finish()) return false;
	}
	return true;
};
void unit_animation::update_last_draw_time() 
{
	unit_anim_.update_last_draw_time();
	std::map<std::string,crude_animation>::iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; anim_itor++) {
		anim_itor->second.update_last_draw_time();
	}
};
int unit_animation::get_end_time() const
{
	int result = unit_anim_.get_end_time();
	std::map<std::string,crude_animation>::const_iterator anim_itor =sub_anims_.end();
	for( /*null*/; anim_itor != sub_anims_.end() ; anim_itor++) {
		result= minimum<int>(result,anim_itor->second.get_end_time());
	}
	return result;
};
int unit_animation::get_begin_time() const
{
	int result = unit_anim_.get_begin_time();
	std::map<std::string,crude_animation>::const_iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; anim_itor++) {
		result= minimum<int>(result,anim_itor->second.get_begin_time());
	}
	return result;
};
void unit_animation::start_animation(int start_time,const gamemap::location &src, const gamemap::location &dst, bool cycles, double acceleration)
{
	unit_anim_.start_animation(start_time, src, dst, cycles, acceleration);
	std::map<std::string,crude_animation>::iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; anim_itor++) {
		anim_itor->second.start_animation(start_time,src,dst,cycles,acceleration);
	}
}
void unit_animation::redraw()
{
	std::map<std::string,crude_animation>::iterator anim_itor =sub_anims_.begin();
	for( /*null*/; anim_itor != sub_anims_.end() ; anim_itor++) {
		anim_itor->second.redraw();
	}
}
void unit_animation::crude_animation::redraw()
{
	const int xsrc = game_display::get_singleton()->get_location_x(src_);
	const int ysrc = game_display::get_singleton()->get_location_y(src_);
	const int xdst = game_display::get_singleton()->get_location_x(dst_);
	const int ydst = game_display::get_singleton()->get_location_y(dst_);
	const gamemap::location::DIRECTION direction = src_.get_relative_dir(dst_);

	double tmp_offset = offset();
	int d2 = game_display::get_singleton()->hex_size() / 2;
	const unit_frame& current_frame= get_current_frame();
	if(!current_frame.sound().empty() &&  get_current_frame_begin_time() != last_frame_begin_time_ ) {
			sound::play_sound(current_frame.sound());
	}
	last_frame_begin_time_ = get_current_frame_begin_time();
	image::locator image_loc;
	if(direction != gamemap::location::NORTH && direction != gamemap::location::SOUTH) {
		image_loc = current_frame.image_diagonal();
	} 
	if(image_loc.is_void()) { // invalid diag image, or not diagonal
		image_loc = current_frame.image();
	}

	surface image;
	if(!image_loc.is_void() && image_loc.get_filename() != "") { // invalid diag image, or not diagonal
		image=image::get_image(image_loc,
				image::SCALED_TO_ZOOM,
				false
				);
	}
	const int x = static_cast<int>(tmp_offset * xdst + (1.0-tmp_offset) * xsrc) + d2 ;
	const int y = static_cast<int>(tmp_offset * ydst + (1.0-tmp_offset) * ysrc) + d2;
	if (image != NULL) {
		bool facing_west = direction == gamemap::location::NORTH_WEST || direction == gamemap::location::SOUTH_WEST;
		bool facing_north = direction == gamemap::location::NORTH_WEST || direction == gamemap::location::NORTH || direction == gamemap::location::NORTH_EAST;
		game_display::get_singleton()->render_unit_image(x- image->w/2, y - image->h/2, image, facing_west, false,
				highlight_ratio(), blend_with_, blend_ratio(),0,!facing_north);
	}
	halo::remove(halo_id_);
	halo_id_ = halo::NO_HALO;
	if(!halo().empty()) {
		halo::ORIENTATION orientation;
		switch(direction)
		{
			case gamemap::location::NORTH:
			case gamemap::location::NORTH_EAST:
				orientation = halo::NORMAL;
				break;
			case gamemap::location::SOUTH_EAST:
			case gamemap::location::SOUTH:
				orientation = halo::VREVERSE;
				break;
			case gamemap::location::SOUTH_WEST:
				orientation = halo::HVREVERSE;
				break;
			case gamemap::location::NORTH_WEST:
				orientation = halo::HREVERSE;
				break;
			case gamemap::location::NDIRECTIONS:
			default:
				orientation = halo::NORMAL;
				break;
		}
		if(direction != gamemap::location::SOUTH_WEST && direction != gamemap::location::NORTH_WEST) {
			halo_id_ = halo::add(x+halo_x(),
					y+halo_y(),
					halo(),
					gamemap::location(-1, -1),
					orientation);
		} else {
			halo_id_ = halo::add(x-halo_x(),
					y+halo_y(),
					halo(),
					gamemap::location(-1, -1),
					orientation);
		}
	}
	update_last_draw_time();
}
unit_animation::crude_animation::~crude_animation()
{
	halo::remove(halo_id_);
	halo_id_ = halo::NO_HALO;
}
void unit_animation::crude_animation::start_animation(int start_time,const gamemap::location &src,const gamemap::location &dst, bool cycles, double acceleration)
{
	halo::remove(halo_id_);
	halo_id_ = halo::NO_HALO;
	animated<unit_frame>::start_animation(start_time,cycles,acceleration);
	last_frame_begin_time_ = get_begin_time() -1;
	if(src != gamemap::location::null_location || dst != gamemap::location::null_location) {
		src_ = src;
		dst_ = dst;
	}
};
