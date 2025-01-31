/*
	Copyright (C) 2016 - 2024
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

#include "desktop/paths.hpp"

#include "filesystem.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "preferences/preferences.hpp"
#include "serialization/unicode.hpp"
#include "utils/general.hpp"

#if !defined(_WIN32) && !defined(__APPLE__)
#include <boost/filesystem.hpp>
#endif

#ifndef _WIN32

// For username stuff on Unix:
#include <pwd.h>

#else // _WIN32

#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <shlobj.h>

#endif

static lg::log_domain log_desktop("desktop");
#define ERR_DU LOG_STREAM(err,   log_desktop)
#define LOG_DU LOG_STREAM(info,  log_desktop)
#define DBG_DU LOG_STREAM(debug, log_desktop)

namespace desktop
{

namespace
{

void enumerate_storage_devices(std::vector<path_info>& res)
{
#ifdef _WIN32

	const DWORD drive_table = GetLogicalDrives();

	for(unsigned n = 0; n < 26; ++n) {
		if((drive_table >> n) & 1) {
			std::string u8drive = "A:";
			u8drive[0] += n;

			LOG_DU << "enumerate_win32_drives(): " << u8drive << " is reported to be present";

			wchar_t drive[] = L"A:\\";
			drive[0] += n;

			const DWORD label_bufsize = MAX_PATH + 1;
			wchar_t label[label_bufsize] { 0 };

			if(GetVolumeInformation(drive, label, label_bufsize, nullptr, nullptr, nullptr, nullptr, 0) == 0) {
				// Probably an empty removable drive, just ignore it and carry on.
				const DWORD err = GetLastError();
				LOG_DU << "enumerate_win32_drives(): GetVolumeInformation() failed (" << err << ")";
				continue;
			}

			// Trailing slash so that we don't get compatibility per-drive working dirs
			// involved in path resolution.
			res.push_back({u8drive, unicode_cast<std::string>(std::wstring{label}), u8drive + '\\'});
		}
	}

#elif defined(__APPLE__)

	// Probably as unreliable as /media|/mnt on other platforms, not worth
	// examining in detail.
	res.push_back({{ N_("filesystem_path_system^Volumes"), GETTEXT_DOMAIN }, "", "/Volumes"});

#else

	namespace bsys = boost::system;
	namespace bfs = boost::filesystem;

	// These are either used as mount points themselves, or host mount points. The
	// reasoning here is that if any or all of them are non-empty, they are
	// probably used for _something_ that might be of interest to the user (if not
	// directly and actively controlled by the user themselves).
	// Note that the first candidate is actually /run/media/USERNAME -- we need
	// to fetch the username at runtime from passwd to complete the path.
	std::vector<std::string> candidates { "/media", "/mnt" };

	// Fetch passwd entry for the effective user the current process runs as
	if(const passwd* pw = getpwuid(geteuid()); pw && pw->pw_name && pw->pw_name[0]) {
		candidates.emplace(candidates.begin(), "/run/media/");
		candidates.front() += pw->pw_name;
	}

	for(const auto& mnt : candidates) {
		bsys::error_code e;
		try {
			if(bfs::is_directory(mnt, e) && !bfs::is_empty(mnt, e) && !e) {
				DBG_DU << "enumerate_mount_parents(): " << mnt << " appears to be a non-empty dir";
				res.push_back({mnt, "", mnt});
			}
		}
		catch(...) {
			//bool is_empty(const path& p, system::error_code& ec) might throw.
			//For example if you have no permission on that directory. Don't list the file in that case.
			DBG_DU << "caught exception " << utils::get_unknown_exception_type() << " in enumerate_storage_devices";
		}
	}

#endif
}

bool have_path(const std::vector<path_info>& pathset, const std::string& path)
{
	for(const auto& pinfo : pathset) {
		if(pinfo.path == path) {
			return true;
		}
	}

	return false;
}

inline std::string pretty_path(const std::string& path)
{
	return filesystem::normalize_path(path, true, true);
}

inline config get_bookmarks_config()
{
	const auto& cfg = prefs::get().dir_bookmarks();
	return cfg.has_value() ? cfg.value() : config();

//	return cfg ? *cfg : config{};
}

inline void commit_bookmarks_config(config& cfg)
{
	prefs::get().set_dir_bookmarks(cfg);
}

} // unnamed namespace

std::string user_profile_dir()
{
#ifndef _WIN32

	// TODO: The filesystem API uses $HOME for this purpose, which may be
	//       overridden or missing. Not sure which one really makes more sense
	//       for us here.
	const passwd* const pwd = getpwuid(geteuid());

	if(!pwd || !pwd->pw_dir || !*pwd->pw_dir) {
		return "";
	}

	return pwd->pw_dir;

#else // _WIN32

	wchar_t profile_path[MAX_PATH];
	HRESULT res = SHGetFolderPath(nullptr, CSIDL_PROFILE, nullptr, SHGFP_TYPE_CURRENT, profile_path);
	return res != S_OK ? "" : unicode_cast<std::string>(std::wstring{profile_path});

#endif // _WIN32
}

std::string path_info::display_name() const
{
	return label.empty() ? name : label + " (" + name + ")";
}

std::ostream& operator<<(std::ostream& os, const path_info& pinf)
{
	return os << pinf.name << " [" << pinf.label << "] - " << pinf.path;
}

std::vector<path_info> game_paths(const std::set<GAME_PATH_TYPES>& paths)
{
	static const std::string& game_bin_dir = pretty_path(filesystem::get_exe_dir());
	static const std::string& game_data_dir = pretty_path(game_config::path);
	static const std::string& game_user_data_dir = pretty_path(filesystem::get_user_data_dir());
	static const std::string& game_editor_map_dir = pretty_path(filesystem::get_legacy_editor_dir() + "/maps");

	std::vector<path_info> res;

	if(paths.count(GAME_BIN_DIR) > 0 && !have_path(res, game_bin_dir)) {
		res.push_back({{ N_("filesystem_path_game^Game executables"), GETTEXT_DOMAIN }, "", game_bin_dir});
	}

	if(paths.count(GAME_CORE_DATA_DIR) > 0 && !have_path(res, game_data_dir)) {
		res.push_back({{ N_("filesystem_path_game^Game data"), GETTEXT_DOMAIN }, "", game_data_dir});
	}

	if(paths.count(GAME_USER_DATA_DIR) > 0 && !have_path(res, game_user_data_dir)) {
		res.push_back({{ N_("filesystem_path_game^User data"), GETTEXT_DOMAIN }, "", game_user_data_dir});
	}

	if(paths.count(GAME_EDITOR_MAP_DIR) > 0 && !have_path(res, game_editor_map_dir)) {
		res.push_back({{ N_("filesystem_path_game^Editor maps"), GETTEXT_DOMAIN }, "", game_editor_map_dir});
	}

	return res;
}

std::vector<path_info> system_paths(const std::set<SYSTEM_PATH_TYPES>& paths)
{
	static const std::string& home_dir = user_profile_dir();

	std::vector<path_info> res;

	if(paths.count(SYSTEM_USER_PROFILE) > 0 && !home_dir.empty()) {
		res.push_back({{ N_("filesystem_path_system^Home"), GETTEXT_DOMAIN }, "", home_dir});
	}

	if(paths.count(SYSTEM_ALL_DRIVES) > 0) {
		enumerate_storage_devices(res);
	}

#ifndef _WIN32
	if(paths.count(SYSTEM_ROOTFS) > 0) {
		res.push_back({{ N_("filesystem_path_system^Root"), GETTEXT_DOMAIN }, "", "/"});
	}
#endif

	return res;
}

unsigned add_user_bookmark(const std::string& label, const std::string& path)
{
	config cfg = get_bookmarks_config();

	config& bookmark_cfg = cfg.add_child("bookmark");
	bookmark_cfg["label"] = label;
	bookmark_cfg["path"]  = path;

	commit_bookmarks_config(cfg);

	return cfg.child_count("bookmark");
}

void remove_user_bookmark(unsigned index)
{
	config cfg = get_bookmarks_config();
	const unsigned prev_size = cfg.child_count("bookmark");

	if(index < prev_size) {
		cfg.remove_child("bookmark", index);
	}

	commit_bookmarks_config(cfg);
}

std::vector<bookmark_info> user_bookmarks()
{
	const config& cfg = get_bookmarks_config();
	std::vector<bookmark_info> res;

	if(cfg.has_child("bookmark")) {
		for(const config& bookmark_cfg : cfg.child_range("bookmark")) {
			res.push_back({ bookmark_cfg["label"], bookmark_cfg["path"] });
		}
	}

	return res;
}

} // namespace desktop
