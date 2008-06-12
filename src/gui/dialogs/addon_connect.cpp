/* $Id$ */
/*
   copyright (c) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#include "gui/dialogs/addon_connect.hpp"

#include "gui/widgets/button.hpp"
#include "gui/widgets/widget.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/window_builder.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "log.hpp"
#include "wml_exception.hpp"

#define DBG_GUI LOG_STREAM_INDENT(debug, widget)
#define LOG_GUI LOG_STREAM_INDENT(info, widget)
#define WRN_GUI LOG_STREAM_INDENT(warn, widget)
#define ERR_GUI LOG_STREAM_INDENT(err, widget)

namespace gui2 {

/*WIKI
 * @page = GUIWindowWML
 * @order = 2_addon_connect
 *
 * == Addon connect ==
 *
 * This shows the dialog for managing addons and connecting to the addon server.
 * 
 * @start_table = container
 *     [] button (2)                   This button closes the dialog and starts
 *                                     the addon manager.
 *     host_name text_box              This text contains the name of the server
 *                                     to connect to.
 * @end_table
 */

twindow taddon_connect::build_window(CVideo& video)
{
	return build(video, get_id(ADDON_CONNECT));
}

void taddon_connect::pre_show(CVideo& /*video*/, twindow& window)
{
	ttext_box* host_widget = dynamic_cast<ttext_box*>(window.find_widget("host_name", false));
	VALIDATE(host_widget, missing_widget("host_name"));

	host_widget->set_text(host_name_);
	window.keyboard_capture(host_widget);
}

void taddon_connect::post_show(twindow& window)
{
	if(get_retval() == tbutton::OK) {
		ttext_box* host_widget = dynamic_cast<ttext_box*>(window.find_widget("host_name", false));
		assert(host_widget);

		host_widget->save_to_history();
		host_name_= host_widget->get_text();
	}
}

} // namespace gui2
