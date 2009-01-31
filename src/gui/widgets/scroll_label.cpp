/* $Id$ */
/*
   copyright (C) 2008 - 2009 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
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
		tlabel* widget = content_grid()->
				find_widget<tlabel>("_label", false, true);
		widget->set_label(label);
	}
}

void tscroll_label::finalize_subclass()
{
	assert(content_grid());
	tlabel* lbl = dynamic_cast<tlabel*>(
		   	content_grid()->find_widget("_label", false));

	assert(lbl);
	lbl->set_label(label());

	/**
	 * @todo wrapping should be a label setting.
	 * This setting shoul be mutual exclusive with the horizontal scrollbar.
	 * Also the scroll_grid needs to set the status for the scrollbars.
	 */
	lbl->set_can_wrap(true);
}

} // namespace gui2

