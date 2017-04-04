/*
   Copyright (C) 2016 - 2017 Jyrki Vesterinen <sandgtx@gmail.com>
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

#include "size_lock.hpp"

#include <gettext.hpp>
#include <gui/core/register_widget.hpp>
#include <gui/widgets/helper.hpp>
#include <gui/widgets/settings.hpp>
#include <wml_exception.hpp>

namespace gui2
{

REGISTER_WIDGET(size_lock)

void size_lock::place(const point& origin, const point& size)
{
	point content_size = widget_->get_best_size();

	if (content_size.x > size.x)
	{
		reduce_width(size.x);
		content_size = widget_->get_best_size();
	}

	if (content_size.y > size.y)
	{
		reduce_height(size.y);
		content_size = widget_->get_best_size();
	}

	container_base::place(origin, size);
}

void size_lock::layout_children()
{
	assert(widget_ != nullptr);

	widget_->layout_children();
}

void size_lock::finalize(builder_widget_const_ptr widget_builder)
{
	set_rows_cols(1u, 1u);

	widget_ = widget_builder->build();
	set_child(widget_, 0u, 0u,
		grid::VERTICAL_GROW_SEND_TO_CLIENT | grid::HORIZONTAL_GROW_SEND_TO_CLIENT,
		0u);
}

size_lock_definition::size_lock_definition(const config& cfg) :
	styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing fixed size widget " << id << '\n';

	load_resolutions<resolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_size_lock
 *
 * == Size lock ==
 *
 * A size lock contains one child widget and forces it to have the specified size.
 * This can be used, for example, when there are two list boxes in different rows of
 * the same grid and it's desired that only one list box changes size when its
 * contents change.
 *
 * A size lock has no states.
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="size_lock_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * @end{tag}{name="size_lock_definition"}
 * @end{tag}{name="gui/"}
 */
size_lock_definition::resolution::resolution(const config& cfg) :
	resolution_definition(cfg), grid(nullptr)
{
	// Add a dummy state since every widget needs a state.
	static config dummy("draw");
	state.emplace_back(dummy);

	const config& child = cfg.child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = std::make_shared<builder_grid>(child);
}

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_size_lock
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="size_lock"}{min=0}{max=-1}{super="generic/widget_instance"}
 * == Size lock ==
 *
 * A size lock contains one child widget and forces it to have the specified size.
 * This can be used, for example, when there are two list boxes in different rows of
 * the same grid and it's desired that only one list box changes size when its
 * contents change.
 *
 * @begin{table}{config}
 *    widget & section    & mandatory &           The widget. $
 *    width  & f_unsigned & mandatory &           The width of the widget. $
 *    height & f_unsigned & mandatory &           The height of the widget. $
 * @end{table}
 *
 * The variables available are the same as for window resolution, see
 * [[GuiToolkitWML#Resolution_2]] for the list of items.
 * @end{tag}{name="size_lock"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

namespace implementation
{

builder_size_lock::builder_size_lock(const config& cfg) :
	builder_styled_widget(cfg), content_(nullptr), width_(cfg["width"]), height_(cfg["height"])
{
	VALIDATE(cfg.has_child("widget"), _("No widget defined."));
	content_ = create_builder_widget(cfg.child("widget"));
}

widget* builder_size_lock::build() const
{
	size_lock* widget = new size_lock();

	init_control(widget);

	DBG_GUI_G << "Window builder: placed fixed size widget '" << id <<
		"' with definition '" << definition << "'.\n";

	auto conf = std::static_pointer_cast<const size_lock_definition::resolution>(widget->config());
	assert(conf != nullptr);

	widget->init_grid(conf->grid);

	wfl::map_formula_callable size = get_screen_size_variables();

	const unsigned width = width_(size);
	const unsigned height = height_(size);

	VALIDATE(width > 0 || height > 0, _("Invalid size."));

	widget->set_target_size(point(width, height));

	widget->finalize(content_);

	return widget;
}

}

}
