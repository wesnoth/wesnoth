/*
   Copyright (C) 2009 - 2014 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/auxiliary/find_widget.tpp"
#include "gui/dialogs/helper.hpp"
#include "gui/widgets/image.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/multi_page.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"
#include "utils/foreach.tpp"
#include "serialization/string_utils.hpp"

#include <boost/bind.hpp>

namespace gui2
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
 * campaign_list & & listbox & m &
 *         A listbox that contains all available campaigns. $
 *
 * -icon & & image & o &
 *         The icon for the campaign. $
 *
 * -name & & control & o &
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
 * -description & & control & o &
 *         The description of the campaign. $
 *
 * @end{table}
 */

REGISTER_DIALOG(campaign_selection)

void tcampaign_selection::campaign_selected(twindow& window)
{
	if(new_widgets && false) {
		ttree_view& tree
				= find_widget<ttree_view>(&window, "campaign_tree", false);

		if(tree.empty()) {
			return;
		}

		assert(tree.selected_item());
		const unsigned choice
				= lexical_cast<unsigned>(tree.selected_item()->id());

		tmulti_page& multi_page
				= find_widget<tmulti_page>(&window, "campaign_details", false);
		multi_page.select_page(choice);
	} else {
		const int selected_row
				= find_widget<tlistbox>(&window, "campaign_list", false)
						  .get_selected_row();

		tmulti_page& multi_page
				= find_widget<tmulti_page>(&window, "campaign_details", false);

		multi_page.select_page(selected_row);
	}
}

void tcampaign_selection::pre_show(CVideo& /*video*/, twindow& window)
{
	if(new_widgets && false) {
		/***** Setup campaign tree. *****/
		ttree_view& tree
				= find_widget<ttree_view>(&window, "campaign_tree", false);

		tree.set_selection_change_callback(
				boost::bind(&tcampaign_selection::campaign_selected,
							this,
							boost::ref(window)));

		window.keyboard_capture(&tree);

		string_map tree_group_field;
		std::map<std::string, string_map> tree_group_item;

		tree_group_field["label"] = "Campaigns won";
		tree_group_item["tree_view_node_label"] = tree_group_field;
		ttree_view_node& completed
				= tree.add_node("campaign_group", tree_group_item);

		tree_group_field["label"] = "Campaigns to conquer";
		tree_group_item["tree_view_node_label"] = tree_group_field;
		ttree_view_node& not_completed
				= tree.add_node("campaign_group", tree_group_item);

		/***** Setup campaign details. *****/
		tmulti_page& multi_page
				= find_widget<tmulti_page>(&window, "campaign_details", false);

		unsigned id = 0;
		FOREACH(const AUTO & campaign, campaigns_)
		{

			/*** Add tree item ***/
			tree_group_field["label"] = campaign["icon"];
			tree_group_item["icon"] = tree_group_field;

			tree_group_field["label"] = campaign["name"];
			tree_group_item["name"] = tree_group_field;

			if(campaign["completed"].to_bool()) {
				completed.add_child("campaign", tree_group_item)
						.set_id(lexical_cast<std::string>(id++));
			} else {
				not_completed.add_child("campaign", tree_group_item)
						.set_id(lexical_cast<std::string>(id++));
			}

			/*** Add detail item ***/
			string_map detail_item;
			std::map<std::string, string_map> detail_page;

			detail_item["label"] = campaign["description"];
			detail_item["use_markup"] = "true";
			detail_page.insert(std::make_pair("description", detail_item));

			detail_item["label"] = campaign["image"];
			detail_page.insert(std::make_pair("image", detail_item));

			multi_page.add_page(detail_page);
		}

		if(completed.empty()) {
			tree.remove_node(&completed);
		}

		if(not_completed.empty()) {
			tree.remove_node(&not_completed);
		}

	} else {
		/***** Hide the tree view. *****/
		if(ttree_view* tree
		   = find_widget<ttree_view>(&window, "campaign_tree", false, false)) {

			tree->set_visible(twidget::tvisible::invisible);
		}

		/***** Setup campaign list. *****/
		tlistbox& list = find_widget<tlistbox>(&window, "campaign_list", false);
#ifdef GUI2_EXPERIMENTAL_LISTBOX
		connect_signal_notify_modified(
				list,
				boost::bind(&tcampaign_selection::campaign_selected,
							this,
							boost::ref(window)));
#else
		list.set_callback_value_change(
				dialog_callback<tcampaign_selection,
								&tcampaign_selection::campaign_selected>);
#endif
		window.keyboard_capture(&list);

		/***** Setup campaign details. *****/
		tmulti_page& multi_page
				= find_widget<tmulti_page>(&window, "campaign_details", false);

		FOREACH(const AUTO & campaign, campaigns_)
		{

			/*** Add list item ***/
			string_map list_item;
			std::map<std::string, string_map> list_item_item;

			list_item["label"] = campaign["icon"];
			list_item_item.insert(std::make_pair("icon", list_item));

			list_item["label"] = campaign["name"];
			list_item_item.insert(std::make_pair("name", list_item));

			list.add_row(list_item_item);

			tgrid* grid = list.get_row_grid(list.get_item_count() - 1);
			assert(grid);

			twidget* widget = grid->find("victory", false);
			if(widget && !campaign["completed"].to_bool()) {
				widget->set_visible(twidget::tvisible::hidden);
			}

			/*** Add detail item ***/
			string_map detail_item;
			std::map<std::string, string_map> detail_page;

			detail_item["label"] = campaign["description"];
			detail_item["use_markup"] = "true";
			detail_page.insert(std::make_pair("description", detail_item));

			detail_item["label"] = campaign["image"];
			detail_page.insert(std::make_pair("image", detail_item));

			multi_page.add_page(detail_page);
		}
	}
	campaign_selected(window);
}

void tcampaign_selection::post_show(twindow& window)
{
	if(new_widgets && false) {
		ttree_view& tree
				= find_widget<ttree_view>(&window, "campaign_tree", false);

		if(tree.empty()) {
			return;
		}

		assert(tree.selected_item());
		choice_ = lexical_cast<unsigned>(tree.selected_item()->id());

	} else {
		choice_ = find_widget<tlistbox>(&window, "campaign_list", false)
						  .get_selected_row();
		deterministic_ = find_widget<ttoggle_button>(&window,"checkbox_deterministic", false).get_value();
	}
}

} // namespace gui2
