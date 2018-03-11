/*
   Copyright (C) 2012 - 2018 by Iris Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "addon/state.hpp"
#include "config.hpp"

#include "addon/manager.hpp"
#include "log.hpp"

static lg::log_domain log_addons_client("addons-client");
#define LOG_AC  LOG_STREAM(info, log_addons_client)

addon_tracking_info get_addon_tracking_info(const addon_info& addon)
{
	const std::string& id = addon.id;
	addon_tracking_info t;

	t.can_publish = have_addon_pbl_info(id);
	t.in_version_control = have_addon_in_vcs_tree(id);
	//t.installed_version = version_info();

	if(is_addon_installed(id)) {
		if(t.can_publish) {
			if(addon.local_only) {
				t.installed_version = addon.version;
				//t.remote_version = version_info();
			} else {
				t.remote_version = addon.version;

				// Try to obtain the version number from the .pbl first.
				config pbl = get_addon_pbl_info(id);

				if(pbl.has_attribute("version")) {
					t.installed_version = pbl["version"].str();
				} else {
					t.installed_version = get_addon_version_info(id);
				}
			}
		} else {
			// We normally use the _info.cfg version instead.
			t.installed_version = get_addon_version_info(id);
			t.remote_version = addon.version;
		}

		if(t.remote_version == t.installed_version) {
			t.state = ADDON_INSTALLED;
		} else if(t.remote_version > t.installed_version) {
			t.state = ADDON_INSTALLED_UPGRADABLE;
		} else if(t.remote_version == version_info()) {
			// Remote version not set.
			t.state = ADDON_INSTALLED_LOCAL_ONLY;
		} else /* if(remote_version < t.installed_version) */ {
			t.state = ADDON_INSTALLED_OUTDATED;
		}
	} else {
		t.state = ADDON_NONE;
	}

	return t;
}
