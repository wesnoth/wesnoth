/*
	Copyright (C) 2008 - 2024
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/toggle_panel.hpp"

#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "gui/core/log.hpp"
#include "gui/core/window_builder/helper.hpp"
#include "gettext.hpp"
#include "sound.hpp"
#include "wml_exception.hpp"

#include <functional>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(toggle_panel)

toggle_panel::toggle_panel(const implementation::builder_toggle_panel& builder)
	: panel(builder, type())
	, state_(ENABLED)
	, state_num_(0)
	, retval_(retval::NONE)
{
	set_wants_mouse_left_double_click();

	connect_signal<event::MOUSE_ENTER>(std::bind(
			&toggle_panel::signal_handler_mouse_enter, this, std::placeholders::_2, std::placeholders::_3));
	connect_signal<event::MOUSE_LEAVE>(std::bind(
			&toggle_panel::signal_handler_mouse_leave, this, std::placeholders::_2, std::placeholders::_3));
#if 0
	connect_signal<event::LEFT_BUTTON_CLICK>(
			std::bind(&toggle_panel::signal_handler_pre_left_button_click,
						this,
						std::placeholders::_2),
			event::dispatcher::back_pre_child);
#endif
	connect_signal<event::LEFT_BUTTON_CLICK>(std::bind(
			&toggle_panel::signal_handler_left_button_click, this, std::placeholders::_2, std::placeholders::_3));
	connect_signal<event::LEFT_BUTTON_CLICK>(
			std::bind(&toggle_panel::signal_handler_left_button_click,
						this,
						std::placeholders::_2,
						std::placeholders::_3),
			event::dispatcher::back_post_child);
	connect_signal<event::LEFT_BUTTON_DOUBLE_CLICK>(
			std::bind(&toggle_panel::signal_handler_left_button_double_click,
						this,
						std::placeholders::_2,
						std::placeholders::_3));
	connect_signal<event::LEFT_BUTTON_DOUBLE_CLICK>(
			std::bind(&toggle_panel::signal_handler_left_button_double_click,
						this,
						std::placeholders::_2,
						std::placeholders::_3),
			event::dispatcher::back_post_child);
}

unsigned toggle_panel::num_states() const
{
	std::div_t res = std::div(this->get_config()->state.size(), COUNT);
	assert(res.rem == 0);
	assert(res.quot > 0);
	return res.quot;
}

void toggle_panel::set_child_members(const widget_data& data)
{
	for(const auto & item : data)
	{
		styled_widget* control = dynamic_cast<styled_widget*>(find(item.first, false));
		if(control) {
			control->set_members(item.second);
		}
	}
}
widget* toggle_panel::find_at(const point& coordinate,
								const bool must_be_active)
{
	/**
	 * @todo since there is no mouse event nesting (or event nesting at all)
	 * we need to capture all events. This means items on the panel will
	 * never receive an event, which gives problems with for example the
	 * intended button on the addon panel. So we need to chain mouse events
	 * as well and also add a handled flag for them.
	 */

	widget* result = container_base::find_at(coordinate, must_be_active);
	return result ? result : styled_widget::find_at(coordinate, must_be_active);
}

const widget* toggle_panel::find_at(const point& coordinate,
									  const bool must_be_active) const
{
	const widget* result = container_base::find_at(coordinate, must_be_active);
	return result ? result : styled_widget::find_at(coordinate, must_be_active);
}

void toggle_panel::set_active(const bool active)
{
	if(active) {
		set_state(ENABLED);
	} else {
		set_state(DISABLED);
	}
}

bool toggle_panel::get_active() const
{
	return state_ != DISABLED;
}

unsigned toggle_panel::get_state() const
{
	return state_ + COUNT * state_num_;
}

SDL_Rect toggle_panel::get_client_rect() const
{
	const auto conf = cast_config_to<toggle_panel_definition>();
	assert(conf);

	SDL_Rect result = get_rectangle();
	result.x += conf->left_border;
	result.y += conf->top_border;
	result.w -= conf->left_border + conf->right_border;
	result.h -= conf->top_border + conf->bottom_border;

	return result;
}

point toggle_panel::border_space() const
{
	const auto conf = cast_config_to<toggle_panel_definition>();
	assert(conf);

	return point(conf->left_border + conf->right_border, conf->top_border + conf->bottom_border);
}

void toggle_panel::set_value(unsigned selected, bool fire_event)
{
	selected = selected % num_states();
	if(selected == get_value()) {
		return;
	}
	state_num_ = selected;
	queue_redraw();

	// Check for get_window() is here to prevent the callback from
	// being called when the initial value is set.
	if(get_window() && fire_event) {
		fire(event::NOTIFY_MODIFIED, *this, nullptr);
	}
}

void toggle_panel::set_retval(const int retval)
{
	retval_ = retval;
}

void toggle_panel::set_state(const state_t state)
{
	if(state == state_) {
		return;
	}

	state_ = state;
	queue_redraw();

	const auto conf = cast_config_to<toggle_panel_definition>();
	assert(conf);
}

bool toggle_panel::impl_draw_background()
{
	// We don't have a fore and background and need to draw depending on
	// our state, like a styled_widget. So we use the styled_widget's drawing method.
	return styled_widget::impl_draw_background();
}

bool toggle_panel::impl_draw_foreground()
{
	// We don't have a fore and background and need to draw depending on
	// our state, like a styled_widget. So we use the styled_widget's drawing method.
	return styled_widget::impl_draw_foreground();
}

void toggle_panel::signal_handler_mouse_enter(const event::ui_event event,
											   bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	set_state(FOCUSED);
	handled = true;
}

void toggle_panel::signal_handler_mouse_leave(const event::ui_event event,
											   bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	set_state(ENABLED);
	handled = true;
}

void
toggle_panel::signal_handler_pre_left_button_click(const event::ui_event event)
{
	DBG_GUI_E << get_control_type() << "[" << id() << "]: " << event << ".";

	set_value(1, true);

#if 0
	/*
	 * Disabled since it causes issues with gamestate inspector (bug #22095).
	 * It was added in b84f2ebff0b53c7e4194da315c43f62a08494c52 for the lobby,
	 * since that code is still experimental, prefer to fix a real issue caused
	 * by it.
	 *
	 * The issue is that the gui2::listbox::add_row code was changed to
	 * increase the content size. Before the list was shown the list was
	 * cleared. The clear operation did not reduce the size (since the widgets
	 * were not shown yet). The add operation afterwards again reserved the
	 * space causing the size of the listbox to be twice the required space.
	 *
	 * 2014.06.09 -- Mordante
	 */

	fire(event::NOTIFY_MODIFIED, *this, nullptr);
#endif
}

void toggle_panel::signal_handler_left_button_click(const event::ui_event event,
													 bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	sound::play_UI_sound(settings::sound_toggle_panel_click);

	set_value(get_value() + 1, true);

	handled = true;
}

void toggle_panel::signal_handler_left_button_double_click(
		const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	if(retval_) {
		window* window = get_window();
		assert(window);

		window->set_retval(retval_);
	}

	handled = true;
}

// }---------- DEFINITION ---------{

toggle_panel_definition::toggle_panel_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing toggle panel " << id;

	load_resolutions<resolution>(cfg);
}

toggle_panel_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
	, top_border(cfg["top_border"].to_unsigned())
	, bottom_border(cfg["bottom_border"].to_unsigned())
	, left_border(cfg["left_border"].to_unsigned())
	, right_border(cfg["right_border"].to_unsigned())
{
	// Note the order should be the same as the enum state_t in toggle_panel.hpp.
	for(const auto& c : cfg.child_range("state"))
	{
		state.emplace_back(VALIDATE_WML_CHILD(c, "enabled", missing_mandatory_wml_tag("toggle_panel_definition][resolution][state", "enabled")));
		state.emplace_back(VALIDATE_WML_CHILD(c, "disabled", missing_mandatory_wml_tag("toggle_panel_definition][resolution][state", "disabled")));
		state.emplace_back(VALIDATE_WML_CHILD(c, "focused", missing_mandatory_wml_tag("toggle_panel_definition][resolution][state", "focused")));
	}
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_toggle_panel::builder_toggle_panel(const config& cfg)
	: builder_styled_widget(cfg)
	, grid(nullptr)
	, retval_id_(cfg["return_value_id"])
	, retval_(cfg["return_value"].to_int())
{
	auto c = cfg.optional_child("grid");

	VALIDATE(c, _("No grid defined."));

	grid = std::make_shared<builder_grid>(*c);
}

std::unique_ptr<widget> builder_toggle_panel::build() const
{
	auto widget = std::make_unique<toggle_panel>(*this);

	widget->set_retval(get_retval(retval_id_, retval_, id));

	DBG_GUI_G << "Window builder: placed toggle panel '" << id
			  << "' with definition '" << definition << "'.";

	widget->init_grid(*grid);
	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
