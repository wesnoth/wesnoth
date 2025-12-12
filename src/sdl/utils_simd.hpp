/*
	Copyright (C) 2003 - 2025
	by David White <dave@whitevine.net>
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

#include <cstdint>
#include <algorithm>

/**
 * @file
 * SIMD-accelerated helper functions for image manipulation.
 * These functions operate on raw pixel buffers to isolate platform-specific
 * intrinsics (SSE2, NEON) from the main codebase.
 */

 /**
  * Optimized implementation for mask_surface.
  * Modifies the alpha channel of the surface pixels based on the mask.
  *
  * @param surf_ptr      Pointer to the source surface pixels (start of buffer).
  * @param mask_ptr      Pointer to the mask surface pixels (start of buffer).
  * @param total_pixels  Total number of pixels to process.
  * @param empty         Output parameter: set to false if any resulting pixel is non-transparent.
  * (Should be initialized to true by the caller).
  *
  * @return              true if a SIMD path was executed (SSE2/NEON).
  * false if the function fell back to scalar or no SIMD was available.
  */
bool mask_surface_simd(uint32_t* surf_ptr, const uint32_t* mask_ptr, std::size_t total_pixels, bool& empty);

/**
 * Optimized implementation for in_mask_surface.
 * Checks if the surface fits entirely within the non-transparent area of the mask.
 *
 * @param surf_ptr      Pointer to the source surface pixels.
 * @param mask_ptr      Pointer to the mask surface pixels.
 * @param total_pixels  Total number of pixels to process.
 * @param fits          Output parameter: set to false if a mismatch is found.
 * (Should be initialized to true by the caller).
 *
 * @return              true if a SIMD path was executed (SSE2/NEON).
 * false if the function fell back to scalar or no SIMD was available.
 */
bool in_mask_surface_simd(const uint32_t* surf_ptr, const uint32_t* mask_ptr, std::size_t total_pixels, bool& fits);

/**
 * @brief Applies an alpha modification to the whole surface using SIMD.
 * @param surf_ptr Pointer to the pixel data.
 * @param total_pixels Number of pixels in the surface.
 * @param alpha_mod The 0-255 modifier to multiply the existing alpha by.
 * @return true if SIMD optimization was used, false otherwise (e.g., small surface or no SIMD support).
 */
bool apply_surface_opacity_simd(uint32_t* surf_ptr, std::size_t total_pixels, uint8_t alpha_mod);

/**
 * @brief Adjusts the color channels of a surface using saturated SIMD arithmetic.
 * @return true if SIMD was used, false otherwise.
 */
bool adjust_surface_color_simd(uint32_t* surf_ptr, std::size_t total_pixels, int r, int g, int b);

/**
 * @brief Flips the order of pixels (reverses a row) using SIMD.
 * @param row_ptr Pointer to the start of the pixel row.
 * @param width_total_pixels The width number of pixels in the row.
 * @return true if SIMD optimization was used, false otherwise (e.g., width too small or no SIMD support).
 */
bool flip_row_simd(uint32_t* row_ptr, std::size_t width_total_pixels);
