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

#include "sdl/rect.hpp"
#include "sdl/texture.hpp"
#include "video.hpp"

#include <SDL_rect.h>
#include <SDL_render.h>

#include <cassert>

/**
 * Sets the renderer output target to the specified texture.
 * @todo: add support for restoring last target.
 */
class render_target_setter
{
public:
	explicit render_target_setter(texture& t)
		: renderer_(CVideo::get_singleton().get_renderer())
	{
		if(renderer_) {
			// Validate we can render to this texture.
			assert(t.get_info().access == SDL_TEXTUREACCESS_TARGET);

			SDL_SetRenderTarget(renderer_, t);
		}
	}

	~render_target_setter()
	{
		if(renderer_) {
			SDL_SetRenderTarget(renderer_, nullptr);
		}
	}

private:
	SDL_Renderer* renderer_;
};

using sdl_rect_getter = void (*)(SDL_Renderer*, SDL_Rect*);
using sdl_rect_setter = int (*)(SDL_Renderer*, const SDL_Rect*);

/**
 * Base class for renderer RAII helpers that operate on SDL_Rects.
 *
 * @tparam G             Getter function. Will fetch the current applicable rect.
 *                       That will be restored as the applicable state once this object is destroyed.
 * @tparam S             Setter function.
 */
template<sdl_rect_getter G, sdl_rect_setter S>
class render_raii_rect_setter_base
{
public:
	explicit render_raii_rect_setter_base(SDL_Rect* rect)
		: operate_(rect != nullptr)
		, last_rect_()
		, renderer_(CVideo::get_singleton().get_renderer())
	{
		if(renderer_ && operate_) {
			(*G)(renderer_, &last_rect_);
			(*S)(renderer_, rect);
		}
	}

	~render_raii_rect_setter_base()
	{
		if(renderer_ && operate_) {
			if(last_rect_ != sdl::empty_rect) {
				(*S)(renderer_, &last_rect_);
			} else {
				(*S)(renderer_, nullptr);
			}
		}
	}

private:
	const bool operate_;
	SDL_Rect last_rect_;
	SDL_Renderer* renderer_;
};

/**
 * Sets the renderer clip rect.
 */
using render_clip_rect_setter = render_raii_rect_setter_base<
	&SDL_RenderGetClipRect,
	&SDL_RenderSetClipRect>;

/**
 * Sets the renderer viewport rect.
 */
using render_viewport_setter = render_raii_rect_setter_base<
	&SDL_RenderGetViewport,
	&SDL_RenderSetViewport>;


/**
 * Set renderer drawing color.
 */
inline void set_draw_color(SDL_Renderer* renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

/**
 * Set renderer drawing color.
 */
inline void set_draw_color(SDL_Renderer* renderer, color_t color)
{
	set_draw_color(renderer, color.r, color.g, color.b, color.a);
}

/*
 * TEXTURE SETTERS =========================================================================
 * Need to decide if these need their own file.
 */

inline void set_texture_alpha(texture& t, uint8_t amount)
{
	SDL_SetTextureAlphaMod(t, amount);
}

inline void set_texture_blend_color(texture& t, uint8_t r, uint8_t g, uint8_t b)
{
	SDL_SetTextureColorMod(t, r, g, b);
}

inline void set_texture_blend_mode(texture& t, SDL_BlendMode mode)
{
	SDL_SetTextureBlendMode(t, mode);
}

/**
 * Sets the texture scale quality. Note this should be called *before* a texture
 * is created, since the hint has no effect on existing textures or render ops.
 *
 * @param value               The scaling mode. Use either 'linear' or 'nearest'.
 */
inline void set_texture_scale_quality(const std::string& value)
{
	SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, value.c_str(), SDL_HINT_OVERRIDE);
}
