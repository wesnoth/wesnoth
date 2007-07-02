/* $Id: unit_types.cpp 9735 2006-01-18 18:31:24Z boucman $ */
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

#include "game_config.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "unit_animation.hpp"
#include "unit_types.hpp"
#include "util.hpp"
#include "wassert.hpp"
#include "serialization/string_utils.hpp"
#include "color_range.hpp"

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
					// add an animw with the if part removed
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
unit_frame::unit_frame(const config& cfg)
{
	xoffset = atoi(cfg["xoffset"].c_str());
	image = cfg["image"];
	image_diagonal = cfg["image_diagonal"];
	halo_x = atoi(cfg["halo_x"].c_str());
	halo_y = atoi(cfg["halo_y"].c_str());
	sound = cfg["sound"];
	begin_time = atoi(cfg["begin"].c_str());
	end_time = atoi(cfg["end"].c_str());
	highlight_ratio = ftofxp(1);
	halo = prepare_halo(cfg["halo"],begin_time,end_time);
	blend_with= 0;
	blend_ratio = 0;

}

std::vector<std::pair<std::string,int> > unit_frame::prepare_halo(const std::string & halo,int begin, int end)
{
		const int duration = end - begin;
		const std::vector<std::string> first_pass = utils::split(halo);
		const int time_chunk = maximum<int>(duration / (first_pass.size()?first_pass.size():1),1);

		std::vector<std::string>::const_iterator tmp;
		std::vector<std::pair<std::string,int> > result;
		for(tmp=first_pass.begin();tmp != first_pass.end() ; tmp++) {
			std::vector<std::string> second_pass = utils::split(*tmp,':');
			if(second_pass.size() > 1) {
				result.push_back(std::pair<std::string,int>(second_pass[0],atoi(second_pass[1].c_str())));
			} else {
				result.push_back(std::pair<std::string,int>(second_pass[0],time_chunk));
			}
		}
		return result;
}



unit_animation::unit_animation(const std::string image )
{
	add_frame(0,unit_frame(image));
}

unit_animation::unit_animation(const config& cfg,const std::string frame_string ):terrain_types(utils::split(cfg["terrain"])){
	config::const_child_itors range = cfg.child_range(frame_string);

	int last_end = INT_MIN;
	for(; range.first != range.second; ++range.first) {
		add_frame(atoi((**range.first)["begin"].c_str()), unit_frame(**range.first));
		last_end = maximum<int>(atoi((**range.first)["end"].c_str()), last_end);
	}
	add_frame(last_end);

	const std::vector<std::string>& my_directions = utils::split(cfg["direction"]);
	for(std::vector<std::string>::const_iterator i = my_directions.begin(); i != my_directions.end(); ++i) {
		const gamemap::location::DIRECTION d = gamemap::location::parse_direction(*i);
		directions.push_back(d);
	}

	/* warn on deprecated WML */
	if(cfg.child("sound")) {
		LOG_STREAM(err, config) << "an animation uses the deprecated [sound] tag, please include sound in the [frame] tag\n";

	}
}

unit_animation::unit_animation(const std::string image, int begin_at, int end_at, const std::string image_diagonal,const std::string halo,int halo_x,int halo_y)
{
	add_frame(begin_at, unit_frame(image,image_diagonal,begin_at,end_at,0,0.0,ftofxp(1),halo,halo_x,halo_y));
	if (end_at != begin_at) {
		add_frame(end_at);
	}
}

unit_animation::unit_animation(const std::string image, const std::string halo,int halo_x,int halo_y)
{
	int duration =0;
	const std::vector<std::pair<std::string,int> > halos = unit_frame::prepare_halo(halo,0,0);
	std::vector<std::pair<std::string,int> >::const_iterator cur_halo;
	for(cur_halo = halos.begin() ; cur_halo != halos.end() ; cur_halo++) {
		duration += cur_halo->second;
	}
	duration = maximum<int>(200,duration);
	add_frame(0, unit_frame(image,"",0,duration,0,0.0,ftofxp(1),halo,halo_x,halo_y));
	if (duration != 0) {
		add_frame(duration);
	}
}

int unit_animation::matches(const std::string &terrain,const gamemap::location::DIRECTION dir) const
{
	int result = 0;
	if(terrain_types.empty()== false) {
		if (std::find(terrain_types.begin(),terrain_types.end(),terrain)== terrain_types.end()) {
			return -1;
		} else {
			result ++;
		}
	}

	if(directions.empty()== false) {
		if (std::find(directions.begin(),directions.end(),dir)== directions.end()) {
			return -1;
		} else {
			result ++;
		}
	}

	return result;
}

fighting_animation::fighting_animation(const config& cfg) :unit_animation(cfg),  range(utils::split(cfg["range"])),
  damage_type(utils::split(cfg["damage_type"])), special(utils::split(cfg["attack_special"]))
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
		if(*hit == "kill" ) {
			hits.push_back(KILL);
		}
	}
}


int fighting_animation::matches(const std::string &terrain,gamemap::location::DIRECTION dir,hit_type hit,const attack_type* attack) const
{
	int result = unit_animation::matches(terrain,dir);
	if(!attack) {
		if(damage_type.empty() && special.empty())
			return result;
		else
			return -1;
	}
	if(hits.empty() == false ) {
		if (std::find(hits.begin(),hits.end(),hit)== hits.end()) {
			return -1;
		} else {
			result ++;
		}
	}
	if(range.empty()== false) {
		if (std::find(range.begin(),range.end(),attack->range())== range.end()) {
			return -1;
		} else {
			result ++;
		}
	}

	if(damage_type.empty()== false) {
		if (std::find(damage_type.begin(),damage_type.end(),attack->type())== damage_type.end()) {
			return -1;
		} else {
			result ++;
		}
	}

	if(special.empty()== false) {
		bool found = false;
		std::vector<std::string> at_specials = utils::split(attack->weapon_specials(true));
		for(std::vector<std::string>::const_iterator sp_it = special.begin(); sp_it != special.end(); ++sp_it) {
			if (std::find(at_specials.begin(),at_specials.end(),*sp_it) != at_specials.end()) {
				result ++;
				found = true;
			}
		}
		if(!found) {
			return -1;
		}
	}
	return result;
}


