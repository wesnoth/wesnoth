/*
   Copyright (C) 2010 - 2017 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "version.hpp"

#include "addon/validation.hpp"

#include <ctime>
#include <set>
#include <map>

struct addon_info;
class config;
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

	std::string core;

	std::vector<std::string> depends;
	// std::vector<addon_dependency> conflicts, recommends, replaces;

	std::string feedback_url;

	time_t updated;
	time_t created;

	// Artificial upload order index used to preserve add-ons upload order
	// until we have actual first-upload timestamps implemented. This index
	// is not serialized anywhere.
	unsigned order;

	// Flag to indicate whether this object was generaled from pbl info for an addon
	// not previously published.
	bool local_only;

	addon_info()
		: id(), title(), description(), icon()
		, version(), author(), size(), downloads()
		, uploads(), type(), locales()
		, core()
		, depends()
		, feedback_url()
		, updated()
		, created()
		, order()
		, local_only(false)
	{}

	explicit addon_info(const config& cfg)
		: id(), title(), description(), icon()
		, version(), author(), size(), downloads()
		, uploads(), type(), locales()
		, core()
		, depends()
		, feedback_url()
		, updated()
		, created()
		, order()
		, local_only(false)
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
			this->core = o.core;
			this->depends = o.depends;
			this->feedback_url = o.feedback_url;
			this->updated = o.updated;
			this->created = o.created;
			this->order = o.order;
			this->local_only = o.local_only;
		}
		return *this;
	}

	void read(const config& cfg);

	void write(config& cfg) const;

	/**
	 * Write only minimal WML used for state tracking (_info.cfg) files.
	 *
	 * This currently only includes the add-on type, upload count,
	 * title, and version number.
	 *
	 * @param cfg Target WML config object.
	 */
	void write_minimal(config& cfg) const;

	/**
	 * Get a title or automatic title for display.
	 *
	 * If the real @a title is empty, the returned value is the @a id with
	 * underscores replaced with blanks.
	 *
	 * @todo FIXME: Is it even possible for the add-ons server to provide untitled
	 *       add-ons in its reply anymore? Titles seem to be required at upload time.
	 */
	std::string display_title() const;

	/** Get an icon path fixed for display (e.g. when TC is missing, or the image doesn't exist). */
	std::string display_icon() const;

	/** Get an add-on type identifier for display in the user's language. */
	std::string display_type() const;

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
 * Parse the specified add-ons list WML into an actual addons_list object.
 *
 * @param cfg  Add-ons list WML, currently a [campaigns] node from a server response.
 * @param dest Target addons_list object. It will be cleared first.
 */
void read_addons_list(const config& cfg, addons_list& dest);

/**
 * Get a human-readable representation of the specified byte count.
 *
 * The result includes the size unit, which is the largest byte multiply
 * that makes sense. (e.g. 1 MiB for 1048576 bytes.)
 */
std::string size_display_string(double size);

/**
 * Replaces underscores to dress up file or dirnames as add-on titles.
 *
 * @todo In the future we should store more local information about add-ons and use
 *       this only as a fallback; it could be desirable to fetch translated names as well
 *       somehow.
 */
std::string make_addon_title(const std::string& id);
