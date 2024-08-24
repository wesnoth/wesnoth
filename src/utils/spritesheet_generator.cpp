/*
	Copyright (C) 2018 - 2022
	by Charles Dang <exodia339@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "utils/spritesheet_generator.hpp"

#include "config.hpp"
#include "filesystem.hpp"
#include "log.hpp"
#include "picture.hpp"
#include "sdl/point.hpp"
#include "sdl/rect.hpp"
#include "sdl/surface.hpp"
#include "sdl/utils.hpp"
#include "serialization/binary_or_text.hpp"

#include <SDL2/SDL_image.h>

#include <algorithm>
#include <future>
#include <iostream>
#include <numeric>

#ifdef __APPLE__
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

namespace image
{
namespace
{
/** Intermediate helper struct to manage surfaces while the sheet is being assembled. */
struct sheet_element
{
	explicit sheet_element(const fs::path& p)
		: surf(IMG_Load_RW(filesystem::make_read_RWops(p.string()).release(), true))
		, filename(p.filename().string())
		, src(get_non_transparent_portion(surf))
		, dst()
	{
	}

	/** Image. */
	surface surf;

	/** Filename. */
	std::string filename;

	/** Non-transparent portion of the surface to compose. */
	rect src;

	/** Location on the final composed sheet. */
	rect dst;

	config to_config() const
	{
		return config{
			"filename", filename,

			/** Source rect of this image on the final sheet. */
			"sheet_rect", formatter() << dst.x << ',' << dst.y << ',' << dst.w  << ',' << dst.h,

			/** Offset at which to render this image, equal to the non-transparent offset from origin (0,0 top left). */
			"draw_offset", formatter() << src.x << ',' << src.y,

			/** Original image size in case we need it. */
			"original_size", formatter() << surf->w << ',' << surf->h,
		};
	}
};

/** Tweak as needed. *Must* be floating-point in order to allow rounding. */
constexpr double max_items_per_loader = 8.0;

void build_sheet_from_images(const std::vector<fs::path>& file_paths)
{
	const unsigned num_loaders = std::ceil(file_paths.size() / max_items_per_loader);
	const unsigned num_to_load = std::ceil(file_paths.size() / double(num_loaders));

	std::vector<std::future<std::vector<sheet_element>>> loaders{};
	loaders.reserve(num_loaders);

	for(unsigned i = 0; i < num_loaders; ++i) {
		loaders.push_back(std::async(std::launch::async, [&file_paths, &num_to_load, i]() {
			std::vector<sheet_element> res;

			for(unsigned k = num_to_load * i; k < std::min<unsigned>(num_to_load * (i + 1u), file_paths.size()); ++k) {
				res.emplace_back(file_paths[k]);
			}

			return res;
		}));
	}

	std::vector<sheet_element> elements;
	elements.reserve(file_paths.size());

	// Wait for results, then combine them with the master list.
	for(auto& loader : loaders) {
		auto res = loader.get();
		std::move(res.begin(), res.end(), std::back_inserter(elements));
	}

	// Sort the surfaces by area, largest last.
	// TODO: should we use plain sort? Output sheet seems ever so slightly smaller when sort is not stable.
	std::stable_sort(elements.begin(), elements.end(),
		[](const auto& lhs, const auto& rhs) { return lhs.surf->w * lhs.surf->h < rhs.surf->w * rhs.surf->h; });

	const unsigned total_area = std::accumulate(elements.begin(), elements.end(), 0,
		[](const int val, const auto& s) { return val + (s.surf->w * s.surf->h); });

	const unsigned side_length = static_cast<unsigned>(std::sqrt(total_area) * 1.3);

	unsigned current_row_max_height = 0;
	unsigned total_height = 0;

	point origin{0, 0};

	//
	// Calculate the destination rects for the images. This uses the Shelf Next Fit algorithm.
	// Our method forgoes the orientation consideration and works top-down instead of bottom-up.
	//
	for(auto& s : elements) {
		current_row_max_height = std::max<unsigned>(current_row_max_height, s.src.h);

		// If we can't fit this element without getting cut off, move to the next line.
		if(static_cast<unsigned>(origin.x + s.src.w) > side_length) {
			// Reset the origin.
			origin.x = 0;
			origin.y += current_row_max_height;

			// Save this row's max height.
			total_height += current_row_max_height;
			current_row_max_height = 0;
		}

		// Save this element's rect.
		s.dst = { origin.x, origin.y, s.src.w, s.src.h };

		// Shift the rect origin for the next element.
		origin.x += s.src.w;
	}

	// If we never reached max width during rect placement, total_height will be empty.
	// In that case, fall back to the row's max height.
	const unsigned res_w = side_length;
	const unsigned res_h = total_height > 0 ? std::min<unsigned>(side_length, total_height) : current_row_max_height;

	// Check that we won't exceed max texture size and that neither dimension is 0. TODO: handle?
	assert(res_w > 0 && res_w <= 8192 && res_h > 0 && res_h <= 8192);

	surface res(res_w, res_h);
	assert(res && "Spritesheet surface is null!");

	// Final record of each image's location on the composed sheet.
	auto out = filesystem::ostream_file("./_sheet.cfg");
	config_writer mapping_data{*out, compression::format::gzip};

	// Assemble everything
	for(auto& s : elements) {
		sdl_blit(s.surf, &s.src, res, &s.dst);
		mapping_data.write_child("image", s.to_config());
	}

	image::save_image(res, "./_sheet.png");
}

void handle_dir_contents(const fs::path& path)
{
	std::vector<fs::path> files_found;
	for(const auto& entry : fs::directory_iterator{path}) {
		if(entry.is_directory()) {
			handle_dir_contents(entry);
		} else if(entry.is_regular_file()) {
			// TODO: should we have a better is-image check, and should we include jpgs?
			// Right now all our sprites are pngs.
			if(auto path = entry.path(); path.extension() == ".png" && path.stem() != "_sheet") {
				files_found.push_back(std::move(path));
			}
		}
	}

	if(!files_found.empty()) {
		try {
			// Allows relative paths to resolve correctly. This needs to be set *after* recursive
			// directory handling or else the path will be wrong when returning to the parent.
			fs::current_path(path);
		} catch(const fs::filesystem_error&) {
			return;
		}

		build_sheet_from_images(files_found);
	}
}

} // end anon namespace

void build_spritesheet_from(const std::string& entry_point)
{
#ifdef DEBUG_SPRITESHEET_OUTPUT
	const std::size_t start = SDL_GetTicks();
#endif

	if(auto path = filesystem::get_binary_file_location("images", entry_point)) {
		try {
			handle_dir_contents(*path);
		} catch(const fs::filesystem_error& e) {
			PLAIN_LOG << "Filesystem Error generating spritesheet: " << e.what();
		}
	} else {
		PLAIN_LOG << "Cannot find entry point to build spritesheet: " << entry_point;
	}

#ifdef DEBUG_SPRITESHEET_OUTPUT
	PLAIN_LOG << "Spritesheet generation of '" << entry_point << "' took: " << (SDL_GetTicks() - start) << "ms\n";
#endif
}

} // namespace image
