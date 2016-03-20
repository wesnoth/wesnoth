/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
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
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/detail/register.hpp"
#include "gui/widgets/settings.hpp"
#include "sound.hpp"
#include "utils/foreach.hpp"
#include "gettext.hpp"
#include "wml_exception.hpp"

#include <boost/bind.hpp>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(slider)

tslider::tslider()
	: tscrollbar_()
	, best_slider_length_(0)
	, minimum_value_(0)
	, minimum_value_label_()
	, maximum_value_label_()
	, value_labels_()
{
	connect_signal<event::SDL_KEY_DOWN>(boost::bind(
			&tslider::signal_handler_sdl_key_down, this, _2, _3, _5));
	connect_signal<event::LEFT_BUTTON_UP>(
			boost::bind(&tslider::signal_handler_left_button_up, this, _2, _3));
}

tpoint tslider::calculate_best_size() const
{
	log_scope2(log_gui_layout, LOG_SCOPE_HEADER);

	// Inherited.
	tpoint result = tcontrol::calculate_best_size();
	if(best_slider_length_ != 0) {

		// Override length.
		boost::intrusive_ptr<const tslider_definition::tresolution> conf
				= boost::dynamic_pointer_cast<const tslider_definition::
													  tresolution>(config());

		assert(conf);

		result.x = conf->left_offset + best_slider_length_ + conf->right_offset;
	}

	DBG_GUI_L << LOG_HEADER << " best_slider_length " << best_slider_length_
			  << " result " << result << ".\n";
	return result;
}

void tslider::set_value(const int value)
{
	if(value == get_value()) {
		return;
	}

	if(value < minimum_value_) {
		set_value(minimum_value_);
	} else if(value > get_maximum_value()) {
		set_value(get_maximum_value());
	} else {
		set_item_position(value - minimum_value_);
	}
}

void tslider::set_minimum_value(const int minimum_value)
{
	if(minimum_value == minimum_value_) {
		return;
	}

	/** @todo maybe make it a VALIDATE. */
	assert(minimum_value <= get_maximum_value());

	const int value = get_value();
	const int maximum_value = get_maximum_value();
	minimum_value_ = minimum_value;

	// The number of items needs to include the begin and end so distance step
	// size.
	set_item_count(maximum_value - minimum_value_ + get_step_size());

	if(value < minimum_value_) {
		set_item_position(0);
	} else {
		set_item_position(minimum_value_ + value);
	}
}

void tslider::set_maximum_value(const int maximum_value)
{
	if(maximum_value == get_maximum_value()) {
		return;
	}

	/** @todo maybe make it a VALIDATE. */
	assert(minimum_value_ <= maximum_value);

	const int value = get_value();

	// The number of items needs to include the begin and end so distance + step
	// size.
	set_item_count(maximum_value - minimum_value_ + get_step_size());

	if(value > maximum_value) {
		set_item_position(get_maximum_value());
	} else {
		set_item_position(minimum_value_ + value);
	}
}

t_string tslider::get_value_label() const
{
	if(value_labels_) {
		return value_labels_(get_item_position(), get_item_count());
	} else if(!minimum_value_label_.empty() && get_value()
											   == get_minimum_value()) {
		return minimum_value_label_;
	} else if(!maximum_value_label_.empty() && get_value()
											   == get_maximum_value()) {
		return maximum_value_label_;
	}

	return t_string((formatter() << get_value()).str());
}

void tslider::child_callback_positioner_moved()
{
	sound::play_UI_sound(settings::sound_slider_adjust);
}

unsigned tslider::minimum_positioner_length() const
{
	boost::intrusive_ptr<const tslider_definition::tresolution>
	conf = boost::dynamic_pointer_cast<const tslider_definition::tresolution>(
			config());
	assert(conf);
	return conf->minimum_positioner_length;
}

unsigned tslider::maximum_positioner_length() const
{
	boost::intrusive_ptr<const tslider_definition::tresolution>
	conf = boost::dynamic_pointer_cast<const tslider_definition::tresolution>(
			config());
	assert(conf);
	return conf->maximum_positioner_length;
}

unsigned tslider::offset_before() const
{
	boost::intrusive_ptr<const tslider_definition::tresolution>
	conf = boost::dynamic_pointer_cast<const tslider_definition::tresolution>(
			config());
	assert(conf);
	return conf->left_offset;
}

unsigned tslider::offset_after() const
{
	boost::intrusive_ptr<const tslider_definition::tresolution>
	conf = boost::dynamic_pointer_cast<const tslider_definition::tresolution>(
			config());
	assert(conf);
	return conf->right_offset;
}

bool tslider::on_positioner(const tpoint& coordinate) const
{
	// Note we assume the positioner is over the entire height of the widget.
	return coordinate.x >= static_cast<int>(get_positioner_offset())
		   && coordinate.x < static_cast<int>(get_positioner_offset()
											  + get_positioner_length())
		   && coordinate.y > 0 && coordinate.y < static_cast<int>(get_height());
}

int tslider::on_bar(const tpoint& coordinate) const
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

bool tslider::in_orthogonal_range(const tpoint& coordinate) const
{
	return static_cast<size_t>(coordinate.x) < (get_width() - offset_after());
}

void tslider::update_canvas()
{

	// Inherited.
	tscrollbar_::update_canvas();

	FOREACH(AUTO & tmp, canvas())
	{
		tmp.set_variable("text", variant(get_value_label()));
	}
}

const std::string& tslider::get_control_type() const
{
	static const std::string type = "slider";
	return type;
}

void tslider::handle_key_decrease(bool& handled)
{
	DBG_GUI_E << LOG_HEADER << '\n';

	handled = true;

	scroll(tscrollbar_::ITEM_BACKWARDS);
}

void tslider::handle_key_increase(bool& handled)
{
	DBG_GUI_E << LOG_HEADER << '\n';

	handled = true;

	scroll(tscrollbar_::ITEM_FORWARD);
}

void tslider::signal_handler_sdl_key_down(const event::tevent event,
										  bool& handled,
										  const SDLKey key)
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

void tslider::signal_handler_left_button_up(const event::tevent event,
											bool& handled)
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

void tslider::set_value_labels(const std::vector<t_string>& value_labels)
{
	//dont use boost::ref becasue we want to store value_labels in the cloasure.
	set_value_labels(boost::bind(&default_value_label_generator, value_labels, _1, _2));
}

// }---------- DEFINITION ---------{

tslider_definition::tslider_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing slider " << id << '\n';

	load_resolutions<tresolution>(cfg);
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
tslider_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
	, minimum_positioner_length(cfg["minimum_positioner_length"])
	, maximum_positioner_length(cfg["maximum_positioner_length"])
	, left_offset(cfg["left_offset"])
	, right_offset(cfg["right_offset"])
{
	VALIDATE(minimum_positioner_length,
			 missing_mandatory_wml_key("resolution",
									   "minimum_positioner_length"));

	// Note the order should be the same as the enum tstate is slider.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));
	state.push_back(tstate_definition(cfg.child("state_pressed")));
	state.push_back(tstate_definition(cfg.child("state_focused")));
}

// }---------- BUILDER -----------{

/*WIKI_MACRO
 * @begin{macro}{slider_description}
 * A slider is a control that can select a value by moving a grip on a groove.
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

tbuilder_slider::tbuilder_slider(const config& cfg)
	: implementation::tbuilder_control(cfg)
	, best_slider_length_(cfg["best_slider_length"])
	, minimum_value_(cfg["minimum_value"])
	, maximum_value_(cfg["maximum_value"])
	, step_size_(cfg["step_size"])
	, value_(cfg["value"])
	, minimum_value_label_(cfg["minimum_value_label"].t_str())
	, maximum_value_label_(cfg["maximum_value_label"].t_str())
	, value_labels_()
{
	const config& labels = cfg.child("value_labels");
	if(!labels) {
		return;
	}

	FOREACH(const AUTO & label, labels.child_range("value"))
	{
		value_labels_.push_back(label["label"]);
	}
}

twidget* tbuilder_slider::build() const
{
	tslider* widget = new tslider();

	init_control(widget);

	widget->set_best_slider_length(best_slider_length_);
	widget->set_maximum_value(maximum_value_);
	widget->set_minimum_value(minimum_value_);
	widget->set_step_size(step_size_);
	widget->set_value(value_);

	if(!value_labels_.empty()) {
		VALIDATE(value_labels_.size() == widget->get_item_count(),
				 _("The number of value_labels and values don't match."));

		widget->set_value_labels(value_labels_);

	} else {
		widget->set_minimum_value_label(minimum_value_label_);
		widget->set_maximum_value_label(maximum_value_label_);
	}

	DBG_GUI_G << "Window builder: placed slider '" << id
			  << "' with definition '" << definition << "'.\n";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
