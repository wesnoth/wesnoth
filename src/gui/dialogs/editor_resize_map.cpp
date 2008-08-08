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

#include "gui/dialogs/editor_resize_map.hpp"

#include "foreach.hpp"
#include "gui/dialogs/field.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/slider.hpp"
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

teditor_resize_map::teditor_resize_map() :
	map_width_(register_integer("width", false)),
	map_height_(register_integer("height", false))
{
}

void teditor_resize_map::set_map_width(int value) 
{ 
	map_width_->set_value(value);
}

int teditor_resize_map::map_width() const
{
	return map_width_->get_value();
}

void teditor_resize_map::set_map_height(int value)
{ 
	map_height_->set_value(value);
}

int teditor_resize_map::map_height() const
{
	return map_height_->get_value();
}

void teditor_resize_map::set_old_map_width(int value) 
{ 
	old_width_ = value;
}

void teditor_resize_map::set_old_map_height(int value)
{ 
	old_height_ = value;
}

twindow teditor_resize_map::build_window(CVideo& video)
{
	return build(video, get_id(EDITOR_RESIZE_MAP));
}

void teditor_resize_map::pre_show(CVideo& /*video*/, twindow& window)
{
	tlabel* old_width = dynamic_cast<tlabel*>(window.find_widget("old_width", false));
	VALIDATE(old_width, missing_widget("old_width"));
	tlabel* old_height = dynamic_cast<tlabel*>(window.find_widget("old_height", false));
	VALIDATE(old_height, missing_widget("old_height"));
	old_width->set_label(lexical_cast<std::string>(old_width_));
	old_height->set_label(lexical_cast<std::string>(old_height_));
}

} // namespace gui2
