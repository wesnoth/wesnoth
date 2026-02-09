/*
	Copyright (C) 2025
	by Durzi/mentos987
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

#include "game_config.hpp"
#include "preferences/preferences.hpp"
#include "sdl/surface.hpp"

#include <cstdint>
#include <algorithm>

/**
 * @file
 * SIMD-accelerated helper functions for image manipulation.
 * These functions operate on raw pixel buffers to isolate platform-specific
 * intrinsics (SSE2, NEON) from the main codebase.
 */

namespace simd {

	/** Minimum number of pixels required to trigger SIMD optimizations. */
	constexpr std::size_t SIMD_THRESHOLD = 64;

	/** Checks if SIMD hardware acceleration is allowed/enabeled.  */
	inline bool is_enabled() {
		// CLI (Command Line Flag). Example: wesnoth --no-simd
		if(game_config::no_simd) {
			return false;
		}

		// Env (Environment Variable). Does not exist by default. Initialized once since getenv() is a heavy operation.
		static const bool env_disables_simd = std::getenv("WESNOTH_NO_SIMD") != nullptr;
		if(env_disables_simd) {
			return false;
		}

		// The user's saved preference. Set by the SIMD checkbox at advanced options menu in the game.
		return prefs::get().simd_enabled();
	}
}

/**
* @brief Modifies the alpha channel of the surface pixels based on the mask.
* @param surf      The surface to modify.
* @param mask      The mask surface to read alpha values from.
* @param empty     Output: set to false if any resulting pixel is non-transparent.
*                  Should be initialised to true by the caller.
* @return Number of pixels processed via SIMD (always a multiple of 4),
*         or 0 if SIMD was unavailable / pixel count was below the threshold.
*/
std::size_t mask_surface_simd(surface& surf, const surface& mask, bool& empty);

/**
* @brief Applies an alpha modification to the whole surface using SIMD.
* @param surf      The surface to modify.
* @param alpha_mod The 0-255 modifier to multiply the existing alpha by.
* @return Number of pixels processed via SIMD, or 0 if SIMD was not used.
*/
std::size_t apply_surface_opacity_simd(surface& surf, uint8_t alpha_mod);

/**
* @brief Adjusts the color channels of a surface using saturated SIMD arithmetic.
* @param surf      The surface to modify.
* @param r, g, b   Amounts to add (positive) or subtract (negative) per channel. Clamped to [-255, 255].
* @return Number of pixels processed via SIMD, or 0 if SIMD was not used.
*/
std::size_t adjust_surface_color_simd(surface& surf, int r, int g, int b);

/**
* @brief Flips each row of pixels (horizontal mirror) using SIMD.
* @param surf      The surface to modify.
* @return Number of columns (per row) processed via SIMD, or 0 if SIMD was not used.
*/
std::size_t flip_image_simd(surface& surf);
