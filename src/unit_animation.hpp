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
		unit_frame() : xoffset(0), halo_x(0), halo_y(0), begin_time(0), end_time(0),highlight_ratio(ftofxp(1)){}
		explicit unit_frame(const std::string& str, const std::string & diag ="",
				int begin=0,int end = 0,
				Uint32 blend_color = 0, double blend_rate = 0.0, 
				fixed_t highlight = ftofxp(1),
				std::string in_halo = "",int halox = 0,int haloy = 0) :
			xoffset(0), image(str),image_diagonal(diag),
			halo_x(halox), halo_y(haloy),
			begin_time(begin), end_time(end),
			blend_with(blend_color), blend_ratio(blend_rate),
			highlight_ratio(highlight)  {halo = prepare_halo(in_halo,begin,end);};
		explicit unit_frame(const config& cfg);

		// int start, end;
		int xoffset;
		std::string image;
		std::string image_diagonal;
		std::vector<std::pair<std::string,int> > halo;
		std::string sound;
		int halo_x, halo_y;
		int begin_time, end_time;
		Uint32 blend_with;
		double blend_ratio;
		fixed_t highlight_ratio;
		static std::vector<std::pair<std::string,int> > prepare_halo(const std::string & halo,int begin, int end);
};
class unit_animation:public animated<unit_frame> 
{
	public:

		unit_animation(){};
		explicit unit_animation(const config& cfg,const std::string frame_string ="frame");
		explicit unit_animation(const std::string image, int begin_at, int end_at, const std::string image_diagonal = "",const std::string halo="",int halo_x=0,int halo_y=0);
		explicit unit_animation(const std::string image);

		enum FRAME_DIRECTION { VERTICAL, DIAGONAL };

	private:
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
