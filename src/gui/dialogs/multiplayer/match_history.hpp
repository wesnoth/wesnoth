/*
	Copyright (C) 2021 - 2022
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

namespace mp { struct user_info; }

class wesnothd_connection;

namespace gui2
{
class window;

namespace dialogs
{
class mp_match_history : public modal_dialog
{
public:
	mp_match_history(mp::user_info& info, wesnothd_connection& connection);

	/**
	 * The display function.
	 *
	 * See @ref modal_dialog for more information.
	 */
	DEFINE_SIMPLE_DISPLAY_WRAPPER(mp_match_history)

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	void request_history(int offset);

	mp::user_info& info_;

	wesnothd_connection& connection_;
};

} // namespace dialogs
} // namespace gui2
