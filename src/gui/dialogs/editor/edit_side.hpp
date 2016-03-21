/*
   Copyright (C) 2010 - 2016 by Fabian Müller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_EDITOR_EDIT_SIDE_HPP_INCLUDED
#define GUI_DIALOGS_EDITOR_EDIT_SIDE_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "gui/widgets/group.hpp"
#include "team.hpp"

namespace gui2
{

class ttoggle_button;

class teditor_edit_side : public tdialog
{
public:
	teditor_edit_side(int side,
					  std::string& team_name,
					  std::string& user_team_name,
					  int& gold,
					  int& income,
					  int& village_income,
					  int& village_support,
					  bool& fog,
					  bool& shroud,
					  team::SHARE_VISION& share_vision,
					  team::CONTROLLER& controller,
					  bool& no_leader,
					  bool& hidden);

	/** The execute function see @ref tdialog for more information. */
	static bool execute(int side,
						std::string& team_name,
						std::string& user_team_name,
						int& gold,
						int& income,
						int& village_income,
						int& village_support,
						bool& fog,
						bool& shroud,
						team::SHARE_VISION& share_vision,
						team::CONTROLLER& controller,
						bool& no_leader,
						bool& hidden,
						CVideo& video)
	{
		return teditor_edit_side(side,
								 team_name,
								 user_team_name,
								 gold,
								 income,
								 village_income,
								 village_support,
								 fog,
								 shroud,
								 share_vision,
								 controller,
								 no_leader,
								 hidden).show(video);
	}

private:
	void pre_show(twindow& window);
	void post_show(twindow& window);

	template <typename T>
	void register_radio_toggle(const std::string& toggle_id, tgroup<T>& group, const T& enum_value, T& current_value, twindow& window);

	team::CONTROLLER& controller_;
	tgroup<team::CONTROLLER> controller_group;

	team::SHARE_VISION& share_vision_;
	tgroup<team::SHARE_VISION> vision_group;

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;
};
}

#endif /* ! GUI_DIALOGS_EDIT_LABEL_INCLUDED */
