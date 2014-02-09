/*
   Copyright (C) 2008 - 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_AUXILIARY_WINDOW_BUILDER_CONTROL_HPP_INCLUDED
#define GUI_AUXILIARY_WINDOW_BUILDER_CONTROL_HPP_INCLUDED

#include "gui/auxiliary/window_builder.hpp"

namespace gui2
{

class tcontrol;

namespace implementation
{

struct tbuilder_control : public tbuilder_widget
{
public:
	tbuilder_control(const config& cfg);

	using tbuilder_widget::build;

	virtual twidget* build(const treplacements& replacements) const OVERRIDE;

	/** @deprecated The control can initialize itself. */
	void init_control(tcontrol* control) const;

	/** Parameters for the control. */
	std::string definition;
	t_string label;
	t_string tooltip;
	t_string help;
	bool use_tooltip_on_label_overflow;
};

} // namespace implementation

} // namespace gui2

#endif
