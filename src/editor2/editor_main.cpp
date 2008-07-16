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

#include "editor_main.hpp"
#include "editor_common.hpp"
#include "editor_controller.hpp"

namespace editor2 {

EXIT_STATUS start(config& game_conf, CVideo& video)
{
	hotkey::scope_changer h_(game_conf, "hotkey_editor");
	hotkey::deactivate_all_scopes();
	hotkey::set_scope_active(hotkey::SCOPE_GENERAL);
	hotkey::set_scope_active(hotkey::SCOPE_EDITOR);	
	editor_controller editor(game_conf, video);
	EXIT_STATUS e = editor.main_loop();
	return e;
}

} //end namespace editor2
