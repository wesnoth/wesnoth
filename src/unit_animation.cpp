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
	halo = cfg["halo"];
	halo_x = atoi(cfg["halo_x"].c_str());
	halo_y = atoi(cfg["halo_y"].c_str());
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

	range = cfg.child_range("sound");
	for(; range.first != range.second; ++range.first){
		sfx sound;
		sound.time = atoi((**range.first)["time"].c_str());
		sound.on_hit = (**range.first)["sound"];
		sound.on_miss = (**range.first)["sound_miss"];
		if(sound.on_miss.empty())
			sound.on_miss = sound.on_hit;

		if(sound.on_miss == "null")
			sound.on_miss = "";

		sfx_.push_back(sound);
	}
}

unit_animation::unit_animation(const std::string image, int begin_at, int end_at, const std::string image_diagonal)
{
	add_frame(begin_at, unit_frame(image,image_diagonal));
	add_frame(end_at);
}


const std::vector<unit_animation::sfx>& unit_animation::sound_effects() const
{
	return sfx_;
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
		if (std::find(special.begin(),special.end(),attack->special())== special.end()) {
			return -1;
		} else {
			result ++;
		}
	}

	return result;
}

movement_animation::movement_animation(const config& cfg)
:unit_animation(cfg), terrain_types(utils::split(cfg["terrain_type"]))
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

