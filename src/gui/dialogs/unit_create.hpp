/*
   Copyright (C) 2009 - 2017 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "units/gender.hpp"

#include <string>
#include <vector>

class display;
class unit_type;

namespace gui2
{

class text_box_base;

namespace dialogs
{

class unit_create : public modal_dialog
{
public:
	unit_create();

	/** Unit type choice from the user. */
	const std::string& choice() const
	{
		return choice_;
	}

	/** Whether the user actually chose a unit type or not. */
	bool no_choice() const
	{
		return choice_.empty();
	}

	/** Gender choice from the user. */
	const unit_gender* gender()
	{
		return gender_;
	}

private:
	std::vector<const unit_type*> units_;

	const unit_gender* gender_;

	std::string choice_;

	std::vector<std::string> last_words_;

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;

	/** Inherited from modal_dialog. */
	virtual void pre_show(window& window) override;

	/** Inherited from modal_dialog. */
	virtual void post_show(window& window) override;

	/** Callbacks */
	void list_item_clicked(window& window);
	void filter_text_changed(text_box_base* textbox, const std::string& text);
	void gender_toggle_callback(window& window);

	void update_displayed_type() const;

	group<const unit_gender*> gender_toggle;
};
} // namespace dialogs
} // namespace gui2
