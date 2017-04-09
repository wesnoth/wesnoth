/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "config.hpp"
#include "filesystem.hpp"
#include "game_config.hpp"
#include "image.hpp"
#include "image_modifications.hpp"
#include "log.hpp"
#include "gettext.hpp"
#include "preferences.hpp"
#include "sdl/rect.hpp"
#include "utils/general.hpp"

#ifdef HAVE_LIBPNG
#include "SDL_SavePNG/savepng.h"
#endif

#include "serialization/string_utils.hpp"

#include <SDL_image.h>

#include "utils/functional.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/functional/hash.hpp>

#include <list>
#include <set>

static lg::log_domain log_display("display");
#define ERR_DP LOG_STREAM(err, log_display)
#define LOG_DP LOG_STREAM(info, log_display)

static lg::log_domain log_config("config");
#define ERR_CFG LOG_STREAM(err , log_config)

using game_config::tile_size;

template<typename T>
struct cache_item
{
	cache_item(): item(), loaded(false)
	{}

	cache_item(const T &item): item(item), loaded(true)
	{}

	T item;
	bool loaded;
};


namespace std {

template<>
struct hash<image::locator::value> {
	size_t operator()(const image::locator::value& val) const {
		using boost::hash_value;
		using boost::hash_combine;

		/*
		 * Boost 1.51.0 seems not longer accept an enumerate value in its hash
		 * function so cast it to a type it does like.
		 */
		size_t hash = hash_value(static_cast<unsigned>(val.type_));
		if (val.type_ == image::locator::FILE || val.type_ == image::locator::SUB_FILE) {
			hash_combine(hash, val.filename_);
		}
		if (val.type_ == image::locator::SUB_FILE) {
			hash_combine(hash, val.loc_.x);
			hash_combine(hash, val.loc_.y);
			hash_combine(hash, val.center_x_);
			hash_combine(hash, val.center_y_);
			hash_combine(hash, val.modifications_);
		}

		return hash;
	}
};

}


namespace image {

template<typename T>
class cache_type
{
public:
	cache_type(): content_()
	{}

	cache_item<T> &get_element(int index) {
		if (static_cast<unsigned>(index) >= content_.size())
			content_.resize(index + 1);
		return content_[index];
	}

	void flush() { content_.clear(); }

private:
	std::vector<cache_item<T> > content_;
};

template <typename T>
bool locator::in_cache(cache_type<T> &cache) const
{
	return index_ < 0 ? false : cache.get_element(index_).loaded;
}

template <typename T>
const T &locator::locate_in_cache(cache_type<T> &cache) const
{
	static T dummy;
	return index_ < 0 ? dummy : cache.get_element(index_).item;
}

template <typename T>
T &locator::access_in_cache(cache_type<T> &cache) const
{
	static T dummy;
	return index_ < 0 ? dummy : cache.get_element(index_).item;
}

template <typename T>
void locator::add_to_cache(cache_type<T> &cache, const T &data) const
{
	if (index_ >= 0)
		cache.get_element(index_) = cache_item<T>(data);
}

}

namespace {

image::locator::locator_finder_t locator_finder;

/** Definition of all image maps */
image::image_cache images_,
		scaled_to_zoom_,
		hexed_images_,
		scaled_to_hex_images_,
		tod_colored_images_,
		brightened_images_;


// cache storing if each image fit in a hex
image::bool_cache in_hex_info_;

// cache storing if this is an empty hex
image::bool_cache is_empty_hex_;

// caches storing the different lighted cases for each image
image::lit_cache lit_images_,
		lit_scaled_images_;
// caches storing each lightmap generated
image::lit_variants lightmaps_;

// const int cache_version_ = 0;

std::map<std::string,bool> image_existence_map;

// directories where we already cached file existence
std::set<std::string> precached_dirs;

std::map<surface, surface> reversed_images_;

int red_adjust = 0, green_adjust = 0, blue_adjust = 0;

/** List of colors used by the TC image modification */
std::vector<std::string> team_colors;

int zoom = tile_size;
int cached_zoom = 0;

/** Algorithm choices */
//typedef std::function<surface(const surface &, int, int)> scaling_function;
typedef surface(*scaling_function)(const surface &, int, int);
scaling_function scale_to_zoom_func;
scaling_function scale_to_hex_func;

} // end anon namespace

namespace image {

mini_terrain_cache_map mini_terrain_cache;
mini_terrain_cache_map mini_fogged_terrain_cache;
mini_terrain_cache_map mini_highlighted_terrain_cache;

static int last_index_ = 0;

void flush_cache()
{
#ifdef _OPENMP
#pragma omp critical(image_cache)
#endif //_OPENMP
	{
		images_.flush();
		hexed_images_.flush();
		tod_colored_images_.flush();
		scaled_to_zoom_.flush();
		scaled_to_hex_images_.flush();
		brightened_images_.flush();
		lit_images_.flush();
		lit_scaled_images_.flush();
		in_hex_info_.flush();
		is_empty_hex_.flush();
		mini_terrain_cache.clear();
		mini_fogged_terrain_cache.clear();
		mini_highlighted_terrain_cache.clear();
		reversed_images_.clear();
		image_existence_map.clear();
		precached_dirs.clear();
	}
	/* We can't reset last_index_, since some locators are still alive
	   when using :refresh. That would cause them to point to the wrong
	   images. Not resetting the variable causes a memory leak, though. */
	// last_index_ = 0;
}

void locator::init_index()
{
	locator_finder_t::iterator i = locator_finder.find(val_);

	if ( i == locator_finder.end() ) {
		index_ = last_index_++;
		locator_finder.insert(std::make_pair(val_, index_));
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
	size_t markup_field = fn.find('~');

	if(markup_field != std::string::npos) {
		val_.type_ = SUB_FILE;
		val_.modifications_ = fn.substr(markup_field, fn.size() - markup_field);
		fn = fn.substr(0,markup_field);
	}
}

locator::locator() :
	index_(-1),
	val_()
{
}

locator::locator(const locator &a, const std::string& mods):
	index_(-1),
	val_(a.val_)
{
	if(!mods.empty()){
			val_.modifications_ += mods;
			val_.type_=SUB_FILE;
			init_index();
	}
	else index_=a.index_;
}

locator::locator(const char *filename) :
	index_(-1),
	val_(filename)
{
	parse_arguments();
	init_index();
}

locator::locator(const std::string &filename) :
	index_(-1),
	val_(filename)
{
	parse_arguments();
	init_index();
}

locator::locator(const std::string &filename, const std::string& modifications) :
	index_(-1),
	val_(filename, modifications)
{
	init_index();
}

locator::locator(const std::string &filename, const map_location &loc,
		int center_x, int center_y, const std::string& modifications) :
	index_(-1),
	val_(filename, loc, center_x, center_y, modifications)
{
	init_index();
}

locator& locator::operator=(const locator &a)
{
	index_ = a.index_;
	val_ = a.val_;

	return *this;
}

locator::value::value(const locator::value& a) :
  type_(a.type_), filename_(a.filename_), loc_(a.loc_),
  modifications_(a.modifications_),
  center_x_(a.center_x_), center_y_(a.center_y_)
{}

locator::value::value() :
	type_(NONE), filename_(), loc_(),
	modifications_(),
  center_x_(0), center_y_(0)
{}

locator::value::value(const char *filename) :
  type_(FILE), filename_(filename), loc_(),
  modifications_(),
  center_x_(0), center_y_(0)
{}

locator::value::value(const std::string& filename) :
  type_(FILE), filename_(filename),  loc_(),
  modifications_(),
  center_x_(0), center_y_(0)
{}

locator::value::value(const std::string& filename, const std::string& modifications) :
  type_(SUB_FILE), filename_(filename), loc_(), modifications_(modifications),
  center_x_(0), center_y_(0)
{}

locator::value::value(const std::string& filename, const map_location& loc, int center_x, int center_y, const std::string& modifications) :
  type_(SUB_FILE), filename_(filename), loc_(loc), modifications_(modifications),
  center_x_(center_x), center_y_(center_y)
{}

bool locator::value::operator==(const value& a) const
{
	if(a.type_ != type_) {
		return false;
	} else if(type_ == FILE) {
		return filename_ == a.filename_;
	} else if(type_ == SUB_FILE) {
	  return filename_ == a.filename_ && loc_ == a.loc_
			&& modifications_ == a.modifications_
			&& center_x_ == a.center_x_ && center_y_ == a.center_y_;
	} else {
		return false;
	}
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
	} else {
		return false;
	}
}

// Check if localized file is up-to-date according to l10n track index.
// Make sure only that the image is not explicitly recorded as fuzzy,
// in order to be able to use non-tracked images (e.g. from UMC).
static std::set<std::string> fuzzy_localized_files;
static bool localized_file_uptodate (const std::string& loc_file)
{
	if (fuzzy_localized_files.empty()) {
		// First call, parse track index to collect fuzzy files by path.
		std::string fsep = "\xC2\xA6"; // UTF-8 for "broken bar"
		std::string trackpath = filesystem::get_binary_file_location("", "l10n-track");
		std::string contents = filesystem::read_file(trackpath);
		std::vector<std::string> lines = utils::split(contents, '\n');
		for (const std::string &line : lines) {
			size_t p1 = line.find(fsep);
			if (p1 == std::string::npos)
				continue;
			std::string state = line.substr(0, p1);
			boost::trim(state);
			if (state == "fuzzy") {
				size_t p2 = line.find(fsep, p1 + fsep.length());
				if (p2 == std::string::npos)
					continue;
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
static std::string get_localized_path (const std::string& file, const std::string& suff = "")
{
	std::string dir = filesystem::directory_name(file);
	std::string base = filesystem::base_name(file);
	const size_t pos_ext = base.rfind(".");
	std::string loc_base;
	if (pos_ext != std::string::npos) {
		loc_base = base.substr(0, pos_ext) + suff + base.substr(pos_ext);
	} else {
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
	for (const std::string &lang : langs) {
		std::string loc_file = dir + "/" + "l10n" + "/" + lang + "/" + loc_base;
		if (filesystem::file_exists(loc_file) && localized_file_uptodate(loc_file)) {
			return loc_file;
		}
	}
	return "";
}

// Load overlay image and compose it with the original surface.
static void add_localized_overlay (const std::string& ovr_file, surface &orig_surf)
{
	SDL_RWops *rwops = filesystem::load_RWops(ovr_file);
	surface ovr_surf = IMG_Load_RW(rwops, true); // SDL takes ownership of rwops
	if (ovr_surf.null()) {
		return;
	}
	SDL_Rect area;
	area.x = 0;
	area.y = 0;
	area.w = ovr_surf->w;
	area.h = ovr_surf->h;
	sdl_blit(ovr_surf, 0, orig_surf, &area);
}

static surface load_image_file(const image::locator &loc)
{
	surface res;

	std::string location = filesystem::get_binary_file_location("images", loc.get_filename());


	{
		if (!location.empty()) {
			// Check if there is a localized image.
			const std::string loc_location = get_localized_path(location);
			if (!loc_location.empty()) {
				location = loc_location;
			}
			SDL_RWops *rwops = filesystem::load_RWops(location);
			res = IMG_Load_RW(rwops, true); // SDL takes ownership of rwops
			// If there was no standalone localized image, check if there is an overlay.
			if (!res.null() && loc_location.empty()) {
				const std::string ovr_location = get_localized_path(location, "--overlay");
				if (!ovr_location.empty()) {
					add_localized_overlay(ovr_location, res);
				}
			}
		}
	}

	if (res.null() && !loc.get_filename().empty()) {
		ERR_DP << "could not open image '" << loc.get_filename() << "'" << std::endl;
		if (game_config::debug && loc.get_filename() != game_config::images::missing)
			return get_image(game_config::images::missing, UNSCALED);
	}

	return res;
}

static surface load_image_sub_file(const image::locator &loc)
{
	surface surf = get_image(loc.get_filename(), UNSCALED);
	if(surf == nullptr)
		return nullptr;

	modification_queue mods = modification::decode(loc.get_modifications());

	while(!mods.empty()) {
		modification* mod = mods.top();

		try {
			surf = (*mod)(surf);
		} catch(const image::modification::imod_exception& e) {
			ERR_CFG << "Failed to apply a modification to an image:\n"
				<< "Image: " << loc.get_filename() << ".\n"
				<< "Modifications: " << loc.get_modifications() << ".\n"
				<< "Error: " << e.message;
		}

		// NOTE: do this *after* applying the mod or you'll get crashes!
		mods.pop();
	}

	if(loc.get_loc().valid()) {
		SDL_Rect srcrect = sdl::create_rect(
									   ((tile_size*3) / 4) * loc.get_loc().x
									   , tile_size * loc.get_loc().y + (tile_size / 2) * (loc.get_loc().x % 2)
									   , tile_size
									   , tile_size);

		if(loc.get_center_x() >= 0 && loc.get_center_y() >= 0){
			srcrect.x += surf->w/2 - loc.get_center_x();
			srcrect.y += surf->h/2 - loc.get_center_y();
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

//small utility function to store an int from (-256,254) to an signed char
static signed char col_to_uchar(int i) {
	return static_cast<signed char>(std::min<int>(127, std::max<int>(-128, i/2)));
}

light_string get_light_string(int op, int r, int g, int b){
	light_string ls;
	ls.reserve(4);
	ls.push_back(op);
	ls.push_back(col_to_uchar(r));
	ls.push_back(col_to_uchar(g));
	ls.push_back(col_to_uchar(b));
	return ls;
}

static surface apply_light(surface surf, const light_string& ls){
	// atomic lightmap operation are handled directly (important to end recursion)
	if(ls.size() == 4){
		//if no lightmap (first char = -1) then we need the initial value
		//(before the halving done for lightmap)
		int m = ls[0] == -1 ? 2 : 1;
		return adjust_surface_color(surf, ls[1]*m, ls[2]*m, ls[3]*m);
	}

	// check if the lightmap is already cached or need to be generated
	surface lightmap = nullptr;
	lit_variants::iterator i = lightmaps_.find(ls);
	if(i != lightmaps_.end()) {
		lightmap = i->second;
	} else {
		//build all the paths for lightmap sources
		static const std::string p = "terrain/light/light";
		static const std::string lm_img[19] = {
			p+".png",
			p+"-concave-2-tr.png", p+"-concave-2-r.png", p+"-concave-2-br.png",
			p+"-concave-2-bl.png", p+"-concave-2-l.png", p+"-concave-2-tl.png",
			p+"-convex-br-bl.png", p+"-convex-bl-l.png", p+"-convex-l-tl.png",
			p+"-convex-tl-tr.png", p+"-convex-tr-r.png", p+"-convex-r-br.png",
			p+"-convex-l-bl.png", p+"-convex-tl-l.png", p+"-convex-tr-tl.png",
			p+"-convex-r-tr.png", p+"-convex-br-r.png", p+"-convex-bl-br.png"
		};

		//decompose into atomic lightmap operations (4 chars)
		for(size_t c = 0; c+3 < ls.size(); c+=4){
			light_string sls = ls.substr(c,4);
			//get the corresponding image and apply the lightmap operation to it
			//This allows to also cache lightmap parts.
			//note that we avoid infinite recursion by using only atomic operation
			surface lts = image::get_lighted_image(lm_img[sls[0]], sls, HEXED);
			//first image will be the base where we blit the others
			if(lightmap == nullptr) {
				//copy the cached image to avoid modifying the cache
				lightmap = make_neutral_surface(lts);
			} else{
				sdl_blit(lts, nullptr, lightmap, nullptr);
			}
		}
		//cache the result
		lightmaps_[ls] = lightmap;
	}
	// apply the final lightmap
	return light_surface(surf, lightmap);
}

bool locator::file_exists() const
{
	return !filesystem::get_binary_file_location("images", val_.filename_).empty();
}

surface load_from_disk(const locator &loc)
{
	switch(loc.get_type()) {
		case locator::FILE:
			return load_image_file(loc);
		case locator::SUB_FILE:
			return load_image_sub_file(loc);
		default:
			return surface(nullptr);
	}
}


manager::manager() {}

manager::~manager()
{
	flush_cache();
}

static SDL_PixelFormat last_pixel_format;

void set_pixel_format(SDL_PixelFormat* format)
{
	assert(format != nullptr);

	SDL_PixelFormat &f = *format;
	SDL_PixelFormat &l = last_pixel_format;
	// if the pixel format change, we clear the cache,
	// because some images are now optimized for the wrong display format
	// FIXME: 8 bpp use palette, need to compare them. For now assume a change
	if (format->BitsPerPixel == 8
			|| f.BitsPerPixel != l.BitsPerPixel
			|| f.BytesPerPixel != l.BytesPerPixel
			|| f.Rmask != l.Rmask
			|| f.Gmask != l.Gmask
			|| f.Bmask != l.Bmask
//			|| f.Amask != l.Amask This field in not checked, not sure why.
			|| f.Rloss != l.Rloss
			|| f.Gloss != l.Gloss
			|| f.Bloss != l.Bloss
//			|| f.Aloss != l.Aloss This field in not checked, not sure why.
			|| f.Rshift != l.Rshift
			|| f.Gshift != l.Gshift
			|| f.Bshift != l.Bshift
//			|| f.Ashift != l.Ashift This field in not checked, not sure why.
			)
	{
		LOG_DP << "detected a new display format\n";
		flush_cache();
	}
	last_pixel_format = *format;
}

void set_color_adjustment(int r, int g, int b)
{
	if(r != red_adjust || g != green_adjust || b != blue_adjust) {
		red_adjust = r;
		green_adjust = g;
		blue_adjust = b;
		tod_colored_images_.flush();
		brightened_images_.flush();
		lit_images_.flush();
		lit_scaled_images_.flush();
		reversed_images_.clear();
	}
}

color_adjustment_resetter::color_adjustment_resetter()
: r_(red_adjust), g_(green_adjust), b_(blue_adjust)
{
}

void color_adjustment_resetter::reset()
{
	set_color_adjustment(r_, g_, b_);
}

void set_team_colors(const std::vector<std::string>* colors)
{
	if (colors == nullptr)
		team_colors.clear();
	else {
		team_colors = *colors;
	}
}

const std::vector<std::string>& get_team_colors()
{
	return team_colors;
}

void set_zoom(int amount)
{
	if(amount != zoom) {
		zoom = amount;
		tod_colored_images_.flush();
		brightened_images_.flush();
		reversed_images_.clear();

		// We keep these caches if:
		// we use default zoom (it doesn't need those)
		// or if they are already at the wanted zoom.
		if (zoom != tile_size && zoom != cached_zoom) {
			scaled_to_zoom_.flush();
			scaled_to_hex_images_.flush();
			lit_scaled_images_.flush();
			cached_zoom = zoom;
		}
	}
}

// F should be a scaling algorithm without "integral" zoom limitations
template <scaling_function F>
static surface scale_xbrz_helper(const surface & res, int w, int h)
{
	int best_integer_zoom = std::min(w / res.get()->w, h / res.get()->h);
	int legal_zoom = util::clamp(best_integer_zoom, 1, 5);
	return F(scale_surface_xbrz(res, legal_zoom), w, h);
}

using SCALING_ALGORITHM = preferences::SCALING_ALGORITHM;

static scaling_function select_algorithm(SCALING_ALGORITHM algo)
{
	switch (algo.v)
	{
		case SCALING_ALGORITHM::LINEAR:
		{
			scaling_function result = &scale_surface;
			return result;
		}
		case SCALING_ALGORITHM::NEAREST_NEIGHBOR:
		{
			scaling_function result = &scale_surface_nn;
			return result;
		}
		case SCALING_ALGORITHM::XBRZ_LIN:
		{
			scaling_function result = &scale_xbrz_helper<scale_surface>;
			return result;
		}
		case SCALING_ALGORITHM::XBRZ_NN:
		{
			scaling_function result = &scale_xbrz_helper<scale_surface_nn>;
			return result;
		}
		default:
			assert(false && "I don't know how to implement this scaling algorithm");
			throw 42;
	}
}

static surface get_hexed(const locator& i_locator)
{
	surface image(get_image(i_locator, UNSCALED));
	// hex cut tiles, also check and cache if empty result
	bool is_empty = false;
	surface res = mask_surface(image, get_hexmask(), &is_empty, i_locator.get_filename());
	i_locator.add_to_cache(is_empty_hex_, is_empty);
	return res;
}

static surface get_scaled_to_hex(const locator& i_locator)
{
	surface img = get_image(i_locator, HEXED);
	//return scale_surface(img, zoom, zoom);

	if (!img.null()) {
		return scale_to_hex_func(img, zoom, zoom);
	} else {
		return surface(nullptr);
	}
}

static surface get_tod_colored(const locator& i_locator)
{
	surface img = get_image(i_locator, SCALED_TO_HEX);
	return adjust_surface_color(img, red_adjust, green_adjust, blue_adjust);
}

static surface get_scaled_to_zoom(const locator& i_locator)
{
	assert(zoom != tile_size);
	assert(tile_size != 0);

	surface res(get_image(i_locator, UNSCALED));
	// For some reason haloes seems to have invalid images, protect against crashing
	if(!res.null()) {
		return scale_to_zoom_func(res, ((res.get()->w * zoom) / tile_size), ((res.get()->h * zoom) / tile_size));
	} else {
		return surface(nullptr);
	}
}

static surface get_brightened(const locator& i_locator)
{
	surface image(get_image(i_locator, TOD_COLORED));
	return brighten_image(image, ftofxp(game_config::hex_brightening));
}

///translate type to a simpler one when possible
static TYPE simplify_type(const image::locator& i_locator, TYPE type){
	switch(type) {
	case SCALED_TO_ZOOM:
		if(zoom == tile_size)
			type = UNSCALED;
		break;
	case BRIGHTENED:
		if(ftofxp(game_config::hex_brightening) == ftofxp(1.0))
			type = TOD_COLORED;
		break;
	default:
		break;
	}

	if(type == TOD_COLORED) {
		if (red_adjust==0 && green_adjust==0 && blue_adjust==0)
			type = SCALED_TO_HEX;
	}

	if(type == SCALED_TO_HEX) {
		if(zoom == tile_size)
			type = HEXED;
	}

	if(type == HEXED) {
		// check if the image is already hex-cut by the location system
		if(i_locator.get_loc().valid())
			type = UNSCALED;
	}

	return type;
}


surface get_image(const image::locator& i_locator, TYPE type)
{
	surface res;

	if(i_locator.is_void())
		return res;

	type = simplify_type(i_locator, type);

	image_cache *imap;
	// select associated cache
	switch(type) {
	case UNSCALED:
		imap = &images_;
		break;
	case TOD_COLORED:
		imap = &tod_colored_images_;
		break;
	case SCALED_TO_ZOOM:
		imap = &scaled_to_zoom_;
		break;
	case HEXED:
		imap = &hexed_images_;
		break;
	case SCALED_TO_HEX:
		imap = &scaled_to_hex_images_;
		break;
	case BRIGHTENED:
		imap = &brightened_images_;
		break;
	default:
		return res;
	}

	// return the image if already cached
	bool tmp;
#ifdef _OPENMP
#pragma omp critical(image_cache)
#endif //_OPENMP
	tmp=i_locator.in_cache(*imap);

	if(tmp) {
		surface result;
#ifdef _OPENMP
#pragma omp critical(image_cache)
#endif //_OPENMP
		result = i_locator.locate_in_cache(*imap);
		return result;
	}

	// not cached, generate it
	switch(type) {
	case UNSCALED:
		// If type is unscaled, directly load the image from the disk.
		res = load_from_disk(i_locator);
		break;
	case TOD_COLORED:
		res = get_tod_colored(i_locator);
		break;
	case SCALED_TO_ZOOM:
		res = get_scaled_to_zoom(i_locator);
		break;
	case HEXED:
		res = get_hexed(i_locator);
		break;
	case SCALED_TO_HEX:
		res = get_scaled_to_hex(i_locator);
		break;
	case BRIGHTENED:
		res = get_brightened(i_locator);
		break;
	default:
		return res;
	}

#ifdef _OPENMP
#pragma omp critical(image_cache)
#endif //_OPENMP
	i_locator.add_to_cache(*imap, res);

	return res;
}


surface get_lighted_image(const image::locator& i_locator, const light_string& ls, TYPE type)
{
	surface res;
	if(i_locator.is_void())
		return res;

	if(type == SCALED_TO_HEX && zoom == tile_size){
		type = HEXED;
	}

	// select associated cache
	lit_cache* imap = &lit_images_;
	if(type == SCALED_TO_HEX)
		imap = &lit_scaled_images_;

	// if no light variants yet, need to add an empty map
	if(!i_locator.in_cache(*imap)){
		i_locator.add_to_cache(*imap, lit_variants());
	}

	//need access to add it if not found
	{ // enclose reference pointing to data stored in a changing vector
		const lit_variants& lvar = i_locator.locate_in_cache(*imap);
		lit_variants::const_iterator lvi = lvar.find(ls);
		if(lvi != lvar.end()) {
			return lvi->second;
		}
	}

	// not cached yet, generate it
	switch(type) {
	case HEXED:
		res = get_image(i_locator, HEXED);
		res = apply_light(res, ls);
		break;
	case SCALED_TO_HEX:
		//we light before scaling to reuse the unscaled cache
		res = get_lighted_image(i_locator, ls, HEXED);
		res = scale_surface(res, zoom, zoom);;
		break;
	default:
		;
	}

	// record the lighted surface in the corresponding variants cache
	i_locator.access_in_cache(*imap)[ls] = res;

	return res;
}

surface get_hexmask()
{
	static const image::locator terrain_mask(game_config::images::terrain_mask);
	return get_image(terrain_mask, UNSCALED);
}

bool is_in_hex(const locator& i_locator)
{
	bool result;
#ifdef _OPENMP
#pragma omp critical(in_hex_info_)
#endif //_OPENMP
	{
	if(i_locator.in_cache(in_hex_info_)) {
		result= i_locator.locate_in_cache(in_hex_info_);
	} else {
		const surface image(get_image(i_locator, UNSCALED));

		bool res = in_mask_surface(image, get_hexmask());

		i_locator.add_to_cache(in_hex_info_, res);

		//std::cout << "in_hex : " << i_locator.get_filename()
		//		<< " " << (res ? "yes" : "no") << "\n";

		result= res;
	}
	}
	return result;
}

bool is_empty_hex(const locator& i_locator)
{
	if(!i_locator.in_cache(is_empty_hex_)) {
		const surface surf = get_image(i_locator, HEXED);
		// emptiness of terrain image is checked during hex cut
		// so, maybe in cache now, let's recheck
		if(!i_locator.in_cache(is_empty_hex_)) {
			//should never reach here
			//but do it manually if it happens
			//assert(false);
			bool is_empty = false;
			mask_surface(surf, get_hexmask(), &is_empty);
			i_locator.add_to_cache(is_empty_hex_, is_empty);
		}
	}
	return i_locator.locate_in_cache(is_empty_hex_);
}


surface reverse_image(const surface& surf)
{
	if(surf == nullptr) {
		return surface(nullptr);
	}

	const std::map<surface,surface>::iterator itor = reversed_images_.find(surf);
	if(itor != reversed_images_.end()) {
		// sdl_add_ref(itor->second);
		return itor->second;
	}

	const surface rev(flip_surface(surf));
	if(rev == nullptr) {
		return surface(nullptr);
	}

	reversed_images_.emplace(surf, rev);
	// sdl_add_ref(rev);
	return rev;
}

bool exists(const image::locator& i_locator)
{
	typedef image::locator loc;
	loc::type type = i_locator.get_type();
	if (type != loc::FILE && type != loc::SUB_FILE)
		return false;

	// The insertion will fail if there is already an element in the cache
	std::pair< std::map< std::string, bool >::iterator, bool >
		it = image_existence_map.insert(std::make_pair(i_locator.get_filename(), false));
	bool &cache = it.first->second;
	if (it.second)
		cache = !filesystem::get_binary_file_location("images", i_locator.get_filename()).empty();
	return cache;
}

static void precache_file_existence_internal(const std::string& dir, const std::string& subdir)
{
	const std::string checked_dir = dir + "/" + subdir;
	if (precached_dirs.find(checked_dir) != precached_dirs.end())
		return;
	precached_dirs.insert(checked_dir);

	if (!filesystem::is_directory(checked_dir))
		return;

	std::vector<std::string> files_found;
	std::vector<std::string> dirs_found;
	filesystem::get_files_in_dir(checked_dir, &files_found, &dirs_found,
			filesystem::FILE_NAME_ONLY, filesystem::NO_FILTER, filesystem::DONT_REORDER);

	for(std::vector<std::string>::const_iterator f = files_found.begin();
			f != files_found.end(); ++f) {
		image_existence_map[subdir + *f] = true;
	}

	for(std::vector<std::string>::const_iterator d = dirs_found.begin();
			d != dirs_found.end(); ++d) {
		precache_file_existence_internal(dir, subdir + *d + "/");
	}
}

void precache_file_existence(const std::string& subdir)
{
	const std::vector<std::string>& paths = filesystem::get_binary_paths("images");

	for(std::vector<std::string>::const_iterator p = paths.begin();
			 p != paths.end(); ++p) {

		precache_file_existence_internal(*p, subdir);
	}
}

bool precached_file_exists(const std::string& file)
{
	std::map<std::string, bool>::const_iterator b =  image_existence_map.find(file);
	if (b != image_existence_map.end())
		return b->second;
	else
		return false;
}

bool save_image(const locator & i_locator, const std::string & filename)
{
	return save_image(get_image(i_locator), filename);
}

bool save_image(const surface & surf, const std::string & filename)
{
	if (surf.null()) {
		return false;
	}
#ifdef HAVE_LIBPNG
	if (!filesystem::ends_with(filename, ".bmp")) {
		LOG_DP << "Writing a png image to " << filename << std::endl;

		surface tmp = SDL_PNGFormatAlpha(surf.get());
		//SDL_SavePNG_RW(tmp, filesystem::load_RWops(filename), 1); //1 means to close the file (RWops) when we finish
		//^ This doesn't work, load_RWops is only for reading not writing
		return SDL_SavePNG(tmp, filename.c_str()) == 0;
	}
#endif

	LOG_DP << "Writing a bmp image to " << filename << std::endl;
	return SDL_SaveBMP(surf, filename.c_str()) == 0;
}

bool update_from_preferences()
{
	SCALING_ALGORITHM algo = preferences::default_scaling_algorithm;
	try {
		algo = SCALING_ALGORITHM::string_to_enum(preferences::get("scale_hex"));
	} catch (bad_enum_cast &) {}

	scale_to_hex_func = select_algorithm(algo);

	algo = preferences::default_scaling_algorithm;
	try {
		algo = SCALING_ALGORITHM::string_to_enum(preferences::get("scale_zoom"));
	} catch (bad_enum_cast &) {}

	scale_to_zoom_func = select_algorithm(algo);

	return true;
}

} // end namespace image

