/*
	Copyright (C) 2003 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "achievements.hpp"
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/menu_button.hpp"

#pragma once

namespace gui2::dialogs
{

class achievements_dialog : public modal_dialog
{
public:
	DEFINE_SIMPLE_EXECUTE_WRAPPER(achievements_dialog)

	achievements_dialog();

private:
	achievements achieve_;
	std::string last_selected_;
	listbox* achievements_box_;
	menu_button* content_names_;

	void set_sub_achievements(grid& newrow, const achievement& ach);

	void set_achievements_row();

	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	virtual void post_show() override;
};

} // namespace gui2::dialogs
