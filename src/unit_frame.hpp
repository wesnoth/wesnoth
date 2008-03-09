/* $Id$ */
/*
   Copyright (C) 2006 - 2008 by Jeremy Rosen <jeremy.rosen@enst-bretagne.fr>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file unit_frame.hpp
//! Frame for unit's animation sequence.

#ifndef UNIT_FRAME_H_INCLUDED
#define UNIT_FRAME_H_INCLUDED

#include "map.hpp"
#include "util.hpp"
#include "image.hpp"
#include "serialization/string_utils.hpp"

class config;

#include <string>
#include <vector>

//
class progressive_string {
	public:
		progressive_string(const std::string& data = "",int duration = 0);
		int duration() const;
		const std::string & get_current_element(int time,const std::string& default_val="") const;
		bool does_not_change() const { return data_.size() <= 1; }
	private:
		std::vector<std::pair<std::string,int> > data_;
};

template <class T>
class progressive_
{
	std::vector<std::pair<std::pair<T, T>, int> > data_;
public:
	progressive_(const std::string& data = "", int duration = 0);
	int duration() const;
	const T get_current_element(int time, const T default_val = 0) const;
	bool does_not_change() const;
};

typedef progressive_<int> progressive_int;
typedef progressive_<double> progressive_double;

#endif
// This hack prevents MSVC++ 6 to issue several warnings
#ifndef UNIT_FRAME_H_PART2
#define UNIT_FRAME_H_PART2

//! keep most parameters in a separate class to simplify handling of large number of parameters
class frame_builder {
	public:
		//! initial constructor
		frame_builder():
		image_(image::locator()),
		image_diagonal_(image::locator()),
		halo_(""),
		sound_(""),
		text_(""),
		text_color_(0),
		halo_x_(""),
		halo_y_(""),
		duration_(1),
		blend_with_(0),
		blend_ratio_(""),
		highlight_ratio_(""),
		offset_("") {};
		//! allow easy chained modifications
		frame_builder & image(const image::locator image );
		frame_builder & image_diagonal(const image::locator image_diagonal);
		frame_builder & sound(const std::string& sound);
		frame_builder & text(const std::string& text,const  Uint32 text_color);
		frame_builder & halo(const std::string &halo, const std::string &halo_x, const std::string& halo_y);
		frame_builder & duration(const int duration);
		frame_builder & blend(const std::string& blend_ratio,const Uint32 blend_color);
		frame_builder & highlight(const std::string& highlight);
		frame_builder & offset(const std::string& offset);
		//! getters for the different parameters
		const image::locator image() const { return image_ ;}
		const image::locator image_diagonal() const { return image_diagonal_ ; }
		const std::string &halo(int current_time,const std::string& default_val="") const
			{ return halo_.get_current_element(current_time,default_val); }

		const std::string sound() const { return sound_ ; };
		const std::pair<std::string,Uint32> text() const { return std::pair<std::string,Uint32>(text_,text_color_) ; };
		const int halo_x(int current_time,const int default_val=0) const { return halo_x_.get_current_element(current_time,default_val); }
		const int halo_y(int current_time,const int default_val=0) const { return halo_y_.get_current_element(current_time,default_val); }
		const int duration() const { return duration_; }
		const Uint32 blend_with(const Uint32 default_val) const { return blend_with_?blend_with_:default_val; }
		const double blend_ratio(int current_time,const double default_val=0.0) const
			{ return blend_ratio_.get_current_element(current_time,default_val); }

		const fixed_t highlight_ratio(int current_time,double default_val =0.0) const
			{  return ftofxp(highlight_ratio_.get_current_element(current_time,default_val)); }

		double offset(int current_time,double default_val =0.0) const
			{ return offset_.get_current_element(current_time,default_val)  ; }

		bool does_not_change() const;
		bool need_update() const;
	private:
		image::locator image_;
		image::locator image_diagonal_;
		progressive_string halo_;
		std::string sound_;
		std::string text_;
		Uint32 text_color_;
		progressive_int halo_x_;
		progressive_int halo_y_;
		int duration_;
		Uint32 blend_with_;
		progressive_double blend_ratio_;
		progressive_double highlight_ratio_;
		progressive_double offset_;
};
//! Describe a unit's animation sequence.
class unit_frame{
	public:
		// Constructors
		unit_frame(const frame_builder builder=frame_builder()):internal_param_(builder){};
		explicit unit_frame(const config& cfg);

		bool does_not_change() const { return internal_param_.does_not_change();}
		bool need_update() const { return internal_param_.need_update();}
		// Passing internal params
		const image::locator image() const { return internal_param_.image() ;}
		const image::locator image_diagonal() const { return internal_param_.image_diagonal() ; }
		const std::string &halo(int current_time,const std::string& default_val="") const
			{ return internal_param_.halo(current_time,default_val); }
		const std::string sound() const { return internal_param_.sound() ; };
		const std::pair<std::string,Uint32> text() const { return internal_param_.text(); };
		const int halo_x(int current_time,const int default_val=0) const { return internal_param_.halo_x(current_time,default_val); }
		const int halo_y(int current_time,const int default_val=0) const { return internal_param_.halo_y(current_time,default_val); }
		const int duration() const { return internal_param_.duration(); }
		const Uint32 blend_with(const Uint32 default_val) const { return internal_param_.blend_with(default_val); }
		const double blend_ratio(int current_time,const double default_val=0.0) const
			{ return internal_param_.blend_ratio(current_time,default_val); }
		const fixed_t highlight_ratio(int current_time,double default_val =0.0) const
			{  return internal_param_.highlight_ratio(current_time,default_val); }
		const double offset(int current_time,double default_val =0.0) const
			{ return internal_param_.offset(current_time,default_val)  ; }

	private:
		frame_builder internal_param_;
};

#endif
