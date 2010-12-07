/* $Id$ */
/*
   Copyright (C) 2010 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_MP_CREATE_GAME_SET_PASSWORD_HPP_INCLUDED
#define GUI_DIALOGS_MP_CREATE_GAME_SET_PASSWORD_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "gui/dialogs/field-fwd.hpp"

namespace gui2 {

class tmp_create_game_set_password : public tdialog
{
public:
	tmp_create_game_set_password(const std::string& password);

	const std::string& password() const {
		return password_;
	}

	void set_password(const std::string& password) {
		password_ = password;
	}

private:
	std::string password_;
	tfield_text* password_field_;

	/** Inherited from tdialog, implemented by REGISTER_WINDOW. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
};

}

#endif /* ! GUI_DIALOGS_MP_CREATE_GAME_SET_PASSWORD_HPP_INCLUDED */
