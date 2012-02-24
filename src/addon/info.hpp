/* $Id$ */
/*
   Copyright (C) 2010 - 2012 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef ADDON_INFO_HPP_INCLUDED
#define ADDON_INFO_HPP_INCLUDED

#include "config.hpp"
#include "version.hpp"

#include "addon/validation.hpp"

#include <set>

struct addon_info;
typedef std::map<std::string, addon_info> addons_list;

struct addon_info
{
	std::string id;
	std::string title;
	std::string description;

	std::string icon;

	version_info version;

	std::string author;

	int size;
	int downloads;
	int uploads;

	ADDON_TYPE type;

	std::vector<std::string> locales;

	std::vector<std::string> depends;
	// std::vector<addon_dependency> conflicts, recommends, replaces;

	addon_info()
		: id(), title(), description(), icon()
		, version(), author(), size(), downloads()
		, uploads(), type(), locales()
		, depends()
	{}

	addon_info(const addon_info& o)
		: id(o.id), title(o.title), description(o.description), icon(o.icon)
		, version(o.version), author(o.author), size(o.size)
		, downloads(o.downloads), uploads(o.uploads), type(o.type)
		, locales(o.locales)
		, depends(o.depends)
	{}

	addon_info(const config& cfg)
		: id(), title(), description(), icon()
		, version(), author(), size(), downloads()
		, uploads(), type(), locales()
		, depends()
	{
		this->read(cfg);
	}

	addon_info& operator=(const addon_info& o) {
		if(this != &o) {
			this->id = o.id;
			this->title = o.title;
			this->description = o.description;
			this->icon = o.icon;
			this->version = o.version;
			this->author = o.author;
			this->size = o.size;
			this->downloads = o.downloads;
			this->uploads = o.uploads;
			this->type = o.type;
			this->locales = o.locales;
			this->depends = o.depends;
		}
		return *this;
	}

	void read(const config& cfg);

	/** Get an icon path fixed for display (e.g. when TC is missing, or the image doesn't exist). */
	std::string display_icon() const;

	/**
	 * Resolve an add-on's dependency tree in a recursive fashion.
	 *
	 * The returned vector contains the list of resolved dependencies for this
	 * and any other add-ons upon which it depends.
	 *
	 * @param addons     The add-ons list.
	 *
	 * @todo Tag resolved dependencies with information about where they come from,
	 *       and implement more dependency tiers.
	 */
	std::set<std::string> resolve_dependencies(const addons_list& addons) const;
};


/**
 * Get a human-readable representation of the specified byte count.
 *
 * The result includes the size unit, which is the largest byte multiply
 * that makes sense. (e.g. 1 MiB for 1048576 bytes.)
 */
std::string size_display_string(double size);

#endif
