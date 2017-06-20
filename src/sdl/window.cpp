/*
   Copyright (C) 2014 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "sdl/window.hpp"

#include "ogl/utils.hpp"
#include "sdl/surface.hpp"
#include "sdl/exception.hpp"

namespace sdl
{

window::window(const std::string& title,
				 const int x,
				 const int y,
				 const int w,
				 const int h,
				 const Uint32 window_flags,
				 const Uint32 render_flags)
	: window_(SDL_CreateWindow(title.c_str(), x, y, w, h, window_flags))
	, info_()
{
	if(!window_) {
		throw exception("Failed to create a SDL_Window object.", true);
	}

#ifndef USE_GL_RENDERING
	if(!SDL_CreateRenderer(window_, -1, render_flags)) {
		throw exception("Failed to create a SDL_Renderer object.", true);
	}

	if(SDL_GetRendererInfo(*this, &info_) != 0) {
		throw exception("Failed to retrieve the information of the renderer.",
						 true);
	}

	if(info_.num_texture_formats == 0) {
		throw exception("The renderer has no texture information available.\n",
						 false);
	}

	// Set default blend mode to blend.
	SDL_SetRenderDrawBlendMode(*this, SDL_BLENDMODE_BLEND);

	// Use linear scaling when rendering, if applicable.
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	fill(0,0,0);

	render();
#endif
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
}

void window::to_window()
{
	SDL_SetWindowFullscreen(window_, 0);
}

void window::restore()
{
	SDL_RestoreWindow(window_);
}

void window::full_screen()
{
	SDL_SetWindowFullscreen(window_, SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void window::fill(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	SDL_SetRenderDrawColor(*this, r, g, b, a);
	if(SDL_RenderClear(*this) != 0) {
		throw exception("Failed to clear the SDL_Renderer object.",
						 true);
	}
}

void window::render()
{
#ifdef USE_GL_RENDERING
	SDL_GL_SwapWindow(*this);
#else
	SDL_RenderPresent(*this);
#endif
}

void window::set_title(const std::string& title)
{
	SDL_SetWindowTitle(window_, title.c_str());
}

void window::set_icon(const surface& icon)
{
	SDL_SetWindowIcon(window_, icon);
}

int window::get_flags()
{
	return SDL_GetWindowFlags(window_);
}

void window::set_minimum_size(int min_w, int min_h)
{
	SDL_SetWindowMinimumSize(window_, min_w, min_h);
}

int window::get_display_index()
{
	return SDL_GetWindowDisplayIndex(window_);
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
