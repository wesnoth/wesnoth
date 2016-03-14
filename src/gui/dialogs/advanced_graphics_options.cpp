/*
   Copyright (C) 2014 - 2016 by Chris Beck <render787@gmail.com>
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

REGISTER_DIALOG(advanced_graphics_options)

const std::vector<std::string> tadvanced_graphics_options::scale_cases = boost::assign::list_of("zoom")("hex");

tadvanced_graphics_options::SCALING_ALGORITHM tadvanced_graphics_options::get_scale_pref(const std::string& pref_id)
{
	SCALING_ALGORITHM algo = SCALING_ALGORITHM::LINEAR;
	try {
		algo = SCALING_ALGORITHM::string_to_enum(preferences::get(pref_id));
	} catch (bad_enum_cast &) {
		preferences::set(pref_id, algo.to_string());
	}
	// algo is now synced with preference, and default value of linear if something went wrong
	return algo;
}
	
void tadvanced_graphics_options::setup_scale_case(const std::string & case_id, twindow & window)
{
	std::string pref_id = "scale_" + case_id;
	tgroup<SCALING_ALGORITHM>& group = groups_[case_id];
	for (size_t x = 0; x < SCALING_ALGORITHM::count; ++x) {
		SCALING_ALGORITHM scale = SCALING_ALGORITHM::from_int(x);
		ttoggle_button* button = &find_widget<ttoggle_button>(&window, pref_id + "_" + scale.to_string(), false);
		group.add_member(button, scale);
	}
	group.set_member_states(get_scale_pref(pref_id));
}

void tadvanced_graphics_options::update_scale_case(const std::string & case_id)
{
	std::string pref_id = "scale_" + case_id;
	SCALING_ALGORITHM new_val = groups_[case_id].get_active_member_value();
	if(new_val != get_scale_pref(pref_id)) {
		image::flush_cache();
	}
	preferences::set(pref_id, new_val.to_string());
}

tadvanced_graphics_options::tadvanced_graphics_options()
{
}

void tadvanced_graphics_options::pre_show(twindow& window)
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
	if(get_retval() == twindow::OK) {
		BOOST_FOREACH(const std::string & i, scale_cases) {
			update_scale_case(i);
		}
		image::update_from_preferences();
	}
}

} // end namespace gui2
