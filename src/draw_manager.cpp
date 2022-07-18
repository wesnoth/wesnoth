/*
	Copyright (C) 2022
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
#include "video.hpp"

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_timer.h>

#include <vector>
#include <map>

static lg::log_domain log_draw_man("draw/manager");
#define ERR_DM LOG_STREAM(err, log_draw_man)
#define WRN_DM LOG_STREAM(warn, log_draw_man)
#define LOG_DM LOG_STREAM(info, log_draw_man)
#define DBG_DM LOG_STREAM(debug, log_draw_man)

using gui2::top_level_drawable;

namespace {
std::vector<top_level_drawable*> top_level_drawables_;
std::vector<rect> invalidated_regions_;
bool drawing_ = false;
bool tlds_invalidated_ = false;
uint32_t last_sparkle_ = 0;
} // namespace

namespace draw_manager {

static void layout();
static void render();
static bool draw();
static void wait_for_vsync();

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
			DBG_DM << "superceding previous invalidation " << r
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

void sparkle()
{
	if (drawing_) {
		ERR_DM << "Draw recursion detected";
		throw game::error("recursive draw");
	}

	// Keep track of whether the TLD vector has been invalidated.
	tlds_invalidated_ = false;

	// Ensure layout and animations are up-to-date.
	draw_manager::layout();

	// Unit tests rely on not doing any rendering, or waiting at all...
	// "behave differently when being tested" is really not good policy
	// but whatever.
	if(CVideo::get_singleton().any_fake()) {
		invalidated_regions_.clear();
		return;
	}

	// Ensure any off-screen render buffers are up-to-date.
	draw_manager::render();

	// Draw to the screen.
	if (draw_manager::draw()) {
		// We only need to flip the screen if something was drawn.
		CVideo::get_singleton().render_screen();
	} else {
		wait_for_vsync();
	}

	last_sparkle_ = SDL_GetTicks();
}

static void wait_for_vsync()
{
	int rr = CVideo::get_singleton().current_refresh_rate();
	if (rr <= 0) {
		// make something up
		rr = 60;
	}
	// allow 1ms for general processing
	int vsync_delay = (1000 / rr) - 1;
	int time_to_wait = last_sparkle_ + vsync_delay - SDL_GetTicks();
	if (time_to_wait > 0) {
		// delay a maximum of 1 second in case something crazy happens
		SDL_Delay(std::min(time_to_wait, 1000));
	}
}

static void layout()
{
	for (auto tld : top_level_drawables_) {
		if (tlds_invalidated_) { break; }
		tld->layout();
	}
}

static void render()
{
	for (auto tld : top_level_drawables_) {
		if (tlds_invalidated_) { break; }
		tld->render();
	}
}

static bool draw()
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
			rect i = r.intersect(tld->screen_location());
			if (i.empty()) {
				//DBG_DM << "  skip " << static_cast<void*>(tld);
				//STREAMING_LOG << "x";
				continue;
			}
			DBG_DM << "  to " << static_cast<void*>(tld);
			//STREAMING_LOG << "*";
			drawn |= tld->expose(i);
		}
	}
	drawing_ = false;
	return drawn;
}

void register_drawable(top_level_drawable* tld)
{
	DBG_DM << "registering TLD " << static_cast<void*>(tld);
	top_level_drawables_.push_back(tld);
}

void deregister_drawable(top_level_drawable* tld)
{
	DBG_DM << "deregistering TLD " << static_cast<void*>(tld);
	auto& vec = top_level_drawables_;
	vec.erase(std::remove(vec.begin(), vec.end(), tld), vec.end());
	tlds_invalidated_ = true;
}

void raise_drawable(top_level_drawable* tld)
{
	DBG_DM << "raising TLD " << static_cast<void*>(tld);
	auto& vec = top_level_drawables_;
	vec.erase(std::remove(vec.begin(), vec.end(), tld), vec.end());
	vec.push_back(tld);
	tlds_invalidated_ = true;
}

} // namespace draw_manager
