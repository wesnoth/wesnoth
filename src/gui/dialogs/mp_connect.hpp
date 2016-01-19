/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_MP_CONNECT_HPP_INCLUDED
#define GUI_DIALOGS_MP_CONNECT_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

namespace gui2
{

class tmp_connect : public tdialog
{
	/** The unit test needs to be able to test the tmp_connect dialog. */
	friend tdialog* unit_test_mp_server_list();

public:
	tmp_connect();

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** The host name of the selected servef. */
	tfield_text* host_name_;

	/**
	 * The unit test needs to be able to test the tmp_connect dialog.
	 *
	 * @returns                   A newly allocated tmp_server_list.
	 */
	static tdialog* mp_server_list_for_unit_test();
};

} // namespace gui2

#endif
