/*
   Copyright (C) 2014 by Mark de Wever <koraq@xs4all.nl>
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
#include "sdl/texture.hpp"

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

	clear();
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

void twindow::full_screen()
{
	/** @todo Implement. */
}

void twindow::clear()
{
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

ttexture twindow::create_texture(const int access, const int w, const int h)
{
	return ttexture(*SDL_GetRenderer(window_), pixel_format_, access, w, h);
}

ttexture twindow::create_texture(const int access,
								 SDL_Surface* source_surface__)
{
	return ttexture(*SDL_GetRenderer(window_), access, source_surface__);
}

ttexture twindow::create_texture(const int access, const surface& surface)
{
	return ttexture(*SDL_GetRenderer(window_), access, surface);
}

void twindow::draw(ttexture& texture, const int x, const int y)
{
	texture.draw(*SDL_GetRenderer(window_), x, y);
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
