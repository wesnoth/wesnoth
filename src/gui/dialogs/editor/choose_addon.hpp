/*
	Copyright (C) 2023 - 2024
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
 * Dialog for adding a translation while editing an add-on's _server.pbl.
 * Key               |Type           |Mandatory|Description
 * ------------------|---------------|---------|-----------
 * existing_addons   | @ref listbox  |yes      |A listbox that contains all available options for choosing an add-on
 */
class editor_choose_addon : public modal_dialog
{
public:
	DEFINE_SIMPLE_EXECUTE_WRAPPER(editor_choose_addon)

	editor_choose_addon(std::string& addon_id);

private:
	std::string& addon_id_;

	void toggle_installed();
	void populate_list(bool show_all);

	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;

	virtual void post_show(window& window) override;
};

} // namespace gui2::dialogs
