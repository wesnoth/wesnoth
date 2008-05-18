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
#include "gui/widgets/button.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/window_builder.hpp"
#include "language.hpp"
#include "log.hpp"
#include "preferences.hpp"
#include "video.hpp"
#include "wml_exception.hpp"

#define DBG_GUI LOG_STREAM_INDENT(debug, widget)
#define LOG_GUI LOG_STREAM_INDENT(info, widget)
#define WRN_GUI LOG_STREAM_INDENT(warn, widget)
#define ERR_GUI LOG_STREAM_INDENT(err, widget)

namespace gui2 {

void tlanguage_selection::show(CVideo& video)
{
	gui2::init();

	twindow window = build(video, get_id(LANGUAGE_SELECTION));

	tlistbox* list = dynamic_cast<tlistbox*>(window.find_widget("language_list", false));
	VALIDATE(list, "No list defined.");

	const std::vector<language_def>& languages = get_languages();
	const language_def& current_language = get_language();
	foreach(const language_def& lang, languages) {

		list->add_item(lang.language);
		if(lang == current_language) {
			std::cerr << "select row " << list->get_item_count() - 1 << ".\n";
			list->select_row(list->get_item_count() - 1);
		}

		list->set_row_active(list->get_item_count() - 1, lang.available());
	}

	window.recalculate_size();

	retval_ = window.show(true);

	if(retval_ == tbutton::OK) {
		const unsigned res = list->get_selected_row();

		::set_language(languages[res]);
		preferences::set_language(languages[res].localename);
	}
}


} // namespace gui2
