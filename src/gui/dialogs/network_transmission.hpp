/*
   Copyright (C) 2011 - 2016 by Sergey Popov <loonycyborg@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_NETWORK_RECEIVE_HPP_INCLUDED
#define GUI_DIALOGS_NETWORK_RECEIVE_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "gui/widgets/control.hpp"
#include "network_asio.hpp"
#include <boost/optional.hpp>
#include "events.hpp"

namespace gui2
{


/**
 * Dialog that tracks network transmissions
 *
 * It shows upload/download progress and allows the user
 * to cancel the transmission.
 */
class tnetwork_transmission : public tdialog
{
public:
	class connection_data 
	{
	public:
		virtual size_t total() { return 0; }
		virtual size_t current() { return 0; }
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
		virtual void process(events::pump_info&);

		pump_monitor(connection_data*& connection)
			: connection_(connection), window_()
		{
		}

		boost::optional<twindow&> window_;
	} pump_monitor_;

public:
	tnetwork_transmission(connection_data& connection,
						  const std::string& title,
						  const std::string& subtitle);

	void set_subtitle(const std::string&);
	void set_connection_data(connection_data& connection)
	{
		connection_ = &connection;
	}

protected:
	/** Inherited from tdialog. */
	void pre_show(twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

private:
	/**
	 * The subtitle for the dialog.
	 *
	 * This field commonly shows the action in progress eg connecting,
	 * uploading, downloading etc..
	 */
	std::string subtitle_;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;
};

} // namespace gui2

#endif
