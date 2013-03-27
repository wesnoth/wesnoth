/*
   Copyright (C) 2010 - 2013 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_EDIT_LABEL_HPP_INCLUDED
#define GUI_DIALOGS_EDIT_LABEL_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

namespace gui2 {

class tedit_label : public tdialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param label [in]          The initial value of the label.
	 * @param label [out]         The label text the user entered if the dialog
	 *                            returns @ref twindow::OK undefined otherise.
	 * @param team_only [in]      The initial value of the team only toggle.
	 * @param team_only [out]     The final value of the team only toggle if the
	 *                            dialog returns @ref twindow::OK undefined
	 *                            otherise.
	 */
	tedit_label(std::string& label, bool& team_only);

	/** The excute function see @ref tdialog for more information. */
	static bool execute(std::string& label, bool& team_only, CVideo& video)
	{
		return tedit_label(label, team_only).show(video);
	}

private:

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;
};

}

#endif /* ! GUI_DIALOGS_EDIT_LABEL_INCLUDED */
