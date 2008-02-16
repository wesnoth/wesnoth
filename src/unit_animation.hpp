/* $Id$ */
/*
   Copyright (C) 2006 - 2008 by Jeremy Rosen <jeremy.rosen@enst-bretagne.fr>
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
#include "config.hpp"
#include "map.hpp"
#include "unit_frame.hpp"

#include <climits>
#include <string>
#include <vector>

class game_display;
class attack_type;

class unit_animation
{		
		//! Shouldn't be used so only declared.
		unit_animation();
	public:
		typedef enum { MATCH_FAIL=-2 , DEFAULT_ANIM=-1};
		typedef enum { HIT, MISS, KILL, INVALID} hit_type;
		static void fill_initial_animations( std::vector<unit_animation> & animations, const config & cfg);
		static void add_anims( std::vector<unit_animation> & animations, const config & cfg);

		int matches(const game_display &disp,const gamemap::location& loc,const unit* my_unit,const std::string & event="",const int value=0,hit_type hit=INVALID,const attack_type* attack=NULL,const attack_type* second_attack = NULL, int swing_num =0) const;


		const unit_frame& get_last_frame() const{ return unit_anim_.get_last_frame() ; };
		void add_frame(int duration, const unit_frame& value,bool force_change =false){ unit_anim_.add_frame(duration,value,force_change) ; };

		bool need_update() const;
		bool animation_finished() const;
		bool animation_finished_potential() const;
		void update_last_draw_time();
		int get_begin_time() const;
		int get_end_time() const;
                int time_to_tick(int animation_time) const { return unit_anim_.time_to_tick(animation_time); };
		int get_animation_time() const{ return unit_anim_.get_animation_time() ; };
		int get_animation_time_potential() const{ return unit_anim_.get_animation_time_potential() ; };
		void start_animation(int start_time,const gamemap::location &src = gamemap::location::null_location, const gamemap::location &dst = gamemap::location::null_location , bool cycles=false, const std::string text="", const Uint32 text_color=0);
		const int get_current_frame_begin_time() const{ return unit_anim_.get_current_frame_begin_time() ; };
		void redraw();

	friend class unit;
	protected:
	// reserved to class unit, for the special case of redrawing the unit base frame
		image::locator image() const { return unit_anim_.get_current_frame().image() ; }
		image::locator image_diagonal() const { return unit_anim_.get_current_frame().image_diagonal() ; }
		std::string sound() const { return unit_anim_.get_current_frame().sound() ; };
		Uint32 blend_with(const Uint32 default_val=0) const{ return unit_anim_.blend_with(default_val) ; };
		const std::string &halo(const std::string&default_val ="") const{ return unit_anim_.halo(default_val); };
		int halo_x(const int default_val = 0) const{ return unit_anim_.halo_x(default_val); };
		int halo_y(const int default_val = 0) const{ return unit_anim_.halo_y(default_val); };
		double blend_ratio(const double default_val = 0.0) const{ return unit_anim_.blend_ratio(default_val); };
		fixed_t highlight_ratio(const float default_val = 1.0) const{ return unit_anim_.highlight_ratio(default_val); };
		public:
		double offset(double default_val =0.0) const{ return unit_anim_.offset(default_val); };
		std::pair<std::string,Uint32> text() const { return unit_anim_.text() ; };
	private:
		static config prepare_animation(const config &cfg,const std::string animation_tag);
		explicit unit_animation(const config& cfg,const std::string frame_string ="");
		explicit unit_animation(int start_time,const unit_frame &frame,const std::string& event="",const int variation=DEFAULT_ANIM);
		class crude_animation:public animated<unit_frame>
	{
		public:
			explicit crude_animation(int start_time=0) :
				animated<unit_frame>(start_time),
				accelerate(true),
				offset_(),
				halo_(),
				halo_x_(),
				halo_y_(),
				blend_with_(0),
				blend_ratio_(),
				highlight_ratio_(),
				src_(),
				dst_(),
				halo_id_(0),
				last_frame_begin_time_(0)
				{};
			explicit crude_animation(const config& cfg,const std::string frame_string ="frame");
			virtual ~crude_animation();
			bool need_update() const;
			void override(int start_time,const std::string highlight="", const std::string blend_ratio ="",Uint32 blend_color = 0,const std::string offset="");
			const std::string &halo(const std::string&default_val ="") const;
			int halo_x(const int default_val = 0) const;
			int halo_y(const int default_val = 0) const;
			double blend_ratio(const double default_val = 0.0) const;
			Uint32 blend_with(const Uint32 default_val = 0) const;
			fixed_t highlight_ratio(const float default_val = 1.0) const;
			double offset(double default_val =0.0) const;
			std::pair<std::string,Uint32> text() const ;
			void redraw( );
			void start_animation(int start_time,const gamemap::location& src,const  gamemap::location& dst, bool cycles=false);
			bool accelerate;
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

class unit_animator 
{
	public:
		unit_animator():start_time_(INT_MIN){};
		void add_animation(unit* animated_unit,const std::string& event,
				const gamemap::location &src = gamemap::location::null_location,
				const int value=0,bool with_bars = false,bool cycles = false,
				const std::string text="",const Uint32 text_color=0,
				const unit_animation::hit_type hit_type = unit_animation::INVALID,
				const attack_type* attack=NULL, const attack_type* second_attack = NULL,
				int swing_num =0);
		void replace_anim_if_invalid(unit* animated_unit,const std::string& event,
				const gamemap::location &src = gamemap::location::null_location,
				const int value=0,bool with_bars = false,bool cycles = false,
				const std::string text="",const Uint32 text_color=0,
				const unit_animation::hit_type hit_type = unit_animation::INVALID,
				const attack_type* attack=NULL, const attack_type* second_attack = NULL,
				int swing_num =0);
		void start_animations();
		void empty(){start_time_ = INT_MIN ; animated_units_.clear();};


		bool would_end() const;
		int get_animation_time() const;
		int get_animation_time_potential() const;
		int get_end_time() const;
		void wait_for_end() const;
		void wait_until( int animation_time) const;
	private:
		typedef struct {
			unit *my_unit;
			const unit_animation * animation;
			std::string text;
			Uint32 text_color;
			gamemap::location src;
			bool with_bars;
			bool cycles;
		} anim_elem;
		std::vector<anim_elem> animated_units_;
		int start_time_;
};
#endif
