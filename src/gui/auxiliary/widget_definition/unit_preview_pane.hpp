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

#ifndef GUI_AUXILIARY_WIDGET_DEFINITION_UNIT_PREVIEW_PANE_HPP_INCLUDED
#define GUI_AUXILIARY_WIDGET_DEFINITION_UNIT_PREVIEW_PANE_HPP_INCLUDED

#include "gui/auxiliary/widget_definition.hpp"
#include "gui/auxiliary/window_builder.hpp"

namespace gui2
{

struct tunit_preview_pane_definition : public tcontrol_definition
{

	explicit tunit_preview_pane_definition(const config& cfg);

	struct tresolution : public tresolution_definition_
	{
		explicit tresolution(const config& cfg);

		tbuilder_grid_ptr grid;
	};
};

} // namespace gui2

#endif
