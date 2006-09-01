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
// a list of element to return at different time during an anim
std::vector<std::pair<std::string,int> > prepare_timed_list(const std::string & data,int duration)
{
		const std::vector<std::string> first_pass = utils::split(data);
		const int time_chunk = maximum<int>(duration / (first_pass.size()?first_pass.size():1),1);

		std::vector<std::string>::const_iterator tmp;
		std::vector<std::pair<std::string,int> > result;
		for(tmp=first_pass.begin();tmp != first_pass.end() ; tmp++) {
			std::vector<std::string> second_pass = utils::split(*tmp,':');
			if(second_pass.size() > 1) {
				result.push_back(std::pair<std::string,int>(second_pass[0],atoi(second_pass[1].c_str())));
			} else {
				result.push_back(std::pair<std::string,int>(second_pass[0],time_chunk));
			}
		}
		return result;
}

const std::string get_current_element_string(const std::vector<std::pair<std::string,int> > &data, int current_time)
{
	int time = 0;
	unsigned int sub_halo = 0;
	if(data.empty()) return std::string();
	while(time < current_time&& sub_halo < data.size()) {
		time += data[sub_halo].second;
		sub_halo++;

	}
	if(sub_halo > 0) sub_halo --;
	if(sub_halo >= data.size()) sub_halo = data.size();
	return data[sub_halo].first;
}


std::vector<std::pair<std::pair<double,double>,int> >  prepare_progressive_double(const std::string &data, int duration)
{
	std::vector<std::pair<std::string,int> > first_pass = prepare_timed_list(data,duration);
	std::vector<std::pair<std::string,int> >::const_iterator tmp;
	std::vector<std::pair<std::pair<double,double>,int> >  result;
	for(tmp=first_pass.begin();tmp != first_pass.end() ; tmp++) {
		std::vector<std::string> range = utils::split(tmp->first,'~');
		result.push_back(std::pair<std::pair<double,double>,int> (
			std::pair<double,double>(
				atof(range[0].c_str()),
				atof(range.size()>1?range[1].c_str():range[0].c_str())),
			tmp->second));
	}
	return result;

}
const double get_current_element_double(const std::vector<std::pair<std::pair<double,double>,int> > &data, int current_time)
{
	int time = 0;
	unsigned int sub_halo = 0;
	if(data.empty()) return 0;
	while(time < current_time&& sub_halo < data.size()) {
		time += data[sub_halo].second;
		sub_halo++;

	}
	if(sub_halo > 0) {
		sub_halo--;
		time -= data[sub_halo].second;
	}
	if(sub_halo >= data.size()) {
		sub_halo = data.size();
		time = current_time; // never more than max allowed
	}

	const double first =  data[sub_halo].first.first;
	const double second =  data[sub_halo].first.second;

	return ( double(current_time - time)/(double)(data[sub_halo].second))*(second - first)+ first;

}



int duration_string(const std::vector<std::pair<std::string,int> > &data) 
{
	int total =0;
	std::vector<std::pair<std::string,int> >::const_iterator cur_halo;
	for(cur_halo = data.begin() ; cur_halo != data.end() ; cur_halo++) {
		total += cur_halo->second;
	}
	return total;

}
int duration_double(const std::vector<std::pair<std::pair<double,double>,int> > &data) 
{
	int total =0;
	std::vector<std::pair<std::pair<double,double>,int> >::const_iterator cur_halo;
	for(cur_halo = data.begin() ; cur_halo != data.end() ; cur_halo++) {
		total += cur_halo->second;
	}
	return total;

}
unit_frame::unit_frame() : 
	xoffset_(0), image_(), image_diagonal_(),halo_(), sound_(),
	halo_x_(0), halo_y_(0), begin_time_(0), end_time_(0),
	blend_with_(0),blend_ratio_(),
	highlight_ratio_()
{
	highlight_ratio_ = prepare_progressive_double("1.0",0);
}


unit_frame::unit_frame(const std::string& str, const std::string & diag,
		int begin,int end,
		Uint32 blend_color, const std::string& blend_rate,
		const std::string& highlight,
		std::string in_halo, int halox, int haloy) :
	xoffset_(0), image_(str),image_diagonal_(diag),
	halo_(),halo_x_(halox), halo_y_(haloy),
	begin_time_(begin), end_time_(end),
	blend_with_(blend_color) 
{
	// let's decide of duration ourselves
	if(begin == end) {
		halo_ = prepare_timed_list(in_halo,0);
		blend_ratio_ = prepare_progressive_double(blend_rate,0);
		highlight_ratio_ = prepare_progressive_double(highlight,0);
		end_time_ =  maximum<int>(duration_double(blend_ratio_),duration_string(halo_));
		end_time_ = maximum<int>(end_time_,duration_double(highlight_ratio_));
		end_time_ += begin_time_;
	}
	halo_ = prepare_timed_list(in_halo,end_time_ - begin_time_);
	blend_ratio_ = prepare_progressive_double(blend_rate,end_time_ - begin_time_);
	highlight_ratio_ = prepare_progressive_double(highlight,end_time_ - begin_time_);
}



unit_frame::unit_frame(const config& cfg)
{
	xoffset_ = atoi(cfg["xoffset"].c_str());
	image_ = cfg["image"];
	image_diagonal_ = cfg["image_diagonal"];
	halo_x_ = atoi(cfg["halo_x"].c_str());
	halo_y_ = atoi(cfg["halo_y"].c_str());
	sound_ = cfg["sound"];
	begin_time_ = atoi(cfg["begin"].c_str());
	end_time_ = atoi(cfg["end"].c_str());
	halo_ = prepare_timed_list(cfg["halo"],end_time_-begin_time_);
	blend_with_= 0;
	blend_ratio_ = prepare_progressive_double(cfg["blend_ratio"],end_time_-begin_time_);
	highlight_ratio_ = prepare_progressive_double(cfg["highlight"].empty()?"1.0":cfg["highlight"],end_time_-begin_time_);

}

		
const std::string unit_frame::halo(int current_time) const 
{
	return get_current_element_string(halo_,current_time - begin_time_);
}

double unit_frame::blend_ratio(int current_time) const 
{
	return get_current_element_double(blend_ratio_,current_time - begin_time_);
}

fixed_t unit_frame::highlight_ratio(int current_time) const 
{
	return ftofxp(get_current_element_double(highlight_ratio_,current_time - begin_time_));
}

