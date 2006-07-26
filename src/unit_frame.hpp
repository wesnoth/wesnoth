/* $Id: unit_frame.hpp 9700 2006-01-15 12:00:53Z noyga $ */
/*
   Copyright (C) 2006 by Jeremy Rosen <jeremy.rosen@enst-bretagne.fr>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
   */
#ifndef UNIT_FRAME_H_INCLUDED
#define UNIT_FRAME_H_INCLUDED

#include "animated.hpp"
#include "map.hpp"
#include "config.hpp"
#include "util.hpp"
#include "serialization/string_utils.hpp"

#include <string>
#include <vector>

//a class to describe a unit's animation sequence
struct unit_frame {
		unit_frame() : xoffset(0), image(), image_diagonal(),halo(), sound(),
			       halo_x(0), halo_y(0), begin_time(0), end_time(0),
			       blend_with(0),blend_ratio(0),
			       highlight_ratio(ftofxp(1)){}
		explicit unit_frame(const std::string& str, const std::string & diag ="",
				int begin=0,int end = 0,
				Uint32 blend_color = 0, double blend_rate = 0.0,
				fixed_t highlight = ftofxp(1),
				std::string in_halo = "",int halox = 0,int haloy = 0) :
			xoffset(0), image(str),image_diagonal(diag),
			halo_x(halox), halo_y(haloy),
			begin_time(begin), end_time(end),
			blend_with(blend_color), blend_ratio(blend_rate),
			highlight_ratio(highlight)  {halo = prepare_halo(in_halo,begin,end);};
		explicit unit_frame(const config& cfg);

		int xoffset;
		std::string image;
		std::string image_diagonal;
		std::vector<std::pair<std::string,int> > halo;
		std::string sound;
		int halo_x, halo_y;
		int begin_time, end_time;
		Uint32 blend_with;
		double blend_ratio;
		fixed_t highlight_ratio;
		static std::vector<std::pair<std::string,int> > prepare_halo(const std::string & halo,int begin, int end);
};

#endif
