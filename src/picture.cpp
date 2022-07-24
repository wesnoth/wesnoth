/*
	Copyright (C) 2003 - 2022
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
 * Routines for images: load, scale, re-color, etc.
 */

#include "picture.hpp"

#include "config.hpp"
#include "display.hpp"
#include "filesystem.hpp"
#include "game_config.hpp"
#include "image_modifications.hpp"
#include "log.hpp"
#include "preferences/general.hpp"
#include "serialization/base64.hpp"
#include "serialization/string_utils.hpp"
#include "sdl/rect.hpp"
#include "sdl/texture.hpp"

#include <SDL2/SDL_image.h>

#include <functional>

#include <boost/algorithm/string.hpp>
#include <boost/functional/hash_fwd.hpp>

#include <array>
#include <set>

static lg::log_domain log_image("image");
#define ERR_IMG LOG_STREAM(err, log_image)
#define WRN_IMG LOG_STREAM(warn, log_image)
#define LOG_IMG LOG_STREAM(info, log_image)
#define DBG_IMG LOG_STREAM(debug, log_image)

static lg::log_domain log_config("config");
#define ERR_CFG LOG_STREAM(err, log_config)

using game_config::tile_size;

template<typename T>
struct cache_item
{
	cache_item()
		: item()
		, loaded(false)
	{
	}

	cache_item(const T& item)
		: item(item)
		, loaded(true)
	{
	}

	T item;
	bool loaded;
};

namespace std
{
template<>
struct hash<image::locator::value>
{
	std::size_t operator()(const image::locator::value& val) const
	{
		std::size_t hash = std::hash<unsigned>{}(val.type_);

		if(val.type_ == image::locator::FILE || val.type_ == image::locator::SUB_FILE) {
			boost::hash_combine(hash, val.filename_);
		}

		if(val.type_ == image::locator::SUB_FILE) {
			boost::hash_combine(hash, val.loc_.x);
			boost::hash_combine(hash, val.loc_.y);
			boost::hash_combine(hash, val.center_x_);
			boost::hash_combine(hash, val.center_y_);
			boost::hash_combine(hash, val.modifications_);
		}

		return hash;
	}
};
}

namespace image
{
template<typename T>
class cache_type
{
public:
	cache_type()
		: content_()
	{
	}

	cache_item<T>& get_element(int index)
	{
		if(static_cast<unsigned>(index) >= content_.size())
			content_.resize(index + 1);
		return content_[index];
	}

	void flush()
	{
		content_.clear();
	}

private:
	std::vector<cache_item<T>> content_;
};

template<typename T>
bool locator::in_cache(cache_type<T>& cache) const
{
	return index_ < 0 ? false : cache.get_element(index_).loaded;
}

template<typename T>
const T& locator::locate_in_cache(cache_type<T>& cache) const
{
	static T dummy;
	return index_ < 0 ? dummy : cache.get_element(index_).item;
}

template<typename T>
T& locator::access_in_cache(cache_type<T>& cache) const
{
	static T dummy;
	return index_ < 0 ? dummy : cache.get_element(index_).item;
}

template<typename T>
void locator::add_to_cache(cache_type<T>& cache, const T& data) const
{
	if(index_ >= 0) {
		cache.get_element(index_) = cache_item<T>(data);
	}
}

namespace
{
image::locator::locator_finder_t locator_finder;

/** Definition of all image maps */
std::array<surface_cache, NUM_TYPES> surfaces_;

/**
 * Texture caches.
 * Note that the latter two are temporary and should be removed once we have OGL and shader support.
 */
using texture_cache_map = std::map<image::scale_quality, image::texture_cache>;

texture_cache_map textures_;
texture_cache_map textures_hexed_;
texture_cache_map texture_tod_colored_;

// cache storing if each image fit in a hex
image::bool_cache in_hex_info_;

// cache storing if this is an empty hex
image::bool_cache is_empty_hex_;

// caches storing the different lighted cases for each image
image::lit_surface_cache lit_surfaces_;
image::lit_texture_cache lit_textures_;
// caches storing each lightmap generated
image::lit_surface_variants surface_lightmaps_;
image::lit_texture_variants texture_lightmaps_;

// diagnostics for tracking skipped cache impact
std::array<bool_cache, NUM_TYPES> skipped_cache_;
int duplicate_loads_ = 0;
int total_loads_ = 0;

// const int cache_version_ = 0;

std::map<std::string, bool> image_existence_map;

// directories where we already cached file existence
std::set<std::string> precached_dirs;

int red_adjust = 0, green_adjust = 0, blue_adjust = 0;

unsigned int zoom = tile_size;

const std::string data_uri_prefix = "data:";
struct parsed_data_URI{
	explicit parsed_data_URI(std::string_view data_URI);
	std::string_view scheme;
	std::string_view mime;
	std::string_view base64;
	std::string_view data;
	bool good;
};
parsed_data_URI::parsed_data_URI(std::string_view data_URI)
{
	const std::size_t colon = data_URI.find(':');
	const std::string_view after_scheme = data_URI.substr(colon + 1);

	const std::size_t comma = after_scheme.find(',');
	const std::string_view type_info = after_scheme.substr(0, comma);

	const std::size_t semicolon = type_info.find(';');

	scheme = data_URI.substr(0, colon);
	base64 = type_info.substr(semicolon + 1);
	mime = type_info.substr(0, semicolon);
	data = after_scheme.substr(comma + 1);
	good = (scheme == "data" && base64 == "base64" && mime.length() > 0 && data.length() > 0);
}

} // end anon namespace

mini_terrain_cache_map mini_terrain_cache;
mini_terrain_cache_map mini_fogged_terrain_cache;
mini_terrain_cache_map mini_highlighted_terrain_cache;

static int last_index_ = 0;

void flush_cache()
{
	for(surface_cache& cache : surfaces_) {
		cache.flush();
	}
	lit_surfaces_.flush();
	lit_textures_.flush();
	in_hex_info_.flush();
	is_empty_hex_.flush();
	textures_.clear();
	textures_hexed_.clear();
	texture_tod_colored_.clear();
	mini_terrain_cache.clear();
	mini_fogged_terrain_cache.clear();
	mini_highlighted_terrain_cache.clear();
	image_existence_map.clear();
	precached_dirs.clear();
	/* We can't reset last_index_, since some locators are still alive
	   when using :refresh. That would cause them to point to the wrong
	   images. Not resetting the variable causes a memory leak, though. */
	// last_index_ = 0;
}

void locator::init_index()
{
	auto i = locator_finder.find(val_);

	if(i == locator_finder.end()) {
		index_ = last_index_++;
		locator_finder.emplace(val_, index_);
	} else {
		index_ = i->second;
	}
}

void locator::parse_arguments()
{
	std::string& fn = val_.filename_;
	if(fn.empty()) {
		return;
	}

	if(boost::algorithm::starts_with(fn, data_uri_prefix)) {
		parsed_data_URI parsed{fn};

		if(!parsed.good) {
			std::string_view view{ fn };
			std::string_view stripped = view.substr(0, view.find(","));
			ERR_IMG << "Invalid data URI: " << stripped;
		}

		val_.is_data_uri_ = true;
	}

	std::size_t markup_field = fn.find('~');

	if(markup_field != std::string::npos) {
		val_.type_ = SUB_FILE;
		val_.modifications_ = fn.substr(markup_field, fn.size() - markup_field);
		fn = fn.substr(0, markup_field);
	}
}

locator::locator()
	: index_(-1)
	, val_()
{
}

locator::locator(const locator& a, const std::string& mods)
	: index_(-1)
	, val_(a.val_)
{
	if(!mods.empty()) {
		val_.modifications_ += mods;
		val_.type_ = SUB_FILE;
		init_index();
	} else {
		index_ = a.index_;
	}
}

locator::locator(const char* filename)
	: index_(-1)
	, val_(filename)
{
	parse_arguments();
	init_index();
}

locator::locator(const std::string& filename)
	: index_(-1)
	, val_(filename)
{
	parse_arguments();
	init_index();
}

locator::locator(const std::string& filename, const std::string& modifications)
	: index_(-1)
	, val_(filename, modifications)
{
	init_index();
}

locator::locator(const char* filename, const char* modifications)
	: index_(-1)
	, val_(filename, modifications)
{
	init_index();
}

locator::locator(const std::string& filename,
		const map_location& loc,
		int center_x,
		int center_y,
		const std::string& modifications)
	: index_(-1)
	, val_(filename, loc, center_x, center_y, modifications)
{
	init_index();
}

locator& locator::operator=(const locator& a)
{
	index_ = a.index_;
	val_ = a.val_;

	return *this;
}

std::ostream& operator<<(std::ostream& s, const locator& l)
{
	s << l.get_filename();
	if(!l.get_modifications().empty()) {
		if(l.get_modifications()[0] != '~') {
			s << '~';
		}
		s << l.get_modifications();
	}
	return s;
}

locator::value::value()
	: type_(NONE)
	, is_data_uri_(false)
	, filename_()
	, loc_()
	, modifications_()
	, center_x_(0)
	, center_y_(0)
{
}

locator::value::value(const char* filename)
	: type_(FILE)
	, is_data_uri_(false)
	, filename_(filename)
	, loc_()
	, modifications_()
	, center_x_(0)
	, center_y_(0)
{
}

locator::value::value(const std::string& filename)
	: type_(FILE)
	, is_data_uri_(false)
	, filename_(filename)
	, loc_()
	, modifications_()
	, center_x_(0)
	, center_y_(0)
{
}

locator::value::value(const std::string& filename, const std::string& modifications)
	: type_(SUB_FILE)
	, is_data_uri_(false)
	, filename_(filename)
	, loc_()
	, modifications_(modifications)
	, center_x_(0)
	, center_y_(0)
{
}

locator::value::value(const char* filename, const char* modifications)
	: type_(FILE)
	, is_data_uri_(false)
	, filename_(filename)
	, loc_()
	, modifications_(modifications)
	, center_x_(0)
	, center_y_(0)
{
}

locator::value::value(const std::string& filename,
		const map_location& loc,
		int center_x,
		int center_y,
		const std::string& modifications)
	: type_(SUB_FILE)
	, is_data_uri_(false)
	, filename_(filename)
	, loc_(loc)
	, modifications_(modifications)
	, center_x_(center_x)
	, center_y_(center_y)
{
}

bool locator::value::operator==(const value& a) const
{
	if(a.type_ != type_) {
		return false;
	} else if(type_ == FILE) {
		return filename_ == a.filename_;
	} else if(type_ == SUB_FILE) {
		return filename_ == a.filename_ && loc_ == a.loc_ && modifications_ == a.modifications_
			   && center_x_ == a.center_x_ && center_y_ == a.center_y_;
	}

	return false;
}

bool locator::value::operator<(const value& a) const
{
	if(type_ != a.type_) {
		return type_ < a.type_;
	} else if(type_ == FILE) {
		return filename_ < a.filename_;
	} else if(type_ == SUB_FILE) {
		if(filename_ != a.filename_)
			return filename_ < a.filename_;
		if(loc_ != a.loc_)
			return loc_ < a.loc_;
		if(center_x_ != a.center_x_)
			return center_x_ < a.center_x_;
		if(center_y_ != a.center_y_)
			return center_y_ < a.center_y_;
		return (modifications_ < a.modifications_);
	}

	return false;
}

// Load overlay image and compose it with the original surface.
static void add_localized_overlay(const std::string& ovr_file, surface& orig_surf)
{
	filesystem::rwops_ptr rwops = filesystem::make_read_RWops(ovr_file);
	surface ovr_surf = IMG_Load_RW(rwops.release(), true); // SDL takes ownership of rwops
	if(!ovr_surf) {
		return;
	}

	SDL_Rect area {0, 0, ovr_surf->w, ovr_surf->h};

	sdl_blit(ovr_surf, 0, orig_surf, &area);
}

static surface load_image_file(const image::locator& loc)
{
	surface res;
	const std::string& name = loc.get_filename();

	std::string location = filesystem::get_binary_file_location("images", name);

	// Many images have been converted from PNG to WEBP format,
	// but the old filename may still be saved in savegame files etc.
	// If the file does not exist in ".png" format, also try ".webp".
	if(location.empty() && filesystem::ends_with(name, ".png")) {
		std::string webp_name = name.substr(0, name.size() - 4) + ".webp";
		location = filesystem::get_binary_file_location("images", webp_name);
		if(!location.empty()) {
			WRN_IMG << "Replaced missing '" << name << "' with found '"
			        << webp_name << "'.";
		}
	}

	{
		if(!location.empty()) {
			// Check if there is a localized image.
			const std::string loc_location = filesystem::get_localized_path(location);
			if(!loc_location.empty()) {
				location = loc_location;
			}

			filesystem::rwops_ptr rwops = filesystem::make_read_RWops(location);
			res = IMG_Load_RW(rwops.release(), true); // SDL takes ownership of rwops

			// If there was no standalone localized image, check if there is an overlay.
			if(res && loc_location.empty()) {
				const std::string ovr_location = filesystem::get_localized_path(location, "--overlay");
				if(!ovr_location.empty()) {
					add_localized_overlay(ovr_location, res);
				}
			}
		}
	}

	if(!res && !name.empty()) {
		ERR_IMG << "could not open image '" << name << "'";
		if(game_config::debug && name != game_config::images::missing)
			return get_surface(game_config::images::missing, UNSCALED);
	}

	return res;
}

static surface load_image_sub_file(const image::locator& loc)
{
	surface surf = get_surface(loc.get_filename(), UNSCALED);
	if(surf == nullptr) {
		return nullptr;
	}

	modification_queue mods = modification::decode(loc.get_modifications());

	while(!mods.empty()) {
		modification* mod = mods.top();

		try {
			surf = (*mod)(surf);
		} catch(const image::modification::imod_exception& e) {
			std::ostringstream ss;
			ss << "\n";

			for(const std::string& mod_name : utils::parenthetical_split(loc.get_modifications(), '~')) {
				ss << "\t" << mod_name << "\n";
			}

			ERR_CFG << "Failed to apply a modification to an image:\n"
					<< "Image: " << loc.get_filename() << "\n"
					<< "Modifications: " << ss.str() << "\n"
					<< "Error: " << e.message;
		}

		// NOTE: do this *after* applying the mod or you'll get crashes!
		mods.pop();
	}

	if(loc.get_loc().valid()) {
		rect srcrect(
			((tile_size * 3) / 4)                           *  loc.get_loc().x,
			  tile_size * loc.get_loc().y + (tile_size / 2) * (loc.get_loc().x % 2),
			  tile_size,
			  tile_size
		);

		if(loc.get_center_x() >= 0 && loc.get_center_y() >= 0) {
			srcrect.x += surf->w / 2 - loc.get_center_x();
			srcrect.y += surf->h / 2 - loc.get_center_y();
		}

		// cut and hex mask, but also check and cache if empty result
		surface cut(cut_surface(surf, srcrect));
		bool is_empty = false;
		surf = mask_surface(cut, get_hexmask(), &is_empty);

		// discard empty images to free memory
		if(is_empty) {
			// Safe because those images are only used by terrain rendering
			// and it filters them out.
			// A safer and more general way would be to keep only one copy of it
			surf = nullptr;
		}

		loc.add_to_cache(is_empty_hex_, is_empty);
	}

	return surf;
}

static surface load_image_data_uri(const image::locator& loc)
{
	surface surf;

	parsed_data_URI parsed{loc.get_filename()};

	if(!parsed.good) {
		std::string_view fn = loc.get_filename();
		std::string_view stripped = fn.substr(0, fn.find(","));
		ERR_IMG << "Invalid data URI: " << stripped;
	} else if(parsed.mime.substr(0, 5) != "image") {
		ERR_IMG << "Data URI not of image MIME type: " << parsed.mime;
	} else {
		const std::vector<uint8_t> image_data = base64::decode(parsed.data);
		filesystem::rwops_ptr rwops{SDL_RWFromConstMem(image_data.data(), image_data.size()), &SDL_FreeRW};

		if(image_data.empty()) {
			ERR_IMG << "Invalid encoding in data URI";
		} else if(parsed.mime == "image/png") {
			surf = IMG_LoadTyped_RW(rwops.release(), true, "PNG");
		} else if(parsed.mime == "image/jpeg") {
			surf = IMG_LoadTyped_RW(rwops.release(), true, "JPG");
		} else {
			ERR_IMG << "Invalid image MIME type: " << parsed.mime;
		}
	}

	return surf;
}

// small utility function to store an int from (-256,254) to an signed char
static signed char col_to_uchar(int i)
{
	return static_cast<signed char>(std::min<int>(127, std::max<int>(-128, i / 2)));
}

light_string get_light_string(int op, int r, int g, int b)
{
	light_string ls;
	ls.reserve(4);
	ls.push_back(op);
	ls.push_back(col_to_uchar(r));
	ls.push_back(col_to_uchar(g));
	ls.push_back(col_to_uchar(b));

	return ls;
}

static surface apply_light(surface surf, const light_string& ls)
{
	// atomic lightmap operation are handled directly (important to end recursion)
	if(ls.size() == 4) {
		// if no lightmap (first char = -1) then we need the initial value
		//(before the halving done for lightmap)
		int m = ls[0] == -1 ? 2 : 1;
		return adjust_surface_color(surf, ls[1] * m, ls[2] * m, ls[3] * m);
	}

	// check if the lightmap is already cached or need to be generated
	surface lightmap = nullptr;
	auto i = surface_lightmaps_.find(ls);
	if(i != surface_lightmaps_.end()) {
		lightmap = i->second;
	} else {
		// build all the paths for lightmap sources
		static const std::string p = "terrain/light/light";
		static const std::string lm_img[19] {
			p + ".png",
			p + "-concave-2-tr.png", p + "-concave-2-r.png", p + "-concave-2-br.png",
			p + "-concave-2-bl.png", p + "-concave-2-l.png", p + "-concave-2-tl.png",
			p + "-convex-br-bl.png", p + "-convex-bl-l.png", p + "-convex-l-tl.png",
			p + "-convex-tl-tr.png", p + "-convex-tr-r.png", p + "-convex-r-br.png",
			p + "-convex-l-bl.png",  p + "-convex-tl-l.png", p + "-convex-tr-tl.png",
			p + "-convex-r-tr.png",  p + "-convex-br-r.png", p + "-convex-bl-br.png"
		};

		// decompose into atomic lightmap operations (4 chars)
		for(std::size_t c = 0; c + 3 < ls.size(); c += 4) {
			light_string sls = ls.substr(c, 4);

			// get the corresponding image and apply the lightmap operation to it
			// This allows to also cache lightmap parts.
			// note that we avoid infinite recursion by using only atomic operation
			surface lts = image::get_lighted_image(lm_img[sls[0]], sls);

			// first image will be the base where we blit the others
			if(lightmap == nullptr) {
				// copy the cached image to avoid modifying the cache
				lightmap = lts.clone();
			} else {
				sdl_blit(lts, nullptr, lightmap, nullptr);
			}
		}

		// cache the result
		surface_lightmaps_[ls] = lightmap;
	}

	// apply the final lightmap
	return light_surface(surf, lightmap);
}

bool locator::file_exists() const
{
	return val_.is_data_uri_
		? parsed_data_URI{val_.filename_}.good
		: !filesystem::get_binary_file_location("images", val_.filename_).empty();
}

static surface load_from_disk(const locator& loc)
{
	switch(loc.get_type()) {
	case locator::FILE:
		if(loc.is_data_uri()){
			return load_image_data_uri(loc);
		} else {
			return load_image_file(loc);
		}
	case locator::SUB_FILE:
		return load_image_sub_file(loc);
	default:
		return surface(nullptr);
	}
}

manager::manager()
{
}

manager::~manager()
{
	flush_cache();
}

void set_color_adjustment(int r, int g, int b)
{
	if(r != red_adjust || g != green_adjust || b != blue_adjust) {
		red_adjust = r;
		green_adjust = g;
		blue_adjust = b;
		surfaces_[TOD_COLORED].flush();
		lit_surfaces_.flush();
		lit_textures_.flush();
		texture_tod_colored_.clear();
	}
}

void set_zoom(unsigned int amount)
{
	// This no longer has to do anything fancy.
	zoom = amount;
}

static surface get_hexed(const locator& i_locator, bool skip_cache = false)
{
	surface image(get_surface(i_locator, UNSCALED, skip_cache));
	surface mask(get_hexmask());
	// Ensure the image is the correct size by cropping and/or centering.
	// TODO: this should probably be a function of sdl/utils
	if(image && (image->w != mask->w || image->h != mask->h)) {
		DBG_IMG << "adjusting [" << image->w << ',' << image->h << ']'
			<< " image to hex mask: " << i_locator;
		// the fitted surface
		surface fit(mask->w, mask->h);
		// if the image is too large in either dimension, crop it.
		if(image->w > mask->w || image->h >= mask->h) {
			// fill the crop surface with transparency
			sdl::fill_surface_rect(fit, nullptr,
				SDL_MapRGBA(fit->format, 0, 0, 0, 0)
			);
			// crop the input image to hexmask dimensions
			int cutx = std::max(0, image->w - mask->w) / 2;
			int cuty = std::max(0, image->h - mask->h) / 2;
			int cutw = std::min(image->w, mask->w);
			int cuth = std::min(image->h, mask->h);
			image = cut_surface(image, {cutx, cuty, cutw, cuth});
			// image will now have dimensions <= mask
		}
		// center image
		int placex = (mask->w - image->w) / 2;
		int placey = (mask->h - image->h) / 2;
		rect dst = {placex, placey, image->w, image->h};
		sdl_blit(image, nullptr, fit, &dst);
		image = fit;
	}
	// hex cut tiles, also check and cache if empty result
	bool is_empty = false;
	surface res = mask_surface(image, mask, &is_empty, i_locator.get_filename());
	i_locator.add_to_cache(is_empty_hex_, is_empty);
	return res;
}

static surface get_tod_colored(const locator& i_locator, bool skip_cache = false)
{
	surface img = get_surface(i_locator, HEXED, skip_cache);
	return adjust_surface_color(img, red_adjust, green_adjust, blue_adjust);
}

/** translate type to a simpler one when possible */
static TYPE simplify_type(const image::locator& i_locator, TYPE type)
{
	if(type == TOD_COLORED) {
		if(red_adjust == 0 && green_adjust == 0 && blue_adjust == 0) {
			type = HEXED;
		}
	}

	if(type == HEXED) {
		// check if the image is already hex-cut by the location system
		if(i_locator.get_loc().valid()) {
			type = UNSCALED;
		}
	}

	return type;
}

surface get_surface(
	const image::locator& i_locator,
	TYPE type,
	bool skip_cache)
{
	surface res;

	if(i_locator.is_void()) {
		return res;
	}

	type = simplify_type(i_locator, type);

	// select associated cache
	if(type >= NUM_TYPES) {
		WRN_IMG << "get_surface called with unknown image type";
		return res;
	}
	surface_cache& imap = surfaces_[type];

	// return the image if already cached
	if (i_locator.in_cache(imap)) {
		return i_locator.locate_in_cache(imap);
	}

	DBG_IMG << "surface cache [" << type << "] miss: " << i_locator;

	// not cached, generate it
	switch(type) {
	case UNSCALED:
		// If type is unscaled, directly load the image from the disk.
		res = load_from_disk(i_locator);
		break;
	case TOD_COLORED:
		res = get_tod_colored(i_locator, skip_cache);
		break;
	case HEXED:
		res = get_hexed(i_locator, skip_cache);
		break;
	default:
		throw game::error("get_surface somehow lost image type?");
	}

	bool_cache& skip = skipped_cache_[type];
	if(i_locator.in_cache(skip) && i_locator.locate_in_cache(skip))
	{
		DBG_IMG << "duplicate load: " << i_locator
			<< " [" << type << "]"
			<< " (" << duplicate_loads_ << "/" << total_loads_ << " total)";
		++duplicate_loads_;
	}
	++total_loads_;

	if(skip_cache) {
		DBG_IMG << "surface cache [" << type << "] skip: " << i_locator;
		i_locator.add_to_cache(skip, true);
	} else {
		i_locator.add_to_cache(imap, res);
	}

	return res;
}

surface get_image(const image::locator& i_locator, TYPE type)
{
	return get_surface(i_locator, type);
}

surface get_lighted_image(const image::locator& i_locator, const light_string& ls)
{
	surface res;
	if(i_locator.is_void()) {
		return res;
	}

	// select associated cache
	lit_surface_cache* imap = &lit_surfaces_;

	// if no light variants yet, need to add an empty map
	if(!i_locator.in_cache(*imap)) {
		i_locator.add_to_cache(*imap, lit_surface_variants());
	}

	// need access to add it if not found
	{ // enclose reference pointing to data stored in a changing vector
		const lit_surface_variants& lvar = i_locator.locate_in_cache(*imap);
		auto lvi = lvar.find(ls);
		if(lvi != lvar.end()) {
			return lvi->second;
		}
	}

	DBG_IMG << "lit surface cache miss: " << i_locator;

	// not cached yet, generate it
	res = get_surface(i_locator, HEXED);
	res = apply_light(res, ls);

	// record the lighted surface in the corresponding variants cache
	i_locator.access_in_cache(*imap)[ls] = res;

	return res;
}

texture get_lighted_texture(
	const image::locator& i_locator,
	const light_string& ls)
{
	if(i_locator.is_void()) {
		return texture();
	}

	// select associated cache
	lit_texture_cache* imap = &lit_textures_;

	// if no light variants yet, need to add an empty map
	if(!i_locator.in_cache(*imap)) {
		i_locator.add_to_cache(*imap, lit_texture_variants());
	}

	// need access to add it if not found
	{ // enclose reference pointing to data stored in a changing vector
		const lit_texture_variants& lvar = i_locator.locate_in_cache(*imap);
		auto lvi = lvar.find(ls);
		if(lvi != lvar.end()) {
			return lvi->second;
		}
	}

	DBG_IMG << "lit texture cache miss: " << i_locator;

	// not cached yet, generate it
	texture tex(get_lighted_image(i_locator, ls));

	// record the lighted texture in the corresponding variants cache
	i_locator.access_in_cache(*imap)[ls] = tex;

	return tex;
}

surface get_hexmask()
{
	static const image::locator terrain_mask(game_config::images::terrain_mask);
	return get_surface(terrain_mask, UNSCALED);
}

point get_size(const locator& i_locator, bool skip_cache)
{
	const surface s(get_surface(i_locator, UNSCALED, skip_cache));
	if (s != nullptr) {
		return {s->w, s->h};
	} else {
		return {0, 0};
	}
}

bool is_in_hex(const locator& i_locator)
{
	bool result;
	{
		if(i_locator.in_cache(in_hex_info_)) {
			result = i_locator.locate_in_cache(in_hex_info_);
		} else {
			const surface image(get_surface(i_locator, UNSCALED));

			bool res = in_mask_surface(image, get_hexmask());

			i_locator.add_to_cache(in_hex_info_, res);

			// std::cout << "in_hex : " << i_locator.get_filename()
			//		<< " " << (res ? "yes" : "no") << "\n";

			result = res;
		}
	}

	return result;
}

bool is_empty_hex(const locator& i_locator)
{
	if(!i_locator.in_cache(is_empty_hex_)) {
		const surface surf = get_surface(i_locator, HEXED);
		// emptiness of terrain image is checked during hex cut
		// so, maybe in cache now, let's recheck
		if(!i_locator.in_cache(is_empty_hex_)) {
			// should never reach here
			// but do it manually if it happens
			// assert(false);
			bool is_empty = false;
			mask_surface(surf, get_hexmask(), &is_empty);
			i_locator.add_to_cache(is_empty_hex_, is_empty);
		}
	}

	return i_locator.locate_in_cache(is_empty_hex_);
}

bool exists(const image::locator& i_locator)
{
	typedef image::locator loc;
	loc::type type = i_locator.get_type();
	if(type != loc::FILE && type != loc::SUB_FILE) {
		return false;
	}

	// The insertion will fail if there is already an element in the cache
	// and this will point to the existing element.
	auto [iter, success] = image_existence_map.emplace(i_locator.get_filename(), false);

	bool& cache = iter->second;
	if(success) {
		if(i_locator.is_data_uri()) {
			cache = parsed_data_URI{i_locator.get_filename()}.good;
		} else {
			cache = !filesystem::get_binary_file_location("images", i_locator.get_filename()).empty();
		}
	}

	return cache;
}

static void precache_file_existence_internal(const std::string& dir, const std::string& subdir)
{
	const std::string checked_dir = dir + "/" + subdir;
	if(precached_dirs.find(checked_dir) != precached_dirs.end()) {
		return;
	}

	precached_dirs.insert(checked_dir);

	if(!filesystem::is_directory(checked_dir)) {
		return;
	}

	std::vector<std::string> files_found;
	std::vector<std::string> dirs_found;
	filesystem::get_files_in_dir(checked_dir, &files_found, &dirs_found, filesystem::name_mode::FILE_NAME_ONLY,
			filesystem::filter_mode::NO_FILTER, filesystem::reorder_mode::DONT_REORDER);

	for(const auto& f : files_found) {
		image_existence_map[subdir + f] = true;
	}

	for(const auto& d : dirs_found) {
		precache_file_existence_internal(dir, subdir + d + "/");
	}
}

void precache_file_existence(const std::string& subdir)
{
	const std::vector<std::string>& paths = filesystem::get_binary_paths("images");

	for(const auto& p : paths) {
		precache_file_existence_internal(p, subdir);
	}
}

bool precached_file_exists(const std::string& file)
{
	const auto b = image_existence_map.find(file);
	if(b != image_existence_map.end()) {
		return b->second;
	}

	return false;
}

save_result save_image(const locator& i_locator, const std::string& filename)
{
	return save_image(get_surface(i_locator), filename);
}

save_result save_image(const surface& surf, const std::string& filename)
{
	if(!surf) {
		return save_result::no_image;
	}

	if(filesystem::ends_with(filename, ".jpeg") || filesystem::ends_with(filename, ".jpg") || filesystem::ends_with(filename, ".jpe")) {
		LOG_IMG << "Writing a JPG image to " << filename;

		const int err = IMG_SaveJPG_RW(surf, filesystem::make_write_RWops(filename).release(), true, 75); // SDL takes ownership of the RWops
		return err == 0 ? save_result::success : save_result::save_failed;
	}

	if(filesystem::ends_with(filename, ".png")) {
		LOG_IMG << "Writing a PNG image to " << filename;

		const int err = IMG_SavePNG_RW(surf, filesystem::make_write_RWops(filename).release(), true); // SDL takes ownership of the RWops
		return err == 0 ? save_result::success : save_result::save_failed;
	}

	return save_result::unsupported_format;
}

/*
 * TEXTURE INTERFACE ======================================================================
 *
 * The only important difference here is that textures must have their
 * scale quality set before creation. All other handling is done by
 * get_surface.
 */

texture get_texture(const image::locator& i_locator, TYPE type, bool skip_cache)
{
	return get_texture(i_locator, scale_quality::nearest, type, skip_cache);
}

/** Returns a texture for the corresponding image. */
texture get_texture(const image::locator& i_locator, scale_quality quality, TYPE type, bool skip_cache)
{
	texture res;

	if(i_locator.is_void()) {
		return res;
	}

	type = simplify_type(i_locator, type);

	//
	// Select the appropriate cache. We don't need caches for every single image types,
	// since some types can be handled by render-time operations.
	//
	texture_cache* cache = nullptr;

	switch(type) {
	case HEXED:
		cache = &textures_hexed_[quality];
		break;
	case TOD_COLORED:
		cache = &texture_tod_colored_[quality];
		break;
	default:
		cache = &textures_[quality];
	}

	//
	// Now attempt to find a cached texture. If found, return it.
	//
	bool in_cache = i_locator.in_cache(*cache);

	if(in_cache) {
		res = i_locator.locate_in_cache(*cache);
		return res;
	}

	DBG_IMG << "texture cache [" << type << "] miss: " << i_locator;

	//
	// No texture was cached. In that case, create a new one. The explicit cases require special
	// handling with surfaces in order to generate the desired effect. This shouldn't be the case
	// once we get OGL and shader support.
	//

	// Get it from the surface cache, also setting the desired scale quality.
	const bool linear_scaling = quality == scale_quality::linear ? true : false;
	if(i_locator.get_modifications().empty()) {
		// skip cache if we're loading plain files with no modifications
		res = texture(get_surface(i_locator, type, true), linear_scaling);
	} else {
		res = texture(get_surface(i_locator, type, skip_cache), linear_scaling);
	}

	// Cache the texture.
	if(skip_cache) {
		DBG_IMG << "texture cache [" << type << "] skip: " << i_locator;
	} else {
		i_locator.add_to_cache(*cache, res);
	}

	return res;
}

} // end namespace image
