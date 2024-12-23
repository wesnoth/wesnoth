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

#include "gui/widgets/multimenu_button.hpp"

#include "gui/core/log.hpp"
#include "gui/core/widget_definition.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "sound.hpp"

#include "formula/string_utils.hpp"
#include <functional>
#include "gettext.hpp"
#include "wml_exception.hpp"

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(multimenu_button)

multimenu_button::multimenu_button(const implementation::builder_multimenu_button& builder)
	: styled_widget(builder, type())
	, state_(ENABLED)
	, max_shown_(1)
	, values_()
	, toggle_states_()
	, droplist_(nullptr)
{
	values_.emplace_back("label", this->get_label());

	connect_signal<event::MOUSE_ENTER>(
		std::bind(&multimenu_button::signal_handler_mouse_enter, this, std::placeholders::_2, std::placeholders::_3));
	connect_signal<event::MOUSE_LEAVE>(
		std::bind(&multimenu_button::signal_handler_mouse_leave, this, std::placeholders::_2, std::placeholders::_3));

	connect_signal<event::LEFT_BUTTON_DOWN>(
		std::bind(&multimenu_button::signal_handler_left_button_down, this, std::placeholders::_2, std::placeholders::_3));
	connect_signal<event::LEFT_BUTTON_UP>(
		std::bind(&multimenu_button::signal_handler_left_button_up, this, std::placeholders::_2, std::placeholders::_3));
	connect_signal<event::LEFT_BUTTON_CLICK>(
		std::bind(&multimenu_button::signal_handler_left_button_click, this, std::placeholders::_2, std::placeholders::_3));

	// TODO: might need to position this differently in the queue if it's called after
	// dialog-specific callbacks.
	connect_signal<event::NOTIFY_MODIFIED>(
		std::bind(&multimenu_button::signal_handler_notify_changed, this));
}

void multimenu_button::set_active(const bool active)
{
	if(get_active() != active) {
		set_state(active ? ENABLED : DISABLED);
	}
}

bool multimenu_button::get_active() const
{
	return state_ != DISABLED;
}

unsigned multimenu_button::get_state() const
{
	return state_;
}

void multimenu_button::set_state(const state_t state)
{
	if(state != state_) {
		state_ = state;
		queue_redraw();
	}
}

void multimenu_button::signal_handler_mouse_enter(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	set_state(FOCUSED);
	handled = true;
}

void multimenu_button::signal_handler_mouse_leave(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	set_state(ENABLED);
	handled = true;
}

void multimenu_button::signal_handler_left_button_down(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	window* window = get_window();
	if(window) {
		window->mouse_capture();
	}

	set_state(PRESSED);
	handled = true;
}

void multimenu_button::signal_handler_left_button_up(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	set_state(FOCUSED);
	handled = true;
}

void multimenu_button::signal_handler_left_button_click(const event::ui_event event, bool& handled)
{
	assert(get_window());
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	sound::play_UI_sound(settings::sound_button_click);

	// If a button has a retval do the default handling.
	dialogs::drop_down_menu droplist(this, values_, -1, true);

	droplist_ = &droplist;
	droplist.show();
	droplist_ = nullptr;

	/* In order to allow toggle button states to be specified by various dialogs in the values config, we write the state
	 * bools to the values_ config here, but only if a checkbox= key was already provided. The value of the checkbox= key
	 * is handled by the drop_down_menu widget.
	 *
	 * Passing the dynamic_bitset directly to the drop_down_menu ctor would mean bool values would need to be passed to this
	 * class independently of the values config by dialogs that use this widget. However, the bool states are also saved
	 * in a dynamic_bitset class member which can be fetched for other uses if necessary.
	 */
	update_config_from_toggle_states();

	handled = true;
}

void multimenu_button::update_label()
{
	std::vector<t_string> selected;
	for(std::size_t i = 0; i < toggle_states_.size() && i < values_.size(); i++) {
		if(!toggle_states_[i]) {
			continue;
		}

		selected.push_back(values_[i]["label"]);
	}

	if(selected.size() == values_.size()) {
		set_label(_("multimenu^All Selected"));
	} else {
		if(selected.size() > max_shown_) {
			const unsigned excess = selected.size() - max_shown_;
			selected.resize(max_shown_ + 1);
			// TRANSLATORS: In a drop-down menu that's a list of toggle-boxes, this becomes part
			// of the text on the button when many of the boxes are selected. The text becomes
			// "x, y and 1 other", "x, y and 2 others", etc.
			selected.back() = VNGETTEXT("multimenu^$excess other", "$excess others", excess, {{"excess", std::to_string(excess)}});
		}
		set_label(utils::format_conjunct_list(_("multimenu^None Selected"), selected));
	}
}

void multimenu_button::update_config_from_toggle_states()
{
	for(unsigned i = 0; i < values_.size(); i++) {
		::config& c = values_[i];

		c["checkbox"] = toggle_states_[i];
	}
}

void multimenu_button::reset_toggle_states()
{
	toggle_states_.reset();
	update_config_from_toggle_states();
	update_label();
}

void multimenu_button::signal_handler_notify_changed()
{
	assert(droplist_ != nullptr);

	toggle_states_ = droplist_->get_toggle_states();
	update_label();
}

void multimenu_button::select_option(const unsigned option, const bool selected)
{
	assert(option < values_.size());
	toggle_states_[option] = selected;
	update_config_from_toggle_states();
	update_label();
}

void multimenu_button::select_options(const boost::dynamic_bitset<>& states)
{
	assert(states.size() == values_.size());
	toggle_states_ = states;
	update_config_from_toggle_states();
	update_label();
}

void multimenu_button::set_values(const std::vector<::config>& values)
{
	queue_redraw(); // TODO: draw_manager - does this need a relayout first?

	values_ = values;
	toggle_states_.resize(values_.size(), false);
	toggle_states_.reset();

	set_label(_("multimenu^None Selected"));
}

// }---------- DEFINITION ---------{

multimenu_button_definition::multimenu_button_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing multimenu_button " << id;

	load_resolutions<resolution>(cfg);
}

multimenu_button_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
{
	// Note the order should be the same as the enum state_t in multimenu_button.hpp.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", missing_mandatory_wml_tag("multimenu_button_definition][resolution", "state_enabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_disabled", missing_mandatory_wml_tag("multimenu_button_definition][resolution", "state_disabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_pressed", missing_mandatory_wml_tag("multimenu_button_definition][resolution", "state_pressed")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_focused", missing_mandatory_wml_tag("multimenu_button_definition][resolution", "state_focused")));
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_multimenu_button::builder_multimenu_button(const config& cfg)
	: builder_styled_widget(cfg)
	, max_shown_(cfg["maximum_shown"].to_unsigned(1))
	, options_()
{
	for(const auto& option : cfg.child_range("option")) {
		options_.push_back(option);
	}
}

std::unique_ptr<widget> builder_multimenu_button::build() const
{
	auto widget = std::make_unique<multimenu_button>(*this);

	widget->set_max_shown(max_shown_);
	if(!options_.empty()) {
		widget->set_values(options_);
	}

	DBG_GUI_G << "Window builder: placed multimenu_button '" << id
	          << "' with definition '" << definition << "'.";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
