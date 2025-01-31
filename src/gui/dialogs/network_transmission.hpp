/*
	Copyright (C) 2011 - 2024
	by Sergey Popov <loonycyborg@gmail.com>
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

#include "events.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "network_asio.hpp"
#include "utils/optional_reference.hpp"

namespace gui2::dialogs
{

/**
 * Dialog that tracks network transmissions
 *
 * It shows upload/download progress and allows the user
 * to cancel the transmission.
 */
class network_transmission : public modal_dialog
{
public:
	/** A wrapper of either a wesnothd_connection or a network_asio::connection. */
	class connection_data
	{
	public:
		virtual std::size_t total() { return 0; }
		virtual std::size_t current() { return 0; }
		virtual bool finished() = 0;
		virtual void cancel() = 0;
		virtual void poll() = 0;
		virtual ~connection_data() {}
	};

private:
	connection_data* connection_;

	class pump_monitor : public events::pump_monitor
	{
	public:
		connection_data*& connection_;
		virtual void process();

		pump_monitor(connection_data*& connection)
			: connection_(connection), window_()
		{
		}

		utils::optional_reference<window> window_;
	} pump_monitor_;

public:
	network_transmission(connection_data& connection,
						  const std::string& title,
						  const std::string& subtitle);

protected:
	virtual void pre_show() override;

	virtual void post_show() override;

private:
	/**
	 * The subtitle for the dialog.
	 *
	 * This field commonly shows the action in progress eg connecting,
	 * uploading, downloading etc..
	 */
	std::string subtitle_;

	virtual const std::string& window_id() const override;
};

} // namespace dialogs
