/*
	Copyright (C) 2010 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/selectable_item.hpp"
#include "gui/widgets/styled_widget.hpp"

#include <functional>

namespace gui2
{
template<typename T>
t_string default_status_value_getter(const T& w)
{
	// Menu Buttons
	if constexpr(std::is_same_v<menu_button, T>) {
		return w.get_value_string();
	}

	// Selectable widgets (like toggle buttons)
	if constexpr(std::is_base_of_v<selectable_item, T>) {
		return w.get_value_bool() ? _("yes") : _("no");
	}

	// Sliders
	if constexpr(std::is_same_v<slider, T>) {
		return w.get_value_label();
	}

	return w.get_label();
}

/**
 * Creates a bound status label that will reflect the label state of a widget. The initial label
 * value is set here, and then again any time the widget is modified. A function is also returned
 * that can be called to update the label manually.
 *
 * This relies on hooking into the NOTIFY_MODIFIED event, so can only be used with widgets that fire
 * that event.
 *
 * @param find_in                  The containing widget (usually a window or grid) in which to find
 *                                 the source and status label widgets.
 * @param source_id                The ID of the source widget.
 * @param value_getter             Functor to process the value of the source widget.
 * @param label_id                 The ID of the status label widget.
 *
 * @returns                        The callback function used to update the status label's value.
 */
template<typename W>
std::function<void()> bind_status_label(
		widget* find_in,
		const std::string& source_id,
		const std::function<t_string(const W&)> value_getter = default_status_value_getter<W>,
		const std::string& label_id = "")
{
	// If no label ID is provided, use the format source ID + '_label'.
	const std::string label_id_ = label_id.empty() ? source_id + "_label" : label_id;

	// Find the source value widget.
	W& source = find_in->find_widget<W>(source_id);

	// Find the target status label.
	styled_widget& label = find_in->find_widget<styled_widget>(label_id_);

	const auto update_label = [&, value_getter]() {
		label.set_label(value_getter(source));
	};

	// Bind the callback.
	connect_signal_notify_modified(source, std::bind(update_label));

	// Set initial value.
	update_label();

	return update_label;
}

} // namespace gui2
