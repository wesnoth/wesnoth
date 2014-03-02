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

#ifndef SDL_COMPAT_HPP_INCLUDED
#define SDL_COMPAT_HPP_INCLUDED

/**
 * @file
 * Compatibility layer for using SDL 1.2 and 2.0.
 *
 * Only has some minimal wrapping for the changes. The layer can be removed once
 * we switch to SDL 2.0.
 */

#include <SDL_version.h>

#if SDL_VERSION_ATLEAST(2, 0, 0)

#define SDLKey SDL_Keycode
#define SDLMod SDL_Keymod
#define SDL_GetKeyState SDL_GetKeyboardState
#define SDLK_LMETA SDLK_LGUI
#define SDLK_RMETA SDLK_RGUI
#define KMOD_LMETA KMOD_LGUI
#define KMOD_RMETA KMOD_RGUI
#define KMOD_META KMOD_GUI
#define SDL_FULLSCREEN SDL_WINDOW_FULLSCREEN_DESKTOP
#define SDL_EVENTMASK(EVENT) EVENT, EVENT
#define SDL_GetAppState CVideo::window_state

#endif

#endif
