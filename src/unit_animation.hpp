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


class game_display;

class unit_animation:public animated<unit_frame>
{
	public:
		static config prepare_animation(const config &cfg,const std::string animation_tag);

		unit_animation(){};
		explicit unit_animation(const config& cfg,const std::string frame_string ="frame");
		explicit unit_animation(int start_time,const unit_frame &frame,const std::string& even="",const int variation=0);
		int matches(const game_display &disp,const gamemap::location& loc,const unit* my_unit,const int value=0,const std::string & event="") const;

	private:
		t_translation::t_list terrain_types_;
		std::vector<config> unit_filter_;
		std::vector<config> secondary_unit_filter_;
		std::vector<config> neighbour_unit_filter_;
		std::vector<gamemap::location::DIRECTION> directions;
		int frequency_;
		int base_score_;
		std::vector<std::string> event_;
		std::vector<int> value_;
};

class attack_type;


class fighting_animation:public unit_animation
{
	public:
		typedef enum { HIT, MISS, KILL} hit_type;

		explicit fighting_animation(const config& cfg);
		explicit fighting_animation(int start_time,const unit_frame &frame):
			unit_animation(start_time,frame) {};
		int matches(const game_display &disp,const gamemap::location& loc,const unit* my_unit,hit_type hit,const attack_type* attack,const attack_type* second_attack, int swing_num,int damage) const;

	private:
		std::vector<config> primary_filter;
		std::vector<config> secondary_filter;
		std::vector<hit_type> hits;
		std::vector<int> swing_num;
};

class defensive_animation:public fighting_animation
{
	public:
		explicit defensive_animation(const config& cfg):fighting_animation(cfg){};
		explicit defensive_animation(int start_time,const unit_frame &frame):fighting_animation(start_time,frame){};
};


class death_animation:public fighting_animation
{
	public:
		explicit death_animation(const config& cfg):fighting_animation(cfg){};
		explicit death_animation(int start_time,const unit_frame &frame):fighting_animation(start_time,frame) {};
	private:
};

class attack_animation: public fighting_animation
{
	public:
		explicit attack_animation(const config& cfg):fighting_animation(cfg),missile_anim(cfg,"missile_frame"){};
		explicit attack_animation(int start_time,const unit_frame &frame):fighting_animation(start_time,frame) {};
		const unit_animation &get_missile_anim() {return missile_anim;}
	private:
		unit_animation missile_anim;

};

class movement_animation:public unit_animation
{
	public:
		explicit movement_animation(const config& cfg):unit_animation(cfg){};
		explicit movement_animation(int start_time,const unit_frame &frame):
			unit_animation(start_time,frame){};

	private:
};



class victory_animation:public fighting_animation
{
	public:
		explicit victory_animation(const config& cfg):fighting_animation(cfg){};
		explicit victory_animation(int start_time,const unit_frame &frame):
			fighting_animation(start_time,frame){};

	private:
};

#endif
