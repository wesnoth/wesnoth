/* $Id$ */
/*
   copyright (C) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#include "gui/dialogs/language_selection.hpp"

#include "foreach.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/window.hpp"
#include "language.hpp"
#include "preferences.hpp"

namespace gui2 {

/*WIKI
 * @page = GUIWindowWML
 * @order = 2_language_selection
 *
 * == Language selection ==
 *
 * This shows the dialog to select the language to use.
 * 
 * @start_table = container
 *     language_list (listbox)         This text contains the list with 
 *                                     available languages.
 * @end_table
 */
twindow* tlanguage_selection::build_window(CVideo& video)
{
	return build(video, get_id(LANGUAGE_SELECTION));
}

void tlanguage_selection::pre_show(CVideo& /*video*/, twindow& window)
{
	tlistbox* list = dynamic_cast<tlistbox*>(window.find_widget("language_list", false));
	VALIDATE(list, missing_widget("language_list"));

	const std::vector<language_def>& languages = get_languages();
	const language_def& current_language = get_language();
	foreach(const language_def& lang, languages) {
		string_map item;
		item.insert(std::make_pair("label", lang.language));
		item.insert(std::make_pair("tooltip", lang.language));

		list->add_row(item);
		if(lang == current_language) {
			list->select_row(list->get_item_count() - 1);
		}

		list->set_row_active(list->get_item_count() - 1, lang.available());
	}
}

void tlanguage_selection::post_show(twindow& window)
{
	if(get_retval() == twindow::OK) {
		tlistbox* list = dynamic_cast<tlistbox*>(window.find_widget("language_list", false));
		assert(list);

		const int res = list->get_selected_row();
		assert(res != -1);

		const std::vector<language_def>& languages = get_languages();
		::set_language(languages[res]);
		preferences::set_language(languages[res].localename);
	}
}

} // namespace gui2
