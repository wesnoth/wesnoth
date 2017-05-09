/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
                 2008 - 2015 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include <string>
#include <vector>

class display;
class CVideo;

/**
 * Shows the add-ons server connection dialog, for access to the various management front-ends.
 *
 * @param v         Target for UI rendering.
 *
 * @return @a true when one or more add-ons have been successfully installed or
 *         removed, thus requiring a local WML cache refresh. @a false otherwise.
 */
bool manage_addons(CVideo& v);

/**
 * Conducts an ad-hoc add-ons server connection to download an add-on with a particular id and all
 * it's dependencies. Launches gui dialogs when issues arise.
 *
 * @param v         Target for UI rendering.
 * @param addon_ids The ids of the target add-on.
 *
 * @return @a true when we successfully installed the target (possibly the user chose to ignore failures)
 */
bool ad_hoc_addon_fetch_session(CVideo& v, const std::vector<std::string>& addon_ids);
