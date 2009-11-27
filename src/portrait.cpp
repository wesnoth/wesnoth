/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by mark de wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "portrait.hpp"

#include "config.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
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
		WRN_CF << "Unknown portrait side '" << side << "' defaulting to left.\n";
		return tportrait::LEFT;
	}
}

tportrait::tportrait(const config& cfg) :
	image(cfg["image"]),
	side(get_side(cfg["side"])),
	size(lexical_cast_default<unsigned>(cfg["size"])),
	mirror(utils::string_bool(cfg["mirror"]))
{
	VALIDATE(!image.empty(), missing_mandatory_wml_key("portrait", "image"));
	VALIDATE(size != 0,  missing_mandatory_wml_key("portrait", "size"));
}

