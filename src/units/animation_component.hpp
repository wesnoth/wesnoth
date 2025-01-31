/*
	Copyright (C) 2014 - 2024
	by Chris Beck <render787@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

// This class encapsulates the animation functionality of unit.

#pragma once

#include "halo.hpp"
#include "units/animation.hpp" //Note: only needed for enum

class config;
class unit;
class unit_type;

class unit_animation_component
{
public:
	/** States for animation. */
	enum STATE {
		STATE_STANDING,   /** anim must fit in a hex */
		STATE_FORGET,     /** animation will be automatically replaced by a standing anim when finished */
		STATE_ANIM};      /** normal anims */

	/** Default construct a unit animation component corresponding to a unit. */
	unit_animation_component(unit& my_unit)
		: u_(my_unit)
		, anim_(nullptr)
		, animations_()
		, state_(STATE_STANDING)
		, next_idling_()
		, frame_begin_time_(0)
		, draw_bars_(false)
		, refreshing_(false)
		, unit_halo_()
		, abil_halos_()
		, abil_halos_ref_()
	{
	}

	/** Copy construct a unit animation component, for use when copy constructing a unit. */
	unit_animation_component(unit& my_unit, const unit_animation_component& o)
		: u_(my_unit)
		, anim_(nullptr)
		, animations_(o.animations_)
		, state_(o.state_)
		, next_idling_()
		, frame_begin_time_(o.frame_begin_time_)
		, draw_bars_(o.draw_bars_)
		, refreshing_(o.refreshing_)
		, unit_halo_()
		, abil_halos_()
		, abil_halos_ref_()
	{
	}

	/** Chooses an appropriate animation from the list of known animations. */
	const unit_animation* choose_animation(
			const map_location& loc, const std::string& event,
			const map_location& second_loc = map_location::null_location(),
			const int damage=0,
			const strike_result::type hit_type = strike_result::type::invalid,
			const const_attack_ptr& attack=nullptr,const const_attack_ptr& second_attack = nullptr,
			int swing_num =0);

	/** Sets the animation state to standing. */
	void set_standing(bool with_bars = true);

	/** Sets the animation state to ghosted. (For use with whiteboard / planning mode.) */
	void set_ghosted(bool with_bars = true);

	/** Whiteboard related somehow. TODO: Figure out exactly what this does. */
	void set_disabled_ghosted(bool with_bars = true);

	/** Sets the animation state to idling. */
	void set_idling();

	/** Sets the animation state to that when the unit is selected */
	void set_selecting();

	/** Begin an animation. */
	void start_animation(const std::chrono::milliseconds& start_time, const unit_animation *animation,
		bool with_bars,  const std::string &text = "",
		color_t text_color = {}, STATE state = STATE_ANIM);

	/** Invalidates an animation with respect to a display object, preparing it for redraw. */
	bool invalidate(const display & disp);

	/** Intermittently activates the idling animations in place of the standing animations. Used by display object. */
	void refresh();

	/** Clear the haloes associated to the unit */
	void clear_haloes();

	/** Resets the animations list after the unit is advanced. */
	void reset_after_advance(const unit_type * newtype = nullptr);

	/** Adds an animation described by a config. Uses an internal cache to avoid redoing work. */
	void apply_new_animation_effect(const config & effect);

	/** Get a pointer to the current animation. */
	unit_animation* get_animation() const { return anim_.get(); }

	/** Get the flags of all registered animations. */
	std::vector<std::string> get_flags();

	friend class unit;
	friend class unit_drawer;
private:
	/** A reference to the unit that owns this object. It does so with a scoped pointer, so this reference should not dangle. */
	const unit & u_;

	/** The current animation. */
	std::unique_ptr<unit_animation> anim_;
	/** List of registered animations for this unit. */
	std::vector<unit_animation> animations_;

	/** animation state */
	STATE state_;

	/** time for next idle animation */
	std::chrono::steady_clock::time_point next_idling_;
	/** time for the frame to begin */
	std::chrono::milliseconds frame_begin_time_;

	/** bool indicating whether to draw bars with the unit */
	bool draw_bars_;
	/** avoid infinite recursion. flag used for drawing / animation */
	bool refreshing_;

	/** handle to the halo of this unit */
	halo::handle unit_halo_;
	/** handle to the abilities halos of this unit */
	std::vector<halo::handle> abil_halos_;
	/** vector used to check that halo_abilities vector isn't modified between each read */
	std::vector<std::string> abil_halos_ref_;
};
