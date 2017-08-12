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

scroll_label::scroll_label(bool wrap, const PangoAlignment text_alignment)
	: scrollbar_container()
	, state_(ENABLED)
	, wrap_on_(wrap)
	, text_alignment_(text_alignment)
{
	connect_signal<event::LEFT_BUTTON_DOWN>(
		std::bind(&scroll_label::signal_handler_left_button_down, this, _2),
		event::dispatcher::back_pre_child);
}

label* scroll_label::get_internal_label()
{
	if(content_grid()) {
		return dynamic_cast<label*>(content_grid()->find("_label", false));
	}

	return nullptr;
}

void scroll_label::set_label(const t_string& lbl)
{
	// Inherit.
	styled_widget::set_label(lbl);

	if(label* widget = get_internal_label()) {
		widget->set_label(lbl);

		bool resize_needed = !content_resize_request();
		if(resize_needed && get_size() != point()) {
			place(get_origin(), get_size());
		}
	}
}

void scroll_label::set_text_alignment(const PangoAlignment text_alignment)
{
	// Inherit.
	styled_widget::set_text_alignment(text_alignment);

	text_alignment_ = text_alignment;

	if(label* widget = get_internal_label()) {
		widget->set_text_alignment(text_alignment_);
	}
}

void scroll_label::set_use_markup(bool use_markup)
{
	// Inherit.
	styled_widget::set_use_markup(use_markup);

	if(label* widget = get_internal_label()) {
		widget->set_use_markup(use_markup);
	}
}

void scroll_label::set_text_alpha(unsigned short alpha)
{
	if(label* widget = get_internal_label()) {
		widget->set_text_alpha(alpha);
	}
}

void scroll_label::set_link_aware(bool l)
{
	if(label* widget = get_internal_label()) {
		widget->set_link_aware(l);
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
	label* lbl = get_internal_label();
	assert(lbl);

	lbl->set_label(get_label());
	lbl->set_can_wrap(wrap_on_);
	lbl->set_text_alignment(text_alignment_);
	lbl->set_use_markup(get_use_markup());
}

void scroll_label::set_can_wrap(bool can_wrap)
{
	label* lbl = get_internal_label();
	assert(lbl);

	wrap_on_ = can_wrap;
	lbl->set_can_wrap(wrap_on_);
}

bool scroll_label::can_wrap() const
{
	return wrap_on_;
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
	state.emplace_back(cfg.child("state_enabled"));
	state.emplace_back(cfg.child("state_disabled"));

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
	, vertical_scrollbar_mode(get_scrollbar_mode(cfg["vertical_scrollbar_mode"]))
	, horizontal_scrollbar_mode(get_scrollbar_mode(cfg["horizontal_scrollbar_mode"]))
	, wrap_on(cfg["wrap"].to_bool(true))
	, text_alignment(decode_text_alignment(cfg["text_alignment"]))
{
}

widget* builder_scroll_label::build() const
{
	scroll_label* widget = new scroll_label(wrap_on, text_alignment);

	init_control(widget);

	widget->set_vertical_scrollbar_mode(vertical_scrollbar_mode);
	widget->set_horizontal_scrollbar_mode(horizontal_scrollbar_mode);

	const auto conf = widget->cast_config_to<scroll_label_definition>();
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
