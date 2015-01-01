/*
   Copyright (C) 2008 - 2015 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_AUXILIARY_WINDOW_BUILDER_SLIDER_HPP_INCLUDED
#define GUI_AUXILIARY_WINDOW_BUILDER_SLIDER_HPP_INCLUDED

#include "gui/auxiliary/window_builder/control.hpp"

#include "tstring.hpp"

#include <vector>

namespace gui2
{

namespace implementation
{

struct tbuilder_slider : public tbuilder_control
{
	explicit tbuilder_slider(const config& cfg);

	using tbuilder_control::build;

	twidget* build() const;

private:
	unsigned best_slider_length_;
	int minimum_value_;
	int maximum_value_;
	unsigned step_size_;
	int value_;

	t_string minimum_value_label_;
	t_string maximum_value_label_;

	std::vector<t_string> value_labels_;
};

} // namespace implementation

} // namespace gui2

#endif
