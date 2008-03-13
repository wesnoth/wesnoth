/* $Id$ */
/*
   Copyright (C) 2007 - 2008 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file setting.hpp
//! This file contains the settings handling of the widget library.

#ifndef __GUI_WIDGETS_SETTING_HPP_INCLUDED__
#define __GUI_WIDGETS_SETTING_HPP_INCLUDED__

#include "gui/widgets/canvas.hpp"
#include "tstring.hpp"

#include <map>
#include <string>
#include <vector>


class config;

namespace gui2 {

struct tbutton_definition
{

	std::string id;
	t_string description;

	const std::string& read(const config& cfg);

	struct tresolution 
	{
	private:
		tresolution();

	public:
		tresolution(const config& cfg);

		unsigned window_width;
		unsigned window_height;

		unsigned min_width;
		unsigned min_height;

		unsigned default_width;
		unsigned default_height;

		unsigned max_width;
		unsigned max_height;

		unsigned text_extra_width;
		unsigned text_extra_height;
		unsigned text_font_size;

		struct tstate
		{
		private:
			tstate();

		public:
			tstate(const config* cfg);

			tcanvas canvas;
		};

		tstate enabled;
		tstate disabled;
		tstate pressed;
		tstate focussed;

	};

	std::vector<tresolution> resolutions;
};

struct tgui_definition
{
	std::string id;
	t_string description;

	const std::string& read(const config& cfg);

	std::map<std::string, tbutton_definition> buttons;
};

	std::vector<tbutton_definition::tresolution>::const_iterator get_button(const std::string& definition);

	//! Loads the setting for the theme.
	void load_settings();

	//! The screen resolution should be available for all widgets since their
	//! drawing method will depend on it.
	extern unsigned screen_width;
	extern unsigned screen_height;

} // namespace gui2

#endif

