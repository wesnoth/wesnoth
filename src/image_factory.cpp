/*
	Copyright (C) 2025 - 2025
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#include "image_factory.hpp"

#include "draw.hpp"
#include "filesystem.hpp"
#include "log.hpp"
#include "picture.hpp"
#include "sdl/rect.hpp"
#include "sdl/utils.hpp"
#include "serialization/base64.hpp"
#include "utils/optional_fwd.hpp"

#include <SDL2/SDL_image.h>

#include <boost/filesystem/path.hpp>
namespace fs = boost::filesystem;

static lg::log_domain log_image("image");
#define ERR_IMG LOG_STREAM(err, log_image)

namespace image
{
namespace
{
auto get_binary_path(const fs::path& path)
{
	return filesystem::get_binary_file_location("images", path.string());
}

utils::optional<std::string> try_webp_fallback(const fs::path& filename)
{
	// We don't want missing webp files to get checked twice.
	auto ext = filename.extension();
	if(!(ext == ".png" || ext == ".jpg")) {
		return utils::nullopt;
	}

	// TODO: consider if a more general fallback mechanism would be useful
	return get_binary_path(fs::path{ filename }.replace_extension("webp"));
}

/**
 * Attempts to resolve the full binary path to the specified image. If the given
 * path points to a nonexistant .png or .jpg file, a .webp file at the same path
 * will also be checked. Most images were converted from PNG to WEBP format, but
 * the old filenames may still be present in save files, etc.
 */
utils::optional<std::string> resolve_path(const fs::path& filename)
{
	auto path = get_binary_path(filename);
	return path ? path : try_webp_fallback(filename);
}

/**
 * Returns a pair of optional paths to an image's localized base and overlay,
 * respectively, should either exist.
 */
auto resolve_localization(const std::string& base_path)
{
	return std::pair{
		filesystem::get_localized_path(base_path),
		filesystem::get_localized_path(base_path, "--overlay")
	};
}

/** Load overlay image and compose it with the original surface. */
void compose_localized_overlay(surface& base, const std::string& overlay_path)
{
	auto overlay = surface::from_disk(overlay_path);
	rect dest{0, 0, overlay->w, overlay->h};
	sdl_blit(overlay, nullptr, base, &dest);
}

/** Load overlay image and compose it with the original texture. */
void compose_localized_overlay(texture& base, const std::string& overlay_path)
{
	auto overlay = texture::from_disk(overlay_path);
	const auto target = draw::set_render_target(base);
	draw::blit(overlay, {0, 0, overlay.w(), overlay.h()});
}

} // anon namespace

#ifdef __cpp_concepts
template<Drawable T>
#else
template<typename T>
#endif
T factory<T>::load(const image::locator& loc)
{
	if(loc.is_data_uri()) {
		return from_data_uri(loc);
	} else {
		return from_disk(loc);
	}
}

#ifdef __cpp_concepts
template<Drawable T>
#else
template<typename T>
#endif
T factory<T>::from_disk(const image::locator& loc)
{
	const auto path = resolve_path(loc.get_filename());
	if(!path) {
		ERR_IMG << "Could not resolve path for image '" << loc.get_filename() << "'";
		return {};
	}

	const auto [i18n_base, i18n_overlay] = resolve_localization(*path);

	// If there's a localized variant (notable case - game logo), load that instead.
	auto res = T::from_disk(i18n_base.value_or(*path));
	if(!res) {
		ERR_IMG << "Could not load image '" << loc.get_filename() << "'";
		return {};
	}

	if(i18n_overlay) {
		// TODO: overlays aren't subject to the webp fallback check :thinking:
		compose_localized_overlay(res, *i18n_overlay);
	}

	return res;
}

#ifdef __cpp_concepts
template<Drawable T>
#else
template<typename T>
#endif
T factory<T>::from_data_uri(const image::locator& loc)
{
	parsed_data_URI parsed{loc.get_filename()};

	if(!parsed.good) {
		std::string_view fn = loc.get_filename();
		std::string_view stripped = fn.substr(0, fn.find(","));
		ERR_IMG << "Invalid data URI: " << stripped;
	} else if(parsed.mime.substr(0, 5) != "image") {
		ERR_IMG << "Data URI not of image MIME type: " << parsed.mime;
	} else {
		const std::vector<uint8_t> image_data = base64::decode(parsed.data);
		filesystem::rwops_ptr rwops{SDL_RWFromConstMem(image_data.data(), image_data.size())};

		if(image_data.empty()) {
			ERR_IMG << "Invalid encoding in data URI";
		} else if(parsed.mime == "image/png") {
			return T{IMG_LoadPNG_RW(rwops.release())};
		} else if(parsed.mime == "image/jpeg") {
			return T{IMG_LoadJPG_RW(rwops.release())};
		} else if(parsed.mime == "image/webp") {
			return T{IMG_LoadWEBP_RW(rwops.release())};
		} else {
			ERR_IMG << "Invalid image MIME type: " << parsed.mime;
		}
	}

	return {};
}

// ================================================
// Explicit instantiations. We only need these two.
// ================================================

template class factory<texture>;
template class factory<surface>;

} // namespace image
