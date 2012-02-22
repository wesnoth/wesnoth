/* $Id$ */
/*
   Copyright (C) 2012 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "addon/info.hpp"

#include "foreach.hpp"
#include "gettext.hpp"
#include "serialization/string_utils.hpp"

void addon_info::read(const config& cfg)
{
	this->title = cfg["title"].str();
	this->description = cfg["description"].str();
	this->icon = cfg["icon"].str();
	this->version = cfg["version"].str();
	this->author = cfg["author"].str();
	this->size = cfg["size"];
	this->downloads = cfg["downloads"];
	this->uploads = cfg["uploads"];
	this->type = get_addon_type(cfg["type"].str());

	const config::const_child_itors& locales = cfg.child_range("translation");

	foreach(const config& locale, locales) {
		this->locales.push_back(locale["language"].str());
	}
}

std::string size_display_string(double size)
{
	if(size > 0.0) {
		return utils::si_string(size, true, _("unit_byte^B"));
	} else {
		return "";
	}
}
