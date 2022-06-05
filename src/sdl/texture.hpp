/*
	Copyright (C) 2017 - 2022
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

#include <SDL2/SDL_render.h>

#include <memory>

class surface;
struct point;
struct color_t;

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

	/**
	 * Construct a texture from a surface.
	 *
	 * @param surf                  The surface to copy.
	 * @param linear_interpolation  If true this texture will use linear
	 *                              interpolation when drawing. Otherwise
	 *                              nearest-neighbour interpolation is used.
	 *                              This does not affect texture creation,
	 *                              only later application.
	 */
	explicit texture(const surface& surf, bool linear_interpolation = false);

	/** Construct a texture of the specified size and access type. */
	texture(int w, int h, SDL_TextureAccess access);

	/** Small wrapper that queries metadata about the provided texture. */
	struct info
	{
		explicit info(SDL_Texture* t);

		uint32_t format;
		int access;
		int w;
		int h;
	};

	/** Queries metadata about the texture, such as its dimensions. */
	const info get_info() const
	{
		return info(*this);
	}

	/**
	 * The draw-space width of the texture, in pixels.
	 *
	 * This will usually be the real texture width in pixels, but may
	 * differ in some situations. For high-DPI text, for example,
	 * it will usually be equal to get_info().w / pixel_scale.
	 */
	int w() const { return w_; }

	/**
	 * The draw-space height of the texture, in pixels.
	 *
	 * This will usually be the real texture height in pixels, but may
	 * differ in some situations. For high-DPI text, for example,
	 * it will usually be equal to get_info().h / pixel_scale.
	 */
	int h() const { return h_; }

	/** Set the intended width of the texture, in draw-space. */
	void set_draw_width(int w) { w_ = w; }

	/** Set the intended height of the texture, in draw-space. */
	void set_draw_height(int h) { h_ = h; }

	/** Set the intended size of the texture, in draw-space. */
	void set_draw_size(int w, int h) { w_ = w; h_ = h; }
	void set_draw_size(const point& p);

	/** Alpha modifier. Multiplies alpha when drawing. */
	void set_alpha_mod(uint8_t alpha);
	uint8_t get_alpha_mod();

	/** Blend mode. Modifies how draw operations are applied. */
	void set_blend_mode(SDL_BlendMode);
	SDL_BlendMode get_blend_mode();

	/** Colour modifier. Multiplies each colour component when drawing. */
	void set_color_mod(uint8_t r, uint8_t g, uint8_t b);
	void set_color_mod(color_t);
	color_t get_color_mod();

	/** Releases ownership of the managed texture and resets the ptr to null. */
	void reset();

	/** Releases ownership of the managed texture and creates a new one. */
	void reset(int width, int height, SDL_TextureAccess access);

	/** Replaces ownership of the managed texture with the given one. */
	void assign(SDL_Texture* t);

	texture& operator=(const texture& t) = default;

	/** Move assignment. Releases ownership of the managed texture from the passed object. */
	texture& operator=(texture&& t);

	operator SDL_Texture*() const
	{
		return texture_.get();
	}

	explicit operator bool() const
	{
		return texture_ != nullptr;
	}

private:
	void finalize();

	std::shared_ptr<SDL_Texture> texture_;
	int w_, h_;
};
