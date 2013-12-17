/*
   Copyright (C) 2010 - 2013 by Fabian Mueller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_EDITOR_EDIT_SCENARIO_HPP_INCLUDED
#define GUI_DIALOGS_EDITOR_EDIT_SCENARIO_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "config.hpp"

namespace gui2 {

class teditor_edit_scenario : public tdialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param[in] text            The initial value of the label.
	 * @param[out] text           The label text the user entered if the dialog
	 *                            returns @ref twindow::OK undefined otherwise.
	 * @param[in] team_only       The initial value of the team only toggle.
	 * @param[out] team_only      The final value of the team only toggle if the
	 *                            dialog returns @ref twindow::OK undefined
	 *                            otherwise.
	 */
//	teditor_edit_scenario(std::string& text, bool& immutable, bool& visible_fog, bool& visible_shroud);

	teditor_edit_scenario(std::string& id, std::string& name, std::string& description,
			int& turns, int& experience_modifier, bool& victory_when_enemies_defeated, bool& random_start_time);

	/** The execute function see @ref tdialog for more information. */
	static bool execute(std::string& id, std::string& name, std::string& description,
			int& turns, int& experience_modifier,
			bool& victory_when_enemies_defeated, bool& random_start_time,
			CVideo& video)
	{
		return teditor_edit_scenario(id, name, description,
				turns, experience_modifier, victory_when_enemies_defeated, random_start_time).show(video);
	}

private:

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

//	std::string id_;
//	std::string name_;
//	int turns_;
};

}

#endif /* ! GUI_DIALOGS_EDIT_LABEL_INCLUDED */
