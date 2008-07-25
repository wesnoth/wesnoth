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

#include "gui/dialogs/dialog.hpp"

#include "foreach.hpp"
#include "gui/dialogs/field.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/text.hpp"
#include "gui/widgets/window.hpp"
#include "log.hpp"


#define DBG_GUI LOG_STREAM_INDENT(debug, widget)
#define LOG_GUI LOG_STREAM_INDENT(info, widget)
#define WRN_GUI LOG_STREAM_INDENT(warn, widget)
#define ERR_GUI LOG_STREAM_INDENT(err, widget)

namespace gui2 {

tdialog::~tdialog()
{
	foreach(tfield_* field, fields_) {
		delete field;
	}
}

void tdialog::show(CVideo& video)
{
	twindow window = build_window(video);

	window.set_owner(this);

	init_fields(window);

	pre_show(video, window);

	retval_ = window.show(true);

	if(retval_ ==  tbutton::OK) {
		finalize_fields(window);
	}

	post_show(window);
}

tfield_bool* tdialog::register_bool(const std::string& id, const bool optional,
		bool (*callback_load_value) (),
		void (*callback_save_value) (const bool value),
		void (*callback_change) (twidget* widget))
{
	tfield_bool* field =  new tfield_bool(id, optional,
		callback_load_value, callback_save_value, callback_change);

	fields_.push_back(field);
	return field;
}

tfield_integer* tdialog::register_integer(const std::string& id, const bool optional,
		int (*callback_load_value) (),
		void (*callback_save_value) (const int value))
{
	tfield_integer* field =  new tfield_integer(id, optional,
		callback_load_value, callback_save_value);

	fields_.push_back(field);
	return field;
}

tfield_text* tdialog::register_text(const std::string& id, const bool optional,
		std::string (*callback_load_value) (),
		void (*callback_save_value) (const std::string& value))
{
	tfield_text* field =  new tfield_text(id, optional,
		callback_load_value, callback_save_value);

	fields_.push_back(field);
	return field;
}

void tdialog::init_fields(twindow& window)
{
	foreach(tfield_* field, fields_) {
		field->widget_init(window);
	}
}

void tdialog::finalize_fields(twindow& window)
{
	foreach(tfield_* field, fields_) {
		field->widget_finalize(window);
	}
}

} // namespace gui2

