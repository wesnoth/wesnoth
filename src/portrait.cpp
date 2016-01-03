/*
   Copyright (C) 2008 - 2016 by mark de wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "portrait.hpp"

#include "config.hpp"
#include "log.hpp"
#include "util.hpp"
#include "wml_exception.hpp"

static lg::log_domain log_config("config");
#define WRN_CF LOG_STREAM(warn, log_config)

static tportrait::tside get_side(const std::string& side)
{
	if(side == "both") {
		return  tportrait::BOTH;
	} else if(side == "right") {
		return tportrait::RIGHT;
	} else if(side == "left") {
		return tportrait::LEFT;
	} else {
		WRN_CF << "Unknown portrait side '" << side << "' defaulting to left." << std::endl;
		return tportrait::LEFT;
	}
}

tportrait::tportrait(const config& cfg) :
	image(cfg["image"]),
	side(get_side(cfg["side"])),
	size(cfg["size"].to_unsigned()),
	mirror(cfg["mirror"].to_bool())
{
	VALIDATE(!image.empty(), missing_mandatory_wml_key("portrait", "image"));
	VALIDATE(size != 0,  missing_mandatory_wml_key("portrait", "size"));
}

