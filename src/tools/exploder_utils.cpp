/*
   Copyright (C) 2004 - 2016 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "exploder_utils.hpp"
#include "game_config.hpp"
#include "serialization/string_utils.hpp"
#include <png.h>
#include <zlib.h>

exploder_point::exploder_point(const std::string &s)
	: x(0)
	, y(0)
{
	std::vector<std::string> items = utils::split(s);
	if(items.size() == 2) {
		x = atoi(items[0].c_str());
		y = atoi(items[1].c_str());
	}
}

exploder_rect::exploder_rect(const std::string &s)
	: x(0)
	, y(0)
	, w(0)
	, h(0)
{
	std::vector<std::string> items = utils::split(s);
	if(items.size() == 4) {
		x = atoi(items[0].c_str());
		y = atoi(items[1].c_str());
		w = atoi(items[2].c_str());
		h = atoi(items[3].c_str());
	}
}

std::string get_mask_dir()
{
	// return ".";
	return game_config::path + "/images/tools/exploder";
}

std::string get_exploder_dir()
{
	return game_config::path + "/data/tools/exploder";
}

//on pixels where mask is not white, overwrite dest with src. Mask and dest are
//translated to the position (x,y) on dest.
//All surfaces are supposed to be neutral surfaces, mask and src are supposed
//to be of identical size.
void masked_overwrite_surface(surface dest, surface src, surface mask, int x, int y)
{
	surface_lock dest_lock(dest);
	surface_lock src_lock(src);
	surface_lock mask_lock(mask);

	Uint32* dest_beg = dest_lock.pixels();
	Uint32* src_beg = src_lock.pixels();
	Uint32* mask_beg = mask_lock.pixels();

	size_t small_shift_before;
	size_t small_shift_after;
	size_t dest_shift;
	size_t src_width = src->w;
	size_t src_height = src->h;

	if(x < 0) {
		small_shift_before = -x;
		if (src_width < small_shift_before)
			return;
		src_width -= small_shift_before;
		x = 0;
	} else {
		small_shift_before = 0;
	}

	if(x + src_width <= unsigned(dest->w)) {
		small_shift_after = 0;
	} else {
		small_shift_after = src_width - (dest->w - x);
		src_width = dest->w - x;
	}

	if(y >= 0) {
		dest_beg += dest->w * y + x;
	} else {
		src_beg += (-y) * src->w;
		mask_beg += (-y) * mask->w;
		dest_beg += x;
		if (src_height < static_cast<size_t>(-y))
			return;
		src_height += y;
		y = 0;
	}

	if(y + src_height > unsigned(dest->h)) {
		src_height = dest->h - y;
	}

	dest_shift = dest->w - src_width;

	for(size_t j = 0; j < src_height; ++j) {
		src_beg += small_shift_before;
		mask_beg += small_shift_before;

		for(size_t i = 0; i < src_width; ++i) {
			//Assuming here alpha mask is 0xff000000
			if((*mask_beg & 0x00ffffff) != 0x00ffffff) {
				*dest_beg = *src_beg;
			}
			++dest_beg;
			++src_beg;
			++mask_beg;
		}

		src_beg += small_shift_after;
		mask_beg += small_shift_after;
		dest_beg += dest_shift;
	}
}


//returns true if the image is empty. the surface surf is considered to be a
//neutral surface.
bool image_empty(surface surf)
{
	//an image is considered empty if
	// * all its pixels have 0 alpha, OR
	// * all of its pixels have the same color (and the same alpha)

	surface_lock lock(surf);

	Uint32* beg = lock.pixels();
	Uint32* end = beg + surf->w*surf->h;

	Uint32 color = *beg;

	while(beg != end) {
		if((*beg & 0xff000000) != 0 && (*beg != color))
			return false;

		++beg;
	}
	return true;
}


namespace {
	struct rgba {
		Uint8 r;
		Uint8 g;
		Uint8 b;
		Uint8 a;
	};
}

//saves the given SDL structure into a given filename.
void save_image(surface surf, const std::string &filename)
{
	//opens the actual file
	const util::scoped_FILE file(fopen(filename.c_str(),"wb"));

	//initializes PNG write structures
	//TODO: review whether providing NULL error handlers is something
	//sensible
	png_struct* png_ptr = png_create_write_struct
		(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr)
		throw exploder_failure("Unable to initialize the png write structure");

	png_info* info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr) {
		png_destroy_write_struct(&png_ptr, NULL);
		throw exploder_failure("Unable to initialize the png info structure");
	}

	//instructs the PNG library to use the open file
	png_init_io(png_ptr, file);

	//sets compression level to the maximum
	png_set_compression_level(png_ptr,
			Z_BEST_COMPRESSION);

	//configures the header
	png_set_IHDR(png_ptr, info_ptr, surf->w, surf->h,
			8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	//puts the actual image data in the row_pointers array
	png_byte **row_pointers = new png_byte *[surf->h];
	surface_lock lock(surf);

	//converts the data to the RGBA format. We cannot pass SDL data
	//directly to the png lib, even if we know its pixel format, because of
	//endianness problems.
	util::scoped_array<rgba> rgba_data(new rgba[surf->w * surf->h]);

	Uint32 *surf_data = lock.pixels();
	int pos = 0;
	for(int y = 0; y < surf->h; ++y) {
		row_pointers[y] = reinterpret_cast<png_byte*>(rgba_data + pos);
		for(int x = 0; x < surf->w; ++x) {
			Uint8 red, green, blue, alpha;
			SDL_GetRGBA(*surf_data, surf->format, &red, &green, &blue, &alpha);
			rgba_data[pos].r = red;
			rgba_data[pos].g = green;
			rgba_data[pos].b = blue;
			rgba_data[pos].a = alpha;
			pos++;
			surf_data++;
		}
	}
	png_set_rows(png_ptr, info_ptr, row_pointers);

	//writes the actual image data
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	//cleans everything
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	delete [] row_pointers;
}

