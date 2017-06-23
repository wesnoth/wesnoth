/*
   Copyright (C) 2010 - 2017 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

class edit_label : public modal_dialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param [in, out] label     The parameter's usage is:
	 *                            - Input: The initial value of the label.
	 *                            - Output: The label text the user entered if
	 *                              the dialog returns @ref window::OK
	 *                              undefined otherwise.
	 * @param [in, out] team_only The parameter's usage is:
	 *                            - Input: The initial value of the team only
	 *                              toggle.
	 *                            - Output: The final value of the team only
	 *                              toggle if the dialog returns @ref
	 *                              window::OK undefined otherwise.
	 */
	edit_label(std::string& label, bool& team_only);

	/** The execute function see @ref modal_dialog for more information. */
	static bool execute(std::string& label, bool& team_only, CVideo& video)
	{
		return edit_label(label, team_only).show(video);
	}

private:
	/** Inherited from modal_dialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const override;
};
} // namespace dialogs
} // namespace gui2
