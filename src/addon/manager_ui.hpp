/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
                 2008 - 2012 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef ADDON_MANAGER_UI_HPP_INCLUDED
#define ADDON_MANAGER_UI_HPP_INCLUDED

#include <string>

class display;

/**
 * Displays the add-ons download dialog.
 *
 * @param disp              Display object on which to render the dialog.
 * @param remote_address    Address of the add-ons server host.
 * @param show_updates_only Whether to display only add-ons updated in the server.
 *
 * @return @a true when one or more add-ons have been successfully installed,
 *         requiring a local WML cache refresh. @a false otherwise.
 */
bool addons_manager_ui(display& disp, const std::string& remote_address, bool show_updates_only);

/**
 * Shows the add-ons server connection dialog, for access to the various management front-ends.
 *
 * @param disp Display object on which to render UI elements.
 *
 * @return @a true when one or more add-ons have been successfully installed or
 *         removed, thus requiring a local WML cache refresh. @a false otherwise.
 */
bool manage_addons(display& disp);

#endif
