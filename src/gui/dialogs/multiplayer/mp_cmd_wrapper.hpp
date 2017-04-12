/*
   Copyright (C) 2009 - 2017 by Thomas Baumhauer
   <thomas.baumhauer@NOSPAMgmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_MP_CMD_WRAPPER_HPP_INCLUDED
#define GUI_DIALOGS_MP_CMD_WRAPPER_HPP_INCLUDED

#include "gui/dialogs/modal_dialog.hpp"
#include "tstring.hpp"

namespace gui2
{
namespace dialogs
{

class mp_cmd_wrapper : public modal_dialog
{
public:
	/**
	 * Constructor.
	 *
	 * The text which shows the selected user.
	 */
	explicit mp_cmd_wrapper(const t_string& user);

	/***** ***** ***** setters / getters for members ***** ****** *****/

	const std::string& message() const
	{
		return message_;
	}
	const std::string& reason() const
	{
		return reason_;
	}
	const std::string& time() const
	{
		return time_;
	}

private:
	/** The message to send to another user. */
	std::string message_;

	/** The reason for an action; kick, ban. */
	std::string reason_;

	/** The duration of a ban. */
	std::string time_;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;
};

} // namespace dialogs
} // namespace gui2

#endif // GUI_DIALOGS_MP_CMD_WRAPPER_HPP_INCLUDED
