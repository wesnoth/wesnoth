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
#include "pathutils.hpp"
#include "unit.hpp"
#include "unit_animation.hpp"
#include "unit_types.hpp"
#include "util.hpp"
#include "variable.hpp"
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
	animated<unit_frame>(start_time), frequency_(0),base_score_(variation)
{
	event_.push_back(event);
	add_frame(frame.duration(),frame,!frame.does_not_change());
}

unit_animation::unit_animation(const config& cfg,const std::string frame_string ) :
	terrain_types_(t_translation::read_list(cfg["terrain"])),base_score_(0)
{
	event_ =utils::split(cfg["apply_to"]);
	config::const_child_itors range = cfg.child_range(frame_string);
	if(cfg["start_time"].empty() &&range.first != range.second) {
		starting_frame_time_ = atoi((**range.first)["begin"].c_str());
	} else {
		starting_frame_time_ = atoi(cfg["start_time"].c_str());
	}

	for(; range.first != range.second; ++range.first) {
		unit_frame tmp_frame(**range.first);
		add_frame(tmp_frame.duration(),tmp_frame,!tmp_frame.does_not_change());
	}

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
	// this whole block is a temporary hack that will stay until proper multi-frame in anim is supported...
	range = cfg.child_range("missile_frame");
	for(; range.first != range.second; ++range.first) {
		unit_frame tmp_frame(**range.first);
		missile_anim_.add_frame(tmp_frame.duration(),tmp_frame,!tmp_frame.does_not_change());
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
