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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/gamestate_inspector.hpp"

#include "gui/dialogs/helper.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/window.hpp"

#include "../../foreach.hpp"
#include "../../gamestatus.hpp"
#include "../../resources.hpp"
#include "../../team.hpp"
#include "../../ai/manager.hpp"

#include <boost/bind.hpp>

namespace gui2 {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_gamestate_inspector
 *
 * == Gamestate inspector ==
 *
 * This shows the gamestate inspector
 *
 * @start_table = grid
 *     (inspector_name) (control) ()   Name of the inspector
 *     (stuff_list) (control) ()       List of various stuff that can be viewed
 *     (inspect) (control) ()          The state of the variable or event
 * @end_table
 */

tgamestate_inspector::tgamestate_inspector(const vconfig &cfg)
	: cfg_(cfg)
{
}


twindow* tgamestate_inspector::build_window(CVideo& video)
{
	return build(video, get_id(GAMESTATE_INSPECTOR));
}


static void add_row_to_stuff_list(tlistbox& list, const std::string &id, const std::string &label)
{
	std::map<std::string, string_map> data;
	string_map item;
	item["id"] = id;
	item["label"] = label;
	data.insert(std::make_pair("name", item));
	list.add_row(data);
}


static void inspect_var(twindow& window, const std::string &s)
{
	std::string s_ = s;
	if (s_.length()>20000) {//workaround for known bug
		s_.resize(20000);
	}
	tcontrol *i = find_widget<tcontrol>(&window, "inspect", false,true);
	i->set_label(s_);
	window.invalidate_layout();//workaround for assertion failure
}


static void inspect_vars(twindow& window)
{
	tlistbox& list = find_widget<tlistbox>(&window, "stuff_list", false);

	int selected = list.get_selected_row();
	if (selected==-1) {
		inspect_var(window,"");
		return;
	}

	int i = 0;//@todo: replace with precached data
	const config &vars = resources::state_of_game->get_variables();

	foreach( const config::attribute &a, vars.attribute_range()) {
		if (selected==i) {
			inspect_var(window, a.second);
			return;
		}
		i++;
	}

	foreach( const config::any_child &c, vars.all_children_range()) {
		if (selected==i) {
			inspect_var(window, c.cfg.debug());
			return;
		}
		i++;
	}
}


/*
static void inspect_ai(twindow& window, int side)
{
	const config &ai_cfg = ai::manager::to_config(side);
	find_widget<tcontrol>(&window, "inspect", false).set_label(ai_cfg.debug());
}
*/

void tgamestate_inspector::pre_show(CVideo& /*video*/, twindow& window)
{

	find_widget<tcontrol>(
			&window, "inspector_name", false).set_label(cfg_["name"]);

	tlistbox& list = find_widget<tlistbox>(&window, "stuff_list", false);

	list.set_callback_value_change(
		dialog_callback<tgamestate_inspector, &tgamestate_inspector::stuff_list_item_clicked>);

	const config &vars = resources::state_of_game->get_variables();
	foreach( const config::attribute &a, vars.attribute_range()) {
		add_row_to_stuff_list(list,a.first,a.first);
	}

	foreach( const config::any_child &c, vars.all_children_range()) {
		add_row_to_stuff_list(list,"["+c.key+"]","["+c.key+"]");
	}
	inspect_vars(window);

	//add_row_to_stuff_list(list,"vars","vars");
	//foreach team
	//int sides = static_cast<int>((*resources::teams).size());
	//for( int side = 1; side<=sides; ++side) {
	//	std::string side_str = str_cast(side);
	//	add_row_to_stuff_list(list,"AI"+side_str,"AI"+side_str);
	//	side++;
	//}

}


void tgamestate_inspector::stuff_list_item_clicked(twindow &window)
{
	inspect_vars(window);
}


} //end of namespace gui2
