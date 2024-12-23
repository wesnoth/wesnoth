/*
	Copyright (C) 2006 - 2024
	by Jeremy Rosen <jeremy.rosen@enst-bretagne.fr>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "color.hpp"
#include "config.hpp"
#include "halo.hpp"
#include "units/frame.hpp"
#include "units/ptr.hpp"
#include "units/strike_result.hpp"

class unit_animation
{
public:
	unit_animation() = delete;
	explicit unit_animation(const config& cfg, const std::string& frame_string = "");

	enum variation_type {MATCH_FAIL = -10 , DEFAULT_ANIM = -9};

	static void fill_initial_animations(std::vector<unit_animation>& animations, const config& cfg);
	static void add_anims(std::vector<unit_animation>& animations, const config& cfg);

	int matches(const map_location& loc, const map_location& second_loc, const unit_const_ptr& my_unit, const std::string& event = "",
		const int value = 0, strike_result::type hit = strike_result::type::invalid, const const_attack_ptr& attack = nullptr, const const_attack_ptr& second_attack = nullptr,
		int value2 = 0) const;

	const unit_frame& get_last_frame() const
	{
		return unit_anim_.get_last_frame();
	}

	void add_frame(const std::chrono::milliseconds& duration, const unit_frame& value, bool force_change = false)
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
	std::chrono::milliseconds get_begin_time() const;
	std::chrono::milliseconds get_end_time() const;

	auto time_to_tick(const std::chrono::milliseconds& animation_time) const
	{
		return unit_anim_.time_to_tick(animation_time);
	}

	auto get_animation_time() const
	{
		return unit_anim_.get_animation_time();
	}

	void set_max_animation_time(const std::chrono::milliseconds& time)
	{
		unit_anim_.set_max_animation_time(time);
	}

	auto get_animation_time_potential() const
	{
		return unit_anim_.get_animation_time_potential();
	}

	void start_animation(const std::chrono::milliseconds& start_time
		, const map_location& src = map_location::null_location()
		, const map_location& dst = map_location::null_location()
		, const std::string& text = ""
		, const color_t text_color = {0,0,0}
		, const bool accelerate = true);

	void update_parameters(const map_location& src, const map_location& dst);
	void pause_animation();
	void restart_animation();
	auto get_current_frame_begin_time() const
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
	frame_parameters get_current_params(const frame_parameters& default_val = frame_parameters()) const
	{
		return unit_anim_.parameters(default_val);
	}

private:
	explicit unit_animation(const std::chrono::milliseconds& start_time
		, const unit_frame &frame
		, const std::string& event = ""
		, const int variation=DEFAULT_ANIM
		, const frame_builder & builder = frame_builder());

	class particle : public animated<unit_frame>
	{
	public:
		explicit particle(const std::chrono::milliseconds& start_time = std::chrono::milliseconds{0}, const frame_builder& builder = frame_builder())
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
		void override(const std::chrono::milliseconds& start_time
			, const std::chrono::milliseconds& duration
			, const cycle_state cycles
			, const std::string& highlight = ""
			, const std::string& blend_ratio =""
			, color_t blend_color = {0,0,0}
			, const std::string& offset = ""
			, const std::string& layer = ""
			, const std::string& modifiers = "");
		void redraw(const frame_parameters& value, const map_location& src, const map_location& dst, halo::manager& halo_man);
		std::set<map_location> get_overlaped_hex(const frame_parameters& value,const map_location& src, const map_location& dst);
		void start_animation(const std::chrono::milliseconds& start_time);
		frame_parameters parameters(const frame_parameters& default_val) const
		{
			return get_current_frame().merge_parameters(get_current_frame_time(), parameters_.parameters(get_animation_time() - get_begin_time()), default_val);
		}
		void clear_halo();
		bool accelerate;

	private:
		//animation params that can be locally overridden by frames
		frame_parsed_parameters parameters_;
		halo::handle halo_id_;
		std::chrono::milliseconds last_frame_begin_time_;
		bool cycles_;
	};

	t_translation::ter_list terrain_types_;
	std::vector<config> unit_filter_;
	std::vector<config> secondary_unit_filter_;
	std::vector<map_location::direction> directions_;
	int frequency_;
	int base_score_;
	std::vector<std::string> event_;
	std::vector<int> value_;
	std::vector<config> primary_attack_filter_;
	std::vector<config> secondary_attack_filter_;
	std::vector<strike_result::type> hits_;
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
	void add_animation(unit_const_ptr animated_unit
		, const unit_animation* animation
		, const map_location& src = map_location::null_location()
		, bool with_bars = false
		, const std::string& text = ""
		, const color_t text_color = {0,0,0});

	void add_animation(unit_const_ptr animated_unit
		, const std::string& event
		, const map_location& src = map_location::null_location()
		, const map_location& dst = map_location::null_location()
		, const int value = 0
		, bool with_bars = false
		, const std::string& text = ""
		, const color_t text_color = {0,0,0}
		, const strike_result::type hit_type = strike_result::type::invalid
		, const const_attack_ptr& attack = nullptr
		, const const_attack_ptr& second_attack = nullptr
		, int value2 = 0);

	/** has_animation : return an boolean value if animated unit present and have animation specified, used for verify prensence of [leading_anim] or [resistance_anim] for playability of [teaching_anim]
	 * @return True if the  @a animated_unit is present and have animation.
	 * @param animated_unit the unit who is checked.
	 * @param event the animation who is checked([leading_anim] or [resistance_anim].
	 * @param src the location of animated_unit.
	 * @param dst location of unit student(attacker or defender).
	 * @param value value of damage.
	 * @param hit_type type of damage inflicted.
	 * @param attack weapon used by student.
	 * @param second_attack weapon used by opponent.
	 * @param value2 i don't understand myself.but this value is used in choose_animation.
	 */
	bool has_animation(const unit_const_ptr& animated_unit
		, const std::string& event
		, const map_location& src = map_location::null_location()
		, const map_location& dst = map_location::null_location()
		, const int value = 0
		, const strike_result::type hit_type = strike_result::type::invalid
		, const const_attack_ptr& attack = nullptr
		, const const_attack_ptr& second_attack = nullptr
		, int value2 = 0) const;

	void replace_anim_if_invalid(const unit_const_ptr& animated_unit
		, const std::string& event
		, const map_location& src = map_location::null_location()
		, const map_location& dst = map_location::null_location()
		, const int value = 0
		, bool with_bars = false
		, const std::string& text = ""
		, const color_t text_color = {0,0,0}
		, const strike_result::type hit_type = strike_result::type::invalid
		, const const_attack_ptr& attack = nullptr
		, const const_attack_ptr& second_attack = nullptr
		, int value2 = 0);
	void start_animations();
	void pause_animation();
	void restart_animation();

	void clear()
	{
		start_time_ = std::chrono::milliseconds::min();
		animated_units_.clear();
	}

	void set_all_standing();

	bool would_end() const;
	std::chrono::milliseconds get_animation_time() const;
	std::chrono::milliseconds get_animation_time_potential() const;
	std::chrono::milliseconds get_end_time() const;
	void wait_for_end() const;
	void wait_until(const std::chrono::milliseconds& animation_time) const;

private:
	struct anim_elem
	{
		unit_const_ptr my_unit;
		const unit_animation* animation = nullptr;
		std::string text;
		color_t text_color;
		map_location src;
		bool with_bars = false;
	};

	std::vector<anim_elem> animated_units_;
	std::chrono::milliseconds start_time_ = std::chrono::milliseconds::min();
};
