/*
   Copyright (C) 2012 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

#define ENUM_ENABLE_STREAM_OPERATORS_IMPLEMENTATION
#define ENUM_TYPE ::gui2::tplacer_
#define ENUM_LIST                                                              \
	ENUM(horizontal, "horizontal");                                            \
	ENUM(vertical, "vertical");

#include "gui/auxiliary/placer.hpp"

#include "asserts.hpp"
#include "gui/auxiliary/placer/horizontal_list.hpp"
#include "gui/auxiliary/placer/vertical_list.hpp"


namespace gui2
{

ENUM_DEFINE_STREAM_OPERATORS(::gui2::tplacer_::tgrow_direction)

tplacer_* tplacer_::build(const tgrow_direction grow_direction,
						  const unsigned parallel_items)
{
	switch(grow_direction) {
		case horizontal:
			return new implementation::tplacer_horizontal_list(parallel_items);
		case vertical:
			return new implementation::tplacer_vertical_list(parallel_items);
	};

	UNREACHABLE_CODE;
}

tplacer_::~tplacer_()
{
}

} // namespace gui2
