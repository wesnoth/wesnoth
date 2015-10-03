/* $Id$ */
/*
   Copyright (C) 2008 - 2011 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/language_selection.hpp"

#include "foreach.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "language.hpp"
#include "preferences.hpp"

namespace gui2 {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_language_selection
 *
 * == Language selection ==
 *
 * This shows the dialog to select the language to use.
 *
 * @start_table = grid
 *     (language_list) (listbox) ()    This text contains the list with
 *                                     available languages.
 *     -[] (control) ()                Gets the name of the language.
 * @end_table
 */
twindow* tlanguage_selection::build_window(CVideo& video)
{
	return build(video, get_id(LANGUAGE_SELECTION));
}

void tlanguage_selection::pre_show(CVideo& /*video*/, twindow& window)
{
	tlistbox* list = find_widget<tlistbox>(
			&window, "language_list", false, true);
	window.keyboard_capture(list);

	const std::vector<language_def>& languages = get_languages();
	const language_def& current_language = get_language();
	BOOST_FOREACH(const language_def& lang, languages) {
		string_map item;
		item.insert(std::make_pair("label", lang.language));

		list->add_row(item);
		if(lang == current_language) {
			list->select_row(list->get_item_count() - 1);
		}
	}
}

void tlanguage_selection::post_show(twindow& window)
{
	if(get_retval() == twindow::OK) {
		tlistbox* list = find_widget<tlistbox>(
				&window, "language_list", false, true);

		const int res = list->get_selected_row();
		assert(res != -1);

		const std::vector<language_def>& languages = get_languages();
		::set_language(languages[res]);
		preferences::set_language(languages[res].localename);
	}
}

} // namespace gui2
