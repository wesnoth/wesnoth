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

#include <boost/algorithm/string/trim.hpp>
#include <SDL_image.h>

#include <future>
#include <set>

static lg::log_domain log_opengl("opengl");
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

}

namespace gl
{

void texture_atlas::init(const std::vector<std::string>& images, thread_pool& thread_pool)
{
	sprites_.clear();
	sprites_by_name_.clear();

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

	// TODO: pack sprites into the texture atlas
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
		}
		catch(const image::modification::imod_exception& e) {
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

}