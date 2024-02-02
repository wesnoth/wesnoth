/*
	Copyright (C) 2017 - 2024
	by Amir Hassan <amir@viel-zu.org>
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
 * This shows the dialog to confirm surrender and/or quitting the game.
 */
class surrender_quit : public modal_dialog
{
public:
	surrender_quit();

	/** The execute function. See @ref modal_dialog for more information. */
	DEFINE_SIMPLE_EXECUTE_WRAPPER(surrender_quit)

private:
	virtual const std::string& window_id() const override;
};
} // namespace dialogs
