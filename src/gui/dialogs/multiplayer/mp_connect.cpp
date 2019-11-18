/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/multiplayer/mp_connect.hpp"

#include "gettext.hpp"
#include "preferences/game.hpp"
#include "gui/auxiliary/field.hpp"
#include "gui/dialogs/edit_text.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/settings.hpp"

#include "log.hpp"

#include "utils/functional.hpp"

#include <boost/algorithm/string/trim.hpp>

static lg::log_domain log_mpconnect{"gui/dialogs/mp_connect"};
#define ERR_DLG   LOG_STREAM(err,   log_mpconnect)
#define WRN_DLG   LOG_STREAM(warn,  log_mpconnect)
#define LOG_DLG   LOG_STREAM(info,  log_mpconnect)
#define DBG_DLG   LOG_STREAM(debug, log_mpconnect)

namespace gui2
{

namespace
{

// NOTE: See mp_connect::select_first_match() below
#if 0
void clear_listbox_selection(listbox& listbox)
{
	const auto selection = listbox.get_selected_row();
	if(selection >= 0) {
		listbox.select_row(selection, false);
	}
}
#endif

}

namespace dialogs
{

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

mp_connect::mp_connect()
	: host_name_(register_text("host_name",
							   true,
							   preferences::network_host,
							   preferences::set_network_host,
							   true))
	, builtin_servers_(preferences::builtin_servers_list())
	, user_servers_(preferences::user_servers_list())
{
	set_restore(true);
}

std::array<mp_connect::server_list*, 2> mp_connect::server_lists()
{
	return {{ &builtin_servers_, &user_servers_ }};
}

void mp_connect::pre_show(window& win)
{
	text_box& hostname_box = find_widget<text_box>(&win, "host_name", false);
	listbox& server_list = find_widget<listbox>(&win, "server_list", false);
	button& button_add = find_widget<button>(&win, "server_add", false);
	button& button_del = find_widget<button>(&win, "server_delete", false);

	for(const auto* servers : server_lists()) {
		for(const auto& server : *servers) {
			insert_into_server_listbox(server_list, server);
		}
	}

	select_first_match();

	connect_signal_notify_modified(hostname_box, std::bind(&mp_connect::on_address_change, this));
	connect_signal_notify_modified(server_list, std::bind(&mp_connect::on_server_select, this));
	connect_signal_mouse_left_click(button_add, std::bind(&mp_connect::on_server_add, this));
	connect_signal_mouse_left_click(button_del, std::bind(&mp_connect::on_server_delete, this));
}

void mp_connect::insert_into_server_listbox(listbox& listbox, const server_info& srv, int pos)
{
	const std::map<std::string, string_map>& entry{
		{ "name",    string_map{{"label", srv.name}} },
		{ "address", string_map{{"label", srv.address}} },
	};

	listbox.add_row(entry, pos);
}

void mp_connect::select_first_match()
{
	window* window = get_window();

	text_box& hostname_box = find_widget<text_box>(window, "host_name", false);
	listbox& server_list = find_widget<listbox>(window, "server_list", false);
	button& button_add = find_widget<button>(window, "server_add", false);
	button& button_del = find_widget<button>(window, "server_delete", false);

	const auto& address = boost::trim_copy(hostname_box.get_value());

	std::size_t row = 0;

	for(const auto* servers : server_lists()) {
		for(const auto& server : *servers)  {
			if(server.address == address) {
				server_list.select_row(row);
				// Can't Add what's already there or Delete built-in servers
				button_add.set_active(false);
				button_del.set_active(servers == &user_servers_);
				return;
			}

			++row;
		}
	}

	// NOTE: Do not use this in production. It requires the listbox to be
	// defined with has_minimum=false in WML, and makes some UI interactions
	// awkward. In particular it means we would need to keep track of where
	// the selection was last at every time we clear the selection so that
	// the Add button can add under it instead of appending to the very end
	// of the list (currently it just bails out if there's no selection).
#if 0
	// The user wrote a brand new hostname in so there's no matches, clear the
	// selection accordingly
	clear_listbox_selection(server_list);
	button_del.set_active(false);
#endif
	button_add.set_active(!address.empty());
}

void mp_connect::on_address_change()
{
	// Select the first matching list entry or clear the current selection
	select_first_match();
}

void mp_connect::on_server_add()
{
	window* window = get_window();

	text_box& hostname_box = find_widget<text_box>(window, "host_name", false);
	listbox& server_list = find_widget<listbox>(window, "server_list", false);

	const auto& address = boost::trim_copy(hostname_box.get_value());
	const auto& selection = current_selection();

	if(address.empty() || !selection.valid()) {
		// We're not supposed to be here
		return;
	}

	// We insert under the selection. If a built-in server is selected or the
	// user-defined list is empty, we insert at the start of the user-defined
	// list instead.

	const std::size_t mem_pos = selection.user_defined() && !user_servers_.empty()
			? 1 + selection.relative_index() : 0;
	const unsigned int ui_pos = 1 + selection.row();

	std::string name;

	if(!gui2::dialogs::edit_text::execute(_("Add Server"), _("Name:"), name, true) || name.empty()) {
		return;
	}

	server_info info;
	info.name = name;
	info.address = address;

	user_servers_.insert(user_servers_.begin() + mem_pos, info);
	preferences::set_user_servers_list(user_servers_);

	insert_into_server_listbox(server_list, info, ui_pos);
	select_first_match();
}

void mp_connect::on_server_delete()
{
	window* window = get_window();

	listbox& server_list = find_widget<listbox>(window, "server_list", false);

	auto selection = current_selection();

	if(!selection.valid() || !selection.user_defined()) {
		// We're not supposed to be here
		return;
	}

	user_servers_.erase(user_servers_.begin() + selection.relative_index());
	preferences::set_user_servers_list(user_servers_);

	server_list.remove_row(selection.row());
	on_server_select();
}

void mp_connect::on_server_select()
{
	window* window = get_window();

	text_box& hostname_box = find_widget<text_box>(window, "host_name", false);
	button& button_add = find_widget<button>(window, "server_add", false);
	button& button_del = find_widget<button>(window, "server_delete", false);

	auto selection = current_selection();

	if(!selection.valid()) {
		// The user cleared the selection. We can't delete what isn't selected
		// and the Add button's status was already set to a value that makes
		// sense by another signal handler, so just disable Delete.
		button_del.set_active(false);
		return;
	}

	hostname_box.set_value(selection.get().address);

	// Can't Add what's already there
	button_add.set_active(false);
	// Can only Delete user-defined servers
	button_del.set_active(selection.user_defined());
}

mp_connect::selection mp_connect::current_selection()
{
	listbox& server_list = find_widget<listbox>(get_window(), "server_list", false);
	return { this, server_list.get_selected_row() };
}

mp_connect::server_info& mp_connect::selection::get()
{
	must_be_valid();
	return parent_list().at(relative_index());
}

unsigned mp_connect::selection::row() const
{
	must_be_valid();
	return unsigned(row_);
}

std::size_t mp_connect::selection::relative_index() const
{
	must_be_valid();
	return user_defined() ? row() - owner_->builtin_servers_.size() : row();
}

mp_connect::server_list& mp_connect::selection::parent_list() const
{
	must_be_valid();
	return user_defined() ? owner_->user_servers_ : owner_->builtin_servers_;
}

} // namespace dialogs
} // namespace gui2
