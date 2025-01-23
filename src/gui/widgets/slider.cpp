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

#include "gui/widgets/slider.hpp"

#include "formatter.hpp"
#include "gettext.hpp"
#include "gui/core/log.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "sdl/rect.hpp"
#include "sound.hpp"
#include "gettext.hpp"
#include "utils/math.hpp"
#include "wml_exception.hpp"

#include <functional>
#include <numeric>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{
// ------------ WIDGET -----------{

REGISTER_WIDGET(slider)

slider::slider(const implementation::builder_slider& builder)
	: slider_base(builder, type())
	, best_slider_length_(builder.best_slider_length)
	, minimum_value_(0)
	, step_size_(1)
	, minimum_value_label_()
	, maximum_value_label_()
	, value_label_generator_()
	, current_item_mouse_position_(0, 0)
{
	connect_signal<event::SDL_KEY_DOWN>(std::bind(&slider::signal_handler_sdl_key_down, this, std::placeholders::_2, std::placeholders::_3, std::placeholders::_5));

	// connect_signal<event::LEFT_BUTTON_DOWN>(
	//		std::bind(&slider::signal_handler_left_button_down, this, std::placeholders::_2, std::placeholders::_3));

	connect_signal<event::LEFT_BUTTON_UP>(std::bind(&slider::signal_handler_left_button_up, this, std::placeholders::_2, std::placeholders::_3));
}

point slider::calculate_best_size() const
{
	log_scope2(log_gui_layout, LOG_SCOPE_HEADER);

	// Inherited.
	point result = styled_widget::calculate_best_size();

	if(best_slider_length_ != 0) {
		// Override length.
		const auto conf = cast_config_to<slider_definition>();
		assert(conf);

		result.x = conf->left_offset + best_slider_length_ + conf->right_offset;
	}

	DBG_GUI_L << LOG_HEADER << " best_slider_length " << best_slider_length_ << " result " << result << ".";
	return result;
}

void slider::set_value(int value)
{
	value = std::clamp(value, minimum_value_, get_maximum_value());
	int old_value = get_value();

	if(value == old_value) {
		return;
	}

	set_slider_position(rounded_division(value - minimum_value_, step_size_));

	if(std::abs(get_value() - value) > (step_size_ / 2)) {
		ERR_GUI_G << "slider::set_value error:"
			<< " old_value=" << old_value
			<< " new_value=" << get_value()
			<< " desired_value=" << value
			<< " minimum_value=" << minimum_value_
			<< " maximum_value=" << get_maximum_value()
			<< " step_size=" << step_size_;
		assert(false);
	}

	fire(event::NOTIFY_MODIFIED, *this, nullptr);
}

t_string slider::get_value_label() const
{
	if(value_label_generator_) {
		return value_label_generator_(get_slider_position(), get_item_count());
	} else if(!minimum_value_label_.empty() && get_value() == get_minimum_value()) {
		return minimum_value_label_;
	} else if(!maximum_value_label_.empty() && get_value() == get_maximum_value()) {
		return maximum_value_label_;
	}

	return t_string(formatter() << get_value());
}

void slider::child_callback_positioner_moved()
{
	sound::play_UI_sound(settings::sound_slider_adjust);
}

int slider::positioner_length() const
{
	const auto conf = cast_config_to<slider_definition>();
	assert(conf);
	return conf->positioner_length;
}

unsigned slider::offset_before() const
{
	const auto conf = cast_config_to<slider_definition>();
	assert(conf);
	return conf->left_offset;
}

unsigned slider::offset_after() const
{
	const auto conf = cast_config_to<slider_definition>();
	assert(conf);
	return conf->right_offset;
}

bool slider::on_positioner(const point& coordinate) const
{
	rect positioner_rect(
		get_positioner_offset(), 0, get_positioner_length(), get_height()
	);

	// Note we assume the positioner is over the entire height of the widget.
	return positioner_rect.contains(coordinate);
}

int slider::on_bar(const point& coordinate) const
{
	const unsigned x = static_cast<std::size_t>(coordinate.x);
	const unsigned y = static_cast<std::size_t>(coordinate.y);

	// Not on the widget, leave.
	if(x > get_width() || y > get_height()) {
		return 0;
	}

	// we also assume the bar is over the entire height of the widget.
	if(x < get_positioner_offset()) {
		return -1;
	} else if(x > get_positioner_offset() + get_positioner_length()) {
		return 1;
	}

	return 0;
}

void slider::update_canvas()
{
	// Inherited.
	slider_base::update_canvas();

	for(auto& tmp : get_canvases()) {
		tmp.set_variable("text", wfl::variant(get_value_label()));
	}
}

void slider::handle_key_decrease(bool& handled)
{
	DBG_GUI_E << LOG_HEADER;

	handled = true;

	scroll(slider_base::ITEM_BACKWARDS);
}

void slider::handle_key_increase(bool& handled)
{
	DBG_GUI_E << LOG_HEADER;

	handled = true;

	scroll(slider_base::ITEM_FORWARD);
}

void slider::signal_handler_sdl_key_down(const event::ui_event event, bool& handled, const SDL_Keycode key)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	if(key == SDLK_DOWN || key == SDLK_LEFT) {
		handle_key_decrease(handled);
	} else if(key == SDLK_UP || key == SDLK_RIGHT) {
		handle_key_increase(handled);
	} else {
		// Do nothing. Ignore other keys.
	}
}

#if 0
void slider::signal_handler_left_button_down(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	update_current_item_mouse_position();

	handled = true;
}
#endif

void slider::signal_handler_left_button_up(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	get_window()->keyboard_capture(this);

	handled = true;
}

static t_string default_value_label_generator(const std::vector<t_string>& value_labels, int item_position, int max)
{
	assert(static_cast<int>(value_labels.size()) == max);
	assert(item_position < max && item_position >= 0);
	return value_labels[item_position];
}

void slider::set_value_labels(const std::vector<t_string>& value_labels)
{
	// Don't use std::ref because we want to store value_labels in the closure.
	set_value_labels(std::bind(&default_value_label_generator, value_labels, std::placeholders::_1, std::placeholders::_2));
}


void slider::set_value_range(int min_value, int max_value)
{
	// Settng both at once instead of having multiple functions set_min(),
	// set_max() ... fixes an old problem where in cases like
	//   set_min(-10);set_min(-1);
	// min and max would tmporarily have invalid values where since the starting max value is 0;

	VALIDATE(min_value <= max_value, "invalid slider data");
	if (min_value == minimum_value_ && max_value == get_maximum_value()) {
		return;
	}

	int diff = max_value - min_value;
	int old_value = get_value();

	step_size_ = std::gcd(diff, step_size_);
	minimum_value_ = min_value;

	slider_set_item_last(diff / step_size_);
	set_value(old_value);

	assert(min_value == get_minimum_value());
	assert(max_value == get_maximum_value());

}

void slider::set_step_size(int step_size)
{
	const int old_min_value = get_minimum_value();
	const int old_max_value = get_maximum_value();

	const int range_diff = get_item_count() - 1;
	const int old_value = get_value();

	step_size_ = std::gcd(range_diff, step_size);

	slider_set_item_last(range_diff / step_size_);
	set_value(old_value);

	assert(old_min_value == get_minimum_value());
	assert(old_max_value == get_maximum_value());
}

// }---------- DEFINITION ---------{

slider_definition::slider_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing slider " << id;

	load_resolutions<resolution>(cfg);
}

slider_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
	, positioner_length(cfg["minimum_positioner_length"].to_unsigned())
	, left_offset(cfg["left_offset"].to_unsigned())
	, right_offset(cfg["right_offset"].to_unsigned())
{
	VALIDATE(positioner_length, missing_mandatory_wml_key("resolution", "minimum_positioner_length"));

	// Note the order should be the same as the enum state_t is slider.hpp.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", missing_mandatory_wml_tag("slider_definition][resolution", "state_enabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_disabled", missing_mandatory_wml_tag("slider_definition][resolution", "state_disabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_pressed", missing_mandatory_wml_tag("slider_definition][resolution", "state_pressed")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_focused", missing_mandatory_wml_tag("slider_definition][resolution", "state_focused")));
}

// }---------- BUILDER -----------{

namespace implementation
{
builder_slider::builder_slider(const config& cfg)
	: implementation::builder_styled_widget(cfg)
	, best_slider_length(cfg["best_slider_length"].to_unsigned())
	, minimum_value_(cfg["minimum_value"].to_int())
	, maximum_value_(cfg["maximum_value"].to_int())
	, step_size_(cfg["step_size"].to_int(1))
	, value_(cfg["value"].to_int())
	, minimum_value_label_(cfg["minimum_value_label"].t_str())
	, maximum_value_label_(cfg["maximum_value_label"].t_str())
	, value_labels_()
{
	auto labels = cfg.optional_child("value_labels");
	if(!labels) {
		return;
	}

	for(const auto& label : labels->child_range("value")) {
		value_labels_.push_back(label["label"]);
	}
}

std::unique_ptr<widget> builder_slider::build() const
{
	auto widget = std::make_unique<slider>(*this);

	widget->set_value_range(minimum_value_, maximum_value_);
	widget->set_step_size(step_size_);
	widget->set_value(value_);

	widget->finalize_setup();

	if(!value_labels_.empty()) {
		VALIDATE(value_labels_.size() == static_cast<std::size_t>(widget->get_item_count()),
				 _("The number of value_labels and values don’t match."));

		widget->set_value_labels(value_labels_);

	} else {
		widget->set_minimum_value_label(minimum_value_label_);
		widget->set_maximum_value_label(maximum_value_label_);
	}

	DBG_GUI_G << "Window builder: placed slider '" << id << "' with definition '" << definition << "'.";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
