/*
   Copyright (C) 2009 - 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/group.hpp"

#include <string>
#include <vector>

namespace gui2
{
namespace dialogs
{

class faction_select : public modal_dialog
{
public:
	faction_select(ng::flg_manager& flg_manager, const std::string& color, const int side);

private:
	ng::flg_manager& flg_manager_;

    const std::string tc_color_;

	const int side_;

	group<std::string> gender_toggle_;

	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from modal_dialog. */
	void pre_show(window& window);

	/** Callbacks */
	void on_faction_select(window& window);

	void on_leader_select(window& window);

	void on_gender_select(window& window);

	void update_leader_image(window& window);
};

} // namespace dialogs
} // namespace gui2

#endif /* ! GUI_DIALOGS_FACTION_SELECT_HPP_INCLUDED */
