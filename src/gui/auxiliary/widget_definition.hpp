/* $Id$ */
/*
   Copyright (C) 2007 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_AUXILIARY_WIDGET_DEFINITION_HPP_INCLUDED
#define GUI_AUXILIARY_WIDGET_DEFINITION_HPP_INCLUDED

#include "gui/auxiliary/canvas.hpp"

class config;

namespace gui2 {

/**
 * Contains the state info for a resolution.
 *
 * At the moment all states are the same so there is no need to use
 * inheritance. If that is needed at some point the containers should contain
 * pointers and we should inherit from reference_counted_object.
 */
struct tstate_definition
{
private:
	tstate_definition();

public:
	tstate_definition(const config &cfg);

	tcanvas canvas;
};


/** Base class of a resolution, contains the common keys for a resolution. */
struct tresolution_definition_ : public reference_counted_object
{
private:
	tresolution_definition_();
public:
	tresolution_definition_(const config& cfg);

	unsigned window_width;
	unsigned window_height;

	unsigned min_width;
	unsigned min_height;

	unsigned default_width;
	unsigned default_height;

	unsigned max_width;
	unsigned max_height;

	unsigned text_extra_width;
	unsigned text_extra_height;
	unsigned text_font_size;
	int text_font_style;

	std::vector<tstate_definition> state;
};

typedef
	boost::intrusive_ptr<tresolution_definition_>
	tresolution_definition_ptr;

typedef
	boost::intrusive_ptr<const tresolution_definition_>
	tresolution_definition_const_ptr;

} // namespace gui2

#endif

