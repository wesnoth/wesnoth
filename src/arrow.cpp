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
 * @file
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
	, style_(STYLE_STANDARD)
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

void arrow::set_path(arrow_path_t const& path)
{
	if (valid_path(path))
	{
		previous_path_ = path_;
		path_ = path;
		update_symbols(previous_path_);
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

void arrow::set_color(std::string const& color)
{
	color_ = color;
	if (valid_path(path_))
	{
		update_symbols(path_);
	}
}

std::string const arrow::STYLE_STANDARD = "standard";
std::string const arrow::STYLE_HIGHLIGHTED = "highlighted";
std::string const arrow::STYLE_FOCUS = "focus";

void arrow::set_style(const std::string& style)
{
	style_ = style;
	if (valid_path(path_))
	{
		update_symbols(path_);
	}
}

void arrow::set_layer(display::tdrawing_layer const& layer)
{
	layer_ = layer;
	if (valid_path(path_))
	{
		invalidate_arrow_path(path_);
		notify_arrow_changed();
	}
}

arrow_path_t const& arrow::get_path() const
{
	return path_;
}

arrow_path_t const& arrow::get_previous_path() const
{
	return previous_path_;
}

bool arrow::path_contains(map_location const& hex) const
{
	bool contains = symbols_map_.find(hex) != symbols_map_.end();
	return contains;
}

void arrow::draw_hex(map_location const& hex)
{
	if(SCREEN && path_contains(hex))
	{
		SCREEN->render_image(SCREEN->get_location_x(hex), SCREEN->get_location_y(hex), layer_,
					hex, image::get_image(symbols_map_[hex], image::SCALED_TO_ZOOM));
	}
}

bool arrow::valid_path(arrow_path_t path)
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

	symbols_map_.clear();

	invalidate_arrow_path(old_path);

	std::string const mods = "~RC(FF00FF>"+ color_ + ")"; //magenta to current color

	std::string const dirname = "arrows/";
	map_location::DIRECTION exit_dir = map_location::NDIRECTIONS;
	map_location::DIRECTION enter_dir = map_location::NDIRECTIONS;
	std::string prefix = "";
	std::string suffix = "";
	std::string image_filename = "";
	arrow_path_t::const_iterator const arrow_start_hex = path_.begin();
	arrow_path_t::const_iterator const arrow_pre_end_hex = path_.end() - 2;
	arrow_path_t::const_iterator const arrow_end_hex = path_.end() - 1;
	bool start = false;
	bool pre_end = false;
	bool end = false;
	bool teleport_out = false;
	bool teleport_in = false;

	arrow_path_t::iterator hex;
	for (hex = path_.begin(); hex != path_.end(); ++hex)
	{
		prefix = "";
		suffix = "";
		image_filename = "";
		start = pre_end = end = false;

		// teleport in if we teleported out last hex
		teleport_in = teleport_out;
		teleport_out = false;

		// Determine some special cases
		if (hex == arrow_start_hex)
			start = true;
		if (hex == arrow_pre_end_hex)
			pre_end = true;
		else if (hex == arrow_end_hex)
			end = true;
		if (hex != arrow_end_hex && !tiles_adjacent(*hex, *(hex + 1)))
			teleport_out = true;

		// calculate enter and exit directions, if available
		enter_dir = map_location::NDIRECTIONS;
		if (!start && !teleport_in)
		{
			enter_dir = hex->get_relative_dir(*(hex-1));
		}
		exit_dir = map_location::NDIRECTIONS;
		if (!end && !teleport_out)
		{
			exit_dir = hex->get_relative_dir(*(hex+1));
		}

		// Now figure out the actual images
		if (teleport_out)
		{
			prefix = "teleport-out";
			if (enter_dir != map_location::NDIRECTIONS)
			{
				suffix = map_location::write_direction(enter_dir);
			}
		}
		else if (teleport_in)
		{
			prefix = "teleport-in";
			if (exit_dir != map_location::NDIRECTIONS)
			{
				suffix = map_location::write_direction(exit_dir);
			}
		}
		else if (start)
		{
			prefix = "start";
			suffix = map_location::write_direction(exit_dir);
			if (pre_end)
			{
				suffix = suffix + "_ending";
			}
		}
		else if (end)
		{
			prefix = "end";
			suffix = map_location::write_direction(enter_dir);
		}
		else
		{
			std::string enter, exit;
			enter = map_location::write_direction(enter_dir);
			exit = map_location::write_direction(exit_dir);
			if (pre_end)
			{
				exit = exit + "_ending";
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
		}

		image_filename = dirname + style_ + "/" + prefix;
		if (suffix != "")
		{
			image_filename += ("-" + suffix);
		}
		image_filename += ".png";
		assert(image_filename != "");

		image::locator image = image::locator(image_filename, mods);
		if (!image.file_exists())
		{
			ERR_ARR << "Image " << image_filename << " not found.\n";
			image = image::locator("misc/missing-image.png");
		}
		symbols_map_[*hex] = image;
	}

	invalidate_arrow_path(path_);

	notify_arrow_changed();
}

void arrow::invalidate_arrow_path(arrow_path_t path)
{
	if(!SCREEN) return;

	foreach(map_location const& loc, path)
	{
		SCREEN->invalidate(loc);
	}
}

void arrow::notify_arrow_changed()
{
	if(!SCREEN) return;

	SCREEN->update_arrow(*this);
}
