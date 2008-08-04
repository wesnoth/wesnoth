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

#include "gui/dialogs/editor_generate_map.hpp"

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

teditor_generate_map::teditor_generate_map()
{
}

twindow teditor_generate_map::build_window(CVideo& video)
{
	return build(video, get_id(EDITOR_GENERATE_MAP));
}

void teditor_generate_map::pre_show(CVideo& /*video*/, twindow& window)
{
}

void teditor_generate_map::post_show(twindow& window)
{
}

} // namespace gui2
