/* $Id$ */
/*
   Copyright (C) 2006 by Jeremy Rosen <jeremy.rosen@enst-bretagne.fr>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
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

unit_animation::unit_animation(int start_time,const unit_frame & frame ):
	animated<unit_frame>(start_time), frequency_(0)
{
	add_frame(frame.duration(),frame,!frame.does_not_change());
}

unit_animation::unit_animation(const config& cfg,const std::string frame_string ) :
	terrain_types_(t_translation::read_list(cfg["terrain"]))
{
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
		directions.push_back(d);
	}
	config::const_child_iterator itor;
	for(itor = cfg.child_range("unit_filter").first; itor <cfg.child_range("unit_filter").second;itor++) {
		unit_filter_.push_back(**itor);
	}

	for(itor = cfg.child_range("secondary_unit_filter").first; itor <cfg.child_range("secondary_unit_filter").second;itor++) {
		secondary_unit_filter_.push_back(**itor);
	}
	frequency_ = atoi(cfg["frequency"].c_str());	

	for(itor = cfg.child_range("neighbour_unit_filter").first; itor <cfg.child_range("neighbour_unit_filter").second;itor++) {
		neighbour_unit_filter_.push_back(**itor);
	}

	/* warn on deprecated WML */
	if(cfg.child("sound")) {
		lg::wml_error << "an animation uses the deprecated [sound] tag, please include sound in the [frame] tag, support will be removed in version 1.3.4\n";

	}
}

int unit_animation::matches(const game_display& disp, const gamemap::location& loc,const unit* my_unit) const
{
	int result = 0;
	if(terrain_types_.empty() == false) {
		if(t_translation::terrain_matches(disp.get_map().get_terrain(loc), terrain_types_)) {	
			result ++;
		} else {
			return -2;
		} 
	}

	if(my_unit) {
		if(directions.empty()== false) {
			if (std::find(directions.begin(),directions.end(),my_unit->facing())== directions.end()) {
				return -2;
			} else {
				result ++;
			}
		}
		std::vector<config>::const_iterator myitor;
		for(myitor = unit_filter_.begin(); myitor != unit_filter_.end(); myitor++) {
			if(!my_unit->matches_filter(&(*myitor),loc)) return -2;
			result++;
		}
		if(!secondary_unit_filter_.empty()) {
			const gamemap::location facing_loc = loc.get_direction(my_unit->facing());
			unit_map::const_iterator unit;
			for(unit=disp.get_const_units().begin() ; unit != disp.get_const_units().end() ; unit++) {
				if(unit->first == facing_loc) {
					std::vector<config>::const_iterator second_itor;
					for(second_itor = secondary_unit_filter_.begin(); second_itor != secondary_unit_filter_.end(); second_itor++) {
						if(!unit->second.matches_filter(&(*second_itor),facing_loc)) return -2;
						result++;
					}

					break;
				}
			}
			if(unit == disp.get_const_units().end()) return -2;
		}
		if(!neighbour_unit_filter_.empty()) {
			gamemap::location neighbour_loc[6] ;
			get_adjacent_tiles(loc,neighbour_loc);
			unit_map::const_iterator unit;
			std::vector<config>::const_iterator second_itor;
			for(second_itor = neighbour_unit_filter_.begin(); second_itor != neighbour_unit_filter_.end(); second_itor++) {
				bool found = false;
				for(unit=disp.get_const_units().begin() ; unit != disp.get_const_units().end() ; unit++) {
					for(int dir =0; dir < 6 ;dir++) {
						if(unit->first == neighbour_loc[dir]) {
							if(unit->second.matches_filter(&(*second_itor),neighbour_loc[dir])) {
								result++;
								found=true;
							}
						}
					}
				}
				if(!found) return -2;
			}
		}

	} else if (!unit_filter_.empty()) return -2;
	if(frequency_ && !(rand()%frequency_)) return -2;

	

	return result;
}

fighting_animation::fighting_animation(const config& cfg) :unit_animation(cfg) 
{
	std::vector<std::string> hits_str = utils::split(cfg["hits"]);
	std::vector<std::string>::iterator hit;
	for(hit=hits_str.begin() ; hit != hits_str.end() ; hit++) {
		if(*hit == "yes" || *hit == "hit") {
			hits.push_back(HIT);
		}
		if(*hit == "no" || *hit == "miss") {
			hits.push_back(MISS);
		}
		if(*hit == "yes" || *hit == "kill" ) {
			hits.push_back(KILL);
		}
	}
	std::vector<std::string> swing_str = utils::split(cfg["swing"]);
	std::vector<std::string>::iterator swing;
	for(swing=swing_str.begin() ; swing != swing_str.end() ; swing++) {
		swing_num.push_back(atoi(swing->c_str()));
	}
	std::vector<std::string> damage_str = utils::split(cfg["damage"]);
	std::vector<std::string>::iterator damage;
	for(damage=damage_str.begin() ; damage != damage_str.end() ; damage++) {
		damage_.push_back(atoi(damage->c_str()));
	}
	config::const_child_iterator itor;
	for(itor = cfg.child_range("attack_filter").first; itor <cfg.child_range("attack_filter").second;itor++) {
		primary_filter.push_back(**itor);
	}
	for(itor = cfg.child_range("secondary_attack_filter").first; itor <cfg.child_range("secondary_attack_filter").second;itor++) {
		secondary_filter.push_back(**itor);
	}

}


int fighting_animation::matches(const game_display& disp, const gamemap::location & loc,const unit* my_unit,
		hit_type hit,const attack_type* attack, const attack_type* secondary_attack,int swing,int damage) const
{
	int result = unit_animation::matches(disp,loc,my_unit);
	if(result == -2) return -2;
	if(hits.empty() == false ) {
		if (std::find(hits.begin(),hits.end(),hit)== hits.end()) {
			return -2;
		} else {
			result ++;
		}
	}
	if(swing_num.empty() == false ) {
		if (std::find(swing_num.begin(),swing_num.end(),swing)== swing_num.end()) {
			return -2;
		} else {
			result ++;
		}
	}
	if(damage_.empty() == false ) {
		if (std::find(damage_.begin(),damage_.end(),damage)== damage_.end()) {
			return -2;
		} else {
			result ++;
		}
	}
	if(!attack) {
		if(!primary_filter.empty())
			return -2;
	}
	std::vector<config>::const_iterator myitor;
	for(myitor = primary_filter.begin(); myitor != primary_filter.end(); myitor++) {
		if(!attack->matches_filter(*myitor)) return -2;
		result++;
	}
	if(!secondary_attack) {
		if(!secondary_filter.empty())
			return -2;
	}
	for(myitor = secondary_filter.begin(); myitor != secondary_filter.end(); myitor++) {
		if(!secondary_attack->matches_filter(*myitor)) return -2;
		result++;
	}
	return result;
}


int poison_animation::matches(const game_display& disp, const gamemap::location & loc,const unit* my_unit,int damage) const
{
	int result = unit_animation::matches(disp,loc,my_unit);
	if(result == -2) return -2;
	if(damage_.empty() == false ) {
		if (std::find(damage_.begin(),damage_.end(),damage)== damage_.end()) {
			return -2;
		} else {
			result ++;
		}
	}
	return result;
}

int healed_animation::matches(const game_display& disp, const gamemap::location & loc,const unit* my_unit,int healing) const
{
	int result = unit_animation::matches(disp,loc,my_unit);
	if(result == -2) return -2;
	if(healing_.empty() == false ) {
		if (std::find(healing_.begin(),healing_.end(),healing)== healing_.end()) {
			return -2;
		} else {
			result ++;
		}
	}
	return result;
}

int healing_animation::matches(const game_display& disp, const gamemap::location & loc,const unit* my_unit,int damage) const
{
	int result = unit_animation::matches(disp,loc,my_unit);
	if(result == -2) return -2;
	if(damage_.empty() == false ) {
		if (std::find(damage_.begin(),damage_.end(),damage)== damage_.end()) {
			return -2;
		} else {
			result ++;
		}
	}
	return result;
}

poison_animation::poison_animation(const config& cfg) :unit_animation(cfg) 
{
	std::vector<std::string> damage_str = utils::split(cfg["damage"]);
	std::vector<std::string>::iterator damage;
	for(damage=damage_str.begin() ; damage != damage_str.end() ; damage++) {
		damage_.push_back(atoi(damage->c_str()));
	}
}

healed_animation::healed_animation(const config& cfg) :unit_animation(cfg) 
{
	std::vector<std::string> damage_str = utils::split(cfg["healing"]);
	std::vector<std::string>::iterator damage;
	for(damage=damage_str.begin() ; damage != damage_str.end() ; damage++) {
		healing_.push_back(atoi(damage->c_str()));
	}
}

healing_animation::healing_animation(const config& cfg) :unit_animation(cfg) 
{
	std::vector<std::string> damage_str = utils::split(cfg["damage"]);
	std::vector<std::string>::iterator damage;
	for(damage=damage_str.begin() ; damage != damage_str.end() ; damage++) {
		damage_.push_back(atoi(damage->c_str()));
	}
}

