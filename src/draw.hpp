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

#pragma once

/** @file
 * Drawing functions, for drawing things on the screen.
 *
 * This includes pixel drawing for lines, rectangles and circles;
 * fill and clear routines; and commands to render SDL surfaces and
 * textures, in full or in part.
 *
 * For the most part draw commands take coordinates in what is called
 * "draw space", or "game-native coordinates". These are the coordinates
 * that are used in WML, and can be thought of as pixels.
 *
 * High-DPI textures and fonts will automatically use their full
 * resolution when possible, without any extra handling required.
 */

#include <SDL2/SDL_rect.h>
#include <vector>

struct color_t;
class surface;
class texture;

namespace draw
{

/**************************************/
/* basic drawing and pixel primatives */
/**************************************/


/**
 * Fill an area with the given colour.
 *
 * If the alpha component is not specified, it defaults to fully opaque.
 *
 * @param rect      The area to fill, in drawing coordinates.
 * @param r         The red   component of the fill colour, 0-255.
 * @param g         The green component of the fill colour, 0-255.
 * @param b         The blue  component of the fill colour, 0-255.
 * @param a         The alpha component of the fill colour, 0-255.
 */
void fill(const SDL_Rect& rect, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void fill(const SDL_Rect& rect, uint8_t r, uint8_t g, uint8_t b);
void fill(const SDL_Rect& rect, const color_t& color);

/**
 * Fill an area.
 *
 * Uses the current drawing colour set by set_draw_color().
 * Coordinates are given in draw space.
 *
 * @param rect      The area to fill, in drawing coordinates.
 */
void fill(const SDL_Rect& rect);

/**
 * Set the drawing colour.
 *
 * This is the colour used by fill(), line(), points(), etc..
 *
 * If the alpha component is not specified, it defaults to fully opaque.
 *
 * @param r         The red   component of the drawing colour, 0-255.
 * @param g         The green component of the drawing colour, 0-255.
 * @param b         The blue  component of the drawing colour, 0-255.
 * @param a         The alpha component of the drawing colour, 0-255.
 */
void set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void set_color(uint8_t r, uint8_t g, uint8_t b);
void set_color(const color_t& c);

/**
 * Draw a rectangle.
 *
 * Uses the current drawing colour set by set_color().
 * Coordinates are given in draw space.
 *
 * @param rect      The rectangle to draw, in drawing coordinates.
 */
void rect(const SDL_Rect& rect);

/**
 * Draw a rectangle using the given colour.
 *
 * @param rect      The rectangle to draw, in drawing coordinates.
 * @param r         The red   component of the drawing colour, 0-255.
 * @param g         The green component of the drawing colour, 0-255.
 * @param b         The blue  component of the drawing colour, 0-255.
 * @param a         The alpha component of the drawing colour, 0-255.
 */
void rect(const SDL_Rect& rect, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void rect(const SDL_Rect& rect, uint8_t r, uint8_t g, uint8_t b);
void rect(const SDL_Rect& rect, const color_t& color);

/**
 * Draw a line.
 *
 * Uses the current drawing colour set by set_color().
 * Coordinates are given in draw space.
 *
 * @param from_x    The X coordinate of the start point, in draw space.
 * @param from_y    The Y coordinate of the start point, in draw space.
 * @param to_x      The X coordinate of the end point, in draw space.
 * @param to_y      The Y coordinate of the end point, in draw space.
 */
void line(int from_x, int from_y, int to_x, int to_y);

/**
 * Draw a line of the given colour.
 *
 * @param from_x    The X coordinate of the start point, in draw space.
 * @param from_y    The Y coordinate of the start point, in draw space.
 * @param to_x      The X coordinate of the end point, in draw space.
 * @param to_y      The Y coordinate of the end point, in draw space.
 * @param c         The RGBA colour of the line.
 */
void line(int from_x, int from_y, int to_x, int to_y, const color_t& c);

/** Draw a set of points. */
void points(const std::vector<SDL_Point>& points);

/** Draw a single point. */
void point(int x, int y);

// TODO: enum for common octant choices - nice but not necessary
/**
 * Draw a circle of the given colour.
 *
 * Only the outline of the circle is drawn. To draw a filled circle,
 * use draw::disc().
 *
 * The octants bitfield can be used to draw only certain octants
 * of the circle, resulting in one or more arcs.
 *
 * If no colour is specified, the current drawing colour will be used.
 *
 * @param x         The x coordinate of the center of the circle.
 * @param y         The y coordinate of the center of the circle.
 * @param r         The radius of the circle.
 * @param c         The colour of the circle.
 * @param octants   A bitfield indicating which octants to draw,
 *                  starting at twelve o'clock and moving clockwise.
 */
void circle(int x, int y, int r, const color_t& c, uint8_t octants = 0xff);
void circle(int x, int y, int r, uint8_t octants = 0xff);

/**
 * Draw a solid disc of the given colour.
 *
 * The octants bitfield can be used to draw only certain octants
 * of the disc, resulting in one or more filled wedges.
 *
 * If no colour is specified, the current drawing colour will be used.
 *
 * @param x         The x coordinate of the center of the circle.
 * @param y         The y coordinate of the center of the circle.
 * @param r         The radius of the circle.
 * @param c         The colour of the circle.
 * @param octants   A bitfield indicating which octants to draw,
 *                  starting at twelve o'clock and moving clockwise.
 */
void disc(int x, int y, int r, const color_t& c, uint8_t octants = 0xff);
void disc(int x, int y, int r, uint8_t octants = 0xff);


/*******************/
/* texture drawing */
/*******************/


/*
 * Draws an SDL surface at the given location.
 *
 * The w and h members of dst are ignored, but will be updated
 * to reflect the final draw extents including clipping.
 *
 * The surface will be rendered in game-native resolution,
 * and all coordinates are given in this context.
 *
 * WARNING: This function will likely be deprecated in the future.
 * variants using textures should be preferred in all cases.
 *
 * @param surf                The surface to draw.
 * @param dst                 Where to draw the surface. w and h are ignored, but will be updated to reflect the final draw extents including clipping.
 */
//void blit(const surface& surf, SDL_Rect* dst);

/*
 * Draws a surface at the given coordinates.
 *
 * The surface will be rendered in game-native resolution, a.k.a.
 * draw space, and all coordinates are given in this context.
 *
 * WARNING: This function will likely be deprecated in the future.
 * variants using textures should be preferred in all cases.
 *
 * @param surf                The surface to draw.
 * @param x                   The x coordinate of the top-left corner.
 * @param y                   The y coordinate of the top-left corner.
 */
//void blit(const surface& surf, int x, int y);

/*
 * Draws an area of a surface at the given location.
 *
 * The surface will be rendered in game-native resolution,
 * and all coordinates are given in this context.
 *
 * WARNING: This function will likely be deprecated in the future.
 * variants using textures should be preferred in all cases.
 *
 * @param surf                The surface to draw.
 * @param x                   The x coordinate at which to draw.
 * @param y                   The y coordinate at which to draw.
 * @param srcrect             The area of the surface to draw. If null, the entire surface is drawn.
 * @param clip_rect           The clipping area. If not null, the surface will only be drawn
 *                            within the bounds of the given rectangle.
 */
//void blit(const surface& surf, int x, int y, const SDL_Rect* src, const SDL_Rect* clip);

/**
 * Draws a texture, or part of a texture, at the given location.
 *
 * The portion of the texture to be drawn will be scaled to fill
 * the target rectangle.
 *
 * This version takes coordinates in game-native resolution,
 * which may be lower than the final output resolution in high-dpi
 * contexts or if pixel scaling is used. The texture will be copied
 * in high-resolution if possible.
 *
 * @param tex       The texture to be copied / drawn.
 * @param dst       The target location to copy the texture to,
 *                  in low-resolution game-native drawing coordinates.
 *                  If null, this fills the entire render target.
 * @param src       The portion of the texture to copy.
 *                  If null, this copies the entire texture.
 */
void blit(const texture& tex, const SDL_Rect& dst, const SDL_Rect& src);
void blit(const texture& tex, const SDL_Rect& dst);
void blit(const texture& tex);

/**
 * Draws a texture, or part of a texture, at the given location,
 * also mirroring/flipping the texture horizontally and/or vertically.
 *
 * By default the texture will be flipped horizontally.
 *
 * @param tex       The texture to be copied / drawn.
 * @param dst       The target location to copy the texture to,
 *                  in low-resolution game-native drawing coordinates.
 *                  If not given, the entire render target will be filled.
 * @param src       The portion of the texture to copy.
 *                  If not given, the entire texture will be copied.
 * @param flip_h    Whether to flip/mirror the texture horizontally.
 * @param flip_v    Whether to flip/mirror the texture vertically.
 */
void flipped(const texture& tex,
	const SDL_Rect& dst,
	const SDL_Rect& src,
	bool flip_h = true,
	bool flip_v = false
);
void flipped(const texture& tex,
	const SDL_Rect& dst,
	bool flip_h = true,
	bool flip_v = false
);
void flipped(const texture& tex, bool flip_h = true, bool flip_v = false);

/**
 * Tile a texture to fill a region.
 *
 * The texture may be aligned either with its center at the center
 * of the region, or with its top-left corner at the top-left corner
 * of the region.
 *
 * @param tex       The texture to use to fill the region.
 * @param dst       The region to fill, in draw space.
 * @param centered  If true the tiled texture will be centered.
 *                  If false, it will align at the top-left.
 * @param mirrored  If true the texture will be mirrored in such a way that
 *                  adjacent tiles always share a common edge. This can look
 *                  better for images that are not perfect tiles.
 */
void tiled(const texture& tex,
	const SDL_Rect& dst,
	bool centered = false,
	bool mirrored = false
);


} // namespace draw
