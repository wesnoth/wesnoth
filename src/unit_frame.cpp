/* $Id: unit_frame.cpp 9735 2006-01-18 18:31:24Z boucman $ */
/*
   Copyright (C) 2006 by Jeremy Rosen <jeremy.rosen@enst-bretagne.fr>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include <global.hpp>
#include <unit_frame.hpp>
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
const std::string grr;

const std::string& progressive_string::get_current_element( int current_time)const
{
	int time = 0;
	unsigned int sub_halo = 0;
	if(data_.empty()) return grr;
	while(time < current_time&& sub_halo < data_.size()) {
		time += data_[sub_halo].second;
		sub_halo++;

	}
	if(sub_halo > 0) sub_halo --;
	if(sub_halo >= data_.size()) sub_halo = data_.size();
	return data_[sub_halo].first;
}



progressive_double::progressive_double(const std::string &data, int duration) 
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
		data_.push_back(std::pair<std::pair<double,double>,int> (
					std::pair<double,double>(
						atof(range[0].c_str()),
						atof(range.size()>1?range[1].c_str():range[0].c_str())),
					tmp2->second));
	}

}
const double progressive_double::get_current_element(int current_time)const
{
	int time = 0;
	unsigned int sub_halo = 0;
	if(data_.empty()) return 0;
	while(time < current_time&& sub_halo < data_.size()) {
		time += data_[sub_halo].second;
		sub_halo++;

	}
	if(sub_halo > 0) {
		sub_halo--;
		time -= data_[sub_halo].second;
	}
	if(sub_halo >= data_.size()) {
		sub_halo = data_.size();
		time = current_time; // never more than max allowed
	}

	const double first =  data_[sub_halo].first.first;
	const double second =  data_[sub_halo].first.second;

	return ( double(current_time - time)/(double)(data_[sub_halo].second))*(second - first)+ first;

}



int progressive_double::duration() const
{
	int total =0;
	std::vector<std::pair<std::pair<double,double>,int> >::const_iterator cur_halo;
	for(cur_halo = data_.begin() ; cur_halo != data_.end() ; cur_halo++) {
		total += cur_halo->second;
	}
	return total;

}
unit_frame::unit_frame() : 
	 image_(), image_diagonal_(),halo_(), sound_(),
	halo_x_(0), halo_y_(0), begin_time_(0), end_time_(0),
	blend_with_(0),blend_ratio_(),
	highlight_ratio_("1.0"),offset_("-20")
{
}


unit_frame::unit_frame(const std::string& str, int begin,int end,
		const std::string& highlight, const std::string& offset,
		Uint32 blend_color, const std::string& blend_rate,
		std::string in_halo, int halox, int haloy,
		const std::string & diag) :
	 image_(str),image_diagonal_(diag),
	halo_(in_halo,end_time_ - begin_time_),halo_x_(halox), halo_y_(haloy),
	begin_time_(begin), end_time_(end),
	blend_with_(blend_color), blend_ratio_(blend_rate,end_time_ - begin_time_),
	highlight_ratio_(highlight,end_time_ - begin_time_)
{
	// let's decide of duration ourselves
	if(offset.empty()) offset_=progressive_double("-20",end_time_-begin_time_);
	else offset_=progressive_double(offset,end_time_-begin_time_);
	end_time_ = end;
	end_time_ = maximum<int>(end_time_,begin + highlight_ratio_.duration());
	end_time_ = maximum<int>(end_time_,begin + blend_ratio_.duration());
	end_time_ = maximum<int>(end_time_,begin + halo_.duration());
	end_time_ = maximum<int>(end_time_,begin + offset_.duration());
}



unit_frame::unit_frame(const config& cfg)
{
	image_ = cfg["image"];
	image_diagonal_ = cfg["image_diagonal"];
	halo_x_ = atoi(cfg["halo_x"].c_str());
	halo_y_ = atoi(cfg["halo_y"].c_str());
	sound_ = cfg["sound"];
	begin_time_ = atoi(cfg["begin"].c_str());
	end_time_ = atoi(cfg["end"].c_str());
	halo_ = progressive_string(cfg["halo"],end_time_-begin_time_);
	blend_with_= 0;
	blend_ratio_ = progressive_double(cfg["blend_ratio"],end_time_-begin_time_);
	highlight_ratio_ = progressive_double(cfg["alpha"].empty()?"1.0":cfg["alpha"],end_time_-begin_time_);
	offset_ = progressive_double(cfg["offset"].empty()?"-20":cfg["offset"],end_time_-begin_time_);

}

		
const std::string &unit_frame::halo(int current_time) const 
{
	return halo_.get_current_element(current_time - begin_time_);
}

double unit_frame::blend_ratio(int current_time) const 
{
	return blend_ratio_.get_current_element(current_time - begin_time_);
}

fixed_t unit_frame::highlight_ratio(int current_time) const 
{
	return ftofxp(highlight_ratio_.get_current_element(current_time - begin_time_));
}

double unit_frame::offset(int current_time) const 
{
	return offset_.get_current_element(current_time - begin_time_);
}

