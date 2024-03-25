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
 * This shows the dialog to create a savegame file.
 * Key               |Type          |Mandatory|Description
 * ------------------|--------------|---------|-----------
 * lblTitle          | @ref label   |yes      |The title of the window.
 * txtFilename       | text_box     |yes      |The name of the savefile.
 */
class game_save : public modal_dialog
{
public:
	game_save(std::string& filename, const std::string& title);

	DEFINE_SIMPLE_EXECUTE_WRAPPER(game_save)

private:
	virtual const std::string& window_id() const override;
};

class game_save_message : public modal_dialog
{
public:
	game_save_message(std::string& filename,
					   const std::string& title,
					   const std::string& message);

	DEFINE_SIMPLE_EXECUTE_WRAPPER(game_save_message)

private:
	virtual const std::string& window_id() const override;
};

class game_save_oos : public modal_dialog
{
public:
	game_save_oos(bool& ignore_all,
				   std::string& filename,
				   const std::string& title,
				   const std::string& message);

	DEFINE_SIMPLE_EXECUTE_WRAPPER(game_save_oos)

private:
	virtual const std::string& window_id() const override;
};
} // namespace dialogs
