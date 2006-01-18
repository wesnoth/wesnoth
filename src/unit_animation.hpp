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

//a class to describe a unit's animation sequence
struct unit_frame {
	unit_frame() : xoffset(0), halo_x(0), halo_y(0) {}
	explicit unit_frame(const std::string& str, const std::string & diag ="") : xoffset(0), image(str),image_diagonal(diag),
										    halo_x(0), halo_y(0) {};
	explicit unit_frame(const config& cfg);

	// int start, end;
	int xoffset;
	std::string image;
	std::string image_diagonal;
	std::string halo;
	int halo_x, halo_y;
};
class unit_animation:public animated<unit_frame> 
{
	public:

		unit_animation(){};
		explicit unit_animation(const config& cfg,const std::string frame_string ="frame");
		explicit unit_animation(const std::string image, int begin_at, int end_at, const std::string image_diagonal = "");

		enum FRAME_DIRECTION { VERTICAL, DIAGONAL };

		struct sfx {
			int time;
			std::string on_hit, on_miss;
		};

		const std::vector<sfx>& sound_effects() const;

	private:

		std::vector<sfx> sfx_;
};



class defensive_animation:public unit_animation
{
	public:
		typedef enum { HIT, MISS, HIT_OR_MISS } hit_type;

		explicit defensive_animation(const config& cfg);
		explicit defensive_animation(const std::string &image, const std::string &range="",const hit_type for_hit= HIT_OR_MISS): unit_animation(image,-150,150),hits(for_hit),range(utils::split(range)) {};
		int matches(bool hits, std::string range) const;

	private:
		hit_type hits;
		std::vector<std::string> range;
};


class attack_type;
class death_animation:public unit_animation
{
	public:
		explicit death_animation(const config& cfg);
		explicit death_animation(const std::string &image):unit_animation(image,0,10) {};
		int matches(const attack_type* attack) const;
	private:
		std::vector<std::string> damage_type, special;
};


class movement_animation:public unit_animation
{
	public:
		explicit movement_animation(const config& cfg);
		explicit movement_animation(const std::string& image,const std::string& terrain="",gamemap::location::DIRECTION dir=gamemap::location::NDIRECTIONS);
		int matches(const std::string &terrain,gamemap::location::DIRECTION dir=gamemap::location::NDIRECTIONS) const;

	private:
		std::vector<std::string> terrain_types;
		std::vector<gamemap::location::DIRECTION> directions;
};


#endif
