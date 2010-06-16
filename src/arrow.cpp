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
#include "log.hpp"
#include "map_location.hpp"
#include "resources.hpp"

static lg::log_domain log_arrows("arrows");
#define ERR_ARR LOG_STREAM(err, log_arrows)
#define WRN_ARR LOG_STREAM(warn, log_arrows)
#define LOG_ARR LOG_STREAM(info, log_arrows)
#define DBG_ARR LOG_STREAM(debug, log_arrows)

#define SCREEN ((display*)resources::screen)

arrow::arrow()
	: layer_(display::LAYER_ARROWS)
	, color_("red")
	, style_("")
	, alpha_()
	, path_()
	, previous_path_()
	, symbols_map_()
{
}

arrow::~arrow()
{
	if (SCREEN)
	{
		invalidate_arrow_path(path_);
		SCREEN->remove_arrow(*this);
	}
}

bool arrow::set_path(const arrow_path_t &path)
{
	if (valid_path(path))
	{
		previous_path_ = path_;
		path_ = path;
		update_symbols(previous_path_);
		return true;
	}
	else
	{
		return false;
	}
}

void arrow::reset()
{
	invalidate_arrow_path(path_);
	symbols_map_.clear();
	notify_arrow_changed();
	path_.clear();
	previous_path_.clear();
}

void arrow::set_color(const std::string& color)
{
	color_ = color;
	if (valid_path(path_))
	{
		update_symbols(path_);
	}
}

void arrow::set_style(const std::string& style)
{
	style_ = style;
	if (valid_path(path_))
	{
		update_symbols(path_);
	}
}

void arrow::set_layer(const display::tdrawing_layer & layer)
{
	layer_ = layer;
	if (valid_path(path_))
	{
		invalidate_arrow_path(path_);
		notify_arrow_changed();
	}
}

void arrow::set_alpha(double alpha)
{
	alpha_ = ftofxp(alpha);
	if (valid_path(path_))
	{
		update_symbols(path_);
	}
}

const arrow_path_t & arrow::get_path() const
{
	return path_;
}

const arrow_path_t & arrow::get_previous_path() const
{
	return previous_path_;
}

void arrow::draw_hex(const map_location & loc)
{
	if(!SCREEN) return;

	SCREEN->render_image(SCREEN->get_location_x(loc), SCREEN->get_location_y(loc), layer_,
				loc, image::get_image(symbols_map_[loc], image::SCALED_TO_ZOOM), false, false, alpha_);
}

bool arrow::valid_path(arrow_path_t path) const
{
	if (path.size() >= 2)
		return true;
	else
		return false;
}

void arrow::update_symbols(arrow_path_t old_path)
{
	if (!valid_path(path_))
	{
		WRN_ARR << "arrow::update_symbols called with invalid path\n";
		return;
	}

	foreach(map_location loc, old_path)
	{
		symbols_map_.erase(loc);
	}

	invalidate_arrow_path(old_path);

	const std::string mods = "~RC(FF00FF>"+ color_ + ")"; //magenta to current color

	const std::string dirname = "arrows/";
	std::string style = style_;
	if (!style.empty())
		style = style + "/";
	map_location::DIRECTION exit_dir = map_location::NDIRECTIONS;
	map_location::DIRECTION enter_dir = map_location::NDIRECTIONS;
	std::string prefix = "";
	std::string suffix = "";
	std::string image_filename = "";
	bool begin = false;
	bool end = false;
	bool teleport_out = false;
	bool teleport_in = false;

	arrow_path_t::iterator hex;
	for (hex = path_.begin(); hex != path_.end() - 1; ++hex)
	{
		exit_dir = map_location::NDIRECTIONS;
		enter_dir = map_location::NDIRECTIONS;
		prefix = "";
		suffix = "";
		image_filename = "";
		begin = end = false;
		// teleport in if we teleported out last hex
		teleport_in = teleport_out;
		teleport_out = false;

		// Determine some special cases
		if (hex == path_.begin())
			begin = true;
		if (hex == path_.end() - 2)
			end = true;
		if (!tiles_adjacent(*hex, *(hex + 1)))
			teleport_out = true;

		// Now figure out the actual images
		if (teleport_out)
		{
			image_filename = "footsteps/teleport-out.png";
		}
		else if (teleport_in)
		{
			image_filename = "footsteps/teleport-in.png";
		}
		else if (begin)
		{
			prefix = "start";
			exit_dir = hex->get_relative_dir(*(hex+1));
			suffix = map_location::write_direction(exit_dir);
			if (end)
			{
				suffix = suffix + "_end";
			}
			image_filename = dirname + style + prefix + "-" + suffix + ".png";
		}
		else
		{
			enter_dir = hex->get_relative_dir(*(hex-1));
			exit_dir = hex->get_relative_dir(*(hex+1));
			std::string enter, exit;
			enter = map_location::write_direction(enter_dir);
			exit = map_location::write_direction(exit_dir);
			if (end)
			{
				exit = exit + "_end";
			}

			assert(abs(enter_dir - exit_dir) > 1); //impossible turn?
			if (enter_dir < exit_dir)
			{
				prefix = enter;
				suffix = exit;
			}
			else //(enter_dir > exit_dir)
			{
				prefix = exit;
				suffix = enter;
			}
			image_filename = dirname + style + prefix + "-" + suffix + ".png";
		}

		if (image_filename != "")
		{
			image::locator image = image::locator(image_filename, mods);
			if (!image.file_exists())
				{
					ERR_ARR << "Image " << image_filename << " not found.\n";
					image = image::locator("misc/missing-image.png");
				}
			symbols_map_[*hex] = image;
		}
	}

	invalidate_arrow_path(path_);

	notify_arrow_changed();
}

void arrow::invalidate_arrow_path(arrow_path_t path)
{
	if(!SCREEN) return;

	foreach(const map_location& loc, path)
	{
		SCREEN->invalidate(loc);
	}
}

void arrow::notify_arrow_changed()
{
	if(!SCREEN) return;

	SCREEN->update_arrow(*this);
}
