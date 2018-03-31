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

#include "sdl/surface.hpp"

#include <GL/glew.h>

#include <utility>

namespace gl
{
/// Thin wrapper for textures.
/// All textures are two-dimensional and in the RGBA8 format.
class texture
{
public:
	static const int MAX_DIMENSION = 8192;

	/** Constructor.
	Initial texture size is 0x0. Use set_size() to actually allocate memory for
	the texture. */
	texture()
	{
		glGenTextures(1, &name_);
		// Bind the texture as 2D to make it two-dimensional.
		glBindTexture(GL_TEXTURE_2D, name_);
		// Unbind the 2D texture target.
		glBindTexture(GL_TEXTURE_2D, 0u);
	}

	/// Destructor.
	~texture()
	{
		glDeleteTextures(1, &name_);
	}

	/** @return the OpenGL texture name.
	It can be passed directly to the OpenGL API.
	The name of a texture does not change and can be safely cached.
	*/
	GLuint get_name() const
	{
		return name_;
	}

	/// @return the size of the texture.
	std::pair<int, int> get_size() const;

	/** Resizes the texture.
	@param size Desired texture size.
	Invalidates existing content of the texture, if any.
	@note Maximum texture size is 8192x8192. It's enforced both ways: Wesnoth refuses
	to launch if the hardware doesn't support 8192x8192 textures, but it doesn't allow
	larger textures even if the hardware supports them. */
	void set_size(const std::pair<int, int>& size);

	/** Uploads the provided texture data.
	@param pixels Texture data. */
	void set_pixels(surface pixels);

private:
	/// OpenGL texture name.
	GLuint name_;

	/// Checks that the SDL surface format matches the format we expect.
	static bool check_format(const SDL_PixelFormat& format);
};
}
