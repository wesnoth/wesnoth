/*
   Copyright (C) 2007 - 2015 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_AUXILIARY_WIDGET_DEFINITION_TEXT_BOX_HPP_INCLUDED
#define GUI_AUXILIARY_WIDGET_DEFINITION_TEXT_BOX_HPP_INCLUDED

#include "gui/auxiliary/formula.hpp"
#include "gui/auxiliary/widget_definition.hpp"

namespace gui2
{

struct ttext_box_definition : public tcontrol_definition
{
	explicit ttext_box_definition(const config& cfg);

	struct tresolution : public tresolution_definition_
	{
		explicit tresolution(const config& cfg);

		tformula<unsigned> text_x_offset;
		tformula<unsigned> text_y_offset;
	};
};

} // namespace gui2

#endif
