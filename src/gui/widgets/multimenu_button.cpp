/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/multimenu_button.hpp"

#include "gui/core/log.hpp"
#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"
#include "gui/core/window_builder/helper.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "sound.hpp"

#include "formula/string_utils.hpp"
#include "utils/functional.hpp"
#include "gettext.hpp"

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(multimenu_button)

multimenu_button::multimenu_button(const implementation::builder_multimenu_button& builder)
	: styled_widget(builder, get_control_type())
	, state_(ENABLED)
	, retval_(0)
	, max_shown_(1)
	, values_()
	, toggle_states_()
	, droplist_(nullptr)
{
	values_.emplace_back(::config {"label", this->get_label()});

	connect_signal<event::MOUSE_ENTER>(
			std::bind(&multimenu_button::signal_handler_mouse_enter, this, _2, _3));
	connect_signal<event::MOUSE_LEAVE>(
			std::bind(&multimenu_button::signal_handler_mouse_leave, this, _2, _3));

	connect_signal<event::LEFT_BUTTON_DOWN>(std::bind(
			&multimenu_button::signal_handler_left_button_down, this, _2, _3));
	connect_signal<event::LEFT_BUTTON_UP>(
			std::bind(&multimenu_button::signal_handler_left_button_up, this, _2, _3));
	connect_signal<event::LEFT_BUTTON_CLICK>(std::bind(
			&multimenu_button::signal_handler_left_button_click, this, _2, _3));
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
		set_is_dirty(true);
	}
}

void multimenu_button::signal_handler_mouse_enter(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	set_state(FOCUSED);
	handled = true;
}

void multimenu_button::signal_handler_mouse_leave(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	set_state(ENABLED);
	handled = true;
}

void multimenu_button::signal_handler_left_button_down(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	window* window = get_window();
	if(window) {
		window->mouse_capture();
	}

	set_state(PRESSED);
	handled = true;
}

void multimenu_button::signal_handler_left_button_up(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	set_state(FOCUSED);
	handled = true;
}

void multimenu_button::signal_handler_left_button_click(const event::ui_event event, bool& handled)
{
	assert(get_window());
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	sound::play_UI_sound(settings::sound_button_click);

	// If a button has a retval do the default handling.
	dialogs::drop_down_menu droplist(this->get_rectangle(), this->values_, -1, this->get_use_markup(), true,
		std::bind(&multimenu_button::toggle_state_changed, this));

	droplist_ = &droplist;
	droplist.show();
	droplist_ = nullptr;

	if(retval_ != retval::NONE) {
		if(window* window = get_window()) {
			window->set_retval(retval_);
			return;
		}
	}

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
	for(size_t i = 0; i < toggle_states_.size() && i < values_.size(); i++) {
		if(!toggle_states_[i]) {
			continue;
		}

		selected.push_back(values_[i]["label"]);
	}

	if(selected.size() == values_.size()) {
		set_label(_("multimenu^All Selected"));
	} else {
		if(selected.size() > static_cast<size_t>(max_shown_)) {
			const int excess = selected.size() - max_shown_;
			selected.resize(max_shown_ + 1);
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

void multimenu_button::toggle_state_changed()
{
	assert(droplist_ != nullptr);

	toggle_states_ = droplist_->get_toggle_states();
	fire(event::NOTIFY_MODIFIED, *this, nullptr);
	update_label();
}

void multimenu_button::select_option(const unsigned option, const bool selected)
{
	assert(option < values_.size());

	if(option < toggle_states_.size()) {
		toggle_states_.resize(option + 1);
	}
	toggle_states_[option] = selected;
	update_config_from_toggle_states();
	update_label();
}

void multimenu_button::select_options(boost::dynamic_bitset<> states)
{
	assert(states.size() == values_.size());
	toggle_states_ = states;
	update_config_from_toggle_states();
	update_label();
}

void multimenu_button::set_values(const std::vector<::config>& values)
{
	set_is_dirty(true);

	values_ = values;
	toggle_states_.resize(values_.size(), false);
	toggle_states_.reset();

	set_label(_("multimenu^None Selected"));
}

// }---------- DEFINITION ---------{

multimenu_button_definition::multimenu_button_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing multimenu_button " << id << '\n';

	load_resolutions<resolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_multimenu_button
 *
 * == multimenu_button ==
 *
 * @macro = multimenu_button_description
 *
 * The following states exist:
 * * state_enabled, the multimenu_button is enabled.
 * * state_disabled, the multimenu_button is disabled.
 * * state_pressed, the left mouse multimenu_button is down.
 * * state_focused, the mouse is over the multimenu_button.
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="multimenu_button_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super="generic/widget_definition/resolution"}
 * @begin{tag}{name="state_enabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_enabled"}
 * @begin{tag}{name="state_disabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_disabled"}
 * @begin{tag}{name="state_pressed"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_pressed"}
 * @begin{tag}{name="state_focused"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_focused"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="multimenu_button_definition"}
 * @end{parent}{name="gui/"}
 */
multimenu_button_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
{
	// Note the order should be the same as the enum state_t in multimenu_button.hpp.
	state.emplace_back(cfg.child("state_enabled"));
	state.emplace_back(cfg.child("state_disabled"));
	state.emplace_back(cfg.child("state_pressed"));
	state.emplace_back(cfg.child("state_focused"));
}

// }---------- BUILDER -----------{

/*WIKI_MACRO
 * @begin{macro}{multimenu_button_description}
 *
 *        A multimenu_button is a styled_widget to choose an element from a list of elements.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_multimenu_button
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="multimenu_button"}{min=0}{max=-1}{super="generic/widget_instance"}
 * == multimenu_button ==
 *
 * @macro = multimenu_button_description
 *
 * Instance of a multimenu_button. When a multimenu_button has a return value it sets the
 * return value for the window. Normally this closes the window and returns
 * this value to the caller. The return value can either be defined by the
 * user or determined from the id of the multimenu_button. The return value has a
 * higher precedence as the one defined by the id. (Of course it's weird to
 * give a multimenu_button an id and then override its return value.)
 *
 * When the multimenu_button doesn't have a standard id, but you still want to use the
 * return value of that id, use return_value_id instead. This has a higher
 * precedence as return_value.
 *
 * List with the multimenu_button specific variables:
 * @begin{table}{config}
 *     return_value_id & string & "" &   The return value id. $
 *     return_value & int & 0 &          The return value. $
 *     maximum_shown & int & -1 &        The maximum number of currently selected values to list on the button. $
 *
 * @end{table}
 * @end{tag}{name="multimenu_button"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

namespace implementation
{

builder_multimenu_button::builder_multimenu_button(const config& cfg)
	: builder_styled_widget(cfg)
	, retval_id_(cfg["return_value_id"])
	, retval_(cfg["return_value"])
	, max_shown_(cfg["maximum_shown"])
	, options_()
{
	for(const auto& option : cfg.child_range("option")) {
		options_.push_back(option);
	}
}

widget* builder_multimenu_button::build() const
{
	multimenu_button* widget = new multimenu_button(*this);

	widget->set_retval(get_retval(retval_id_, retval_, id));
	widget->set_max_shown(max_shown_);
	if(!options_.empty()) {
		widget->set_values(options_);
	}

	DBG_GUI_G << "Window builder: placed multimenu_button '" << id
	          << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
