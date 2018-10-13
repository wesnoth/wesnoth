/*
   Copyright (C) 2018 by Jyrki Vesterinen <sandgtx@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "ogl/sprite.hpp"
#include "ogl/texture.hpp"
#include "sdl/surface.hpp"
#include "thread_pool.hpp"

#include <SDL.h>

#include <list>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace gl
{
class texture_atlas
{
public:
	texture_atlas()
		: texture_()
		, sprites_()
		, sprites_by_name_()
		, free_rectangles_()
	{}

	texture_atlas(const texture_atlas&) = delete;

	/** Initializes the texture atlas.
	Clears existing sprites, if any.
	@param images Images to pack into the texture atlas.
	As full image paths including IPF chains.
	@param thread_pool Thread pool that will be used to load the images. */
	void init(const std::vector<std::string>& images, thread_pool& thread_pool);

	/** Gets a sprite by name.
	For performance, do not call this in every frame!
	Cache the sprite instead. */
	const sprite& get_sprite(const std::string& name) const
	{
		return *sprites_by_name_.at(name);
	}

	texture_atlas& operator=(texture_atlas&) = delete;

	/// Thrown if packing sprites fails.
	class packing_error : public std::exception
	{

	};

private:
	struct sprite_data
	{
		std::string name;
		surface surf;
		unsigned int x_min;
		unsigned int x_max;
		unsigned int y_min;
		unsigned int y_max;

		bool operator<(const sprite_data& other) const;
	};

	texture texture_;
	std::list<sprite> sprites_;
	std::unordered_map<std::string, const sprite*> sprites_by_name_;
	std::vector<SDL_Rect> free_rectangles_;

	void pack_sprites_wrapper(std::vector<sprite_data>& sprites);
	void pack_sprites(std::vector<sprite_data>& sprites);
	void place_sprite(sprite_data& sprite);
	static void load_image(sprite_data& sprite);
	static void apply_IPFs(sprite_data& sprite);
	/// @return true if it would be better to place the @param sprite to @param rect_a than @param rect_b.
	static bool better_fit(const sprite_data& sprite, const SDL_Rect& rect_a, const SDL_Rect& rect_b);
	static std::pair<SDL_Rect, SDL_Rect> split_rectangle(const SDL_Rect& rectangle, const sprite_data& sprite);
	static std::pair<int, int> calculate_initial_texture_size(unsigned int combined_sprite_size);
};
}
