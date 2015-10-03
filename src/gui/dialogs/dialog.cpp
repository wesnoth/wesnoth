/* $Id$ */
/*
   Copyright (C) 2008 - 2011 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/dialog.hpp"

#include "foreach.hpp"
#include "gui/dialogs/field.hpp"
#include "gui/widgets/integer_selector.hpp"
#include "video.hpp"

namespace gui2 {

tdialog::~tdialog()
{
	BOOST_FOREACH(tfield_* field, fields_) {
		delete field;
	}
}

void tdialog::show(CVideo& video, const unsigned auto_close_time)
{
	if(video.faked()) {
		return;
	}

	std::auto_ptr<twindow> window(build_window(video));
	assert(window.get());

	window->set_owner(this);

	init_fields(*window);

	pre_show(video, *window);

	retval_ = window->show(restore_, auto_close_time);

	/*
	 * It can happen that when two clicks follow eachother fast that the event
	 * handling code in events.cpp generates a DOUBLE_CLICK_EVENT. For some
	 * reason it can happen that this event gets pushed in the queue when the
	 * window is shown, but processed after the window is closed. This causes
	 * the next window to get this pending event.
	 *
	 * This caused a bug where double clicking in the campaign selection dialog
	 * directly selected a difficulty level and started the campaign. In order
	 * to avoid that problem, filter all pending DOUBLE_CLICK_EVENT events after
	 * the window is closed.
	 */
	events::discard(SDL_EVENTMASK(DOUBLE_CLICK_EVENT));

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
	BOOST_FOREACH(tfield_* field, fields_) {
		field->widget_init(window);
	}
}

void tdialog::finalize_fields(twindow& window)
{
	BOOST_FOREACH(tfield_* field, fields_) {
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

