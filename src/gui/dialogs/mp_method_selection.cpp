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

#include "gui/dialogs/mp_method_selection.hpp"

#include "gui/widgets/listbox.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "multiplayer.hpp"

namespace gui2 {

/*WIKI
 * @page = GUIWindowWML
 * @order = 2_mp_method_selection
 *
 * == MP method selection ==
 *
 * This shows the dialog to select the kind of MP game the user wants to play.
 *
 * @start_table = container
 *     user_name (text_box)            This text contains the name the user on
 *                                     the MP server. This widget will get a
 *                                     fixed maximum length by the engine.
 *     method_list (listbox)           The list with possible game methods.
 * @end_table
 */
twindow* tmp_method_selection::build_window(CVideo& video)
{
	return build(video, get_id(MP_METHOD_SELECTION));
}

void tmp_method_selection::pre_show(CVideo& /*video*/, twindow& window)
{
	user_name_ = preferences::login();
	ttext_box* user_widget = dynamic_cast<ttext_box*>(window.find_widget("user_name", false));
	VALIDATE(user_widget, missing_widget("user_name"));

	user_widget->set_value(user_name_);
	user_widget->set_maximum_length(mp::max_login_size);
	window.keyboard_capture(user_widget);

	ttext_box* password_widget = dynamic_cast<ttext_box*>(window.find_widget("password", false));
	VALIDATE(password_widget, missing_widget("password"));

	password_widget->set_value(preferences::password());

	ttoggle_button* remember_password
			= dynamic_cast<ttoggle_button*>(window.find_widget("remember_password", false));
	if(remember_password) remember_password->set_value(preferences::remember_password());

	tlistbox* list = dynamic_cast<tlistbox*>(window.find_widget("method_list", false));
	VALIDATE(list, missing_widget("method_list"));

	window.add_to_keyboard_chain(list);
	window.add_to_keyboard_chain(user_widget);
}

void tmp_method_selection::post_show(twindow& window)
{
	if(get_retval() == twindow::OK) {

		ttext_box* user_widget = dynamic_cast<ttext_box*>(window.find_widget("user_name", false));
		assert(user_widget);

		ttext_box* password_widget = dynamic_cast<ttext_box*>(window.find_widget("password", false));
		assert(password_widget);

		tlistbox* list = dynamic_cast<tlistbox*>(window.find_widget("method_list", false));
		assert(list);

		ttoggle_button* remember_password
				= dynamic_cast<ttoggle_button*>(window.find_widget("remember_password", false));
		if(remember_password) preferences::set_remember_password(remember_password->get_value());

		choice_ = list->get_selected_row();

		user_widget->save_to_history();
		user_name_= user_widget->get_value();
		preferences::set_login(user_name_);

		preferences::set_password(password_widget->get_value());
	}
}

} // namespace gui2
