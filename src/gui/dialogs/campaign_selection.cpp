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

#include "preferences/game.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/helper.hpp"
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
#include "utils/irdya_datetime.hpp"
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
		auto iter = std::find(page_ids_.begin(), page_ids_.end(), tree.selected_item()->id());
		const int choice = iter - page_ids_.begin();
		if(iter == page_ids_.end()) {
			return;
		}

		multi_page& pages = find_widget<multi_page>(&window, "campaign_details", false);
		pages.select_page(choice);
		engine_.set_current_level(choice);
	}

}

void campaign_selection::sort_campaigns(window& window, campaign_selection::CAMPAIGN_ORDER order, bool ascending)
{
	using level_ptr = ng::create_engine::level_ptr;
	auto levels = engine_.get_levels_by_type_unfiltered(ng::level::TYPE::SP_CAMPAIGN);
	switch(order) {
		case RANK: // Already sorted by rank
			if(!ascending) {
				// This'll actually never happen, but who knows if that'll ever change...
				std::reverse(levels.begin(), levels.end());
			}
			break;
		case DATE:
			std::sort(levels.begin(), levels.end(), [ascending](const level_ptr& a, const level_ptr& b) {
				auto cpn_a = std::dynamic_pointer_cast<ng::campaign>(a), cpn_b = std::dynamic_pointer_cast<ng::campaign>(b);
				if(cpn_b == nullptr) return cpn_a != nullptr;
				if(cpn_a == nullptr) return false;
				return ascending ? cpn_a->dates().first < cpn_b->dates().first : cpn_a->dates().first > cpn_b->dates().first;
			});
			break;
		case NAME:
			std::sort(levels.begin(), levels.end(), [ascending](const level_ptr& a, const level_ptr& b) {
				int cmp = translation::icompare(a->name(), b->name());
				return ascending ? cmp < 0 : cmp > 0;
			});
			break;
	}

	tree_view& tree = find_widget<tree_view>(&window, "campaign_tree", false);
	// Remember which campaign was selected...
	std::string was_selected = tree.selected_item()->id();
	tree.clear();
	for(const auto& level : levels) {
		add_campaign_to_tree(window, level->data());
	}

	if(!was_selected.empty()) {
		tree_view_node& node = find_widget<tree_view_node>(&window, was_selected, false);
		node.select_node();
	}
}

void campaign_selection::toggle_sorting_selection(window& window, CAMPAIGN_ORDER order) {
	static bool force = false;
	if(force) {
		return;
	}
	if(current_sorting_ == order) {
		if(currently_sorted_asc_) {
			currently_sorted_asc_ = false;
		} else {
			currently_sorted_asc_ = true;
			current_sorting_ = RANK;
		}
	} else if(current_sorting_ == RANK) {
		currently_sorted_asc_ = true;
		current_sorting_ = order;
	} else {
		currently_sorted_asc_ = true;
		current_sorting_ = order;
		force = true;
		if(order == NAME) {
			toggle_button& sort_time = find_widget<toggle_button>(&window, "sort_time", false);
			sort_time.set_value(0);
		} else if(order == DATE) {
			toggle_button& sort_name = find_widget<toggle_button>(&window, "sort_name", false);
			sort_name.set_value(0);
		}
		force = false;
	}
	sort_campaigns(window, current_sorting_, currently_sorted_asc_);
}

void campaign_selection::pre_show(window& window)
{
	/***** Setup campaign tree. *****/
	tree_view& tree = find_widget<tree_view>(&window, "campaign_tree", false);

	tree.set_selection_change_callback(
		std::bind(&campaign_selection::campaign_selected, this, std::ref(window)));

	toggle_button& sort_name = find_widget<toggle_button>(&window, "sort_name", false);
	toggle_button& sort_time = find_widget<toggle_button>(&window, "sort_time", false);

	connect_signal_notify_modified(sort_name,
		std::bind(&campaign_selection::toggle_sorting_selection, this, std::ref(window), NAME));

	connect_signal_notify_modified(sort_time,
		std::bind(&campaign_selection::toggle_sorting_selection, this, std::ref(window), DATE));

	window.keyboard_capture(&tree);

	/***** Setup campaign details. *****/
	multi_page& pages = find_widget<multi_page>(&window, "campaign_details", false);

	for(const auto & level : engine_.get_levels_by_type_unfiltered(ng::level::TYPE::SP_CAMPAIGN)) {
		const config& campaign = level->data();

		/*** Add tree item ***/
		add_campaign_to_tree(window, campaign);

		/*** Add detail item ***/
		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = campaign["description"];
		item["use_markup"] = "true";

		if(!campaign["description_alignment"].empty()) {
			item["text_alignment"] = campaign["description_alignment"];
		}

		data.emplace("description", item);

		item["label"] = campaign["image"];
		data.emplace("image", item);

		pages.add_page(data);
		page_ids_.push_back(campaign["id"]);
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

			mod_menu_values.emplace_back(config {"label", mod->name, "checkbox", active});

			mod_states_.push_back(active);
		}

		mods_menu.set_values(mod_menu_values);
		mods_menu.select_options(mod_states_);
		mods_menu.set_callback_toggle_state_change(std::bind(&campaign_selection::mod_toggled, this, std::ref(window)));
	} else {
		mods_menu.set_active(false);
		mods_menu.set_label(_("None"));
	}

	campaign_selected(window);
}

void campaign_selection::add_campaign_to_tree(window& window, const config& campaign)
{
	tree_view& tree = find_widget<tree_view>(&window, "campaign_tree", false);
	std::map<std::string, string_map> data;
	string_map item;

	item["label"] = campaign["icon"];
	data.emplace("icon", item);

	item["label"] = campaign["name"];
	data.emplace("name", item);

	if(campaign["completed_on_hardest"].to_bool()) {
		item["label"] = game_config::images::victory_laurel_hardest;
	} else if(campaign["completed"].to_bool()) {
		if(campaign["completed_difficulties"] == campaign.child_or_empty("difficulty")["define"]) {
			item["label"] = game_config::images::victory_laurel_easy;
		} else {
			item["label"] = game_config::images::victory_laurel;
		}
	} else {
		item["label"] = "misc/blank-hex.png";
	}
	data.emplace("victory", item);

	tree.add_node("campaign", data).set_id(campaign["id"]);
}

void campaign_selection::post_show(window& window)
{
	tree_view& tree = find_widget<tree_view>(&window, "campaign_tree", false);

	if(tree.empty()) {
		return;
	}

	assert(tree.selected_item());
	if(tree.selected_item()->id() != "") {
		auto iter = std::find(page_ids_.begin(), page_ids_.end(), tree.selected_item()->id());
		if(iter != page_ids_.end()) {
			choice_ = iter - page_ids_.begin();
		}
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
