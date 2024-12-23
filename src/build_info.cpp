/*
	Copyright (C) 2015 - 2024
	by Iris Morelle <shadowm2006@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "filesystem.hpp"
#include "formatter.hpp"
#include "gettext.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#include "game_version.hpp"
#include "sound.hpp"
#include "video.hpp"
#include "addon/manager.hpp"
#include "sdl/point.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>

#include "lua/wrapper_lua.h"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include <boost/algorithm/string.hpp>
#include <boost/predef.h>
#include <boost/version.hpp>

#ifndef __APPLE__
#include <openssl/crypto.h>
#include <openssl/opensslv.h>
#endif

#include <curl/curl.h>

#include <pango/pangocairo.h>

#ifdef __APPLE__
// apple_notification.mm uses Foundation.h, which is an Objective-C header;
// but CoreFoundation.h is a C header which also defines these.
#include <CoreFoundation/CoreFoundation.h>
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

std::string format_version(unsigned a, unsigned b, unsigned c)
{
	return formatter() << a << '.' << b << '.' << c;
}

std::string format_version(const SDL_version& v)
{
	return formatter() << static_cast<unsigned>(v.major) << '.'
						<< static_cast<unsigned>(v.minor) << '.'
						<< static_cast<unsigned>(v.patch);
}

#ifndef __APPLE__

std::string format_openssl_patch_level(uint8_t p)
{
	return p <= 26
		? std::string(1, 'a' + static_cast<char>(p) - 1)
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

#endif

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
	// Boost
	//

	compiled[LIB_BOOST] = BOOST_LIB_VERSION;
	std::replace(compiled[LIB_BOOST].begin(), compiled[LIB_BOOST].end(), '_', '.');
	names[LIB_BOOST] = "Boost";

	//
	// Lua
	//

	compiled[LIB_LUA] = LUA_VERSION_MAJOR "." LUA_VERSION_MINOR "." LUA_VERSION_RELEASE;
	names[LIB_LUA] = "Lua";

	//
	// OpenSSL/libcrypto
	//

#ifndef __APPLE__
	compiled[LIB_CRYPTO] = format_openssl_version(OPENSSL_VERSION_NUMBER);
	linked[LIB_CRYPTO] = format_openssl_version(SSLeay());
	names[LIB_CRYPTO] = "OpenSSL/libcrypto";
#endif

	//
	// libcurl
	//

	compiled[LIB_CURL] = format_version(
		(LIBCURL_VERSION_NUM & 0xFF0000) >> 16,
		(LIBCURL_VERSION_NUM & 0x00FF00) >> 8,
		LIBCURL_VERSION_NUM & 0x0000FF);
	curl_version_info_data *curl_ver = curl_version_info(CURLVERSION_NOW);
	if(curl_ver && curl_ver->version) {
		linked[LIB_CURL] = curl_ver->version;
	}
	// This is likely to upset somebody out there, but the cURL authors
	// consistently call it 'libcurl' (all lowercase) in all documentation.
	names[LIB_CURL] = "libcurl";

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
	// Features table.
	//

	features.emplace_back(N_("feature^Lua console completion"));
#ifdef HAVE_HISTORY
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
	// Always compiled in.
	features.emplace_back(N_("feature^Cocoa notifications back end"));
	features.back().enabled = true;
#endif /* __APPLE__ */
}

const std::string empty_version = "";

} // end anonymous namespace 1

std::string build_arch()
{
#if BOOST_ARCH_X86_64
	return "x86_64";
#elif BOOST_ARCH_X86_32
	return "x86";
#elif BOOST_ARCH_ARM && (defined(__arm64) || defined(_M_ARM64))
	return "arm64";
#elif BOOST_ARCH_ARM
	return "arm";
#elif BOOST_ARCH_IA64
	return "ia64";
#elif BOOST_ARCH_PPC
	return "ppc";
#elif BOOST_ARCH_ALPHA
	return "alpha";
#elif BOOST_ARCH_MIPS
	return "mips";
#elif BOOST_ARCH_SPARC
	return "sparc";
#else
	#warning Unrecognized platform or Boost.Predef broken/unavailable
	// Congratulations, you're running Wesnoth on an exotic platform -- either that or you live in
	// the foretold future where x86 and ARM stopped being the dominant CPU architectures for the
	// general-purpose consumer market. If you want to add label support for your platform, check
	// out the Boost.Predef library's documentation and alter the code above accordingly.
	//
	// On the other hand, if you got here looking for Wesnoth's biggest secret let me just say
	// right here and now that Irdya is round. There, I said the thing that nobody has dared say
	// in mainline content before.
	return _("cpu_architecture^<unknown>");
#endif
}

std::vector<optional_feature> optional_features_table(bool localize)
{
	std::vector<optional_feature> res = versions.features;

	for(std::size_t k = 0; k < res.size(); ++k) {
		if(localize) {
			res[k].name = _(res[k].name.c_str());
		} else {
			// Strip annotation carets ("blah blah^actual text here") from translatable
			// strings.
			const auto caret_pos = res[k].name.find('^');
			if(caret_pos != std::string::npos) {
				res[k].name.erase(0, caret_pos + 1);
			}
		}
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

std::string dist_channel_id()
{
	std::string info;
	std::ifstream infofile(game_config::path + "/data/dist");
	if(infofile.is_open()) {
		std::getline(infofile, info);
		infofile.close();
		boost::trim(info);
	}

	if(info.empty()) {
		return "Default";
	}

	return info;
}

namespace {

/**
 * Formats items into a tidy 2-column list with a fixed-length first column.
 */
class list_formatter
{
public:
	using list_entry = std::pair<std::string, std::string>;
	using contents_list = std::vector<list_entry>;

	list_formatter(const std::string& heading, const contents_list& contents = {}, const std::string& empty_placeholder = "")
		: heading_(heading)
		, placeholder_(empty_placeholder)
		, contents_(contents)
	{
	}

	void insert(const std::string& label, const std::string& value)
	{
		contents_.emplace_back(label, value);
	}

	void set_placeholder(const std::string& placeholder)
	{
		placeholder_ = placeholder;
	}

	void stream_put(std::ostream& os) const;

private:
	static const char heading_delimiter;
	static const std::string label_delimiter;

	std::string heading_;
	std::string placeholder_;

	contents_list contents_;
};

const char list_formatter::heading_delimiter = '=';
const std::string list_formatter::label_delimiter = ": ";

void list_formatter::stream_put(std::ostream& os) const
{
	if(!heading_.empty()) {
		os << heading_ << '\n' << std::string(utf8::size(heading_), heading_delimiter) << "\n\n";
	}

	if(contents_.empty() && !placeholder_.empty()) {
		os << placeholder_ << '\n';
	} else if(!contents_.empty()) {
		auto label_length_comparator = [](const list_entry& a, const list_entry& b)
		{
			return utf8::size(a.first) < utf8::size(b.first);
		};

		const auto longest_entry_label = std::max_element(contents_.begin(), contents_.end(), label_length_comparator);
		const std::size_t min_length = longest_entry_label != contents_.end()
				? utf8::size(label_delimiter) + utf8::size(longest_entry_label->first)
				: 0;

		// Save stream attributes for resetting them later after completing the loop
		const std::size_t prev_width = os.width();
		const std::ostream::fmtflags prev_flags = os.flags();

		os << std::left;

		for(const auto& entry : contents_) {
			os << std::setw(min_length) << entry.first + label_delimiter << entry.second << '\n';
		}

		os.width(prev_width);
		os.flags(prev_flags);
	}

	os << '\n';
}

std::ostream& operator<<(std::ostream& os, const list_formatter& fmt)
{
	fmt.stream_put(os);
	return os;
}

list_formatter library_versions_report_internal(const std::string& heading = "")
{
	list_formatter fmt{heading};

	for(unsigned n = 0; n < LIB_COUNT; ++n)
	{
		if(versions.names[n].empty()) {
			continue;
		}

		std::string text = versions.compiled[n];
		if(!versions.linked[n].empty()) {
			text += " (runtime " + versions.linked[n] + ")";
		}

		fmt.insert(versions.names[n], text);
	}

	return fmt;
}

list_formatter optional_features_report_internal(const std::string& heading = "")
{
	list_formatter fmt{heading};

	const std::vector<optional_feature>& features = optional_features_table(false);

	for(const auto& feature : features) {
		fmt.insert(feature.name, feature.enabled ? "yes" : "no");
	}

	return fmt;
}

inline std::string geometry_to_string(point p)
{
	return std::to_string(p.x) + 'x' + std::to_string(p.y);
}

template<typename coordinateType>
inline std::string geometry_to_string(coordinateType horizontal, coordinateType vertical)
{
	// Use a stream in order to control significant digits in non-integers
	return formatter() << std::fixed << std::setprecision(2) << horizontal << 'x' << vertical;
}

std::string format_sdl_driver_list(std::vector<std::string> drivers, const std::string& current_driver)
{
	bool found_current_driver = false;

	for(auto& drvname : drivers) {
		if(current_driver == drvname) {
			found_current_driver = true;
			drvname = "[" + current_driver + "]";
		}
	}

	if(drivers.empty() || !found_current_driver) {
		// This shouldn't happen but SDL is weird at times so whatevs
		drivers.emplace_back("[" + current_driver + "]");
	}

	return utils::join(drivers, " ");
}

list_formatter video_settings_report_internal(const std::string& heading = "")
{
	list_formatter fmt{heading};

	std::string placeholder;

	if(video::headless()) {
		placeholder = "Running in non-interactive mode.";
	}

	if(!video::has_window()) {
		placeholder = "Video not initialized yet.";
	}

	if(!placeholder.empty()) {
		fmt.set_placeholder(placeholder);
		return fmt;
	}

	const auto& current_driver = video::current_driver();
	auto drivers = video::enumerate_drivers();

	const auto& dpi = video::get_dpi();
	std::string dpi_report;

	dpi_report = dpi.first == 0.0f || dpi.second == 0.0f ?
				 "<unknown>" :
				 geometry_to_string(dpi.first, dpi.second);

	fmt.insert("SDL video drivers", format_sdl_driver_list(drivers, current_driver));
	fmt.insert("Window size", geometry_to_string(video::current_resolution()));
	fmt.insert("Game canvas size", geometry_to_string(video::game_canvas_size()));
	fmt.insert("Final render target size", geometry_to_string(video::output_size()));
	fmt.insert("Render refresh rate", std::to_string(video::current_refresh_rate()));
	fmt.insert("Screen refresh rate", std::to_string(video::native_refresh_rate()));
	fmt.insert("Screen dpi", dpi_report);

	const auto& renderer_report = video::renderer_report();

	for(const auto& info : renderer_report) {
		fmt.insert(info.first, info.second);
	}

	return fmt;
}

list_formatter sound_settings_report_internal(const std::string& heading = "")
{
	list_formatter fmt{heading};

	const auto& driver_status = sound::driver_status::query();

	if(!driver_status.initialized) {
		fmt.set_placeholder("Audio not initialized.");
		return fmt;
	}

	const auto& current_driver = sound::current_driver();
	auto drivers = sound::enumerate_drivers();

	static std::map<uint16_t, std::string> audio_format_names = {
		// 8 bits
		{ AUDIO_U8,     "unsigned 8 bit" },
		{ AUDIO_S8,     "signed 8 bit" },
		// 16 bits
		{ AUDIO_U16LSB, "unsigned 16 bit little-endian" },
		{ AUDIO_U16MSB, "unsigned 16 bit big-endian" },
		{ AUDIO_S16LSB, "signed 16 bit little-endian" },
		{ AUDIO_S16MSB, "signed 16 bit big-endian" },
		// 32 bits
		{ AUDIO_S32LSB, "signed 32 bit little-endian" },
		{ AUDIO_S32MSB, "signed 32 bit big-endian" },
		{ AUDIO_F32LSB, "signed 32 bit floating point little-endian" },
		{ AUDIO_F32MSB, "signed 32 bit floating point big-endian" },
	};

	auto fmt_names_it = audio_format_names.find(driver_status.format);
	// If we don't recognize the format id just print the raw number
	const std::string fmt_name = fmt_names_it != audio_format_names.end()
			? fmt_names_it->second
			: formatter() << "0x" << std::setfill('0') << std::setw(2*sizeof(driver_status.format)) << std::hex << std::uppercase << driver_status.format;

	fmt.insert("SDL audio drivers", format_sdl_driver_list(drivers, current_driver));
	fmt.insert("Number of channels", std::to_string(driver_status.channels));
	fmt.insert("Output rate", std::to_string(driver_status.frequency) + " Hz");
	fmt.insert("Sample format", fmt_name);
	fmt.insert("Sample size", std::to_string(driver_status.chunk_size) + " bytes");

	return fmt;
}

} // end anonymous namespace 2

std::string library_versions_report()
{
	return formatter{} << library_versions_report_internal();
}

std::string optional_features_report()
{
	return formatter{} << optional_features_report_internal();
}

std::string full_build_report()
{
	list_formatter::contents_list paths{
		{"Data dir",        game_config::path},
		{"User data dir",   filesystem::get_user_data_dir()},
		{"Saves dir",       filesystem::get_saves_dir()},
		{"Add-ons dir",     filesystem::get_addons_dir()},
		{"Cache dir",       filesystem::get_cache_dir()},
		{"Logs dir",        filesystem::get_logs_dir()},
	};

	// Obfuscate usernames in paths
	for(auto& entry : paths) {
		entry.second = filesystem::sanitize_path(entry.second);
	}

	list_formatter::contents_list addons;

	for(const auto& addon_info : installed_addons_and_versions()) {
		addons.emplace_back(addon_info.first, addon_info.second);
	}

	std::ostringstream o;

	o << "The Battle for Wesnoth version " << game_config::revision << " " << build_arch() << '\n'
	  << "Running on " << desktop::os_version() << '\n'
	  << "Distribution channel: " << dist_channel_id() << '\n'
	  << '\n'
	  << list_formatter{"Game paths", paths}
	  << library_versions_report_internal("Libraries")
	  << optional_features_report_internal("Features")
	  << video_settings_report_internal("Current video settings")
	  << sound_settings_report_internal("Current audio settings")
	  << list_formatter("Installed add-ons", addons, "No add-ons installed.");

	return o.str();
}

} // end namespace game_config
