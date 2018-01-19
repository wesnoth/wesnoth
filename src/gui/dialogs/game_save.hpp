/*
   Copyright (C) 2008 - 2018 by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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

class game_save : public modal_dialog
{
public:
	game_save(std::string& filename, const std::string& title);

	static bool
	execute(std::string& filename, const std::string& title)
	{
		return game_save(filename, title).show();
	}

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
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
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
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
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;
};
} // namespace dialogs
} // namespace gui2
