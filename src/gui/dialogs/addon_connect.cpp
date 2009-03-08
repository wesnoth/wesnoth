/* $Id$ */
/*
   copyright (c) 2008 - 2009 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/addon_connect.hpp"

#include "gui/widgets/window.hpp"
#include "gui/widgets/text_box.hpp"

namespace gui2 {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_addon_connect
 *
 * == Addon connect ==
 *
 * This shows the dialog for managing addons and connecting to the addon server.
 *
 * @start_table = container
 *     [] (button) (2)                 This button closes the dialog and starts
 *                                     the addon manager.
 *     [] (button) (3)                 This button closes the dialog and starts
 *                                     the update routine.
 *     host_name (text_box)            This text contains the name of the server
 *                                     to connect to.
 * @end_table
 */

twindow* taddon_connect::build_window(CVideo& video)
{
	return build(video, get_id(ADDON_CONNECT));
}

void taddon_connect::pre_show(CVideo& /*video*/, twindow& window)
{
	ttext_box* host_widget = dynamic_cast<ttext_box*>(window.find_widget("host_name", false));
	VALIDATE(host_widget, missing_widget("host_name"));

	host_widget->set_value(host_name_);
	window.keyboard_capture(host_widget);
}

void taddon_connect::post_show(twindow& window)
{
	if(get_retval() == twindow::OK) {
		ttext_box* host_widget = dynamic_cast<ttext_box*>(window.find_widget("host_name", false));
		assert(host_widget);

		host_widget->save_to_history();
		host_name_= host_widget->get_value();
	}
}

} // namespace gui2
