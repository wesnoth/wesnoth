/*
	Copyright (C) 2023 - 2023
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

namespace gui2
{

namespace dialogs
{

/**
 * @ingroup GUIWindowDefinitionWML
 *
 * Dialog for getting a single text value from the player.
 * Key               |Type           |Mandatory|Description
 * ------------------|---------------|---------|-----------
 * enter_text_box    | text_box      |yes      |The text box to enter the value into.
 */
class enter_text : public modal_dialog
{
public:
	enter_text(std::string& value);

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(enter_text)

private:
	virtual void pre_show(window& window) override;
	virtual void post_show(window& window) override;

	virtual const std::string& window_id() const override;

    std::string& value_;
};

}
}
