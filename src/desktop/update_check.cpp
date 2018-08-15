/*
 Copyright (C) 2018 by Martin Hrubý <hrubymar10@gmail.com>
 Part of the Battle for Wesnoth Project https://www.wesnoth.org/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.
 
 See the COPYING file for more details.
 */

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "update_check.hpp"

#include "formula/string_utils.hpp"
#include "game_config.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/label.hpp"
#include "preferences/game.hpp"

namespace desktop {
namespace update_check {
void check_for_updates(gui2::window& win) {
	std::string version_string = VGETTEXT("Version $version", {{ "version", game_config::revision }});
	if (preferences::update_check()) {
		if (game_config::new_version == "") {
			//TODO: Add actual update checking stuff
			game_config::new_version = "1.555";
		}
		
		if (game_config::new_version != "no_update") {
			version_string += " | " + VGETTEXT("New updated version $new_version available!", {{ "new_version", game_config::new_version }});
		}
	}
	
	if(gui2::label* version_label = gui2::find_widget<gui2::label>(&win, "revision_number", false, false)) { //TODO: This line produces error, if is title already rendered
		version_label->set_label(version_string);
	}
}
} // end namespace update_check
} // end namespace desktop
