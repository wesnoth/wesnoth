/*
   Copyright (C) 2006 - 2014 by Jeremy Rosen <jeremy.rosen@enst-bretagne.fr>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#include "global.hpp"

#include "game_display.hpp"
#include "halo.hpp"
#include "sound.hpp"
#include "unit_frame.hpp"


progressive_string::progressive_string(const std::string & data,int duration) :
	data_(),
	input_(data)
{
		const std::vector<std::string> first_pass = utils::square_parenthetical_split(data);
		int time_chunk = std::max<int>(duration, 1);
		std::vector<std::string>::const_iterator tmp;

		if (duration > 1 && first_pass.size() > 0) {
			// If duration specified, divide evenly the time for items with unspecified times
			int total_specified_time = 0;
			for(tmp=first_pass.begin(); tmp != first_pass.end(); ++tmp) {
				std::vector<std::string> second_pass = utils::split(*tmp,':');
				if(second_pass.size() > 1) {
					total_specified_time += atoi(second_pass[1].c_str());
				}
			}
			time_chunk = std::max<int>((duration - total_specified_time) / first_pass.size(), 1);
		}

		for(tmp=first_pass.begin(); tmp != first_pass.end(); ++tmp) {
			std::vector<std::string> second_pass = utils::split(*tmp,':');
			if(second_pass.size() > 1) {
				data_.push_back(std::pair<std::string,int>(second_pass[0],atoi(second_pass[1].c_str())));
			} else {
				data_.push_back(std::pair<std::string,int>(second_pass[0],time_chunk));
			}
		}
}
int progressive_string::duration() const
{
	int total =0;
	std::vector<std::pair<std::string,int> >::const_iterator cur_halo;
	for(cur_halo = data_.begin() ; cur_halo != data_.end() ; ++cur_halo) {
		total += cur_halo->second;
	}
	return total;
}

progressive_image::progressive_image(const std::string & data,int duration) :
	data_(),
	input_(data)
{
		const std::vector<std::string> first_pass = utils::square_parenthetical_split(data);
		int time_chunk = std::max<int>(duration, 1);
		std::vector<std::string>::const_iterator tmp;

		if (duration > 1 && first_pass.size() > 0) {
			// If duration specified, divide evenly the time for images with unspecified times
			int total_specified_time = 0;
			for(tmp=first_pass.begin(); tmp != first_pass.end(); ++tmp) {
				std::vector<std::string> second_pass = utils::split(*tmp,':');
				if(second_pass.size() > 1) {
					total_specified_time += atoi(second_pass[1].c_str());
				}
			}
			time_chunk = std::max<int>((duration - total_specified_time) / first_pass.size(), 1);
		}

		for(tmp=first_pass.begin(); tmp != first_pass.end(); ++tmp) {
			std::vector<std::string> second_pass = utils::split(*tmp,':');
			if(second_pass.size() > 1) {
				data_.push_back(std::pair<image::locator,int>(second_pass[0],atoi(second_pass[1].c_str())));
			} else {
				data_.push_back(std::pair<image::locator,int>(second_pass[0],time_chunk));
			}
		}
}
int progressive_image::duration() const
{
	int total =0;
	std::vector<std::pair<image::locator,int> >::const_iterator cur_halo;
	for(cur_halo = data_.begin() ; cur_halo != data_.end() ; ++cur_halo) {
		total += cur_halo->second;
	}
	return total;
}

const image::locator empty_image;

const image::locator& progressive_image::get_current_element(int current_time) const
{
	int time = 0;
	unsigned int sub_image = 0;
	if(data_.empty()) return empty_image;
	while(time < current_time&& sub_image < data_.size()) {
		time += data_[sub_image].second;
		++sub_image;

	}
	if(sub_image) sub_image--;
	return data_[sub_image].first;
}

static const std::string empty_string;

const std::string& progressive_string::get_current_element(int current_time) const
{
	int time = 0;
	unsigned int sub_halo = 0;
	if(data_.empty()) return empty_string;
	while(time < current_time&& sub_halo < data_.size()) {
		time += data_[sub_halo].second;
		++sub_halo;

	}
	if(sub_halo) sub_halo--;
	return data_[sub_halo].first;
}

template <class T>
progressive_<T>::progressive_(const std::string &data, int duration) :
	data_(),
	input_(data)
{
	int split_flag = utils::REMOVE_EMPTY; // useless to strip spaces
	const std::vector<std::string> comma_split = utils::split(data,',',split_flag);
	const int time_chunk = std::max<int>(1, duration / std::max<int>(comma_split.size(),1));

	std::vector<std::string>::const_iterator com_it = comma_split.begin();
	for(; com_it != comma_split.end(); ++com_it) {
		std::vector<std::string> colon_split = utils::split(*com_it,':',split_flag);
		int time = (colon_split.size() > 1) ? atoi(colon_split[1].c_str()) : time_chunk;

		std::vector<std::string> range = utils::split(colon_split[0],'~',split_flag);
		T range0 = lexical_cast<T>(range[0]);
		T range1 = (range.size() > 1) ? lexical_cast<T>(range[1]) : range0;
		typedef std::pair<T,T> range_pair;
		data_.push_back(std::pair<range_pair,int>(range_pair(range0, range1), time));
	}
}

template <class T>
const T progressive_<T>::get_current_element(int current_time, T default_val) const
{
	int time = 0;
	unsigned int sub_halo = 0;
	int searched_time = current_time;
	if(searched_time < 0) searched_time = 0;
	if(searched_time > duration()) searched_time = duration();
	if(data_.empty()) return default_val;
	while(time < searched_time&& sub_halo < data_.size()) {
		time += data_[sub_halo].second;
		++sub_halo;

	}
	if(sub_halo != 0) {
		sub_halo--;
		time -= data_[sub_halo].second;
	}

	const T first =  data_[sub_halo].first.first;
	const T second =  data_[sub_halo].first.second;

	return T((static_cast<double>(searched_time - time) /
		static_cast<double>(data_[sub_halo].second)) *
		(second - first) + first);
}

template<class T>
int progressive_<T>::duration() const
{
	int total = 0;
	typename std::vector<std::pair<std::pair<T, T>, int> >::const_iterator cur_halo;
	for(cur_halo = data_.begin() ; cur_halo != data_.end() ; ++cur_halo) {
		total += cur_halo->second;
	}
	return total;

}

template <class T>
bool progressive_<T>::does_not_change() const
{
return data_.empty() ||
	( data_.size() == 1 && data_[0].first.first == data_[0].first.second);
}

// Force compilation of the following template instantiations
template class progressive_<int>;
template class progressive_<double>;

bool tristate_to_bool(tristate tri, bool def)
{
	switch(tri)
	{
	case(t_false):
		return false;
	case(t_true):
		return true;
	case(t_unset):
		return def;
	default:
		throw "found unexpected tristate";
	}
}

frame_parameters::frame_parameters() :
	duration(0),
	image(),
	image_diagonal(),
	image_mod(""),
	halo(""),
	halo_x(0),
	halo_y(0),
	halo_mod(""),
	sound(""),
	text(""),
	text_color(0),
	blend_with(0),
	blend_ratio(0.0),
	highlight_ratio(1.0),
	offset(0),
	submerge(0.0),
	x(0),
	y(0),
	directional_x(0),
	directional_y(0),
	auto_vflip(t_unset),
	auto_hflip(t_unset),
	primary_frame(t_unset),
	drawing_layer(display::LAYER_UNIT_DEFAULT - display::LAYER_UNIT_FIRST)
{}

frame_builder::frame_builder() :
	duration_(1),
	image_(),
	image_diagonal_(),
	image_mod_(""),
	halo_(""),
	halo_x_(""),
	halo_y_(""),
	halo_mod_(""),
	sound_(""),
	text_(""),
	text_color_(0),
	blend_with_(0),
	blend_ratio_(""),
	highlight_ratio_(""),
	offset_(""),
	submerge_(""),
	x_(""),
	y_(""),
	directional_x_(""),
	directional_y_(""),
	auto_vflip_(t_unset),
	auto_hflip_(t_unset),
	primary_frame_(t_unset),
	drawing_layer_(str_cast(display::LAYER_UNIT_DEFAULT - display::LAYER_UNIT_FIRST))
{}

frame_builder::frame_builder(const config& cfg,const std::string& frame_string) :
	duration_(1),
	image_(cfg[frame_string + "image"]),
	image_diagonal_(cfg[frame_string + "image_diagonal"]),
	image_mod_(cfg[frame_string + "image_mod"]),
	halo_(cfg[frame_string + "halo"]),
	halo_x_(cfg[frame_string + "halo_x"]),
	halo_y_(cfg[frame_string + "halo_y"]),
	halo_mod_(cfg[frame_string + "halo_mod"]),
	sound_(cfg[frame_string + "sound"]),
	text_(cfg[frame_string + "text"]),
	text_color_(0),
	blend_with_(0),
	blend_ratio_(cfg[frame_string + "blend_ratio"]),
	highlight_ratio_(cfg[frame_string + "alpha"]),
	offset_(cfg[frame_string + "offset"]),
	submerge_(cfg[frame_string + "submerge"]),
	x_(cfg[frame_string + "x"]),
	y_(cfg[frame_string + "y"]),
	directional_x_(cfg[frame_string + "directional_x"]),
	directional_y_(cfg[frame_string + "directional_y"]),
	auto_vflip_(t_unset),
	auto_hflip_(t_unset),
	primary_frame_(t_unset),
	drawing_layer_(cfg[frame_string + "layer"])
{
	if(!cfg.has_attribute(frame_string + "auto_vflip")) {
		auto_vflip_ = t_unset;
	} else if(cfg[frame_string + "auto_vflip"].to_bool()) {
		auto_vflip_ = t_true;
	} else {
		auto_vflip_ = t_false;
	}
	if(!cfg.has_attribute(frame_string + "auto_hflip")) {
		auto_hflip_ = t_unset;
	} else if(cfg[frame_string + "auto_hflip"].to_bool()) {
		auto_hflip_ = t_true;
	} else {
		auto_hflip_ = t_false;
	}
	if(!cfg.has_attribute(frame_string + "primary")) {
		primary_frame_ = t_unset;
	} else if(cfg[frame_string + "primary"].to_bool()) {
		primary_frame_ = t_true;
	} else {
		primary_frame_ = t_false;
	}
	std::vector<std::string> color = utils::split(cfg[frame_string + "text_color"]);
	if (color.size() == 3) {
		text_color_ = display::rgb(atoi(color[0].c_str()),
			atoi(color[1].c_str()), atoi(color[2].c_str()));
	}

	if (const config::attribute_value *v = cfg.get(frame_string + "duration")) {
		duration(*v);
	} else if (!cfg.get(frame_string + "end")) {
		int halo_duration = (progressive_string(halo_,1)).duration();
		int image_duration = (progressive_image(image_,1)).duration();
		int image_diagonal_duration = (progressive_image(image_diagonal_,1)).duration();
		duration(std::max(std::max(image_duration,image_diagonal_duration),halo_duration));
		
	} else {
		duration(cfg[frame_string + "end"].to_int() - cfg[frame_string + "begin"].to_int());
	}
	duration_ = std::max(duration_,1);

	color = utils::split(cfg[frame_string + "blend_color"]);
	if (color.size() == 3) {
		blend_with_ = display::rgb(atoi(color[0].c_str()),
			atoi(color[1].c_str()), atoi(color[2].c_str()));
	}
}

frame_builder & frame_builder::image(const std::string& image ,const std::string & image_mod)
{
	image_ = image;
	image_mod_ = image_mod;
	return *this;
}
frame_builder & frame_builder::image_diagonal(const std::string& image_diagonal,const std::string& image_mod)
{
	image_diagonal_ = image_diagonal;
	image_mod_ = image_mod;
	return *this;
}
frame_builder & frame_builder::sound(const std::string& sound)
{
	sound_=sound;
	return *this;
}
frame_builder & frame_builder::text(const std::string& text,const  Uint32 text_color)
{
	text_=text;
	text_color_=text_color;
	return *this;
}
frame_builder & frame_builder::halo(const std::string &halo, const std::string &halo_x, const std::string& halo_y,const std::string & halo_mod)
{
	halo_ = halo;
	halo_x_ = halo_x;
	halo_y_ = halo_y;
	halo_mod_= halo_mod;
	return *this;
}
frame_builder & frame_builder::duration(const int duration)
{
	duration_= duration;
	return *this;
}
frame_builder & frame_builder::blend(const std::string& blend_ratio,const Uint32 blend_color)
{
	blend_with_=blend_color;
	blend_ratio_=blend_ratio;
	return *this;
}
frame_builder & frame_builder::highlight(const std::string& highlight)
{
	highlight_ratio_=highlight;
	return *this;
}
frame_builder & frame_builder::offset(const std::string& offset)
{
	offset_=offset;
	return *this;
}
frame_builder & frame_builder::submerge(const std::string& submerge)
{
	submerge_=submerge;
	return *this;
}
frame_builder & frame_builder::x(const std::string& x)
{
	x_=x;
	return *this;
}
frame_builder & frame_builder::y(const std::string& y)
{
	y_=y;
	return *this;
}
frame_builder & frame_builder::directional_x(const std::string& directional_x)
{
	directional_x_=directional_x;
	return *this;
}
frame_builder & frame_builder::directional_y(const std::string& directional_y)
{
	directional_y_=directional_y;
	return *this;
}
frame_builder & frame_builder::auto_vflip(const bool auto_vflip)
{
	if(auto_vflip) auto_vflip_ = t_true;
	else auto_vflip_ = t_false;
	return *this;
}
frame_builder & frame_builder::auto_hflip(const bool auto_hflip)
{
	if(auto_hflip) auto_hflip_ = t_true;
	else auto_hflip_ = t_false;
	return *this;
}
frame_builder & frame_builder::primary_frame(const bool primary_frame)
{
	if(primary_frame) primary_frame_ = t_true;
	else primary_frame_ = t_false;
	return *this;
}
frame_builder & frame_builder::drawing_layer(const std::string& drawing_layer)
{
	drawing_layer_=drawing_layer;
	return *this;
}

frame_parsed_parameters::frame_parsed_parameters(const frame_builder & builder, int duration) :
	duration_(duration ? duration :builder.duration_),
	image_(builder.image_,duration_),
	image_diagonal_(builder.image_diagonal_,duration_),
	image_mod_(builder.image_mod_),
	halo_(builder.halo_,duration_),
	halo_x_(builder.halo_x_,duration_),
	halo_y_(builder.halo_y_,duration_),
	halo_mod_(builder.halo_mod_),
	sound_(builder.sound_),
	text_(builder.text_),
	text_color_(builder.text_color_),
	blend_with_(builder.blend_with_),
	blend_ratio_(builder.blend_ratio_,duration_),
	highlight_ratio_(builder.highlight_ratio_,duration_),
	offset_(builder.offset_,duration_),
	submerge_(builder.submerge_,duration_),
	x_(builder.x_,duration_),
	y_(builder.y_,duration_),
	directional_x_(builder.directional_x_,duration_),
	directional_y_(builder.directional_y_,duration_),
	auto_vflip_(builder.auto_vflip_),
	auto_hflip_(builder.auto_hflip_),
	primary_frame_(builder.primary_frame_),
	drawing_layer_(builder.drawing_layer_,duration_)
{}


bool frame_parsed_parameters::does_not_change() const
{
	return image_.does_not_change() &&
		image_diagonal_.does_not_change() &&
		halo_.does_not_change() &&
		halo_x_.does_not_change() &&
		halo_y_.does_not_change() &&
		blend_ratio_.does_not_change() &&
		highlight_ratio_.does_not_change() &&
		offset_.does_not_change() &&
		submerge_.does_not_change() &&
		x_.does_not_change() &&
		y_.does_not_change() &&
		directional_x_.does_not_change() &&
		directional_y_.does_not_change() &&
		drawing_layer_.does_not_change();
}
bool frame_parsed_parameters::need_update() const
{
	if(!image_.does_not_change() ||
			!image_diagonal_.does_not_change() ||
			!halo_.does_not_change() ||
			!halo_x_.does_not_change() ||
			!halo_y_.does_not_change() ||
			!blend_ratio_.does_not_change() ||
			!highlight_ratio_.does_not_change() ||
			!offset_.does_not_change() ||
			!submerge_.does_not_change() ||
			!x_.does_not_change() ||
			!y_.does_not_change() ||
			!directional_x_.does_not_change() ||
			!directional_y_.does_not_change() ||
			!drawing_layer_.does_not_change() ) {
			return true;
	}
	return false;
}

const frame_parameters frame_parsed_parameters::parameters(int current_time) const
{
	frame_parameters result;
	result.duration = duration_;
	result.image = image_.get_current_element(current_time);
	result.image_diagonal = image_diagonal_.get_current_element(current_time);
	result.image_mod = image_mod_;
	result.halo = halo_.get_current_element(current_time);
	result.halo_x = halo_x_.get_current_element(current_time);
	result.halo_y = halo_y_.get_current_element(current_time);
	result.halo_mod = halo_mod_;
	result.sound = sound_;
	result.text = text_;
	result.text_color = text_color_;
	result.blend_with = blend_with_;
	result.blend_ratio = blend_ratio_.get_current_element(current_time);
	result.highlight_ratio = highlight_ratio_.get_current_element(current_time,1.0);
	result.offset = offset_.get_current_element(current_time,-1000);
	result.submerge = submerge_.get_current_element(current_time);
	result.x = x_.get_current_element(current_time);
	result.y = y_.get_current_element(current_time);
	result.directional_x = directional_x_.get_current_element(current_time);
	result.directional_y = directional_y_.get_current_element(current_time);
	result.auto_vflip = auto_vflip_;
	result.auto_hflip = auto_hflip_;
	result.primary_frame = primary_frame_;
	result.drawing_layer = drawing_layer_.get_current_element(current_time,display::LAYER_UNIT_DEFAULT-display::LAYER_UNIT_FIRST);
	return result;
}

void frame_parsed_parameters::override( int duration
		, const std::string& highlight
		, const std::string& blend_ratio
		, Uint32 blend_color
		, const std::string& offset
		, const std::string& layer
		, const std::string& modifiers)
{
	if(!highlight.empty()) {
		highlight_ratio_ = progressive_double(highlight,duration);
	} else if(duration != duration_){
		highlight_ratio_=progressive_double(highlight_ratio_.get_original(),duration);
	}
	if(!offset.empty()) {
		offset_= progressive_double(offset,duration);
	} else  if(duration != duration_){
		offset_=progressive_double(offset_.get_original(),duration);
	}
	if(!blend_ratio.empty()) {
		blend_ratio_ = progressive_double(blend_ratio,duration);
		blend_with_  = blend_color;
	} else  if(duration != duration_){
		blend_ratio_=progressive_double(blend_ratio_.get_original(),duration);
	}
	if(!layer.empty()) {
		drawing_layer_ = progressive_int(layer,duration);
	} else  if(duration != duration_){
		drawing_layer_=progressive_int(drawing_layer_.get_original(),duration);
	}
	if(!modifiers.empty()) {
		image_mod_+=modifiers;
	}

	if(duration != duration_) {
		image_ = progressive_image(image_.get_original(),duration);
		image_diagonal_ = progressive_image(image_diagonal_.get_original(),duration);
		halo_ = progressive_string(halo_.get_original(),duration);
		halo_x_ = progressive_int(halo_x_.get_original(),duration);
		halo_y_ = progressive_int(halo_y_.get_original(),duration);
		submerge_=progressive_double(submerge_.get_original(),duration);
		x_=progressive_int(x_.get_original(),duration);
		y_=progressive_int(y_.get_original(),duration);
		directional_x_=progressive_int(directional_x_.get_original(),duration);
		directional_y_=progressive_int(directional_y_.get_original(),duration);
		duration_ = duration;
	}
}
std::vector<std::string> frame_parsed_parameters::debug_strings() const {
	std::vector<std::string> v;
	if (duration_>0) v.push_back("duration="+utils::half_signed_value(duration_));
	if (!image_.get_original().empty()) v.push_back("image="+image_.get_original());
	if (!image_diagonal_.get_original().empty()) v.push_back("image_diagonal="+image_diagonal_.get_original());
	if (!image_mod_.empty()) v.push_back("image_mod="+image_mod_);
	if (!halo_.get_original().empty()) v.push_back("halo="+halo_.get_original());
	if (!halo_x_.get_original().empty()) v.push_back("halo_x="+halo_x_.get_original());
	if (!halo_y_.get_original().empty()) v.push_back("halo_y="+halo_y_.get_original());
	if (!halo_mod_.empty()) v.push_back("halo_mod="+halo_mod_);
	if (!sound_.empty()) v.push_back("sound="+sound_);
	if (!text_.empty()) {
		v.push_back("text="+text_);
		v.push_back("text_color="+str_cast<Uint32>(text_color_));
	}
	if (!blend_ratio_.get_original().empty()) {
		v.push_back("blend_ratio="+blend_ratio_.get_original());
		v.push_back("blend_with="+str_cast<Uint32>(blend_with_));
	}
	if (!highlight_ratio_.get_original().empty()) v.push_back("highlight_ratio="+highlight_ratio_.get_original());
	if (!offset_.get_original().empty()) v.push_back("offset="+offset_.get_original());
	if (!submerge_.get_original().empty()) v.push_back("submerge="+submerge_.get_original());
	if (!x_.get_original().empty()) v.push_back("x="+x_.get_original());
	if (!y_.get_original().empty()) v.push_back("y="+y_.get_original());
	if (!directional_x_.get_original().empty()) v.push_back("directional_x="+directional_x_.get_original());
	if (!directional_y_.get_original().empty()) v.push_back("directional_y="+directional_y_.get_original());
	if (auto_vflip_ == t_true) v.push_back("auto_vflip=true");
	if (auto_vflip_ == t_false) v.push_back("auto_vflip=false");
	if (auto_hflip_ == t_true) v.push_back("auto_hflip=true");
	if (auto_hflip_ == t_false) v.push_back("auto_hflip=false");
	if (primary_frame_ == t_true) v.push_back("primary_frame=true");
	if (primary_frame_ == t_false) v.push_back("primary_frame=false");
	if (!drawing_layer_.get_original().empty()) v.push_back("drawing_layer="+drawing_layer_.get_original());
	return v;
}


void unit_frame::redraw(const int frame_time,bool on_start_time,bool in_scope_of_frame,const map_location & src,const map_location & dst,int*halo_id,const frame_parameters & animation_val,const frame_parameters & engine_val)const
{
	const int xsrc = game_display::get_singleton()->get_location_x(src);
	const int ysrc = game_display::get_singleton()->get_location_y(src);
	const int xdst = game_display::get_singleton()->get_location_x(dst);
	const int ydst = game_display::get_singleton()->get_location_y(dst);
	const map_location::DIRECTION direction = src.get_relative_dir(dst);

	const frame_parameters current_data = merge_parameters(frame_time,animation_val,engine_val);
	double tmp_offset = current_data.offset;

		// debug code allowing to see the number of frames and their position
		// you need to add a '/n'
		// if (tmp_offset) std::cout << (int)(tmp_offset*100) << ",";

	int d2 = display::get_singleton()->hex_size() / 2;
	if(on_start_time ) {
		// stuff that should be done only once per frame
		if(!current_data.sound.empty()  ) {
			sound::play_sound(current_data.sound);
		}
		if(!current_data.text.empty()  ) {
			game_display::get_singleton()->float_label(src,current_data.text,
			(current_data.text_color & 0x00FF0000) >> 16,
			(current_data.text_color & 0x0000FF00) >> 8,
			(current_data.text_color & 0x000000FF) >> 0);
		}
	}
	image::locator image_loc;
	if(direction != map_location::NORTH && direction != map_location::SOUTH) {
		image_loc = image::locator(current_data.image_diagonal,current_data.image_mod);
	}
	if(image_loc.is_void() || image_loc.get_filename() == "") { // invalid diag image, or not diagonal
		image_loc = image::locator(current_data.image,current_data.image_mod);
	}

	surface image;
	if(!image_loc.is_void() && image_loc.get_filename() != "") { // invalid diag image, or not diagonal
		image=image::get_image(image_loc, image::SCALED_TO_ZOOM);
	}
	const int x = static_cast<int>(tmp_offset * xdst + (1.0-tmp_offset) * xsrc) + d2;
	const int y = static_cast<int>(tmp_offset * ydst + (1.0-tmp_offset) * ysrc) + d2;
	if (image != NULL) {
#ifdef LOW_MEM
		bool facing_west = false;
#else
		bool facing_west = direction == map_location::NORTH_WEST || direction == map_location::SOUTH_WEST;
#endif
		bool facing_north = direction == map_location::NORTH_WEST || direction == map_location::NORTH || direction == map_location::NORTH_EAST;
		if(!current_data.auto_hflip) facing_west = false;
		if(!current_data.auto_vflip) facing_north = true;
		int my_x = x + current_data.x- image->w/2;
		int my_y = y + current_data.y- image->h/2;
		if(facing_west) {
			my_x -= current_data.directional_x;
		} else {
			my_x += current_data.directional_x;
		}
		if(facing_north) {
			my_y += current_data.directional_y;
		} else {
			my_y -= current_data.directional_y;
		}

		game_display::get_singleton()->render_image( my_x,my_y,
			       	static_cast<display::tdrawing_layer>(display::LAYER_UNIT_FIRST+current_data.drawing_layer),
			       	src, image, facing_west, false,
				ftofxp(current_data.highlight_ratio), current_data.blend_with,
			       	current_data.blend_ratio,current_data.submerge,!facing_north);
	}
	
	halo::remove(*halo_id);
	*halo_id = halo::NO_HALO;
	
	if (!in_scope_of_frame) { //check after frame as first/last frame image used in defense/attack anims
		return;
	}
	
	if(!current_data.halo.empty()) {
		halo::ORIENTATION orientation;
		switch(direction)
		{
			case map_location::NORTH:
			case map_location::NORTH_EAST:
				orientation = halo::NORMAL;
				break;
			case map_location::SOUTH_EAST:
			case map_location::SOUTH:
				if(!current_data.auto_vflip) {
					orientation = halo::NORMAL;
				} else {
					orientation = halo::VREVERSE;
				}
				break;
			case map_location::SOUTH_WEST:
				if(!current_data.auto_vflip) {
					orientation = halo::HREVERSE;
				} else {
					orientation = halo::HVREVERSE;
				}
				break;
			case map_location::NORTH_WEST:
				orientation = halo::HREVERSE;
				break;
			case map_location::NDIRECTIONS:
			default:
				orientation = halo::NORMAL;
				break;
		}
		
		if(direction != map_location::SOUTH_WEST && direction != map_location::NORTH_WEST) {
			*halo_id = halo::add(static_cast<int>(x+current_data.halo_x* game_display::get_singleton()->get_zoom_factor()),
					static_cast<int>(y+current_data.halo_y* game_display::get_singleton()->get_zoom_factor()),
					current_data.halo + current_data.halo_mod,
					map_location(-1, -1),
					orientation);
		} else {
			*halo_id = halo::add(static_cast<int>(x-current_data.halo_x* game_display::get_singleton()->get_zoom_factor()),
					static_cast<int>(y+current_data.halo_y* game_display::get_singleton()->get_zoom_factor()),
					current_data.halo + current_data.halo_mod,
					map_location(-1, -1),
					orientation);
		}
	}
}
std::set<map_location> unit_frame::get_overlaped_hex(const int frame_time,const map_location & src,const map_location & dst,const frame_parameters & animation_val,const frame_parameters & engine_val) const
{
	display* disp = display::get_singleton();
	const int xsrc = disp->get_location_x(src);
	const int ysrc = disp->get_location_y(src);
	const int xdst = disp->get_location_x(dst);
	const int ydst = disp->get_location_y(dst);
	const map_location::DIRECTION direction = src.get_relative_dir(dst);

	const frame_parameters current_data = merge_parameters(frame_time,animation_val,engine_val);
	double tmp_offset = current_data.offset;
	int d2 = game_display::get_singleton()->hex_size() / 2;

	image::locator image_loc;
	if(direction != map_location::NORTH && direction != map_location::SOUTH) {
		image_loc = image::locator(current_data.image_diagonal,current_data.image_mod);
	}
	if(image_loc.is_void() || image_loc.get_filename() == "") { // invalid diag image, or not diagonal
		image_loc = image::locator(current_data.image,current_data.image_mod);
	}

	// we always invalidate our own hex because we need to be called at redraw time even
	// if we don't draw anything in the hex itself
	std::set<map_location> result;
	if(tmp_offset==0 && current_data.x == 0 && current_data.directional_x == 0 && image::is_in_hex(image_loc)) {
		result.insert(src);
		int my_y = current_data.y;
		bool facing_north = direction == map_location::NORTH_WEST || direction == map_location::NORTH || direction == map_location::NORTH_EAST;
		if(!current_data.auto_vflip) facing_north = true;
		if(facing_north) {
			my_y += current_data.directional_y;
		} else {
			my_y -= current_data.directional_y;
		}
		if(my_y < 0) {
			result.insert(src.get_direction(map_location::NORTH));
			result.insert(src.get_direction(map_location::NORTH_EAST));
			result.insert(src.get_direction(map_location::NORTH_WEST));
		} else if(my_y > 0) {
			result.insert(src.get_direction(map_location::SOUTH));
			result.insert(src.get_direction(map_location::SOUTH_EAST));
			result.insert(src.get_direction(map_location::SOUTH_WEST));
		}
	} else {
		int w=0;
		int h =0;
#ifdef _OPENMP
#pragma omp critical(frame_surface) // with the way surfaces work it's hard to lock the refcount within sdl_utils
#endif //_OPENMP
		{
			surface image;
			if(!image_loc.is_void() && image_loc.get_filename() != "") { // invalid diag image, or not diagonal
				image=image::get_image(image_loc,
						image::SCALED_TO_ZOOM
						);
			}
			if(image != NULL) {
				w = image->w;
				h = image->h;
			}
		}
		if (w != 0 || h != 0) {
			const int x = static_cast<int>(tmp_offset * xdst + (1.0-tmp_offset) * xsrc);
			const int y = static_cast<int>(tmp_offset * ydst + (1.0-tmp_offset) * ysrc);
#ifdef LOW_MEM
			bool facing_west = false;
#else
			bool facing_west = direction == map_location::NORTH_WEST || direction == map_location::SOUTH_WEST;
#endif
			bool facing_north = direction == map_location::NORTH_WEST || direction == map_location::NORTH || direction == map_location::NORTH_EAST;
			if(!current_data.auto_vflip) facing_north = true;
			if(!current_data.auto_hflip) facing_west = false;
			int my_x = x +current_data.x+d2- w/2;
			int my_y = y +current_data.y+d2- h/2;
			if(facing_west) {
				my_x += current_data.directional_x;
			} else {
				my_x -= current_data.directional_x;
			}
			if(facing_north) {
				my_y += current_data.directional_y;
			} else {
				my_y -= current_data.directional_y;
			}

			const SDL_Rect r = create_rect(my_x, my_y, w, h);
			// check if our underlying hexes are invalidated
			// if we need to update ourselves because we changed, invalidate our hexes
			// and return whether or not our hexes was invalidated
			// invalidate ourself to be called at redraw time
			result.insert(src);
			display::rect_of_hexes underlying_hex = disp->hexes_under_rect(r);
			result.insert(underlying_hex.begin(),underlying_hex.end());
		} else {
			// we have no "redraw surface" but we still need to invalidate our own hex
			// in case we have a halo and/or sound that needs a redraw
			// invalidate ourself to be called at redraw time
			result.insert(src);
			result.insert(dst);
		}
	}
	return result;
}



const frame_parameters unit_frame::merge_parameters(int current_time,const frame_parameters & animation_val,const frame_parameters & engine_val) const
{
	/**
	 * this function merges the value provided by
	 *  * the frame
	 *  * the engine (poison, flying unit...)
	 *  * the animation as a whole
	 *  there is no absolute rule for merging, so creativity is the rule
	 *  if a value is never provided by the engine, assert. (this way if it becomes used, people will easily find the right place to look)
	 *
	 */
	frame_parameters result;
	const frame_parameters & current_val = builder_.parameters(current_time);

	result.primary_frame = engine_val.primary_frame;
	if(animation_val.primary_frame != t_unset) result.primary_frame = animation_val.primary_frame;
	if(current_val.primary_frame != t_unset) result.primary_frame = current_val.primary_frame;
	const bool primary = tristate_to_bool(result.primary_frame, true);

	/** engine provides a default image to use for the unit when none is available */
	result.image = current_val.image.is_void() || current_val.image.get_filename() == ""?animation_val.image:current_val.image;
	if(primary && ( result.image.is_void() || result.image.get_filename().empty())) {
		result.image = engine_val.image;
	}

	/** engine provides a default image to use for the unit when none is available */
	result.image_diagonal = current_val.image_diagonal.is_void() || current_val.image_diagonal.get_filename() == ""?animation_val.image_diagonal:current_val.image_diagonal;
	if(primary && ( result.image_diagonal.is_void() || result.image_diagonal.get_filename().empty())) {
		result.image_diagonal = engine_val.image_diagonal;
	}

	/** engine provides a string for "petrified" and "team color" modifications
          note that image_mod is the complete modification and halo_mod is only the TC part
          see unit.cpp, we know that and use it*/
		result.image_mod = current_val.image_mod +animation_val.image_mod;
	if(primary) {
                result.image_mod += engine_val.image_mod;
        } else {
                result.image_mod += engine_val.halo_mod;
        }

	assert(engine_val.halo.empty());
	result.halo = current_val.halo.empty()?animation_val.halo:current_val.halo;

	assert(engine_val.halo_x == 0);
	result.halo_x =  current_val.halo_x?current_val.halo_x:animation_val.halo_x;

	/** the engine provide y modification for terrain with height adjust and flying units */
	result.halo_y = current_val.halo_y?current_val.halo_y:animation_val.halo_y;
	result.halo_y += engine_val.halo_y;

	result.halo_mod = current_val.halo_mod +animation_val.halo_mod;
	result.halo_mod += engine_val.halo_mod;

	assert(engine_val.duration == 0);
	result.duration = current_val.duration;

	assert(engine_val.sound.empty());
	result.sound = current_val.sound.empty()?animation_val.sound:current_val.sound;

	assert(engine_val.text.empty());
	result.text = current_val.text.empty()?animation_val.text:current_val.text;

	assert(engine_val.text_color == 0);
	result.text_color = current_val.text_color?current_val.text_color:animation_val.text_color;

	/** engine provide a blend color for poisoned units */
	result.blend_with = current_val.blend_with?current_val.blend_with:animation_val.blend_with;
	if(primary&& engine_val.blend_with) result.blend_with = display::max_rgb(engine_val.blend_with,result.blend_with);

	/** engine provide a blend color for poisoned units */
	result.blend_ratio = current_val.blend_ratio?current_val.blend_ratio:animation_val.blend_ratio;
	if(primary && engine_val.blend_ratio) {
		result.blend_ratio = std::min(
				  result.blend_ratio + engine_val.blend_ratio
				, 1.0);
	}

	/** engine provide a highlight ratio for selected units and visible "invisible" units */
	result.highlight_ratio = current_val.highlight_ratio!=1.0?current_val.highlight_ratio:animation_val.highlight_ratio;
	if(primary && engine_val.highlight_ratio != 1.0) result.highlight_ratio = result.highlight_ratio +engine_val.highlight_ratio - 1.0; // selected unit

	assert(engine_val.offset == 0);
	result.offset = (current_val.offset!=-1000)?current_val.offset:animation_val.offset;
	if(result.offset == -1000) result.offset = 0.0;

	/** engine provides a submerge for units in water */
	result.submerge = current_val.submerge?current_val.submerge:animation_val.submerge;
	if(primary && engine_val.submerge && !result.submerge ) result.submerge = engine_val.submerge;

	assert(engine_val.x == 0);
	result.x = current_val.x?current_val.x:animation_val.x;

	/** the engine provide y modification for terrain with height adjust and flying units */
	result.y = current_val.y?current_val.y:animation_val.y;
	result.y += engine_val.y;

	assert(engine_val.directional_x == 0);
	result.directional_x = current_val.directional_x?current_val.directional_x:animation_val.directional_x;
	assert(engine_val.directional_y == 0);
	result.directional_y = current_val.directional_y?current_val.directional_y:animation_val.directional_y;

	assert(engine_val.drawing_layer == display::LAYER_UNIT_DEFAULT-display::LAYER_UNIT_FIRST);
	result.drawing_layer = current_val.drawing_layer !=  display::LAYER_UNIT_DEFAULT-display::LAYER_UNIT_FIRST?
		current_val.drawing_layer:animation_val.drawing_layer;

	/** the engine provide us with default value to compare with, we update if different */
	result.auto_hflip = engine_val.auto_hflip;
	if(animation_val.auto_hflip != t_unset) result.auto_hflip = animation_val.auto_hflip;
	if(current_val.auto_hflip != t_unset) result.auto_hflip = current_val.auto_hflip;
	if(result.auto_hflip == t_unset) result.auto_hflip = t_true;

	result.auto_vflip = engine_val.auto_vflip;
	if(animation_val.auto_vflip != t_unset) result.auto_vflip = animation_val.auto_vflip;
	if(current_val.auto_vflip != t_unset) result.auto_vflip = current_val.auto_vflip;
	if(result.auto_vflip == t_unset) {
		if(primary) result.auto_vflip=t_false;
		else result.auto_vflip = t_true;
	}
#ifdef LOW_MEM
	if(primary) {
		result.image= engine_val.image;
		result.image_diagonal= engine_val.image;
	}
#endif
	return result;
}
