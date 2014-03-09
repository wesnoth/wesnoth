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

#include "exceptions.hpp"
#include "log.hpp"

#include <SDL_render.h>

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)

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
{
	if(!window_) {
		ERR_DP << "Failed to create a SDL_Window object with error »"
			   << SDL_GetError() << "«.\n";

		throw game::error("");
	}

	if(!SDL_CreateRenderer(window_, -1, render_flags)) {
		ERR_DP << "Failed to create a SDL_Window object with error »"
			   << SDL_GetError() << "«.\n";

		throw game::error("");
	}
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
