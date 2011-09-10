/* $Id$ */
/*
   Copyright (C) 2006 - 2011 by Jeremy Rosen <jeremy.rosen@enst-bretagne.fr>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file
 *  Frame for unit's animation sequence.
 */

#ifndef UNIT_FRAME_H_INCLUDED
#define UNIT_FRAME_H_INCLUDED

#include "image.hpp"
#include "utils/interned.hpp"

class config;

template <class T>
class progressive_discrete {
public:
	progressive_discrete(const n_token::t_token& data = n_token::t_token::z_empty(), int duration = 0) : data_(), input_(data) {
		progressive_discrete_core(data, duration); }
	void progressive_discrete_core(const n_token::t_token& data = n_token::t_token::z_empty(), int duration = 0);
	progressive_discrete(progressive_discrete const & );
	int duration() const;
	const T & get_current_element(int time) const;
	bool does_not_change() const { return data_.size() <= 1; }
	n_token::t_token const & get_original() const { return input_; }
	bool operator==(progressive_discrete const &) const;

	template<typename X>
	friend size_t hash_value(progressive_discrete<X> const &);

private:
	typedef std::vector<std::pair<n_token::t_token, int> > t_data;
	t_data data_;
	n_token::t_token input_;
};

template <class T>
class progressive_continuous {
	typedef std::pair<T,T> range_pair;
	typedef std::vector<std::pair<range_pair, int> > t_data;
	t_data data_;
	n_token::t_token input_;

	//Do not inline it fixes static initialization problems
	static T const & default_default_value();

public:
	progressive_continuous(const n_token::t_token& data = n_token::t_token::z_empty(), int duration = 0) : data_(), input_(data) {
		progressive_continuous_core(data, duration); }
	void progressive_continuous_core(const n_token::t_token& data = n_token::t_token::z_empty(), int duration = 0);
	progressive_continuous(progressive_continuous const &);
	int duration() const;
	const T get_current_element(int time,T const & default_val=progressive_continuous<T>::default_default_value() ) const;
	bool does_not_change() const;
	n_token::t_token const & get_original() const { return input_; }

	bool operator==(progressive_continuous const &) const;
	template <typename X> friend size_t hash_value(progressive_continuous<X> const &);
};

typedef progressive_discrete<n_token::t_token> progressive_token;
typedef progressive_discrete<std::string> progressive_string;
typedef progressive_continuous<int> progressive_int;
typedef progressive_continuous<double> progressive_double;

typedef enum tristate {t_false,t_true,t_unset} tristate;
/** All parameters from a frame at a given instant */
class frame_parameters {
	public:
	frame_parameters();

	int duration;
	image::locator image;
	image::locator image_diagonal;
	std::string image_mod;
	n_token::t_token halo;
	int halo_x;
	int halo_y;
	std::string halo_mod;
	n_token::t_token sound;
	n_token::t_token text;
	Uint32 text_color;
	Uint32 blend_with;
	double blend_ratio;
	double highlight_ratio;
	double offset;
	double submerge;
	int x;
	int y;
	int directional_x;
	int directional_y;
	tristate auto_vflip;
	tristate auto_hflip;
	tristate primary_frame;
	int drawing_layer;
} ;
/**
 * easily build frame parameters with the serialized constructors
 */
class frame_parsed_parameters;
class frame_builder {
	public:
		frame_builder();
		frame_builder(const config& cfg,const n_token::t_token &frame_string = n_token::t_token::z_empty());
		/** allow easy chained modifications will raised assert if used after initialization */
		frame_builder & duration(const int duration);
		frame_builder & image(const image::locator& image ,const n_token::t_token & image_mod= n_token::t_token::z_empty());
		frame_builder & image_diagonal(const image::locator& image_diagonal,const n_token::t_token & image_mod= n_token::t_token::z_empty());
		frame_builder & sound(const n_token::t_token& sound);
		frame_builder & text(const n_token::t_token& text,const  Uint32 text_color);
		frame_builder & halo(const n_token::t_token &halo, const n_token::t_token &halo_x, const n_token::t_token& halo_y,const n_token::t_token& halo_mod);
		frame_builder & blend(const n_token::t_token& blend_ratio,const Uint32 blend_color);
		frame_builder & highlight(const n_token::t_token& highlight);
		frame_builder & offset(const n_token::t_token& offset);
		frame_builder & submerge(const n_token::t_token& submerge);
		frame_builder & x(const n_token::t_token& x);
		frame_builder & y(const n_token::t_token& y);
		frame_builder & directional_x(const n_token::t_token& directional_x);
		frame_builder & directional_y(const n_token::t_token& directional_y);
		frame_builder & auto_vflip(const bool auto_vflip);
		frame_builder & auto_hflip(const bool auto_hflip);
		frame_builder & primary_frame(const bool primary_frame);
		frame_builder & drawing_layer(const n_token::t_token& drawing_layer);
		/** getters for the different parameters */
	private:
		friend class frame_parsed_parameters;
		int duration_;
		image::locator image_;
		image::locator image_diagonal_;
		n_token::t_token image_mod_;
		n_token::t_token halo_;
		n_token::t_token halo_x_;
		n_token::t_token halo_y_;
		n_token::t_token halo_mod_;
		n_token::t_token sound_;
		n_token::t_token text_;
		Uint32 text_color_;
		Uint32 blend_with_;
		n_token::t_token blend_ratio_;
		n_token::t_token highlight_ratio_;
		n_token::t_token offset_;
		n_token::t_token submerge_;
		n_token::t_token x_;
		n_token::t_token y_;
		n_token::t_token directional_x_;
		n_token::t_token directional_y_;
		tristate auto_vflip_;
		tristate auto_hflip_;
		tristate primary_frame_;
		n_token::t_token drawing_layer_;
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
				, const n_token::t_token& highlight = n_token::t_token::z_empty()
				, const n_token::t_token& blend_ratio = n_token::t_token::z_empty()
				, Uint32 blend_color = 0
				, const n_token::t_token& offset = n_token::t_token::z_empty()
				, const n_token::t_token& layer = n_token::t_token::z_empty()
				, const n_token::t_token& modifiers = n_token::t_token::z_empty());
		/** getters for the different parameters */
		const frame_parameters parameters(int current_time) const ;

		int duration() const{ return duration_;};
		bool does_not_change() const;
		bool need_update() const;

	bool operator==(frame_parsed_parameters const & a) const;
	friend size_t hash_value(frame_parsed_parameters const &);
	friend std::ostream & operator<<(std::ostream &out, frame_parsed_parameters const & a){
		return out << a.text_; }

	private:
		int duration_;
		image::locator image_;
		image::locator image_diagonal_;
		n_token::t_token image_mod_;
		progressive_token halo_;
		progressive_int halo_x_;
		progressive_int halo_y_;
		n_token::t_token halo_mod_;
		n_token::t_token sound_;
		n_token::t_token text_;
		Uint32 text_color_;
		Uint32 blend_with_;
		progressive_double blend_ratio_;
		progressive_double highlight_ratio_;
		progressive_double offset_;
		progressive_double submerge_;
		progressive_int x_;
		progressive_int y_;
		progressive_int directional_x_;
		progressive_int directional_y_;
		tristate auto_vflip_;
		tristate auto_hflip_;
		tristate primary_frame_;
		progressive_int drawing_layer_;
};

/// A token to a frame builder which are 10:1 redundant

///todo a second index to the frame_parameter_token via the builder so that the toke can be
///created without constructing the frame parameters
typedef n_interned::t_interned_token<frame_parsed_parameters> t_frame_parameter_token;


/** Describe a unit's animation sequence. */
class unit_frame {
public:
	// Constructors
	unit_frame(const frame_builder builder=frame_builder()) : builder_(frame_parsed_parameters(builder)){ }
	void redraw(const int frame_time,bool first_time,const map_location & src,const map_location & dst,int*halo_id,const frame_parameters & animation_val,const frame_parameters & engine_val)const;
	const frame_parameters merge_parameters(int current_time,const frame_parameters & animation_val,const frame_parameters & engine_val=frame_parameters()) const;
	const frame_parameters parameters(int current_time) const {
		return static_cast<frame_parsed_parameters const &>(builder_).parameters(current_time); }

	int duration() const { return static_cast<frame_parsed_parameters const &>(builder_).duration();};
	bool does_not_change() const{ return static_cast<frame_parsed_parameters const &>(builder_).does_not_change();};
	bool need_update() const{ return static_cast<frame_parsed_parameters const &>(builder_).need_update();};
	std::set<map_location> get_overlaped_hex(const int frame_time,const map_location & src,const map_location & dst,const frame_parameters & animation_val,const frame_parameters & engine_val) const;
private:
	t_frame_parameter_token builder_;
};

#endif
