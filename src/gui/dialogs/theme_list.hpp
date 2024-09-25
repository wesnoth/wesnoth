/*
	Copyright (C) 2014 - 2024
	by Iris Morelle <shadowm2006@gmail.com>
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

struct theme_info;

namespace gui2::dialogs
{

class theme_list : public modal_dialog
{
public:
	explicit theme_list(const std::vector<theme_info>& themes,
						 int selection = -1);

	/**
	 * Returns the selected item index after displaying.
	 * @return -1 if the dialog was canceled.
	 */
	int selected_index() const
	{
		return index_;
	}

	/** Sets the initially selected item index (-1 by default). */
	void set_selected_index(int index)
	{
		index_ = index;
	}

private:
	int index_;

	std::vector<theme_info> themes_;

	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	virtual void post_show() override;
};
} // namespace dialogs
