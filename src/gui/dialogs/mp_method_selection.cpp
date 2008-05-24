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

#include "gui/dialogs/mp_method_selection.hpp"

#include "game_preferences.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/widget.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/window_builder.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "log.hpp"
#include "video.hpp"
#include "wml_exception.hpp"

#define DBG_GUI LOG_STREAM_INDENT(debug, widget)
#define LOG_GUI LOG_STREAM_INDENT(info, widget)
#define WRN_GUI LOG_STREAM_INDENT(warn, widget)
#define ERR_GUI LOG_STREAM_INDENT(err, widget)

namespace gui2 {

void tmp_method_selection::show(CVideo& video)
{

	gui2::init();

	twindow window = build(video, get_id(MP_METHOD_SELECTION));

	user_name_ = preferences::login();
	ttext_box* user_widget = dynamic_cast<ttext_box*>(window.find_widget("user_name", false));
	if(user_widget) {
		user_widget->set_text(user_name_);
		window.keyboard_capture(user_widget);
	}

	tlistbox* list = dynamic_cast<tlistbox*>(window.find_widget("method_list", false));
	VALIDATE(list, "No list defined.");

	list->add_item("Join Official Server", "icons/icon-server.png");
	list->add_item("Connect to Server", "icons/icon-serverother.png");
	list->add_item("Local Game", "icons/icon-hotseat.png");
	list->select_row(0);

	window.recalculate_size();

	retval_ = window.show(true);

	if(retval_ == tbutton::OK) {

		choice_ = list->get_selected_row();
		user_widget->save_to_history();
		user_name_= user_widget->get_text();
		preferences::set_login(user_name_);
	}
}

} // namespace gui2
