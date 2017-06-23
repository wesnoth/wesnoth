/*
   Copyright (C) 2013 - 2017 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gui/dialogs/modal_dialog.hpp"

namespace gui2
{
namespace dialogs
{

class edit_text : public modal_dialog
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
	 *                              if the dialog returns @ref window::OK,
	 *                              undefined otherwise.
	 */
	edit_text(const std::string& title,
			   const std::string& label,
			   std::string& text);

	/**
	 * Executes the dialog.
	 * See @ref modal_dialog for more information.
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
		return edit_text(title, label, text).show(video);
	}

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;
};
} // namespace dialogs
} // namespace gui2
