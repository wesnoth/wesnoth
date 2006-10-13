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
#include "image.hpp"
#include "serialization/string_utils.hpp"

#include <string>
#include <vector>

//
class progressive_string {
	public:
		progressive_string(const std::string& data = "",int duration = 0);
		int duration() const;
		const std::string & get_current_element(int time) const;
	private:
		std::vector<std::pair<std::string,int> > data_;
};

class progressive_double {
	public:
		progressive_double(const std::string& data = "",int duration = 0);
		int duration() const;
		const double get_current_element(int time) const;
	private:
		std::vector<std::pair<std::pair<double,double>,int> > data_;
};


class progressive_int {
	public:
		progressive_int(const std::string& data = "",int duration = 0);
		int duration() const;
		const int get_current_element(int time) const;
	private:
		std::vector<std::pair<std::pair<int,int>,int> > data_;
};


//a class to describe a unit's animation sequence
class unit_frame {
	public:
	// constructors
		unit_frame();
		explicit unit_frame(const image::locator& image, int begin=0,int end=1,
				const std::string& highlight="1.0",const std::string& offset="",
				Uint32 blend_color = 0, const std::string& blend_rate = "0",
				const std::string & in_halo = "",
				const std::string & halox = "",const std::string & haloy = "",
				const image::locator & diag ="");
		explicit unit_frame(const config& cfg);

		image::locator image() const { return image_ ;}
		image::locator image_diagonal() const { return image_diagonal_ ; }
		const std::string &halo(int current_time) const;
		std::string sound() const { return sound_ ; };
		int halo_x(int current_time) const;
		int halo_y(int current_time) const;
		int begin_time() const { return begin_time_; }
		int end_time() const { return end_time_ ; }
		Uint32 blend_with() const { return blend_with_; }
		double blend_ratio(int current_time) const;
		fixed_t highlight_ratio(int current_time) const;
		double offset(int current_time) const;
	private:
		image::locator image_;
		image::locator image_diagonal_;
		progressive_string halo_;

		std::string sound_;
		progressive_int halo_x_;
		progressive_int halo_y_;
		int begin_time_, end_time_;
		Uint32 blend_with_;
		progressive_double blend_ratio_;
		progressive_double highlight_ratio_;
		progressive_double offset_;
};

#endif
