/*
	Copyright (C) 2003 - 2024
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

#include "color_range.hpp"
#include "color.hpp"
#include "sdl/surface.hpp"
#include "utils/math.hpp"
#include "game_version.hpp"

#include <cstdlib>
#include <string>

namespace sdl
{

/** Returns the runtime SDL version. */
version_info get_version();

/**
 * Returns true if the runtime SDL version is at or greater than the
 * specified version, false otherwise.
 */
bool runtime_at_least(uint8_t major, uint8_t minor = 0, uint8_t patch = 0);

} // namespace sdl


inline void sdl_blit(const surface& src, const SDL_Rect* src_rect, surface& dst, SDL_Rect* dst_rect){
	// Note: this is incorrect when both src and dst combine transparent pixels.
	// The correct equation is, per-pixel:
	//   outA = srcA + dstA * (1 - srcA)
	//   outRGB = (srcRGB * srcA + dstRGB * dstA * (1 - srcA)) / outA
	// When outA is 0, outRGB can of course be anything.
	// TODO: implement proper transparent blending using the above formula
	SDL_BlitSurface(src, src_rect, dst, dst_rect);
}

/** Scale a surface using xBRZ algorithm
 *  @param surf		     The source surface
 *  @param z                 The scaling factor. Should be an integer 2-5 (1 is tolerated).
 *  @return		     The scaled surface
 */
surface scale_surface_xbrz(const surface & surf, std::size_t z);

/** Scale a surface using alpha-weighted modified bilinear filtering
 *  Note: causes artifacts with alpha gradients, for example in some portraits
 *  @param surf              The source surface.
 *  @param w                 The width of the resulting surface.
 *  @param h                 The height of the resulting surface.
 *  @return                  A surface containing the scaled version of the source.
 *  @retval 0                Returned upon error.
 *  @retval surf             Returned if w == surf->w and h == surf->h.
 */
surface scale_surface(const surface &surf, int w, int h);

/** Scale a surface using simple bilinear filtering (discarding rgb from source
 *  pixels with 0 alpha)
 *  @param surf              The source surface.
 *  @param w                 The width of the resulting surface.
 *  @param h                 The height of the resulting surface.
 *  @return                  A surface containing the scaled version of the source.
 *  @retval 0                Returned upon error.
 *  @retval surf             Returned if w == surf->w and h == surf->h.
 */
surface scale_surface_legacy(const surface &surf, int w, int h);

/** Scale a surface using modified nearest neighbour algorithm. Use only if
 * preserving sharp edges is a priority (e.g. minimap).
 *  @param surf              The source surface.
 *  @param w                 The width of the resulting surface.
 *  @param h                 The height of the resulting surface.
 *  @return                  A surface containing the scaled version of the source.
 *  @retval 0                Returned upon error.
 *  @retval surf             Returned if w == surf->w and h == surf->h.
 */
surface scale_surface_sharp(const surface& surf, int w, int h);

void adjust_surface_color(surface& surf, int r, int g, int b);
void greyscale_image(surface& surf);
void monochrome_image(surface& surf, const int threshold);
void sepia_image(surface& surf);
void negative_image(surface& surf, const int thresholdR, const int thresholdG, const int thresholdB);
void alpha_to_greyscale(surface& surf);
void wipe_alpha(surface& surf);
/** create an heavy shadow of the image, by blurring, increasing alpha and darkening */
void shadow_image(surface& surf, int scale = 1);

enum channel { RED, GREEN, BLUE, ALPHA };
void swap_channels_image(surface& surf, channel r, channel g, channel b, channel a);

/**
 * Recolors a surface using a map with source and converted palette values.
 * This is most often used for team-coloring.
 *
 * @param surf               The source surface.
 * @param map_rgb            Map of color values, with the keys corresponding to the
 *                           source palette, and the values to the recolored palette.
 */
void recolor_image(surface& surf, const color_range_map& map_rgb);

void brighten_image(surface& surf, int32_t amount);

/** Get a portion of the screen.
 *  Send nullptr if the portion is outside of the screen.
 *  @param surf              The source surface.
 *  @param rect              The portion of the source surface to copy.
 *  @return                  A surface containing the portion of the source.
 *                           No RLE or Alpha bits are set.
 *  @retval 0                if error or the portion is outside of the surface.
 */
surface get_surface_portion(const surface &surf, SDL_Rect &rect);

void adjust_surface_alpha(surface& surf, uint8_t alpha_mod);
void adjust_surface_alpha_add(surface& surf, int amount);

/** Applies a mask on a surface. */
void mask_surface(surface& surf, const surface& mask, bool* empty_result = nullptr, const std::string& filename = std::string());

/** Check if a surface fit into a mask */
bool in_mask_surface(const surface& surf, const surface& mask);

/**
 * Light surf using lightmap
 * @param surf               The source surface.
 * @param lightmap           add/subtract this color to surf
 *                           but RGB values are converted to (X-128)*2
 *                           to cover the full (-256,256) spectrum.
 *                           Should already be neutral
*/
void light_surface(surface& surf, const surface &lightmap);

/**
 * Cross-fades a surface in place.
 *
 * @param surf                    The surface to blur, must have 32 bits per pixel.
 * @param rect                    The part of the surface to blur.
 * @param depth                   The depth of the blurring.
 */
void blur_surface(surface& surf, SDL_Rect rect, int depth = 1);

/**
 * Cross-fades a surface with alpha channel.
 *
 * @param surf                    The source surface.
 * @param depth                   The depth of the blurring.
 */
void blur_alpha_surface(surface& surf, int depth = 1);

/** Cuts a rectangle from a surface. */
surface cut_surface(const surface &surf, const SDL_Rect& r);

/**
 * Blends a surface with a color.
 *
 * Every pixel in the surface will be blended with the @p color given. The
 * final color of a pixel is amount * @p color + (1 - amount) * original.
 *
 * @param surf                    The surface to blend.
 * @param amount                  The amount of the new color is determined by
 *                                @p color. Must be a number in the range
 *                                [0, 1].
 * @param color                   The color to blend width, note its alpha
 *                                channel is ignored.
 */
void blend_surface(surface& surf, const double amount, const color_t color);

/**
 * Rotates a surface by any degrees.
 *
 * @pre @p zoom >= @p offset      Otherwise @return will have empty pixels.
 * @pre @p offset > 0             Otherwise the procedure will not return.
 *
 * @param surf                    The surface to rotate.
 * @param angle                   The angle of rotation.
 * @param zoom                    Which zoom level to use for calculating the result.
 * @param offset                  Pixel offset when scanning the zoomed source.
 *
 * @return                        The rotated surface.
 */
surface rotate_any_surface(const surface& surf, float angle, int zoom, int offset);

/**
 * Rotates a surface 180 degrees.
 *
 * @param surf                    The surface to rotate.
 *
 * @return                        The rotated surface.
 */
surface rotate_180_surface(const surface& surf);

/**
 * Rotates a surface 90 degrees.
 *
 * @param surf                    The surface to rotate.
 * @param clockwise               Whether the rotation should be clockwise (true)
 *                                or counter-clockwise (false).
 *
 * @return                        The rotated surface.
 */
surface rotate_90_surface(const surface& surf, bool clockwise);

void flip_surface(surface& surf);
void flop_surface(surface& surf);

rect get_non_transparent_portion(const surface& surf);
