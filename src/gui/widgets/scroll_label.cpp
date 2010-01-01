/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/scroll_label.hpp"

#include "gui/widgets/label.hpp"
#include "gui/widgets/scrollbar.hpp"
#include "gui/widgets/spacer.hpp"

namespace gui2 {
tscroll_label::tscroll_label()
	: tscrollbar_container(COUNT)
	, state_(ENABLED)
{
}

void tscroll_label::set_label(const t_string& label)
{
	// Inherit.
	tcontrol::set_label(label);

	if(content_grid()) {
		tlabel* widget =
				find_widget<tlabel>(content_grid(), "_label", false, true);
		widget->set_label(label);

		content_resize_request();
	}
}

void tscroll_label::set_use_markup(bool use_markup)
{
	// Inherit.
	tcontrol::set_use_markup(use_markup);

	if(content_grid()) {
		tlabel* widget =
				find_widget<tlabel>(content_grid(), "_label", false, true);
		widget->set_use_markup(use_markup);
	}
}

void tscroll_label::finalize_subclass()
{
	assert(content_grid());
	tlabel* lbl = dynamic_cast<tlabel*>(
			content_grid()->find("_label", false));

	assert(lbl);
	lbl->set_label(label());

	/**
	 * @todo wrapping should be a label setting.
	 * This setting shoul be mutual exclusive with the horizontal scrollbar.
	 * Also the scroll_grid needs to set the status for the scrollbars.
	 */
	lbl->set_can_wrap(true);
}

const std::string& tscroll_label::get_control_type() const
{
	static const std::string type = "scroll_label";
	return type;
}

} // namespace gui2

