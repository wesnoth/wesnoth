/*
   wesnoth menu styles Copyright (C) 2006 by Patrick Parker <patrick_x99@hotmail.com>
   wesnoth menu Copyright (C) 2003-5 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/


#include "global.hpp"

#include "widgets/menu.hpp"

#include "language.hpp"
#include "font.hpp"
#include "image.hpp"
#include "marked-up_text.hpp"
#include "sdl_utils.hpp"
#include "SDL_image.h"
#include "util.hpp"
#include "video.hpp"
#include "wml_separators.hpp"
#include "serialization/string_utils.hpp"

#include <algorithm>
#include <cassert>
#include <numeric>

namespace {
//const size_t menu_font_size = font::SIZE_NORMAL;
//const size_t menu_cell_padding = font::SIZE_NORMAL * 3/5;
//gui::menu::style gui::menu::default_style;
//gui::menu::imgsel_style gui::menu::slateborder_style("misc/selection");
}

namespace gui {

	//static initializations
menu::style gui::menu::default_style;
#ifdef WESNOTH_PATH
menu::imgsel_style gui::menu::slateborder_style( WESNOTH_PATH + "/images/misc/selection");
#else
menu::imgsel_style gui::menu::slateborder_style("./images/misc/selection");
#endif

	//constructors
menu::style::style() : font_size_(font::SIZE_NORMAL), cell_padding_(font::SIZE_NORMAL * 3/5),
		normal_rgb_(0x000000), normal_alpha_(0.2),
		selected_rgb_(0x990000), selected_alpha_(0.6),
		heading_rgb_(0x333333), heading_alpha_(0.3)
{}
menu::imgsel_style::imgsel_style(const std::string &img_base) : img_base_(img_base), thickness_(3),
			initialized_(false), load_failed_(false)
{}

size_t menu::style::get_font_size() const { return font_size_; }
size_t menu::style::get_cell_padding() { return cell_padding_; }



bool menu::imgsel_style::load_image(const std::string &img_sub)
{
	std::string path = img_base_ + "-" + img_sub + ".png";
	surface image(IMG_Load(path.c_str()));
	img_map_[img_sub] = image;
	return(!image.null());
}

bool menu::imgsel_style::load_images()
{
		if(    load_image("border-botleft")
			&& load_image("border-botright")
			&& load_image("border-topleft")
			&& load_image("border-topright")
			&& load_image("border-left")
			&& load_image("border-right")
			&& load_image("border-top")
			&& load_image("border-bottom") )
		{
			cell_padding_ = maximum(cell_padding_, thickness_);

			selected_rgb_ = 0x000000;
			selected_alpha_ = 0.9;
			normal_rgb_ = 0x4a4440;
			normal_alpha_ = 0.2;
			heading_rgb_ = 0x999999;
			heading_alpha_ = 0.2;

			initialized_ = true;
			load_failed_ = false;
		}
		else
		{
			initialized_ = true;
			load_failed_ = true;
		}
		return (!load_failed_);
}

void menu::imgsel_style::draw_row(const menu& menu_ref, const std::vector<std::string>& row, const SDL_Rect& rect, ROW_TYPE type)
{
	if(!initialized_) load_images();
	style::draw_row(menu_ref, row, rect, type);
	if(type == SELECTED_ROW) {

		/* Draw border if it was succesfully loaded. */
		if(!load_failed_) {
			surface image;
			SDL_Rect area;
			SDL_Rect clip = rect;
			area.x = rect.x;
			area.y = rect.y;
			//area.w = minimum(image->w,rect->w);
			//area.h = minimum(image->h,rect->h);
			//SDL_BlitSurface(image, 0, gdis, &area);

			image = img_map_["border-top"];
			area.x = rect.x;
			area.y = rect.y;
			do {
				menu_ref.video().blit_surface(area.x,area.y,image,NULL,&clip);
				area.x += image->w;
			} while( area.x < rect.x + rect.w );

			image = img_map_["border-left"];
			area.x = rect.x;
			area.y = rect.y;
			do {
				menu_ref.video().blit_surface(area.x,area.y,image,NULL,&clip);
				area.y += image->h;
			} while( area.y < rect.y + rect.h );

			image = img_map_["border-topleft"];
			area.x = rect.x;
			area.y = rect.y;
			menu_ref.video().blit_surface(area.x,area.y,image);

			image = img_map_["border-right"];
			area.x = rect.x + rect.w - thickness_;
			area.y = rect.y;
			do {
				menu_ref.video().blit_surface(area.x,area.y,image,NULL,&clip);
				area.y += image->h;
			} while( area.y < rect.y + rect.h );

			image = img_map_["border-topright"];
			area.x = rect.x + rect.w - image->w;
			area.y = rect.y;
			menu_ref.video().blit_surface(area.x,area.y,image);

			image = img_map_["border-bottom"];
			area.x = rect.x;
			area.y = rect.y + rect.h - thickness_;
			do {
				menu_ref.video().blit_surface(area.x,area.y,image,NULL,&clip);
				area.x += image->w;
			} while( area.x < rect.x + rect.w );

			image = img_map_["border-botright"];
			area.x = rect.x + rect.w - image->w;
			area.y = rect.y + rect.h - image->h;
			menu_ref.video().blit_surface(area.x,area.y,image);

			image = img_map_["border-botleft"];
			area.x = rect.x;
			area.y = rect.y + rect.h - image->h;
			menu_ref.video().blit_surface(area.x,area.y,image);

		}
	}
}

SDL_Rect menu::imgsel_style::item_size(const std::string& item) const
{
	return style::item_size(item);
}

size_t menu::imgsel_style::get_cell_padding()
{
	if(!initialized_) load_images();
	return cell_padding_;
}

}
