/*
	Copyright (C) 2014 - 2025
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

#include "preferences/preferences.hpp"
#include "sdl/window.hpp"
#include "sdl/exception.hpp"
#include "sdl/surface.hpp"

#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_render.h>


#ifdef __ANDROID__
#include <SDL3/SDL_mouse.h>
#endif

namespace sdl
{

window::window(const std::string& title,
				 const int w,
				 const int h,
				 const uint32_t window_flags)
	: window_(SDL_CreateWindow(title.c_str(), w, h, window_flags | SDL_WINDOW_HIDDEN))
{
	if(!window_) {
		throw exception("Failed to create a SDL_Window object.", true);
	}

#ifdef _WIN32
	// SDL uses Direct3D v9 by default on Windows systems. However, returning
	// from the Windows lock screen causes issues with rendering. Resolution
	// is either to rebuild render textures on the SDL_EVENT_RENDER_TARGETS_RESET
	// event or use an alternative renderer that does not have this issue.
	// Suitable options are Direct3D v11+ or OpenGL.
	// See https://github.com/wesnoth/wesnoth/issues/8038 for details.
	// Note that SDL_HINT_RENDER_DRIVER implies SDL_HINT_RENDER_BATCHING is
	// disabled, according to https://discourse.libsdl.org/t/a-couple-of-questions-regarding-batching-in-sdl-2-0-10/26453/2.
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d11");
#endif

	SDL_PropertiesID props = SDL_CreateProperties();

	if(prefs::get().vsync()) {
		if(!SDL_SetNumberProperty(props, SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER, 1)) {
			throw exception("Failed to set vsync", true);
		};
	}

	if(!SDL_SetPointerProperty(props, SDL_PROP_RENDERER_CREATE_WINDOW_POINTER, window_)) {
		throw exception("Failed to set window pointer property", true);
	}

	if(!SDL_CreateRendererWithProperties(props)) {
		throw exception("Failed to create a SDL_Renderer object.", true);
	}

	// Set default blend mode to blend.
	SDL_SetRenderDrawBlendMode(*this, SDL_BLENDMODE_BLEND);

	// In fullscreen mode, do not minimize on focus loss.
	// Minimizing was reported as bug #1606 with blocker priority.
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

	fill(0,0,0);

	render();

	// If we didn't explicitly ask for the window to be hidden, show it
	if(!(window_flags & SDL_WINDOW_HIDDEN)) {
		SDL_ShowWindow(window_);
	}
}

window::~window()
{
	if(window_) {
		SDL_DestroyWindow(window_);
	}
}

void window::set_size(const int w, const int h)
{
#ifdef __ANDROID__
	SDL_RenderSetLogicalSize(SDL_GetRenderer(window_), w, h);
	SDL_WarpMouseInWindow(window_, w / 2, h / 2);
#else
	SDL_SetWindowSize(window_, w, h);
#endif
}

point window::get_size()
{
	point res;
#ifdef __ANDROID__
	SDL_RenderGetLogicalSize(SDL_GetRenderer(window_), &res.x, &res.y);
#else
	SDL_GetWindowSize(*this, &res.x, &res.y);
#endif

	return res;
}

point window::get_output_size()
{
	point res;
	SDL_GetCurrentRenderOutputSize(*this, &res.x, &res.y);

	return res;
}

void window::center()
{
#ifndef __ANDROID__
	SDL_SetWindowPosition(window_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
#endif
}

void window::maximize()
{
#ifndef __ANDROID__
	SDL_MaximizeWindow(window_);
#endif
}

void window::to_window()
{
#ifndef __ANDROID__
	SDL_SetWindowFullscreen(window_, false);
#endif
}

void window::restore()
{
#ifndef __ANDROID__
	SDL_RestoreWindow(window_);
#endif
}

void window::full_screen()
{
#ifndef __ANDROID__
	SDL_SetWindowFullscreen(window_, true);
#endif
}

void window::fill(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	SDL_SetRenderDrawColor(*this, r, g, b, a);
	if(!SDL_RenderClear(*this)) {
		throw exception("Failed to clear the SDL_Renderer object.", true);
	}
}

void window::render()
{
	SDL_RenderPresent(*this);
}

#ifndef __ANDROID__
void window::set_title(const std::string& title)
{
	SDL_SetWindowTitle(window_, title.c_str());
}

void window::set_icon(const surface& icon)
{
	SDL_SetWindowIcon(window_, icon);
}

void window::set_minimum_size(int min_w, int min_h)
{
	SDL_SetWindowMinimumSize(window_, min_w, min_h);
}
#else
void window::set_title(const std::string&) {};
void window::set_icon(const surface&) {};
void window::set_minimum_size(int, int) {};
#endif

uint32_t window::get_flags()
{
	return SDL_GetWindowFlags(window_);
}

int window::get_display_index()
{
	return SDL_GetDisplayForWindow(window_);
}

void window::set_logical_size(int w, int h)
{
	SDL_Renderer* r = SDL_GetRenderer(window_);
	// Non-integer scales are not currently supported.
	// This option makes things neater when window size is not a perfect
	// multiple of logical size, which can happen when manually resizing.
	SDL_SetRenderLogicalPresentation(r, w, h, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);
}

void window::set_logical_size(const point& p)
{
	set_logical_size(p.x, p.y);
}

point window::get_logical_size() const
{
	SDL_Renderer* r = SDL_GetRenderer(window_);
	int w, h;
	SDL_RendererLogicalPresentation mode = SDL_LOGICAL_PRESENTATION_INTEGER_SCALE;
	SDL_GetRenderLogicalPresentation(r, &w, &h, &mode);
	return {w, h};
}

void window::get_logical_size(int& w, int& h) const
{
	SDL_Renderer* r = SDL_GetRenderer(window_);
	SDL_RendererLogicalPresentation mode = SDL_LOGICAL_PRESENTATION_INTEGER_SCALE;
	SDL_GetRenderLogicalPresentation(r, &w, &h, &mode);
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
