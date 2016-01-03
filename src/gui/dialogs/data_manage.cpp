/*
   Copyright (C) 2010 - 2016 by Jody Northup
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

#include "gui/dialogs/data_manage.hpp"

#include "formula_string_utils.hpp"
#include "game_config.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/dialogs/field.hpp"
#include "gui/dialogs/game_delete.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/minimap.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"
#include "language.hpp"
#include "preferences_display.hpp"
#include "savegame.hpp"
#include "utils/foreach.tpp"

#include <cctype>
#include <boost/bind.hpp>

namespace gui2
{


REGISTER_DIALOG(data_manage)

tdata_manage::tdata_manage()
	: txtFilter_(register_text("txtFilter", true))
	, filename_()
	, games_()
	, last_words_()
{
}

void tdata_manage::pre_show(CVideo& /*video*/, twindow& window)
{
	assert(txtFilter_);

	ttext_box* filter
			= find_widget<ttext_box>(&window, "txtFilter", false, true);
	window.keyboard_capture(filter);
	filter->set_text_changed_callback(
			boost::bind(&tdata_manage::filter_text_changed, this, _1, _2));

	tlistbox& list = find_widget<tlistbox>(&window, "persist_list", false);
	window.keyboard_capture(&list);

#ifdef GUI2_EXPERIMENTAL_LISTBOX
	connect_signal_notify_modified(list,
								   boost::bind(&tdata_manage::list_item_clicked,
											   this,
											   boost::ref(window)));
#else
	list.set_callback_value_change(
			dialog_callback<tdata_manage, &tdata_manage::list_item_clicked>);
#endif

	{
		cursor::setter cur(cursor::WAIT);
		games_ = savegame::get_saves_list();
	}
	fill_game_list(window, games_);

	connect_signal_mouse_left_click(
			find_widget<tbutton>(&window, "clear", false),
			boost::bind(&tdata_manage::delete_button_callback,
						this,
						boost::ref(window)));
}

void tdata_manage::fill_game_list(twindow& window,
								  std::vector<savegame::save_info>& games)
{
	tlistbox& list = find_widget<tlistbox>(&window, "persist_list", false);
	list.clear();

	FOREACH(const AUTO & game, games)
	{
		std::map<std::string, string_map> data;
		string_map item;

		item["label"] = game.name();
		data.insert(std::make_pair("filename", item));

		item["label"] = game.format_time_summary();
		data.insert(std::make_pair("date", item));

		list.add_row(data);
	}
}

void tdata_manage::list_item_clicked(twindow& /*window*/)
{
}

bool tdata_manage::filter_text_changed(ttext_* textbox, const std::string& text)
{
	twindow& window = *textbox->get_window();

	tlistbox& list = find_widget<tlistbox>(&window, "persist_list", false);

	const std::vector<std::string> words = utils::split(text, ' ');

	if(words == last_words_)
		return false;
	last_words_ = words;

	std::vector<bool> show_items(list.get_item_count(), true);

	if(!text.empty()) {
		for(unsigned int i = 0; i < list.get_item_count(); i++) {
			tgrid* row = list.get_row_grid(i);

			tgrid::iterator it = row->begin();
			tlabel& filename_label
					= find_widget<tlabel>(*it, "filename", false);

			bool found = false;
			FOREACH(const AUTO & word, words)
			{
				found = std::search(filename_label.label().str().begin(),
									filename_label.label().str().end(),
									word.begin(),
									word.end(),
									chars_equal_insensitive)
						!= filename_label.label().str().end();

				if(!found) {
					// one word doesn't match, we don't reach words.end()
					break;
				}
			}

			show_items[i] = found;
		}
	}

	list.set_row_shown(show_items);

	return false;
}

void tdata_manage::delete_button_callback(twindow& window)
{
	tlistbox& list = find_widget<tlistbox>(&window, "persist_list", false);

	const size_t index = size_t(list.get_selected_row());
	if(index < games_.size()) {

		// See if we should ask the user for deletion confirmation
		if(preferences::ask_delete_saves()) {
			if(!gui2::tgame_delete::execute(window.video())) {
				return;
			}
		}

		// Delete the file
		savegame::delete_game(games_[index].name());

		// Remove it from the list of saves
		games_.erase(games_.begin() + index);
		list.remove_row(index);
	}
}

} // namespace gui2
