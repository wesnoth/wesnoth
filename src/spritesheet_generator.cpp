/*
   Copyright (C) 2018 by Charles Dang <exodia339@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "spritesheet_generator.hpp"

#include "filesystem.hpp"
#include "image.hpp"
#include "sdl/point.hpp"
#include "sdl/rect.hpp"
#include "sdl/surface.hpp"
#include "sdl/texture.hpp"
#include "sdl/utils.hpp"

#include <algorithm>
#include <iostream>
#include <map>

namespace image
{
namespace
{
struct sheet_elment_data
{
	/**
	 * The spritesheet.
	 *
	 * Do note The texture class is really a "texture reference" class; it only
	 * owns a shared pointer to the actual texture in memory. Therefor, having
	 * this object does not mean we're keeping multiple copies of each sheet.
	 */
	texture sheet;

	/** The coordinates of this image on the spritesheet. */
	SDL_Rect rect;
};

/** Helper sorting struct for build_sheet_from_images to use. */
struct surf_area_sort
{
	bool operator()(const surface& lhs, const surface& rhs) const
	{
		return lhs->w * lhs->h < rhs->w * rhs->h;
	}
};

/** Map of path to rect. */
std::map<std::string, sheet_elment_data> path_sheet_mapping;

void build_sheet_from_images(const std::vector<std::string>& file_paths)
{
	// Surface -> [path, sheet element data] map, sorted by surface area (largest last).
	// TODO: simplify
	std::multimap<surface, std::pair<std::string, sheet_elment_data>, surf_area_sort> surf_path_map;

	// Load all the images.
	for(const auto& f : file_paths) {
		surface temp = image::get_image(f);

		if(!temp.null()) {
			std::pair<std::string, sheet_elment_data> another_temp(f, {});
			surf_path_map.emplace(std::move(temp), another_temp);
		}
	}

	if(surf_path_map.empty()) {
		return;
	}

	const unsigned total_area = std::accumulate(surf_path_map.begin(), surf_path_map.end(), 0,
		[](const int val, const auto& s) { return val + (s.first->w * s.first->h); });

	const unsigned side_length = static_cast<unsigned>(std::sqrt(total_area) * 1.3);

	std::vector<sheet_elment_data> packaged_data;
	packaged_data.reserve(surf_path_map.size());

	unsigned current_row_max_height = 0;
	unsigned total_height = 0;

	point origin(0, 0);

	//
	// Calculate the destination rects for the images.
	// This uses the Shelf Next Fit algorithm as described here: http://clb.demon.fi/files/RectangleBinPack.pdf
	// Our method forgoes the orientation consideration and works top-down instead of bottom-up, however.
	//
	for(auto& iter : surf_path_map) {
		SDL_Rect r = get_non_transparent_portion(iter.first);

		current_row_max_height = std::max<unsigned>(current_row_max_height, r.h);

		// If we can't fit this element without getting cut off, move to the next line.
		if(static_cast<unsigned>(origin.x + r.w) > side_length) {
			// Reset the origin.
			origin.x = 0;
			origin.y += current_row_max_height;

			// Save this row's max height.
			total_height += current_row_max_height;
			current_row_max_height = 0;
		}

		r.x = origin.x;
		r.y = origin.y;

		// Save this element's rect.
		iter.second.second.rect = r;

		// Shift the rect origin for the next element.
		origin.x += r.w;
	}

	// If we never reached max width during rect placement, total_height will be empty.
	// In that case, fall back to the row's max height.
	const unsigned res_w = side_length;
	const unsigned res_h = total_height > 0 ? std::min<unsigned>(side_length, total_height) : current_row_max_height;

	// Check that we won't exceed max texture size and that neither dimension is 0. TODO: handle?
	assert(res_w > 0 && res_w <= 8192 && res_h > 0 && res_h <= 8192);

	// Assemble everything
	surface res = create_neutral_surface(res_w, res_h);
	assert(!res.null() && "Spritesheet surface is null!");

	for(auto& iter : surf_path_map) {
		const surface& s = iter.first;
		sheet_elment_data& data = iter.second.second;

		SDL_Rect src_rect = get_non_transparent_portion(s);
		sdl_blit(s, &src_rect, res, &data.rect);
	}

#ifdef DEBUG_SPRITESHEET_OUTPUT
	static unsigned test_i = 0;
	image::save_image(res, "spritesheets/sheet_test_" + std::to_string(test_i++) + ".png");
#endif

	// Convert the sheet to a texture.
	texture sheet_tex(res);

	// Add path mappings.
	for(auto& iter : surf_path_map) {
		// Copy a texture reference;
		sheet_elment_data& data = iter.second.second;
		data.sheet = sheet_tex;

#ifdef HAVE_CXX17
		path_sheet_mapping.insert(std::exchange(iter->second, {}));
#else
		path_sheet_mapping.emplace(iter.second.first, data);
#endif
	}
}

void build_spritesheet_from_impl(const std::string& dir, const std::string& subdir)
{
	const std::string checked_dir = dir + subdir;

	if(!filesystem::is_directory(checked_dir)) {
		return;
	}

	std::vector<std::string> files_found;
	std::vector<std::string> dirs_found;

	filesystem::get_files_in_dir(checked_dir, &files_found, &dirs_found,
		filesystem::FILE_NAME_ONLY,
		filesystem::NO_FILTER,
		filesystem::DONT_REORDER
	);

	for(std::string& file : files_found) {
		file = subdir + file;
	}

	if(!files_found.empty()) {
		build_sheet_from_images(files_found);
	}

	for(const auto& d : dirs_found) {
		build_spritesheet_from_impl(dir, subdir + d + "/");
	}
}

} // end anon namespace
void build_spritesheet_from(const std::string& subdir)
{
#ifdef DEBUG_SPRITESHEET_OUTPUT
	const std::size_t start = SDL_GetTicks();
#endif

	for(const auto& p : filesystem::get_binary_paths("images")) {
		build_spritesheet_from_impl(p, subdir);
	}

#ifdef DEBUG_SPRITESHEET_OUTPUT
	std::cerr << "Spritesheet generation of '" << subdir << "' took: " << (SDL_GetTicks() - start) << "ms\n";
#endif
}

} // namespace image
