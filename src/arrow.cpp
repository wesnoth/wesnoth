/*
   Copyright (C) 2010 - 2018 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Method bodies for the arrow class.
 */

#include "arrow.hpp"

#include "game_display.hpp"
#include "log.hpp"

static lg::log_domain log_arrows("arrows");
#define ERR_ARR LOG_STREAM(err, log_arrows)
#define WRN_ARR LOG_STREAM(warn, log_arrows)
#define LOG_ARR LOG_STREAM(info, log_arrows)
#define DBG_ARR LOG_STREAM(debug, log_arrows)

arrow::arrow(bool hidden)
	: layer_(display::LAYER_ARROWS)
	, color_("red")
	, style_(STYLE_STANDARD)
	, path_()
	, previous_path_()
	, symbols_map_()
	, hidden_(true)
{
	if(!hidden)
		show();
}

arrow::~arrow()
{
	hide();
}

void arrow::hide()
{
	if(hidden_)
		return;
	hidden_ = true;

	if(display* disp = display::get_singleton()) {
		invalidate_arrow_path(path_);
		disp->remove_arrow(*this);
	}
}

void arrow::show()
{
	if(!hidden_)
		return;
	hidden_ = false;

	if(display* disp = display::get_singleton()) {
		display::get_singleton()->add_arrow(*this);
	}
}

void arrow::set_path(const arrow_path_t& path)
{
	if (valid_path(path))
	{
		previous_path_ = path_;
		path_ = path;
		update_symbols();
		if(!hidden_)
		{
			invalidate_arrow_path(previous_path_);
			notify_arrow_changed();
		}
	}
}

void arrow::reset()
{
	invalidate_arrow_path(previous_path_);
	invalidate_arrow_path(path_);
	previous_path_.clear();
	path_.clear();
	symbols_map_.clear();
	notify_arrow_changed();
}

void arrow::set_color(const std::string& color)
{
	color_ = color;
	if (valid_path(path_))
	{
		update_symbols();
	}
}

const std::string arrow::STYLE_STANDARD = "standard";
const std::string arrow::STYLE_HIGHLIGHTED = "highlighted";
const std::string arrow::STYLE_FOCUS = "focus";
const std::string arrow::STYLE_FOCUS_INVALID = "focus_invalid";

void arrow::set_style(const std::string& style)
{
	style_ = style;
	if (valid_path(path_))
	{
		update_symbols();
	}
}

const arrow_path_t& arrow::get_path() const
{
	return path_;
}

const arrow_path_t& arrow::get_previous_path() const
{
	return previous_path_;
}

bool arrow::path_contains(const map_location& hex) const
{
	bool contains = symbols_map_.find(hex) != symbols_map_.end();
	return contains;
}

void arrow::draw_hex(const map_location& hex)
{
	if(path_contains(hex))
	{
		display* disp = display::get_singleton();
		disp->render_image(disp->get_location_x(hex), disp->get_location_y(hex), layer_,
					hex, image::get_image(symbols_map_[hex], image::SCALED_TO_ZOOM));
	}
}

bool arrow::valid_path(const arrow_path_t& path)
{
	if (path.size() >= 2)
		return true;
	else
		return false;
}

void arrow::update_symbols()
{
	if (!valid_path(path_))
	{
		WRN_ARR << "arrow::update_symbols called with invalid path" << std::endl;
		return;
	}

	symbols_map_.clear();
	invalidate_arrow_path(path_);

	const std::string mods = "~RC(FF00FF>"+ color_ + ")"; //magenta to current color

	const std::string dirname = "arrows/";
	std::string prefix = "";
	std::string suffix = "";
	std::string image_filename = "";
	arrow_path_t::const_iterator const arrow_start_hex = path_.begin();
	arrow_path_t::const_iterator const arrow_pre_end_hex = path_.end() - 2;
	arrow_path_t::const_iterator const arrow_end_hex = path_.end() - 1;
	bool teleport_out = false;

	arrow_path_t::iterator hex;
	for (hex = path_.begin(); hex != path_.end(); ++hex)
	{
		prefix = "";
		suffix = "";
		image_filename = "";
		bool start = false;
		bool pre_end = false;
		bool end = false;

		// teleport in if we teleported out last hex
		bool teleport_in = teleport_out;
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
		map_location::DIRECTION enter_dir = map_location::NDIRECTIONS;
		if (!start && !teleport_in)
		{
			enter_dir = hex->get_relative_dir(*(hex-1));
		}
		map_location::DIRECTION exit_dir = map_location::NDIRECTIONS;
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

			assert(std::abs(enter_dir - exit_dir) > 1); //impossible turn?
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
		if (!suffix.empty())
		{
			image_filename += ("-" + suffix);
		}
		image_filename += ".png";
		assert(!image_filename.empty());

		image::locator image = image::locator(image_filename, mods);
		if (!image.file_exists())
		{
			ERR_ARR << "Image " << image_filename << " not found." << std::endl;
			image = image::locator(game_config::images::missing);
		}
		symbols_map_[*hex] = image;
	}
}

void arrow::invalidate_arrow_path(const arrow_path_t& path)
{
	if(display* disp = display::get_singleton()) {
		for(const map_location& loc : path) {
			disp->invalidate(loc);
		}
	}
}

void arrow::notify_arrow_changed()
{
	if(display* disp = display::get_singleton()) {
		disp->update_arrow(*this);
	}
}
