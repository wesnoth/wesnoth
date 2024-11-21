/*
	Copyright (C) 2007 - 2024
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

#include "sdl/rect.hpp"

#include <chrono>

namespace gui2 { class top_level_drawable; }

/**
 * A global draw management interface.
 *
 * This interface governs drawing things to the screen in the correct order.
 * Drawable objects must inherit from gui2::top_level_drawable, and may be
 * referred to as "TLD"s.
 *
 * There is an absolute requirement that events happen in a certain order.
 * This is mostly managed by this interface, which calls the TLD methods in
 * order at most once every vsync.
 *
 * The order of events is:
 *   1. General event processing. This is governed by events::pump()
 *      and is currently independent of this interface.
 *   2. Layout and animation. TLDs should ensure their layout information
 *      is up-to-date inside gui2::top_level_drawable::layout(). After
 *      this call they should know where they are on the screen, and be
 *      ready to report it via gui2::top_level_drawable::screen_location().
 *      Animation can also be performed during this stage.
 *   3. Offscreen rendering. TLDs should perform any offscreen rendering
 *      during gui2::top_level_drawable::render(). After this call, all
 *      internal drawing buffers should be in a state ready for display.
 *   4. Drawing to the screen. Drawing to the screen should only ever happen
 *      inside gui2::top_level_drawable::expose(). Drawing to the screen at
 *      any other point is an error.
 *
 * The draw manager will call layout(), render() and expose() in the correct
 * order to ensure all TLD objects are laid out correctly and drawn in the
 * correct order.
 *
 * Drawing order of TLDs is initially set by creation time, but a TLD may
 * be raised to the top of the drawing stack by calling
 * draw_manager::raise_drawable() manually. register_drawable() and
 * deregister_drawable() are called automatically by gui2::top_level_drawable
 * in its constructor and destructor, and do not need to be manually managed.
 *
 * The drawing process happens inside draw_manager::sparkle(). In general,
 * a game loop should perform two basic steps.
 *   1. call events::pump() to process events. Anything other than drawing
 *      to the screen may happen during this step.
 *   2. call draw_manager::sparkle() to draw the screen, if necessary.
 *
 * The main sparkle() function will also rate-limit, so callers do not need
 * to add their own delay to their loops. If vsync is disabled, drawing will
 * happen as frequently as possible. If vsync is enabled, this function will
 * wait for the next screen refresh before drawing. In both cases, if nothing
 * needs to be drawn the function will block for an appropriate length of
 * time before returning.
 *
 * To ensure they are presented for drawing, any drawable object must call
 * draw_manager::invalidate_region() to indicate that an area of the screen
 * needs to be redrawn. This may be called during any phase other than the
 * draw phase. Invalidating regions during the draw phase is an error and
 * will throw an exception.
 */
namespace draw_manager
{

/**
 * Mark a region of the screen as requiring redraw.
 *
 * This should be called any time an item changes in such a way as to
 * require redrawing.
 *
 * This may only be called outside the Draw phase.
 *
 * Regions are combined to result in a minimal number of draw calls,
 * so this may be called for every invalidation without much concern.
 */
void invalidate_region(const rect& region);

/** Mark the entire screen as requiring redraw. */
void invalidate_all();

/**
 * Request an extra render pass.
 *
 * This is used for blur effects, which need to first render what's
 * underneath so that it can be blurred.
 *
 * There is not currently any limit to the number of extra render passes,
 * but do try to keep it finite.
 */
void request_extra_render_pass();

/**
 * Ensure that everything which needs to be drawn is drawn.
 *
 * This includes making sure window sizes and locations are up to date,
 * updating animation frames, and drawing whatever regions of the screen
 * need drawing or redrawing.
 *
 * If vsync is enabled, this function will block until the next vblank.
 * If nothing is drawn, it will still block for an appropriate amount of
 * time to simulate vsync, even if vsync is disabled.
 */
void sparkle();

/**
 * Returns the length of one display frame, in milliseconds.
 *
 * This will usually be determined by the active monitor's refresh rate.
 */
std::chrono::milliseconds get_frame_length();

/** Register a top-level drawable.
 *
 * Registered drawables will be drawn in the order of registration,
 * so the most recently-registered drawable will be "on top".
 */
void register_drawable(gui2::top_level_drawable* tld);

/** Remove a top-level drawable from the drawing stack. */
void deregister_drawable(gui2::top_level_drawable* tld);

/** Raise a TLD to the top of the drawing stack. */
void raise_drawable(gui2::top_level_drawable* tld);

} // namespace draw_manager
