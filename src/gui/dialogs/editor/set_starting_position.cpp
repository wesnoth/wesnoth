/*
   Copyright (C) 2011 - 2018 by Iris Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "gui/dialogs/editor/set_starting_position.hpp"

#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/find_widget.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "map/location.hpp"

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_editor_set_starting_position
 *
 * == Editor set starting position ==
 *
 * Map editor dialog for setting player starting positions.
 *
 * @begin{table}{dialog_widgets}
 *
 * listbox & & listbox & m &
 *         Listbox displaying player choices. $
 *
 * -player & & styled_widget & m &
 *         Widget which shows a player item label. $
 *
 * -location & & styled_widget & m &
 *         Widget which shows the coordinates to the current
 *         starting position for a player if it exists. $
 *
 * ok & & button & m &
 *         OK button. $
 *
 * cancel & & button & m &
 *         Cancel button. $
 *
 * @end{table}
 */

REGISTER_DIALOG(editor_set_starting_position)

editor_set_starting_position::editor_set_starting_position(
		unsigned current_player,
		unsigned maximum_players,
		const std::vector<map_location>& starting_positions)
	: selection_(std::min(current_player, maximum_players))
	, starting_positions_(starting_positions)
{
	if(starting_positions_.size() != maximum_players) {
		starting_positions_.resize(maximum_players);
	}
}

void editor_set_starting_position::pre_show(window& window)
{
	listbox& list = find_widget<listbox>(&window, "listbox", false);
	window.keyboard_capture(&list);

	std::map<std::string, string_map> data;
	string_map column;

	column["label"] = _("player^None");
	data.emplace("player", column);
	list.add_row(data);

	for(unsigned i = 0; i < starting_positions_.size(); ++i) {
		const map_location& player_pos = starting_positions_[i];

		data.clear();

		utils::string_map symbols;
		symbols["player_number"] = std::to_string(i + 1);

		column["label"] = utils::interpolate_variables_into_string(
				_("Player $player_number"), &symbols);
		data.emplace("player", column);

		if(player_pos.valid()) {
			column["label"] = (formatter() << "(" << player_pos.wml_x() << ", "
										   << player_pos.wml_y() << ")").str();
			data.emplace("location", column);
		}

		list.add_row(data);
	}

	list.select_row(selection_);
}

void editor_set_starting_position::post_show(window& window)
{
	if(get_retval() != retval::OK) {
		return;
	}

	listbox& list = find_widget<listbox>(&window, "listbox", false);
	selection_ = list.get_selected_row();
}
} // namespace dialogs
} // namespace gui2
