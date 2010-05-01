/* $Id$ */
/*
   Copyright (C) 2006 - 2010 by Jeremy Rosen <jeremy.rosen@enst-bretagne.fr>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file unit_frame.hpp
 *  Frame for unit's animation sequence.
 */

#ifndef UNIT_FRAME_H_INCLUDED
#define UNIT_FRAME_H_INCLUDED

#include "map_location.hpp"
#include "util.hpp"
#include "image.hpp"
#include "serialization/string_utils.hpp"

class config;

#include <string>
#include <vector>
#include <set>

class progressive_string {
	public:
		progressive_string(const std::string& data = "",int duration = 0);
		int duration() const;
		const std::string & get_current_element(int time) const;
		bool does_not_change() const { return data_.size() <= 1; }
		std::string get_original() const { return input_; }
	private:
		std::vector<std::pair<std::string,int> > data_;
		std::string input_;
};

template <class T>
class progressive_
{
	std::vector<std::pair<std::pair<T, T>, int> > data_;
	std::string input_;
public:
	progressive_(const std::string& data = "", int duration = 0);
	int duration() const;
	const T get_current_element(int time,T default_val=0) const;
	bool does_not_change() const;
	std::string get_original() const { return input_; }
};

typedef progressive_<int> progressive_int;
typedef progressive_<double> progressive_double;

#endif
// This hack prevents MSVC++ 6 to issue several warnings
#ifndef UNIT_FRAME_H_PART2
#define UNIT_FRAME_H_PART2
/** All parameters from a frame at a given instant */
class frame_parameters{
	public:
	frame_parameters();

	int duration;
	image::locator image;
	image::locator image_diagonal;
	std::string image_mod;
	std::string halo;
	int halo_x;
	int halo_y;
	std::string halo_mod;
	std::string sound;
	std::string text;
	Uint32 text_color;
	Uint32 blend_with;
	double blend_ratio;
	double highlight_ratio;
	double offset;
	double submerge;
	int x;
	int y;
	int drawing_layer;
	bool in_hex;
	bool diagonal_in_hex;
} ;
/**
 * easily build frame parameters with the serialized constructors
 */
class frame_parsed_parameters;
class frame_builder {
	public:
		frame_builder();
		frame_builder(const config& cfg,const std::string &frame_string = "");
		/** allow easy chained modifications will raised assert if used after initialization */
		frame_builder & duration(const int duration);
		frame_builder & image(const image::locator& image ,const std::string & image_mod="");
		frame_builder & image_diagonal(const image::locator& image_diagonal,const std::string & image_mod="");
		frame_builder & sound(const std::string& sound);
		frame_builder & text(const std::string& text,const  Uint32 text_color);
		frame_builder & halo(const std::string &halo, const std::string &halo_x, const std::string& halo_y,const std::string& halo_mod);
		frame_builder & blend(const std::string& blend_ratio,const Uint32 blend_color);
		frame_builder & highlight(const std::string& highlight);
		frame_builder & offset(const std::string& offset);
		frame_builder & submerge(const std::string& submerge);
		frame_builder & x(const std::string& x);
		frame_builder & y(const std::string& y);
		frame_builder & drawing_layer(const std::string& drawing_layer);
		/** getters for the different parameters */
	private:
		friend class frame_parsed_parameters;
		int duration_;
		image::locator image_;
		image::locator image_diagonal_;
		std::string image_mod_;
		std::string halo_;
		std::string halo_x_;
		std::string halo_y_;
		std::string halo_mod_;
		std::string sound_;
		std::string text_;
		Uint32 text_color_;
		Uint32 blend_with_;
		std::string blend_ratio_;
		std::string highlight_ratio_;
		std::string offset_;
		std::string submerge_;
		std::string x_;
		std::string y_;
		std::string drawing_layer_;
};
/**
 * keep most parameters in a separate class to simplify handling of large
 * number of parameters hanling is common for frame level and animation level
 */
class frame_parsed_parameters {
	public:
		frame_parsed_parameters(const frame_builder& builder=frame_builder(),int override_duration = 0);
		/** allow easy chained modifications will raised assert if used after initialization */
		void override( int duration
				, const std::string& highlight = ""
				, const std::string& blend_ratio =""
				, Uint32 blend_color = 0
				, const std::string& offset = ""
				, const std::string& layer = "");
		/** getters for the different parameters */
		const frame_parameters parameters(int current_time) const ;

		int duration() const{ return duration_;};
		bool does_not_change() const;
		bool need_update() const;
	private:
		int duration_;
		image::locator image_;
		image::locator image_diagonal_;
		std::string image_mod_;
		progressive_string halo_;
		progressive_int halo_x_;
		progressive_int halo_y_;
		std::string halo_mod_;
		std::string sound_;
		std::string text_;
		Uint32 text_color_;
		Uint32 blend_with_;
		progressive_double blend_ratio_;
		progressive_double highlight_ratio_;
		progressive_double offset_;
		progressive_double submerge_;
		progressive_int x_;
		progressive_int y_;
		progressive_int drawing_layer_;
};
/** Describe a unit's animation sequence. */
class unit_frame {
	public:
		// Constructors
		unit_frame(const frame_builder builder=frame_builder()):builder_(builder){};
		void redraw(const int frame_time,bool first_time,const map_location & src,const map_location & dst,int*halo_id,const frame_parameters & animation_val,const frame_parameters & engine_val,const bool primary)const;
		const frame_parameters merge_parameters(int current_time,const frame_parameters & animation_val,const frame_parameters & engine_val=frame_parameters(),bool primary=false) const;
		const frame_parameters parameters(int current_time) const {return builder_.parameters(current_time);};

		int duration() const { return builder_.duration();};
		bool does_not_change() const{ return builder_.does_not_change();};
		bool need_update() const{ return builder_.need_update();};
		std::set<map_location> get_overlaped_hex(const int frame_time,const map_location & src,const map_location & dst,const frame_parameters & animation_val,const frame_parameters & engine_val,const bool primary) const;
	private:
		frame_parsed_parameters builder_;

};

#endif
