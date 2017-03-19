/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/scroll_label.hpp"

#include "gui/widgets/label.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/log.hpp"
#include "gui/core/window_builder/helper.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/scrollbar.hpp"
#include "gui/widgets/spacer.hpp"
#include "gui/widgets/window.hpp"
#include "gettext.hpp"

#include "utils/functional.hpp"

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(scroll_label)

scroll_label::scroll_label(bool wrap, const std::string& text_alignment)
	: scrollbar_container(COUNT)
	, state_(ENABLED)
	, wrap_on(wrap)
	, text_alignment(text_alignment)
{
	connect_signal<event::LEFT_BUTTON_DOWN>(
			std::bind(
					&scroll_label::signal_handler_left_button_down, this, _2),
			event::dispatcher::back_pre_child);
}

void scroll_label::set_label(const t_string& lbl)
{
	// Inherit.
	styled_widget::set_label(lbl);

	if(content_grid()) {
		label* widget
				= find_widget<label>(content_grid(), "_label", false, true);
		widget->set_label(lbl);

		// We want the width to stay cosistent
		widget->request_reduce_width(widget->get_size().x);

		content_resize_request();
	}
}

void scroll_label::set_use_markup(bool use_markup)
{
	// Inherit.
	styled_widget::set_use_markup(use_markup);

	if(content_grid()) {
		label* widget
				= find_widget<label>(content_grid(), "_label", false, true);
		widget->set_use_markup(use_markup);
	}
}

void scroll_label::set_self_active(const bool active)
{
	state_ = active ? ENABLED : DISABLED;
}

bool scroll_label::get_active() const
{
	return state_ != DISABLED;
}

unsigned scroll_label::get_state() const
{
	return state_;
}

void scroll_label::finalize_subclass()
{
	assert(content_grid());
	label* lbl = dynamic_cast<label*>(content_grid()->find("_label", false));

	assert(lbl);
	lbl->set_label(get_label());
	lbl->set_can_wrap(wrap_on);
	lbl->set_text_alignment(decode_text_alignment(text_alignment));
	lbl->set_use_markup(get_use_markup());
}

void scroll_label::set_can_wrap(bool can_wrap)
{
	assert(content_grid());
	label* lbl = dynamic_cast<label*>(content_grid()->find("_label", false));

	assert(lbl);
	wrap_on = can_wrap;
	lbl->set_can_wrap(wrap_on);
}

bool scroll_label::can_wrap() const
{
	return wrap_on;
}

const std::string& scroll_label::get_control_type() const
{
	static const std::string type = "scroll_label";
	return type;
}

void scroll_label::signal_handler_left_button_down(const event::ui_event event)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	get_window()->keyboard_capture(this);
}

// }---------- DEFINITION ---------{

scroll_label_definition::scroll_label_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing scroll label " << id << '\n';

	load_resolutions<resolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_scroll_label
 *
 * == Scroll label ==
 *
 * @macro = scroll_label_description
 *
 * @begin{parent}{name="gui/"}
 * This widget is slower as a normal label widget so only use this widget
 * when the scrollbar is required (or expected to become required).
 * @begin{tag}{name="scroll_label_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super="generic/widget_definition/resolution"}
 * @begin{table}{config}
 *     grid & grid & &                 A grid containing the widgets for main
 *                                     widget. $
 * @end{table}
 * @allow{link}{name="gui/window/resolution/grid"}
 * TODO we need one definition for a vertical scrollbar since this is the second
 * time we use it.
 *
 * @begin{table}{dialog_widgets}
 *     _content_grid & & grid & m &    A grid which should only contain one
 *                                     label widget. $
 *     _scrollbar_grid & & grid & m &  A grid for the scrollbar
 *                                     (Merge with listbox info.) $
 * @end{table}
 * @begin{tag}{name="content_grid"}{min=0}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="content_grid"}
 * @begin{tag}{name="scrollbar_grid"}{min=0}{max=1}{super="gui/window/resolution/grid"}
 * @end{tag}{name="scrollbar_grid"}
 * The following states exist:
 * * state_enabled, the scroll label is enabled.
 * * state_disabled, the scroll label is disabled.
 * @begin{tag}{name="state_enabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_enabled"}
 * @begin{tag}{name="state_disabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_disabled"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="scroll_label_definition"}
 * @end{parent}{name="gui/"}
 */
scroll_label_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg), grid(nullptr)
{
	// Note the order should be the same as the enum state_t is scroll_label.hpp.
	state.push_back(state_definition(cfg.child("state_enabled")));
	state.push_back(state_definition(cfg.child("state_disabled")));

	const config& child = cfg.child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = std::make_shared<builder_grid>(child);
}

// }---------- BUILDER -----------{

/*WIKI_MACRO
 * @begin{macro}{scroll_label_description}
 *
 *        A scroll label is a label that wraps its text and also has a
 *        vertical scrollbar. This way a text can't be too long to be shown
 *        for this widget.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_scroll_label
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="scroll_label"}{min="0"}{max="-1"}{super="generic/widget_instance"}
 * == Scroll label ==
 *
 * @macro = scroll_label_description
 *
 * List with the scroll label specific variables:
 * @begin{table}{config}
 *     vertical_scrollbar_mode & scrollbar_mode & initial_auto &
 *                                     Determines whether or not to show the
 *                                     scrollbar. $
 *     horizontal_scrollbar_mode & scrollbar_mode & initial_auto &
 *                                     Determines whether or not to show the
 *                                     scrollbar. $
 *     wrap & boolean & true &         Determines whether the text of the
 *                                     label is allowed to wrap. $
 * @end{table}
 * @end{tag}{name="scroll_label"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

namespace implementation
{

builder_scroll_label::builder_scroll_label(const config& cfg)
	: implementation::builder_styled_widget(cfg)
	, vertical_scrollbar_mode(
			  get_scrollbar_mode(cfg["vertical_scrollbar_mode"]))
	, horizontal_scrollbar_mode(
			  get_scrollbar_mode(cfg["horizontal_scrollbar_mode"]))
	, wrap_on(cfg["wrap"].to_bool(true))
	, text_alignment(cfg["text_alignment"])
{
}

widget* builder_scroll_label::build() const
{
	scroll_label* widget = new scroll_label(wrap_on, text_alignment);

	init_control(widget);

	widget->set_vertical_scrollbar_mode(vertical_scrollbar_mode);
	widget->set_horizontal_scrollbar_mode(horizontal_scrollbar_mode);

	std::shared_ptr<const scroll_label_definition::resolution>
	conf = std::static_pointer_cast<const scroll_label_definition::resolution>(
					widget->config());
	assert(conf);

	widget->init_grid(conf->grid);
	widget->finalize_setup();

	DBG_GUI_G << "Window builder: placed scroll label '" << id
			  << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
