/*
	Copyright (C) 2010 - 2024
	by Fabian Müller <fabianmueller5@gmx.de>
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
 * Dialog for editing gamemap scenarios.
 * Key               |Type           |Mandatory|Description
 * ------------------|---------------|---------|-----------
 * title             | @ref label    |yes      |Dialog title label.
 * label             | @ref text_box |yes      |Input field for the map label.
 * team_only_toggle  | toggle_button |yes      |Checkbox for whether to make the label visible to the player's team only or not.
 */
class editor_edit_scenario : public modal_dialog
{
public:
	editor_edit_scenario(std::string& id,
						  std::string& name,
						  std::string& description,
						  int& turns,
						  int& experience_modifier,
						  bool& victory_when_enemies_defeated,
						  bool& random_start_time);

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(editor_edit_scenario)

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show(window& window) override;
};
} // namespace dialogs
