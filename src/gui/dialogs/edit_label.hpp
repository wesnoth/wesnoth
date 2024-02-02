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

namespace gui2::dialogs
{

/**
 * @ingroup GUIWindowDefinitionWML
 *
 * Dialog for editing gamemap labels.
 * Key               |Type           |Mandatory|Description
 * ------------------|---------------|---------|-----------
 * title             | @ref label    |yes      |Dialog title label.
 * label             | @ref text_box |yes      |Input field for the map label.
 * team_only_toggle  | toggle_button |yes      |Checkbox for whether to make the label visible to the player's team only or not.
 */
class edit_label : public modal_dialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param [in, out] label     The parameter's usage is:
	 *                            - Input: The initial value of the label.
	 *                            - Output: The label text the user entered if
	 *                              the dialog returns retval::OK
	 *                              undefined otherwise.
	 * @param [in, out] team_only The parameter's usage is:
	 *                            - Input: The initial value of the team only
	 *                              toggle.
	 *                            - Output: The final value of the team only
	 *                              toggle if the dialog returns retval::OK undefined otherwise.
	 */
	edit_label(std::string& label, bool& team_only);

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(edit_label)

private:
	virtual const std::string& window_id() const override;
};
} // namespace dialogs
