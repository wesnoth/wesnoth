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
class attack_type;

class unit_animation
{
	public:
		typedef enum { MATCH_FAIL=-2 , DEFAULT_ANIM=-1};
		typedef enum { HIT, MISS, KILL, INVALID} hit_type;
		static void initialize_anims( std::vector<unit_animation> & animations, const config & cfg, std::vector<attack_type> tmp_attacks);

		int matches(const game_display &disp,const gamemap::location& loc,const unit* my_unit,const std::string & event="",const int value=0,hit_type hit=INVALID,const attack_type* attack=NULL,const attack_type* second_attack = NULL, int swing_num =0) const;


		const unit_frame& get_last_frame() const{ return unit_anim_.get_last_frame() ; };
		void add_frame(int duration, const unit_frame& value,bool force_change =false){ unit_anim_.add_frame(duration,value,force_change) ; };

		bool need_update() const;
		bool animation_finished() const;
		bool animation_would_finish() const;
		void update_last_draw_time();
		int get_begin_time() const;
		int get_end_time() const;
		int get_animation_time() const{ return unit_anim_.get_animation_time() ; };
		void start_animation(int start_time,const gamemap::location &src = gamemap::location::null_location, const gamemap::location &dst = gamemap::location::null_location , bool cycles=false, double acceleration=1);
		const int get_current_frame_begin_time() const{ return unit_anim_.get_current_frame_begin_time() ; };
		void redraw();

		// only to support all [attack_anim] format, to remove at 1.3.10 time
		void back_compat_add_name(const std::string name="",const std::string range ="");
		// to be privatized post 1.3.10
		static config prepare_animation(const config &cfg,const std::string animation_tag);
		explicit unit_animation(const config& cfg,const std::string frame_string ="");
	friend class unit;
	protected:
	// reserved to class unit, for the special case of redrawing the unit base frame
		image::locator image() const { return unit_anim_.get_current_frame().image() ; }
		image::locator image_diagonal() const { return unit_anim_.get_current_frame().image_diagonal() ; }
		std::string sound() const { return unit_anim_.get_current_frame().sound() ; };
		Uint32 blend_with() const{ return unit_anim_.get_current_frame().blend_with() ; };
		const std::string &halo(const std::string&default_val ="") const{ return unit_anim_.halo(default_val); };
		int halo_x(const int default_val = 0) const{ return unit_anim_.halo_x(default_val); };
		int halo_y(const int default_val = 0) const{ return unit_anim_.halo_y(default_val); };
		double blend_ratio(const double default_val = 0) const{ return unit_anim_.blend_ratio(default_val); };
		fixed_t highlight_ratio(const float default_val = 1.0) const{ return unit_anim_.highlight_ratio(default_val); };
		double offset(double default_val =0.0) const{ return unit_anim_.offset(default_val); };
	private:
		unit_animation(){};
		explicit unit_animation(int start_time,const unit_frame &frame,const std::string& even="",const int variation=0);
		class crude_animation:public animated<unit_frame>
	{
		public:
			explicit crude_animation(int start_time=0):animated<unit_frame>(start_time){};
			explicit crude_animation(const config& cfg,const std::string frame_string ="frame");
			virtual ~crude_animation();
			bool need_update() const;
			const std::string &halo(const std::string&default_val ="") const;
			int halo_x(const int default_val = 0) const;
			int halo_y(const int default_val = 0) const;
			double blend_ratio(const double default_val = 0) const;
			fixed_t highlight_ratio(const float default_val = 1.0) const;
			double offset(double default_val =0.0) const;
			void redraw( );
			void start_animation(int start_time,const gamemap::location& src,const  gamemap::location& dst, bool cycles=false, double acceleration=1);
		private:

			//animation params that can be locally overridden by frames
			progressive_double offset_;
			progressive_string halo_;
			progressive_int halo_x_;
			progressive_int halo_y_;
			Uint32 blend_with_;
			progressive_double blend_ratio_;
			progressive_double highlight_ratio_;
			gamemap::location src_;
			gamemap::location dst_;
			int halo_id_;
                        int last_frame_begin_time_;

	};
		t_translation::t_list terrain_types_;
		std::vector<config> unit_filter_;
		std::vector<config> secondary_unit_filter_;
		std::vector<gamemap::location::DIRECTION> directions_;
		int frequency_;
		int base_score_;
		std::vector<std::string> event_;
		std::vector<int> value_;
		std::vector<config> primary_attack_filter_;
		std::vector<config> secondary_attack_filter_;
		std::vector<hit_type> hits_;
		std::vector<int> swing_num_;
		std::map<std::string,crude_animation> sub_anims_;
		crude_animation unit_anim_;
};


#endif
