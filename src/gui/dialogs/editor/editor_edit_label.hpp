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

#ifndef GUI_DIALOGS_EDITOR_EDIT_LABEL_HPP_INCLUDED
#define GUI_DIALOGS_EDITOR_EDIT_LABEL_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "map_label.hpp"

namespace gui2 {

class teditor_edit_label : public tdialog
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
	teditor_edit_label(std::string& text, bool& immutable, bool& visible_fog, bool& visible_shroud);

	/** The execute function see @ref tdialog for more information. */
	static bool execute(std::string& text, bool& immutable, bool& visible_fog, bool& visible_shroud, CVideo& video)
	{
		return teditor_edit_label(text, immutable, visible_fog, visible_shroud).show(video);
	}

private:

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;
};

}

#endif /* ! GUI_DIALOGS_EDIT_LABEL_INCLUDED */
