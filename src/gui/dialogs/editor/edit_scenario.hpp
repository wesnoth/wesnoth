/*
   Copyright (C) 2010 - 2017 by Fabian Mueller <fabianmueller5@gmx.de>
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

	/** The execute function see @ref modal_dialog for more information. */
	static bool execute(std::string& id,
						std::string& name,
						std::string& description,
						int& turns,
						int& experience_modifier,
						bool& victory_when_enemies_defeated,
						bool& random_start_time,
						CVideo& video)
	{
		return editor_edit_scenario(id,
									 name,
									 description,
									 turns,
									 experience_modifier,
									 victory_when_enemies_defeated,
									 random_start_time).show(video);
	}

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;
};
} // namespace dialogs
} // namespace gui2
