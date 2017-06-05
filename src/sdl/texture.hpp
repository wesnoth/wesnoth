/*
   Copyright (C) 2017 the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <SDL_render.h>

class surface;

/**
 * Wrapper class to encapsulate creation and management of an SDL_Texture.
 * Supports free creation and creation from a surface.
 */
class texture
{
public:
	/** Default ctor. Texture will be a nullptr. */
	texture();

	/** Assigns the given texture to this one. */
	explicit texture(SDL_Texture* txt);

	/** Construct a texture from a surface. */
	explicit texture(const surface& surf);

	/** Construct a texture of the specified size and access type. */
	texture(int w, int h, SDL_TextureAccess access);

	~texture();

	/** Small wrapper that queries metadata about the provided texture. */
	struct info
	{
		explicit info(const texture& t);

		Uint32 format;
		int access;
		int w;
		int h;
	};

	/** Queries metadata about the texture, such as its dimensions. */
	const info get_info() const
	{
		return info(*this);
	}

	/** Destroys the managed texture and creates a new one. */
	void reset(int w, int h, SDL_TextureAccess access);

#if 0
	/** Move assignment. Frees the managed texture from the passed object. */
	texture& operator=(texture&& t);
#endif

	operator SDL_Texture*() const
	{
		return texture_;
	}

	bool null() const
	{
		return texture_ == nullptr;
	}

private:
	void finalize();

	void destroy_texture();

	SDL_Texture* texture_;
};

/**
 * Small RAII helper class to temporarily set the renderer target to a texture.
 */
class render_target_setter
{
public:
	explicit render_target_setter(texture& t);

	~render_target_setter();

private:
	SDL_Renderer* renderer_;
};
