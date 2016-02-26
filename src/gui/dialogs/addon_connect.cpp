/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/addon_connect.hpp"

#include "gui/auxiliary/find_widget.tpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "help/help.hpp"

#include <boost/bind.hpp>

namespace gui2
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

taddon_connect::taddon_connect(std::string& host_name,
							   const bool allow_remove,
							   display*)
	: allow_remove_(allow_remove)
{
	set_restore(true);
	register_text("host_name", false, host_name, true);
}

void taddon_connect::help_button_callback(twindow& window)
{
	help::show_help(window.video(), "installing_addons");
}

void taddon_connect::pre_show(CVideo& /*video*/, twindow& window)
{
	find_widget<tbutton>(&window, "remove_addons", false)
			.set_active(allow_remove_);

	connect_signal_mouse_left_click(
			find_widget<tbutton>(&window, "show_help", false),
			boost::bind(&taddon_connect::help_button_callback,
						this,
						boost::ref(window)));
}

void taddon_connect::post_show(twindow& window)
{
	if(get_retval() == twindow::OK) {
		ttext_box& host_widget
				= find_widget<ttext_box>(&window, "host_name", false);

		host_widget.save_to_history();
	}
}

} // namespace gui2
