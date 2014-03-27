/*
   Copyright (C) 2008 - 2014 by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_SAVE_GAME_HPP_INCLUDED
#define GUI_DIALOGS_SAVE_GAME_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "tstring.hpp"

namespace gui2 {

class taddon_rate : public tdialog
{
public:


	static bool execute(int& input
			, CVideo& video)
	{
		return taddon_rate(input).show(video);
	}

private:

	taddon_rate(int& rate);

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog */
	void pre_show(CVideo& video, twindow& window);

};

}

#endif

