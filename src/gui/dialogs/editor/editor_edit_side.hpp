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
#include "config.hpp"
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
	void pre_show(CVideo& /*video*/, twindow& window);

	template <typename T>
	void register_radio_toggle(twindow& window, const std::string& toggle_id, T enum_value, T& current_value, std::vector<std::pair<ttoggle_button*, T> >& dst);

	team::CONTROLLER& controller_;

	team::SHARE_VISION& share_vision_;

	typedef std::pair<ttoggle_button*, team::CONTROLLER> controller_toggle;
	std::vector<controller_toggle> controller_tgroup_;

	typedef std::pair<ttoggle_button*, team::SHARE_VISION> vision_toggle;
	std::vector<vision_toggle> vision_tgroup_;

	template <typename C>
	void toggle_radio_callback(const std::vector<std::pair<ttoggle_button*, C> >& vec, C& value, ttoggle_button* active);

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;
};
}

#endif /* ! GUI_DIALOGS_EDIT_LABEL_INCLUDED */
