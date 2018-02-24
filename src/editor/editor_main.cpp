/*
   Copyright (C) 2008 - 2018 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor/controller/editor_controller.hpp"

#include "gettext.hpp"
#include "filesystem.hpp"
#include "editor/action/action_base.hpp"

#include <boost/algorithm/string/replace.hpp>

lg::log_domain log_editor("editor");

namespace editor {

EXIT_STATUS start(const config& game_conf, const std::string& filename /* = "" */,
	bool take_screenshot /* = false */, const std::string& screenshot_filename /* = "map_screenshot.bmp" */)
{
	EXIT_STATUS e = EXIT_ERROR;
	try {
		hotkey::scope_changer h_;
		hotkey::deactivate_all_scopes();
		hotkey::set_scope_active(hotkey::SCOPE_EDITOR);
		editor_controller editor(game_conf);
		if (!filename.empty() && filesystem::file_exists (filename)) {
			if (filesystem::is_directory(filename)) {
				editor.context_manager_->set_default_dir(filename);
				editor.context_manager_->load_map_dialog(true);
			} else {
				editor.context_manager_->load_map(filename, false);

				// HACK: this fixes an issue where the button overlays would be missing when
				// the loaded map appears. Since we're gonna drop this ridiculous GUI1 drawing
				// stuff in 1.15 I'm not going to waste time coming up with a better fix.
				//
				// Do note adding a redraw_everything call to context_manager::refresh_all also
				// fixes the issue, but I'm pretty sure thats just because editor_controller::
				// display_redraw_callback gets called, which then calls set_button_state.
				//
				// -- vultraz, 2018-02-24
				editor.set_button_state();
			}

			if (take_screenshot) {
				editor.do_screenshot(screenshot_filename);
				e = EXIT_NORMAL;
			}
		}

		if (!take_screenshot)
			e = editor.main_loop();

	} catch (editor_exception& e) {
		ERR_ED << "Editor exception in editor::start: " << e.what() << std::endl;
		throw;
	}
	if (editor_action::get_instance_count() != 0) {
		ERR_ED << "Possibly leaked " << editor_action::get_instance_count() << " action objects" << std::endl;
	}

	return e;
}

} //end namespace editor
