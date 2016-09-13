/*
   Copyright (C) 2009 - 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_FACTION_SELECT_HPP_INCLUDED
#define GUI_DIALOGS_FACTION_SELECT_HPP_INCLUDED

#include "game_initialization/flg_manager.hpp"
#include "gui/dialogs/dialog.hpp"
#include "gui/widgets/group.hpp"

#include <string>
#include <vector>

namespace gui2
{

class tfaction_select : public tdialog
{
public:
	tfaction_select(ng::flg_manager& flg_manager, const std::string& color, const int side);

private:
	ng::flg_manager& flg_manager_;

    const std::string tc_color_;

	const int side_;

	tgroup<std::string> gender_toggle_;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(twindow& window);

	/** Callbacks */
	void on_faction_select(twindow& window);

	void on_leader_select(twindow& window);

	void on_gender_select(twindow& window);

	void update_leader_image(twindow& window);
};

}

#endif /* ! GUI_DIALOGS_FACTION_SELECT_HPP_INCLUDED */
