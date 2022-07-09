/*
	Copyright (C) 2007 - 2022
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
 *   - layout()
 *   - render()
 *   - screen_location()
 *   - zero or more expose() calls
 */
class top_level_drawable
{
protected:
	top_level_drawable();
	virtual ~top_level_drawable();
public:
	/**
	 * Size and position the drawable and its children.
	 *
	 * If this results in a change, it should invalidate both the previous
	 * and new drawable regions via draw_manager::invalidate_region().
	 * If the drawable was not previously laid out, it should invalidate
	 * the newly determined region.
	 *
	 * TLDs must not perform any actual drawing during layout.
	 *
	 * Implementation of this interface is mandatory.
	 */
	virtual void layout() = 0;

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
	virtual bool expose(const SDL_Rect& region) = 0;

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
