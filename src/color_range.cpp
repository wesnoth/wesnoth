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
#include "map/map.hpp"
#include "serialization/string_utils.hpp"
#include "util.hpp"

#include <iomanip>

std::map<uint32_t, uint32_t> recolor_range(const color_range& new_range, const std::vector<uint32_t>& old_rgb){
	std::map<uint32_t, uint32_t> map_rgb;

	uint16_t new_red  = (new_range.mid() & 0x00FF0000)>>16;
	uint16_t new_green= (new_range.mid() & 0x0000FF00)>>8;
	uint16_t new_blue = (new_range.mid() & 0x000000FF);
	uint16_t max_red  = (new_range.max() & 0x00FF0000)>>16;
	uint16_t max_green= (new_range.max() & 0x0000FF00)>>8 ;
	uint16_t max_blue = (new_range.max() & 0x000000FF)    ;
	uint16_t min_red  = (new_range.min() & 0x00FF0000)>>16;
	uint16_t min_green= (new_range.min() & 0x0000FF00)>>8 ;
	uint16_t min_blue = (new_range.min() & 0x000000FF)    ;

	// Map first color in vector to exact new color
	uint32_t temp_rgb= old_rgb.empty() ? 0 : old_rgb[0];
	uint16_t old_r=(temp_rgb & 0X00FF0000)>>16;
	uint16_t old_g=(temp_rgb & 0X0000FF00)>>8;
	uint16_t old_b=(temp_rgb & 0X000000FF);
	uint16_t reference_avg = (( old_r + old_g + old_b) / 3);

	for(std::vector< uint32_t >::const_iterator temp_rgb2 = old_rgb.begin();
	      temp_rgb2 != old_rgb.end(); ++temp_rgb2)
	{
		old_r=((*temp_rgb2) & 0X00FF0000)>>16;
		old_g=((*temp_rgb2) & 0X0000FF00)>>8;
		old_b=((*temp_rgb2) & 0X000000FF);

		const uint16_t old_avg = (( old_r + old_g +  old_b) / 3);
	     // Calculate new color
		uint32_t new_r, new_g, new_b;

		if(reference_avg && old_avg <= reference_avg){
			float old_rat = static_cast<float>(old_avg)/reference_avg;
			new_r=uint32_t( old_rat * new_red   + (1 - old_rat) * min_red);
			new_g=uint32_t( old_rat * new_green + (1 - old_rat) * min_green);
			new_b=uint32_t( old_rat * new_blue  + (1 - old_rat) * min_blue);
		}else if(255 - reference_avg){
			float old_rat = (255.0f - static_cast<float>(old_avg)) /
				(255.0f - reference_avg);

			new_r=static_cast<uint32_t>( old_rat * new_red   + (1 - old_rat) * max_red);
			new_g=static_cast<uint32_t>( old_rat * new_green + (1 - old_rat) * max_green);
			new_b=static_cast<uint32_t>( old_rat * new_blue  + (1 - old_rat) * max_blue);
		}else{
			new_r=0; new_g=0; new_b=0; // Suppress warning
			assert(false);
			// Should never get here.
			// Would imply old_avg > reference_avg = 255
	     }

		if(new_r>255) new_r=255;
		if(new_g>255) new_g=255;
		if(new_b>255) new_b=255;

		uint32_t newrgb = (new_r << 16) + (new_g << 8) + (new_b );
		map_rgb[*temp_rgb2]=newrgb;
	}

	return map_rgb;
}

bool string2rgb(const std::string& s, std::vector<uint32_t>& result) {
	result = std::vector<uint32_t>();
	std::vector<uint32_t> out;
	std::vector<std::string> rgb_vec = utils::split(s);
	std::vector<std::string>::iterator c=rgb_vec.begin();
	while(c!=rgb_vec.end())
	{
		uint32_t rgb_hex;
		if(c->length() != 6)
		{
			try {
				// integer triplets, e.g. white="255,255,255"
				rgb_hex =  (0x00FF0000 & (std::stoi(*c++))<<16); //red
				if(c!=rgb_vec.end())
				{
					rgb_hex += (0x0000FF00 & (std::stoi(*c++)<<8)); //green
					if(c!=rgb_vec.end())
					{
						rgb_hex += (0x000000FF & (std::stoi(*c++)<<0)); //blue
					}
				}
			} catch (std::invalid_argument&) {
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

std::vector<uint32_t> palette(color_range cr){
// generate a color palette from a color range
	std::vector<uint32_t> temp,res;
	std::set<uint32_t> clist;
	// use blue to make master set of possible colors
	for(int i=255;i!=0;i--){
		int j=255-i;
		uint32_t rgb = i;
		temp.push_back(rgb);
		rgb = (j << 16) + (j << 8) + 255;
		temp.push_back(rgb);
	}

	// Use recolor function to generate list of possible colors.
	// Could use a special function, would be more efficient,
	// but harder to maintain.
	std::map<uint32_t,uint32_t> cmap = recolor_range(cr,temp);
	for(std::map<uint32_t,uint32_t>::const_iterator k=cmap.begin(); k!=cmap.end();++k){
		clist.insert(k->second);
	}
	res.push_back(cmap[255]);
	for(std::set<uint32_t>::const_iterator c=clist.begin();c!=clist.end();++c){
		if(*c != res[0] && *c!=0 && *c != 0x00FFFFFF){
			res.push_back(*c);}
	}
	return(res);
}

std::string color_range::debug() const
{
	std::ostringstream o;

	static const uint32_t mask = 0x00FFFFFF;

	o << std::hex << std::setfill('0')
	  << '{' << std::setw(6) << (mid_ & mask)
	  << ',' << std::setw(6) << (max_ & mask)
	  << ',' << std::setw(6) << (min_ & mask)
	  << ',' << std::setw(6) << (rep_ & mask)
	  << '}';

	return o.str();
}
