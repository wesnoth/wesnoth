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

#include "tstring.hpp"

#include <string>
#include <vector>

class config;
class CVideo;

namespace gui2 {

class twindow;

twindow build(CVideo& video, const std::string& type);

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
		
		std::string definition;

		struct tgrid
		{
		private:
			tgrid();

		public:
			tgrid(const config* cfg);

			unsigned rows;
			unsigned cols;

			struct twidget
			{
			// NOTE a widget is always a button atm.
			private:
				twidget();

			public:
				twidget(const config& cfg);

				std::string id;
				std::string definition;
				t_string label;
			};

			std::vector<twidget> widgets;
		};

	
		tgrid grid;
	};

	std::vector<tresolution> resolutions;
	
private:
	std::string id_;
	std::string description_;

};

} // namespace gui2


#endif



