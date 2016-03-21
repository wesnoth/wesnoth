/*
   Copyright (C) 2007 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_WIDGETS_DETAIL_REGISTER_TPP_INCLUDED
#define GUI_WIDGETS_DETAIL_REGISTER_TPP_INCLUDED

/**
 * Registers a widget.
 *
 * Call this function to register a widget. Use this macro in the
 * implementation, inside the gui2 namespace.
 *
 * See @ref gui2::load_widget_definitions for more information.
 *
 * @note When the type is tfoo_definition, the id "foo" and no special key best
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
	namespace ns_##type                                                        \
	{                                                                          \
                                                                               \
		struct tregister_helper                                                \
		{                                                                      \
			tregister_helper()                                                 \
			{                                                                  \
				register_widget(#id,                                           \
								boost::bind(load_widget_definitions<type>,     \
											_1,                                \
											_2,                                \
											_3,                                \
											key));                             \
                                                                               \
				register_builder_widget(                                       \
						#id,                                                   \
						boost::bind(                                           \
								build_widget<implementation::tbuilder_##id>,   \
								_1));                                          \
			}                                                                  \
		};                                                                     \
                                                                               \
		static tregister_helper register_helper;                               \
	}                                                                          \
	}

/**
 * Wrapper for REGISTER_WIDGET3.
 *
 * "Calls" REGISTER_WIDGET3(tid_definition, id, _4)
 */
#define REGISTER_WIDGET(id) REGISTER_WIDGET3(t##id##_definition, id, _4)

#endif
