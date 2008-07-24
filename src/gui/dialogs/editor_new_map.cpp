/* $Id$ */
/*
   copyright (C) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#include "gui/dialogs/editor_new_map.hpp"

#include "foreach.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/window_builder.hpp"
#include "language.hpp"
#include "log.hpp"
#include "preferences.hpp"
#include "video.hpp"
#include "wml_exception.hpp"

#define DBG_GUI LOG_STREAM_INDENT(debug, widget)
#define LOG_GUI LOG_STREAM_INDENT(info, widget)
#define WRN_GUI LOG_STREAM_INDENT(warn, widget)
#define ERR_GUI LOG_STREAM_INDENT(err, widget)

namespace gui2 {

teditor_new_map::teditor_new_map()
: map_width_(0), map_height_(0)
{
}

twindow teditor_new_map::build_window(CVideo& video)
{
	return build(video, get_id(EDITOR_NEW_MAP));
}

void teditor_new_map::pre_show(CVideo& /*video*/, twindow& window)
{
	ttext_box* width_widget = dynamic_cast<ttext_box*>(window.find_widget("width", false));
	VALIDATE(width_widget, missing_widget("width"));
	width_widget->set_value(lexical_cast<std::string>(map_width_));

	ttext_box* height_widget = dynamic_cast<ttext_box*>(window.find_widget("height", false));
	VALIDATE(height_widget, missing_widget("height"));
	height_widget->set_value(lexical_cast<std::string>(map_height_));
}

void teditor_new_map::post_show(twindow& window)
{
	if(get_retval() == tbutton::OK) {
		ttext_box* width_widget = dynamic_cast<ttext_box*>(window.find_widget("width", false));
		ttext_box* height_widget = dynamic_cast<ttext_box*>(window.find_widget("height", false));
		assert(width_widget);
		assert(height_widget);
		map_width_ = lexical_cast_default<int>(width_widget->get_value(), 0);
		map_height_ = lexical_cast_default<int>(height_widget->get_value(), 0);
	}
}

} // namespace gui2
