/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_ADDON_CONNECT_HPP_INCLUDED
#define GUI_DIALOGS_ADDON_CONNECT_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

namespace gui2 {

/** Addon connect dialog. */
class taddon_connect
	: public tdialog
{
public:
	taddon_connect()
		: host_name_()
		, allow_updates_()
		, allow_remove_()
	{
	}

	bool allow_updates() const { return allow_updates_; }

	void set_allow_updates(bool allow_updates)
	{
		allow_updates_ = allow_updates;
	}

	bool allow_remove() const { return allow_remove_; }

	void set_allow_remove(bool allow_remove)
	{
		allow_remove_ = allow_remove;
	}

	const std::string& host_name() const { return host_name_; }

	void set_host_name(const std::string& host_name)
	{
		host_name_ = host_name;
	}

private:
	std::string host_name_;

	bool allow_updates_;
	bool allow_remove_;

	/** Inherited from tdialog. */
	twindow* build_window(CVideo& video);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
};

} // namespace gui2

#endif
