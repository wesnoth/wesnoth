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

#include "gui/dialogs/core_selection.hpp"

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
#include "gui/widgets/window.hpp"
#include "utils/foreach.tpp"
#include "serialization/string_utils.hpp"

#include <boost/bind.hpp>

namespace gui2
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_core_selection
 *
 * == Core selection ==
 *
 * This shows the dialog which allows the user to choose which core to
 * play.
 *
 * @begin{table}{dialog_widgets}
 *
 * core_list & & listbox & m &
 *         A listbox that contains all available cores. $
 *
 * -icon & & image & o &
 *         The icon for the core. $
 *
 * -name & & control & o &
 *         The name of the core. $
 *
 * core_details & & multi_page & m &
 *         A multi page widget that shows more details for the selected
 *         core. $
 *
 * -image & & image & o &
 *         The image for the core. $
 *
 * -description & & control & o &
 *         The description of the core. $
 *
 * @end{table}
 */

REGISTER_DIALOG(core_selection)

void tcore_selection::core_selected(twindow& window)
{
	if(new_widgets && false) {
		ttree_view& tree
				= find_widget<ttree_view>(&window, "core_tree", false);

		if(tree.empty()) {
			return;
		}

		assert(tree.selected_item());
		const unsigned choice
				= lexical_cast<unsigned>(tree.selected_item()->id());

		tmulti_page& multi_page
				= find_widget<tmulti_page>(&window, "core_details", false);
		multi_page.select_page(choice);
	} else {
		const int selected_row
				= find_widget<tlistbox>(&window, "core_list", false)
						  .get_selected_row();

		tmulti_page& multi_page
				= find_widget<tmulti_page>(&window, "core_details", false);

		multi_page.select_page(selected_row);
	}
}

void tcore_selection::pre_show(CVideo& /*video*/, twindow& window)
{
	if(new_widgets && false) {
		/***** Setup core tree. *****/
		ttree_view& tree
				= find_widget<ttree_view>(&window, "core_tree", false);

		tree.set_selection_change_callback(
				boost::bind(&tcore_selection::core_selected,
							this,
							boost::ref(window)));

		window.keyboard_capture(&tree);

		string_map tree_group_field;
		std::map<std::string, string_map> tree_group_item;

		tree_group_item["tree_view_node_label"] = tree_group_field;

		/***** Setup core details. *****/
		tmulti_page& multi_page
				= find_widget<tmulti_page>(&window, "core_details", false);

	//	unsigned id = 0;
		FOREACH(const AUTO & core, cores_)
		{

			/*** Add tree item ***/
			tree_group_field["label"] = core["icon"];
			tree_group_item["icon"] = tree_group_field;

			tree_group_field["label"] = core["name"];
			tree_group_item["name"] = tree_group_field;

			/*** Add detail item ***/
			string_map detail_item;
			std::map<std::string, string_map> detail_page;

			detail_item["label"] = core["description"];
			detail_item["use_markup"] = "true";
			detail_page.insert(std::make_pair("description", detail_item));

			detail_item["label"] = core["image"];
			detail_page.insert(std::make_pair("image", detail_item));

			multi_page.add_page(detail_page);
		}

	} else {
		/***** Hide the tree view. *****/
		if(ttree_view* tree
		   = find_widget<ttree_view>(&window, "core_tree", false, false)) {

			tree->set_visible(twidget::tvisible::invisible);
		}

		/***** Setup core list. *****/
		tlistbox& list = find_widget<tlistbox>(&window, "core_list", false);
#ifdef GUI2_EXPERIMENTAL_LISTBOX
		connect_signal_notify_modified(
				list,
				boost::bind(&tcore_selection::core_selected,
							this,
							boost::ref(window)));
#else
		list.set_callback_value_change(
				dialog_callback<tcore_selection,
								&tcore_selection::core_selected>);
#endif
		window.keyboard_capture(&list);

		/***** Setup core details. *****/
		tmulti_page& multi_page
				= find_widget<tmulti_page>(&window, "core_details", false);

		FOREACH(const AUTO & core, cores_)
		{

			/*** Add list item ***/
			string_map list_item;
			std::map<std::string, string_map> list_item_item;

			list_item["label"] = core["icon"];
			list_item_item.insert(std::make_pair("icon", list_item));

			list_item["label"] = core["name"];
			list_item_item.insert(std::make_pair("name", list_item));

			list.add_row(list_item_item);

			tgrid* grid = list.get_row_grid(list.get_item_count() - 1);
			assert(grid);

			/*** Add detail item ***/
			string_map detail_item;
			std::map<std::string, string_map> detail_page;

			detail_item["label"] = core["description"];
			detail_item["use_markup"] = "true";
			detail_page.insert(std::make_pair("description", detail_item));

			detail_item["label"] = core["image"];
			detail_page.insert(std::make_pair("image", detail_item));

			multi_page.add_page(detail_page);
		}
		list.select_row(choice_, true);
	}
	core_selected(window);
}

void tcore_selection::post_show(twindow& window)
{
	if(new_widgets && false) {
		ttree_view& tree
				= find_widget<ttree_view>(&window, "core_tree", false);

		if(tree.empty()) {
			return;
		}

		assert(tree.selected_item());
		choice_ = lexical_cast<unsigned>(tree.selected_item()->id());

	} else {
		choice_ = find_widget<tlistbox>(&window, "core_list", false)
						  .get_selected_row();
	}
}

} // namespace gui2
