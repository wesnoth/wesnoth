/*
   Copyright (C) 2009 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/campaign_selection.hpp"

#include "config_assign.hpp"
#include "game_preferences.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/dialogs/campaign_settings.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/multimenu_button.hpp"
#include "gui/widgets/multi_page.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"
#include "lexical_cast.hpp"
#include "serialization/string_utils.hpp"

#include "utils/functional.hpp"
#include "video.hpp"

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_campaign_selection
 *
 * == Campaign selection ==
 *
 * This shows the dialog which allows the user to choose which campaign to
 * play.
 *
 * @begin{table}{dialog_widgets}
 *
 * campaign_tree & & tree_view & m &
 *         A tree_view that contains all available campaigns. $
 *
 * -icon & & image & o &
 *         The icon for the campaign. $
 *
 * -name & & styled_widget & o &
 *         The name of the campaign. $
 *
 * -victory & & image & o &
 *         The icon to show when the user finished the campaign. The engine
 *         determines whether or not the user has finished the campaign and
 *         sets the visible flag for the widget accordingly. $
 *
 * campaign_details & & multi_page & m &
 *         A multi page widget that shows more details for the selected
 *         campaign. $
 *
 * -image & & image & o &
 *         The image for the campaign. $
 *
 * -description & & styled_widget & o &
 *         The description of the campaign. $
 *
 * @end{table}
 */

REGISTER_DIALOG(campaign_selection)

void campaign_selection::campaign_selected(window& window)
{
	tree_view& tree = find_widget<tree_view>(&window, "campaign_tree", false);

	if(tree.empty()) {
		return;
	}

	assert(tree.selected_item());
	if(tree.selected_item()->id() != "") {
		const unsigned choice = lexical_cast<unsigned>(tree.selected_item()->id());

		multi_page& pages = find_widget<multi_page>(&window, "campaign_details", false);
		pages.select_page(choice);
		engine_.set_current_level(choice);
	}

}

void campaign_selection::show_settings(CVideo& video) {
	campaign_settings settings_dlg(engine_);
	settings_dlg.show(video);
}

void campaign_selection::pre_show(window& window)
{
	/***** Setup campaign tree. *****/
	tree_view& tree = find_widget<tree_view>(&window, "campaign_tree", false);

	tree.set_selection_change_callback(
		std::bind(&campaign_selection::campaign_selected, this, std::ref(window)));

	window.keyboard_capture(&tree);

	/***** Setup campaign details. *****/
	multi_page& pages = find_widget<multi_page>(&window, "campaign_details", false);

	unsigned id = 0;
	for(const auto & level : engine_.get_levels_by_type_unfiltered(ng::level::TYPE::SP_CAMPAIGN)) {
		const config& campaign = level->data();

		/*** Add tree item ***/
		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = campaign["icon"];
		data.emplace("icon", item);

		item["label"] = campaign["name"];
		data.emplace("name", item);

		item["label"] = campaign["completed"].to_bool() ? "misc/laurel.png" : "misc/blank-hex.png";
		data.emplace("victory", item);

		tree.add_node("campaign", data).set_id(std::to_string(id++));

		/*** Add detail item ***/
		item.clear();
		data.clear();

		item["label"] = campaign["description"];
		item["use_markup"] = "true";

		if(!campaign["description_alignment"].empty()) {
			item["text_alignment"] = campaign["description_alignment"];
		}

		data.emplace("description", item);

		item["label"] = campaign["image"];
		data.emplace("image", item);

		pages.add_page(data);
	}

	//
	// Set up Mods selection dropdown
	//
	multimenu_button& mods_menu = find_widget<multimenu_button>(&window, "mods_menu", false);

	if(!engine_.get_const_extras_by_type(ng::create_engine::MOD).empty()) {

		std::vector<config> mod_menu_values;
		std::vector<std::string> enabled = engine_.active_mods();

		for(const auto& mod : engine_.get_const_extras_by_type(ng::create_engine::MOD)) {
			const bool active = std::find(enabled.begin(), enabled.end(), mod->id) != enabled.end();

			mod_menu_values.emplace_back(config_of("label", mod->name)("checkbox", active));

			mod_states_.push_back(active);
		}

		mods_menu.set_values(mod_menu_values);
		mods_menu.set_callback_toggle_state_change(std::bind(&campaign_selection::mod_toggled, this, std::ref(window)));
	} else {
		mods_menu.set_active(false);
		mods_menu.set_label(_("None"));
	}

	campaign_selected(window);

	/***** Setup advanced settings button *****/
	button* advanced_settings_button =
			find_widget<button>(&window, "advanced_settings", false, false);
	if(advanced_settings_button) {
		advanced_settings_button->connect_click_handler(
			std::bind(&campaign_selection::show_settings, this, std::ref(window.video())));
	}
}

void campaign_selection::post_show(window& window)
{
	tree_view& tree = find_widget<tree_view>(&window, "campaign_tree", false);

	if(tree.empty()) {
		return;
	}

	assert(tree.selected_item());
	if(tree.selected_item()->id() != "") {
		choice_ = lexical_cast<unsigned>(tree.selected_item()->id());
	}

	deterministic_ = find_widget<toggle_button>(&window, "checkbox_deterministic", false).get_value_bool();

	preferences::set_modifications(engine_.active_mods(), false);
}

void campaign_selection::mod_toggled(window& window)
{
	boost::dynamic_bitset<> new_mod_states = find_widget<multimenu_button>(&window, "mods_menu", false).get_toggle_states();

	// Get a mask of any mods that were toggled, regardless of new state
	mod_states_ = mod_states_ ^ new_mod_states;

	for(unsigned i = 0; i < mod_states_.size(); i++) {
		if(mod_states_[i]) {
			engine_.set_current_mod_index(i);
			engine_.toggle_current_mod();
		}
	}

	// Save the full toggle states for next time
	mod_states_ = new_mod_states;
}

} // namespace dialogs
} // namespace gui2
