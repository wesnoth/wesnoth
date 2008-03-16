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

static void hello()
{
	std::cerr << "\n\n\nHello world.\n\n\n";
}

void addon_connect(CVideo& video, const std::string& server)
{

	gui2::init();
	gui2::twindow window = build(video, get_id(ADDON_CONNECT));

//	window.connect("connect", &hello);

	window.show(true);
}


} // namespace gui2
