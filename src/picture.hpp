/*
	Copyright (C) 2003 - 2024
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

#pragma once

#include "map/location.hpp"
#include "terrain/translation.hpp"

#include "utils/optional_fwd.hpp"

class surface;
class texture;
struct point;

/**
 * Functions to load and save images from/to disk.
 *
 * image::get_image() and other loading functions implement a pseudo-functional
 * syntax to apply transformations to image files by including them as a suffix
 * to the path (Image Path Functions). They also offer the option to choose
 * between different rendering formats for a single image path according to the
 * display intent -- unscaled, masked to hex, rescaled to zoom, etc.
 *
 * @code
 * surface surf = image::get_image("units/elves-wood/shyde.png~TC(4,magenta)~FL()",
 *                                 image::UNSCALED);
 * @endcode
 *
 * Internally, all loading functions utilize a cache to avoid reading
 * individual images from disk more than once, or wasting valuable CPU time
 * applying potentially expensive transforms every time (e.g. team colors on
 * animated units). The cache can be manually invalidated using
 * image::flush_cache(). Certain functions will invalidate parts of the cache
 * as needed when relevant configuration parameters change in a way that would
 * be expected to alter the output (e.g. Time of Day-tinted images).
 */
namespace image {
/**
 * Generic locator abstracting the location of an image.
 *
 * Constructing locators is somewhat slow, while accessing images through
 * locators is fast. The general idea is that callers should store locators
 * and not strings to construct new ones. (The latter will still work, of
 * course, even if it is slower.)
 */
class locator
{
public:
	enum type { NONE, FILE, SUB_FILE };

	locator() = default;
	locator(locator&&) noexcept = default;
	locator(const locator&) = default;

	locator(const std::string& filename);
	locator(const std::string& filename, const std::string& modifications);
	locator(const std::string& filename, const map_location& loc, int center_x, int center_y, const std::string& modifications = "");

	locator& operator=(const locator& a) = default;
	locator& operator=(locator&&) = default;

	/** Returns a copy of this locator with the given IPF */
	locator clone(const std::string& mods) const;

	bool operator==(const locator& a) const;
	bool operator!=(const locator& a) const { return !operator==(a); }

	bool operator<(const locator& a) const;

	const std::string& get_filename() const { return filename_; }
	bool is_data_uri() const { return is_data_uri_; }
	const map_location& get_loc() const { return loc_ ; }
	int get_center_x() const { return center_x_; }
	int get_center_y() const { return center_y_; }
	const std::string& get_modifications() const { return modifications_; }
	type get_type() const { return type_; }

	/**
	 * Returns @a true if the locator does not correspond to an actual image.
	 */
	bool is_void() const { return type_ == NONE; }

private:
	locator::type type_ = NONE;
	bool is_data_uri_ = false;
	std::string filename_{};
	std::string modifications_{};
	map_location loc_{};
	int center_x_ = 0;
	int center_y_ = 0;

public:
	friend struct std::hash<locator>;
};

// write a readable representation of a locator, mostly for debugging
std::ostream& operator<<(std::ostream&, const locator&);

/**
 * Type used to store color information of central and adjacent hexes.
 *
 * The structure is one or several 4-char blocks: [L,R,G,B]
 * The R, G, B values represent the color, and L the lightmap to use:
 *
 *    -1: none
 *     0: full hex
 *   1-6: concave corners
 *  7-12: convex half-corners 1
 * 13-19: convex half-corners 2
 */
typedef std::basic_string<signed char> light_string;

/**
 * Returns the light_string for one light operation.
 *
 * See light_string for more information.
 */
light_string get_light_string(int op, int r, int g, int b);

/**
 * Purges all image caches.
 */
void flush_cache();

/**
 * Image cache manager.
 *
 * This class is responsible for setting up and flushing the image cache. No
 * more than one instance of it should exist at a time.
 */
struct manager
{
	manager();
	~manager();
};

/**
 * Changes Time of Day color tint for all applicable image types.
 *
 * In particular this affects TOD_COLORED images, as well as
 * images with lightmaps applied. Changing the previous values automatically
 * invalidates all cached images of those types.
 */
void set_color_adjustment(int r, int g, int b);

/**
 * Used to specify the rendering format of images.
 */
enum TYPE
{
	/** Unmodified original-size image. */
	UNSCALED,
	/** Standard hexagonal tile mask applied, removing portions that don't fit. */
	HEXED,
	/** Same as HEXED, but with Time of Day color tint applied. */
	TOD_COLORED,
	NUM_TYPES // Equal to the number of types specified above
};

enum class scale_quality { nearest, linear };

/**
 * Returns an image surface suitable for software manipulation.
 *
 * The equivalent get_texture() function should generally be preferred.
 *
 * Surfaces will be cached for repeat access, unless skip_cache is set.
 *
 * @param i_locator            Image path.
 * @param type                 Rendering format.
 * @param skip_cache           Skip adding the result to the surface cache.
 */
surface get_surface(const locator& i_locator, TYPE type = UNSCALED,
	bool skip_cache = false);

/**
 * Returns an image texture suitable for hardware-accelerated rendering.
 *
 * Texture pointers are not unique, and will be cached and retained
 * until no longer needed. Users of the returned texture do not have to
 * worry about texture management.
 *
 * If caching is disabled via @a skip_cache, texture memory will be
 * automatically freed once the returned object and all other linked
 * textures (if any) are destroyed.
 *
 * @param i_locator            Image path.
 * @param type                 Rendering format.
 * @param skip_cache           Skip adding the result to the surface cache.
 */
texture get_texture(const locator& i_locator, TYPE type = UNSCALED,
	bool skip_cache = false);

texture get_texture(const image::locator& i_locator, scale_quality quality,
	TYPE type = UNSCALED, bool skip_cache = false);

/**
 * Caches and returns an image with a lightmap applied to it.
 *
 * Images will always be HEXED type.
 *
 * @param i_locator            Image path.
 * @param ls                   Light map to apply to the image.
 */
surface get_lighted_image(const image::locator& i_locator, const light_string& ls);
texture get_lighted_texture(const image::locator& i_locator, const light_string& ls);

/**
 * Retrieves the standard hexagonal tile mask.
 */
surface get_hexmask();

/**
 * Returns the width and height of an image.
 *
 * If the image is not yet in the surface cache, it will be loaded and cached
 * unless skip_cache is explicitly set.
 *
 * @param i_locator            Image path.
 * @param skip_cache           If true, do not cache the image if loaded.
 */
point get_size(const locator& i_locator, bool skip_cache = false);

/**
 * Checks if an image fits into a single hex.
 */
bool is_in_hex(const locator& i_locator);

/**
 * Checks if an image is empty after hex masking.
 *
 * This should be only used on terrain images, and it will automatically cache
 * the hex-masked version if necessary.
 */
bool is_empty_hex(const locator& i_locator);

/**
 * Returns @a true if the given image actually exists, without loading it.
 */
bool exists(const locator& i_locator);

/**
 * Precache the existence of files in a binary path subdirectory (e.g. "terrain/").
 */
void precache_file_existence(const std::string& subdir = "");

bool precached_file_exists(const std::string& file);

enum class save_result
{
	success,
	unsupported_format,
	save_failed,
	no_image
};

save_result save_image(const locator& i_locator, const std::string& outfile);
save_result save_image(const surface& surf, const std::string& outfile);

}
