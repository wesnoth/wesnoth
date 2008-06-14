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
#include "game_config.hpp"
#include "game_preferences.hpp"
#include "gui/dialogs/dialog.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/widget.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/window_builder.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "log.hpp"
#include "video.hpp"
#include "wml_exception.hpp"

#define DBG_GUI LOG_STREAM_INDENT(debug, widget)
#define LOG_GUI LOG_STREAM_INDENT(info, widget)
#define WRN_GUI LOG_STREAM_INDENT(warn, widget)
#define ERR_GUI LOG_STREAM_INDENT(err, widget)

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
 * @start_table = container
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

void tmp_server_list::pre_show(CVideo& video, twindow& window)
{
	tlistbox* list = 
		dynamic_cast<tlistbox*>(window.find_widget("server_list", false));
	VALIDATE(list, missing_widget("server_list"));

	const std::vector<game_config::server_info>& 
		pref_servers = preferences::server_list();

	foreach(const game_config::server_info& server, pref_servers) {
		
		std::map<std::string, tlistbox::titem> data;
		data.insert(std::make_pair("name", tlistbox::titem(server.name, "")));
		data.insert(std::make_pair(
			"address", tlistbox::titem(server.address, "")));

		list->add_item(data);
	}

	window.recalculate_size();
}

void tmp_server_list::post_show(twindow& window)
{
	if(get_retval() == tbutton::OK) {

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

	mp_connect->show_server_list();
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
 *     host_name text_box              The name of the server to connect to.
 *     [list] button                   Shows a dialog with a list of predefined
 *                                     servers to connect to.
 * @end_table
 */

twindow tmp_connect::build_window(CVideo& video)
{
	return build(video, get_id(MP_CONNECT));
}

void tmp_connect::pre_show(CVideo& video, twindow& window)
{
	assert(!video_);
	assert(!host_name_widget_);
	video_ = &video;

	host_name_widget_ = 
		dynamic_cast<ttext_box*>(window.find_widget("host_name", false));
	VALIDATE(host_name_widget_, missing_widget("host_name"));

	host_name_widget_->set_text(preferences::network_host());
	window.keyboard_capture(host_name_widget_);

	// Set view list callback button.
	tbutton *view_list = 
		dynamic_cast<tbutton*>(window.find_widget("list", false));
	if(view_list) {
		view_list->set_callback_mouse_left_click(callback_view_list_button);
	}
}

void tmp_connect::post_show(twindow& window)
{
	if(get_retval() == tbutton::OK) {
		host_name_widget_->save_to_history();
		host_name_= host_name_widget_->get_text();
		preferences::set_network_host(host_name_);
	}

	video_ = 0;
	host_name_widget_ = 0;
}

void tmp_connect::show_server_list()
{
	assert(video_);
	assert(host_name_widget_);

	tmp_server_list dlg;
	dlg.show(*video_);

	if(dlg.get_retval() == tbutton::OK) {
		host_name_widget_->set_text(dlg.host_name());
	}
}

} // namespace gui2

