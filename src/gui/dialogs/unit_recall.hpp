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
#include "gui/widgets/group.hpp"
#include "units/race.hpp"
#include "units/ptr.hpp"

#include <memory>
#include <string>
#include <vector>

class team;

namespace gui2
{

class text_box_base;

namespace dialogs
{

class unit_recall : public modal_dialog
{
public:
	unit_recall(std::vector<unit_const_ptr>& recall_list, team& team);

	int get_selected_index() const
	{
		return selected_index_;
	}

private:
	std::vector<unit_const_ptr>& recall_list_;

	team& team_;

	int selected_index_;

	std::vector<std::string> filter_options_;
	std::vector<std::string> last_words_;

	/** Callbacks */
	void list_item_clicked();
	void filter_text_changed(const std::string& text);
	void rename_unit();
	void dismiss_unit();
	void show_help();

	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	virtual void post_show(window& window) override;
};

} // namespace dialogs
} // namespace gui2
