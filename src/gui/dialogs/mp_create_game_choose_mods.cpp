/* $Id: mp_game_settings.cpp 55412 2012-09-25 17:42:18Z lipk $ */
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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/mp_create_game_choose_mods.hpp"

#include "game_preferences.hpp"
#include "gui/dialogs/field.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "../../settings.hpp"
#include "gettext.hpp"

namespace gui2 {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_mp_create_game_choose_mods
 *
 * == Create Game: Choose Modifications ==
 *
 * The dialog for selecting modifications.
 * 
 * @begin{table}{dialog_widgets}
 * 
 * mod_list & & listbox & m &
 * 		displays the list of the available modifications $
 *
 * -checkbox & & toggle_button & o &
 * 		enable/disable a modification $
 *
 * -name & & label & o &
 * 		displays the modification's name $
 *
 * -description & & label & o &
 * 		displays the modification's description $
 * 
 * ok & & button & m &
 * 		closes the dialog, applies changes $
 *
 * cancel & & button & m &
 * 		closes the dialog, discards changes $
 * 
 * @end{table}
 */

REGISTER_DIALOG(mp_create_game_choose_mods)

tmp_create_game_choose_mods::tmp_create_game_choose_mods
					(const config& game_cfg,
					 std::vector<std::string>& result)
	: result_(result)
	, game_cfg_(game_cfg)
{
}

void tmp_create_game_choose_mods::pre_show(CVideo &/*video*/, twindow &window)
{
	mod_list_ = find_widget<tlistbox>(&window, "mod_list", false, true);
	std::vector<string_map>::iterator mod_itor;

	BOOST_FOREACH (const config& mod, game_cfg_.child_range("modification")) {

		string_map column;
		std::map<std::string, string_map> item;
		column["label"] = mod["name"];
		item.insert(std::make_pair("name", column));
		column["label"] = mod["description"];
		item.insert(std::make_pair("description", column));

		mod_list_->add_row(item);

		tgrid *grid = mod_list_->get_row_grid(mod_list_->get_item_count()-1);
		ttoggle_button &checkbox =
					find_widget<ttoggle_button>(grid, "checkbox", false);

		checkbox.set_value(std::find(	result_.begin(), result_.end(),
								mod["id"].str()) != result_.end());
	}

}

void tmp_create_game_choose_mods::post_show(twindow& /*window*/)
{
	if (get_retval() == twindow::OK) {
		result_.clear();

		for(unsigned int i = 0; i < mod_list_->get_item_count(); i++) {
			const tgrid *grid = mod_list_->get_row_grid(i);
			const ttoggle_button &checkbox =
						find_widget<const ttoggle_button>(grid, "checkbox", false);

			if (checkbox.get_value()) {
				result_.push_back(game_cfg_.child("modification", i)["id"]);
			}
		}
	}
}

} // namespace gui2

