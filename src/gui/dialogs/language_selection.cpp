/*
	Copyright (C) 2008 - 2024
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "language.hpp"
#include "preferences/general.hpp"

namespace gui2::dialogs
{

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
		widget_data data;

		data["language"]["label"] = lang.language;

		list.add_row(data);
		if(lang == current_language) {
			list.select_last_row();
		}
	}
}

void language_selection::post_show(window& window)
{
	if(get_retval() == retval::OK) {
		const int res = find_widget<listbox>(&window, "language_list", false)
								.get_selected_row();

		assert(res != -1);

		const std::vector<language_def>& languages = get_languages();
		::set_language(languages[res]);
		preferences::set_language(languages[res].localename);
	}
}

} // namespace dialogs
