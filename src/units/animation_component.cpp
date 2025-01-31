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

#include "units/animation_component.hpp"

#include "config.hpp"
#include "display.hpp"
#include "map/map.hpp"
#include "preferences/preferences.hpp"
#include "random.hpp"
#include "units/unit.hpp"
#include "units/types.hpp"

#include <set>

using namespace std::chrono_literals;

namespace
{
std::chrono::steady_clock::time_point get_next_idle_tick()
{
	if(!prefs::get().idle_anim()) {
		return std::chrono::steady_clock::time_point::max();
	}

	const double rate = std::pow(2.0, -prefs::get().idle_anim_rate() / 10.0);
	const int duration = randomness::rng::default_instance().get_random_int(20000, 39999) * rate;
	return get_current_animation_tick() + std::chrono::milliseconds{duration};
}
} // namespace

const unit_animation* unit_animation_component::choose_animation(const map_location& loc,const std::string& event,
		const map_location& second_loc,const int value,const strike_result::type hit,
		const const_attack_ptr& attack, const const_attack_ptr& second_attack, int swing_num)
{
	// Select one of the matching animations at random
	std::vector<const unit_animation*> options;
	int max_val = unit_animation::MATCH_FAIL;
	for(const unit_animation& anim : animations_) {
		int matching = anim.matches(loc,second_loc,u_.shared_from_this(),event,value,hit,attack,second_attack,swing_num);
		if(matching > unit_animation::MATCH_FAIL && matching == max_val) {
			options.push_back(&anim);
		} else if(matching > max_val) {
			max_val = matching;
			options.clear();
			options.push_back(&anim);
		}
	}

	if(max_val == unit_animation::MATCH_FAIL) {
		return nullptr;
	}
	return options[randomness::rng::default_instance().get_random_int(0, options.size()-1)];
}

void unit_animation_component::set_standing(bool with_bars)
{
	if (prefs::get().show_standing_animations()&& !u_.incapacitated()) {
		start_animation(std::chrono::milliseconds::max(), choose_animation(u_.loc_, "standing"),
			with_bars,  "", {0,0,0}, STATE_STANDING);
	} else {
		start_animation(std::chrono::milliseconds::max(), choose_animation(u_.loc_, "_disabled_"),
			with_bars,  "", {0,0,0}, STATE_STANDING);
	}
}

void unit_animation_component::set_ghosted(bool with_bars)
{
	start_animation(std::chrono::milliseconds::max(), choose_animation(u_.loc_, "_ghosted_"),
			with_bars);
	anim_->pause_animation();
}

void unit_animation_component::set_disabled_ghosted(bool with_bars)
{
	start_animation(std::chrono::milliseconds::max(), choose_animation(u_.loc_, "_disabled_ghosted_"),
			with_bars);
}

void unit_animation_component::set_idling()
{
	start_animation(std::chrono::milliseconds::max(), choose_animation(u_.loc_, "idling"),
		true, "", {0,0,0}, STATE_FORGET);
}

void unit_animation_component::set_selecting()
{
	if (prefs::get().show_standing_animations() && !u_.incapacitated()) {
		start_animation(std::chrono::milliseconds::max(), choose_animation(u_.loc_, "selected"),
			true, "", {0,0,0}, STATE_FORGET);
	} else {
		start_animation(std::chrono::milliseconds::max(), choose_animation(u_.loc_, "_disabled_selected_"),
			true, "", {0,0,0}, STATE_FORGET);
	}
}

void unit_animation_component::start_animation(const std::chrono::milliseconds& start_time, const unit_animation *animation,
	bool with_bars,  const std::string &text, color_t text_color, STATE state)
{
	if (!animation) {
		if (state == STATE_STANDING)
			state_ = state;
		if (!anim_ && state_ != STATE_STANDING)
			set_standing(with_bars);
		return ;
	}
	state_ = state;
	// everything except standing select and idle
	bool accelerate = (state != STATE_FORGET && state != STATE_STANDING);
	draw_bars_ =  with_bars;
	anim_.reset(new unit_animation(*animation));
	const auto real_start_time = start_time == std::chrono::milliseconds::max() ? anim_->get_begin_time() : start_time;
	anim_->start_animation(real_start_time, u_.loc_, u_.loc_.get_direction(u_.facing_),
		 text, text_color, accelerate);
	frame_begin_time_ = anim_->get_begin_time() - 1ms;
	next_idling_ = get_next_idle_tick();
}

void unit_animation_component::refresh()
{
	if (state_ == STATE_FORGET && anim_ && anim_->animation_finished_potential())
	{
		set_standing();
		return;
	}
	display &disp = *display::get_singleton();
	if (state_ != STATE_STANDING || get_current_animation_tick() < next_idling_ ||
	    !disp.tile_nearly_on_screen(u_.loc_) || u_.incapacitated())
	{
		return;
	}
	if (get_current_animation_tick() > next_idling_ + 1000ms)
	{
		// prevent all units animating at the same time
		next_idling_ = get_next_idle_tick();
	} else {
		set_idling();
	}
}

void unit_animation_component::clear_haloes ()
{
	unit_halo_.reset();
	abil_halos_.clear();
	abil_halos_ref_.clear();
	if(anim_ ) anim_->clear_haloes();
}

bool unit_animation_component::invalidate (const display & disp)
{
	bool result = false;

	// Very early calls, anim not initialized yet
	if(get_animation()) {
		frame_parameters params;
		const gamemap& map = disp.context().map();
		const t_translation::terrain_code terrain = map.get_terrain(u_.loc_);
		const terrain_type& terrain_info = map.get_terrain_info(terrain);

		int height_adjust = static_cast<int>(terrain_info.unit_height_adjust() * disp.get_zoom_factor());
		if (u_.is_flying() && height_adjust < 0) {
			height_adjust = 0;
		}
		params.y -= height_adjust;
		params.halo_y -= height_adjust;
		params.image_mod = u_.image_mods();
		params.halo_mod = u_.TC_image_mods();
		params.image= u_.default_anim_image();

		result |= get_animation()->invalidate(params);
	}

	return result;

}

void unit_animation_component::reset_after_advance(const unit_type * newtype)
{
	if (newtype) {
		animations_ = newtype->animations();
	}

	refreshing_ = false;
	anim_.reset();
}

void unit_animation_component::apply_new_animation_effect(const config & effect) {
	if(effect["id"].empty()) {
		unit_animation::add_anims(animations_, effect);
	} else {
		static std::map< std::string, std::vector<unit_animation>> animation_cache;
		std::vector<unit_animation> &built = animation_cache[effect["id"]];
		if(built.empty()) {
			unit_animation::add_anims(built, effect);
		}
		animations_.insert(animations_.end(),built.begin(),built.end());
	}
}

std::vector<std::string> unit_animation_component::get_flags() {
	std::set<std::string> result;
	for(const auto& anim : animations_) {
		const std::vector<std::string>& flags = anim.get_flags();
		std::copy_if(flags.begin(), flags.end(), std::inserter(result, result.begin()), [](const std::string& flag) {
			return !(flag.empty() || (flag.front() == '_' && flag.back() == '_'));
		});
	}
	return std::vector<std::string>(result.begin(), result.end());
}
