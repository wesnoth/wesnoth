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

#include "gui/widgets/slider.hpp"

#include "formatter.hpp"
#include "gettext.hpp"
#include "gui/core/log.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/core/log.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "sdl/rect.hpp"
#include "sound.hpp"
#include "utils/math.hpp"
#include "gettext.hpp"
#include "wml_exception.hpp"

#include "utils/functional.hpp"

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{
// ------------ WIDGET -----------{

REGISTER_WIDGET(slider)

slider::slider(const implementation::builder_slider& builder)
	: slider_base(builder, get_control_type())
	, best_slider_length_(0)
	, minimum_value_(0)
	, step_size_(1)
	, minimum_value_label_()
	, maximum_value_label_()
	, value_label_generator_()
	, current_item_mouse_position_(0, 0)
{
	connect_signal<event::SDL_KEY_DOWN>(std::bind(&slider::signal_handler_sdl_key_down, this, _2, _3, _5));

	// connect_signal<event::LEFT_BUTTON_DOWN>(
	//		std::bind(&slider::signal_handler_left_button_down, this, _2, _3));

	connect_signal<event::LEFT_BUTTON_UP>(std::bind(&slider::signal_handler_left_button_up, this, _2, _3));
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

	DBG_GUI_L << LOG_HEADER << " best_slider_length " << best_slider_length_ << " result " << result << ".\n";
	return result;
}

void slider::set_value(int value)
{
	value = utils::clamp(value, minimum_value_, get_maximum_value());
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
			<< " step_size=" << step_size_ << "\n";
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
	SDL_Rect positioner_rect =
		sdl::create_rect(get_positioner_offset(), 0, get_positioner_length(), get_height());

	// Note we assume the positioner is over the entire height of the widget.
	return sdl::point_in_rect(coordinate, positioner_rect);
}

int slider::on_bar(const point& coordinate) const
{
	const unsigned x = static_cast<size_t>(coordinate.x);
	const unsigned y = static_cast<size_t>(coordinate.y);

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
	DBG_GUI_E << LOG_HEADER << '\n';

	handled = true;

	scroll(slider_base::ITEM_BACKWARDS);
}

void slider::handle_key_increase(bool& handled)
{
	DBG_GUI_E << LOG_HEADER << '\n';

	handled = true;

	scroll(slider_base::ITEM_FORWARD);
}

void slider::signal_handler_sdl_key_down(const event::ui_event event, bool& handled, const SDL_Keycode key)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

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
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	update_current_item_mouse_position();

	handled = true;
}
#endif

void slider::signal_handler_left_button_up(const event::ui_event event, bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	get_window()->keyboard_capture(this);

	handled = true;
}

static t_string default_value_label_generator(const std::vector<t_string>& value_labels, int item_position, int max)
{
	assert(int(value_labels.size()) == max);
	assert(item_position < max && item_position >= 0);
	return value_labels[item_position];
}

void slider::set_value_labels(const std::vector<t_string>& value_labels)
{
	// Don't use std::ref because we want to store value_labels in the closure.
	set_value_labels(std::bind(&default_value_label_generator, value_labels, _1, _2));
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

	step_size_ = gcd(diff, step_size_);
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

	step_size_ = gcd(range_diff, step_size);
	slider_set_item_last(range_diff / step_size_);
	set_value(old_value);

	assert(old_min_value == get_minimum_value());
	assert(old_max_value == get_maximum_value());
}

// }---------- DEFINITION ---------{

slider_definition::slider_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing slider " << id << '\n';

	load_resolutions<resolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_slider
 *
 * == Slider ==
 *
 * @macro = slider_description
 *
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="slider_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super="generic/widget_definition/resolution"}
 * @begin{table}{config}
 *     minimum_positioner_length & unsigned & &
 *                                     The minimum size the positioner is
 *                                     allowed to be. The engine needs to know
 *                                     this in order to calculate the best size
 *                                     for the positioner. $
 *     maximum_positioner_length & unsigned & 0 &
 *                                     The maximum size the positioner is
 *                                     allowed to be. If minimum and maximum are
 *                                     the same value the positioner is fixed
 *                                     size. If the maximum is 0 (and the
 *                                     minimum not) there's no maximum. $
 *     left_offset & unsigned & 0 &    The number of pixels at the left side
 *                                     which can't be used by the positioner. $
 *     right_offset & unsigned & 0 &   The number of pixels at the right side
 *                                     which can't be used by the positioner. $
 * @end{table}
 *
 * The following states exist:
 * * state_enabled, the slider is enabled.
 * * state_disabled, the slider is disabled.
 * * state_pressed, the left mouse button is down on the positioner of the
 *   slider.
 * * state_focused, the mouse is over the positioner of the slider.
 * @begin{tag}{name="state_enabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_enabled"}
 * @begin{tag}{name="state_disabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_disabled"}
 * @begin{tag}{name="state_pressed"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_pressed"}
 * @begin{tag}{name="state_focused"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_focused"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="slider_definition"}
 * @end{parent}{name="gui/"}
 */
slider_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
	, positioner_length(cfg["minimum_positioner_length"])
	, left_offset(cfg["left_offset"])
	, right_offset(cfg["right_offset"])
{
	VALIDATE(positioner_length, missing_mandatory_wml_key("resolution", "minimum_positioner_length"));

	// Note the order should be the same as the enum state_t is slider.hpp.
	state.emplace_back(cfg.child("state_enabled"));
	state.emplace_back(cfg.child("state_disabled"));
	state.emplace_back(cfg.child("state_pressed"));
	state.emplace_back(cfg.child("state_focused"));
}

// }---------- BUILDER -----------{

/*WIKI_MACRO
 * @begin{macro}{slider_description}
 * A slider is a styled_widget that can select a value by moving a grip on a groove.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 3_slider
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="slider"}{min="0"}{max="-1"}{super="generic/widget_instance"}
 * == Slider ==
 *
 * @macro = slider_description
 *
 * @begin{table}{config}
 *     best_slider_length & unsigned & 0 &
 *                                    The best length for the sliding part. $
 *     minimum_value & int & 0 &        The minimum value the slider can have. $
 *     maximum_value & int & 0 &        The maximum value the slider can have. $
 *
 *     step_size & unsigned & 0 &       The number of items the slider's value
 *                                    increases with one step. $
 *     value & int & 0 &                The value of the slider. $
 *
 *     minimum_value_label & t_string & "" &
 *                                    If the minimum value is chosen there
 *                                    might be the need for a special value
 *                                    (eg off). When this key has a value
 *                                    that value will be shown if the minimum
 *                                    is selected. $
 *     maximum_value_label & t_string & "" &
 *                                    If the maximum value is chosen there
 *                                    might be the need for a special value
 *                                    (eg unlimited)). When this key has a
 *                                    value that value will be shown if the
 *                                    maximum is selected. $
 *     value_labels & [] &              It might be the labels need to be shown
 *                                    are not a linear number sequence eg
 *                                    (0.5, 1, 2, 4) in that case for all
 *                                    items this section can be filled with
 *                                    the values, which should be the same
 *                                    number of items as the items in the
 *                                    slider. NOTE if this option is used,
 *                                    'minimum_value_label' and
 *                                    'maximum_value_label' are ignored. $
 * @end{table}
 * @end{tag}{name="slider"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

namespace implementation
{
builder_slider::builder_slider(const config& cfg)
	: implementation::builder_styled_widget(cfg)
	, best_slider_length_(cfg["best_slider_length"])
	, minimum_value_(cfg["minimum_value"])
	, maximum_value_(cfg["maximum_value"])
	, step_size_(cfg["step_size"].to_int(1))
	, value_(cfg["value"])
	, minimum_value_label_(cfg["minimum_value_label"].t_str())
	, maximum_value_label_(cfg["maximum_value_label"].t_str())
	, value_labels_()
{
	const config& labels = cfg.child("value_labels");
	if(!labels) {
		return;
	}

	for(const auto& label : labels.child_range("value")) {
		value_labels_.push_back(label["label"]);
	}
}

widget* builder_slider::build() const
{
	slider* widget = new slider(*this);

	widget->set_best_slider_length(best_slider_length_);
	widget->set_value_range(minimum_value_, maximum_value_);
	widget->set_step_size(step_size_);
	widget->set_value(value_);

	widget->finalize_setup();

	if(!value_labels_.empty()) {
		VALIDATE(value_labels_.size() == static_cast<size_t>(widget->get_item_count()),
				 _("The number of value_labels and values don't match."));

		widget->set_value_labels(value_labels_);

	} else {
		widget->set_minimum_value_label(minimum_value_label_);
		widget->set_maximum_value_label(maximum_value_label_);
	}

	DBG_GUI_G << "Window builder: placed slider '" << id << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
