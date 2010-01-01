/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Thomas Baumhauer <thomas.baumhauer@NOSPAMgmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_MP_CMD_WRAPPER_HPP_INCLUDED
#define GUI_DIALOGS_MP_CMD_WRAPPER_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "tstring.hpp"

namespace gui2 {

class tmp_cmd_wrapper : public tdialog
{
public:
	tmp_cmd_wrapper(const t_string& user);

	const std::string& message() const { return message_; }
	const std::string& reason() const { return reason_; }
	const std::string& time() const { return time_; }

private:
	/** Inherited from tdialog. */
	twindow* build_window(CVideo& video);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	std::string message_;
	std::string reason_;
	std::string time_;

	t_string user_;
};

} // namespace gui2

#endif // GUI_DIALOGS_MP_CMD_WRAPPER_HPP_INCLUDED

