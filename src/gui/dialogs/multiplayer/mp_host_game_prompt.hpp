/*
	Copyright (C) 2012 - 2024
	by Iris Morelle <shadowm2006@gmail.com>
	Copyright (C) 2008 - 2018 by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
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
 * do_not_show_again | boolean_selector |yes      |A checkbox to not show this dialog again.
 */
class mp_host_game_prompt : public modal_dialog
{
public:
	mp_host_game_prompt();

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(mp_host_game_prompt)

private:
	virtual const std::string& window_id() const override;
};
} // namespace dialogs
