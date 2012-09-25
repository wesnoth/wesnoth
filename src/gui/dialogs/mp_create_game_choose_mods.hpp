/*
   Copyright (C) 2012 by Boldizsár Lipka <lipka.boldizsar@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_DIALOGS_MP_CREATE_GAME_CHOOSE_MODS_HPP_INCLUDED
#define GUI_DIALOGS_MP_CREATE_GAME_CHOOSE_MODS_HPP_INCLUDED

#include "gui/dialogs/dialog.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/button.hpp"
#include <utility>
#include <vector>
#include <string>

class config;

namespace gui2 {

class tmp_create_game_choose_mods : public tdialog
{
public:

	/**
	 * Constructor.
	 * 
	 * @param game_cfg 		the config which contains the information for the
	 * 						modifications
	 * @param result [in]	the list of the currently activated modifications
	 * @param result [out]	the list of all activated modifications
	 */
	tmp_create_game_choose_mods(const config& game_cfg,
								std::vector<std::string>& result);

private:

	/** Inherited from tdialog, implemented by REGISTER_DIALOG. */
	virtual const std::string& window_id() const;

	/** Inherited from tdialog */
	void pre_show(CVideo &video, twindow &window);

	/** Inherited from tdialog */
	void post_show(twindow &window);

	/** a reference to the variable the result should be written into */
	std::vector<std::string>& result_;

	/** a reference to the config which contains the info about modifications*/
	const config& game_cfg_;

	/** a pointer to the listbox which displays the items */
	gui2::tlistbox *mod_list_;
};

} // namespace gui2

#endif 
