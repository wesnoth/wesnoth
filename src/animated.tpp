/*
	Copyright (C) 2005 - 2024
	by Guillaume Melquiond <guillaume.melquiond@gmail.com>
	Copyright (C) 2004 by Philippe Plantier <ayin@anathas.org>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/**
 * @file animated.tpp
 * Templates related to animations.
 */

template<typename T>
const T animated<T>::void_value_ = T();

template<typename T>
inline animated<T>::animated(const std::chrono::milliseconds& start_time)
	: starting_frame_time_(start_time)
	, does_not_change_(true)
	, started_(false)
	, force_next_update_(false)
	, frames_()
	, max_animation_time_(0)
	, start_tick_()
	, cycles_(false)
	, acceleration_(1)
	, last_update_tick_()
	, current_frame_key_(0)
{
}

template<typename T>
inline animated<T>::animated(const animated<T>::anim_description& cfg, const std::chrono::milliseconds& start_time, bool force_change)
	: starting_frame_time_(start_time)
	, does_not_change_(true)
	, started_(false)
	, force_next_update_(false)
	, frames_()
	, max_animation_time_(0)
	, start_tick_()
	, cycles_(false)
	, acceleration_(1)
	, last_update_tick_()
	, current_frame_key_(0)
{
	for(const auto& [duration, value] : cfg) {
		add_frame(duration, value, force_change);
	}
}

template<typename T>
inline void animated<T>::add_frame(const std::chrono::milliseconds& duration, const T& value, bool force_change)
{
	// NOTE: We cannot use emplace_back here, because the value may be a reference into the same vector,
	// which case emplace_back could invalidate it before the new frame is constructed.
	if(frames_.empty()) {
		does_not_change_ = !force_change;
		frames_.push_back(frame{duration, value, starting_frame_time_});
	} else {
		does_not_change_ = false;
		frames_.push_back(frame{duration, value, frames_.back().start_time_ + frames_.back().duration_});
	}
}

template<typename T>
inline void animated<T>::start_animation(const std::chrono::milliseconds& start_time, bool cycles)
{
	started_ = true;
	last_update_tick_ = get_current_animation_tick();
	acceleration_ = 1.0; // assume acceleration is 1, this will be fixed at first update_last_draw_time
	start_tick_ = last_update_tick_ + (starting_frame_time_ - start_time);
	cycles_ = cycles;
	current_frame_key_ = 0;
	force_next_update_ = !frames_.empty();
}

template<typename T>
inline void animated<T>::update_last_draw_time(double acceleration)
{
	if(acceleration > 0 && acceleration_ != acceleration) {
		auto tmp = tick_to_time(last_update_tick_);
		acceleration_ = acceleration;
		start_tick_ = last_update_tick_ + std::chrono::duration_cast<std::chrono::milliseconds>((starting_frame_time_ - tmp) / acceleration_);
	}

	if(!started_ && start_tick_ != std::chrono::steady_clock::time_point{}) {
		// animation is paused
		start_tick_ += get_current_animation_tick() - last_update_tick_;
	}

	last_update_tick_ = get_current_animation_tick();
	if(force_next_update_) {
		force_next_update_ = false;
		return;
	}

	if(does_not_change_) {
		return;
	}

	// Always update last_update_tick_, for the animation_time functions to work.
	if(!started_) {
		return;
	}

	if(frames_.empty()) {
		does_not_change_ = true;
		return;
	}

	if(cycles_) {
		while(get_animation_time() > get_end_time()) { // cut extra time
			start_tick_ += std::max(std::chrono::floor<std::chrono::milliseconds>(get_animation_duration() / acceleration_), std::chrono::milliseconds{1});
			current_frame_key_ = 0;
		}
	}

	const auto current_frame_end_time = get_current_frame_end_time();
	// catch up && don't go after the end
	if(current_frame_end_time < get_animation_time() && current_frame_end_time < get_end_time()) {
		current_frame_key_++;
	}
}

template<typename T>
inline bool animated<T>::not_started() const
{
	return !started_ && start_tick_ == std::chrono::steady_clock::time_point{};
}

template<typename T>
inline bool animated<T>::need_update() const
{
	if(force_next_update_) {
		return true;
	}

	if(does_not_change_) {
		return false;
	}

	if(frames_.empty()) {
		return false;
	}

	if(not_started()) {
		return false;
	}

	if(get_current_animation_tick() > std::chrono::floor<std::chrono::milliseconds>(get_current_frame_end_time() / acceleration_ + start_tick_)) {
		return true;
	}

	return false;
}

template<typename T>
inline bool animated<T>::animation_finished_potential() const
{
	if(frames_.empty()) {
		return true;
	}

	if(not_started()) {
		return true;
	}

	if(cycles_) {
		return true;
	}

	if(tick_to_time(get_current_animation_tick()) > get_end_time()) {
		return true;
	}

	return false;
}

template<typename T>
inline bool animated<T>::animation_finished() const
{
	if(frames_.empty()) {
		return true;
	}

	if(not_started()) {
		return true;
	}

	if(cycles_) {
		return true;
	}

	if(get_animation_time() > get_end_time()) {
		return true;
	}

	return false;
}

template<typename T>
inline std::chrono::milliseconds animated<T>::get_animation_time_potential() const
{
	if(not_started()) {
		return starting_frame_time_;
	}

	return tick_to_time(get_current_animation_tick());
}

template<typename T>
inline std::chrono::milliseconds animated<T>::get_animation_time() const
{
	if(not_started()) {
		return starting_frame_time_;
	}

	auto time = tick_to_time(last_update_tick_);
	if(time > max_animation_time_ && max_animation_time_ > std::chrono::milliseconds{0}) {
		return max_animation_time_;
	}
	return time;
}

template<typename T>
inline void animated<T>::set_animation_time(const std::chrono::milliseconds& time)
{
	start_tick_ = last_update_tick_ + std::chrono::floor<std::chrono::milliseconds>((starting_frame_time_ - time) / acceleration_);

	current_frame_key_ = 0;
	force_next_update_ = true;
}

template<typename T>
inline void animated<T>::set_max_animation_time(const std::chrono::milliseconds& time)
{
	max_animation_time_ = time;
}

template<typename T>
inline std::chrono::milliseconds animated<T>::get_animation_duration() const
{
	return get_end_time() - get_begin_time();
}

template<typename T>
inline const T& animated<T>::get_current_frame() const
{
	if(frames_.empty()) {
		return void_value_;
	}

	return frames_[current_frame_key_].value_;
}

template<typename T>
inline std::chrono::milliseconds animated<T>::get_current_frame_begin_time() const
{
	if(frames_.empty()) {
		return starting_frame_time_;
	}

	return frames_[current_frame_key_].start_time_;
}

template<typename T>
inline std::chrono::milliseconds animated<T>::get_current_frame_end_time() const
{
	if(frames_.empty()) {
		return starting_frame_time_;
	}

	return get_current_frame_begin_time() + get_current_frame_duration();
}

template<typename T>
inline std::chrono::milliseconds animated<T>::get_current_frame_duration() const
{
	if(frames_.empty()) {
		return std::chrono::milliseconds{0};
	}

	return frames_[current_frame_key_].duration_;
}

template<typename T>
inline std::chrono::milliseconds animated<T>::get_current_frame_time() const
{
	if(frames_.empty()) {
		return std::chrono::milliseconds{0};
	}

	// FIXME: get_animation_time() use acceleration but get_current_frame_begin_time() doesn't ?
	return std::max(std::chrono::milliseconds{0}, get_animation_time() - get_current_frame_begin_time());
}

template<typename T>
inline const T& animated<T>::get_first_frame() const
{
	if(frames_.empty()) {
		return void_value_;
	}

	return frames_[0].value_;
}

template<typename T>
inline const T& animated<T>::get_frame(std::size_t n) const
{
	if(n >= frames_.size()) {
		return void_value_;
	}

	return frames_[n].value_;
}

template<typename T>
inline const T& animated<T>::get_last_frame() const
{
	if(frames_.empty()) {
		return void_value_;
	}

	return frames_.back().value_;
}

template<typename T>
inline std::size_t animated<T>::get_frames_count() const
{
	return frames_.size();
}

template<typename T>
inline std::chrono::milliseconds animated<T>::get_begin_time() const
{
	return starting_frame_time_;
}

template<typename T>
inline std::chrono::steady_clock::time_point animated<T>::time_to_tick(const std::chrono::milliseconds& animation_time) const
{
	if(not_started()) {
		return {};
	}

	return start_tick_ + std::chrono::floor<std::chrono::milliseconds>((animation_time - starting_frame_time_) / acceleration_);
}

template<typename T>
inline std::chrono::milliseconds animated<T>::tick_to_time(const std::chrono::steady_clock::time_point& animation_tick) const
{
	if(not_started()) {
		return std::chrono::milliseconds{0};
	}

	return starting_frame_time_ + std::chrono::floor<std::chrono::milliseconds>((animation_tick - start_tick_) * acceleration_);
}

template<typename T>
inline std::chrono::milliseconds animated<T>::get_end_time() const
{
	if(frames_.empty()) {
		return starting_frame_time_;
	}

	return frames_.back().start_time_ + frames_.back().duration_;
}

template<typename T>
void animated<T>::remove_frames_until(const std::chrono::milliseconds& new_starting_time)
{
	while(starting_frame_time_ < new_starting_time && !frames_.empty()) {
		starting_frame_time_ += frames_[0].duration_;
		frames_.erase(frames_.begin());
	}
}

template<typename T>
inline void animated<T>::set_end_time(const std::chrono::milliseconds& new_ending_time)
{
	auto last_start_time = starting_frame_time_;
	auto current_frame = frames_.cbegin();
	while(last_start_time < new_ending_time && current_frame != frames_.cend()) {
		last_start_time += current_frame->duration_;
		++current_frame;
	}

	// at this point last_start_time is set to the beginning of the first frame past the end
	// or set to frames_.end()
	frames_.erase(current_frame, frames_.end());
	frames_.back().duration_ += new_ending_time - last_start_time;
}

template<typename T>
inline void animated<T>::set_begin_time(const std::chrono::milliseconds& new_begin_time)
{
	const auto variation = new_begin_time - starting_frame_time_;
	starting_frame_time_ += variation;
	for(auto& frame : frames_) {
		frame.start_time_ += variation;
	}
}
