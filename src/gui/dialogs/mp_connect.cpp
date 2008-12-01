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

#include "gui/dialogs/mp_connect.hpp"

#include "foreach.hpp"
#include "game_preferences.hpp"
#include "gui/dialogs/field.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/listbox.hpp"

namespace gui2 {

namespace {

/*WIKI
 * @page = GUIWindowWML
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
	twindow build_window(CVideo& video)
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
 * @page = GUIWindowWML
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

twindow tmp_connect::build_window(CVideo& video)
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

} // namespace gui2

