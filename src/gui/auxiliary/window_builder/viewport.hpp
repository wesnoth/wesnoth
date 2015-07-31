/*
   Copyright (C) 2012 - 2015 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_AUXILIARY_WINDOW_BUILDER_VIEWPORT_HPP_INCLUDED
#define GUI_AUXILIARY_WINDOW_BUILDER_VIEWPORT_HPP_INCLUDED

#include "gui/auxiliary/window_builder.hpp"

namespace gui2
{

namespace implementation
{

struct tbuilder_viewport : public tbuilder_widget
{
	explicit tbuilder_viewport(const config& cfg);

	twidget* build() const;

	twidget* build(const treplacements& replacements) const;

	tbuilder_widget_ptr widget;
};

} // namespace implementation

} // namespace gui2

#endif
