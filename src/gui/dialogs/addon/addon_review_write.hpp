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
#include "addon/info.hpp"

namespace gui2 {

class taddon_review_write : public tdialog
{
public:

	static bool execute(addon_info::addon_review& review
			, CVideo& video)
	{
		return taddon_review_write(review).show(video);
	}

private:

	taddon_review_write(addon_info::addon_review& review);

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;
};

}

#endif

