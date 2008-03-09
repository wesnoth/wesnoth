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

//! @file unit_frame.cpp
//!

#include "global.hpp"
// NOTE: global.hpp must be first!

#include "display.hpp"
#define UNIT_FRAME_H_PART2
#include "unit_frame.hpp"


progressive_string::progressive_string(const std::string & data,int duration)
{
		const std::vector<std::string> first_pass = utils::split(data);
		const int time_chunk = maximum<int>(duration / (first_pass.size()?first_pass.size():1),1);

		std::vector<std::string>::const_iterator tmp;
		for(tmp=first_pass.begin();tmp != first_pass.end() ; tmp++) {
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
	for(cur_halo = data_.begin() ; cur_halo != data_.end() ; cur_halo++) {
		total += cur_halo->second;
	}
	return total;

}

const std::string& progressive_string::get_current_element(int current_time,const std::string& default_val)const
{
	int time = 0;
	unsigned int sub_halo = 0;
	if(data_.empty()) return default_val;
	while(time < current_time&& sub_halo < data_.size()) {
		time += data_[sub_halo].second;
		sub_halo++;

	}
	if(sub_halo) sub_halo--;
	if(sub_halo >= data_.size()) sub_halo = data_.size();
	return data_[sub_halo].first;
}

template <class T>
progressive_<T>::progressive_(const std::string &data, int duration)
{
	const std::vector<std::string> first_split = utils::split(data);
	const int time_chunk = maximum<int>(duration / (first_split.size()?first_split.size():1),1);

	std::vector<std::string>::const_iterator tmp;
	std::vector<std::pair<std::string,int> > first_pass;
	for(tmp=first_split.begin();tmp != first_split.end() ; tmp++) {
		std::vector<std::string> second_pass = utils::split(*tmp,':');
		if(second_pass.size() > 1) {
			first_pass.push_back(std::pair<std::string,int>(second_pass[0],atoi(second_pass[1].c_str())));
		} else {
			first_pass.push_back(std::pair<std::string,int>(second_pass[0],time_chunk));
		}
	}
	std::vector<std::pair<std::string,int> >::const_iterator tmp2;
	for(tmp2=first_pass.begin();tmp2 != first_pass.end() ; tmp2++) {
		std::vector<std::string> range = utils::split(tmp2->first,'~');
		data_.push_back(std::pair<std::pair<T, T>,int> (
			std::pair<T, T>(
				lexical_cast<T>(range[0].c_str()),
				lexical_cast<T>(range.size() > 1 ? range[1].c_str() : range[0].c_str())),
				tmp2->second));
	}

}

template <class T>
const T progressive_<T>::get_current_element(int current_time,const T default_val) const
{
	int time = 0;
	unsigned int sub_halo = 0;
	int searched_time = current_time;
	if(searched_time < 0) searched_time = 0;
	if(searched_time > duration()) searched_time = duration();
	if(data_.empty()) return default_val;
	while(time < searched_time&& sub_halo < data_.size()) {
		time += data_[sub_halo].second;
		sub_halo++;

	}
	if(sub_halo != 0) {
		sub_halo--;
		time -= data_[sub_halo].second;
	}
	if(sub_halo >= data_.size()) {
		sub_halo = data_.size();
		time = searched_time; // Never more than max allowed
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
	for(cur_halo = data_.begin() ; cur_halo != data_.end() ; cur_halo++) {
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

#undef UNIT_FRAME_H_PART2
#include "unit_frame.hpp"



unit_frame::unit_frame(const config& cfg)
{
	internal_param_.image(image::locator(cfg["image"]));
	internal_param_.image_diagonal(image::locator(cfg["image_diagonal"]));
	internal_param_.sound(cfg["sound"]);
	std::vector<std::string> tmp_string_vect=utils::split(cfg["text_color"]);
	if(tmp_string_vect.size() ==3) {
	internal_param_.text(cfg["text"],
		 display::rgb(atoi(tmp_string_vect[0].c_str()),atoi(tmp_string_vect[1].c_str()),atoi(tmp_string_vect[2].c_str())));
	} else {
		internal_param_.text(cfg["text"],0);
	}

	if(!cfg["duration"].empty()) {
		internal_param_.duration(atoi(cfg["duration"].c_str()));
	} else {
		internal_param_.duration(atoi(cfg["end"].c_str()) - atoi(cfg["begin"].c_str()));
	}
	internal_param_.halo(cfg["halo"],cfg["halo_x"],cfg["halo_y"]);
	 tmp_string_vect=utils::split(cfg["blend_color"]);
	if(tmp_string_vect.size() ==3) {
		internal_param_.blend(cfg["blend_ratio"],display::rgb(atoi(tmp_string_vect[0].c_str()),atoi(tmp_string_vect[1].c_str()),atoi(tmp_string_vect[2].c_str())));
	} else {
		internal_param_.blend(cfg["blend_ratio"],0);
	}
	internal_param_.highlight(cfg["alpha"]);
	internal_param_.offset(cfg["offset"]);

}

frame_builder & frame_builder::image(const image::locator image )
{
	image_ = image;
	return *this;
}
frame_builder & frame_builder::image_diagonal(const image::locator image_diagonal)
{
	image_diagonal_ = image_diagonal;
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
frame_builder & frame_builder::halo(const std::string &halo, const std::string &halo_x, const std::string& halo_y)
{
	halo_ = progressive_string(halo,duration_);
	halo_x_ = progressive_int(halo_x,duration_);
	halo_y_ = progressive_int(halo_y,duration_);
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
	blend_ratio_=progressive_double(blend_ratio,duration_);
	return *this;
}
frame_builder & frame_builder::highlight(const std::string& highlight)
{
	highlight_ratio_=progressive_double(highlight,duration_);
	return *this;
}
frame_builder & frame_builder::offset(const std::string& offset)
{
	offset_=progressive_double(offset);
	return *this;
}
bool frame_builder::does_not_change() const
{
	return halo_.does_not_change() &&
		halo_x_.does_not_change() &&
		halo_y_.does_not_change() &&
		blend_ratio_.does_not_change() &&
		highlight_ratio_.does_not_change() &&
		offset_.does_not_change();
}
bool frame_builder::need_update() const
{
	if(!halo_.does_not_change() ||
			!halo_x_.does_not_change() ||
			!halo_y_.does_not_change() ||
			!blend_ratio_.does_not_change() ||
			!highlight_ratio_.does_not_change() ||
			!offset_.does_not_change() ) {
			return true;
	}
	return false;
}

