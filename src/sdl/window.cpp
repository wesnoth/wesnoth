/*
	Copyright (C) 2014 - 2022
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "sdl/surface.hpp"
#include "sdl/window.hpp"
#include "sdl/input.hpp"

#include "sdl/exception.hpp"

#include <SDL2/SDL_render.h>

namespace sdl
{

window::window(const std::string& title,
				 const int x,
				 const int y,
				 const int w,
				 const int h,
				 const uint32_t window_flags,
				 const uint32_t render_flags)
	: window_(SDL_CreateWindow(title.c_str(), x, y, w, h, window_flags))
	, pixel_format_(SDL_PIXELFORMAT_UNKNOWN)
{
	if(!window_) {
		throw exception("Failed to create a SDL_Window object.", true);
	}

#if SDL_VERSION_ATLEAST(2, 0, 10)
	// Rendering in batches (for efficiency) is enabled by default from SDL 2.0.10
	// The way Wesnoth uses SDL as of September 2019 does not work well with this rendering mode (eg story-only scenarios)
	SDL_SetHint(SDL_HINT_RENDER_BATCHING, "0");
#endif

	if(!SDL_CreateRenderer(window_, -1, render_flags)) {
		throw exception("Failed to create a SDL_Renderer object.", true);
	}

	SDL_RendererInfo info;
	if(SDL_GetRendererInfo(*this, &info) != 0) {
		throw exception("Failed to retrieve the information of the renderer.",
						 true);
	}

	if(info.num_texture_formats == 0) {
		throw exception("The renderer has no texture information available.\n",
						 false);
	}

	if((info.flags & SDL_RENDERER_TARGETTEXTURE) == 0) {
		throw exception("Render-to-texture not supported or enabled!", false);
	}

	// Set default blend mode to blend.
	SDL_SetRenderDrawBlendMode(*this, SDL_BLENDMODE_BLEND);

	// In fullscreen mode, do not minimize on focus loss.
	// Minimizing was reported as bug #1606 with blocker priority.
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

	pixel_format_ = info.texture_formats[0];

	fill(0,0,0);

	// Now that we have a window and renderer we can scale input correctly.
	update_input_dimensions(get_logical_size(), get_size());

	render();
}

window::~window()
{
	if(window_) {
		SDL_DestroyWindow(window_);
	}
}

void window::set_size(const int w, const int h)
{
	SDL_SetWindowSize(window_, w, h);
	update_input_dimensions(get_logical_size(), get_size());
}

SDL_Point window::get_size()
{
	SDL_Point res;
	SDL_GetWindowSize(*this, &res.x, &res.y);

	return res;
}

SDL_Point window::get_output_size()
{
	SDL_Point res;
	SDL_GetRendererOutputSize(*this, &res.x, &res.y);

	return res;
}

void window::center()
{
	SDL_SetWindowPosition(window_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

void window::maximize()
{
	SDL_MaximizeWindow(window_);
	update_input_dimensions(get_logical_size(), get_size());
}

void window::to_window()
{
	SDL_SetWindowFullscreen(window_, 0);
	update_input_dimensions(get_logical_size(), get_size());
}

void window::restore()
{
	SDL_RestoreWindow(window_);
	update_input_dimensions(get_logical_size(), get_size());
}

void window::full_screen()
{
	SDL_SetWindowFullscreen(window_, SDL_WINDOW_FULLSCREEN_DESKTOP);
	update_input_dimensions(get_logical_size(), get_size());
}

void window::fill(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	SDL_SetRenderDrawColor(*this, r, g, b, a);
	if(SDL_RenderClear(*this) != 0) {
		throw exception("Failed to clear the SDL_Renderer object.",
						 true);
	}
}

void window::render()
{
	SDL_RenderPresent(*this);
}

void window::set_title(const std::string& title)
{
	SDL_SetWindowTitle(window_, title.c_str());
}

void window::set_icon(const surface& icon)
{
	SDL_SetWindowIcon(window_, icon);
}

uint32_t window::get_flags()
{
	return SDL_GetWindowFlags(window_);
}

void window::set_minimum_size(int min_w, int min_h)
{
	SDL_SetWindowMinimumSize(window_, min_w, min_h);
	// Can this change the size of the window?
	update_input_dimensions(get_logical_size(), get_size());
}

int window::get_display_index()
{
	return SDL_GetWindowDisplayIndex(window_);
}

void window::set_logical_size(int w, int h)
{
	SDL_Renderer* r = SDL_GetRenderer(window_);
	SDL_RenderSetLogicalSize(r, w, h);
	update_input_dimensions(get_logical_size(), get_size());
}

SDL_Point window::get_logical_size() const
{
	SDL_Renderer* r = SDL_GetRenderer(window_);
	int w, h;
	SDL_RenderGetLogicalSize(r, &w, &h);
	return {w, h};
}

void window::get_logical_size(int& w, int& h) const
{
	SDL_Renderer* r = SDL_GetRenderer(window_);
	SDL_RenderGetLogicalSize(r, &w, &h);
}

uint32_t window::pixel_format()
{
	return pixel_format_;
}

window::operator SDL_Window*()
{
	return window_;
}

window::operator SDL_Renderer*()
{
	return SDL_GetRenderer(window_);
}

} // namespace sdl
