/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file window_builder_private.hpp
 * This file contains all classes used privately in window_builder.cpp and
 * should only be included by window_builder.cpp.
 */

#ifndef GUI_AUXILIARY_WINDOW_BUILDER_PRIVATE_HPP_INCLUDED
#define GUI_AUXILIARY_WINDOW_BUILDER_PRIVATE_HPP_INCLUDED

#include "gui/auxiliary/window_builder.hpp"

#include "config.hpp"
#include "gui/auxiliary/window_builder/control.hpp"
#include "gui/widgets/menubar.hpp"
#include "gui/widgets/scrollbar_container.hpp"

namespace gui2 {

/**
 * A temporary helper class.
 *
 * @todo refactor with the grid builder.
 */
struct tbuilder_gridcell : public tbuilder_widget
{
	tbuilder_gridcell(const config& cfg);

	/** The flags for the cell. */
	unsigned flags;

	/** The bordersize for the cell. */
	unsigned border_size;

	/** The widgets for the cell. */
	tbuilder_widget_ptr widget;

	/** We're a dummy the building is done on construction. */
	twidget* build () const { return NULL; }
};

struct tbuilder_horizontal_scrollbar : public implementation::tbuilder_control
{
private:
	tbuilder_horizontal_scrollbar();

public:
/*WIKI
 * @page = GUIToolkitWML
 * @order = 2_horizontal_scrollbar
 *
 * == Horizontal scrollbar ==
 *
 * A horizontal scrollbar has no special fields.
 *
 */
	tbuilder_horizontal_scrollbar(const config& cfg) :
		implementation::tbuilder_control(cfg)
	{}

	twidget* build () const;
};

struct tbuilder_listbox : public implementation::tbuilder_control
{

private:
	tbuilder_listbox();
public:
	tbuilder_listbox(const config& cfg);

	twidget* build () const;

	tscrollbar_container::tscrollbar_mode
			vertical_scrollbar_mode,
			horizontal_scrollbar_mode;

	tbuilder_grid_ptr header;
	tbuilder_grid_ptr footer;

	tbuilder_grid_ptr list_builder;

	/**
	 * Listbox data.
	 *
	 * Contains a vector with the data to set in every cell, it's used to
	 * serialize the data in the config, so the config is no longer required.
	 */
	std::vector<string_map>list_data;
};


struct tbuilder_multi_page
	: public implementation::tbuilder_control
{

private:
	tbuilder_multi_page();
public:
	tbuilder_multi_page(const config& cfg);

	twidget* build () const;

	tbuilder_grid_ptr builder;

	/**
	 * Multi page data.
	 *
	 * Contains a vector with the data to set in every cell, it's used to
	 * serialize the data in the config, so the config is no longer required.
	 */
	std::vector<string_map> data;
};

struct tbuilder_slider : public implementation::tbuilder_control
{

private:
	tbuilder_slider();
public:
	tbuilder_slider(const config& cfg);

	twidget* build () const;

private:
	unsigned best_slider_length_;
	int minimum_value_;
	int maximum_value_;
	unsigned step_size_;
	int value_;

	t_string minimum_value_label_;
	t_string maximum_value_label_;

	std::vector<t_string> value_labels_;
};

struct tbuilder_scroll_label : public implementation::tbuilder_control
{
public:
/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_scroll_label
 *
 * == Scroll label ==
 *
 * A scroll label has no special fields.
 *
 * @start_table = config
 *     auto_hide_scrollbar (bool = true)
 *                                     Automatically hide the scrollbar when the
 *                                     text can be shown without the scrollbar.
 * @end_table
 */
	tbuilder_scroll_label(const config& cfg)
		: implementation::tbuilder_control(cfg)
		, auto_hide_scrollbar_(
			utils::string_bool(cfg["auto_hide_scrollbar"], true))
	{
	}

	twidget* build () const;

private:

	bool auto_hide_scrollbar_;
};

struct tbuilder_scrollbar_panel
	: public implementation::tbuilder_control
{

private:
	tbuilder_scrollbar_panel();
public:
	tbuilder_scrollbar_panel(const config& cfg);

	twidget* build () const;

	tscrollbar_container::tscrollbar_mode
			vertical_scrollbar_mode,
			horizontal_scrollbar_mode;

	tbuilder_grid_ptr grid;
};

struct tbuilder_toggle_button : public implementation::tbuilder_control
{
private:
	tbuilder_toggle_button();

public:
/*WIKI
 * @page = GUIToolkitWML
 * @order = 2_toggle_button
 *
 * == Toggle button ==
 *
 * @start_table = config
 *     icon (f_string = "")            The name of the icon file to show.
 *     return_value_id (string = "")   The return value id, see
 *                                     [[GUIToolkitWML#Button]] for more info.
 *     return_value (int = 0)          The return value, see
 *                                     [[GUIToolkitWML#Button]] for more info.
 * @end_table
 */
	tbuilder_toggle_button(const config& cfg) :
		implementation::tbuilder_control(cfg),
		icon_name_(cfg["icon"]),
		retval_id_(cfg["return_value_id"]),
		retval_(lexical_cast_default<int>(cfg["return_value"]))
	{}

	twidget* build () const;

private:
	std::string icon_name_;
	std::string retval_id_;
	int retval_;
};

struct tbuilder_vertical_scrollbar : public implementation::tbuilder_control
{
private:
	tbuilder_vertical_scrollbar();

public:
/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_vertical_scrollbar
 *
 * == Vertical scrollbar ==
 *
 * A vertical scrollbar has no special fields.
 *
 */
	tbuilder_vertical_scrollbar(const config& cfg) :
		implementation::tbuilder_control(cfg)
	{}

	twidget* build () const;
};

} // namespace gui2

#endif

