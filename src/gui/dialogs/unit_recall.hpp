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
	typedef std::vector<unit_const_ptr> recalls_ptr_vector;

public:
	unit_recall(recalls_ptr_vector& recall_list, team& team);

	int get_selected_index() const
	{
		return selected_index_;
	}

private:
	recalls_ptr_vector& recall_list_;

	team& team_;

	int selected_index_;

	std::vector<std::string> filter_options_;
	std::vector<std::string> last_words_;

	/** Callbacks */
	void list_item_clicked(window& window);
	void filter_text_changed(text_box_base* textbox, const std::string& text);
	void rename_unit(window& window);
	void dismiss_unit(window& window);
	void show_help();

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog. */
	virtual void post_show(window& window) override;
};

} // namespace dialogs
} // namespace gui2
