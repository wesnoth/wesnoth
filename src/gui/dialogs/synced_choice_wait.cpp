/*
   Copyright (C) 2014 - 2016 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "gui/dialogs/synced_choice_wait.hpp"

#include "gui/auxiliary/find_widget.tpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "quit_confirmation.hpp"

#include "message.hpp"
#include "../../game_end_exceptions.hpp"
#include "../../gettext.hpp"


#include <boost/bind.hpp>

namespace gui2
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
 * -name & & control & m &
 *         Widget which shows a theme item name. $
 *
 * -description & & control & m &
 *         Widget which shows a theme item description. $
 *
 * @end{table}
 */

REGISTER_DIALOG(synced_choice_wait)

tsynced_choice_wait::tsynced_choice_wait(user_choice_manager& mgr)
	: mgr_(mgr)
	, message_()
{
	mgr_.changed_event_.attach_handler(this);
}

tsynced_choice_wait::~tsynced_choice_wait()
{
	mgr_.changed_event_.detach_handler(this);
}

void tsynced_choice_wait::pre_show(CVideo& video, twindow& window)
{
	window_ = &window;
	message_ = find_widget<tlabel>(&window, "lblMessage", false, true);


	tbutton& quit_button = find_widget<tbutton>(
				&window, "btn_quit_game", false);

	connect_signal_mouse_left_click(quit_button,
		boost::bind(&quit_confirmation::quit_to_title));

	message_->set_label(mgr_.wait_message());
	if(mgr_.finished() || !mgr_.waiting()) {
		window_->close();
	}
}

void tsynced_choice_wait::handle_generic_event(const std::string& event_name)
{
	assert(event_name == "user_choice_update");
	assert(message_);
	message_->set_label(mgr_.wait_message());
	if(mgr_.finished() || !mgr_.waiting()) {
		window_->close();
	}
}

}
