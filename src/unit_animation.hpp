/* $Id: unit_types.hpp 9700 2006-01-15 12:00:53Z noyga $ */
/*
   Copyright (C) 2006 by Jeremy Rosen <jeremy.rosen@enst-bretagne.fr>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
   */
#ifndef UNIT_ANIMATION_H_INCLUDED
#define UNIT_ANIMATION_H_INCLUDED

#include "animated.hpp"
#include "map.hpp"
#include "config.hpp"
#include "util.hpp"
#include "serialization/string_utils.hpp"

#include <string>
#include <vector>

#include "unit_frame.hpp"

class unit_animation:public animated<unit_frame> 
{
	public:

		unit_animation(){};
		explicit unit_animation(const config& cfg,const std::string frame_string ="frame");
		explicit unit_animation(const std::string image, int begin_at, int end_at,
				const std::string image_diagonal = "",const std::string halo="",int halo_x=0,int halo_y=0);
		explicit unit_animation(const std::string image);
		int matches(const std::string &terrain,const gamemap::location::DIRECTION dir) const;

		enum FRAME_DIRECTION { VERTICAL, DIAGONAL };

	private:
		std::vector<std::string> terrain_types;
		std::vector<gamemap::location::DIRECTION> directions;
};

class attack_type;


class fighting_animation:public unit_animation
{
	public:
		typedef enum { HIT, MISS, KILL} hit_type;

		explicit fighting_animation(const config& cfg);
		explicit fighting_animation(const std::string &image, const std::string &range="",int begin_at = -150, int end_at = 150):
			unit_animation(image,begin_at,end_at),range(utils::split(range)) {};
		int matches(const std::string &terrain,gamemap::location::DIRECTION dir,hit_type hit,const attack_type* attack) const;

	private:
		std::vector<hit_type> hits;
		std::vector<std::string> range;
		std::vector<std::string> damage_type, special;
};

class defensive_animation:public fighting_animation 
{
	public:
		explicit defensive_animation(const config& cfg):fighting_animation(cfg){};
		explicit defensive_animation(const std::string &image, const std::string &range="",int begin_at = -150, int end_at = 150):fighting_animation(image,range,begin_at,end_at){};
};


class death_animation:public fighting_animation
{
	public:
		explicit death_animation(const config& cfg):fighting_animation(cfg){};
		explicit death_animation(const std::string &image):fighting_animation(image,"",0,10) {};
	private:
};


class movement_animation:public unit_animation
{
	public:
		explicit movement_animation(const config& cfg):unit_animation(cfg){};
		explicit movement_animation(const std::string& image):
			unit_animation(image,0,150){};

	private:
};


#endif
