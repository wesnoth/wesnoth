/*
	Copyright (C) 2022 - 2025
	by Thomas Iorns <mesilliac@tomanui.nz>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include <cstdint>

#pragma once

/**
 * @file
 * Contains functions for cleanly handling SDL input.
 */

struct point;

namespace sdl
{

/**
 * A wrapper for SDL_GetMouseState that gives coordinates in draw space.
 */
uint32_t get_mouse_state(int *x, int *y);

/** Returns the current mouse button mask */
uint32_t get_mouse_button_mask();

/** Returns the current mouse location in draw space. */
point get_mouse_location();

/**
 * Returns a bitmask of active modifier keys (ctrl, shift, alt, gui).
 *
 * Unused modifier keys (caps lock, scroll lock, num lock, AltGr) are
 * filtered out and will always be unset.
 *
 * Left and right keys are not distinguished. If either is detected, both
 * will be set. For example if only left shift is down, both KMOD_LSHIFT
 * and KMOD_RSHIFT will be set in the returned bitmask.
 *
 * @returns  A bitmask of SDL_Keymod values representing the active state.
 */
unsigned get_mods();

} // namespace sdl
