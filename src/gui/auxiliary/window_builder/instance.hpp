/*
   Copyright (C) 2012 - 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_AUXILIARY_WINDOW_BUILDER_INSTANCE_HPP_INCLUDED
#define GUI_AUXILIARY_WINDOW_BUILDER_INSTANCE_HPP_INCLUDED

#include "gui/auxiliary/window_builder.hpp"

#include "config.hpp"

namespace gui2
{

namespace implementation
{

struct tbuilder_instance : public tbuilder_widget
{
	explicit tbuilder_instance(const config& cfg);

	twidget* build() const;

	twidget* build(const treplacements& replacements) const;

	/**
	 * Holds a copy of the cfg parameter in the constructor.
	 *
	 * This is used when instantiating a spacer, it can still use the
	 * parameters originally sent.
	 */
	config configuration;
};

} // namespace implementation

} // namespace gui2

#endif
