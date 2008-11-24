/* $Id$ */
/*
   Copyright (C) 2008 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "editor_controller.hpp"

#include "../construct_dialog.hpp"

#include <boost/algorithm/string/replace.hpp>

namespace editor2 {

EXIT_STATUS start(config& game_conf, CVideo& video, const std::string& filename /* = "" */)
{
	EXIT_STATUS e = EXIT_ERROR;
	try {
		hotkey::scope_changer h_(game_conf, "hotkey_editor");
		hotkey::deactivate_all_scopes();
		hotkey::set_scope_active(hotkey::SCOPE_GENERAL);
		hotkey::set_scope_active(hotkey::SCOPE_EDITOR);
		std::auto_ptr<map_context> mc(NULL);
		std::string map_error;
		if (!filename.empty()) {
			try {
				mc.reset(new map_context(game_conf, filename));
				LOG_ED << "Map " << filename << " loaded. " 
					<< mc->get_map().w() << " by " << mc->get_map().h() << "\n";
			} catch (editor_map_load_exception& e) {
				std::stringstream ss;
				ss << "\"" << boost::replace_all_copy(filename, "\\", "\\\\") << "\"";
				ss << ":\n";
				ss << e.what();
				map_error = ss.str();
				ERR_ED << map_error << "\n";
				mc.reset();
			}
		}
		editor_controller editor(game_conf, video, mc.get());
		mc.release();
		if (!map_error.empty()) {
			gui::message_dialog(editor.gui(), _("Error loading map"), map_error).show();
		}
		e = editor.main_loop();
	} catch (editor_exception& e) {
		ERR_ED << "Editor exception in editor2::start: " << e.what() << "\n";
		throw;
	}
	if (editor_action::get_instance_count() != 0) {
		ERR_ED << "Possibly leaked " << editor_action::get_instance_count() << " action objects\n";
	}
	return e;
}

} //end namespace editor2
