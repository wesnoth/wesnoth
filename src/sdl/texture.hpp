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

#include <memory>

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

	texture(const texture&) = default;

	/** Assigns the given texture to this one. */
	explicit texture(SDL_Texture* txt);

	/** Construct a texture from a surface. */
	explicit texture(const surface& surf);

	/** Construct a texture of the specified size and access type. */
	texture(int w, int h, SDL_TextureAccess access);

	/** Small wrapper that queries metadata about the provided texture. */
	struct info
	{
		explicit info(SDL_Texture* t);

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

	/** Releases ownership of the managed texture and resets the ptr to null. */
	void reset();

	/** Releases ownership of the managed texture and creates a new one. */
	void reset(int w, int h, SDL_TextureAccess access);

	/** Replaces ownership of the managed texture with the given one. */
	void assign(SDL_Texture* t);

	texture& operator=(const texture& t) = default;

	/** Move assignment. Releases ownership of the managed texture from the passed object. */
	texture& operator=(texture&& t);

	operator SDL_Texture*() const
	{
		return texture_.get();
	}

	bool null() const
	{
		return texture_ == nullptr;
	}

private:
	void finalize();

	std::shared_ptr<SDL_Texture> texture_;
};
