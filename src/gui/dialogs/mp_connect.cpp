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

#include "gui/dialogs/mp_connect.hpp"

#include "foreach.hpp"
#include "game_preferences.hpp"
#include "gui/dialogs/field.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/password_box.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/toggle_button.hpp"

namespace gui2 {

namespace {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_mp_server_list
 *
 * == Multiplayer server list ==
 *
 * This shows the dialog with a list of predefined multiplayer servers.
 *
 * @start_table = container--SPECIAL
 *     server_list listbox             Listbox with the predefined servers to
 *                                     connect to.
 *     # [name] -                      Widgets which shows the name of the
 *                                     server.
 *     # address -                     The address/host_name of the server.
 * @end_table
 */
class tmp_server_list : public tdialog
{
public:
	tmp_server_list() :
		host_name_()
	{}

	const std::string& host_name() const { return host_name_; }

private:
	std::string host_name_;

	/** Inherited from tdialog. */
	twindow* build_window(CVideo& video)
		{ return build(video, get_id(MP_SERVER_LIST)); }

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
};

void tmp_server_list::pre_show(CVideo& /*video*/, twindow& window)
{
	tlistbox* list =
		dynamic_cast<tlistbox*>(window.find_widget("server_list", false));
	VALIDATE(list, missing_widget("server_list"));

	const std::vector<game_config::server_info>&
		pref_servers = preferences::server_list();

	foreach(const game_config::server_info& server, pref_servers) {

		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = server.name;
		data.insert(std::make_pair("name", item));

		item["label"] = server.address;
		data.insert(std::make_pair("address", item));

		list->add_row(data);
	}
}

void tmp_server_list::post_show(twindow& window)
{
	if(get_retval() == twindow::OK) {

		const tlistbox* list =
			dynamic_cast<tlistbox*>(window.find_widget("server_list", false));
		assert(list);

		const tgrid* row = list->get_row_grid(list->get_selected_row());
		assert(row);

		const tcontrol* address =
			dynamic_cast<const tcontrol*>(row->find_widget("address", false));
		assert(address);

		host_name_ = address->label();
	}
}

void callback_view_list_button(twidget* caller)
{
	assert(caller);

	tmp_connect* mp_connect = dynamic_cast<tmp_connect*>(caller->dialog());
	assert(mp_connect);
	twindow* window = dynamic_cast<twindow*>(caller->get_window());
	assert(window);

	mp_connect->show_server_list(*window);
}

} // namespace

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_mp_connect
 *
 * == Multiplayer connect ==
 *
 * This shows the dialog to the MP server to connect to.
 *
 * @start_table = container
 *     host_name (text_box)            The name of the server to connect to.
 *     [list] (button)                 Shows a dialog with a list of predefined
 *                                     servers to connect to.
 * @end_table
 */
tmp_connect::tmp_connect() :
	video_(0),
	host_name_(register_text("host_name", false,
		preferences::network_host,
		preferences::set_network_host))
{
}

twindow* tmp_connect::build_window(CVideo& video)
{
	return build(video, get_id(MP_CONNECT));
}

void tmp_connect::pre_show(CVideo& video, twindow& window)
{
	assert(!video_);
	assert(host_name_);
	video_ = &video;

	window.keyboard_capture(host_name_->widget(window));

	// Set view list callback button.
	tbutton *view_list =
		dynamic_cast<tbutton*>(window.find_widget("list", false));
	if(view_list) {
		view_list->set_callback_mouse_left_click(callback_view_list_button);
	}
}

void tmp_connect::post_show(twindow& /*window*/)
{
	video_ = 0;
}

void tmp_connect::show_server_list(twindow& window)
{
	assert(video_);
	assert(host_name_);

	tmp_server_list dlg;
	dlg.show(*video_);

	if(dlg.get_retval() == twindow::OK) {
		host_name_->set_widget_value(window, dlg.host_name());
	}
}

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_mp_login
 *
 * == Multiplayer connect ==
 *
 * This shows the dialog to log in to the MP server
 *
 * @start_table = container
 *     user_name            (text_box) the login user name
 *     password            (text_box)  the password
 *     [password_reminder] (button)    Request a password reminder
 *     [change_username]   (button)    Use a different username
 *     [login_label]       (button)    Displays the information received
 *                                     from the server
 * @end_table
 */

tmp_login::tmp_login(const t_string& label,	const bool focus_password) :
		label_(label), focus_password_(focus_password) { }

twindow* tmp_login::build_window(CVideo& video)
{
	return build(video, get_id(MP_LOGIN));
}

void tmp_login::pre_show(CVideo& /*video*/, twindow& window)
{
	ttext_box* username =
		dynamic_cast<ttext_box*>(window.find_widget("user_name", false));
	VALIDATE(username, missing_widget("user_name"));
	username->set_value(preferences::login());

	tpassword_box* password =
		dynamic_cast<tpassword_box*>(window.find_widget("password", false));
	VALIDATE(password, missing_widget("password"));
	password->set_value(preferences::password());

	window.keyboard_capture(focus_password_ ? password : username);

	tbutton *password_reminder =
		dynamic_cast<tbutton*>(window.find_widget("password_reminder", false));
    if(password_reminder) password_reminder->set_retval(1);

	tbutton* change_username =
		dynamic_cast<tbutton*>(window.find_widget("change_username", false));
    if(change_username) change_username->set_retval(2);

	// Needs to be a scroll_label since the text can get long and a normal label
	// can't wrap (at the moment).
	tcontrol* label =
		dynamic_cast<tscroll_label*>(window.find_widget("login_label", false));
	if(label) label->set_label(label_);

	ttoggle_button* remember_password
			= dynamic_cast<ttoggle_button*>(window.find_widget("remember_password", false));
	if(remember_password) remember_password->set_value(preferences::remember_password());

}

void tmp_login::post_show(twindow& window)
{
	if(get_retval() == twindow::OK) {
		ttoggle_button* remember_password
				= dynamic_cast<ttoggle_button*>(window.find_widget("remember_password", false));
		if(remember_password) preferences::set_remember_password(remember_password->get_value());

		ttext_box* username =
			dynamic_cast<ttext_box*>(window.find_widget("user_name", false));
		assert(username);

		preferences::set_login(username->get_value());

		tpassword_box* password =
			dynamic_cast<tpassword_box*>(window.find_widget("password", false));
		assert(password);

		preferences::set_password(password->get_real_value());
	}
}

} // namespace gui2

