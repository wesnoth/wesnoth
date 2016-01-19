/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Generate ranges of colors, and color palettes.
 * Used e.g. to color HP, XP.
 */

#include "color_range.hpp"

#include "game_config.hpp"
#include "map.hpp"
#include "serialization/string_utils.hpp"
#include "util.hpp"

#include <iomanip>

std::map<Uint32, Uint32> recolor_range(const color_range& new_range, const std::vector<Uint32>& old_rgb){
	std::map<Uint32, Uint32> map_rgb;

	Uint16 new_red  = (new_range.mid() & 0x00FF0000)>>16;
	Uint16 new_green= (new_range.mid() & 0x0000FF00)>>8;
	Uint16 new_blue = (new_range.mid() & 0x000000FF);
	Uint16 max_red  = (new_range.max() & 0x00FF0000)>>16;
	Uint16 max_green= (new_range.max() & 0x0000FF00)>>8 ;
	Uint16 max_blue = (new_range.max() & 0x000000FF)    ;
	Uint16 min_red  = (new_range.min() & 0x00FF0000)>>16;
	Uint16 min_green= (new_range.min() & 0x0000FF00)>>8 ;
	Uint16 min_blue = (new_range.min() & 0x000000FF)    ;

	// Map first color in vector to exact new color
	Uint32 temp_rgb= old_rgb.empty() ? 0 : old_rgb[0];
	Uint16 old_r=(temp_rgb & 0X00FF0000)>>16;
	Uint16 old_g=(temp_rgb & 0X0000FF00)>>8;
	Uint16 old_b=(temp_rgb & 0X000000FF);
	Uint16 reference_avg = (( old_r + old_g + old_b) / 3);

	for(std::vector< Uint32 >::const_iterator temp_rgb2 = old_rgb.begin();
	      temp_rgb2 != old_rgb.end(); ++temp_rgb2)
	{
		Uint16 old_r=((*temp_rgb2) & 0X00FF0000)>>16;
		Uint16 old_g=((*temp_rgb2) & 0X0000FF00)>>8;
		Uint16 old_b=((*temp_rgb2) & 0X000000FF);

		const Uint16 old_avg = (( old_r + old_g +  old_b) / 3);
	     // Calculate new color
		Uint32 new_r, new_g, new_b;

		if(reference_avg && old_avg <= reference_avg){
			float old_rat = static_cast<float>(old_avg)/reference_avg;
			new_r=Uint32( old_rat * new_red   + (1 - old_rat) * min_red);
			new_g=Uint32( old_rat * new_green + (1 - old_rat) * min_green);
			new_b=Uint32( old_rat * new_blue  + (1 - old_rat) * min_blue);
		}else if(255 - reference_avg){
			float old_rat = (255.0f - static_cast<float>(old_avg)) /
				(255.0f - reference_avg);

			new_r=static_cast<Uint32>( old_rat * new_red   + (1 - old_rat) * max_red);
			new_g=static_cast<Uint32>( old_rat * new_green + (1 - old_rat) * max_green);
			new_b=static_cast<Uint32>( old_rat * new_blue  + (1 - old_rat) * max_blue);
		}else{
			new_r=0; new_g=0; new_b=0; // Suppress warning
			assert(false);
			// Should never get here.
			// Would imply old_avg > reference_avg = 255
	     }

		if(new_r>255) new_r=255;
		if(new_g>255) new_g=255;
		if(new_b>255) new_b=255;

		Uint32 newrgb = (new_r << 16) + (new_g << 8) + (new_b );
		map_rgb[*temp_rgb2]=newrgb;
	}

	return map_rgb;
}

bool string2rgb(const std::string& s, std::vector<Uint32>& result) {
	result = std::vector<Uint32>();
	std::vector<Uint32> out;
	std::vector<std::string> rgb_vec = utils::split(s);
	std::vector<std::string>::iterator c=rgb_vec.begin();
	while(c!=rgb_vec.end())
	{
		Uint32 rgb_hex;
		if(c->length() != 6)
		{
			try {
				// integer triplets, e.g. white="255,255,255"
				rgb_hex =  (0x00FF0000 & ((lexical_cast<int>(*c++))<<16)); //red
				if(c!=rgb_vec.end())
				{
					rgb_hex += (0x0000FF00 & ((lexical_cast<int>(*c++))<<8)); //green
					if(c!=rgb_vec.end())
					{
						rgb_hex += (0x000000FF & ((lexical_cast<int>(*c++))<<0)); //blue
					}
				}
			} catch (bad_lexical_cast&) {
				return false;
			}
		} else {
			// hexadecimal format, e.g. white="FFFFFF"
			char* endptr;
			rgb_hex = (0x00FFFFFF & strtol(c->c_str(), &endptr, 16));
			if (*endptr != '\0') {
				return false;
			}
			++c;
		}
		out.push_back(rgb_hex);
	}
	result = out;
	return true;
}

std::vector<Uint32> palette(color_range cr){
// generate a color palette from a color range
	std::vector<Uint32> temp,res;
	std::set<Uint32> clist;
	// use blue to make master set of possible colors
	for(int i=255;i!=0;i--){
		int j=255-i;
		Uint32 rgb = i;
		temp.push_back(rgb);
		rgb = (j << 16) + (j << 8) + 255;
		temp.push_back(rgb);
	}

	// Use recolor function to generate list of possible colors.
	// Could use a special function, would be more efficient,
	// but harder to maintain.
	std::map<Uint32,Uint32> cmap = recolor_range(cr,temp);
	for(std::map<Uint32,Uint32>::const_iterator k=cmap.begin(); k!=cmap.end();++k){
		clist.insert(k->second);
	}
	res.push_back(cmap[255]);
	for(std::set<Uint32>::const_iterator c=clist.begin();c!=clist.end();++c){
		if(*c != res[0] && *c!=0 && *c != 0x00FFFFFF){
			res.push_back(*c);}
	}
	return(res);
}

std::string rgb2highlight(Uint32 rgb)
{
	std::ostringstream h;
	// Must match what the escape interpreter for marked-up-text expects
	h << "<" << ((rgb & 0xFF0000) >> 16)
	  << "," << ((rgb & 0x00FF00) >> 8)
	  << "," << (rgb & 0x0000FF) << ">";
	return h.str();
}

std::string rgb2highlight_pango(Uint32 rgb)
{
	std::ostringstream h;
	// Must match what the pango expects
	h << "#" << std::hex << std::setfill('0') << std::setw(2) << ((rgb & 0xFF0000) >> 16)
	  << std::hex << std::setfill('0') << std::setw(2) <<((rgb & 0x00FF00) >> 8)
	  << std::hex << std::setfill('0') << std::setw(2) <<(rgb & 0x0000FF);
	return h.str();
}

int color_range::index() const
{
	for(int i = 1; i <= gamemap::MAX_PLAYERS; ++i) {
		if(*this==(game_config::color_info(lexical_cast<std::string>(i)))) {
			return i;
		}
	}
	return 0;
}

std::string color_range::debug() const
{
	std::ostringstream o;

	static const Uint32 mask = 0x00FFFFFF;

	o << std::hex << std::setfill('0')
	  << '{' << std::setw(6) << (mid_ & mask)
	  << ',' << std::setw(6) << (max_ & mask)
	  << ',' << std::setw(6) << (min_ & mask)
	  << ',' << std::setw(6) << (rep_ & mask)
	  << '}';

	return o.str();
}
