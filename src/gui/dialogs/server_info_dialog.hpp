/*
	Copyright (C) 2020 - 2024
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
class server_info : public modal_dialog
{
public:
	/**
	 * Constructor.
	 */
	server_info(const std::string& info, const std::string& announcement);

	/**
	 * The display function.
	 *
	 * See @ref modal_dialog for more information.
	 */
	static void display(const std::string& info, const std::string& announcements)
	{
		server_info(info, announcements).show();
	}

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	void tab_switch_callback();

	const std::string& server_information_;
	const std::string& announcements_;
};

} // namespace gui2::dialogs
