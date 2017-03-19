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

#include "gui/dialogs/addon/connect.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "help/help.hpp"

#include "utils/functional.hpp"

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_addon_connect
 *
 * == Addon connect ==
 *
 * This shows the dialog for managing addons and connecting to the addon server.
 *
 * @begin{table}{dialog_widgets}
 * hostname & & text_box & m &
 *         This text contains the name of the server to connect to. $
 *
 * show_help & & button & m &
 *         Thus button shows the in-game help about add-ons management when
 *         triggered. $
 *
 * & 2 & button & o &
 *         This button closes the dialog to display a dialog for removing
 *         installed add-ons. $
 *
 * @end{table}
 */

REGISTER_DIALOG(addon_connect)

addon_connect::addon_connect(std::string& host_name,
							   const bool allow_remove)
	: allow_remove_(allow_remove)
{
	set_restore(true);
	register_text("host_name", false, host_name, true);
}

void addon_connect::help_button_callback(window& window)
{
	help::show_help(window.video(), "installing_addons");
}

void addon_connect::pre_show(window& window)
{
	find_widget<button>(&window, "remove_addons", false)
			.set_active(allow_remove_);

	connect_signal_mouse_left_click(
			find_widget<button>(&window, "show_help", false),
			std::bind(&addon_connect::help_button_callback,
						this,
						std::ref(window)));
}

void addon_connect::post_show(window& window)
{
	if(get_retval() == window::OK) {
		text_box& host_widget
				= find_widget<text_box>(&window, "host_name", false);

		host_widget.save_to_history();
	}
}

} // namespace dialogs
} // namespace gui2
