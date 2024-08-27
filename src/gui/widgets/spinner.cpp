/*
	Copyright (C) 2023 - 2024
	by babaissarkar(Subhraman Sarkar) <suvrax@gmail.com>
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

#include "gui/widgets/spinner.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/repeating_button.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/core/log.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/window.hpp"
#include "gettext.hpp"
#include "wml_exception.hpp"

#include <functional>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(spinner)

spinner::spinner(const implementation::builder_spinner& builder)
	: container_base(builder, type())
	, state_(ENABLED)
	, step_size_(1)
	, invalid_(false)
{
	connect_signal<event::LEFT_BUTTON_DOWN>(
		std::bind(&spinner::signal_handler_left_button_down, this, std::placeholders::_2),
		event::dispatcher::back_pre_child);
}

text_box* spinner::get_internal_text_box()
{
	return find_widget<text_box>(this, "_text", false, true);
}

void spinner::set_value(int val)
{
	if((minimum_value_) && (val < *minimum_value_)) {
		val = *minimum_value_;
	}
	if((maximum_value_) && (val > *maximum_value_)) {
		val = *maximum_value_;
	}

	text_box* edit_area = get_internal_text_box();
	if (edit_area != nullptr) {
		edit_area->set_value(std::to_string(val));
	}

	find_widget<repeating_button>(this, "_prev", false, true)->set_active((!minimum_value_) || (val > *minimum_value_));
	find_widget<repeating_button>(this, "_next", false, true)->set_active((!maximum_value_) || (val < *maximum_value_));
}

int spinner::get_value()
{
	/* Return 0 if invalid.
	 * TODO: give visual indication of wrong value
	 */
	int val;
	try {
		text_box* edit_area = get_internal_text_box();
		if (edit_area != nullptr) {
			val = stoi(edit_area->get_value());
			if((minimum_value_) && (val < *minimum_value_)) { val = *minimum_value_; }
			if((maximum_value_) && (val > *maximum_value_)) { val = *maximum_value_; }
			invalid_ = false;
		} else {
			val = 0;
			invalid_ = true;
		}
	} catch(std::invalid_argument const& /*ex*/) {
		val = 0;
		invalid_ = true;
	} catch(std::out_of_range const& /*ex*/) {
		val = 0;
		invalid_ = true;
	}

	return val;
}
void spinner::set_step_size(int step)
{
	VALIDATE(((!minimum_value_) || (!maximum_value_) ||
				(step <= *maximum_value_ - *minimum_value_)),
			"step size (" + std::to_string(step) +") must be <= the range (" + std::to_string(*maximum_value_ - *minimum_value_) +
			") allowed by min (" + std::to_string(*minimum_value_) + ") and max (" + std::to_string(*maximum_value_) + ").");
	step_size_ = step;
};

int spinner::get_step_size() { return step_size_; }

void spinner::set_minimum_value(utils::optional<int> min) {
	VALIDATE(((!min) || (!maximum_value_) || (step_size_ <= *maximum_value_ - *min)),
			"minimum_value (" + std::to_string(*min) + ") must be <= maximum_value (" + std::to_string(*maximum_value_) + ").");
	minimum_value_ = min;
	set_value(get_value());
}

utils::optional<int> spinner::get_minimum_value() {
	return minimum_value_;
}

void spinner::set_maximum_value(utils::optional<int> max) {
	VALIDATE(((!max) || (!minimum_value_) || (step_size_ <= *max - *minimum_value_)),
			"maximum_value (" + std::to_string(*max) + ") must be >= minimum_value (" + std::to_string(*minimum_value_) + ").");
	maximum_value_ = max;
	set_value(get_value());
}

utils::optional<int> spinner::get_maximum_value() {
	return maximum_value_;
}

void spinner::prev()
{
	const int value = get_value();
	if(std::numeric_limits<int>::min() + step_size_ < value) {
		if(minimum_value_) {
			set_value(std::max(*minimum_value_, value - step_size_));
		} else {
			set_value(value - step_size_);
		}
	} else {
		set_value(std::numeric_limits<int>::min());
	}
}

void spinner::next()
{
	const int value = get_value();
	if(std::numeric_limits<int>::max() - step_size_ > value) {
		if(maximum_value_) {
			set_value(std::min(*maximum_value_, value + step_size_));
		} else {
			set_value(value + step_size_);
		}
	} else {
		set_value(std::numeric_limits<int>::max());
	}
}

void spinner::finalize_setup()
{
	repeating_button* btn_prev = find_widget<repeating_button>(this, "_prev", false, true);
	repeating_button* btn_next = find_widget<repeating_button>(this, "_next", false, true);
	btn_prev->connect_signal_mouse_left_down(std::bind(&spinner::prev, this));
	btn_next->connect_signal_mouse_left_down(std::bind(&spinner::next, this));
}

void spinner::set_self_active(const bool active)
{
	state_ = active ? ENABLED : DISABLED;
}

bool spinner::get_active() const
{
	return state_ != DISABLED;
}

unsigned spinner::get_state() const
{
	return state_;
}

bool spinner::can_wrap() const
{
	return true;
}

void spinner::signal_handler_left_button_down(const event::ui_event event)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	get_window()->keyboard_capture(this);
}

// }---------- DEFINITION ---------{

spinner_definition::spinner_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing spinner " << id;

	load_resolutions<resolution>(cfg);
}

spinner_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg), grid(nullptr)
{
	// Note the order should be the same as the enum state_t is spinner.hpp.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", missing_mandatory_wml_tag("spinner", "state_enabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_disabled", missing_mandatory_wml_tag("spinner", "state_disabled")));

	auto child = VALIDATE_WML_CHILD(cfg, "grid", missing_mandatory_wml_tag("spinner", "grid"));
	grid = std::make_shared<builder_grid>(child);
}

// }---------- BUILDER -----------{

namespace implementation
{

	builder_spinner::builder_spinner(const config& cfg)
		: implementation::builder_styled_widget(cfg)
		  , step_size_(cfg["step_size"].to_int(1))
	{
		if(cfg.has_attribute("minimum_value")) {
			minimum_value_ = cfg["minimum_value"].to_int();
		}
		if(cfg.has_attribute("maximum_value")) {
			maximum_value_ = cfg["maximum_value"].to_int();
		}
		VALIDATE(((!minimum_value_) || (!maximum_value_) || (*minimum_value_ <= *maximum_value_)),
				"minimum_value (" + std::to_string(*minimum_value_) + ") must be <= maximum_value (" + std::to_string(*maximum_value_) + ").");

		if(cfg.has_attribute("value")) {
			value_ = cfg["value"].to_int();
		} else {
			if((minimum_value_) && (maximum_value_)) {
				value_ = ((*minimum_value_ + *maximum_value_) / 2);
			} else {
				value_ = 0;
			}
		}
		// value_ may not be within the min/max limits at this point,
		// build() will take care of that.
	}

	std::unique_ptr<widget> builder_spinner::build() const
	{
		auto widget = std::make_unique<spinner>(*this);

		const auto conf = widget->cast_config_to<spinner_definition>();
		assert(conf);

		widget->init_grid(*conf->grid);
		widget->finalize_setup();

		if(minimum_value_) { widget->set_minimum_value(minimum_value_); }
		if(maximum_value_) { widget->set_maximum_value(*maximum_value_); }

		widget->set_step_size(step_size_);

		if((minimum_value_) && (value_ < *minimum_value_)) {
			widget->set_value(*minimum_value_);
		} else if((maximum_value_) && (value_ > *maximum_value_)) {
			widget->set_value(*maximum_value_);
		} else {
			widget->set_value(value_);
		}

		DBG_GUI_G << "Window builder: placed spinner '" << id
			<< "' with definition '" << definition << "'.";

		return widget;
	}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
