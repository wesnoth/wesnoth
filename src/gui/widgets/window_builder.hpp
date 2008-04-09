/* $Id$ */
/*
   Copyright (C) 2008 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef __GUI_WIDGETS_WINDOW_BUILDER_HPP_INCLUDED__
#define __GUI_WIDGETS_WINDOW_BUILDER_HPP_INCLUDED__

#include "reference_counted_object.hpp"
#include "tstring.hpp"

#include <string>
#include <vector>

class config;
class CVideo;

namespace gui2 {

class tbuilder_grid;
class twidget;
class twindow;

twindow build(CVideo& video, const std::string& type);


//! Contains the info needed to instantiate a widget.
struct tbuilder_widget : public reference_counted_object
{
private:
	tbuilder_widget();

public:
	tbuilder_widget(const config& /*cfg*/) {}


	virtual twidget* build() const = 0;
	virtual ~tbuilder_widget() {}
};

typedef boost::intrusive_ptr<tbuilder_widget> tbuilder_widget_ptr;
typedef boost::intrusive_ptr<const tbuilder_widget> const_tbuilder_widget_ptr;

class twindow_builder
{
public:
	const std::string& read(const config& cfg);

	struct tresolution
	{
	private:
		tresolution();

	public:
		tresolution(const config& cfg);

		unsigned window_width;
		unsigned window_height;

		unsigned width;
		unsigned height;

		// note x, y hardcoded.
		
		// FIXME add min max and default size
		// the we can use best size to get the best.
		
		std::string definition;

	
		tbuilder_grid* grid;
	};

	std::vector<tresolution> resolutions;
	
private:
	std::string id_;
	std::string description_;

};

} // namespace gui2


#endif



