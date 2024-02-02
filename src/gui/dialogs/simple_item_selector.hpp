/*
	Copyright (C) 2010 - 2024
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

#include <vector>

namespace gui2::dialogs
{

/**
 * @ingroup GUIWindowDefinitionWML
 *
 * A simple one-column listbox with OK and Cancel buttons.
 * Key               |Type          |Mandatory|Description
 * ------------------|--------------|---------|-----------
 * title             | @ref label   |yes      |Dialog title label.
 * message           | control      |yes      |Text label displaying a description or instructions.
 * listbox           | @ref listbox |yes      |Listbox displaying user choices.
 * item              | control      |yes      |Widget which shows a listbox item label.
 * ok                | @ref button  |yes      |OK button.
 * cancel            | @ref button  |yes      |Cancel button.
 */
class simple_item_selector : public modal_dialog
{
public:
	typedef std::vector<std::string> list_type;

	simple_item_selector(const std::string& title,
						  const std::string& message,
						  const list_type& items,
						  bool title_uses_markup = false,
						  bool message_uses_markup = false);

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

	/** Sets the caption for the OK button. */
	void set_ok_label(const std::string& s)
	{
		ok_label_ = s;
	}
	const std::string& ok_label() const
	{
		return ok_label_;
	}

	/** Sets the caption for the Cancel button. */
	void set_cancel_label(const std::string& s)
	{
		cancel_label_ = s;
	}
	const std::string& cancel_label() const
	{
		return cancel_label_;
	}

	/** Sets whether the Cancel button should be hidden or not. */
	void set_single_button(bool value)
	{
		single_button_ = value;
	}
	bool single_button() const
	{
		return single_button_;
	}

private:
	int index_;

	bool single_button_;
	list_type items_;

	std::string ok_label_, cancel_label_;

	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	virtual void post_show(window& window) override;
};
} // namespace dialogs
