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

#include "gui/dialogs/dialog.hpp"

#include "foreach.hpp"
#include "gui/dialogs/field.hpp"
#include "gui/widgets/integer_selector.hpp"

namespace gui2 {

tdialog::~tdialog()
{
	foreach(tfield_* field, fields_) {
		delete field;
	}
}

void tdialog::show(CVideo& video)
{
	std::auto_ptr<twindow> window(build_window(video));
	assert(window.get());

	window->set_owner(this);

	init_fields(*window);

	pre_show(video, *window);

	retval_ = window->show(restore_);

	if(retval_ ==  twindow::OK) {
		finalize_fields(*window);
	}

	post_show(*window);
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


/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 1
 *
 * THIS PAGE IS AUTOMATICALLY GENERATED, DO NOT MODIFY DIRECTLY !!!
 *
 * = Window definition =
 *
 * The window definition define how the windows shown in the dialog look.
 */

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = ZZZZZZ_footer
 *
 * [[Category: WML Reference]]
 * [[Category: GUI WML Reference]]
 * [[Category: Generated]]
 *
 */

