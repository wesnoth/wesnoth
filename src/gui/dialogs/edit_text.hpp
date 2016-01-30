/*
   Copyright (C) 2013 - 2016 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_EDIT_TEXT_INCLUDED
#define GUI_DIALOGS_EDIT_TEXT_INCLUDED

#include "gui/dialogs/dialog.hpp"

namespace gui2
{

class tedit_text : public tdialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param title               The dialog's title.
	 * @param label               Label of the text field.
	 * @param [in, out] text      The parameter's usage is:
	 *                            - Input: The initial value of the text field.
	 *                            - Output: The new unit name the user entered
	 *                              if the dialog returns @ref twindow::OK,
	 *                              undefined otherwise.
	 */
	tedit_text(const std::string& title,
			   const std::string& label,
			   std::string& text);

	/**
	 * Executes the dialog.
	 * See @ref tdialog for more information.
	 *
	 * @param [in, out] text      The parameter's usage is:
	 *                            - Input:  The initial value of the unit name.
	 *                            - Output: The new unit name the user entered
	 *                              if this method returns @a true, undefined
	 *                              otherwise.
	 */
	static bool execute(const std::string& title,
						const std::string& label,
						std::string& text,
						CVideo& video)
	{
		return tedit_text(title, label, text).show(video);
	}

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;
};
}

#endif /* ! GUI_DIALOGS_EDIT_TEXT_INCLUDED */
