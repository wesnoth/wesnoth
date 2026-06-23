/*
	Copyright (C) 2026 - 2026
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

#include <SDL3/SDL_properties.h>

class sdl3_properties
{
public:
	sdl3_properties()
		: props_(SDL_CreateProperties())
	{
	}

	~sdl3_properties()
	{
		SDL_DestroyProperties(props_);
	}

	SDL_PropertiesID id() const { return props_; }

	operator SDL_PropertiesID() const { return props_; }

private:
	SDL_PropertiesID props_;
};
