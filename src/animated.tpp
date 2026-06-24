/*
	Copyright (C) 2005 - 2025
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
 * Template implementation for frame-based animations.
 */


// ============================================================================
// Static Members
// ============================================================================

template<typename T>
const T animated<T>::void_value_ = T();


// ============================================================================
// Construction
// ============================================================================

template<typename T>
inline animated<T>::animated(const std::chrono::milliseconds& start_time)
	: starting_frame_time_(start_time)
	, max_animation_time_(0)
	, frames_()
	, current_frame_index_(0)
	, started_(false)
	, paused_(false)
	, finished_(false)
	, cycles_(false)
	, uses_acceleration_(false)
	, animation_start_tick_()
	, next_frame_tick_()
	, pause_tick_()
	, is_static_(true)
{
}

template<typename T>
inline animated<T>::animated(const animated<T>::anim_description& cfg, const std::chrono::milliseconds& start_time, bool force_change)
	: starting_frame_time_(start_time)
	, max_animation_time_(0)
	, frames_()
	, current_frame_index_(0)
	, started_(false)
	, paused_(false)
	, finished_(false)
	, cycles_(false)
	, uses_acceleration_(false)
	, animation_start_tick_()
	, next_frame_tick_()
	, pause_tick_()
	, is_static_(true)
{
	for(const auto& [duration, value] : cfg) {
		add_frame(duration, value, force_change);
	}
}


// ============================================================================
// Frame Management
// ============================================================================

template<typename T>
inline void animated<T>::add_frame(const std::chrono::milliseconds& duration, const T& value, bool force_change)
{
	// NOTE: We cannot use emplace_back here, because the value may be a reference into the same vector,
	// which case emplace_back could invalidate it before the new frame is constructed.

	if(!frames_.empty()) {
		const auto& last_frame = frames_.back();
		frames_.push_back(frame{duration, value, last_frame.start_time_ + last_frame.duration_});
		is_static_ = false;
	} else {
		is_static_ = !force_change;
		frames_.push_back(frame{duration, value, starting_frame_time_});
	}
}


// ============================================================================
// Playback Control
// ============================================================================

template<typename T>
inline void animated<T>::start_animation(const std::chrono::milliseconds& start_time_offset, bool cycles)
{
	started_ = true;
	paused_ = false;
	cycles_ = cycles;
	current_frame_index_ = 0;
	const auto current_tick = get_current_animation_tick(uses_acceleration_);
	animation_start_tick_ = current_tick - start_time_offset;
	if(!frames_.empty()) {
		next_frame_tick_ = animation_start_tick_ + frames_[0].duration_;
	}
}

template<typename T>
inline void animated<T>::pause_animation()
{
	if(!started_ || paused_) {
		return;
	}
	paused_ = true;
	pause_tick_ = get_current_animation_tick(uses_acceleration_);
}

template<typename T>
inline void animated<T>::unpause_animation()
{
	if(!paused_) {
		return;
	}

	const auto current_tick = get_current_animation_tick(uses_acceleration_);
	const auto pause_duration = current_tick - pause_tick_;

	// Shift the timeline forward by the pause duration
	animation_start_tick_ += pause_duration;
	next_frame_tick_ += pause_duration;

	paused_ = false;
	pause_tick_ = {};
}

template<typename T>
inline void animated<T>::advance_to_current_frame()
{
	const auto current_tick = get_current_animation_tick(uses_acceleration_);

	// Fast-forward if the animation is way behind
	// This handles cases where many full cycles have elapsed (e.g., when the animation is un-updated offscreen)
	if(current_tick > (next_frame_tick_ + std::chrono::milliseconds{200})) {
		const auto animation_duration = get_animation_duration();
		if(current_tick > (next_frame_tick_ + animation_duration)) {
			if(cycles_) {
				// Calculate total elapsed time and skip forward in whole cycles
				const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_tick - next_frame_tick_);
				next_frame_tick_ += (elapsed_ms / animation_duration) * animation_duration; // The int division truncates to whole cycles.
			} else {
				// Non-looping animation finished, stay on last frame
				current_frame_index_ = frames_.size() - 1;
				finished_ = true;
				return;
			}
		}
	}

	// Advance through frames until we reach the current time
	while(current_tick >= next_frame_tick_) {
		// Check if we're on the last frame
		if(current_frame_index_ >= frames_.size() - 1) {
			if(cycles_) {
				// Loop back to first frame
				current_frame_index_ = 0;
				next_frame_tick_ += frames_[0].duration_;
			} else {
				// Non-looping animation finished, stay on last frame
				finished_ = true;
				return;
			}
		} else {
			// Move to next frame
			current_frame_index_++;
			next_frame_tick_ += frames_[current_frame_index_].duration_;
		}
	}
}


// ============================================================================
// Playback State Queries
// ============================================================================

template<typename T>
inline bool animated<T>::need_update() const
{
	if(is_static_ || !started_ || paused_ || finished_ || frames_.empty()) {
		return false;
	}
	const auto current_tick = get_current_animation_tick(uses_acceleration_);
	return current_tick >= next_frame_tick_;
}

template<typename T>
inline bool animated<T>::animation_finished() const
{
	if(finished_ || frames_.empty() || !started_ || cycles_ || (get_elapsed_time() >= get_end_time())) {
		return true;
	}
	return false;
}


// ============================================================================
// Time Queries and Manipulation
// ============================================================================

template<typename T>
inline std::chrono::milliseconds animated<T>::get_elapsed_time() const
{
	if(!started_) {return starting_frame_time_;}

	// Calculate elapsed time since animation started
	const auto current_tick = get_current_animation_tick(uses_acceleration_);
	auto elapsed = current_tick - animation_start_tick_;
	auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);

	// Apply maximum time cap if set
	if(max_animation_time_ > std::chrono::milliseconds{0} && elapsed_ms > max_animation_time_) {
		return max_animation_time_;
	}

	return elapsed_ms;
}

template<typename T>
inline void animated<T>::apply_time_offset(const std::chrono::milliseconds& time)
{
	// Adjust the anchor point to make the current tick correspond to the requested time offset
	animation_start_tick_ -= time;

	// Restart animation from beginning; advance_to_current_frame() will sync on next call
	if(!frames_.empty()) {
		current_frame_index_ = 0;
		next_frame_tick_ = animation_start_tick_ + frames_[0].duration_;
	}
}

template<typename T>
inline void animated<T>::set_duration_limit(const std::chrono::milliseconds& time)
{
	max_animation_time_ = time;
}

template<typename T>
inline std::chrono::milliseconds animated<T>::get_animation_duration() const
{
    return get_end_time() - get_begin_time();
}

template<typename T>
inline std::chrono::milliseconds animated<T>::get_begin_time() const
{
	return starting_frame_time_;
}

template<typename T>
inline std::chrono::milliseconds animated<T>::get_end_time() const
{
	if(frames_.empty()) {
		return starting_frame_time_;
	}
	const auto& last_frame = frames_.back();
	return last_frame.start_time_ + last_frame.duration_;
}

template<typename T>
inline void animated<T>::set_begin_time(const std::chrono::milliseconds& new_begin_time)
{
	const auto shift = new_begin_time - starting_frame_time_;
	starting_frame_time_ = new_begin_time;
	for(auto& frame : frames_) {
		frame.start_time_ += shift;
	}
}

template<typename T>
inline std::chrono::steady_clock::time_point animated<T>::time_to_tick(const std::chrono::milliseconds& animation_time) const
{
	if(!started_) {
		return {};
	}
	return animation_start_tick_ + animation_time;
}


// ============================================================================
// Frame Queries
// ============================================================================

template<typename T>
inline const T& animated<T>::get_current_frame() const
{
	if(frames_.empty()) {
		return void_value_;
	}
	return frames_[current_frame_index_].value_;
}

template<typename T>
inline std::chrono::milliseconds animated<T>::get_current_frame_begin_time() const
{
	if(frames_.empty()) {
		return starting_frame_time_;
	}
	return frames_[current_frame_index_].start_time_;
}

template<typename T>
inline std::chrono::milliseconds animated<T>::get_current_frame_end_time() const
{
	if(frames_.empty()) {
		return starting_frame_time_;
	}
	const auto& frame = frames_[current_frame_index_];
	return frame.start_time_ + frame.duration_;
}

template<typename T>
inline std::chrono::milliseconds animated<T>::get_time_in_current_frame() const
{
	if(frames_.empty() || !started_) {
		return std::chrono::milliseconds{0};
	}
	const auto current_tick = get_current_animation_tick(uses_acceleration_);
	const auto& frame = frames_[current_frame_index_];

	// For finished non-looping animations, clamp to frame duration
	const bool is_last_frame = (current_frame_index_ == frames_.size() - 1);
	if(!cycles_ && is_last_frame && current_tick >= next_frame_tick_) {
		return frame.duration_;
	}

	// Calculate how much time has elapsed in this frame
	const auto time_until_next = next_frame_tick_ - current_tick;
	const auto time_until_next_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_until_next);
	const auto elapsed_in_frame = frame.duration_ - time_until_next_ms;

	// Clamp to valid range [0, duration]
	if(elapsed_in_frame < std::chrono::milliseconds{0}) {
		return std::chrono::milliseconds{0};
	}
	if(elapsed_in_frame > frame.duration_) {
		return frame.duration_;
	}

	return elapsed_in_frame;
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


// ============================================================================
// Protected Methods (for unit_animation)
// ============================================================================

template<typename T>
void animated<T>::remove_frames_until(const std::chrono::milliseconds& new_starting_time)
{
	std::chrono::milliseconds removed_duration{0};

	// Remove frames from the beginning until we reach the target time
	while(removed_duration < new_starting_time && !frames_.empty()) {
		removed_duration += frames_[0].duration_;
		frames_.erase(frames_.begin());
	}

	// Adjust the timeline anchor to account for removed frames
	animation_start_tick_ += removed_duration;
}

template<typename T>
inline void animated<T>::set_end_time(const std::chrono::milliseconds& new_ending_time)
{
	auto current_time = starting_frame_time_;
	auto it = frames_.begin();

	while(it != frames_.end()) {
		auto next_time = current_time + it->duration_;

		// Check if the cut-off point is within this frame
		if(next_time > new_ending_time) {
			// Truncate this frame
			auto new_duration = new_ending_time - current_time;
			if(new_duration < std::chrono::milliseconds{0}) {
				new_duration = std::chrono::milliseconds{0};
			}
			it->duration_ = new_duration;

			// Erase all SUBSEQUENT frames (keep the current one)
			frames_.erase(std::next(it), frames_.end());
			return;
		}

		// Check if cut-off is exactly at the end of this frame
		if(next_time == new_ending_time) {
			frames_.erase(std::next(it), frames_.end());
			return;
		}

		current_time = next_time;
		++it;
	}
}
