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

#pragma once

#include "gui/dialogs/modal_dialog.hpp"

namespace gui2::dialogs
{

class addon_connect : public modal_dialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param host_name           The parameter's usage is:
	 *                            - Input: The initial value for the host_name.
	 *                            - Output :The final value of the host_name if
	 *                              the dialog returns retval::OK or 3
	 *                              undefined otherwise.
	 * @param allow_remove        Sets @ref allow_remove_.
	 */
	addon_connect(std::string& host_name, bool allow_remove);

private:
	/** Enable the addon remove button? */
	bool allow_remove_;

	void help_button_callback();

	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	virtual void post_show(window& window) override;
};

} // namespace dialogs
