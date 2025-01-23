/*
	Copyright (C) 2008 - 2024
	by Mark de Wever <koraq@xs4all.nl>
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
#include "preferences/preferences.hpp"
#include "gui/auxiliary/field.hpp"
#include "gui/dialogs/edit_text.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/listbox.hpp"

#include "log.hpp"

#include <functional>

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

REGISTER_DIALOG(mp_connect)

mp_connect::mp_connect()
	: modal_dialog(window_id())
	, host_name_(register_text("host_name",
							   true,
							   []() {return prefs::get().network_host();},
							   [](const std::string& v) {prefs::get().set_network_host(v);},
							   true))
	, builtin_servers_(prefs::get().builtin_servers_list())
	, user_servers_(prefs::get().user_servers_list())
{
}

std::array<mp_connect::server_list*, 2> mp_connect::server_lists()
{
	return {{ &builtin_servers_, &user_servers_ }};
}

void mp_connect::pre_show()
{
	text_box& hostname_box = find_widget<text_box>("host_name");
	listbox& server_list = find_widget<listbox>("server_list");
	button& button_add = find_widget<button>("server_add");
	button& button_del = find_widget<button>("server_delete");

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
	const widget_data& entry{
		{ "name",    widget_item{{"label", srv.name}} },
		{ "address", widget_item{{"label", srv.address}} },
	};

	listbox.add_row(entry, pos);
}

void mp_connect::select_first_match()
{
	text_box& hostname_box = find_widget<text_box>("host_name");
	listbox& server_list = find_widget<listbox>("server_list");
	button& button_add = find_widget<button>("server_add");
	button& button_del = find_widget<button>("server_delete");

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
	text_box& hostname_box = find_widget<text_box>("host_name");
	listbox& server_list = find_widget<listbox>("server_list");

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
	const unsigned int ui_pos = selection.user_defined() ? 1 + selection.row() : builtin_servers_.size();

	std::string name;

	if(!gui2::dialogs::edit_text::execute(_("Add Server"), _("Name:"), name, true) || name.empty()) {
		return;
	}

	server_info info;
	info.name = name;
	info.address = address;

	user_servers_.insert(user_servers_.begin() + mem_pos, info);
	prefs::get().set_user_servers_list(user_servers_);

	insert_into_server_listbox(server_list, info, ui_pos);
	select_first_match();
}

void mp_connect::on_server_delete()
{
	listbox& server_list = find_widget<listbox>("server_list");

	auto selection = current_selection();

	if(!selection.valid() || !selection.user_defined()) {
		// We're not supposed to be here
		return;
	}

	user_servers_.erase(user_servers_.begin() + selection.relative_index());
	prefs::get().set_user_servers_list(user_servers_);

	server_list.remove_row(selection.row());
	on_server_select();
}

void mp_connect::on_server_select()
{
	text_box& hostname_box = find_widget<text_box>("host_name");
	button& button_add = find_widget<button>("server_add");
	button& button_del = find_widget<button>("server_delete");

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
	listbox& server_list = find_widget<listbox>("server_list");
	return { this, server_list.get_selected_row() };
}

mp_connect::server_info& mp_connect::selection::get()
{
	must_be_valid();
	return parent_list().at(relative_index());
}

bool mp_connect::selection::user_defined() const
{
	// An invalid selection is the same as one from the read-only list of
	// built-in servers for interaction purposes since it can't be written to.
	return valid() && std::size_t(row_) >= owner_->builtin_servers_.size();
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
