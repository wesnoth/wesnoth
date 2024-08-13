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

#include "gui/widgets/options_button.hpp"

#include "gui/core/log.hpp"
#include "gui/core/widget_definition.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "sound.hpp"
#include "wml_exception.hpp"

#include <functional>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(options_button)

options_button::options_button(const implementation::builder_styled_widget& builder, const std::string& control_type)
	: styled_widget(builder, control_type)
	, state_(ENABLED)
	, values_()
	, selected_(0)
	, keep_open_(false)
	, persistent_(false)
{
	values_.emplace_back("label", this->get_label());

	connect_signal<event::MOUSE_ENTER>(
		std::bind(&options_button::signal_handler_mouse_enter, this, std::placeholders::_2, std::placeholders::_3));

	connect_signal<event::MOUSE_LEAVE>(
		std::bind(&options_button::signal_handler_mouse_leave, this, std::placeholders::_2, std::placeholders::_3));

	connect_signal<event::LEFT_BUTTON_DOWN>(
		std::bind(&options_button::signal_handler_left_button_down, this, std::placeholders::_2, std::placeholders::_3));

	connect_signal<event::LEFT_BUTTON_UP>(
		std::bind(&options_button::signal_handler_left_button_up, this, std::placeholders::_2, std::placeholders::_3));

	connect_signal<event::LEFT_BUTTON_CLICK>(
		std::bind(&options_button::signal_handler_left_button_click, this, std::placeholders::_2, std::placeholders::_3));

	connect_signal<event::SDL_WHEEL_UP>(
		std::bind(&options_button::signal_handler_sdl_wheel_up, this, std::placeholders::_2, std::placeholders::_3),
		event::dispatcher::back_post_child);

	connect_signal<event::SDL_WHEEL_DOWN>(
		std::bind(&options_button::signal_handler_sdl_wheel_down, this, std::placeholders::_2, std::placeholders::_3),
		event::dispatcher::back_post_child);
}

void options_button::set_active(const bool active)
{
	if(get_active() != active) {
		set_state(active ? ENABLED : DISABLED);
	}
}

bool options_button::get_active() const
{
	return state_ != DISABLED;
}

unsigned options_button::get_state() const
{
	return state_;
}

void options_button::set_state(const state_t state)
{
	if(state != state_) {
		state_ = state;
		queue_redraw();
	}
}

void options_button::signal_handler_mouse_enter(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	set_state(FOCUSED);
	handled = true;
}

void options_button::signal_handler_mouse_leave(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	set_state(ENABLED);
	handled = true;
}

void options_button::signal_handler_left_button_down(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	window* window = get_window();
	if(window) {
		window->mouse_capture();
	}

	set_state(PRESSED);
	handled = true;
}

void options_button::signal_handler_left_button_up(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	set_state(FOCUSED);
	handled = true;
}

void options_button::signal_handler_left_button_click(const event::ui_event event, bool& handled)
{
	assert(get_window());
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	sound::play_UI_sound(settings::sound_button_click);

	// If a button has a retval do the default handling.
	dialogs::drop_down_menu droplist(this, values_, selected_, keep_open_);

	// Whether we want the DDM to retain the selected item if previously opened
	droplist.set_start_selected(persistent_);

	if(droplist.show()) {
		const int selected = droplist.selected_item();

		// Safety check. If the user clicks a selection in the dropdown and moves their mouse away too
		// quickly, selected_ could be set to -1. This returns in that case, preventing crashes.
		if(selected < 0) {
			return;
		}

		set_selected(selected, true);
	}

	handled = true;
}

void options_button::signal_handler_sdl_wheel_up(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	// TODO: should values wrap?
	if(selected_ > 0) {
		set_selected(selected_ - 1);
	}

	handled = true;
}

void options_button::signal_handler_sdl_wheel_down(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	// TODO: should values wrap?
	if(selected_ < values_.size() - 1) {
		set_selected(selected_ + 1);
	}

	handled = true;
}

void options_button::set_values(const std::vector<::config>& values, unsigned selected)
{
	assert(selected < values.size());
	assert(selected_ < values_.size());

	if(values[selected]["label"] != values_[selected_]["label"]) {
		queue_redraw();
	}

	values_ = values;
	selected_ = selected;

	if(persistent_) {
		set_label(values_[selected_]["label"]);
	}
}

void options_button::set_selected(unsigned selected, bool fire_event)
{
	assert(selected < values_.size());
	assert(selected_ < values_.size());

	if(selected != selected_) {
		queue_redraw();
	}

	selected_ = selected;

	if(persistent_) {
		set_label(values_[selected_]["label"]);
	}

	if (fire_event) {
		fire(event::NOTIFY_MODIFIED, *this, nullptr);
	}
}

// }---------- DEFINITION ---------{

options_button_definition::options_button_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing options_button " << id;

	load_resolutions<resolution>(cfg);
}

options_button_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
{
	// Note the order should be the same as the enum state_t in options_button.hpp.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", missing_mandatory_wml_tag("options_button][resolution", "state_enabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_disabled", missing_mandatory_wml_tag("options_button][resolution", "state_disabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_pressed", missing_mandatory_wml_tag("options_button][resolution", "state_pressed")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_focused", missing_mandatory_wml_tag("options_button][resolution", "state_focused")));
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_options_button::builder_options_button(const config& cfg)
	: builder_styled_widget(cfg)
	, options_()
{
	for(const auto& option : cfg.child_range("option")) {
		options_.push_back(option);
	}
}

std::unique_ptr<widget> builder_options_button::build() const
{
	auto widget = std::make_unique<options_button>(*this, options_button::type());

	if(!options_.empty()) {
		widget->set_values(options_);
	}

	DBG_GUI_G << "Window builder: placed options_button '" << id
	          << "' with definition '" << definition << "'.";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
