/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file color_range.hpp
//!

#ifndef COLOR_RANGE_H_INCLUDED
#define COLOR_RANGE_H_INCLUDED

//These macros interfere with MS VC++
#ifdef _MSC_VER
	#undef max
	#undef min
#endif

#include "global.hpp"
#include <map>
#include <string>
#include <vector>

#include "SDL_types.h"

//- Convert comma separated string into rgb values
std::vector<Uint32> string2rgb(std::string s);

class color_range
{
public:
  color_range(Uint32 mid , Uint32 max = 0x00FFFFFF , Uint32 min = 0x00000000 , Uint32 rep = 0x00808080):mid_(mid),max_(max),min_(min),rep_(rep){};
  color_range(const std::vector<Uint32>& v)
    : mid_(v.size()     ? v[0] : 0x00808080),
      max_(v.size() > 1 ? v[1] : 0x00FFFFFF),
      min_(v.size() > 2 ? v[2] : 0x00000000),
      rep_(v.size() > 3 ? v[3] : mid_)
  {
  };
  color_range() : mid_(0x00808080), max_(0x00FFFFFF), min_(0x00000000), rep_(0x00808080) {};
  Uint32 mid() const{return(mid_);};
  Uint32 max() const{return(max_);};
  Uint32 min() const{return(min_);};
//note: use mid() instead of rep() unless high contrast is needed over a map or minimap!
  Uint32 rep() const{return(rep_);};
  bool operator<(const color_range& b) const
  {
    if(mid_ != b.mid()) return(mid_ < b.mid());
    if(max_ != b.max()) return(max_ < b.max());
    if(min_ != b.min()) return(min_ < b.min());
    return(rep_ < b.rep());
  }
  bool operator==(const color_range& b) const
  {
    return(mid_ == b.mid() && max_ == b.max() && min_ == b.min() && rep_ == b.rep());
  }
  int index() const; // the default team index for this color, or 0 for none
private:
  Uint32 mid_ , max_ , min_ , rep_;
};

std::vector<Uint32> palette(color_range cr); //return color palette from color range
std::map<Uint32, Uint32> recolor_range(const color_range& new_rgb, const std::vector<Uint32>& old_rgb);
std::string rgb2highlight(Uint32 rgb);
#endif
