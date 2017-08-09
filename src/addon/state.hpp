/*
   Copyright (C) 2012 - 2017 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "addon/info.hpp"
#include <map>

/** Defines various add-on installation statuses. */
enum ADDON_STATUS {
	/** Add-on is not installed. */
	ADDON_NONE,
	/** Version in the server matches local installation. */
	ADDON_INSTALLED,
	/** Version in the server is newer than local installation. */
	ADDON_INSTALLED_UPGRADABLE,
	/** Version in the server is older than local installation. */
	ADDON_INSTALLED_OUTDATED,
	/** No version in the server. */
	ADDON_INSTALLED_LOCAL_ONLY,
	/** Dependencies not satisfied.
	 *  @todo This option isn't currently implemented! */
	ADDON_INSTALLED_BROKEN,
	/** No tracking information available. */
	ADDON_NOT_TRACKED
};

inline bool is_installed_addon_status(ADDON_STATUS s)
{
	return s >= ADDON_INSTALLED && s <= ADDON_NOT_TRACKED;
}

/** Stores additional status information about add-ons. */
struct addon_tracking_info
{
	addon_tracking_info()
		: state(ADDON_NONE)
		, can_publish(false)
		, in_version_control(false)
		, installed_version()
		, remote_version()
	{
	}

	ADDON_STATUS state;
	bool can_publish;
	bool in_version_control;
	version_info installed_version;
	version_info remote_version;
};

typedef std::map<std::string, addon_tracking_info> addons_tracking_list;

/**
 * Get information about an add-on comparing its local state with the add-ons server entry.
 *
 * The add-on doesn't need to be locally installed; this is part of
 * the retrieved information.
 *
 * @param addon The add-ons server entry information.
 * @return      The local tracking status information.
 */
addon_tracking_info get_addon_tracking_info(const addon_info& addon);

/**
 * Add-on installation status filters for the user interface.
 *
 * These are not currently an exact match with the @a ADDON_STATUS
 * enum type in order to keep the UI aspect simple for the user.
 * This might change later.
 */
enum ADDON_STATUS_FILTER {
	FILTER_ALL,
	FILTER_INSTALLED,
	FILTER_UPGRADABLE,
	FILTER_PUBLISHABLE,
	FILTER_NOT_INSTALLED,
	FILTER_COUNT
};

/**
 * Add-on fallback/default sorting criteria for the user interface.
 */
enum ADDON_SORT {
	SORT_NAMES,			/**< Sort by add-on name. */
	SORT_UPDATED,		/**< Sort by last upload time. */
	SORT_CREATED		/**< Sort by creation time. */
};

/**
 * Add-on fallback/default sorting direction.
 */
enum ADDON_SORT_DIRECTION {
	DIRECTION_ASCENDING,		/**< Ascending sort. */
	DIRECTION_DESCENDING		/**< Descending sort. */
};
