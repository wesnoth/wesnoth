/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/multiplayer/mp_method_selection.hpp"

#include "game_preferences.hpp"
#include "gui/auxiliary/find_widget.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/window.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "game_initialization/multiplayer.hpp"

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_mp_method_selection
 *
 * == MP method selection ==
 *
 * This shows the dialog to select the kind of MP game the user wants to play.
 *
 * @begin{table}{dialog_widgets}
 *
 * user_name & & text_box & m &
 *         This text contains the name the user on the MP server. This widget
 *         will get a fixed maximum length by the engine. $
 *
 * method_list & & listbox & m &
 *         The list with possible game methods. $
 *
 * @end{table}
 */

REGISTER_DIALOG(mp_method_selection)

void mp_method_selection::pre_show(window& window)
{
	user_name_ = preferences::login();
	text_box* user_widget
			= find_widget<text_box>(&window, "user_name", false, true);
	user_widget->set_value(user_name_);
	user_widget->set_maximum_length(mp::max_login_size);
	window.keyboard_capture(user_widget);

	listbox* list = find_widget<listbox>(&window, "method_list", false, true);

	window.add_to_keyboard_chain(list);
}

void mp_method_selection::post_show(window& window)
{
	if(get_retval() == window::OK) {
		text_box& user_widget
				= find_widget<text_box>(&window, "user_name", false);
		listbox& list = find_widget<listbox>(&window, "method_list", false);

		choice_ = list.get_selected_row();

		user_widget.save_to_history();
		user_name_ = user_widget.get_value();
		preferences::set_login(user_name_);
	}
}

} // namespace dialogs
} // namespace gui2
