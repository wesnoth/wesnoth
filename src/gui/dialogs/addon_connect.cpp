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

#include "gui/widgets/widget.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/window_builder.hpp"
#include "gui/widgets/button.hpp"

#include "gui/widgets/settings.hpp"
#include "log.hpp"
#include "video.hpp"

#define DBG_GUI LOG_STREAM(debug, widget)
#define LOG_GUI LOG_STREAM(info, widget)
#define WRN_GUI LOG_STREAM(warn, widget)
#define ERR_GUI LOG_STREAM(err, widget)

namespace gui2 {

void taddon_connect::show(CVideo& video)
{

	gui2::init();

	twindow window = build(video, get_id(ADDON_CONNECT));

	tcontrol* host_widget = dynamic_cast<tcontrol*>(window.get_widget_by_id("host_name"));
	if(host_widget) {
		host_widget->set_label(host_name_);
	}

	retval_ = window.show(true);

	if(host_widget) {
		host_name_= host_widget->label();
	}
}


} // namespace gui2
