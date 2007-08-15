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
		explicit unit_animation(int start_time,const unit_frame &frame);
		int matches(const game_display &disp,const gamemap::location& loc,const unit* my_unit) const;

	private:
		t_translation::t_list terrain_types_;
		std::vector<config> unit_filter_;
		std::vector<config> secondary_unit_filter_;
		std::vector<config> neighbour_unit_filter_;
		std::vector<gamemap::location::DIRECTION> directions;
		int frequency_;
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
		std::vector<int> damage_;
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

class standing_animation:public unit_animation
{
	public:
		explicit standing_animation(const config& cfg):unit_animation(cfg){};
		explicit standing_animation(int start_time,const unit_frame &frame):
			unit_animation(start_time,frame){};

	private:
};

class leading_animation:public unit_animation
{
	public:
		explicit leading_animation(const config& cfg):unit_animation(cfg){};
		explicit leading_animation(int start_time,const unit_frame &frame):
			unit_animation(start_time,frame){};

	private:
};

class healing_animation:public unit_animation
{
	public:
		explicit healing_animation(const config& cfg);
		explicit healing_animation(int start_time,const unit_frame &frame):
			unit_animation(start_time,frame){};
		int matches(const game_display &disp,const gamemap::location& loc,const unit* my_unit,int damage) const;

	private:
		std::vector<int> damage_;

};

class victory_animation:public fighting_animation
{
	public:
		explicit victory_animation(const config& cfg):fighting_animation(cfg){};
		explicit victory_animation(int start_time,const unit_frame &frame):
			fighting_animation(start_time,frame){};

	private:
};

class recruit_animation:public unit_animation
{
	public:
		explicit recruit_animation(const config& cfg):unit_animation(cfg){};
		explicit recruit_animation(int start_time,const unit_frame &frame):
			unit_animation(start_time,frame){};

	private:
};

class idle_animation:public unit_animation
{
	public:
		explicit idle_animation(const config& cfg):unit_animation(cfg){};
		explicit idle_animation(int start_time,const unit_frame &frame):
			unit_animation(start_time,frame){};

	private:
};

class levelin_animation:public unit_animation
{
	public:
		explicit levelin_animation(const config& cfg):unit_animation(cfg){};
		explicit levelin_animation(int start_time,const unit_frame &frame):
			unit_animation(start_time,frame){};

	private:
};

class levelout_animation:public unit_animation
{
	public:
		explicit levelout_animation(const config& cfg):unit_animation(cfg){};
		explicit levelout_animation(int start_time,const unit_frame &frame):
			unit_animation(start_time,frame){};

	private:
};

class poison_animation:public unit_animation
{
	public:
		explicit poison_animation(const config& cfg);
		explicit poison_animation(int start_time,const unit_frame &frame):
			unit_animation(start_time,frame){};
		int matches(const game_display &disp,const gamemap::location& loc,const unit* my_unit,int damage) const;

	private:
		std::vector<int> damage_;
};

class healed_animation:public unit_animation
{
	public:
		explicit healed_animation(const config& cfg);
		explicit healed_animation(int start_time,const unit_frame &frame):
			unit_animation(start_time,frame){};
		int matches(const game_display &disp,const gamemap::location& loc,const unit* my_unit,int healing) const;

	private:
		std::vector<int> healing_;
};


#endif
