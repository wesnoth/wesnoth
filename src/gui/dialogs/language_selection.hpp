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
 * This shows the dialog to select the language to use.
 * When the dialog is closed with the OK button it also updates the selected language in the preferences.
 * Key               |Type          |Mandatory|Description
 * ------------------|--------------|---------|-----------
 * language_list     | @ref listbox |yes      |This listbox contains the list with available languages.
 * free to choose    | control      |no       |Show the name of the language in the current row.
 */
class language_selection : public modal_dialog
{
public:
	language_selection()
		: modal_dialog(window_id())
	{
	}

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(language_selection)

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	virtual void post_show(window& window) override;
};

} // namespace dialogs
