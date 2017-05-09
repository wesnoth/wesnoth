/*
   Copyright (C) 2008 - 2017 by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
   Copyright (C) 2012 - 2017 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

class mp_host_game_prompt : public modal_dialog
{
public:
	mp_host_game_prompt();

	/** The execute function see @ref modal_dialog for more information. */
	static bool execute(CVideo& video)
	{
		return mp_host_game_prompt().show(video);
	}

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;
};
} // namespace dialogs
} // namespace gui2
