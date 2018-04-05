/*
   Copyright (C) 2018 by Jyrki Vesterinen <sandgtx@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "ogl/draw_op.hpp"
#include "ogl/texture.hpp"

#include <GL/glew.h>

#include <utility>

namespace gl
{

class sprite
{
public:
	/** Texture name. */
	const GLuint texture_name;

	/** X coordinate of left edge. Normalized to the range [0, 1]. */
	float x_min;
	/** X coordinate of right edge. Normalized to the range [0, 1]. */
	float x_max;
	/** Y coordinate of bottom edge. Normalized to the range [0, 1]. */
	float y_min;
	/** Y coordinate of top edge. Normalized to the range [0, 1]. */
	float y_max;

	/** X coordinate of left edge as pixels. */
	unsigned int x_min_px;
	/** X coordinate of right edge as pixels. */
	unsigned int x_max_px;
	/** Y coordinate of bottom edge as pixels. */
	unsigned int y_min_px;
	/** Y coordinate of top edge as pixels. */
	unsigned int y_max_px;

	/** Half of the width of the sprite, in pixels. */
	float half_width;

	/** Half of the height of the sprite, in pixels. */
	float half_height;

	/** Constructor. */
	sprite(const texture& tex) : texture_name(tex.get_name())
	{

	}

	/** Calculates normalized texture coordinates from pixel coordinates and texture size. */
	void normalize_coordinates(const std::pair<int, int>& texture_size);

	/** Generates draw operations which draw the sprite.
	The sprite is anchored to center.
	@param x X coordinate for center of the sprite.
	@param y Y coordinate for center of the sprite.
	@param out An output iterator for the container where the draw operations will be pushed.
	@todo Scaling support. */
	template<typename T>
	void to_draw_ops(float x, float y, T out) const
	{
		draw_op op;
		op.texture_name = texture_name;

		op.vertices[0].x = x - half_width;
		op.vertices[0].y = y - half_height;
		op.vertices[1].x = x + half_width;
		op.vertices[1].y = y - half_height;
		op.vertices[2].x = x + half_width;
		op.vertices[2].y = y + half_height;
		op.vertices[0].u = x_min;
		op.vertices[0].v = y_min;
		op.vertices[1].u = x_max;
		op.vertices[1].v = y_min;
		op.vertices[2].u = x_max;
		op.vertices[2].v = y_max;

		// Assigning to the iterator pushes the draw operation.
		// It pushes a copy, allowing us to modify the original.
		out = op;

		op.vertices[0].x = x - half_width;
		op.vertices[0].y = y - half_height;
		op.vertices[1].x = x + half_width;
		op.vertices[1].y = y + half_height;
		op.vertices[2].x = x - half_width;
		op.vertices[2].y = y + half_height;
		op.vertices[0].u = x_min;
		op.vertices[0].v = y_min;
		op.vertices[1].u = x_max;
		op.vertices[1].v = y_max;
		op.vertices[2].u = x_min;
		op.vertices[2].v = y_max;

		out = op;
	}
};

}
