/*
   Copyright (C) 2011, 2015 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_PREFERENCES_DIALOG_HPP_INCLUDED
#define GUI_DIALOGS_PREFERENCES_DIALOG_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"

// This file is not named preferences.hpp in order -I conflicts with
// src/preferences.hpp.

namespace gui2
{

class tpreferences : public tdialog
{
public:
	/**
	 * Constructor.
	 */
	tpreferences();

	/** The execute function -- see @ref tdialog for more information. */
	//
	// TODO: we want to get status info out in case it's necessary, e.g.
	//       to refresh the game display or stuff, hence this is execute() and
	//       not display(). But we probably should drop this later and force
	//       callers communicate with the dialog directly. Undecided. -- shadowm
	//
	static bool execute(CVideo& video)
	{
		tpreferences().show(video);
		return true;
	}

private:
	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

	void add_pager_row(class tlistbox& selector, const std::string& icon, const std::string& label);
	void set_visible_page(twindow& window, unsigned int page);

	/** Callback for page selection changes. */
	void on_page_select(twindow& window);

	void button_test_callback();
};

} // namespace gui2

#endif /* ! GUI_DIALOGS_PREFERENCES_DIALOG_HPP_INCLUDED */
