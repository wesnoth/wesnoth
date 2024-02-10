/*
	Copyright (C) 2008 - 2024
	by Iris Morelle <shadowm2006@gmail.com>
	Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
 * Local add-on content management interface.
 *
 * This API only concerns local functionality dealing with enumerating and
 * manipulating installed add-ons, as well as extracting add-ons in WML pack
 * form, usually but not necessarily obtained through the networked client.
 *
 * It also includes functions for the handling of add-on versioning
 * information (_info.cfg) and publishing information (_server.pbl).
 */

#pragma once

class config;
class version_info;

#include "addon/validation.hpp"

#include <functional>
#include <string>
#include <vector>
#include <utility>
#include <map>

/**
 * Exception thrown when the WML parser fails to read a .pbl file.
 */
struct invalid_pbl_exception : public std::exception
{
	/**
	 * Constructor.
	 *
	 * @param pbl_path Path to the faulty .pbl file.
	 * @param msg      An error message to display.
	 */
	invalid_pbl_exception(const std::string& pbl_path, const std::string& msg)
		: path(pbl_path), message(msg)
	{}

	/** Path to the faulty .pbl file. */
	const std::string path;

	/** Error message to display. */
	const std::string message;

	/** Destructor.
	 * Virtual to allow for subclassing.
	 */
	virtual ~invalid_pbl_exception() noexcept {}

	/** Returns a pointer to the (constant) error description.
	 *  @return A pointer to a const char*. The underlying memory
	 *          is in posession of the Exception object. Callers must
	 *          not attempt to free the memory.
	 */
	virtual const char* what() const noexcept {
		return message.c_str();
	}
};

/**
 * Removes the specified add-on, deleting its full directory structure.
 */
bool remove_local_addon(const std::string& addon);

/**
 * Returns whether a .pbl file is present for the specified add-on or not.
 */
bool have_addon_pbl_info(const std::string& addon_name);

/**
 * Returns whether the specified add-on appears to be managed by a VCS or not.
 *
 * This is used by the add-ons client to prompt the user before overwriting
 * add-ons that may currently be managed externally instead of through the
 * built-in campaignd client.
 *
 * Currently supported VCSes are: Subversion, Git, Mercurial.
 */
bool have_addon_in_vcs_tree(const std::string& addon_name);

/**
 * Gets the publish information for an add-on.
 *
 * @param addon_name              The add-on's main directory/file name.
 * @param do_validate             Whether we want to run validation on the .pbl file.
 *
 * @exception invalid_pbl_exception If it is not possible to read the .pbl file
 *                                  (often due to invalid WML).
 */
config get_addon_pbl_info(const std::string& addon_name, bool do_validate);

/**
 * Sets the publish information for an add-on.
 *
 * @param addon_name              The add-on's main directory/file name.
 * @param cfg                     A config object from which the add-on's
 *                                properties are copied.
 */
void set_addon_pbl_info(const std::string& addon_name, const class config& cfg);

/**
 * Returns true if there is a local installation info (_info.cfg) file for the add-on.
 */
bool have_addon_install_info(const std::string& addon_name);

/**
 * Gets the installation info (_info.cfg) for an add-on.
 *
 * @param addon_name              The add-on's main directory/file name.
 * @param cfg                     A config object to store the add-on's
 *                                properties.
 */
void get_addon_install_info(const std::string& addon_name, class config& cfg);

/**
 * Writes the installation info (_info.cfg) for an add-on to disk.
 *
 * @param addon_name              The add-on's main directory/file name.
 * @param cfg                     A config object containing the add-on's
 *                                properties.
 */
void write_addon_install_info(const std::string& addon_name, const class config& cfg);

/** Returns a list of local add-ons that can be published. */
std::vector<std::string> available_addons();

/** Retrieves the names of all installed add-ons. */
std::vector<std::string> installed_addons();

/** Retrieves the ids and versions of all installed add-ons. */
std::map<std::string, std::string> installed_addons_and_versions();

/** Check whether the specified add-on is currently installed. */
bool is_addon_installed(const std::string& addon_name);

/** Archives an add-on into a config object for campaignd transactions. */
void archive_addon(const std::string& addon_name, class config& cfg);

/** Unarchives an add-on from campaignd's retrieved config object. */
void unarchive_addon(const class config& cfg, std::function<void(unsigned)> progress_callback = {});

/** Removes the listed files from the addon. */
void purge_addon(const config& removelist);

/** Refreshes the per-session cache of add-on's version information structs. */
void refresh_addon_version_info_cache();

/** Returns a particular installed add-on's version information. */
version_info get_addon_version_info(const std::string& addon);
