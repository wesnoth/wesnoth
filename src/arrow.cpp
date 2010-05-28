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
#include "arrow_observer.hpp"

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
	notify_arrow_changed();
}

void arrow::set_color(const SDL_Color color)
{
	color_ = color;
	notify_arrow_changed();
}

void arrow::set_layer(const display::tdrawing_layer & layer)
{
	layer_ = layer;
	notify_arrow_changed();
}

void arrow::add_observer(arrow_observer & observer)
{
	observers_.push_back(&observer);
}

void arrow::remove_observer(arrow_observer & observer)
{
	observers_.remove(&observer);
}

std::list<arrow_image> arrow::get_images() const
{
	//TODO: return the proper images instead of this placeholder
	std::list<arrow_image> images;
	surface test_picture = image::get_image("footsteps/teleport-in.png", image::SCALED_TO_HEX);

	foreach(map_location loc, path_)
	{
		images.push_back(arrow_image(loc, test_picture));
	}

	return images;
}

const std::list<map_location> & arrow::get_path() const
{
	return path_;
}


const std::list<map_location> & arrow::get_previous_path() const
{
	return previous_path_;
}

void arrow::notify_arrow_changed() {
	foreach(arrow_observer* observer, observers_)
	{
		observer->arrow_changed(*this);
	}
}

void arrow::notify_arrow_deleted() {
	foreach(arrow_observer* observer, observers_)
	{
		observer->arrow_deleted(*this);
	}
}
