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

#pragma once

#include "gui/dialogs/modal_dialog.hpp"

namespace gui2
{
namespace dialogs
{

class mp_method_selection : public modal_dialog
{
public:
	mp_method_selection() : user_name_(), choice_(-1)
	{
	}

	const std::string& user_name() const
	{
		return user_name_;
	}

	int get_choice() const
	{
		return choice_;
	}

private:
	/** The name to use on the MP server. */
	std::string user_name_;

	/** The selected method to `connect' to the MP server. */
	int choice_;

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog. */
	virtual void post_show(window& window) override;
};

} // namespace dialogs
} // namespace gui2
