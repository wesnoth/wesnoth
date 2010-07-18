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

/** @file unit_frame.cpp */

#include "global.hpp"

#include "game_display.hpp"
#include "halo.hpp"
#include "sound.hpp"
#include "unit_frame.hpp"

#define UNIT_FRAME_H_PART2


progressive_string::progressive_string(const std::string & data,int duration) :
	data_(),
	input_(data)
{
		const std::vector<std::string> first_pass = utils::split(data);
		const int time_chunk = std::max<int>(duration / (first_pass.size()?first_pass.size():1),1);

		std::vector<std::string>::const_iterator tmp;
		for(tmp=first_pass.begin();tmp != first_pass.end() ; ++tmp) {
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
static const std::string empty_string ="";
const std::string& progressive_string::get_current_element(int current_time)const
{
	int time = 0;
	unsigned int sub_halo = 0;
	if(data_.empty()) return empty_string;
	while(time < current_time&& sub_halo < data_.size()) {
		time += data_[sub_halo].second;
		++sub_halo;

	}
	if(sub_halo) sub_halo--;
	if(sub_halo >= data_.size()) sub_halo = data_.size();
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

#undef UNIT_FRAME_H_PART2

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
	drawing_layer(display::LAYER_UNIT_DEFAULT - display::LAYER_UNIT_FIRST),
	in_hex(false),
	diagonal_in_hex(false)
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
	drawing_layer_(str_cast(display::LAYER_UNIT_DEFAULT - display::LAYER_UNIT_FIRST))
{}

frame_builder::frame_builder(const config& cfg,const std::string& frame_string) :
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
	drawing_layer_("")
{
	image(image::locator(cfg[frame_string+"image"]),cfg[frame_string+"image_mod"]);
	image_diagonal(image::locator(cfg[frame_string+"image_diagonal"]),cfg[frame_string+"image_mod"]);
	sound(cfg[frame_string+"sound"]);
	std::vector<std::string> tmp_string_vect=utils::split(cfg[frame_string+"text_color"]);
	if(tmp_string_vect.size() ==3) {
		text(cfg[frame_string+"text"],
		 display::rgb(atoi(tmp_string_vect[0].c_str()),atoi(tmp_string_vect[1].c_str()),atoi(tmp_string_vect[2].c_str())));
	} else {
		text(cfg[frame_string+"text"],0);
	}

	if(!cfg[frame_string+"duration"].empty()) {
		duration(cfg[frame_string + "duration"]);
	} else {
		duration(cfg[frame_string + "end"].to_int() - cfg[frame_string + "begin"].to_int());
	}
	halo(cfg[frame_string+"halo"],cfg[frame_string+"halo_x"],cfg[frame_string+"halo_y"],cfg[frame_string+"halo_mod"]);
	 tmp_string_vect=utils::split(cfg[frame_string+"blend_color"]);
	if(tmp_string_vect.size() ==3) {
		blend(cfg[frame_string+"blend_ratio"],display::rgb(atoi(tmp_string_vect[0].c_str()),atoi(tmp_string_vect[1].c_str()),atoi(tmp_string_vect[2].c_str())));
	} else {
		blend(cfg[frame_string+"blend_ratio"],0);
	}
	highlight(cfg[frame_string+"alpha"]);
	offset(cfg[frame_string+"offset"]);
	submerge(cfg[frame_string+"submerge"]);
	x(cfg[frame_string+"x"]);
	y(cfg[frame_string+"y"]);
	drawing_layer(cfg[frame_string+"layer"]);
}

frame_builder & frame_builder::image(const image::locator& image ,const std::string & image_mod)
{
	image_ = image;
	image_mod_ = image_mod;
	return *this;
}
frame_builder & frame_builder::image_diagonal(const image::locator& image_diagonal,const std::string& image_mod)
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
frame_builder & frame_builder::drawing_layer(const std::string& drawing_layer)
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
	drawing_layer_(builder.drawing_layer_,duration_)
{}


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
		halo_ = progressive_string(halo_.get_original(),duration);
		halo_x_ = progressive_int(halo_x_.get_original(),duration);
		halo_y_ = progressive_int(halo_y_.get_original(),duration);
		submerge_=progressive_double(submerge_.get_original(),duration);
		x_=progressive_int(x_.get_original(),duration);
		y_=progressive_int(y_.get_original(),duration);
		duration_ = duration;
	}
}


void unit_frame::redraw(const int frame_time,bool first_time,const map_location & src,const map_location & dst,int*halo_id,const frame_parameters & animation_val,const frame_parameters & engine_val,const bool primary)const
{
	const int xsrc = game_display::get_singleton()->get_location_x(src);
	const int ysrc = game_display::get_singleton()->get_location_y(src);
	const int xdst = game_display::get_singleton()->get_location_x(dst);
	const int ydst = game_display::get_singleton()->get_location_y(dst);
	const map_location::DIRECTION direction = src.get_relative_dir(dst);

	const frame_parameters current_data = merge_parameters(frame_time,animation_val,engine_val,primary);
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
		if(primary) facing_north = true;
		game_display::get_singleton()->render_image(x + current_data.x- image->w/2,
			       	y  + current_data.y- image->h/2,
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
				if(primary) {
					orientation = halo::NORMAL;
				} else {
					orientation = halo::VREVERSE;
				}
				break;
			case map_location::SOUTH_WEST:
				if(primary) {
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
std::set<map_location> unit_frame::get_overlaped_hex(const int frame_time,const map_location & src,const map_location & dst,const frame_parameters & animation_val,const frame_parameters & engine_val,const bool primary) const
{
	game_display* disp = game_display::get_singleton();
	const int xsrc = disp->get_location_x(src);
	const int ysrc = disp->get_location_y(src);
	const int xdst = disp->get_location_x(dst);
	const int ydst = disp->get_location_y(dst);
	const map_location::DIRECTION direction = src.get_relative_dir(dst);

	const frame_parameters current_data = merge_parameters(frame_time,animation_val,engine_val,primary);
	double tmp_offset = current_data.offset;
	int d2 = game_display::get_singleton()->hex_size() / 2;

	image::locator image_loc;
	if(direction != map_location::NORTH && direction != map_location::SOUTH) {
		image_loc = current_data.image_diagonal;
	}
	if(image_loc.is_void() || image_loc.get_filename() == "") { // invalid diag image, or not diagonal
		image_loc = current_data.image;
	}

	// we always invalidate our own hex because we need to be called at redraw time even
	// if we don't draw anything in the hex itself
	std::set<map_location> result;
	if(tmp_offset==0 && current_data.x == 0 && current_data.y == 0 && image::is_in_hex(image_loc)) {
		result.insert(src);
	} else {
		surface image;
		if(!image_loc.is_void() && image_loc.get_filename() != "") { // invalid diag image, or not diagonal
			image=image::get_image(image_loc,
					image::SCALED_TO_ZOOM
					);
		}
		if (image != NULL) {
			const int x = static_cast<int>(tmp_offset * xdst + (1.0-tmp_offset) * xsrc)+current_data.x+d2-(image->w/2);
			const int y = static_cast<int>(tmp_offset * ydst + (1.0-tmp_offset) * ysrc)+current_data.y+d2-(image->h/2);
			const SDL_Rect r = create_rect(x, y, image->w, image->h);
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



const frame_parameters unit_frame::merge_parameters(int current_time,const frame_parameters & animation_val,const frame_parameters & engine_val, bool primary) const
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

	assert(engine_val.drawing_layer == display::LAYER_UNIT_DEFAULT-display::LAYER_UNIT_FIRST);
	result.drawing_layer = current_val.drawing_layer !=  display::LAYER_UNIT_DEFAULT-display::LAYER_UNIT_FIRST?
		current_val.drawing_layer:animation_val.drawing_layer;

#ifdef LOW_MEM
	if(primary) {
		result.image= engine_val.image;
		result.image_diagonal= engine_val.image;
	}
#endif
	return result;
}
