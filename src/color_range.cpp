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
#include "sdl_utils.hpp"

#include <string>
#include <vector>
#include <set>

std::vector<Uint32> string2rgb(std::string s){
  std::vector<Uint32> out;
  std::vector<std::string> rgb_vec = utils::split(s);

  while(rgb_vec.size()%3) {
	rgb_vec.push_back("0");
  }

  std::vector<std::string>::iterator c=rgb_vec.begin();
  int r,g,b;

  while(c!=rgb_vec.end()){
    r = (lexical_cast<int>(*c));
    c++;
    g = (lexical_cast<int>(*c));
    c++;
    b=(lexical_cast<int>(*c));
    c++;
    out.push_back((Uint32) ((r<<16 & 0x00FF0000) + (g<<8 & 0x0000FF00) + (b & 0x000000FF)));
   }
  return(out);
}

std::vector<Uint32> palette(color_range cr){
//generate a color palette from a color range
	std::vector<Uint32> temp,res;
	std::set<Uint32> clist;
	//use blue to make master set of possible colors
	for(int i=255;i!=0;i--){
		int j=255-i;
		Uint32 rgb = i;
		temp.push_back(rgb);
		rgb = (j << 16) + (j << 8) + 255;
		temp.push_back(rgb); 		
	}
	
	//use recolor function to generate list of possible colors.
	//could use a special function, would be more efficient, but
	//harder to maintain.
	std::map<Uint32,Uint32> cmap = recolor_range(cr,temp);
	for(std::map<Uint32,Uint32>::const_iterator k=cmap.begin(); k!=cmap.end();k++){
		clist.insert(k->second);		
	}
	res.push_back(cmap[255]);
	for(std::set<Uint32>::const_iterator c=clist.begin();c!=clist.end();c++){
		if(*c != res[0] && *c!=0 && *c != 0x00FFFFFF){
			res.push_back(*c);}			
	}
	return(res);
}
