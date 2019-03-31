/*
   Copyright (C) 2018 by Jyrki Vesterinen <sandgtx@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "ogl/texture_atlas.hpp"

#include "filesystem.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "image_modifications.hpp"
#include "log.hpp"
#include "sdl/utils.hpp"
#include "serialization/string_utils.hpp"
#include "utils/math.hpp"

#include <boost/algorithm/string/trim.hpp>
#include <SDL_image.h>

#include <algorithm>
#include <chrono>
#include <future>
#include <iomanip>
#include <numeric>
#include <set>

static lg::log_domain log_opengl("opengl");
#define DBG_GL LOG_STREAM(debug, log_opengl)
#define LOG_GL LOG_STREAM(info, log_opengl)
#define ERR_GL LOG_STREAM(err, log_opengl)

namespace
{

void standardize_surface_format(surface& surf)
{
	if(!surf.null() && !is_neutral(surf)) {
		surf = make_neutral_surface(surf);
		assert(is_neutral(surf));
	}
}

std::string get_base_image_name(const std::string& image_path)
{
	return utils::split(image_path, '~')[0];
}

std::string get_ipf_string(const std::string& image_path)
{
	std::size_t index = image_path.find('~');
	if(index == std::string::npos) {
		return "";
	} else {
		return image_path.substr(index);
	}
}

// Load overlay image and compose it with the original surface.
void add_localized_overlay(const std::string& ovr_file, surface& orig_surf)
{
	filesystem::rwops_ptr rwops = filesystem::make_read_RWops(ovr_file);
	surface ovr_surf = IMG_Load_RW(rwops.release(), true); // SDL takes ownership of rwops
	if(ovr_surf.null()) {
		return;
	}

	standardize_surface_format(ovr_surf);

	SDL_Rect area{0, 0, ovr_surf->w, ovr_surf->h};

	sdl_blit(ovr_surf, 0, orig_surf, &area);
}

// Check if localized file is up-to-date according to l10n track index.
// Make sure only that the image is not explicitly recorded as fuzzy,
// in order to be able to use non-tracked images (e.g. from UMC).
static std::set<std::string> fuzzy_localized_files;
bool localized_file_uptodate(const std::string& loc_file)
{
	if(fuzzy_localized_files.empty()) {
		// First call, parse track index to collect fuzzy files by path.
		std::string fsep = "\xC2\xA6"; // UTF-8 for "broken bar"
		std::string trackpath = filesystem::get_binary_file_location("", "l10n-track");

		// l10n-track file not present. Assume image is up-to-date.
		if(trackpath.empty()) {
			return true;
		}

		std::string contents = filesystem::read_file(trackpath);

		for(const std::string& line : utils::split(contents, '\n')) {
			std::size_t p1 = line.find(fsep);
			if(p1 == std::string::npos) {
				continue;
			}

			std::string state = line.substr(0, p1);
			boost::trim(state);
			if(state == "fuzzy") {
				std::size_t p2 = line.find(fsep, p1 + fsep.length());
				if(p2 == std::string::npos) {
					continue;
				}

				std::string relpath = line.substr(p1 + fsep.length(), p2 - p1 - fsep.length());
				fuzzy_localized_files.insert(game_config::path + '/' + relpath);
			}
		}

		fuzzy_localized_files.insert(""); // make sure not empty any more
	}

	return fuzzy_localized_files.count(loc_file) == 0;
}

// Return path to localized counterpart of the given file, if any, or empty string.
// Localized counterpart may also be requested to have a suffix to base name.
std::string get_localized_path(const std::string& file, const std::string& suff = "")
{
	std::string dir = filesystem::directory_name(file);
	std::string base = filesystem::base_name(file);

	const std::size_t pos_ext = base.rfind(".");

	std::string loc_base;
	if(pos_ext != std::string::npos) {
		loc_base = base.substr(0, pos_ext) + suff + base.substr(pos_ext);
	}
	else {
		loc_base = base + suff;
	}

	// TRANSLATORS: This is the language code which will be used
	// to store and fetch localized non-textual resources, such as images,
	// when they exist. Normally it is just the code of the PO file itself,
	// e.g. "de" of de.po for German. But it can also be a comma-separated
	// list of language codes by priority, when the localized resource
	// found for first of those languages will be used. This is useful when
	// two languages share sufficient commonality, that they can use each
	// other's resources rather than duplicating them. For example,
	// Swedish (sv) and Danish (da) are such, so Swedish translator could
	// translate this message as "sv,da", while Danish as "da,sv".
	std::vector<std::string> langs = utils::split(_("language code for localized resources^en_US"));

	// In case even the original image is split into base and overlay,
	// add en_US with lowest priority, since the message above will
	// not have it when translated.
	langs.push_back("en_US");
	for(const std::string& lang : langs) {
		std::string loc_file = dir + "/" + "l10n" + "/" + lang + "/" + loc_base;
		if(filesystem::file_exists(loc_file) && localized_file_uptodate(loc_file)) {
			return loc_file;
		}
	}

	return "";
}

// Merges adjacent rectangles together when possible.
void rectangle_merge(std::vector<SDL_Rect>& rectangles)
{
	for(std::size_t i = 0; i < rectangles.size(); ++i) {
		for(std::size_t j = 0; j < rectangles.size(); ++j) {
			if(i == j) {
				continue;
			}

			if(rectangles[i].x == rectangles[j].x && rectangles[i].w == rectangles[j].w &&
				rectangles[i].y == rectangles[j].y + rectangles[j].h) {
				rectangles[j].h += rectangles[i].h;
				rectangles.erase(rectangles.begin() + i);
				i = 0;
				j = 0;
			} else if(rectangles[i].y == rectangles[j].y && rectangles[i].h == rectangles[j].h &&
				rectangles[i].x == rectangles[j].x + rectangles[j].w) {
				rectangles[j].w += rectangles[i].w;
				rectangles.erase(rectangles.begin() + i);
				i = 0;
				j = 0;
			}
		}
	}
}

}

namespace gl
{

void texture_atlas::init(const std::vector<std::string>& images, thread_pool& thread_pool)
{
	// Determine unique base images.
	std::unordered_map<std::string, surface> base_images;
	for(const std::string& i : images) {
		base_images.emplace(get_base_image_name(i), surface());
	}

	std::vector<sprite_data> base_image_data;
	base_image_data.reserve(base_images.size());
	for(const auto& s : base_images) {
		sprite_data data;
		data.name = s.first;
		base_image_data.push_back(data);
	}

	// Load base images from disk.
	thread_pool.run(base_image_data, &load_image).wait();

	for(const sprite_data& s : base_image_data) {
		base_images[s.name] = s.surf;
	}

	std::vector<sprite_data> sprites;
	sprites.reserve(images.size());
	for(const std::string& i : images) {
		sprite_data data;
		data.name = i;
		data.surf = base_images[get_base_image_name(i)];
		sprites.push_back(data);
	}

	// Apply IPFs.
	thread_pool.run(sprites, &apply_IPFs).wait();

	// Pack sprites.
	pack_sprites_wrapper(sprites);

	// Create sprite objects.
	std::for_each(sprites.begin(), sprites.end(),
		std::bind(&texture_atlas::create_sprite,
			this, texture_.get_size(), std::placeholders::_1));
}

bool texture_atlas::sprite_data::operator<(const sprite_data& other) const
{
	std::pair<int, int> my_dims = std::minmax(surf->w, surf->h);
	std::pair<int, int> other_dims = std::minmax(other.surf->w, other.surf->h);
	return my_dims > other_dims;
}

void texture_atlas::pack_sprites_wrapper(std::vector<sprite_data>& sprites)
{
	using std::chrono::duration_cast;
	using std::chrono::milliseconds;

	auto start_time = std::chrono::high_resolution_clock::now();

	sprites_.clear();
	sprites_by_name_.clear();

	// Sort the sprites to make them pack better.
	std::stable_sort(sprites.begin(), sprites.end());

	unsigned int total_size = std::accumulate(sprites.begin(), sprites.end(), 0u,
		[](const unsigned int& size, const sprite_data& sprite)
	{
		return size + sprite.surf->w * sprite.surf->h;
	});

	std::pair<int, int> texture_size = calculate_initial_texture_size(total_size);
	if(texture_size.first > texture::MAX_DIMENSION ||
		texture_size.second > texture::MAX_DIMENSION) {
		// No way the sprites would fit.
		throw packing_error();
	}

	texture_.set_size(texture_size);

	while(true) {
		try {
			pack_sprites(sprites);
			break;
		} catch(packing_error&) {
			// Double the shorter dimension (width in case of tie).
			if(texture_size.first <= texture_size.second) {
				texture_size.first *= 2;
			} else {
				texture_size.second *= 2;
			}
			if(texture_size.first > texture::MAX_DIMENSION ||
				texture_size.second > texture::MAX_DIMENSION) {
				// Ran out of space.
				throw;
			}
			texture_.set_size(texture_size);
		}
	}

	auto end_time = std::chrono::high_resolution_clock::now();
	auto time = end_time - start_time;

	const double BYTES_PER_PIXEL = 4.0;
	double efficiency = static_cast<double>(total_size) /
		(texture_size.first * texture_size.second);

	DBG_GL << std::setprecision(3u) << "Texture atlas packed: " << sprites.size() <<
		" sprites (" << (BYTES_PER_PIXEL * total_size / (1 << 20)) << " MB)" <<
		" packed to a " << texture_size.first << "x" << texture_size.second <<
		" texture, efficiency << " << 100.0 * efficiency << " %, packing took " <<
		duration_cast<milliseconds>(time).count() << " ms";

	build(sprites);
}

void texture_atlas::pack_sprites(std::vector<sprite_data>& sprites)
{
	free_rectangles_.clear();

	std::pair<int, int> texture_size = texture_.get_size();
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = texture_size.first;
	rect.h = texture_size.second;
	free_rectangles_.push_back(rect);

	std::for_each(sprites.begin(), sprites.end(),
		std::bind(&texture_atlas::place_sprite, this, std::placeholders::_1));
}

void texture_atlas::place_sprite(sprite_data& sprite)
{
	// Find the target rectangle.
	auto rect = std::min_element(free_rectangles_.begin(), free_rectangles_.end(),
		std::bind(&better_fit, sprite, std::placeholders::_1, std::placeholders::_2));
	if(rect->w < sprite.surf->w || rect->h < sprite.surf->h) {
		// The sprite doesn't fit anywhere. Packing has failed.
		throw packing_error();
	}

	// Place the sprite into the rectangle.
	sprite.x_min = rect->x;
	sprite.x_max = rect->x + sprite.surf->w;
	sprite.y_max = rect->y + rect->h;
	sprite.y_min = sprite.y_max - sprite.surf->h;

	// Split the remaining parts of the rectangle.
	auto new_rectangles = split_rectangle(*rect, sprite);
	free_rectangles_.erase(rect);
	if(new_rectangles.first.w != 0 && new_rectangles.first.h != 0) {
		free_rectangles_.push_back(new_rectangles.first);
	}
	if(new_rectangles.second.w != 0 && new_rectangles.second.h != 0) {
		free_rectangles_.push_back(new_rectangles.second);
	}

	// Merge adjacent rectangles.
	rectangle_merge(free_rectangles_);
}

void texture_atlas::build(const std::vector<sprite_data>& sprites)
{
	std::pair<int, int> texture_size = texture_.get_size();

	surface pixels(SDL_CreateRGBSurfaceWithFormat(
		0u, texture_size.first, texture_size.second, 32, SDL_PIXELFORMAT_RGBA32));
	SDL_FillRect(pixels, nullptr, 0u);

	for(const sprite_data& s : sprites) {
		SDL_Rect rect;
		rect.x = s.x_min;
		rect.y = texture_size.second - s.y_max;
		rect.w = s.x_max - s.x_min;
		rect.h = s.y_max - s.y_min;

		sdl_blit(s.surf, nullptr, pixels, &rect);
	}

	texture_.set_pixels(pixels);
}

sprite& texture_atlas::create_sprite(const std::pair<int, int>& texture_size, const sprite_data& data)
{
	sprite spriteObj(texture_);
	spriteObj.x_min_px = data.x_min;
	spriteObj.x_max_px = data.x_max;
	spriteObj.y_min_px = data.y_min;
	spriteObj.y_max_px = data.y_max;
	spriteObj.normalize_coordinates(texture_size);
	sprites_.push_back(spriteObj);
	sprites_by_name_.emplace(data.name, &sprites_.back());
	return sprites_.back();
}

void texture_atlas::load_image(sprite_data& sprite)
{
	std::string location = filesystem::get_binary_file_location("images", sprite.name);

	{
		if(!location.empty()) {
			// Check if there is a localized image.
			const std::string loc_location = get_localized_path(location);
			if(!loc_location.empty()) {
				location = loc_location;
			}

			filesystem::rwops_ptr rwops = filesystem::make_read_RWops(location);
			sprite.surf = IMG_Load_RW(rwops.release(), true); // SDL takes ownership of rwops

			standardize_surface_format(sprite.surf);

			// If there was no standalone localized image, check if there is an overlay.
			if(!sprite.surf.null() && loc_location.empty()) {
				const std::string ovr_location = get_localized_path(location, "--overlay");
				if(!ovr_location.empty()) {
					add_localized_overlay(ovr_location, sprite.surf);
				}
			}
		}
	}

	if(sprite.surf.null()) {
		ERR_GL << "could not open image '" << sprite.name << "'" << std::endl;
	}
}

void texture_atlas::apply_IPFs(sprite_data& sprite)
{
	surface surf = sprite.surf;
	image::modification_queue mods = image::modification::decode(get_ipf_string(sprite.name));

	while(!mods.empty()) {
		image::modification* mod = mods.top();

		try {
			surf = (*mod)(surf);
		} catch(const image::modification::imod_exception& e) {
			std::ostringstream ss;
			ss << "\n";

			for(const std::string& mod2 : utils::parenthetical_split(get_ipf_string(sprite.name), '~')) {
				ss << "\t" << mod2 << "\n";
			}

			ERR_GL << "Failed to apply a modification to an image:\n"
				<< "Image: " << get_base_image_name(sprite.name) << "\n"
				<< "Modifications: " << ss.str() << "\n"
				<< "Error: " << e.message << "\n";
		}

		// NOTE: do this *after* applying the mod or you'll get crashes!
		mods.pop();
	}

	sprite.surf = surf;
}

bool texture_atlas::better_fit(const sprite_data& sprite, const SDL_Rect& rect_a, const SDL_Rect& rect_b)
{
	// Best Short Side Fit: use the rectangle with the lowest amount of leftover area
	// in the smaller dimension.
	int leftover_a = std::min(rect_a.w - sprite.surf->w, rect_a.h - sprite.surf->h);
	int leftover_b = std::min(rect_b.w - sprite.surf->w, rect_b.h - sprite.surf->h);
	// If either doesn't have enough space to fit the sprite...
	if(std::min(leftover_a, leftover_b) < 0) {
		// ...choose the other rectangle, it might fit.
		return leftover_b < leftover_a;
	} else {
	 // Otherwise choose the rectangle with less leftover space.
		return leftover_a < leftover_b;
	}
}

std::pair<SDL_Rect, SDL_Rect> texture_atlas::split_rectangle(const SDL_Rect& rectangle, const sprite_data& sprite)
{
	// Shorter Axis Split: split the rectangle in the shorter direction
	SDL_Rect rect_a;
	SDL_Rect rect_b;
	if(rectangle.w < rectangle.h) {
		rect_a.x = sprite.x_max;
		rect_a.y = rectangle.y;
		rect_a.w = rectangle.w - sprite.surf->w;
		rect_a.h = rectangle.h;

		rect_b.x = rectangle.x;
		rect_b.y = rectangle.y;
		rect_b.w = sprite.surf->w;
		rect_b.h = rectangle.h - sprite.surf->h;
	} else {
		rect_a.x = rectangle.x;
		rect_a.y = rectangle.y;
		rect_a.w = rectangle.w;
		rect_a.h = rectangle.h - sprite.surf->h;

		rect_b.x = sprite.x_max;
		rect_b.y = sprite.y_min;
		rect_b.w = rectangle.w - sprite.surf->w;
		rect_b.h = sprite.surf->h;
	}

	return {rect_a, rect_b};
}

std::pair<int, int> texture_atlas::calculate_initial_texture_size(unsigned int combined_sprite_size)
{
	double num_bits = bit_width<unsigned int>() - count_leading_zeros(combined_sprite_size) + 1;
	int width = 1 << static_cast<unsigned int>(std::ceil(num_bits / 2.0));
	int height = 1 << static_cast<unsigned int>(std::floor(num_bits / 2.0));
	return {width, height};
}

}
