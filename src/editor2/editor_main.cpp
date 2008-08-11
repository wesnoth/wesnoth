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

#include "action_base.hpp"
#include "editor_main.hpp"
#include "editor_common.hpp"
#include "editor_controller.hpp"

#include "../hotkeys.hpp"

namespace editor2 {

EXIT_STATUS start(config& game_conf, CVideo& video)
{
	EXIT_STATUS e = EXIT_ERROR;
	try {
		hotkey::scope_changer h_(game_conf, "hotkey_editor");
		hotkey::deactivate_all_scopes();
		hotkey::set_scope_active(hotkey::SCOPE_GENERAL);
		hotkey::set_scope_active(hotkey::SCOPE_EDITOR);	
		editor_controller editor(game_conf, video);
		e = editor.main_loop();
	} catch (editor_exception& e) {
		ERR_ED << "Editor exception in editor2::start: " << e.what() << "\n";
		throw;
	}
	if (editor_action::get_instance_count() != 0) {
		WRN_ED << "Possibly leaked " << editor_action::get_instance_count() << " action objects\n";
	}
	return e;
}

} //end namespace editor2
