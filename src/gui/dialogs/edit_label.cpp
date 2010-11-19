/* $Id$ */
/*
   Copyright (C) 2010 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/edit_label.hpp"

#include "gui/dialogs/field.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {

REGISTER_WINDOW(edit_label)

tedit_label::tedit_label(const std::string& label, bool team_only)
	: team_only_(team_only)
	, label_(label)
	, label_field_(register_text("label", false))
{
}

void tedit_label::pre_show(CVideo& /*video*/, twindow& window)
{
	assert(label_field_);

	find_widget<ttoggle_button>(&window, "team_only_toggle", false).set_value(team_only_);
	label_field_->set_widget_value(window, label_);

	window.keyboard_capture(label_field_->widget(window));
}

void tedit_label::post_show(twindow& window)
{
	if(get_retval() != twindow::OK) {
		return;
	}

	label_ = label_field_->get_widget_value(window);
}

}
