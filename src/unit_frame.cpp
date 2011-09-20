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

/** @file */

#include "global.hpp"

#include "game_display.hpp"
#include "halo.hpp"
#include "sound.hpp"
#include "unit_frame.hpp"



template <class T>
void progressive_discrete<T>::progressive_discrete_core(n_token::t_token const & data, int duration) {
	const std::vector<n_token::t_token> first_pass = utils::split_token(data);
	const int time_chunk = std::max<int>(duration / (first_pass.size() ? first_pass.size() : 1), 1);

	std::vector<n_token::t_token>::const_iterator tmp;
	for(tmp=first_pass.begin(); tmp != first_pass.end() ; ++tmp) {
		std::vector<n_token::t_token> second_pass = utils::split_token(*tmp,':');
		if(second_pass.size() > 1) {
			data_.push_back(std::pair<n_token::t_token, int>(second_pass[0],atoi(second_pass[1].c_str())));
		} else {
			data_.push_back(std::pair<n_token::t_token, int>(second_pass[0],time_chunk));
		}
	}
}

template <class T>
progressive_discrete<T>::progressive_discrete(progressive_discrete<T> const & a) : data_(), input_(a.input_) {
	typename t_data::const_iterator i=a.data_.begin(), iend=a.data_.end();
	for(;i!= iend; ++i){ data_.push_back(*i); } }

template <class T>
int progressive_discrete<T>::duration() const {
	int total =0;
	std::vector<std::pair<n_token::t_token, int> >::const_iterator cur_halo;
	for(cur_halo = data_.begin() ; cur_halo != data_.end() ; ++cur_halo) {
		total += cur_halo->second; }
	return total;
}

template <class T>
const T& progressive_discrete<T>::get_current_element(int current_time)const {
	int time = 0;
	unsigned int sub_halo = 0;
	if(data_.empty()) return n_token::t_token::z_empty();
	while(time < current_time&& sub_halo < data_.size()) {
		time += data_[sub_halo].second;
		++sub_halo; }
	if(sub_halo) sub_halo--;
	return data_[sub_halo].first;
}

template <class T>
bool progressive_discrete<T>::operator==(progressive_discrete const & b) const {
	if (data_.size() != b.data_.size() ){ return false; }
	return std::equal(data_.begin(), data_.end(), b.data_.begin());
}

template <typename T>
size_t hash_value(progressive_discrete<T> const & a) {
	return boost::hash_value(a.data_); }



template <class T>
T const & progressive_continuous<T>::default_default_value() {
	static T * defval= new T();
	return *defval;
}

template <class T>
void progressive_continuous<T>::progressive_continuous_core(const config::t_token &data, int duration) {
	int split_flag = utils::REMOVE_EMPTY; // useless to strip spaces
	const std::vector<config::t_token> comma_split = utils::split_token(data,',',split_flag);
	const int time_chunk = std::max<int>(1, duration / std::max<int>(comma_split.size(),1));

	std::vector<config::t_token>::const_iterator com_it = comma_split.begin();
	for(; com_it != comma_split.end(); ++com_it) {
		std::vector<config::t_token> colon_split = utils::split_token(*com_it,':',split_flag);
		int time = (colon_split.size() > 1) ? atoi(colon_split[1].c_str()) : time_chunk;

		std::vector<config::t_token> range = utils::split_token(colon_split[0],'~',split_flag);
		T range0 = lexical_cast<T>(range[0]);
		T range1 = (range.size() > 1) ? lexical_cast<T>(range[1]) : range0;
		data_.push_back(std::pair<range_pair, int>(range_pair(range0, range1), time));
	}
}

template <class T>
progressive_continuous<T>::progressive_continuous(progressive_continuous<T> const & a) : data_(), input_(a.input_) {
	typename t_data::const_iterator i=a.data_.begin(), iend=a.data_.end();
	for(;i!= iend; ++i){ data_.push_back(*i); }
}

template <class T>
const T progressive_continuous<T>::get_current_element(int current_time, T const & default_val) const  {
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
int progressive_continuous<T>::duration() const {
	int total = 0;
	typename std::vector<std::pair<std::pair<T, T>, int> >::const_iterator cur_halo;
	for(cur_halo = data_.begin() ; cur_halo != data_.end() ; ++cur_halo) {
		total += cur_halo->second;
	}
	return total;
}

template <class T>
bool progressive_continuous<T>::does_not_change() const {
	return data_.empty() || ( data_.size() == 1 && data_[0].first.first == data_[0].first.second);
}

template <class T>
bool progressive_continuous<T>::operator==(progressive_continuous const & b) const {
	if (data_.size() != b.data_.size()) { return false; }
	return std::equal(data_.begin(), data_.end(), b.data_.begin());
}

template <typename T>
size_t hash_value(progressive_continuous<T> const & a) {
	return boost::hash_value(a.data_); }


// Force compilation of the following template instantiations
template class progressive_discrete<n_token::t_token>;
template class progressive_discrete<std::string>;
template class progressive_continuous<int>;
template class progressive_continuous<double>;

frame_parameters::frame_parameters() :
	duration(0),
	image(),
	image_diagonal(),
	image_mod(n_token::t_token::z_empty()),
	halo(n_token::t_token::z_empty()),
	halo_x(0),
	halo_y(0),
	halo_mod(n_token::t_token::z_empty()),
	sound(n_token::t_token::z_empty()),
	text(n_token::t_token::z_empty()),
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
	image_mod_(n_token::t_token::z_empty()),
	halo_(n_token::t_token::z_empty()),
	halo_x_(n_token::t_token::z_empty()),
	halo_y_(n_token::t_token::z_empty()),
	halo_mod_(n_token::t_token::z_empty()),
	sound_(n_token::t_token::z_empty()),
	text_(n_token::t_token::z_empty()),
	text_color_(0),
	blend_with_(0),
	blend_ratio_(n_token::t_token::z_empty()),
	highlight_ratio_(n_token::t_token::z_empty()),
	offset_(n_token::t_token::z_empty()),
	submerge_(n_token::t_token::z_empty()),
	x_(n_token::t_token::z_empty()),
	y_(n_token::t_token::z_empty()),
	directional_x_(n_token::t_token::z_empty()),
	directional_y_(n_token::t_token::z_empty()),
	auto_vflip_(t_unset),
	auto_hflip_(t_unset),
	primary_frame_(t_unset),
	drawing_layer_(str_cast(display::LAYER_UNIT_DEFAULT - display::LAYER_UNIT_FIRST))
{}

namespace{
	DEFAULT_TOKEN_BODY(z_image_default, "image")
	DEFAULT_TOKEN_BODY(z_image_diagonal_default, "image_diagonal")
	DEFAULT_TOKEN_BODY(z_image_mod_default, "image_mod")
	DEFAULT_TOKEN_BODY(z_halo_default, "halo")
	DEFAULT_TOKEN_BODY(z_halo_x_default, "halo_x")
	DEFAULT_TOKEN_BODY(z_halo_y_default, "halo_y")
	DEFAULT_TOKEN_BODY(z_halo_mod_default, "halo_mod")
	DEFAULT_TOKEN_BODY(z_sound_default, "sound")
	DEFAULT_TOKEN_BODY(z_text_default, "text")
	DEFAULT_TOKEN_BODY(z_blend_ratio_default, "blend_ratio")
	DEFAULT_TOKEN_BODY(z_alpha_default, "alpha")
	DEFAULT_TOKEN_BODY(z_offset_default, "offset")
	DEFAULT_TOKEN_BODY(z_submerge_default, "submerge")
	DEFAULT_TOKEN_BODY(z_x_default, "x")
	DEFAULT_TOKEN_BODY(z_y_default, "y")
	DEFAULT_TOKEN_BODY(z_directional_x_default, "directional_x")
	DEFAULT_TOKEN_BODY(z_directional_y_default, "directional_y")
	DEFAULT_TOKEN_BODY(z_layer_default, "layer")
}
frame_builder::frame_builder(const config& cfg,const n_token::t_token& frame_string) :
	duration_(1),
	image_(cfg[frame_string + z_image_default()].token()),
	image_diagonal_(cfg[frame_string + z_image_diagonal_default()].token()),
	image_mod_(cfg[frame_string + z_image_mod_default()].token()),
	halo_(cfg[frame_string + z_halo_default()].token()),
	halo_x_(cfg[frame_string + z_halo_x_default()].token()),
	halo_y_(cfg[frame_string + z_halo_y_default()].token()),
	halo_mod_(cfg[frame_string + z_halo_mod_default()].token()),
	sound_(cfg[frame_string + z_sound_default()].token()),
	text_(cfg[frame_string + z_text_default()].token()),
	text_color_(0),
	blend_with_(0),
	blend_ratio_(cfg[frame_string + z_blend_ratio_default()].token()),
	highlight_ratio_(cfg[frame_string + z_alpha_default()].token()),
	offset_(cfg[frame_string + z_offset_default()].token()),
	submerge_(cfg[frame_string + z_submerge_default()].token()),
	x_(cfg[frame_string + z_x_default()].token()),
	y_(cfg[frame_string + z_y_default()].token()),
	directional_x_(cfg[frame_string + z_directional_x_default()].token()),
	directional_y_(cfg[frame_string + z_directional_y_default()].token()),
	auto_vflip_(t_unset),
	auto_hflip_(t_unset),
	primary_frame_(t_unset),
	drawing_layer_(cfg[frame_string + z_layer_default()].token())
{
	static const config::t_token z_auto_vflip("auto_vflip", false);
	static const config::t_token z_auto_hflip("auto_hflip", false);
	static const config::t_token z_primary("primary", false);
	static const config::t_token z_text_color("text_color", false);
	static const config::t_token z_duration("duration", false);
	static const config::t_token z_end("end", false);
	static const config::t_token z_begin("begin", false);
	static const config::t_token z_blend_color("blend_color", false);
	if(!cfg.has_attribute(frame_string + z_auto_vflip)) {
		auto_vflip_ = t_unset;
	} else if(cfg[frame_string + z_auto_vflip].to_bool()) {
		auto_vflip_ = t_true;
	} else {
		auto_vflip_ = t_false;
	}
	if(!cfg.has_attribute(frame_string + z_auto_hflip)) {
		auto_hflip_ = t_unset;
	} else if(cfg[frame_string + z_auto_hflip].to_bool()) {
		auto_hflip_ = t_true;
	} else {
		auto_hflip_ = t_false;
	}
	if(!cfg.has_attribute(frame_string + z_primary)) {
		primary_frame_ = t_unset;
	} else if(cfg[frame_string + z_primary].to_bool()) {
		primary_frame_ = t_true;
	} else {
		primary_frame_ = t_false;
	}
	std::vector<n_token::t_token> color = utils::split_attr(cfg[frame_string + z_text_color]);
	if (color.size() == 3) {
		text_color_ = display::rgb(atoi(color[0].c_str()),
			atoi(color[1].c_str()), atoi(color[2].c_str()));
	}

	if (const config::attribute_value *v = cfg.get(frame_string + z_duration)) {
		duration(*v);
	} else {
		duration(cfg[frame_string + z_end].to_int() - cfg[frame_string + z_begin].to_int());
	}

	color = utils::split_attr(cfg[frame_string + z_blend_color]);
	if (color.size() == 3) {
		blend_with_ = display::rgb(atoi(color[0].c_str()),
			atoi(color[1].c_str()), atoi(color[2].c_str()));
	}
}

frame_builder & frame_builder::image(const image::locator& image ,const n_token::t_token & image_mod)
{
	image_ = image;
	image_mod_ = image_mod;
	return *this;
}
frame_builder & frame_builder::image_diagonal(const image::locator& image_diagonal,const n_token::t_token& image_mod)
{
	image_diagonal_ = image_diagonal;
	image_mod_ = image_mod;
	return *this;
}
frame_builder & frame_builder::sound(const n_token::t_token& sound)
{
	sound_=sound;
	return *this;
}
frame_builder & frame_builder::text(const n_token::t_token& text,const  Uint32 text_color)
{
	text_=text;
	text_color_=text_color;
	return *this;
}
frame_builder & frame_builder::halo(const n_token::t_token &halo, const n_token::t_token &halo_x, const n_token::t_token& halo_y,const n_token::t_token & halo_mod)
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
frame_builder & frame_builder::blend(const n_token::t_token& blend_ratio,const Uint32 blend_color)
{
	blend_with_=blend_color;
	blend_ratio_=blend_ratio;
	return *this;
}
frame_builder & frame_builder::highlight(const n_token::t_token& highlight)
{
	highlight_ratio_=highlight;
	return *this;
}
frame_builder & frame_builder::offset(const n_token::t_token& offset)
{
	offset_=offset;
	return *this;
}
frame_builder & frame_builder::submerge(const n_token::t_token& submerge)
{
	submerge_=submerge;
	return *this;
}
frame_builder & frame_builder::x(const n_token::t_token& x)
{
	x_=x;
	return *this;
}
frame_builder & frame_builder::y(const n_token::t_token& y)
{
	y_=y;
	return *this;
}
frame_builder & frame_builder::directional_x(const n_token::t_token& directional_x)
{
	directional_x_=directional_x;
	return *this;
}
frame_builder & frame_builder::directional_y(const n_token::t_token& directional_y)
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
frame_builder & frame_builder::drawing_layer(const n_token::t_token& drawing_layer)
{
	drawing_layer_=drawing_layer;
	return *this;
}


frame_parsed_parameters::frame_parsed_parameters(const frame_builder & builder, int duration) :
	duration_(duration ? duration :builder.duration_),
	image_(builder.image_),
	image_diagonal_(builder.image_diagonal_),
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
{
}

bool frame_parsed_parameters::operator==(frame_parsed_parameters const & b) const {
	return duration_ == b.duration_
		&& image_ == b.image_
		&& image_diagonal_ == b.image_diagonal_
		&& image_mod_ == b.image_mod_
		&& halo_ == b.halo_
		&& halo_x_ == b.halo_x_
		&& halo_y_ == b.halo_y_
		&& halo_mod_ == b. halo_mod_

		&& sound_ == b.sound_
		&& text_ == b.text_
		&& text_color_ == b.text_color_
		&& blend_with_ == b.blend_with_
		&& blend_ratio_ == b.blend_ratio_
		&& highlight_ratio_ == b.highlight_ratio_
		&& offset_ == b.offset_
		&& submerge_ == b.submerge_

		&& x_ == b.x_
		&& y_ == b.y_
		&& directional_x_ == b.directional_y_
		&& directional_y_ == b.directional_x_
		&& auto_vflip_ == b.auto_vflip_
		&& auto_hflip_ == b.auto_hflip_
		&& primary_frame_ == b.primary_frame_
		&& drawing_layer_ == b.drawing_layer_
		;
}

size_t hash_value(frame_parsed_parameters const & a) {
	std::size_t hash = 0;
	boost::hash_combine(hash, a.duration_);
	boost::hash_combine(hash, a.image_);
	boost::hash_combine(hash, a.image_diagonal_);
	boost::hash_combine(hash, a.image_mod_);

	boost::hash_combine(hash, a.halo_);
	boost::hash_combine(hash, a.halo_mod_);
	boost::hash_combine(hash, a.halo_x_);
	boost::hash_combine(hash, a.halo_y_);

	boost::hash_combine(hash, a.sound_);
	boost::hash_combine(hash, a.text_);
	boost::hash_combine(hash, a.text_color_);
	boost::hash_combine(hash, a.blend_with_);
	boost::hash_combine(hash, a.blend_ratio_);
	boost::hash_combine(hash, a.highlight_ratio_);
	boost::hash_combine(hash, a.offset_);
	boost::hash_combine(hash, a.submerge_);

	boost::hash_combine(hash, a.x_);
	boost::hash_combine(hash, a.y_);
	boost::hash_combine(hash, a.directional_x_);
	boost::hash_combine(hash, a.directional_y_);
	boost::hash_combine(hash, a.auto_vflip_);
	boost::hash_combine(hash, a.auto_hflip_);
	boost::hash_combine(hash, a.primary_frame_);
	boost::hash_combine(hash, a.drawing_layer_);

	return hash;
}


bool frame_parsed_parameters::does_not_change() const
{
	return halo_.does_not_change() &&
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
	if(!halo_.does_not_change() ||
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
	result.image = image_;
	result.image_diagonal = image_diagonal_;
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
		, const n_token::t_token& highlight
		, const n_token::t_token& blend_ratio
		, Uint32 blend_color
		, const n_token::t_token& offset
		, const n_token::t_token& layer
		, const n_token::t_token& modifiers)
{

	if(!highlight.empty()) {
		highlight_ratio_ = progressive_double(highlight,duration);
	} else if(duration != duration_){
		highlight_ratio_ = progressive_double(highlight_ratio_.get_original(),duration);
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
		image_mod_ = n_token::t_token( image_mod_ + modifiers );
	}

	if(duration != duration_) {
		halo_ = progressive_token(halo_.get_original(),duration);
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


void unit_frame::redraw(const int frame_time,bool first_time,const map_location & src,const map_location & dst,int*halo_id,const frame_parameters & animation_val,const frame_parameters & engine_val)const
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

	int d2 = game_display::get_singleton()->hex_size() / 2;
	if(first_time ) {
		// stuff sthat should be done only once per frame
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
		image_loc = image::locator(current_data.image_diagonal, n_token::t_token(current_data.image_mod)); //todo remove
	}
	if(image_loc.is_void() || image_loc.get_filename() == n_token::t_token::z_empty()) { // invalid diag image, or not diagonal
		image_loc = image::locator(current_data.image,n_token::t_token(current_data.image_mod)); //rmove extra contructor
	}

	surface image;
	if(!image_loc.is_void() && image_loc.get_filename() != n_token::t_token::z_empty()) { // invalid diag image, or not diagonal
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
	game_display* disp = game_display::get_singleton();
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
		image_loc = image::locator(current_data.image_diagonal, n_token::t_token(current_data.image_mod));
	}
	if(image_loc.is_void() || image_loc.get_filename() == n_token::t_token::z_empty()) { // invalid diag image, or not diagonal
		image_loc = image::locator(current_data.image, n_token::t_token(current_data.image_mod));
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
			if(!image_loc.is_void() && image_loc.get_filename() != n_token::t_token::z_empty()) { // invalid diag image, or not diagonal
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
			// if we need to update ourselve because we changed, invalidate our hexes
			// and return whether or not our hexs was invalidated
			// invalidate ouself to be called at redraw time
			result.insert(src);
			display::rect_of_hexes underlying_hex = disp->hexes_under_rect(r);
			result.insert(underlying_hex.begin(),underlying_hex.end());
		} else {
			// we have no "redraw surface" but we still need to invalidate our own hex
			// in case we have a halo and/or sound that needs a redraw
			// invalidate ouself to be called at redraw time
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
	const frame_parameters & current_val = static_cast<frame_parsed_parameters const &>(builder_).parameters(current_time);

	result.primary_frame = engine_val.primary_frame;
	if(animation_val.primary_frame != t_unset) result.primary_frame = animation_val.primary_frame;
	if(current_val.primary_frame != t_unset) result.primary_frame = current_val.primary_frame;
	const bool primary = result.primary_frame;

	/** engine provides a default image to use for the unit when none is available */
	result.image = current_val.image.is_void() || current_val.image.get_filename() == n_token::t_token::z_empty() ?animation_val.image:current_val.image;
	if(primary && ( result.image.is_void() || result.image.get_filename().empty())) {
		result.image = engine_val.image;
	}

	/** engine provides a default image to use for the unit when none is available */
	result.image_diagonal = current_val.image_diagonal.is_void() || current_val.image_diagonal.get_filename() == n_token::t_token::z_empty() ?animation_val.image_diagonal:current_val.image_diagonal;
	if(primary && ( result.image_diagonal.is_void() || result.image_diagonal.get_filename().empty())) {
		result.image_diagonal = engine_val.image_diagonal;
	}

	/** engine provides a string for "petrified" and "team color" modifications
          note that image_mod is the complete modification and halo_mod is only the TC part
          see unit.cpp, we know that and use it*/
	result.image_mod = current_val.image_mod + animation_val.image_mod;
	if(primary) {
		result.image_mod +=  engine_val.image_mod;
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
	if(primary && engine_val.blend_ratio) result.blend_ratio = std::min(result.blend_ratio + engine_val.blend_ratio, (double) 1.0);

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
