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

#include "gui/auxiliary/window_builder/instance.hpp"

#include "gui/widgets/spacer.hpp"

namespace gui2
{

namespace implementation
{

tbuilder_instance::tbuilder_instance(const config& cfg)
	: tbuilder_widget(cfg), configuration(cfg)
{
}

twidget* tbuilder_instance::build() const
{
	return build(treplacements());
}

twidget* tbuilder_instance::build(const treplacements& replacements) const
{
	const treplacements::const_iterator itor = replacements.find(id);
	if(itor != replacements.end()) {
		return itor->second->build();
	} else {
		implementation::tbuilder_spacer builder(configuration);
		return builder.build();
	}
}

} // namespace implementation

} // namespace gui2

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_instance
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="instance"}{min=0}{max=-1}{super="generic/widget_instance"}
 * @end{tag}{name="instance"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */
