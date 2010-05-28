/* $Id$ */
/*
   Copyright (C) 2010 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file arrow.cpp
 * Method bodies for the arrow class.
 */

#include "arrow.hpp"

#include "foreach.hpp"

arrow::arrow(): layer_(display::LAYER_ARROWS)
{
	color_.b = 0;
	color_.g = 0;
	color_.r = 0;
}

void arrow::set_path(const std::list<map_location> path)
{
	previous_path_ = path_;
	path_ = path;
	update_symbols();
}

void arrow::set_color(const SDL_Color color)
{
	color_ = color;
	update_symbols();
}

void arrow::set_layer(const display::tdrawing_layer & layer)
{
	layer_ = layer;
}

const std::list<map_location> & arrow::get_path() const
{
	return path_;
}


const std::list<map_location> & arrow::get_previous_path() const
{
	return previous_path_;
}

void arrow::draw_hex(const map_location & hex)
{

}

void arrow::update_symbols()
{
	//TODO: use the proper images instead of this placeholder
	surface test_picture = image::get_image("footsteps/teleport-in.png", image::SCALED_TO_HEX);

	foreach(map_location loc, path_)
	{
		symbols_map_[loc] = test_picture;
	}
}
