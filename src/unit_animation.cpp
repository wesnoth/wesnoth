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
#include <cstdlib>
#include <iostream>

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
unit_animation::unit_animation(const config& cfg,const std::string frame_string )
{
	config::const_child_itors range = cfg.child_range(frame_string);

	int last_end = INT_MIN;
	for(; range.first != range.second; ++range.first) {
		add_frame(atoi((**range.first)["begin"].c_str()), unit_frame(**range.first));
		last_end = maximum<int>(atoi((**range.first)["end"].c_str()), last_end);
	}
	add_frame(last_end);

	/* warn on deprecated WML */
	if(cfg.child("sound")) {
		LOG_STREAM(err, config) << "an animation uses the deprecated [sound] tag, please include sound in the [frame] tag\n";

	}
}

unit_animation::unit_animation(const std::string image, int begin_at, int end_at, const std::string image_diagonal,const std::string halo,int halo_x,int halo_y)
{
	add_frame(begin_at, unit_frame(image,image_diagonal,begin_at,end_at,0,0.0,ftofxp(1),halo,halo_x,halo_y));
	add_frame(end_at);
}

defensive_animation::defensive_animation(const config& cfg) :unit_animation(cfg), hits(HIT_OR_MISS), range(utils::split(cfg["range"]))
{
	const std::string& hits_str = cfg["hits"];
	if(hits_str.empty() == false) {
		hits = (hits_str == "yes") ? HIT : MISS;
	}
}


int defensive_animation::matches(bool h, std::string r) const
{
	int result = 0;
	if(hits != HIT_OR_MISS ) {
		if(h && (hits == HIT)) {
			result++;
		} else if(!h && (hits == MISS)) {
			result++;
		} else {
			return -1;
		}
	}
	if(range.empty()== false) {
		if (std::find(range.begin(),range.end(),r)== range.end()) {
			return -1;
		} else {
			result ++;
		}
	}

	return result;
}

death_animation::death_animation(const config& cfg):unit_animation(cfg),
 damage_type(utils::split(cfg["damage_type"])), special(utils::split(cfg["attack_special"]))
{
}

int death_animation::matches(const attack_type* attack) const
{
	int result = 0;
	if(attack == NULL) {
		if(damage_type.empty() && special.empty())
			return 0;
		else
			return -1;
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

movement_animation::movement_animation(const config& cfg)
:unit_animation(cfg), terrain_types(utils::split(cfg["terrain"]))
{
	const std::vector<std::string>& my_directions = utils::split(cfg["direction"]);
	for(std::vector<std::string>::const_iterator i = my_directions.begin(); i != my_directions.end(); ++i) {
		const gamemap::location::DIRECTION d = gamemap::location::parse_direction(*i);
		directions.push_back(d);
	}
}

movement_animation::movement_animation(const std::string& image,const std::string& terrain,gamemap::location::DIRECTION dir):unit_animation(image,0,150),terrain_types(utils::split(terrain))
{
	if(dir !=gamemap::location::NDIRECTIONS) {
		directions.push_back(dir);
	}
}
int movement_animation::matches(const std::string &terrain,gamemap::location::DIRECTION dir) const
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

