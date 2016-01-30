/*
   Copyright (C) 2015 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_MP_JOIN_GAME_PASSWORD_PROMPT_HPP_INCLUDED
#define GUI_DIALOGS_MP_JOIN_GAME_PASSWORD_PROMPT_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

namespace gui2
{

class tmp_join_game_password_prompt : public tdialog
{
public:
	/**
	 * Constructor.
	 *
	 * @param [in, out] password  The parameter's usage is:
	 *                            - Input: The initial value for the password.
	 *                            - Output: The password input by the user
	 *                              if the dialog returns @ref twindow::OK,
	 *                              undefined otherwise.
	 */
	explicit tmp_join_game_password_prompt(std::string& password);

	/** The execute function -- see @ref tdialog for more information. */
	static bool execute(std::string& password, CVideo& video)
	{
		return tmp_join_game_password_prompt(password).show(video);
	}

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;
};

} // namespace gui2

#endif /* ! GUI_DIALOGS_MP_JOIN_GAME_PASSWORD_PROMPT_HPP_INCLUDED */
