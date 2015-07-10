/*
   Copyright (C) 2011, 2015 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/preferences_dialog.hpp"

#include "formatter.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/auxiliary/find_widget.tpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/grid.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
	#include "gui/widgets/list.hpp"
#else
	#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/stacked_widget.hpp"
#include "gui/widgets/window.hpp"

#include "gettext.hpp"

#include <boost/bind.hpp>

namespace {
	//const unsigned int num_pages = 5;
}

namespace gui2 {

REGISTER_DIALOG(preferences)

tpreferences::tpreferences()
{
}

void tpreferences::add_pager_row(tlistbox& selector, const std::string& icon, const std::string& label)
{
	std::map<std::string, string_map> data;
	data["icon"]["label"] = "icons/icon-" + icon;
	data["label"]["label"] = label;
	selector.add_row(data);
}

void tpreferences::pre_show(CVideo& /*video*/, twindow& window)
{
	tlistbox& selector = find_widget<tlistbox>(&window, "selector", false);
	tstacked_widget& pager = find_widget<tstacked_widget>(&window, "pager", false);

#ifdef GUI2_EXPERIMENTAL_LISTBOX
	connect_signal_notify_modified(selector, boost::bind(
			  &tpreferences::on_page_select
			, this
			, boost::ref(window)));
#else
	selector.set_callback_value_change(dialog_callback
			<tpreferences
			, &tpreferences::on_page_select>);
#endif
	window.keyboard_capture(&selector);

	add_pager_row(selector, "general.png", _("Prefs section^General"));
	add_pager_row(selector, "display.png", _("Prefs section^Display"));
	add_pager_row(selector, "music.png",  _("Prefs section^Sound"));
	add_pager_row(selector, "multiplayer.png", _("Prefs section^Multiplayer"));
	add_pager_row(selector, "advanced.png", _("Advanced section^Advanced"));

	//
	// We need to establish callbacks before selecting the initial page,
	// otherwise widgets from other pages cannot be found afterwards.
	//
#if 0
	tbutton& test_button = find_widget<tbutton>(&window, "button1", false);

	connect_signal_mouse_left_click(
				test_button,
				boost::bind(&tpreferences::button_test_callback,
							this));
#endif
	assert(selector.get_item_count() == pager.get_layer_count());

	selector.select_row(0);
	pager.select_layer(0);
}

void tpreferences::set_visible_page(twindow& window, unsigned int page)
{
	tstacked_widget& pager = find_widget<tstacked_widget>(&window, "pager", false);
	pager.select_layer(page);
}

void tpreferences::button_test_callback()
{
	std::cerr << "hi from button1\n";
}

void tpreferences::on_page_select(twindow& window)
{
	const int selected_row =
		std::max(0, find_widget<tlistbox>(&window, "selector", false).get_selected_row());
	set_visible_page(window, static_cast<unsigned int>(selected_row));
}

void tpreferences::post_show(twindow& /*window*/)
{
}

} // end namespace gui2
