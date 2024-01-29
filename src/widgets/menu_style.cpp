/*
	Copyright (C) 2006 - 2024
	by Patrick Parker <patrick_x99@hotmail.com>
	Copyright (C) 2003 - 2005 by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "widgets/menu.hpp"

#include "draw.hpp"
#include "font/constants.hpp"
#include "picture.hpp"
#include "lexical_cast.hpp"
#include "sdl/utils.hpp"

namespace gui {

	//static initializations
menu::imgsel_style menu::bluebg_style("dialogs/selection", true,
										   0x000000, 0x000000,
										   0.35, 0.0);

menu::style &menu::default_style = menu::bluebg_style;

	//constructors
menu::style::style() : font_size_(font::SIZE_NORMAL),
		cell_padding_(font::SIZE_NORMAL * 3/5), thickness_(0),
		normal_rgb_(0x000000), selected_rgb_(0x000099),
		normal_alpha_(0.2),  selected_alpha_(0.6)
{}

menu::style::~style()
{}
menu::imgsel_style::imgsel_style(const std::string &img_base, bool has_bg,
								 int normal_rgb, int selected_rgb,
								 double normal_alpha, double selected_alpha)
								 : img_base_(img_base), has_background_(has_bg),  initialized_(false), load_failed_(false),
								 normal_rgb2_(normal_rgb), selected_rgb2_(selected_rgb),
								 normal_alpha2_(normal_alpha), selected_alpha2_(selected_alpha)
{}
menu::imgsel_style::~imgsel_style()
{}

std::size_t menu::style::get_font_size() const { return font_size_; }
std::size_t menu::style::get_cell_padding() const { return cell_padding_; }
std::size_t menu::style::get_thickness() const { return thickness_; }

bool menu::imgsel_style::load_image(const std::string &img_sub)
{
	std::string path = img_base_ + "-" + img_sub + ".png";
	const texture image = image::get_texture(path);
	img_map_[img_sub] = image;
	return bool(image);
}

bool menu::imgsel_style::load_images()
{
	if(!initialized_)
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
			thickness_ = std::min(
					img_map_["border-top"].h(),
					img_map_["border-left"].w());


			if(has_background_ && !load_image("background"))
			{
				load_failed_ = true;
			}
			else
			{
				normal_rgb_ = normal_rgb2_;
				normal_alpha_ = normal_alpha2_;
				selected_rgb_ = selected_rgb2_;
				selected_alpha_ = selected_alpha2_;

				load_failed_ = false;
			}
			initialized_ = true;
		}
		else
		{
			thickness_ = 0;
			initialized_ = true;
			load_failed_ = true;
		}
	}
	return (!load_failed_);
}

void menu::imgsel_style::unload_images()
{
	img_map_.clear();
}

void menu::imgsel_style::draw_row_bg(menu& menu_ref, const std::size_t row_index, const SDL_Rect& rect, ROW_TYPE type)
{
	if(type == SELECTED_ROW && has_background_ && !load_failed_) {
		draw::blit(img_map_["background"], rect);
	}
	else {
		style::draw_row_bg(menu_ref, row_index, rect, type);
	}
}

void menu::imgsel_style::draw_row(menu& menu_ref, const std::size_t row_index, const SDL_Rect& rect, ROW_TYPE type)
{
	if(!load_failed_) {
		//draw item inside
		style::draw_row(menu_ref, row_index, rect, type);

		if(type == SELECTED_ROW) {
			// draw border
			texture image;
			SDL_Rect area;
			auto clipper = draw::reduce_clip(rect);
			area.x = rect.x;
			area.y = rect.y;

			image = img_map_["border-top"];
			area.x = rect.x;
			area.y = rect.y;
			area.w = image.w();
			area.h = image.h();
			do {
				draw::blit(image, area);
				area.x += area.w;
			} while( area.x < rect.x + rect.w );

			image = img_map_["border-left"];
			area.x = rect.x;
			area.y = rect.y;
			area.w = image.w();
			area.h = image.h();
			do {
				draw::blit(image, area);
				area.y += area.h;
			} while( area.y < rect.y + rect.h );

			image = img_map_["border-right"];
			area.x = rect.x + rect.w - thickness_;
			area.y = rect.y;
			area.w = image.w();
			area.h = image.h();
			do {
				draw::blit(image, area);
				area.y += area.h;
			} while( area.y < rect.y + rect.h );

			image = img_map_["border-bottom"];
			area.x = rect.x;
			area.y = rect.y + rect.h - thickness_;
			area.w = image.w();
			area.h = image.h();
			do {
				draw::blit(image, area);
				area.x += area.w;
			} while( area.x < rect.x + rect.w );

			image = img_map_["border-topleft"];
			area.x = rect.x;
			area.y = rect.y;
			area.w = image.w();
			area.h = image.h();
			draw::blit(image, area);

			image = img_map_["border-topright"];
			area.x = rect.x + rect.w - image.w();
			area.y = rect.y;
			area.w = image.w();
			area.h = image.h();
			draw::blit(image, area);

			image = img_map_["border-botleft"];
			area.x = rect.x;
			area.y = rect.y + rect.h - image.h();
			area.w = image.w();
			area.h = image.h();
			draw::blit(image, area);

			image = img_map_["border-botright"];
			area.x = rect.x + rect.w - image.w();
			area.y = rect.y + rect.h - image.h();
			area.w = image.w();
			area.h = image.h();
			draw::blit(image, area);
		}
	}
		else {
		//default drawing
		style::draw_row(menu_ref, row_index, rect, type);
	}
}

SDL_Rect menu::imgsel_style::item_size(const indented_menu_item& imi) const
{
	SDL_Rect bounds = style::item_size(imi);

	bounds.w += 2 * thickness_;
	bounds.h += 2 * thickness_ + 4;

	return bounds;
}


} //namesapce gui
