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
struct SDL_Texture;

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


/***************************/
/* RAII state manipulation */
/***************************/


/** A class to manage automatic restoration of the clipping region.
 *
 * While this can be constructed on its own, it is usually easier to
 * use the draw::set_clip() utility function.
 */
class clip_setter
{
public:
	explicit clip_setter(const SDL_Rect& clip);
	~clip_setter();
private:
	SDL_Rect c_;
};

/**
 * Set the clipping area. All draw calls will be clipped to this region.
 *
 * The clipping area is specified in draw-space coordinates.
 *
 * The returned object will reset the clipping area when it is destroyed,
 * so it should be kept in scope until drawing is complete.
 *
 * @param clip          The clipping region in draw-space coordinates.
 * @returns             A clip_setter object. When this object is destroyed
 *                      the clipping region will be restored to whatever
 *                      it was before this call.
 */
clip_setter set_clip(const SDL_Rect& clip);

/**
 * Set the clipping area to the intersection of the current clipping
 * area and the given rectangle.
 *
 * Otherwise acts as set_clip().
 */
clip_setter reduce_clip(const SDL_Rect& clip);

/**
 * Set the clipping area, without any provided way of setting it back.
 *
 * @param clip          The clipping area, in draw-space coordinates.
 */
void force_clip(const SDL_Rect& clip);

/** Get the current clipping area, in draw coordinates. */
SDL_Rect get_clip();


/** A class to manage automatic restoration of the viewport region.
 *
 * While this can be constructed on its own, it is usually easier to
 * use the draw::set_viewport() utility function.
 */
class viewport_setter
{
public:
	explicit viewport_setter(const SDL_Rect& viewport);
	~viewport_setter();
private:
	SDL_Rect v_;
};

/**
 * Set the viewport. Drawing operations will have their coordinates
 * adjusted to the viewport.
 *
 * The top-left corner of the viewport will be interpreted as (0,0) in
 * draw space coordinates while the returned object is in scope.
 *
 * The new viewport is specified in absolute coordinates, relative to the
 * full drawing surface.
 *
 * The returned object will reset the viewport when it is destroyed, so
 * it should be kept in scope until viewport-relative drawing is complete.
 *
 * @param viewport      The new viewport region, relative to the current
 *                      viewport.
 * @param clip          If true, the clipping region will also be set to
 *                      the new viewport.
 * @returns             A viewport_setter object. When this object is
 *                      destroyed the viewport will be restored to whatever
 *                      it was before this call.
 */
viewport_setter set_viewport(const SDL_Rect&);

/**
 * Set the viewport, without any provided way of setting it back.
 *
 * The new viewport is specified in absolute coordinates, relative to the
 * full drawing surface.
 *
 * @param viewport      The viewport, in absolute draw-space coordinates.
 *                      If null, the viewport is reset to the full draw area.
 */
void force_viewport(const SDL_Rect&);

/**
 * Get the current viewport.
 *
 * @returns             The current viewport, in the coordinate space of
 *                      the original drawing surface
 */
SDL_Rect get_viewport();


/** A class to manage automatic restoration of the render target.
 *
 * While this can be constructed on its own, it is usually easier to
 * use the draw::set_render_target() utility function.
 */
class render_target_setter
{
public:
	explicit render_target_setter(const texture& t);
	~render_target_setter();

private:
	SDL_Texture* target_;
	SDL_Rect viewport_;
};

/**
 * Set the given texture as the active render target.
 *
 * All draw calls will draw to this texture until the returned object
 * goes out of scope. Do not retain the render_target_setter longer
 * than necessary.
 *
 * The provided texture must have been created with the
 * SDL_TEXTUREACCESS_TARGET access mode.
 *
 * @param t     The new render target. This must be a texture created
 *              with SDL_TEXTUREACCESS_TARGET.
 * @returns     A render_target_setter object. When this object is
 *              destroyed the render target will be restored to
 *              whatever it was before this call.
 */
render_target_setter set_render_target(const texture& t);


} // namespace draw
