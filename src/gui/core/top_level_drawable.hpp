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

namespace gui2
{

/**
 * A top-level drawable item (TLD), such as a window.
 *
 * For now, TLDs keep track of where they are on the screen on their own.
 * They must draw themselves when requested via expose().
 *
 * The TLD interface will be called in the following order:
 *   - update()
 *   - layout()
 *   - render()
 *   - screen_location()
 *   - zero or more expose() calls
 *
 * It can be assumed that events will have been processed before TLD
 * interface calls commence, and that no more events will be processed
 * until all TLD interfaces have been called. As such the main program
 * loop will usually be:
 *   1. process events
 *   2. call update() on all TLDs
 *   3. call layout() on all TLDs
 *   4. call render() on all TLDs
 *   5. call expose() on TLDs as appropriate
 *
 * TLDs are responsible for propagating these calls to their children.
 *
 * This process is loosely based on the GTK drawing model. Ref:
 *   - https://docs.gtk.org/gtk3/drawing-model.html
 *   - https://docs.gtk.org/gtk4/drawing-model.html
 */
class top_level_drawable
{
protected:
	top_level_drawable();
	virtual ~top_level_drawable();

	// These make sure the TLD is registered.
	top_level_drawable(const top_level_drawable&);
	top_level_drawable& operator=(const top_level_drawable&);
	top_level_drawable(top_level_drawable&&);
	top_level_drawable& operator=(top_level_drawable&&);

public:
	/**
	 * Update state and any parameters that may effect layout, or any of
	 * the later stages.
	 *
	 * In general this should be used to make changes to things that will
	 * affect the visible state of the drawable. This can include changing
	 * the drawable's size or position, or updating animation frames to
	 * show the appropriate image for the current time. Exact usage is up
	 * to the drawable to decide.
	 *
	 * This interface is optional.
	 */
	virtual void update() {}

	/**
	 * Finalize the size and position of the drawable and its children,
	 * and invalidate any regions requiring redraw.
	 *
	 * Visibly changed screen locations should be invalidated using
	 * draw_manager::invalidate_region(), both in the previous location
	 * and in the new location if different.
	 *
	 * TLDs must not perform any actual drawing during layout.
	 *
	 * This interface is optional.
	 */
	virtual void layout() {};

	/**
	 * Perform any internal rendering necessary to prepare the drawable.
	 *
	 * For example if the drawable has an offscreen buffer it manages,
	 * it should ensure this buffer is up to date.
	 *
	 * TLDs should also invalidate any regions visibly changed by this call.
	 *
	 * This interface is optional.
	 */
	virtual void render() {}

	/**
	 * Draw the portion of the drawable intersecting @p region to the screen.
	 *
	 * TLDs must not invalidate regions during expose. Only drawing must
	 * occur, with no modification of layout.
	 *
	 * Implementation of this interface is mandatory.
	 *
	 * @param region    The region to expose, in absolute draw-space
	 *                  coordinates.
	 * @returns         True if anything was drawn, false otherwise.
	 */
	virtual bool expose(const rect& region) = 0;

	/**
	 * The location of the TLD on the screen, in drawing coordinates.
	 *
	 * This will be used to determine the region (if any) to expose.
	 *
	 * Implementation of this interface is mandatory.
	 */
	virtual rect screen_location() = 0;
};

} // namespace gui2
