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

#include "gui/widgets/tooltip.hpp"

#include "gui/auxiliary/widget_definition/tooltip.hpp"
#include "gui/auxiliary/window_builder/control.hpp"
#include "gui/widgets/settings.hpp"

#include <boost/bind.hpp>

namespace gui2 {

namespace implementation {

/** @todo See whether this hack can be removed. */
// Needed to fix a compiler error in REGISTER_WIDGET.
class tbuilder_tooltip
	: public tbuilder_control
{
public:
	tbuilder_tooltip(const config& cfg)
		: tbuilder_control(cfg)
	{
	}

	twidget* build() const { return NULL; }
};

} // namespace implementation

REGISTER_WIDGET(tooltip)

const std::string& ttooltip::get_control_type() const
{
	static const std::string type = "tooltip";
	return type;
}

} // namespace gui2

