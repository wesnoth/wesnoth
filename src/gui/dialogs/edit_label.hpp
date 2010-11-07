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

#ifndef GUI_DIALOGS_EDIT_LABEL_HPP_INCLUDED
#define GUI_DIALOGS_EDIT_LABEL_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "gui/dialogs/field-fwd.hpp"

#include <vector>

namespace gui2 {

class tedit_label : public tdialog
{
public:
	tedit_label(const std::string& label, bool team_only = true);

	const std::string& label() const {
		return label_;
	}

	void set_label(const std::string& label) {
		label_ = label;
	}

	bool team_only() const {
		return team_only_;
	}

	void set_team_only(bool team_only) {
		team_only_ = team_only;
	}

private:
	bool team_only_;
	std::string label_;

	tfield_text* label_field_;

	/** Inherited from tdialog, implemented by REGISTER_WINDOW. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
};

}

#endif /* ! GUI_DIALOGS_EDIT_LABEL_INCLUDED */
