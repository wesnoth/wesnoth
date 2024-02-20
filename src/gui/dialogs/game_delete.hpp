/*
	Copyright (C) 2008 - 2024
	by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
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
 * This shows the dialog to confirm deleting a savegame file.
 * Key               |Type              |Mandatory|Description
 * ------------------|------------------|---------|-----------
 * dont_ask_again    | boolean_selector |yes      |A checkbox to not show this dialog again.
 */
class game_delete : public modal_dialog
{
public:
	game_delete();

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(game_delete)

private:
	virtual const std::string& window_id() const override;
};
} // namespace dialogs
