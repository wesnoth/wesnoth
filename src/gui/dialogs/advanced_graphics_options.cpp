/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
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

#include "gui/dialogs/advanced_graphics_options.hpp"

#include "desktop/notifications.hpp"
#include "gui/auxiliary/find_widget.tpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/window.hpp"

#include "image.hpp"
#include "preferences.hpp"
#include "formula_string_utils.hpp"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>

#include "gettext.hpp"

namespace gui2
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_mp_alerts_options
 *
 * == Lobby sounds options ==
 *
 * A Preferences subdialog permitting to configure the sounds and notifications
 * generated in response to various mp lobby / game events.
 *
 * @begin{table}{dialog_widgets}
 *
 * _label & & label & m &
 *        Item name. $
 *
 * _sound & & toggle_button & m &
 *        Toggles whether to play the item sound. $
 *
 * _notif & & toggle_button & m &
 *        Toggles whether to give a notification. $
 *
 * _lobby & & toggle_button & m &
 *        Toggles whether to take actions for this item when in the lobby. $
 *
 * @end{table}
 */

REGISTER_DIALOG(advanced_graphics_options)

const std::vector<std::string> tadvanced_graphics_options::scale_cases = boost::assign::list_of("zoom")("hex");

void tadvanced_graphics_options::setup_scale_button(const std::string & case_id, SCALING_ALGORITHM button, twindow & window )
{
	std::string pref_id = "scale_" + case_id;

	tadvanced_graphics_options::SCALING_ALGORITHM algo = tadvanced_graphics_options::LEGACY_LINEAR;
	try {
		algo = string_to_SCALING_ALGORITHM(preferences::get(pref_id));
	} catch (bad_enum_cast &) {
		preferences::set(pref_id, SCALING_ALGORITHM_to_string(algo));
	}

	// algo is now synced with preference, and default value of linear if something went wrong

	ttoggle_button * b = &find_widget<ttoggle_button>(&window, pref_id + "_" + SCALING_ALGORITHM_to_string(button), false);
	b->set_value(algo == button);

	connect_signal_mouse_left_click(*b, boost::bind(&tadvanced_graphics_options::scale_button_callback, this, pref_id, button, boost::ref(window)));
}

void tadvanced_graphics_options::scale_button_callback(std::string pref_id, SCALING_ALGORITHM me, twindow & window)
{
	tadvanced_graphics_options::SCALING_ALGORITHM algo = tadvanced_graphics_options::LEGACY_LINEAR;
	try {
		algo = string_to_SCALING_ALGORITHM(preferences::get(pref_id));
	} catch (bad_enum_cast &) {
		preferences::set(pref_id, SCALING_ALGORITHM_to_string(algo));
	}

	if (algo != me) {
		image::flush_cache();
	}

	preferences::set(pref_id, SCALING_ALGORITHM_to_string(me));

	for (size_t x = 0; x < SCALING_ALGORITHM_COUNT; ++x) {
		ttoggle_button * b = &find_widget<ttoggle_button>(&window, pref_id + "_" + SCALING_ALGORITHM_to_string(static_cast<SCALING_ALGORITHM>(x)), false);
		b->set_value(x == me);
	}

	image::update_from_preferences();
}

void tadvanced_graphics_options::setup_scale_case(const std::string & i, twindow & window)
{
	for (size_t x = 0; x < SCALING_ALGORITHM_COUNT; ++x) {
		setup_scale_button(i, static_cast<SCALING_ALGORITHM>(x), window);
	}
}

tadvanced_graphics_options::tadvanced_graphics_options()
{
}

void tadvanced_graphics_options::pre_show(CVideo& /*video*/, twindow& window)
{
	BOOST_FOREACH(const std::string & i, scale_cases) {
		setup_scale_case(i, window);
	}

	/*
	tbutton * defaults;
	defaults = &find_widget<tbutton>(&window,"revert_to_defaults", false);
	connect_signal_mouse_left_click(*defaults, boost::bind(&revert_to_default_pref_values, boost::ref(window)));
	*/
}

void tadvanced_graphics_options::post_show(twindow& /*window*/)
{
}

} // end namespace gui2
