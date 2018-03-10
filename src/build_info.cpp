/*
   Copyright (C) 2015 - 2018 by Iris Morelle <shadowm2006@gmail.com>
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

#include "build_info.hpp"

#include "desktop/version.hpp"
#include "game_config.hpp"
#include "filesystem.hpp"
#include "formatter.hpp"
#include "gettext.hpp"

#include <algorithm>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include <boost/version.hpp>

#include <openssl/crypto.h>
#include <openssl/opensslv.h>

#include <pango/pangocairo.h>

#ifdef HAVE_LIBPNG
#include <png.h>
#endif

#ifdef __APPLE__
// apple_notification.mm uses Foundation.h, which is an Objective-C header;
// but CoreFoundation.h is a C header which also defines these.
#include <CoreFoundation/CoreFoundation.h>

#if (defined MAC_OS_X_VERSION_10_8) && (MAC_OS_X_VERSION_10_8 <= MAC_OS_X_VERSION_MAX_ALLOWED)
#define HAVE_NS_USER_NOTIFICATION
#endif
#endif

namespace game_config
{

namespace {

struct version_table_manager
{
	std::vector<std::string> compiled, linked, names;
	std::vector<optional_feature> features;

	version_table_manager();
};

const version_table_manager versions;

#if 0
std::string format_version(unsigned a, unsigned b, unsigned c)
{
	return formatter() << a << '.' << b << '.' << c;
}
#endif

std::string format_version(const SDL_version& v)
{
	return formatter() << unsigned(v.major) << '.'
						<< unsigned(v.minor) << '.'
						<< unsigned(v.patch);
}

std::string format_openssl_patch_level(uint8_t p)
{
	return p <= 26
		? std::string(1, 'a' + char(p) - 1)
		: "patch" + std::to_string(p);
}

std::string format_openssl_version(long v)
{
	int major, minor, fix, patch, status;
	std::ostringstream fmt;

	//
	// The people who maintain OpenSSL are not from this world. I suppose it's
	// only fair that I'm the one who gets to try to make sense of their version
	// encoding scheme.  -- shadowm
	//

	if(v < 0x0930L) {
		// Pre-0.9.3 seems simpler times overall.
		minor = v & 0x0F00L >> 8;
		fix   = v & 0x00F0L >> 4;
		patch = v & 0x000FL;

		fmt << "0." << minor << '.' << fix;
		if(patch) {
			fmt << format_openssl_patch_level(patch);
		}
	} else {
		//
		// Note that they either assume the major version will never be greater than
		// 9, they plan to use hexadecimal digits for versions 10.x.x through
		// 15.x.x, or they expect long to be always > 32-bits by then. Who the hell
		// knows, really.
		//
		major  = (v & 0xF0000000L) >> 28;
		minor  = (v & 0x0FF00000L) >> 20;
		fix    = (v & 0x000FF000L) >> 12;
		patch  = (v & 0x00000FF0L) >> 4;
		status = (v & 0x0000000FL);

		if(v < 0x00905100L) {
			//
			// From wiki.openssl.org (also mentioned in opensslv.h, in the most oblique
			// fashion possible):
			//
			// "Versions between 0.9.3 and 0.9.5 had a version identifier with this interpretation:
			// MMNNFFRBB major minor fix final beta/patch"
			//
			// Both the wiki and opensslv.h fail to accurately list actual version
			// numbers that ended up used in the wild -- e.g. 0.9.3a is supposedly
			// 0x0090301f when it really was 0x00903101.
			//
			const uint8_t is_final = (v & 0xF00L) >> 8;
			status = is_final ? 0xF : 0;
			patch = v & 0xFFL;
		} else if(v < 0x00906000L) {
			//
			// Quoth opensslv.h:
			//
			// "For continuity reasons (because 0.9.5 is already out, and is coded
			// 0x00905100), between 0.9.5 and 0.9.6 the coding of the patch level
			// part is slightly different, by setting the highest bit. This means
			// that 0.9.5a looks like this: 0x0090581f. At 0.9.6, we can start
			// with 0x0090600S..."
			//
			patch ^= 1 << 7;
		}

		fmt << major << '.' << minor << '.' << fix;

		if(patch) {
			fmt << format_openssl_patch_level(patch);
		}

		if(status == 0x0) {
			fmt << "-dev";
		} else if(status < 0xF) {
			fmt << "-beta" << status;
		}
	}

	return fmt.str();

}

version_table_manager::version_table_manager()
	: compiled(LIB_COUNT, "")
	, linked(LIB_COUNT, "")
	, names(LIB_COUNT, "")
	, features()
{
	SDL_version sdl_version;


	//
	// SDL
	//

	SDL_VERSION(&sdl_version);
	compiled[LIB_SDL] = format_version(sdl_version);

	SDL_GetVersion(&sdl_version);
	linked[LIB_SDL] = format_version(sdl_version);

	names[LIB_SDL] = "SDL";

	//
	// SDL_image
	//

	SDL_IMAGE_VERSION(&sdl_version);
	compiled[LIB_SDL_IMAGE] = format_version(sdl_version);

	const SDL_version* sdl_rt_version = IMG_Linked_Version();
	if(sdl_rt_version) {
		linked[LIB_SDL_IMAGE] = format_version(*sdl_rt_version);
	}

	names[LIB_SDL_IMAGE] = "SDL_image";

	//
	// SDL_mixer
	//

	SDL_MIXER_VERSION(&sdl_version);
	compiled[LIB_SDL_MIXER] = format_version(sdl_version);

	sdl_rt_version = Mix_Linked_Version();
	if(sdl_rt_version) {
		linked[LIB_SDL_MIXER] = format_version(*sdl_rt_version);
	}

	names[LIB_SDL_MIXER] = "SDL_mixer";

	//
	// SDL_ttf
	//

	SDL_TTF_VERSION(&sdl_version);
	compiled[LIB_SDL_TTF] = format_version(sdl_version);

	sdl_rt_version = TTF_Linked_Version();
	if(sdl_rt_version) {
		linked[LIB_SDL_TTF] = format_version(*sdl_rt_version);
	}

	names[LIB_SDL_TTF] = "SDL_ttf";

	//
	// Boost
	//

	compiled[LIB_BOOST] = BOOST_LIB_VERSION;
	std::replace(compiled[LIB_BOOST].begin(), compiled[LIB_BOOST].end(), '_', '.');
	names[LIB_BOOST] = "Boost";

	//
	// OpenSSL/libcrypto
	//

	compiled[LIB_CRYPTO] = format_openssl_version(OPENSSL_VERSION_NUMBER);
	linked[LIB_CRYPTO] = format_openssl_version(SSLeay());
	names[LIB_CRYPTO] = "OpenSSL/libcrypto";

	//
	// Cairo
	//

	compiled[LIB_CAIRO] = CAIRO_VERSION_STRING;
	linked[LIB_CAIRO] = cairo_version_string();
	names[LIB_CAIRO] = "Cairo";

	//
	// Pango
	//

	compiled[LIB_PANGO] = PANGO_VERSION_STRING;
	linked[LIB_PANGO] = pango_version_string();
	names[LIB_PANGO] = "Pango";

	//
	// libpng
	//

#ifdef HAVE_LIBPNG
	compiled[LIB_PNG] = PNG_LIBPNG_VER_STRING;
	linked[LIB_PNG] = png_get_libpng_ver(nullptr);
	names[LIB_PNG] = "libpng";
#endif

	//
	// Features table.
	//

	features.emplace_back(N_("feature^Experimental OpenMP support"));
#ifdef _OPENMP
	features.back().enabled = true;
#endif

	features.emplace_back(N_("feature^PNG screenshots"));
#ifdef HAVE_LIBPNG
	features.back().enabled = true;
#endif

	features.emplace_back(N_("feature^Lua console completion"));
#ifdef HAVE_HISTORY
	features.back().enabled = true;
#endif

	features.emplace_back(N_("feature^Legacy bidirectional rendering"));
#ifdef HAVE_FRIBIDI
	features.back().enabled = true;
#endif

#ifdef _X11

	features.emplace_back(N_("feature^D-Bus notifications back end"));
#ifdef HAVE_LIBDBUS
	features.back().enabled = true;
#endif

#endif /* _X11 */

#ifdef _WIN32
	// Always compiled in.
	features.emplace_back(N_("feature^Win32 notifications back end"));
	features.back().enabled = true;
#endif

#ifdef __APPLE__

	features.emplace_back(N_("feature^Cocoa notifications back end"));
#ifdef HAVE_NS_USER_NOTIFICATION
	features.back().enabled = true;
#endif

	features.emplace_back(N_("feature^Growl notifications back end"));
#ifdef HAVE_GROWL
	features.back().enabled = true;
#endif

#endif /* __APPLE__ */
}

const std::string empty_version = "";

} // end anonymous namespace 1

std::vector<optional_feature> optional_features_table()
{
	std::vector<optional_feature> res = versions.features;

	for(size_t k = 0; k < res.size(); ++k) {
		res[k].name = _(res[k].name.c_str());
	}
	return res;
}

const std::string& library_build_version(LIBRARY_ID lib)
{
	if(lib >= LIB_COUNT) {
		return empty_version;
	}

	return versions.compiled[lib];
}

const std::string& library_runtime_version(LIBRARY_ID lib)
{
	if(lib >= LIB_COUNT) {
		return empty_version;
	}

	return versions.linked[lib];
}

const std::string& library_name(LIBRARY_ID lib)
{
	if(lib >= LIB_COUNT) {
		return empty_version;
	}

	return versions.names[lib];
}

namespace {

bool strlen_comparator(const std::string& a, const std::string& b)
{
	return a.length() < b.length();
}

size_t max_strlen(const std::vector<std::string>& strs)
{
	const std::vector<std::string>::const_iterator it =
			std::max_element(strs.begin(), strs.end(), strlen_comparator);

	return it != strs.end() ? it->length() : 0;
}

} // end anonymous namespace 2

std::string library_versions_report()
{
	std::ostringstream o;

	const size_t col2_start = max_strlen(versions.names) + 2;
	const size_t col3_start = max_strlen(versions.compiled) + 1;

	for(unsigned n = 0; n < LIB_COUNT; ++n)
	{
		const std::string& name = versions.names[n];
		const std::string& compiled = versions.compiled[n];
		const std::string& linked = versions.linked[n];

		if(name.empty()) {
			continue;
		}

		o << name << ": ";

		const size_t pos2 = name.length() + 2;
		if(pos2 < col2_start) {
			o << std::string(col2_start - pos2, ' ');
		}

		o << compiled;

		if(!linked.empty()) {
			const size_t pos3 = compiled.length() + 1;
			if(pos3 < col3_start) {
				o << std::string(col3_start - pos3, ' ');
			}
			o << " (runtime " << linked << ")";
		}

		o << '\n';
	}

	return o.str();
}

std::string optional_features_report()
{
	// Yes, it's for stdout/stderr but we still want the localized version so
	// that the context prefixes are stripped.
	const std::vector<optional_feature>& features = optional_features_table();

	size_t col2_start = 0;

	for(size_t k = 0; k < features.size(); ++k)
	{
		col2_start = std::max(col2_start, features[k].name.length() + 2);
	}

	std::ostringstream o;

	for(size_t k = 0; k < features.size(); ++k)
	{
		const optional_feature& f = features[k];

		o << f.name << ": ";

		const size_t pos2 = f.name.length() + 2;
		if(pos2 < col2_start) {
			o << std::string(col2_start - pos2, ' ');
		}

		o << (f.enabled ? "yes" : "no") << '\n';
	}

	return o.str();
}

std::string full_build_report()
{
	std::ostringstream o;

	o << "The Battle for Wesnoth version " << game_config::revision << '\n'
	  << "Running on " << desktop::os_version() << '\n'
	  << '\n'
	  << "Game paths\n"
	  << "==========\n"
	  << '\n'
	  << "Data dir:        " << filesystem::sanitize_path(game_config::path) << '\n'
	  << "User config dir: " << filesystem::sanitize_path(filesystem::get_user_config_dir()) << '\n'
	  << "User data dir:   " << filesystem::sanitize_path(filesystem::get_user_data_dir()) << '\n'
	  << "Saves dir:       " << filesystem::sanitize_path(filesystem::get_saves_dir()) << '\n'
	  << "Add-ons dir:     " << filesystem::sanitize_path(filesystem::get_addons_dir()) << '\n'
	  << "Cache dir:       " << filesystem::sanitize_path(filesystem::get_cache_dir()) << '\n'
	  << '\n'
	  << "Libraries\n"
	  << "=========\n"
	  << '\n'
	  << game_config::library_versions_report()
	  << '\n'
	  << "Features\n"
	  << "========\n"
	  << '\n'
	  << game_config::optional_features_report();

	return o.str();
}

} // end namespace game_config
