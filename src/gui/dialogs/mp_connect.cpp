/*
   Copyright (C) 2008 - 2015 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/mp_connect.hpp"

#include "game_preferences.hpp"
#include "gui/dialogs/field.hpp"
#include "gui/widgets/button.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "utils/foreach.tpp"
#include "video.hpp"

#include <boost/bind.hpp>

namespace gui2
{

namespace
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_mp_server_list
 *
 * == Multiplayer server list ==
 *
 * This shows the dialog with a list of predefined multiplayer servers.
 *
 * @begin{table}{dialog_widgets}
 *
 * server_list & & listbox & m &
 *         Listbox with the predefined servers to connect to. $
 *
 * -name & & control & o &
 *         Widget which shows the name of the server. $
 *
 * -address & & control & m &
 *         The address/host_name of the server. $
 *
 * @end{table}
 */

class tmp_server_list : public tdialog
{
public:
	tmp_server_list() : host_name_()
	{
	}

	const std::string& host_name() const
	{
		return host_name_;
	}

private:
	std::string host_name_;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
};

REGISTER_DIALOG(mp_server_list)

void tmp_server_list::pre_show(CVideo& /*video*/, twindow& window)
{
	tlistbox& list = find_widget<tlistbox>(&window, "server_list", false);

	window.keyboard_capture(&list);

	const std::vector<game_config::server_info>& pref_servers
			= preferences::server_list();

	FOREACH(const AUTO & server, pref_servers)
	{

		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = server.name;
		data.insert(std::make_pair("name", item));

		item["label"] = server.address;
		data.insert(std::make_pair("address", item));

		list.add_row(data);
	}
}

void tmp_server_list::post_show(twindow& window)
{
	if(get_retval() == twindow::OK) {

		const tlistbox& list
				= find_widget<const tlistbox>(&window, "server_list", false);

		const tgrid* row = list.get_row_grid(list.get_selected_row());
		assert(row);

		host_name_ = find_widget<const tcontrol>(row, "address", false).label();
	}
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
 * @begin{table}{dialog_widgets}
 *
 * start_table & & text_box & m &
 *         The name of the server to connect to. $
 *
 * list & & button & o &
 *         Shows a dialog with a list of predefined servers to connect to. $
 *
 * @end{table}
 */

REGISTER_DIALOG(mp_connect)

static void
show_server_list(CVideo& video, twindow& window, tfield_text* host_name)
{
	assert(host_name);

	tmp_server_list dlg;
	dlg.show(video);

	if(dlg.get_retval() == twindow::OK) {
		host_name->set_widget_value(window, dlg.host_name());
	}
}

tmp_connect::tmp_connect()
	: host_name_(register_text("host_name",
							   true,
							   preferences::network_host,
							   preferences::set_network_host,
							   true))
{
}

void tmp_connect::pre_show(CVideo& video, twindow& window)
{
	assert(host_name_);

	// Set view list callback button.
	if(tbutton* button = find_widget<tbutton>(&window, "list", false, false)) {

		connect_signal_mouse_left_click(*button,
										boost::bind(show_server_list,
													boost::ref(video),
													boost::ref(window),
													host_name_));
	}
}

tdialog* tmp_connect::mp_server_list_for_unit_test()
{
	return new tmp_server_list();
}

} // namespace gui2
