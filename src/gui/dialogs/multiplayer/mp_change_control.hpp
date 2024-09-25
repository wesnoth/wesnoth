/*
	Copyright (C) 2011 - 2024
	by Lukasz Dobrogowski <lukasz.dobrogowski@gmail.com>
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

#include <vector>

namespace events
{
	class menu_handler;
}

namespace gui2::dialogs
{

class mp_change_control : public modal_dialog
{
public:
	explicit mp_change_control(events::menu_handler& mh);

	DEFINE_SIMPLE_DISPLAY_WRAPPER(mp_change_control)

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	virtual void post_show() override;

	void handle_sides_list_item_clicked();
	void handle_nicks_list_item_clicked();

	void highlight_side_nick();

	events::menu_handler& menu_handler_;

	unsigned int selected_side_;
	unsigned int selected_nick_;

	// Contains the mapping from listbox labels to actual sides
	std::vector<int> sides_;

	// Contains the mapping from listbox labels to actual nicks
	std::vector<std::string> nicks_;
};

} // namespace dialogs
