/*
	Copyright (C) 2010 - 2024
	by Iris Morelle <shadowm2006@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "game_version.hpp"

#include "addon/validation.hpp"

#include <chrono>
#include <set>
#include <map>

struct addon_info;
class config;
typedef std::map<std::string, addon_info> addons_list;

struct addon_info_translation
{
	static addon_info_translation invalid;

	bool supported;
	std::string title;
	std::string description;

	addon_info_translation()
		: supported(true)
		, title()
		, description()
	{}

	addon_info_translation(bool sup, std::string titl, std::string desc)
		: supported(sup)
		, title(titl)
		, description(desc)
	{
	}

	explicit addon_info_translation(const config& cfg)
		: supported(true)
		, title()
		, description()
	{
		this->read(cfg);
	}

	addon_info_translation(const addon_info_translation&) = default;

	addon_info_translation& operator=(const addon_info_translation& o) = default;

	void read(const config& cfg);

	void write(config& cfg) const;

	bool valid()
	{
		return !title.empty();
	}
};

struct addon_info
{
	std::string id;
	std::string title;
	std::string description;

	std::string icon;

	version_info current_version;
	std::set<version_info, std::greater<version_info>> versions;

	std::string author;

	int size;
	int downloads;
	int uploads;

	ADDON_TYPE type;

	std::vector<std::string> tags;
	std::vector<std::string> locales;

	std::string core;

	std::vector<std::string> depends;
	// std::vector<addon_dependency> conflicts, recommends, replaces;

	std::string feedback_url;

	std::chrono::system_clock::time_point updated;
	std::chrono::system_clock::time_point created;

	// Flag to indicate whether this object was generaled from pbl info for an addon
	// not previously published.
	bool local_only;

	std::map<std::string, addon_info_translation> info_translations;

	addon_info()
		: id(), title(), description(), icon()
		, current_version(), versions(), author(), size()
		, downloads(), uploads(), type(), tags(), locales()
		, core()
		, depends()
		, feedback_url()
		, updated()
		, created()
		, local_only(false)
		, info_translations()
	{}

	explicit addon_info(const config& cfg)
		: id(), title(), description(), icon()
		, current_version(), versions(), author(), size()
		, downloads(), uploads(), type(), tags(), locales()
		, core()
		, depends()
		, feedback_url()
		, updated()
		, created()
		, local_only(false)
		, info_translations()
	{
		this->read(cfg);
	}

	addon_info(const addon_info&) = default;

	addon_info& operator=(const addon_info& o) = default;

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
	 */
	std::string display_title() const;

	addon_info_translation translated_info() const;

	std::string display_title_translated() const;

	std::string display_title_translated_or_original() const;

	std::string display_title_full() const;

	std::string description_translated() const;

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
