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

/**
 * Registers a widget.
 *
 * Call this function to register a widget. Use this macro in the
 * implementation, inside the gui2 namespace.
 *
 * See @ref gui2::load_widget_definitions for more information.
 *
 * @note When the type is foo_definition, the id "foo" and no special key best
 * use RESISTER_WIDGET(foo) instead.
 *
 * @param type                    Class type of the window to register.
 * @param id                      Id of the widget
 * @param key                     The id to load if differs from id.
 */
#define REGISTER_WIDGET3(type, id, key)                                        \
	namespace                                                                  \
	{                                                                          \
                                                                               \
	namespace ns_##type##id                                                    \
	{                                                                          \
                                                                               \
		struct register_helper                                                 \
		{                                                                      \
			register_helper()                                                  \
			{                                                                  \
				register_widget(#id, [](const config& cfg) { return std::make_shared<type>(cfg); }, key); \
                                                                               \
				register_builder_widget(#id, &build_widget<implementation::builder_##id>); \
			}                                                                  \
		};                                                                     \
                                                                               \
		static struct register_helper register_helper;                         \
	}                                                                          \
	}

/**
 * Wrapper for REGISTER_WIDGET3.
 *
 * "Calls" REGISTER_WIDGET3(id_definition, id, nullptr)
 */
#define REGISTER_WIDGET(id) REGISTER_WIDGET3(id##_definition, id, nullptr)
