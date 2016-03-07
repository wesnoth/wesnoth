/*
   Copyright (C) 2016 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/auxiliary/window_builder/unit_preview_pane.hpp"

#include "config.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/widget_definition/unit_preview_pane.hpp"
#include "gui/auxiliary/window_builder/helper.hpp"
#include "gui/widgets/unit_preview_pane.hpp"

namespace gui2
{

namespace implementation
{

tbuilder_unit_preview_pane::tbuilder_unit_preview_pane(const config& cfg)
	: tbuilder_control(cfg)
{
}

twidget* tbuilder_unit_preview_pane::build() const
{
	tunit_preview_pane* widget = new tunit_preview_pane();

	init_control(widget);

	DBG_GUI_G << "Window builder: placed unit preview pane '" << id
			  << "' with definition '" << definition << "'.\n";

	boost::intrusive_ptr<const tunit_preview_pane_definition::tresolution> conf
		= boost::dynamic_pointer_cast<
			const tunit_preview_pane_definition::tresolution>(widget->config());

	assert(conf);

	widget->init_grid(conf->grid);
	widget->finalize_setup();

	return widget;
}

} // namespace implementation

} // namespace gui2
