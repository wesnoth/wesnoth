/*
	Copyright (C) 2021 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "gui/dialogs/modal_dialog.hpp"

class wesnothd_connection;

namespace gui2
{
class window;

namespace dialogs
{
class mp_match_history : public modal_dialog
{
public:
	/**
	 * Creates a dialog to view a player's history 10 games at a time.
	 *
	 * @param player_name The username of the player whose history is being viewed
	 * @param connection A reference to the lobby's network connection to wesnothd
	 * @param wait_for_response Whether to wait a few seconds for a response or not.
	 */
	mp_match_history(const std::string& player_name, wesnothd_connection& connection, bool wait_for_response = true);

	/**
	 * The display function.
	 *
	 * See @ref modal_dialog for more information.
	 */
	static void display(const std::string& player_name, wesnothd_connection& connection, bool wait_for_response = true)
	{
		mp_match_history(player_name, connection, wait_for_response).show();
	}

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	/**
	 * Requests game history from the server based on the offset.
	 * 11 rows are returned and 10 displayed - the presence of the 11th indicates whether incrementing the offset again would return any data.
	 * A request can time out if the server takes too long to respond so that a failure by the server to respond at all doesn't lock the game indefinitely.
	 *
	 * @return A config containing the game history information or an empty config if either the request times out or returns with an error
	 */
	const config request_history();

	/**
	 * Updates the dialog with the information returned by the server.
	 * This is called on dialog open as well as when incrementing or decrementing the offset.
	 *
	 * @return Whether the game history information was returned by the server or not.
	 */
	bool update_display();

	/**
	 * Handles changing the selected horizontal listbox item for the specified game history row.
	 */
	void tab_switch_callback();

	/** Executes a new search for the entered username */
	void new_search();
	/** Increments the offset to use for querying data by 10 and updates the information displayed by the dialog. */
	void newer_history_offset();
	/** Decrements the offset to use for querying data by 10 and updates the information displayed by the dialog. */
	void older_history_offset();

	/** The username of the player whose history is being viewed */
	std::string player_name_;
	/** A reference to the lobby's network connection to wesnothd */
	wesnothd_connection& connection_;
	/** The offset to start retrieving history data at - should be increments of 10 */
	int offset_;
	/**
	 * Whether to wait a few seconds for a response or not.
	 * True for the regular client.
	 * False for the boost unit tests, since otherwise the gui2 test times out waiting for the request to the dummy wesnothd_connection to fail.
	 */
	bool wait_for_response_;
};

} // namespace dialogs
} // namespace gui2
