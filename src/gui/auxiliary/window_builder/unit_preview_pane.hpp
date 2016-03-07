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

#ifndef GUI_AUXILIARY_WINDOW_BUILDER_UNIT_PREVIEW_PANE_HPP_INCLUDED
#define GUI_AUXILIARY_WINDOW_BUILDER_UNIT_PREVIEW_PANE_HPP_INCLUDED

#include "gui/auxiliary/window_builder/control.hpp"

namespace gui2
{

namespace implementation
{

struct tbuilder_unit_preview_pane : public tbuilder_control
{
	explicit tbuilder_unit_preview_pane(const config& cfg);

	using tbuilder_control::build;

	twidget* build() const;
};

} // namespace implementation

} // namespace gui2

#endif
