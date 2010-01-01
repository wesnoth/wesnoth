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

#ifndef GUI_DIALOGS_MP_CONNECT_HPP_INCLUDED
#define GUI_DIALOGS_MP_CONNECT_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "tstring.hpp"

namespace gui2 {

class tmp_connect : public tdialog
{
public:
	tmp_connect();

private:

	/** Used in show in order to show list. */
	CVideo* video_;

	/** Inherited from tdialog. */
	twindow* build_window(CVideo& video);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
public:
	void show_server_list(twindow& window);

private:
	tfield_text* host_name_;
};

class tmp_login : public tdialog
{
public:
	tmp_login(const t_string& label,
		const bool focus_password = false);

private:
	/** Inherited from tdialog. */
	twindow* build_window(CVideo& video);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	t_string label_;
	bool focus_password_;
};

} // namespace gui2

#endif

