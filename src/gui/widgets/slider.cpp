/* $Id$ */
/*
   Copyright (C) 2008 - 2011 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/slider.hpp"

#include "foreach.hpp"
#include "formatter.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/widget_definition/slider.hpp"
#include "gui/widgets/settings.hpp"
#include "sound.hpp"

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2 {

static int distance(const int a, const int b)
{
	/**
	 * @todo once this works properly the assert can be removed and the code
	 * inlined.
	 */
	int result =  b - a;
	assert(result >= 0);
	return result;
}

tpoint tslider::calculate_best_size() const
{
	log_scope2(log_gui_layout, LOG_SCOPE_HEADER);

	// Inherited.
	tpoint result = tcontrol::calculate_best_size();
	if(best_slider_length_ != 0) {

		// Override length.
		boost::intrusive_ptr<const tslider_definition::tresolution> conf =
			boost::dynamic_pointer_cast<const tslider_definition::tresolution>
			(config());

		assert(conf);

		result.x = conf->left_offset + best_slider_length_ + conf->right_offset;
	}

	DBG_GUI_L << LOG_HEADER
		<< " best_slider_length " << best_slider_length_
		<< " result " << result
		<< ".\n";
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
		set_item_position(distance(minimum_value_, value));
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

	// The number of items needs to include the begin and end so distance + 1.
	set_item_count(distance(minimum_value_, maximum_value) + 1);

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

	// The number of items needs to include the begin and end so distance + 1.
	set_item_count(distance(minimum_value_, maximum_value) + 1);

	if(value > maximum_value) {
		set_item_position(get_maximum_value());
	} else {
		set_item_position(minimum_value_ + value);
	}
}

t_string tslider::get_value_label() const
{
	if(!value_labels_.empty()) {
		assert(value_labels_.size() == get_item_count());
		return value_labels_[get_item_position()];
	} else if(!minimum_value_label_.empty()
			&& get_value() == get_minimum_value()) {
		return minimum_value_label_;
	} else if(!maximum_value_label_.empty()
			&& get_value() == get_maximum_value()) {
		return maximum_value_label_;
	} else {
		return t_string((formatter() << get_value()).str());
	}
}

void tslider::child_callback_positioner_moved()
{
	sound::play_UI_sound(settings::sound_slider_adjust);
}

unsigned tslider::minimum_positioner_length() const
{
	boost::intrusive_ptr<const tslider_definition::tresolution> conf =
		boost::dynamic_pointer_cast<const tslider_definition::tresolution>(config());
	assert(conf);
	return conf->minimum_positioner_length;
}

unsigned tslider::maximum_positioner_length() const
{
	boost::intrusive_ptr<const tslider_definition::tresolution> conf =
		boost::dynamic_pointer_cast<const tslider_definition::tresolution>(config());
	assert(conf);
	return conf->maximum_positioner_length;
}

unsigned tslider::offset_before() const
{
	boost::intrusive_ptr<const tslider_definition::tresolution> conf =
		boost::dynamic_pointer_cast<const tslider_definition::tresolution>(config());
	assert(conf);
	return conf->left_offset;
}

unsigned tslider::offset_after() const
{
	boost::intrusive_ptr<const tslider_definition::tresolution> conf =
		boost::dynamic_pointer_cast<const tslider_definition::tresolution>(config());
	assert(conf);
	return conf->right_offset;
}

bool tslider::on_positioner(const tpoint& coordinate) const
{
	// Note we assume the positioner is over the entire height of the widget.
	return coordinate.x >= static_cast<int>(get_positioner_offset())
		&& coordinate.x < static_cast<int>(get_positioner_offset() + get_positioner_length())
		&& coordinate.y > 0
		&& coordinate.y < static_cast<int>(get_height());
}

int tslider::on_bar(const tpoint& coordinate) const
{
	// Not on the widget, leave.
	if(static_cast<size_t>(coordinate.x) > get_width()
			|| static_cast<size_t>(coordinate.y) > get_height()) {
		return 0;
	}

	// we also assume the bar is over the entire height of the widget.
	if(static_cast<size_t>(coordinate.x) < get_positioner_offset()) {
		return -1;
	} else if(static_cast<size_t>(coordinate.x) >get_positioner_offset() + get_positioner_length()) {
		return 1;
	} else {
		return 0;
	}
}

void tslider::update_canvas()
{

	// Inherited.
	tscrollbar_::update_canvas();

	BOOST_FOREACH(tcanvas& tmp, canvas()) {
		tmp.set_variable("text", variant(get_value_label()));
	}
}

const std::string& tslider::get_control_type() const
{
	static const std::string type = "slider";
	return type;
}

} // namespace gui2

