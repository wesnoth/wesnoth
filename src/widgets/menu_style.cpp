/*
   wesnoth menu styles Copyright (C) 2006 - 2015 by Patrick Parker <patrick_x99@hotmail.com>
   wesnoth menu Copyright (C) 2003-5 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "global.hpp"

#include "widgets/menu.hpp"

#include "font.hpp"
#include "image.hpp"
#include "video.hpp"

namespace gui {

	//static initializations
menu::imgsel_style menu::bluebg_style("dialogs/selection", true,
										   0x000000, 0x000000, 0x333333,
										   0.35, 0.0, 0.3);
menu::style menu::simple_style;

menu::style &menu::default_style = menu::bluebg_style;

	//constructors
menu::style::style() : font_size_(font::SIZE_NORMAL),
		cell_padding_(font::SIZE_NORMAL * 3/5), thickness_(0),
		normal_rgb_(0x000000), selected_rgb_(0x000099), heading_rgb_(0x333333),
#ifdef SDL_GPU
		normal_alpha_(50),  selected_alpha_(150), heading_alpha_(75),
#else
		normal_alpha_(0.2),  selected_alpha_(0.6), heading_alpha_(0.3),
#endif
		max_img_w_(-1), max_img_h_(-1)
{}

menu::style::~style()
{}
menu::imgsel_style::imgsel_style(const std::string &img_base, bool has_bg,
								 int normal_rgb, int selected_rgb, int heading_rgb,
								 double normal_alpha, double selected_alpha, double heading_alpha)
								 : img_base_(img_base), has_background_(has_bg),  initialized_(false), load_failed_(false),
								 normal_rgb2_(normal_rgb), selected_rgb2_(selected_rgb), heading_rgb2_(heading_rgb),
								 normal_alpha2_(normal_alpha), selected_alpha2_(selected_alpha), heading_alpha2_(heading_alpha)
{}
menu::imgsel_style::~imgsel_style()
{}

size_t menu::style::get_font_size() const { return font_size_; }
size_t menu::style::get_cell_padding() const { return cell_padding_; }
size_t menu::style::get_thickness() const { return thickness_; }

void menu::style::scale_images(int max_width, int max_height)
{
	max_img_w_ = max_width;
	max_img_h_ = max_height;
}

#ifdef SDL_GPU
sdl::timage menu::style::get_item_image(const image::locator& img_loc) const
{
	sdl::timage img = image::get_texture(img_loc);
	if(!img.null())
	{
		int scale = 100;
		if(max_img_w_ > 0 && img.width() > max_img_w_) {
			scale = (max_img_w_ * 100) / img.width();
		}
		if(max_img_h_ > 0 && img.height() > max_img_h_) {
			scale = std::min<int>(scale, ((max_img_h_ * 100) / img.height()));
		}
		if(scale != 100)
		{
			img.set_scale(scale, scale);
			return img;
		}
	}
	return img;
}
#else
surface menu::style::get_item_image(const image::locator& img_loc) const
{
	surface surf = image::get_image(img_loc);
	if(!surf.null())
	{
		int scale = 100;
		if(max_img_w_ > 0 && surf->w > max_img_w_) {
			scale = (max_img_w_ * 100) / surf->w;
		}
		if(max_img_h_ > 0 && surf->h > max_img_h_) {
			scale = std::min<int>(scale, ((max_img_h_ * 100) / surf->h));
		}
		if(scale != 100)
		{
			return scale_surface(surf, (scale * surf->w)/100, (scale * surf->h)/100);
		}
	}
	return surf;
}
#endif

bool menu::imgsel_style::load_image(const std::string &img_sub)
{
#ifdef SDL_GPU
	std::string path = img_base_ + "-" + img_sub + ".png";
	sdl::timage image = image::get_image(path);
	img_map_[img_sub] = image;
	return(!image.null());
#else
	std::string path = img_base_ + "-" + img_sub + ".png";
	const surface image = image::get_image(path);
	img_map_[img_sub] = image;
	return(!image.null());
#endif
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
#ifdef SDL_GPU
			thickness_ = std::min(
					img_map_["border-top"].height(),
					img_map_["border-left"].width());
#else
			thickness_ = std::min(
					img_map_["border-top"]->h,
					img_map_["border-left"]->w);
#endif


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
				heading_rgb_ = heading_rgb2_;
				heading_alpha_ = heading_alpha2_;

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

void menu::imgsel_style::draw_row_bg(menu& menu_ref, const size_t row_index, const SDL_Rect& rect, ROW_TYPE type)
{
#ifdef SDL_GPU
	if(type == SELECTED_ROW && has_background_ && !load_failed_) {
		background_image_.set_scale(float(rect.w) / background_image_.width(),
									float(rect.h) / background_image_.height());
		menu_ref.video().draw_texture(background_image_, rect.x, rect.y);
	}
#else
	if(type == SELECTED_ROW && has_background_ && !load_failed_) {
		if(bg_cache_.width != rect.w || bg_cache_.height != rect.h)
		{
			//draw scaled background image
			//scale image each time (to prevent loss of quality)
			bg_cache_.surf = scale_surface(img_map_["background"], rect.w, rect.h);
			bg_cache_.width = rect.w;
			bg_cache_.height = rect.h;
		}
		SDL_Rect clip = rect;
		menu_ref.video().blit_surface(rect.x,rect.y,bg_cache_.surf,NULL,&clip);
	}
#endif
	else {
		style::draw_row_bg(menu_ref, row_index, rect, type);
	}
}

void menu::imgsel_style::draw_row(menu& menu_ref, const size_t row_index, const SDL_Rect& rect, ROW_TYPE type)
{
	if(!load_failed_) {
		//draw item inside
		style::draw_row(menu_ref, row_index, rect, type);

#ifdef SDL_GPU
		if(type == SELECTED_ROW) {
			// draw border
			sdl::timage image;
			SDL_Rect area;
			area.x = rect.x;
			area.y = rect.y;

			GPU_SetClip(get_render_target(), rect.x, rect.y, rect.w, rect.h);

			image = img_map_["border-top"];
			area.x = rect.x;
			area.y = rect.y;
			do {
				menu_ref.video().draw_texture(image, area.x, area.y);
				area.x += image.width();
			} while( area.x < rect.x + rect.w );

			image = img_map_["border-left"];
			area.x = rect.x;
			area.y = rect.y;
			do {
				menu_ref.video().draw_texture(image, area.x, area.y);
				area.y += image.height();
			} while( area.y < rect.y + rect.h );

			image = img_map_["border-right"];
			area.x = rect.x + rect.w - thickness_;
			area.y = rect.y;
			do {
				menu_ref.video().draw_texture(image, area.x, area.y);
				area.y += image.height();
			} while( area.y < rect.y + rect.h );

			image = img_map_["border-bottom"];
			area.x = rect.x;
			area.y = rect.y + rect.h - thickness_;
			do {
				menu_ref.video().draw_texture(image, area.x, area.y);
				area.x += image.width();
			} while( area.x < rect.x + rect.w );

			image = img_map_["border-topleft"];
			area.x = rect.x;
			area.y = rect.y;
			menu_ref.video().draw_texture(image, area.x, area.y);

			image = img_map_["border-topright"];
			area.x = rect.x + rect.w - image.width();
			area.y = rect.y;
			menu_ref.video().draw_texture(image, area.x, area.y);

			image = img_map_["border-botleft"];
			area.x = rect.x;
			area.y = rect.y + rect.h - image.height();
			menu_ref.video().draw_texture(image, area.x, area.y);

			image = img_map_["border-botright"];
			area.x = rect.x + rect.w - image.width();
			area.y = rect.y + rect.h - image.height();
			menu_ref.video().draw_texture(image, area.x, area.y);

			GPU_UnsetClip(get_render_target());
		}
	}
#else
		if(type == SELECTED_ROW) {
			// draw border
			surface image;
			SDL_Rect area;
			SDL_Rect clip = rect;
			area.x = rect.x;
			area.y = rect.y;

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

			image = img_map_["border-right"];
			area.x = rect.x + rect.w - thickness_;
			area.y = rect.y;
			do {
				menu_ref.video().blit_surface(area.x,area.y,image,NULL,&clip);
				area.y += image->h;
			} while( area.y < rect.y + rect.h );

			image = img_map_["border-bottom"];
			area.x = rect.x;
			area.y = rect.y + rect.h - thickness_;
			do {
				menu_ref.video().blit_surface(area.x,area.y,image,NULL,&clip);
				area.x += image->w;
			} while( area.x < rect.x + rect.w );

			image = img_map_["border-topleft"];
			area.x = rect.x;
			area.y = rect.y;
			menu_ref.video().blit_surface(area.x,area.y,image);

			image = img_map_["border-topright"];
			area.x = rect.x + rect.w - image->w;
			area.y = rect.y;
			menu_ref.video().blit_surface(area.x,area.y,image);

			image = img_map_["border-botleft"];
			area.x = rect.x;
			area.y = rect.y + rect.h - image->h;
			menu_ref.video().blit_surface(area.x,area.y,image);

			image = img_map_["border-botright"];
			area.x = rect.x + rect.w - image->w;
			area.y = rect.y + rect.h - image->h;
			menu_ref.video().blit_surface(area.x,area.y,image);
		}
	}
#endif
		else {
		//default drawing
		style::draw_row(menu_ref, row_index, rect, type);
	}
}

SDL_Rect menu::imgsel_style::item_size(const std::string& item) const
{
	SDL_Rect bounds = style::item_size(item);

	bounds.w += 2 * thickness_;
	bounds.h += 2 * thickness_;

	return bounds;
}


} //namesapce gui
