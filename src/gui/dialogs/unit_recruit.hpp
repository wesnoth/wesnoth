/*
   Copyright (C) 2016 - 2018 by the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "gui/widgets/text_box_base.hpp"

class unit_type;
class team;

namespace gui2
{
namespace dialogs
{

class unit_recruit : public modal_dialog
{
public:
	/// @param recruit_map maps unit typs to strings. The strings are ""
	/// if the unit can be recalled and an error message string otherwise.
	unit_recruit(std::map<const unit_type*, std::string>& recruit_map, team& team);

	const unit_type *get_selected_unit_type() const
	{
		return selected_index_ < 0 ? nullptr : recruit_list_[selected_index_];
	}

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;
	virtual void post_show(window& window) override;

	void list_item_clicked(window& window);
	void filter_text_changed(text_box_base* textbox, const std::string& text);

	void show_help();

	/// A vector of unit types in the order listed in the UI. Used by unit_recruit::post_show.
	std::vector<const unit_type*> recruit_list_;
	std::map<const unit_type*, std::string>& recruit_map_;

	team& team_;

	int selected_index_;

	std::vector<std::string> last_words_;
};

} // namespace dialogs
} // namespace gui2
