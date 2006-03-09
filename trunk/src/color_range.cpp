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

#include "global.hpp"
#include "color_range.hpp"
#include "serialization/string_utils.hpp"

#include <string>
#include <vector>


std::vector<Uint32> string2rgb(std::string s){
  std::vector<Uint32> out;
  std::vector<std::string> rgb_vec = utils::split(s);
  std::vector<std::string>::iterator c=rgb_vec.begin();
  int r,g,b;

  while(c!=rgb_vec.end()){
    r = (atoi(c->c_str()));
    c++;
    if(c != rgb_vec.end()){
      g = (atoi(c->c_str()));
    }else{
      g=0;
    }
    c++;
    if(c != rgb_vec.end()){
      b=(atoi(c->c_str()));
    }else{
      b=0;
    }
    c++;
     out.push_back((Uint32) ((r<<16 & 0x00FF0000) + (g<<8 & 0x0000FF00) + (b & 0x000000FF)));
   }
  return(out);
}
