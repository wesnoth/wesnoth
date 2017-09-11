/*
   Copyright (C) 2014 - 2017 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "gui/dialogs/multiplayer/synced_choice_wait.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "quit_confirmation.hpp"

#include "gui/dialogs/message.hpp"
#include "game_end_exceptions.hpp"
#include "gettext.hpp"


#include "utils/functional.hpp"

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_theme_list
 *
 * == Theme list ==
 *
 * Dialog for selecting a GUI theme.
 *
 * @begin{table}{dialog_widgets}
 *
 * themes & & listbox & m &
 *         Listbox displaying user choices. $
 *
 * -name & & styled_widget & m &
 *         Widget which shows a theme item name. $
 *
 * -description & & styled_widget & m &
 *         Widget which shows a theme item description. $
 *
 * @end{table}
 */

REGISTER_DIALOG(synched_choice_wait)

synched_choice_wait::synched_choice_wait(user_choice_manager& mgr)
	: mgr_(mgr)
	, message_()
{
	mgr_.changed_event_.attach_handler(this);
}

synched_choice_wait::~synched_choice_wait()
{
	mgr_.changed_event_.detach_handler(this);
}

void synched_choice_wait::pre_show(window& window)
{
	message_ = find_widget<label>(&window, "lblMessage", false, true);


	button& quit_button = find_widget<button>(
				&window, "btn_quit_game", false);

	connect_signal_mouse_left_click(quit_button,
		std::bind(&quit_confirmation::quit_to_title));

	message_->set_label(mgr_.wait_message());
	if(mgr_.finished() || !mgr_.waiting()) {
		window.close();
	}
}

void synched_choice_wait::handle_generic_event(const std::string& event_name)
{
	assert(event_name == "user_choice_update");
	assert(message_);
	message_->set_label(mgr_.wait_message());
	if(mgr_.finished() || !mgr_.waiting()) {
		get_window()->close();
	}
}

} // namespace dialogs
} // namespace gui2
