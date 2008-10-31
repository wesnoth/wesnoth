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

#ifndef GUI_WIDGETS_IMAGE_HPP_INCLUDED
#define GUI_WIDGETS_IMAGE_HPP_INCLUDED

#include "gui/widgets/control.hpp"

namespace gui2 {

/** An image. */
class timage : public tcontrol
{
public:
	
	timage() 
		: tcontrol(COUNT)
	{
	}

	/** Inherited from tcontrol. */
	tpoint get_minimum_size() const { return get_best_size(); }

	/** Inherited from tcontrol. */
	tpoint get_best_size() const;

	/** Import overloaded versions. */
	using tcontrol::get_best_size;

	/** Inherited from tcontrol. */
	tpoint get_maximum_size() const { return get_best_size(); }

	/** Inherited from tcontrol. */
	void set_active(const bool /*active*/) {}

	/** Inherited from tcontrol. */
	bool get_active() const { return true; }

	/** Inherited from tcontrol. */
	unsigned get_state() const { return ENABLED; }

	/** Inherited from tcontrol. */
	bool does_block_easy_close() const { return false; }

private:

	/**
	 * Possible states of the widget.
	 *
	 * Note the order of the states must be the same as defined in settings.hpp.
	 */
	enum tstate { ENABLED, COUNT };

	/** Inherited from tcontrol. */
	const std::string& get_control_type() const 
		{ static const std::string type = "image"; return type; }
};

} // namespace gui2

#endif

