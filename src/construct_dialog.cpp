/*
   Copyright (C) 2006 - 2017 by Patrick Parker <patrick_x99@hotmail.com>
   wesnoth widget Copyright (C) 2003-5 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "construct_dialog.hpp"

#include "config_assign.hpp"
#include "formula/string_utils.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "sound.hpp"
#include "log.hpp"
#include "font/marked-up_text.hpp"
#include "font/standard_colors.hpp"
#include "scripting/plugins/context.hpp"
#include "scripting/plugins/manager.hpp"
#include "sdl/surface.hpp"
#include "sdl/rect.hpp"
#include "utils/general.hpp"

#include "utils/functional.hpp"

namespace gui {

int dialog_button::action(dialog_process_info &info) {
	if(handler_ != nullptr) {
		dialog_button_action::RESULT res = handler_->button_pressed();

		if( res == CLOSE_DIALOG) {
			return res;
		}

		//reset button-tracking flags so that if the action displays a dialog, a button-press
		//at the end of the dialog won't be mistaken for a button-press in this dialog.
		//(We should eventually use a proper event-handling system instead of tracking
		//flags to avoid problems like this altogether).
		info.clear_buttons();
		return CONTINUE_DIALOG;
	}
	return simple_result_;
}

}//end namespace gui
