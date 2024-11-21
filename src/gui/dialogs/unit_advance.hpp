/*
	Copyright (C) 2016 - 2024
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
#include "units/ptr.hpp"

namespace gui2::dialogs
{

class unit_advance : public modal_dialog
{
public:
	unit_advance(const std::vector<unit_const_ptr>& samples, std::size_t real);

	int get_selected_index() const
	{
		return selected_index_;
	}

private:
	virtual const std::string& window_id() const override;
	virtual void pre_show() override;
	virtual void post_show() override;

	void list_item_clicked();

	void show_help();

	const std::vector<unit_const_ptr>& previews_;

	std::size_t selected_index_, last_real_advancement_;
};

} // namespace dialogs
