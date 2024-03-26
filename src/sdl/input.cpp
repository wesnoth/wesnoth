/*
	Copyright (C) 2022 - 2024
	by Thomas Iorns <mesilliac@tomanui.nz>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "sdl/input.hpp"

#include "sdl/point.hpp"
#include "video.hpp"

#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_keyboard.h>

namespace sdl
{

uint32_t get_mouse_state(float* x, float* y)
{
	uint32_t buttons = SDL_GetMouseState(x, y);

	if (video::headless()) {
		return buttons;
	}

	// The game canvas may be offset inside the window,
	// as well as potentially having a different size.
	rect input_area = video::input_area();
	*x -= input_area.x;
	*y -= input_area.y;

	// Translate to game-native coordinates
	point canvas_size = video::game_canvas_size();
	*x = (*x * canvas_size.x) / input_area.w;
	*y = (*y * canvas_size.y) / input_area.h;

	return buttons;
}

uint32_t get_mouse_button_mask()
{
	return SDL_GetMouseState(nullptr, nullptr);
}

point get_mouse_location()
{
	float x;
	float y;
	get_mouse_state(&x, &y);
	return {static_cast<int>(x), static_cast<int>(y)};
}

unsigned get_mods()
{
	unsigned mods = SDL_GetModState();

	// Filter for only the mods we use: shift, ctrl, alt, gui
	mods &= SDL_KMOD_SHIFT | SDL_KMOD_CTRL | SDL_KMOD_ALT | SDL_KMOD_GUI;

	// Set both left and right modifiers if either is active
	if(mods & SDL_KMOD_SHIFT) {
		mods |= SDL_KMOD_SHIFT;
	}

	if(mods & SDL_KMOD_CTRL) {
		mods |= SDL_KMOD_CTRL;
	}

	if(mods & SDL_KMOD_ALT)
		mods |= SDL_KMOD_ALT;

	if(mods & SDL_KMOD_GUI) {
		mods |= SDL_KMOD_GUI;
	}

	return mods;
}

} // namespace sdl
