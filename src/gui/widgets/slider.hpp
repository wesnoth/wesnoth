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

#ifndef GUI_WIDGETS_SLIDER_HPP_INCLUDED
#define GUI_WIDGETS_SLIDER_HPP_INCLUDED

#include "gui/widgets/scrollbar.hpp"

namespace gui2 {

/** A slider. */
class tslider : public tscrollbar_, public tinteger_selector_ 
{
public:
	
	tslider() :
		tscrollbar_(),
		minimum_value_(0)
	{
	}

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** Inherited from tinteger_selector_. */
	void set_value(const int value);

	/** Inherited from tinteger_selector_. */
	int get_value() const { return minimum_value_ + get_item_position(); }

	/** Inherited from tinteger_selector_. */
	void set_minimum_value(const int minimum_value);

	/** Inherited from tinteger_selector_. */
	int get_minimum_value() const { return minimum_value_; }

	/** Inherited from tinteger_selector_. */
	void set_maximum_value(const int maximum_value);

	/** Inherited from tinteger_selector_. */
	int get_maximum_value() const 
		// The number of items needs to include the begin and end so count - 1.
		{ return minimum_value_ + get_item_count() - 1; }

private:

	/**
	 * The minimum value the slider holds.
	 *
	 * The maximum value is minimum + item_count_.
	 * The current value is minimum + item_position_.
	 */
	int minimum_value_;
	
	/** Inherited from tscrollbar. */
	unsigned get_length() const { return get_width(); }

	/** Inherited from tscrollbar. */
	unsigned minimum_positioner_length() const;

	/** Inherited from tscrollbar. */
	unsigned maximum_positioner_length() const;

	/** Inherited from tscrollbar. */
	unsigned offset_before() const;

	/** Inherited from tscrollbar. */
	unsigned offset_after() const;

	/** Inherited from tscrollbar. */
	bool on_positioner(const tpoint& coordinate) const;

	/** Inherited from tscrollbar. */
	int on_bar(const tpoint& coordinate) const;

	/** Inherited from tscrollbar. */
	int get_length_difference(const tpoint& original, const tpoint& current) const
		{ return current.x - original.x; }

	/** Inherited from tscrollbar. */
	void update_canvas();

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const 
		{ static const std::string type = "slider"; return type; }
};

} // namespace gui2

#endif

