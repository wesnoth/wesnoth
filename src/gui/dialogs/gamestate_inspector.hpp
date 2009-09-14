/* $Id$ */
/*
   Copyright (C) 2009 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_GAMESTATE_INSPECTOR_HPP_INCLUDED
#define GUI_DIALOGS_GAMESTATE_INSPECTOR_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "../../variable.hpp"
namespace gui2 {

class tgamestate_inspector : public tdialog {
public:
	tgamestate_inspector(const vconfig &cfg);

	/** Inherited from tdialog. */
	twindow* build_window(CVideo& video);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);
private:
	vconfig cfg_;
	void stuff_list_item_clicked(twindow &window);
};

}

#endif /* ! GUI_DIALOGS_GAMESTATE_INSPECTOR_HPP_INCLUDED */
