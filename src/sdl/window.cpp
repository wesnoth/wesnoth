/*
   Copyright (C) 2014 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

#if SDL_VERSION_ATLEAST(2, 0, 0)

#include "sdl/exception.hpp"
#include "sdl/image.hpp"

#include <SDL_render.h>

namespace sdl
{

twindow::twindow(const std::string& title,
				 const int x,
				 const int y,
				 const int w,
				 const int h,
				 const Uint32 window_flags,
				 const Uint32 render_flags)
	: window_(SDL_CreateWindow(title.c_str(), x, y, w, h, window_flags))
	, pixel_format_(SDL_PIXELFORMAT_UNKNOWN)
{
	if(!window_) {
		throw texception("Failed to create a SDL_Window object.", true);
	}

	if(!SDL_CreateRenderer(window_, -1, render_flags)) {
		throw texception("Failed to create a SDL_Renderer object.", true);
	}

	SDL_RendererInfo info;
	if(SDL_GetRendererInfo(*this, &info) != 0) {
		throw texception("Failed to retrieve the information of the renderer.",
						 true);
	}

	if(info.num_texture_formats == 0) {
		throw texception("The renderer has no texture information available.\n",
						 false);
	}

	pixel_format_ = info.texture_formats[0];

	fill(0,0,0);
}

twindow::~twindow()
{
	if(window_) {
		SDL_DestroyWindow(window_);
	}
}

void twindow::set_size(const int w, const int h)
{
	SDL_SetWindowSize(window_, w, h);
}

void twindow::center()
{
	SDL_SetWindowPosition(window_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

void twindow::maximize()
{
	SDL_MaximizeWindow(window_);
}

void twindow::to_window()
{
	SDL_SetWindowFullscreen(window_, 0);
}

void twindow::restore()
{
	SDL_RestoreWindow(window_);
}

void twindow::full_screen()
{
	SDL_SetWindowFullscreen(window_, SDL_WINDOW_FULLSCREEN_DESKTOP);
}

void twindow::fill(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	SDL_SetRenderDrawColor(*this, r, g, b, a);
	if(SDL_RenderClear(*this) != 0) {
		throw texception("Failed to clear the SDL_Renderer object.",
						 true);
	}
}

void twindow::render()
{
	SDL_RenderPresent(*this);
}

void twindow::set_title(const std::string& title)
{
	SDL_SetWindowTitle(window_, title.c_str());
}

void twindow::set_icon(const surface& icon)
{
	SDL_SetWindowIcon(window_, icon);
}

int twindow::get_flags()
{
	return SDL_GetWindowFlags(window_);
}

void twindow::set_minimum_size(int min_w, int min_h)
{
	SDL_SetWindowMinimumSize(window_, min_w, min_h);
}

twindow::operator SDL_Window*()
{
	return window_;
}

twindow::operator SDL_Renderer*()
{
	return SDL_GetRenderer(window_);
}

} // namespace sdl

#endif
