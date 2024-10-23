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
#include "gui/widgets/listbox.hpp"
#include "team.hpp"
#include "units/ptr.hpp"
#include "units/types.hpp"

#include <string>
#include <vector>

class team;

namespace gui2
{


namespace dialogs
{

class unit_recall : public modal_dialog
{
public:
	unit_recall(std::vector<const unit_type*>& recruit_list, team* team = nullptr);
	unit_recall(std::vector<unit_const_ptr>& recall_list, team* team = nullptr);

	int get_selected_index() const
	{
		return selected_index_;
	}

	/** Gender choice from the user. */
	unit_race::GENDER gender()
	{
		return gender_;
	}

	/** Variation choice from the user. */
	std::string variation() const
	{
		return variation_;
	}

	enum class dialog_type {
		RECRUIT,
		RECALL,
		UNIT_LIST,
		UNIT_CREATE
	};

	void set_mode(dialog_type mode) {
		mode_ = mode;
		update_dialog();
	}

private:
	std::vector<const unit_type*> recruit_list_;
	std::vector<unit_const_ptr> recall_list_;

	team* team_;

	int selected_index_;

	unit_race::GENDER gender_;
	std::string variation_;

	std::vector<std::string> filter_options_;
	std::vector<std::string> last_words_;
	group<unit_race::GENDER> gender_toggle_;

	dialog_type mode_;

	void update_dialog();

	/** Callbacks */
	void list_item_clicked();
	void filter_text_changed();
	void rename_unit();
	void dismiss_unit();
	void show_help();

	t_string can_recruit(const std::string& name, int side_num, team& team, map_location& loc, map_location& recruited_from);

	void show_unit_types(listbox& list);
	void show_units(listbox& list);

	void gender_toggle_callback(const unit_race::GENDER val);
	void variation_menu_callback();

	virtual const std::string& window_id() const override;

	virtual void pre_show() override;
	virtual void post_show() override;
};

} // namespace dialogs
} // namespace gui2
