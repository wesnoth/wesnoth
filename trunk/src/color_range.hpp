/* $Id: team.hpp 9124 2005-12-12 04:09:50Z darthfool $ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef COLOR_RANGE_H_INCLUDED
#define COLOR_RANGE_H_INCLUDED

#include "global.hpp"

#include <vector>
#include "SDL_types.h"

//convert comma seperated string into rgb values
std::vector<Uint32> string2rgb(std::string s);

class color_range
{
public:
  color_range(Uint32 mid = 0x00808080 , Uint32 max = 0x00FFFFFF, Uint32 min = 0x00000000):mid_(mid),max_(max),min_(min){};
  color_range(const std::vector<Uint32>& v)
  {
    mid_ = v.size() ? v[0] : 0x00808080;
    max_ = v.size() ? v[1] : 0x00FFFFFF;
    min_ = v.size() ? v[2] : 0x00000000;
  };
  Uint32 mid() const{return(mid_);};
  Uint32 max() const{return(max_);};
  Uint32 min() const{return(min_);};
  bool operator<(const color_range& b) const
  {
    if(mid_ != b.mid()) return(mid_ < b.mid());
    if(max_ != b.max()) return(max_ < b.max());
    return(min_ < b.min());
  }
  bool operator==(const color_range& b) const
  {
    return(mid_ == b.mid() && max_ == b.max() && min_ == b.min());
  }
private:
  Uint32 mid_ , max_ , min_;
};

#endif
