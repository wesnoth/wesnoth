/* $Id$ */
/*
   copyright (C) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#include "gui/widgets/slider.hpp"

#include "foreach.hpp"
#include "formatter.hpp"
#include "log.hpp"

#include <cassert>


#define DBG_G LOG_STREAM_INDENT(debug, gui)
#define LOG_G LOG_STREAM_INDENT(info, gui)
#define WRN_G LOG_STREAM_INDENT(warn, gui)
#define ERR_G LOG_STREAM_INDENT(err, gui)

#define DBG_G_D LOG_STREAM_INDENT(debug, gui_draw)
#define LOG_G_D LOG_STREAM_INDENT(info, gui_draw)
#define WRN_G_D LOG_STREAM_INDENT(warn, gui_draw)
#define ERR_G_D LOG_STREAM_INDENT(err, gui_draw)

#define DBG_G_E LOG_STREAM_INDENT(debug, gui_event)
#define LOG_G_E LOG_STREAM_INDENT(info, gui_event)
#define WRN_G_E LOG_STREAM_INDENT(warn, gui_event)
#define ERR_G_E LOG_STREAM_INDENT(err, gui_event)

#define DBG_G_P LOG_STREAM_INDENT(debug, gui_parse)
#define LOG_G_P LOG_STREAM_INDENT(info, gui_parse)
#define WRN_G_P LOG_STREAM_INDENT(warn, gui_parse)
#define ERR_G_P LOG_STREAM_INDENT(err, gui_parse)


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
	assert(minimum_value < get_maximum_value());

	const int value = get_value();
	const int maximum_value = get_maximum_value();
	minimum_value_ = minimum_value;

	set_item_count(distance(minimum_value_, maximum_value));

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
	assert(minimum_value_ < maximum_value);

	const int value = get_value();

	set_item_count(distance(minimum_value_, maximum_value));

	if(value > maximum_value) {
		set_item_position(get_maximum_value());
	} else {
		set_item_position(minimum_value_ + value);
	}
}

unsigned tslider::minimum_positioner_length() const
{ 
	const tslider_definition::tresolution* conf = 
		dynamic_cast<const tslider_definition::tresolution*>(config());
	assert(conf); 
	return conf->minimum_positioner_length; 
}

unsigned tslider::maximum_positioner_length() const
{
	const tslider_definition::tresolution* conf = 
		dynamic_cast<const tslider_definition::tresolution*>(config());
	assert(conf); 
	return conf->maximum_positioner_length; 
}

unsigned tslider::offset_before() const
{ 
	const tslider_definition::tresolution* conf = 
		dynamic_cast<const tslider_definition::tresolution*>(config());
	assert(conf); 
	return conf->left_offset; 
}

unsigned tslider::offset_after() const
{ 
	const tslider_definition::tresolution* conf = 
		dynamic_cast<const tslider_definition::tresolution*>(config());
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
	if(coordinate.x < 0 || coordinate.x > get_width() 
			|| coordinate.y < 0 || coordinate.y > get_height()) {
		return 0;
	}

	// we also assume the bar is over the entire height of the widget.
	if(coordinate.x < get_positioner_offset()) {
		return -1;
	} else if(coordinate.x >get_positioner_offset() + get_positioner_length()) {
		return 1;
	} else {
		return 0;
	}
}

void tslider::update_canvas()
{

	// Inherited.
	tscrollbar_::update_canvas();

	foreach(tcanvas& tmp, canvas()) {
		tmp.set_variable("text", variant((formatter() << get_value()).str()));
	}
}

} // namespace gui2

