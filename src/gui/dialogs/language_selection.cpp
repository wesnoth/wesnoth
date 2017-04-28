/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/language_selection.hpp"

#include "gui/auxiliary/find_widget.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "language.hpp"
#include "preferences.hpp"

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_language_selection
 *
 * == Language selection ==
 *
 * This shows the dialog to select the language to use. When the dialog is
 * closed with the OK button it also updates the selected language in the
 * preferences.
 *
 * @begin{table}{dialog_widgets}
 *
 * language_list & & listbox & m &
 *         This listbox contains the list with available languages. $
 *
 * - & & styled_widget & o &
 *         Show the name of the language in the current row. $
 *
 * @end{table}
 */

/**
 * @todo show we also reset the translations and is the tips of day call
 * really needed?
 */

REGISTER_DIALOG(language_selection)

void language_selection::pre_show(window& window)
{
	listbox& list = find_widget<listbox>(&window, "language_list", false);
	window.keyboard_capture(&list);

	const std::vector<language_def>& languages = get_languages();
	const language_def& current_language = get_language();
	for(const auto & lang : languages)
	{
		std::map<std::string, string_map> data;

		data["language"]["label"] = lang.language;

		list.add_row(data);
		if(lang == current_language) {
			list.select_row(list.get_item_count() - 1);
		}
	}
}

void language_selection::post_show(window& window)
{
	if(get_retval() == window::OK) {
		const int res = find_widget<listbox>(&window, "language_list", false)
								.get_selected_row();

		assert(res != -1);

		const std::vector<language_def>& languages = get_languages();
		::set_language(languages[res]);
		preferences::set_language(languages[res].localename);
	}
}

} // namespace dialogs
} // namespace gui2
