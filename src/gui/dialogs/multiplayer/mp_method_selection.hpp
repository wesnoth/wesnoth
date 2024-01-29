/*
	Copyright (C) 2008 - 2024
	by Mark de Wever <koraq@xs4all.nl>
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

namespace gui2::dialogs
{

/**
 * @ingroup GUIWindowDefinitionWML
 *
 * This shows the dialog to select the kind of MP game the user wants to play.
 * Key               |Type          |Mandatory|Description
 * ------------------|--------------|---------|-----------
 * user_name         | text_box     |yes      |This text contains the name the user on the MP server. This widget will get a fixed maximum length by the engine.
 * method_list       | @ref listbox |yes      |The list with possible game methods.
 */
class mp_method_selection : public modal_dialog
{
public:
	/** Corresponds to each connection option. */
	enum class choice { JOIN = 0, CONNECT, HOST, LOCAL };

	mp_method_selection()
		: modal_dialog(window_id()) , user_name_(), choice_()
	{
	}

	const std::string& user_name() const
	{
		return user_name_;
	}

	choice get_choice() const
	{
		return choice_;
	}

private:
	/** The name to use on the MP server. */
	std::string user_name_;

	/** The selected method to `connect' to the MP server. */
	choice choice_;

	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	virtual void post_show(window& window) override;
};

} // namespace dialogs
