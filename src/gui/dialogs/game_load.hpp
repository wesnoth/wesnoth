/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_LOAD_GAME_HPP_INCLUDED
#define GUI_DIALOGS_LOAD_GAME_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "gui/widgets/listbox.hpp"
#include "savegame.hpp"
#include "tstring.hpp"

namespace gui2 {

class tgame_load : public tdialog
{
public:
	tgame_load(const config& cache_config);

	const std::string& filename() const { return filename_; }

protected:
	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

private:
	/** Inherited from tdialog. */
	twindow* build_window(CVideo& video);

	void list_item_clicked(twindow& window);

	void display_savegame(twindow& window);
	void evaluate_summary_string(std::stringstream& str, const config& cfg_summary);

	//tfield_text* txtFilename_;
	std::string filename_;

	std::vector<save_info> games_;
	const config& cache_config_;
};

}

#endif

