/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/auxiliary/window_builder/slider.hpp"

#include "config.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/slider.hpp"
#include "utils/foreach.tpp"
#include "wml_exception.hpp"

namespace gui2
{

namespace implementation
{

tbuilder_slider::tbuilder_slider(const config& cfg)
	: implementation::tbuilder_control(cfg)
	, best_slider_length_(cfg["best_slider_length"])
	, minimum_value_(cfg["minimum_value"])
	, maximum_value_(cfg["maximum_value"])
	, step_size_(cfg["step_size"])
	, value_(cfg["value"])
	, minimum_value_label_(cfg["minimum_value_label"].t_str())
	, maximum_value_label_(cfg["maximum_value_label"].t_str())
	, value_labels_()
{
	const config& labels = cfg.child("value_labels");
	if(!labels) {
		return;
	}

	FOREACH(const AUTO & label, labels.child_range("value"))
	{
		value_labels_.push_back(label["label"]);
	}
}

twidget* tbuilder_slider::build() const
{
	tslider* widget = new tslider();

	init_control(widget);

	widget->set_best_slider_length(best_slider_length_);
	widget->set_maximum_value(maximum_value_);
	widget->set_minimum_value(minimum_value_);
	widget->set_step_size(step_size_);
	widget->set_value(value_);

	if(!value_labels_.empty()) {
		VALIDATE(value_labels_.size() == widget->get_item_count(),
				 _("The number of value_labels and values don't match."));

		widget->set_value_labels(value_labels_);

	} else {
		widget->set_minimum_value_label(minimum_value_label_);
		widget->set_maximum_value_label(maximum_value_label_);
	}

	DBG_GUI_G << "Window builder: placed slider '" << id
			  << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

} // namespace gui2

/*WIKI_MACRO
 * @begin{macro}{slider_description}
 * A slider is a control that can select a value by moving a grip on a groove.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 3_slider
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="slider"}{min="0"}{max="-1"}{super="generic/widget_instance"}
 * == Slider ==
 *
 * @macro = slider_description
 *
 * @begin{table}{config}
 *     best_slider_length & unsigned & 0 &
 *                                    The best length for the sliding part. $
 *     minimum_value & int & 0 &        The minimum value the slider can have. $
 *     maximum_value & int & 0 &        The maximum value the slider can have. $
 *
 *     step_size & unsigned & 0 &       The number of items the slider's value
 *                                    increases with one step. $
 *     value & int & 0 &                The value of the slider. $
 *
 *     minimum_value_label & t_string & "" &
 *                                    If the minimum value is chosen there
 *                                    might be the need for a special value
 *                                    (eg off). When this key has a value
 *                                    that value will be shown if the minimum
 *                                    is selected. $
 *     maximum_value_label & t_string & "" &
 *                                    If the maximum value is chosen there
 *                                    might be the need for a special value
 *                                    (eg unlimited)). When this key has a
 *                                    value that value will be shown if the
 *                                    maximum is selected. $
 *     value_labels & [] &              It might be the labels need to be shown
 *                                    are not a linear number sequence eg
 *                                    (0.5, 1, 2, 4) in that case for all
 *                                    items this section can be filled with
 *                                    the values, which should be the same
 *                                    number of items as the items in the
 *                                    slider. NOTE if this option is used,
 *                                    'minimum_value_label' and
 *                                    'maximum_value_label' are ignored. $
 * @end{table}
 * @end{tag}{name="slider"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */
