/*
	Copyright (C) 2010 - 2025
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
namespace implementation
{
template<typename T>
t_string get_status_label(const T& w)
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
 * Searches for the status target, starting with the source widget's parent.
 * and walking backwards up the parent tree until the target is found.
 *
 * @todo this is a more powerful version of find_widget. Investigate whether
 * we want this behavior exposed more broadly.
 */
inline auto& find_target(std::string_view target_id, widget& source)
{
	widget* parent = source.parent();
	widget* target = nullptr;

	while(parent) {
		target = parent->find(target_id, false);
		parent = parent->parent();

		if(target) {
			break;
		}
	}

	VALIDATE(target, missing_widget(std::string(target_id)));
	return dynamic_cast<styled_widget&>(*target);
}

/** Returns the dereferenced target pointer if valid, else the default target. */
inline auto& validate_target(styled_widget* target, widget& source)
{
	if(target) {
		return *target;
	}

	return find_target(source.id() + "_label", source);
}

} // namespace implementation

/**
 * Binds a given target widget to reflect another widget's label.
 *
 * The initial status label value will be set here and updated whenever the source
 * widget is modified (specifically, when a NOTIFY_MODIFIED event is fired).
 *
 * @param source           The widget whose value will be represented in the target.
 * @param value_getter     Functor to process the value of the source widget.
 *                         The expected signature is [](const W&) -> [t_]string.
 * @param target_ptr       Pointer to the status widget. If no target is specified,
 *                         the default with ID [sourceID + "_label"] will be used.
 */
template<typename W, typename F>
void bind_status_label(W& source, const F& value_getter, styled_widget* target_ptr = nullptr)
{
	// Get fallback if necessary.
	styled_widget& target = implementation::validate_target(target_ptr, source);

	connect_signal_notify_modified(source,
		[&, value_getter](auto&&...) { target.set_label(value_getter(source)); });

	// Set initial value.
	source.fire(event::NOTIFY_MODIFIED, source, nullptr);
}

/**
 * Binds a status label using the default value getter and default target ID.
 */
template<typename W>
void bind_default_status_label(W& source)
{
	bind_status_label(source, implementation::get_status_label<W>, nullptr);
}

/**
 * Binds a status label using the default value getter and the given ID.
 */
template<typename W>
void bind_default_status_label(W& source, std::string_view target_id)
{
	auto& target = implementation::find_target(target_id, source);
	bind_status_label(source, implementation::get_status_label<W>, &target);
}

} // namespace gui2
