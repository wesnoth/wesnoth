/*
	Copyright (C) 2015 - 2024
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
 * Dialog for entering a password for joining a password-protected MP game.
 * Key               |Type          |Mandatory|Description
 * ------------------|--------------|---------|-----------
 * password          | text_box     |yes      |Input field for the game password.
 */
class mp_join_game_password_prompt : public modal_dialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param [in, out] password  The parameter's usage is:
	 *                            - Input: The initial value for the password.
	 *                            - Output: The password input by the user
	 *                              if the dialog returns retval::OK,
	 *                              undefined otherwise.
	 */
	explicit mp_join_game_password_prompt(std::string& password);

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(mp_join_game_password_prompt)

private:
	virtual const std::string& window_id() const override;
};

} // namespace dialogs
