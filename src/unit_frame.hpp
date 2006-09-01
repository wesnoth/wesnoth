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
extern std::string grr(const char*);
class unit_frame {
	public:
	// constructors
		unit_frame();
		explicit unit_frame(const std::string& str, const std::string & diag ="",
				int begin=0,int end = 0,
				Uint32 blend_color = 0, const std::string& blend_rate = "",
				const std::string& highlight="1.0",
				std::string in_halo = "",int halox = 0,int haloy = 0);
		explicit unit_frame(const config& cfg);

		int xoffset() const { return xoffset_ ; }
		std::string image() const { return image_ ;}
		std::string image_diagonal() const { return image_diagonal_ ; }
		const std::string halo(int current_time) const;
		std::string sound() const { return sound_ ; };
		int halo_x() const { return halo_x_; }
		int halo_y() const { return halo_y_; }
		int begin_time() const { return begin_time_; }
		int end_time() const { return end_time_ ; }
		Uint32 blend_with() const { return blend_with_; }
		double blend_ratio(int current_time) const;
		fixed_t highlight_ratio(int current_time) const;
	private:
		int xoffset_;
		std::string image_;
		std::string image_diagonal_;
		std::vector<std::pair<std::string,int> > halo_;
		std::string sound_;
		int halo_x_, halo_y_;
		int begin_time_, end_time_;
		Uint32 blend_with_;
		std::vector<std::pair<std::pair<double,double>,int> > blend_ratio_;
		std::vector<std::pair<std::pair<double,double>,int> > highlight_ratio_;
};

#endif
