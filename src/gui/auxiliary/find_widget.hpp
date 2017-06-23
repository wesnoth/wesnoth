/*
   Copyright (C) 2007 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "gui/widgets/helper.hpp"
#include "gui/widgets/widget.hpp"
#include "utils/const_clone.hpp"
#include "wml_exception.hpp"

namespace gui2
{

/**
 * Returns the first parent of a widget with a certain type.
 *
 * @param child                   The widget to get the parent from,
 * @tparam T                      The class of the widget to return.
 *
 * @returns                       The parent widget.
 */
template <class T>
T& get_parent(widget& child)
{
	T* result;
	widget* w = &child;
	do {
		w = w->parent();
		result = dynamic_cast<T*>(w);

	} while(w && !result);

	assert(result);
	return *result;
}

/**
 * Gets a widget with the wanted id.
 *
 * This template function doesn't return a pointer to a generic widget but
 * returns the wanted type and tests for its existence if required.
 *
 * @param widget              The widget test or find a child with the wanted
 *                            id.
 * @param id                  The id of the widget to find.
 * @param must_be_active      The widget should be active, not all widgets
 *                            have an active flag, those who don't ignore
 *                            flag.
 * @param must_exist          The widget should be exist, the function will
 *                            fail if the widget doesn't exist or is
 *                            inactive and must be active. Upon failure a
 *                            wml_error is thrown.
 *
 * @returns                   The widget with the id.
 */
template <class T>
T* find_widget(utils::const_clone_ptr<widget, T> widget,
			   const std::string& id,
			   const bool must_be_active,
			   const bool must_exist)
{
	T* result = dynamic_cast<T*>(widget->find(id, must_be_active));
	VALIDATE(!must_exist || result, missing_widget(id));

	return result;
}

/**
 * Gets a widget with the wanted id.
 *
 * This template function doesn't return a reference to a generic widget but
 * returns a reference to the wanted type
 *
 * @param widget              The widget test or find a child with the wanted
 *                            id.
 * @param id                  The id of the widget to find.
 * @param must_be_active      The widget should be active, not all widgets
 *                            have an active flag, those who don't ignore
 *                            flag.
 *
 * @returns                   The widget with the id.
 */
template <class T>
T& find_widget(utils::const_clone_ptr<widget, T> widget,
			   const std::string& id,
			   const bool must_be_active)
{
	return *find_widget<T>(widget, id, must_be_active, true);
}

} // namespace gui2
