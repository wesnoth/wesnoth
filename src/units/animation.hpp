/*
   Copyright (C) 2006 - 2017 by Jeremy Rosen <jeremy.rosen@enst-bretagne.fr>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "animated.hpp"
#include "config.hpp"
#include "halo.hpp"
#include "units/frame.hpp"
#include "units/ptr.hpp"
#include "utils/make_enum.hpp"

class display;
class unit;

class unit_animation
{
public:
	unit_animation() = delete;
	explicit unit_animation(const config& cfg, const std::string& frame_string = "");

	enum variation_type {MATCH_FAIL = -10 , DEFAULT_ANIM = -9};

	MAKE_ENUM(hit_type,
		(HIT, "hit")
		(MISS, "miss")
		(KILL, "kill")
		(INVALID, "invalid")
	)

	static void fill_initial_animations(std::vector<unit_animation>& animations, const config& cfg);
	static void add_anims(std::vector<unit_animation>& animations, const config& cfg);

	int matches(const display& disp, const map_location& loc, const map_location& second_loc, const unit* my_unit, const std::string& event = "", const int value = 0, hit_type hit = hit_type::INVALID, const_attack_ptr attack = nullptr, const_attack_ptr second_attack = nullptr, int value2 = 0) const;

	const unit_frame& get_last_frame() const
	{
		return unit_anim_.get_last_frame();
	}

	void add_frame(int duration, const unit_frame& value, bool force_change = false)
	{
		unit_anim_.add_frame(duration,value,force_change);
	}

	std::vector<std::string> get_flags() const
	{
		return event_;
	}

	bool need_update() const;
	bool need_minimal_update() const;
	bool animation_finished() const;
	bool animation_finished_potential() const;
	void update_last_draw_time();
	int get_begin_time() const;
	int get_end_time() const;

	int time_to_tick(int animation_time) const
	{
		return unit_anim_.time_to_tick(animation_time);
	}

	int get_animation_time() const
	{
		return unit_anim_.get_animation_time();
	}

	void set_max_animation_time(int time)
	{
		unit_anim_.set_max_animation_time(time);
	}

	int get_animation_time_potential() const
	{
		return unit_anim_.get_animation_time_potential();
	}

	void start_animation(int start_time
		, const map_location& src = map_location::null_location()
		, const map_location& dst = map_location::null_location()
		, const std::string& text = ""
		, const color_t text_color = {0,0,0}
		, const bool accelerate = true);

	void update_parameters(const map_location& src, const map_location& dst);
	void pause_animation();
	void restart_animation();
	int get_current_frame_begin_time() const
	{
		return unit_anim_.get_current_frame_begin_time();

	}
	void redraw(frame_parameters& value, halo::manager& halo_man);
	void clear_haloes();
	bool invalidate(frame_parameters& value );
	std::string debug() const;
	friend std::ostream& operator << (std::ostream& outstream, const unit_animation& u_animation);

	friend class unit;
	friend class unit_drawer;

protected:
	// reserved to class unit, for the special case of redrawing the unit base frame
	const frame_parameters get_current_params(const frame_parameters& default_val = frame_parameters()) const
	{
		return unit_anim_.parameters(default_val);
	}

private:
	explicit unit_animation(int start_time
		, const unit_frame &frame
		, const std::string& event = ""
		, const int variation=DEFAULT_ANIM
		, const frame_builder & builder = frame_builder());

	class particle : public animated<unit_frame>
	{
	public:
		explicit particle(int start_time = 0, const frame_builder& builder = frame_builder())
			: animated<unit_frame>(start_time)
			, accelerate(true)
			, parameters_(builder)
			, halo_id_()
			, last_frame_begin_time_(0)
			, cycles_(false)
		{}
		explicit particle(const config& cfg, const std::string& frame_string = "frame");

		virtual ~particle();
		bool need_update() const;
		bool need_minimal_update() const;
		enum cycle_state {UNSET, CYCLE, NO_CYCLE};
		void override(int start_time
			, int duration
			, const cycle_state cycles
			, const std::string& highlight = ""
			, const std::string& blend_ratio =""
			, color_t blend_color = {0,0,0}
			, const std::string& offset = ""
			, const std::string& layer = ""
			, const std::string& modifiers = "");
		void redraw(const frame_parameters& value, const map_location& src, const map_location& dst, halo::manager& halo_man);
		std::set<map_location> get_overlaped_hex(const frame_parameters& value,const map_location& src, const map_location& dst);
		void start_animation(int start_time);
		const frame_parameters parameters(const frame_parameters& default_val) const
		{
			return get_current_frame().merge_parameters(get_current_frame_time(), parameters_.parameters(get_animation_time() - get_begin_time()), default_val);
		}
		void clear_halo();
		bool accelerate;

	private:
		//animation params that can be locally overridden by frames
		frame_parsed_parameters parameters_;
		halo::handle halo_id_;
		int last_frame_begin_time_;
		bool cycles_;
	};

	t_translation::ter_list terrain_types_;
	std::vector<config> unit_filter_;
	std::vector<config> secondary_unit_filter_;
	std::vector<map_location::DIRECTION> directions_;
	int frequency_;
	int base_score_;
	std::vector<std::string> event_;
	std::vector<int> value_;
	std::vector<config> primary_attack_filter_;
	std::vector<config> secondary_attack_filter_;
	std::vector<hit_type> hits_;
	std::vector<int> value2_;
	std::map<std::string,particle> sub_anims_;
	particle unit_anim_;
	/* these are drawing parameters, but for efficiency reason they are in the anim and not in the particle */
	map_location src_;
	map_location dst_;
	// optimization
	bool invalidated_;
	bool play_offscreen_;
	std::set<map_location> overlaped_hex_;
};

class unit_animator
{
public:
	unit_animator() :
		animated_units_(),
		start_time_(INT_MIN)
	{}

	void add_animation(const unit* animated_unit
		, const unit_animation* animation
		, const map_location& src = map_location::null_location()
		, bool with_bars = false
		, const std::string& text = ""
		, const color_t text_color = {0,0,0});

	void add_animation(const unit* animated_unit
		, const std::string& event
		, const map_location& src = map_location::null_location()
		, const map_location& dst = map_location::null_location()
		, const int value = 0
		, bool with_bars = false
		, const std::string& text = ""
		, const color_t text_color = {0,0,0}
		, const unit_animation::hit_type hit_type =
			unit_animation::hit_type::INVALID
		, const_attack_ptr attack = nullptr
		, const_attack_ptr second_attack = nullptr
		, int value2 = 0);

	void replace_anim_if_invalid(const unit* animated_unit
		, const std::string& event
		, const map_location& src = map_location::null_location()
		, const map_location& dst = map_location::null_location()
		, const int value = 0
		, bool with_bars = false
		, const std::string& text = ""
		, const color_t text_color = {0,0,0}
		, const unit_animation::hit_type hit_type = unit_animation::hit_type::INVALID
		, const_attack_ptr attack = nullptr
		, const_attack_ptr second_attack = nullptr
		, int value2 = 0);
	void start_animations();
	void pause_animation();
	void restart_animation();

	void clear()
	{
		start_time_ = INT_MIN;
		animated_units_.clear();
	}

	void set_all_standing();

	bool would_end() const;
	int get_animation_time() const;
	int get_animation_time_potential() const;
	int get_end_time() const;
	void wait_for_end() const;
	void wait_until( int animation_time) const;

private:
	struct anim_elem
	{
		anim_elem()
			: my_unit(0)
			, animation(0)
			, text()
			, text_color()
			, src()
			, with_bars(false)
		{}

		unit_const_ptr my_unit;
		const unit_animation * animation;
		std::string text;
		color_t text_color;
		map_location src;
		bool with_bars;
	};

	std::vector<anim_elem> animated_units_;
	int start_time_;
};
