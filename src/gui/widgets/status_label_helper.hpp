/*
   Copyright (C) 2010 - 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gettext.hpp"
#include "gui/core/event/dispatcher.hpp"
#include "gui/widgets/styled_widget.hpp"
#include "gui/widgets/integer_selector.hpp"
#include "gui/widgets/selectable_item.hpp"

#include "utils/functional.hpp"

namespace gui2 {

/**
 * Default value getter for selectable widgets (like toggle buttons)
 */
template<typename T>
static inline typename std::enable_if<std::is_base_of<selectable_item, T>::value, std::string>::type
default_value_getter(T& w)
{
	return w.get_value_bool() ? _("yes") : _("no");
}

/**
 * Default value getter for integer-based widgets (like sliders)
 */
template<typename T>
static inline typename std::enable_if<std::is_base_of<integer_selector, T>::value, std::string>::type
default_value_getter(T& w)
{
	return w.get_value_label();
}

/**
 * Creates a bound status label that will reflect the label state of a widget. The initial label
 * value is set here, and then again any time the widget is modified. A function is also returned
 * that can be called to update the label manually.
 *
 * This relies on hooking into the NOTIFY_MODIFIED event, so can only be used with widgets that fire
 * that event.
 */
template<typename W>
std::function<void()> bind_status_label(widget& find_in, const std::string& id,
		const std::function<std::string(W&)> value_getter = default_value_getter<W>,
		const std::string& label_id = "")
{
	const std::string label_id_ = label_id.empty() ? id + "_label" : label_id;

	W& source = find_widget<W>(&find_in, id, false);
	styled_widget& label = find_widget<styled_widget>(&find_in, label_id_, false);

	const auto update_label = [&, value_getter]() {
		const std::string value = value_getter(source);

		label.set_label(value);
	};

	// Bind the callback
	connect_signal_notify_modified(source, std::bind(update_label));

	// Set initial value
	update_label();

	return update_label;
}

} // namespace gui2
