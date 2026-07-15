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
	finished_ = false;
	cycles_ = cycles;
	current_frame_index_ = 0;
	const auto current_tick = get_current_animation_tick(uses_acceleration_);
	animation_start_tick_ = current_tick - start_time_offset;
	if(!frames_.empty()) {
		next_frame_tick_ = animation_start_tick_ + frames_[0].start_time_ + frames_[0].duration_;
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
inline void animated<T>::resume_animation()
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
}

template<typename T>
inline void animated<T>::advance_to_current_frame()
{
	if(frames_.empty()) {
		return;
	}

	const auto current_tick = get_current_animation_tick(uses_acceleration_);

	// If a cycling animation is more than a whole cycle behind (e.g., it went un-updated
	// while offscreen), skip forward in whole cycles so the loop below never has to walk
	// more than one cycle of frames.
	if(cycles_) {
		const std::chrono::milliseconds animation_duration = get_animation_duration();
		if(animation_duration > std::chrono::milliseconds{0} && current_tick > (next_frame_tick_ + animation_duration)) {
			const auto behind = current_tick - next_frame_tick_;
			next_frame_tick_ += (behind / animation_duration) * animation_duration; // The int division truncates to whole cycles.
		}
	}

	// Advance frame by frame until we reach the frame containing the current time
	while(current_tick >= next_frame_tick_) {
		std::size_t next_index = current_frame_index_ + 1;

		// Non-looping animation finished, stay on last frame
		if(!cycles_ && next_index == frames_.size()) {
			finished_ = true;
			return;
		}

		// Advance to the next frame, wrapping around when looping
		current_frame_index_ = next_index % frames_.size();
		next_frame_tick_ += frames_[current_frame_index_].duration_;
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
	return get_current_animation_tick(uses_acceleration_) >= next_frame_tick_;
}

template<typename T>
inline bool animated<T>::animation_finished() const
{
	return finished_ || frames_.empty() || !started_ || cycles_ || (get_elapsed_time() >= get_end_time());
}


// ============================================================================
// Time Queries and Manipulation
// ============================================================================

template<typename T>
inline std::chrono::steady_clock::time_point animated<T>::get_playback_tick() const
{
	return paused_ ? pause_tick_ : get_current_animation_tick(uses_acceleration_);
}

template<typename T>
inline std::chrono::milliseconds animated<T>::get_elapsed_time() const
{
	if(!started_) {
		return starting_frame_time_;
	}

	// Calculate elapsed time since animation started
	const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(get_playback_tick() - animation_start_tick_);

	// Apply maximum time cap if set
	if(max_animation_time_ > std::chrono::milliseconds{0} && elapsed > max_animation_time_) {
		return max_animation_time_;
	}

	return elapsed;
}

template<typename T>
inline void animated<T>::apply_time_offset(const std::chrono::milliseconds& time)
{
	// Adjust the anchor point to make the current tick correspond to the requested time offset
	animation_start_tick_ -= time;

	// Restart animation from beginning; advance_to_current_frame() will sync on next call
	if(!frames_.empty()) {
		current_frame_index_ = 0;
		finished_ = false;
		next_frame_tick_ = animation_start_tick_ + frames_[0].start_time_ + frames_[0].duration_;
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
	const auto current_tick = get_playback_tick();
	const auto& frame = frames_[current_frame_index_];

	// How much of this frame has elapsed, clamped to [0, duration].
	const auto time_until_next = std::chrono::duration_cast<std::chrono::milliseconds>(next_frame_tick_ - current_tick);
	const auto elapsed_in_frame = frame.duration_ - time_until_next;
	return std::clamp(elapsed_in_frame, std::chrono::milliseconds{0}, frame.duration_);
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
	// Remove frames from the beginning until we reach the target time
	while(starting_frame_time_ < new_starting_time && !frames_.empty()) {
		starting_frame_time_ += frames_[0].duration_;
		frames_.erase(frames_.begin());
	}
}

template<typename T>
inline void animated<T>::set_end_time(const std::chrono::milliseconds& new_ending_time)
{
	if(frames_.empty()) {
		return;
	}

	// Cut-off before the animation begins: keep nothing but a zero-length first frame
	if(new_ending_time <= starting_frame_time_) {
		frames_.erase(std::next(frames_.begin()), frames_.end());
		frames_.front().duration_ = std::chrono::milliseconds{0};
		return;
	}

	// Cut-off at or past the current end: extend the last frame to reach it
	const auto current_end_time = get_end_time();
	if(new_ending_time >= current_end_time) {
		frames_.back().duration_ += new_ending_time - current_end_time;
		return;
	}

	// The cut-off lands within a frame: walk to it
	auto current_start_time = starting_frame_time_;
	auto it = frames_.begin();

	while(it != frames_.end()) {
		const auto next_end_time = current_start_time + it->duration_;

		// Check if the cut-off point is within or at the end of this frame
		if(next_end_time >= new_ending_time) {
			// Truncate this frame to time remaining (a no-op if the cut-off is exactly at its end)
			it->duration_ = new_ending_time - current_start_time;

			// Erase all SUBSEQUENT frames (keep the current one)
			frames_.erase(std::next(it), frames_.end());
			return;
		}

		current_start_time = next_end_time;
		++it;
	}
}
