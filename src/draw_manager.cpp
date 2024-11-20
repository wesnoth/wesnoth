/*
	Copyright (C) 2022 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "draw_manager.hpp"

#include "draw.hpp"
#include "exceptions.hpp"
#include "log.hpp"
#include "gui/core/top_level_drawable.hpp"
#include "sdl/rect.hpp"
#include "utils/general.hpp"
#include "video.hpp"

#include <thread>
#include <algorithm>
#include <vector>

static lg::log_domain log_draw_man("draw/manager");
#define ERR_DM LOG_STREAM(err, log_draw_man)
#define WRN_DM LOG_STREAM(warn, log_draw_man)
#define LOG_DM LOG_STREAM(info, log_draw_man)
#define DBG_DM LOG_STREAM(debug, log_draw_man)

using gui2::top_level_drawable;
using std::chrono::steady_clock;
using namespace std::chrono_literals;

// This is not publically exposed, because nobody else should be using it.
// Implementation is in video.cpp.
namespace video { void render_screen(); }

namespace {
std::vector<top_level_drawable*> top_level_drawables_;
std::vector<rect> invalidated_regions_;
bool drawing_ = false;
bool tlds_need_tidying_ = false;
steady_clock::time_point last_sparkle_;
bool extra_pass_requested_ = false;
} // namespace

namespace draw_manager {

static void update();
static void layout();
static void render();
static bool expose();
static void wait_for_vsync();
static void tidy_drawables();

void invalidate_region(const rect& region)
{
	if (drawing_) {
		ERR_DM << "Attempted to invalidate region " << region
			<< " during draw";
		throw game::error("invalidate during draw");
	}

	// On-add region optimization
	rect progressive_cover = region;
	int64_t cumulative_area = 0;
	for (auto& r : invalidated_regions_) {
		if (r.contains(region)) {
			// An existing invalidated region already contains it,
			// no need to do anything in this case.
			//DBG_DM << "no need to invalidate " << region;
			//STREAMING_LOG << '.';
			return;
		}
		if (region.contains(r)) {
			// This region contains a previously invalidated region,
			// might as well supercede it with this.
			DBG_DM << "superseding previous invalidation " << r
				<< " with " << region;
			//STREAMING_LOG << '\'';
			r = region;
			return;
		}
		// maybe merge with another rect
		rect m = r.minimal_cover(region);
		if (m.area() <= r.area() + region.area()) {
			// This won't always be the best,
			// but it also won't ever be the worst.
			DBG_DM << "merging " << region << " with " << r
				<< " to invalidate " << m;
			//STREAMING_LOG << ':';
			r = m;
			return;
		}
		// maybe merge *all* the rects
		progressive_cover.expand_to_cover(r);
		cumulative_area += r.area();
		if (progressive_cover.area() <= cumulative_area) {
			DBG_DM << "conglomerating invalidations to "
				<< progressive_cover;
			//STREAMING_LOG << '%';
			// replace the first one, so we can easily prune later
			invalidated_regions_[0] = progressive_cover;
			return;
		}
	}

	// No optimization was found, so add a new invalidation
	DBG_DM << "invalidating region " << region;
	//STREAMING_LOG << '.';
	invalidated_regions_.push_back(region);
}

void invalidate_all()
{
	// Note: this does not support render targets other than the screen.
	invalidate_region(video::game_canvas());
}

void request_extra_render_pass()
{
	extra_pass_requested_ = true;
}

void sparkle()
{
	if (drawing_) {
		ERR_DM << "Draw recursion detected";
		throw game::error("recursive draw");
	}

	// Remove any invalidated TLDs from previous iterations or events.
	if (tlds_need_tidying_) {
		tidy_drawables();
		tlds_need_tidying_ = false;
	}

	// Animate, process, and update state.
	draw_manager::update();

	// Ensure layout is up-to-date.
	draw_manager::layout();

	// If we are running headless or executing unit tests, do not render.
	// There are not currently any tests for actual rendering output.
	if(video::headless() || video::testing()) {
		invalidated_regions_.clear();
		return;
	}

	// Ensure any off-screen render buffers are up-to-date.
	draw_manager::render();

	// Draw to the screen.
	bool drew_something = draw_manager::expose();

	// If extra render passes are requested, render and draw again.
	while (extra_pass_requested_) {
		extra_pass_requested_ = false;
		draw_manager::render();
		drew_something |= draw_manager::expose();
	}

	if (drew_something) {
		// We only need to flip the screen if something was drawn.
		video::render_screen();
	} else {
		wait_for_vsync();
	}

	last_sparkle_ = steady_clock::now();
}

std::chrono::milliseconds get_frame_length()
{
	int rr = video::current_refresh_rate();
	if (rr <= 0) {
		// make something up
		rr = 60;
	}
	// allow 1ms for general processing
	auto vsync_delay = (1000ms / rr) - 1ms;
	return std::min(vsync_delay, 1000ms);
}

static void wait_for_vsync()
{
	auto now = steady_clock::now();
	auto next_frame = last_sparkle_ + get_frame_length();
	if (now < next_frame) {
		// delay a maximum of 1 second in case something crazy happens
		std::this_thread::sleep_for(std::min<steady_clock::duration>(next_frame - now, 1s));
	}
}

static void update()
{
	for (size_t i = 0; i < top_level_drawables_.size(); ++i) {
		top_level_drawable* tld = top_level_drawables_[i];
		if (tld) { tld->update(); }
	}
}

static void layout()
{
	for (size_t i = 0; i < top_level_drawables_.size(); ++i) {
		top_level_drawable* tld = top_level_drawables_[i];
		if (tld) { tld->layout(); }
	}
}

static void render()
{
	for (size_t i = 0; i < top_level_drawables_.size(); ++i) {
		top_level_drawable* tld = top_level_drawables_[i];
		if (tld) { tld->render(); }
	}
}

static bool expose()
{
	drawing_ = true;

	// For now just send all regions to all TLDs in the correct order.
	bool drawn = false;
next:
	while (!invalidated_regions_.empty()) {
		rect r = invalidated_regions_.back();
		invalidated_regions_.pop_back();
		// check if this will be superceded by or should be merged with another
		for (auto& other : invalidated_regions_) {
			// r will never contain other, due to construction
			if (other.contains(r)) {
				DBG_DM << "skipping redundant draw " << r;
				//STREAMING_LOG << "-";
				goto next;
			}
			rect m = other.minimal_cover(r);
			if (m.area() <= r.area() + other.area()) {
				DBG_DM << "merging inefficient draws " << r;
				//STREAMING_LOG << "=";
				other = m;
				goto next;
			}
		}
		DBG_DM << "drawing " << r;
		//STREAMING_LOG << "+";
		auto clipper = draw::override_clip(r);
		for (auto tld : top_level_drawables_) {
			if (!tld) { continue; }
			rect i = r.intersect(tld->screen_location());
			if (i.empty()) {
				//DBG_DM << "  skip " << static_cast<void*>(tld);
				//STREAMING_LOG << "x";
				continue;
			}
			DBG_DM << "  to " << static_cast<void*>(tld);
			//STREAMING_LOG << "*";
			try {
				drawn |= tld->expose(i);
			} catch(...) {
				WRN_DM << "exception " << utils::get_unknown_exception_type()
					   << " thrown during expose " << static_cast<void*>(tld);
				drawing_ = false;
				throw;
			}
		}
	}
	drawing_ = false;
	return drawn;
}

// Note: This function ensures that multiple copies are not added.
// We can assume top_level_drawables_ will contain at most one of each TLD.
void register_drawable(top_level_drawable* tld)
{
	DBG_DM << "registering TLD " << static_cast<void*>(tld);
	auto& vec = top_level_drawables_;
	if (std::find(vec.begin(), vec.end(), tld) != vec.end()) {
		raise_drawable(tld);
	} else {
		top_level_drawables_.push_back(tld);
	}
}

void deregister_drawable(top_level_drawable* tld)
{
	DBG_DM << "deregistering TLD " << static_cast<void*>(tld);
	auto& vec = top_level_drawables_;
	auto it = std::find(vec.begin(), vec.end(), tld);
	// Sanity check
	if (it == vec.end()) {
		WRN_DM << "attempted to deregister nonexistent TLD "
			<< static_cast<void*>(tld);
		return;
	}
	// Replace it with a null pointer. We will tidy it later.
	// This prevents removals from interfering with TLD iteration.
	*it = nullptr;
	tlds_need_tidying_ = true;
}

void raise_drawable(top_level_drawable* tld)
{
	DBG_DM << "raising TLD " << static_cast<void*>(tld);
	auto& vec = top_level_drawables_;
	auto it = std::find(vec.begin(), vec.end(), tld);
	// Sanity check
	if (it == vec.end()) {
		ERR_DM << "attempted to raise nonexistent TLD "
			<< static_cast<void*>(tld);
		return;
	}
	// Invalidate existing occurances. They will be removed later.
	for ( ; it != vec.end(); it = std::find(it, vec.end(), tld)) {
		*it = nullptr;
	}
	// Then just readd it on the end.
	vec.push_back(tld);
	tlds_need_tidying_ = true;
}

static void tidy_drawables()
{
	// Remove all invalidated TLDs from the list.
	DBG_DM << "tidying " << top_level_drawables_.size() << " drawables";
	utils::erase(top_level_drawables_, nullptr);
	DBG_DM << top_level_drawables_.size() << " after tidying";
}

} // namespace draw_manager
